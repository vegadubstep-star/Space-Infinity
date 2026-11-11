#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "EffectControls.h"
#include "FactoryRefinements.h"
#include "StudioParameters.h"
#include "FactoryStudio.h"
#include <BinaryData.h>
#include <algorithm>
#include <cmath>
#include <cstring>
namespace {
using APVTS=juce::AudioProcessorValueTreeState;
APVTS::ParameterLayout makeLayout(){
    APVTS::ParameterLayout p;
    auto f=[&](const char* id,const char* label,float low,float high,float def){p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1},label,juce::NormalisableRange<float>(low,high),def));};
    f("cutoff","Filter Cutoff",80,16000,8000);f("res","Resonance",0,0.95f,0.12f);
    f("attack","Attack",0.0f,1.0f,0.01f);f("release","Release",0.0f,1.0f,0.5f);
    f("ampDecay","Amp Decay",0.005f,8.0f,0.35f);
    f("ampSustain","Amp Sustain",0.0f,1.0f,1.0f);
    p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"osc1Enabled",1},"Oscillator 1 Enabled",true));
    p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"osc2Enabled",1},"Oscillator 2 Enabled",true));
    p.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"osc1Unison",1},"Oscillator 1 Unison",1,16,1));
    p.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"osc2Unison",1},"Oscillator 2 Unison",1,16,1));
    f("noiseLevel","Noise Level",0.0f,1.0f,0.0f);
    p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"noiseType",1},"Noise Type",juce::StringArray{"White","Dark","Bright"},0));
    const char* octaveIds[]={"masterOctave","osc1Octave","osc2Octave"};
    const char* octaveNames[]={"Master Octave","Oscillator 1 Octave","Oscillator 2 Octave"};
    for(int i=0;i<3;++i)
        p.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{octaveIds[i],1},octaveNames[i],-3,3,0));
    p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"eqEnabled",1},"EQ Enabled",false));
    for(int band=0;band<7;++band){
        const auto name="EQ "+juce::String(band+1)+" ";
        for(int field=0;field<3;++field){
            const char* fields[]={"Frequency","Gain","Q"};
            const float minimum[]={20.0f,-18.0f,0.2f};
            const float maximum[]={20000.0f,18.0f,12.0f};
            const float initial[]={SpaceInfinityProcessor::eqFrequencies[size_t(band)],0.0f,1.0f};
            p.add(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID{SpaceInfinityProcessor::eqId(band,fields[field]),1},
                name+fields[field],juce::NormalisableRange<float>(minimum[field],maximum[field]),initial[field]));
        }
    }
    for(int oscillator=1;oscillator<=2;++oscillator){
        const auto prefix="osc"+juce::String(oscillator);
        const auto label="Oscillator "+juce::String(oscillator)+" Warp";
        p.add(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{prefix+"Warp",1},label,
            juce::StringArray{"Off",oscillator==1?"FM from B":"FM from A","Mirror","PWM"},0));
        p.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{prefix+"WarpAmount",1},label+" Amount",
            juce::NormalisableRange<float>(0.0f,1.0f),0.0f));
    }
    for(const auto& control:effectControls)
        if(control.added){
            const auto label=juce::String(effectRows[size_t(control.row)])+" "+control.label;
            f(control.id,label.toRawUTF8(),control.minimum,control.maximum,control.initial);
        }
    for(const auto& c:studioControls)f(c.id,c.label,c.low,c.high,c.value);
    const auto flag=[&](const char* id,const char* name,bool value){p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{id,1},name,value));};
    const auto menu=[&](const char* id,const char* name,juce::StringArray items,int value){p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{id,1},name,items,value));};
    menu("compMode","Compressor mode",{"Legacy","Single","Multiband"},0);flag("compLimit","Peak ceiling",false);
    menu("echoMode","Delay mode",{"Legacy","Normal","Ping-pong","Tap -> Delay"},0);
    menu("echoSyncL","Delay L sync",{"ms","1/64","1/32","1/16","1/8","1/4","1/2","1 bar"},0);
    menu("echoSyncR","Delay R sync",{"ms","1/64","1/32","1/16","1/8","1/4","1/2","1 bar"},0);
    flag("echoLink","Delay time link",true);flag("echoHQ","Delay high quality",true);
    menu("irPreset","Impulse",{"Custom / imported","Small Room","Bright Plate","Large Hall","Metal Chamber"},0);
    flag("irMinimumPhase","IR minimum phase",false);flag("irReverse","IR reverse",false);
    menu("irSync","IR pre-delay sync",{"ms","1/64","1/32","1/16","1/8","1/4"},0);
    flag("gateEnabled","Trance gate enabled",false);flag("pumpEnabled","Sidechain enabled",false);
    menu("gateSplit","Gate / sidechain frequency range",{"Full range","Bass only","Mid band","High band"},0);
    menu("gateRate","Gate step division",{"1/64","1/32","1/16","1/8","1/4","1/2","1 bar"},2);
    menu("pumpRate","Sidechain cycle",{"1/16","1/8","1/4","1/2","1 bar","2 bars","4 bars"},2);
    menu("sidechainSource","Sidechain source",{"Internal pump","External audio"},0);
    f("gateDepth","Gate depth",0,1,1);f("gateAttack","Gate attack",.1f,50,3);f("gateRelease","Gate release",.1f,300,35);
    f("pumpDepth","Sidechain depth",0,1,.7f);f("pumpAttack","Sidechain attack",.1f,100,3);f("pumpRelease","Sidechain release",10,1000,180);
    f("sidechainThreshold","Sidechain threshold",-60,0,-24);f("sidechainRatio","Sidechain ratio",1,20,8);
    f("gateLow","Split low",20,4000,180);f("gateHigh","Split high",200,16000,2600);
    for(int i=0;i<16;++i)f(("gateStep"+juce::String(i)).toRawUTF8(),("Gate step "+juce::String(i+1)).toRawUTF8(),0,1,studio::gatePatterns[0][size_t(i)]);
    for(int i=0;i<7;++i){menu(("eqMode"+juce::String(i)).toRawUTF8(),("EQ "+juce::String(i+1)+" shape").toRawUTF8(),{"Bell","Low shelf","High shelf","Low-pass","High-pass","Notch","Band-pass"},0);
        menu(("eqSlope"+juce::String(i)).toRawUTF8(),("EQ "+juce::String(i+1)+" slope").toRawUTF8(),{"12 dB/oct","24 dB/oct","36 dB/oct","48 dB/oct"},0);}
    for(size_t row=0;row<effectRows.size();++row)
        p.add(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{effectEnableIds[row],1},juce::String(effectRows[row])+" Enabled",true));
    p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"ampTimed",1},"Timed Amp Envelope",false));
    f("analogueCharacter","Analogue Character",0.0f,1.0f,0.0f);
    p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"analogueUpgrade",1},"Analogue Voice Behaviour",false));
    f("osc1Tune","Oscillator 1 Fine Tune",-50.0f,50.0f,0.0f);
    f("osc2Tune","Oscillator 2 Fine Tune",-50.0f,50.0f,0.0f);
    f("osc1Width","Oscillator 1 Width",0.0f,1.0f,0.0f);
    f("osc2Width","Oscillator 2 Width",0.0f,1.0f,0.0f);
    p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"osc2Wave",1},"Oscillator 2 Waveform",SpaceInfinityProcessor::oscillatorWaves(true),0));
    f("detune","Detune",0,0.04f,0.008f);f("mix","Layer Mix",0,1,0.55f);
    f("drive","Drive",0,1,0.15f);f("chorus","Chorus",0,1,0.25f);
    f("flanger","Flanger Mix",0.0f,1.0f,0.0f);
    f("phaser","Phaser Mix",0.0f,1.0f,0.0f);
    f("delay","Delay",0,0.8f,0.17f);f("reverb","Reverb",0,0.8f,0.25f);
    f("lforate","LFO Rate",0.1f,12,4.0f);f("lfodepth","LFO Depth",0,1,0.1f);
    f("gain","Output",0,1.2f,0.65f);
    p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"wave",1},"Oscillator",SpaceInfinityProcessor::oscillatorWaves(),0));
    p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"source",1},"Sound Source",SpaceInfinityProcessor::sourceNames(),0));
    p.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"sampleRoot",1},"Imported Sample Root (MIDI)",0,127,60));
    auto choice=[&](const char* id,const char* label,juce::StringArray options,int def){
        p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{id,1},label,options,def));
    };
    f("sampleStart","Sample Trim Start",0.0f,1.0f,0.0f);
    f("sampleEnd","Sample Trim End",0.0f,1.0f,1.0f);
    f("motionDepth","Motion to Cutoff",0.0f,1.0f,0.0f);
    choice("motionBars","Motion Length",{"1 bar","2 bars","4 bars","8 bars","1/2 bar","1/4 bar","1/8 bar","1/16 bar"},2);
    for(int i=0;i<17;++i)
        p.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{SpaceInfinityProcessor::motionPointId(i),1},
            "Motion Point "+juce::String(i+1),juce::NormalisableRange<float>(0.0f,1.0f),float(i)/16.0f));
    const auto macros=SpaceInfinityProcessor::macroNames();
    for(int i=0;i<macros.size();++i){
        const auto suffix=juce::String(i);
        p.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"macro"+suffix,1},macros[i],juce::NormalisableRange<float>(-1.0f,1.0f),0.0f));
        for(int slot=0;slot<3;++slot){
            const auto label=macros[i]+" Route "+juce::String(slot+1);
            p.add(std::make_unique<juce::AudioParameterChoice>(
                juce::ParameterID{SpaceInfinityProcessor::macroRouteId(i,slot,false),1},
                label+" Destination",SpaceInfinityProcessor::macroTargets(),slot==0?i+1:0));
            p.add(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID{SpaceInfinityProcessor::macroRouteId(i,slot,true),1},
                label+" Depth",juce::NormalisableRange<float>(-1.0f,1.0f),1.0f));
        }
    }
    choice("voiceMode","Voice Mode",{"Poly","Mono","Legato"},0);
    f("filterAttack","Filter Attack",0,1,.01f);f("filterDecay","Filter Decay",.005f,8,.35f);
    f("filterSustain","Filter Sustain",0,1,.4f);f("filterRelease","Filter Release",0,1,.4f);
    f("filterEnvAmount","Filter Envelope Amount",-6,6,0);
    f("grainPosition","Grain Position",0,1,.3f);f("grainSize","Grain Size (ms)",5,500,90);
    f("grainDensity","Grains Per Second",1,100,20);f("grainSpray","Grain Position Spray",0,1,.1f);
    f("grainSpread","Grain Stereo Spread",0,1,.6f);
    f("wavetableScan","Wavetable Frame Scan",0,1,0);
    choice("patternMode","Note Pattern",{"Off","Arpeggiator","Sequencer","Arp + Sequencer"},0);
    choice("arpOrder","Arp Order",{"Up","Down","Up-Down","Random"},0);
    choice("patternRate","Pattern Rate",{"1/4","1/8","1/16","1/32","1/8 triplet","1/16 triplet"},2);
    p.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"arpOctaves",1},"Arp Octaves",1,4,1));
    p.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"sequenceLength",1},"Sequence Length",1,16,16));
    f("patternGate","Note Gate",.05f,.95f,.65f);
    for(int i=0;i<16;++i){
        const auto id=juce::String(i);
        p.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"stepPitch"+id,1},"Step "+juce::String(i+1)+" Semitones",-24,24,0));
        p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"stepVelocity"+id,1},"Step "+juce::String(i+1)+" Velocity",juce::NormalisableRange<float>(.01f,1),.8f));
        p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"stepOn"+id,1},"Step "+juce::String(i+1)+" Enabled",true));
    }
    p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"convolverEnabled",1},"Convolver Enabled",false));
    f("convolverMix","Convolver Mix",0,1,.25f);

    for(int lfo=0;lfo<3;++lfo){
        const auto title="LFO "+juce::String(lfo+1);
        p.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{SpaceInfinityProcessor::lfoParameterId(lfo,"Speed"),1},title+" Speed",
            juce::NormalisableRange<float>(0.01f,20.0f),1.0f));
        p.add(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{SpaceInfinityProcessor::lfoParameterId(lfo,"Sync"),1},title+" Sync",juce::StringArray{"Hz","BPM"},1));
        p.add(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{SpaceInfinityProcessor::lfoParameterId(lfo,"Mode"),1},title+" Mode",juce::StringArray{"Free","Retrigger","One-shot"},0));
        if(lfo>0){
            p.add(std::make_unique<juce::AudioParameterChoice>(
                juce::ParameterID{SpaceInfinityProcessor::lfoParameterId(lfo,"Bars"),1},title+" Length",juce::StringArray{"1 bar","2 bars","4 bars","8 bars","1/2 bar","1/4 bar","1/8 bar","1/16 bar"},2));
            for(int point=0;point<17;++point)
                p.add(std::make_unique<juce::AudioParameterFloat>(
                    juce::ParameterID{SpaceInfinityProcessor::lfoPointId(lfo,point),1},
                    title+" Point "+juce::String(point+1),juce::NormalisableRange<float>(0.0f,1.0f),
                    lfo==1?1.0f-std::abs(float(point)/8.0f-1.0f):float(point)/16.0f));
        }
    }
    for(int source=4;source<8;++source){
        for(int slot=0;slot<3;++slot){
            const auto label=(source==7?juce::String("Amp Envelope"):"LFO "+juce::String(source-3))+" Route "+juce::String(slot+1);
            p.add(std::make_unique<juce::AudioParameterChoice>(
                juce::ParameterID{SpaceInfinityProcessor::modulationRouteId(source,slot,false),1},
                label+" Destination",SpaceInfinityProcessor::macroTargets(),0));
            p.add(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID{SpaceInfinityProcessor::modulationRouteId(source,slot,true),1},
                label+" Depth",juce::NormalisableRange<float>(-1.0f,1.0f),0.5f));
        }
    }
    choice("filterMode","Filter Mode",{"Classic LP","Resonant LP","High-pass","Band-pass","24 dB LP","Analogue ZDF"},0);
    choice("sampleLoop","Sample Loop",{"Off","On"},0);
    choice("sampleReverse","Sample Reverse",{"Off","On"},0);
    choice("delaySync","Delay Timing",{"Free","1/16","1/8","Dotted 1/8","1/4"},0);
    choice("delayMode","Delay Mode",{"Stereo","Ping-pong"},0);
    choice("reverbSpace","Reverb Space",{"Room","Plate","Hall"},1);
    f("compAmount","Multiband Amount",0,1,0);
    f("pitchBend","Pitch Wheel",-24,24,0);
    return p;
}
float osc(double x,int wave){ x-=std::floor(x); switch(wave){case 1:return x<0.48?1.0f:-1.0f;case 2:return std::sin(float(juce::MathConstants<double>::twoPi*x));case 3:return float(4*std::abs(x-0.5)-1);default:return float(2*x-1);} }
}
juce::String SpaceInfinityProcessor::eqId(int band,const char* field){
    return "eq"+juce::String(band)+field;
}
juce::StringArray SpaceInfinityProcessor::oscillatorWaves(bool follow){
    juce::StringArray names{"Saw","Pulse","Sine","Triangle","Narrow Pulse","Wide Pulse","Rounded Saw","Folded Sine","Double Sine","Rectified","Vowel Ah","Vowel Oh","Vowel Ee","Reed","Hollow Reed","Brass","Soft Brass","Organ","Drawbar","Glass","Bell","Metal","Sync Rise","Sync Fall","Bent Ramp","Steps","Buzz","Octave Saw","Fifth Stack","Sub Harmonics","Custom Wavetable"};
    if(follow)names.insert(0,"Follow Osc 1");
    return names;
}
float SpaceInfinityProcessor::warpedSample(double position,int shape,int mode,float amount,float modulator){
    if(amount==0.0f||mode==0)return oscillatorSample(position,shape);
    if(mode==1)return oscillatorSample(position+modulator*amount*1.5,shape);
    const double phase=position-std::floor(position);
    if(mode==2){
        const double mirror=1.0-std::abs(phase*2.0-1.0);
        return oscillatorSample(phase+(mirror-phase)*amount,shape);
    }
    const double split=0.5+0.45*amount;
    const double mapped=phase<split?0.5*phase/split:0.5+0.5*(phase-split)/(1.0-split);
    return oscillatorSample(mapped,shape);
}
float SpaceInfinityProcessor::oscillatorSample(double phase,int wave){
    if(wave<4)return osc(phase,wave);
    static const auto tables=[]{
        std::array<std::array<float,512>,26> result{};
        constexpr float harmonics[][5]={
            {1,.52f,.11f,-.22f,-.25f},{1,-.52f,.11f,.22f,-.25f},{1,.32f,.12f,.04f,0},
            {1,0,-.62f,0,.2f},{.25f,1,.1f,.22f,0},{.1f,1,0,-.2f,.07f},
            {1,.18f,.74f,.55f,.17f},{1,.8f,.25f,.1f,.05f},{.45f,.2f,.23f,.7f,1},
            {1,0,.65f,0,.44f},{1,0,.24f,0,-.32f},{1,.64f,.48f,.32f,.22f},
            {1,.35f,.18f,.1f,.04f},{1,.5f,0,.22f,0},{1,.8f,.5f,.2f,0},
            {1,.05f,.4f,0,.45f},{1,.08f,-.35f,.08f,.65f},{.4f,.65f,-.6f,.4f,.56f},
            {1,-.7f,.45f,-.33f,.27f},{1,.72f,-.5f,.33f,-.25f},{1,-.35f,.14f,.14f,-.08f},
            {1,.3f,.16f,-.13f,-.22f},{1,.78f,.6f,.48f,.38f},{1,1,.33f,.5f,.2f},
            {1,.14f,.8f,.1f,.18f},{1,.24f,.08f,0,0}};
        for(size_t shape=0;shape<result.size();++shape){
            float peak=0.0f;
            for(size_t i=0;i<result[shape].size();++i){
                const double angle=juce::MathConstants<double>::twoPi*double(i)/512.0;
                for(size_t h=0;h<5;++h)
                    result[shape][i]+=harmonics[shape][h]*float(std::sin(angle*double(h+1)));
                peak=std::max(peak,std::abs(result[shape][i]));
            }
            for(auto& sample:result[shape])sample/=peak;
        }
        return result;
    }();
    const auto& table=tables[size_t(juce::jlimit(4,29,wave)-4)];
    const double index=(phase-std::floor(phase))*512.0;
    const auto left=size_t(index)&511;
    return table[left]+(table[(left+1)&511]-table[left])*float(index-std::floor(index));
}
SpaceInfinityProcessor::SpaceInfinityProcessor(const juce::File& presetDirectory):juce::AudioProcessor(BusesProperties().withInput("Sidechain",juce::AudioChannelSet::stereo(),false).withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"PARAMETERS",makeLayout()){
    for(size_t band=0;band<eqParameters.size();++band){
        const char* fields[]={"Frequency","Gain","Q"};
        for(size_t field=0;field<3;++field)
            eqParameters[band][field]=state.getRawParameterValue(eqId(int(band),fields[field]));
    }
    for(int step=0;step<16;++step){
        const auto suffix=juce::String(step);
        stepParameters[0][size_t(step)]=state.getRawParameterValue("stepPitch"+suffix);
        stepParameters[1][size_t(step)]=state.getRawParameterValue("stepVelocity"+suffix);
        stepParameters[2][size_t(step)]=state.getRawParameterValue("stepOn"+suffix);
    }
    state.getParameter("analogueUpgrade")->setValueNotifyingHost(1.0f);
    state.getParameter("ampTimed")->setValueNotifyingHost(1.0f);
    oscillatorSample(0.0,4); // Initialise lookup tables before audio processing.
    for(size_t i=0;i<macroValues.size();++i){
        const auto suffix=juce::String(int(i));
        macroValues[i]=state.getRawParameterValue("macro"+suffix);
        for(int slot=0;slot<3;++slot){
            macroRoutes[i][size_t(slot)]=state.getRawParameterValue(macroRouteId(int(i),slot,false));
            macroDepths[i][size_t(slot)]=state.getRawParameterValue(macroRouteId(int(i),slot,true));
        }
    }
    for(int source=4;source<8;++source)
        for(int slot=0;slot<3;++slot){
            macroRoutes[size_t(source)][size_t(slot)]=state.getRawParameterValue(modulationRouteId(source,slot,false));
            macroDepths[size_t(source)][size_t(slot)]=state.getRawParameterValue(modulationRouteId(source,slot,true));
        }
    for(auto& phase:displayedLfoPhase)phase.store(0.0f);
    for(int lfo=0;lfo<3;++lfo){
        for(int point=0;point<17;++point)
            designerPoints[size_t(lfo)][size_t(point)]=state.getRawParameterValue(lfoPointId(lfo,point));
        const char* fields[]={"Speed","Sync","Bars","Mode"};
        for(size_t i=0;i<4;++i)
            designerControls[size_t(lfo)][i]=state.getRawParameterValue(lfoParameterId(lfo,fields[i]));
    }
    decodeSample(juce::MemoryBlock{BinaryData::Orbital_Choir_Ah_Chord_F2_wav,
                  BinaryData::Orbital_Choir_Ah_Chord_F2_wavSize}, choirAh);
    decodeSample(juce::MemoryBlock{BinaryData::Ethereal_Choir_Ooh_C3_wav,
                  BinaryData::Ethereal_Choir_Ooh_C3_wavSize}, choirOoh);
    for(size_t i=0;i<extraSamples.size();++i){int bytes=0;const auto* data=BinaryData::getNamedResource(extraSampleSources[i].resource,bytes);
        if(data&&bytes>0)decodeSample(juce::MemoryBlock(data,size_t(bytes)),extraSamples[i]);}
    juce::StringArray categories;for(int i=0;i<presetNames().size();++i)categories.add(presetCategory(i));
    for(int i=0;i<7;++i){eqTypes[size_t(i)]=state.getRawParameterValue("eqMode"+juce::String(i));eqSlopes[size_t(i)]=state.getRawParameterValue("eqSlope"+juce::String(i));}
    for(int i=0;i<16;++i)gateParameters[size_t(i)]=state.getRawParameterValue("gateStep"+juce::String(i));
    library=std::make_unique<PresetLibrary>(presetNames(),categories,presetDirectory,96);
    startImpulseWorker();

}
juce::String SpaceInfinityProcessor::motionPointId(int index){return "motionPoint"+juce::String(index);}
juce::StringArray SpaceInfinityProcessor::macroNames(){return {"Energy","Width","Space","Shape"};}
juce::String SpaceInfinityProcessor::macroRouteId(int macro,int slot,bool depth){
    auto id=(depth?"macroDepth":"macroRoute")+juce::String(macro);
    if(slot>0)id+="_"+juce::String(slot);
    return id;
}
juce::String SpaceInfinityProcessor::modulationRouteId(int source,int slot,bool depth){
    if(source<4)return macroRouteId(source,slot,depth);
    return juce::String(depth?"modDepth":"modRoute")+juce::String(source)+"_"+juce::String(slot);
}
juce::String SpaceInfinityProcessor::lfoParameterId(int lfo,const char* field){
    if(lfo==0&&juce::String(field)=="Bars")return "motionBars";
    return "designerLfo"+juce::String(lfo+1)+field;
}
juce::String SpaceInfinityProcessor::lfoPointId(int lfo,int point){
    return lfo==0?motionPointId(point):"designerLfo"+juce::String(lfo+1)+"Point"+juce::String(point);
}
juce::StringArray SpaceInfinityProcessor::macroTargets(){
    return {"Off","Cutoff","Detune","Reverb","Attack","Drive","Delay","Chorus","Layer mix","Release","Resonance","Noise","Osc 1 Width","Osc 2 Width","Output","Decay","Sustain","Motion Depth","LFO 1 Speed","Vibrato Depth","Analogue","LFO 2 Speed","LFO 3 Speed"};
}
juce::StringArray SpaceInfinityProcessor::motionNames(){
    return {"Slow Rise","Rise and Fall","Rise into Gate","Late Bloom","Long Fall","Soft Pulse",
        "Double Swell","Triple Swell","Terraces","Descending Steps","Peak Hold","Dip and Return",
        "Quick Strike","Reverse Pull","Saw Rise","Saw Fall","Alternating Gates","Syncopated","Wave Train","Final Lift"};
}
void SpaceInfinityProcessor::loadMotionShape(int index){loadLfoShape(0,index);}
void SpaceInfinityProcessor::loadLfoShape(int lfo,int index){
    static const std::vector<float> curves[]={
        {0,.12f,.35f,.66f,1},{0,.18f,.80f,1,.58f,.20f,0},{0,.08f,.18f,.42f,.72f,1},
        {0,0,.04f,.14f,.44f,1},{1,.85f,.59f,.28f,0},{.25f,.42f,.70f,.84f,.70f,.42f,.25f},
        {0,.65f,.10f,1,.26f,0},{0,.58f,.04f,.76f,.12f,1,0},
        {0,.15f,.15f,.39f,.39f,.70f,.70f,1},{1,.80f,.80f,.54f,.54f,.28f,.28f,0},
        {0,.33f,1,1,1,.64f,.1f},{.92f,.63f,.22f,.04f,.30f,.67f,.92f},
        {0,1,.43f,.18f,.08f,0},{0,.10f,.20f,.48f,1,.16f,0},
        {0,.95f,.03f,.95f,.03f,1,0},{1,.04f,.92f,.04f,.92f,0},
        {1,1,0,0,.55f,.55f,0,0,1},{0,.85f,.85f,0,.3f,0,1,.2f,0},
        {.50f,.90f,.50f,.10f,.50f,.90f,.50f,.10f,.50f},{.08f,.12f,.10f,.18f,.36f,.68f,1}};
    if(lfo<0||lfo>=3||index<0||index>=int(std::size(curves)))return;
    const auto& curve=curves[index];
    for(int i=0;i<17;++i){
        const float x=float(i)*float(curve.size()-1)/16.0f;
        const auto left=std::min(size_t(x),curve.size()-2);
        const float value=curve[left]+(curve[left+1]-curve[left])*(x-float(left));
        state.getParameter(lfoPointId(lfo,i))->setValueNotifyingHost(value);
    }
}
juce::StringArray SpaceInfinityProcessor::sourceNames(){
    juce::StringArray names{"Synth", "Choir Ah", "Choir Ooh", "Imported WAV", "Granular WAV"};
    for(const auto& sample:extraSampleSources)names.add(sample.name);return names;
}
juce::Result SpaceInfinityProcessor::decodeSample(const juce::MemoryBlock& bytes, Sample& sample){
    juce::WavAudioFormat format;
    std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(
        new juce::MemoryInputStream(bytes, false), true));
    if(!reader || reader->numChannels < 1 || reader->numChannels > 2
       || reader->sampleRate < 8000 || reader->sampleRate > 192000
       || reader->lengthInSamples < 2 || reader->lengthInSamples > 30 * reader->sampleRate)
        return juce::Result::fail("Choose a mono or stereo WAV up to 30 seconds.");
    juce::AudioBuffer<float> decoded(2, static_cast<int>(reader->lengthInSamples));
    if(!reader->read(&decoded,0,decoded.getNumSamples(),0,true,true))
        return juce::Result::fail("Could not read the WAV.");
    sample.audio.setSize(1,decoded.getNumSamples());
    sample.audio.copyFrom(0,0,decoded,0,0,decoded.getNumSamples());
    if(reader->numChannels == 2){
        sample.audio.addFrom(0,0,decoded,1,0,decoded.getNumSamples());
        sample.audio.applyGain(0.5f);
    }
    sample.sampleRate = reader->sampleRate;
    sample.view.samples=sample.audio.getNumSamples();sample.view.rate=sample.sampleRate;
    const float onsetThreshold=juce::jmax(.000001f,sample.audio.getMagnitude(0,sample.audio.getNumSamples())*.01f);
    int onset=0;while(onset<sample.audio.getNumSamples()-1&&std::abs(sample.audio.getSample(0,onset))<onsetThreshold)++onset;
    sample.view.onset=float(onset)/juce::jmax(1,sample.audio.getNumSamples()-1);
    for(int bin=0;bin<512;++bin){
        const int begin=bin*sample.view.samples/512,end=juce::jmax(begin+1,(bin+1)*sample.view.samples/512);
        float low=0,high=0;for(int i=begin;i<juce::jmin(end,sample.view.samples);++i){const float v=sample.audio.getSample(0,i);low=juce::jmin(low,v);high=juce::jmax(high,v);}
        sample.view.low[size_t(bin)]=low;sample.view.high[size_t(bin)]=high;
    }
    return juce::Result::ok();
}
juce::Result SpaceInfinityProcessor::importSample(const juce::File& file){
    constexpr auto maxBytes = 16 * 1024 * 1024;
    juce::MemoryBlock bytes;
    if(file.getSize() > maxBytes || !file.loadFileAsData(bytes) || bytes.getSize() > maxBytes)
        return juce::Result::fail("Choose a readable WAV up to 16 MB.");
    Sample next;
    const auto result = decodeSample(bytes,next);
    if(result.failed())return result;
    {
        const juce::SpinLock::ScopedLockType lock(sampleLock);
        std::swap(importedSample,next);
        std::swap(importedBytes,bytes);
        ++sampleRevision;
    }
    state.state.setProperty("sampleName",file.getFileName(),nullptr);
    state.getParameter("sampleRoot")->setValueNotifyingHost(state.getParameter("sampleRoot")->convertTo0to1(60));
    state.getParameter("source")->setValueNotifyingHost(state.getParameter("source")->convertTo0to1(3.0f));
    return juce::Result::ok();
}
void SpaceInfinityProcessor::prepareToPlay(double sampleRate,int maxSamples){
    maximumHostBlock=std::max(1,maxSamples);
    oversampling.initProcessing(size_t(maximumHostBlock));oversampling.reset();
    highRateMidi.ensureSize(262144);
    sidechainHigh.setSize(1,maximumHostBlock*2);
    setLatencySamples(int(std::ceil(oversampling.getLatencyInSamples())));
    sr=sampleRate*2;maxSamples=maximumHostBlock*2;
    pattern=infinity::Pattern{};patternMidi.ensureSize(262144);
    transportKnown=false;transportPlaying=false;previousPpq=0;
    convolver.prepare({sr,(juce::uint32)std::max(1,maxSamples),2});
    convolutionDry.setSize(2,std::max(1,maxSamples));
    convolutionWet.reset(sr,.03);convolutionWet.setCurrentAndTargetValue(0);

    eqState={};
    previousOutputGain=param("gain");
    analogueAmount.reset(sr,0.04);
    analogueAmount.setCurrentAndTargetValue(param("analogueCharacter"));
    analogueCutoff.reset(sr,.02);
    analogueCutoff.setCurrentAndTargetValue(param("cutoff"));
    analogueResonance.reset(sr,.02);
    analogueResonance.setCurrentAndTargetValue(param("res"));
    juce::dsp::ProcessSpec spec{sr,(juce::uint32)std::max(maxSamples,1),2};chorus.prepare(spec);chorus.setRate(0.25f);chorus.setDepth(0.38f);chorus.setCentreDelay(9);chorus.setFeedback(0.1f);chorus.setMix(param("chorus"));reverb.setSampleRate(sr);
    flanger.prepare(spec);
    flanger.setRate(0.23f);
    flanger.setDepth(0.28f);
    flanger.setCentreDelay(1.2f);
    flanger.setFeedback(0.58f);
    phaser.prepare(spec);
    phaser.setRate(0.32f);
    phaser.setDepth(0.66f);
    phaser.setCentreFrequency(750.0f);
    phaser.setFeedback(0.42f);
    flangerActive=phaserActive=false;
    delayL.setMaximumDelayInSamples(int(sr*2)+8);delayR.setMaximumDelayInSamples(int(sr*2)+8);
    delayL.prepare({sr,(juce::uint32)std::max(maxSamples,1),1});delayR.prepare({sr,(juce::uint32)std::max(maxSamples,1),1});delayL.reset();delayR.reset();
    crossoverLow.prepare(spec); crossoverLow.setCutoffFrequency(180.0f);
    crossoverHigh.prepare(spec); crossoverHigh.setCutoffFrequency(2600.0f);
    alignLow.prepare(spec); alignLow.setCutoffFrequency(2600.0f);
    alignLow.setType(juce::dsp::LinkwitzRileyFilterType::allpass);
    for(auto& band:bandCompressors){band.prepare(spec);band.setAttack(9);band.setRelease(110);}
    delaySamples.reset(sr,0.07);
    delaySamples.setCurrentAndTargetValue(float(sr*.375));
    delayWet.reset(sr,0.025);delayWet.setCurrentAndTargetValue(param("delay"));
    studioCompressor.prepare(sr);studioDelay.prepare(sr);rhythm.prepare(sr);
    echoWet.reset(sr,.025);echoWet.setCurrentAndTargetValue(0);irLevel.reset(sr,.025);irLevel.setCurrentAndTargetValue(1);
    studioCompWet.reset(sr,.025);studioCompWet.setCurrentAndTargetValue(param("compMix"));
    bandMix.reset(sr,0.025);bandMix.setCurrentAndTargetValue(param("compAmount")>0.001f?1.0f:0.0f);
    delayToneL=delayToneR=0;
    delayBpm=120;
    midiBend=0;pitchBend.reset(sr,.006);pitchBend.setCurrentAndTargetValue(param("pitchBend"));
    for(size_t i=0;i<voices.size();++i){auto& v=voices[i];v={};v.calibration=VoiceCalibration::forVoice(int(i));
        v.drift.reset(0x83af71u+uint32_t(i)*7193u);
        const auto phase=[&]{return .5+.5*double(v.drift.random());};v.phase=phase();v.phase2=phase();
        for(auto& position:v.extraPhase1)position=phase();for(auto& position:v.extraPhase2)position=phase();
        v.idleRateA=v.idleRateB=220/sr;
    } lfoPhase=0; clock=0;
    heldNotes={};noteOrder=0;voiceMode=int(param("voiceMode"));
    designerPhase={};
    for(auto& phase:displayedLfoPhase)phase.store(0.0f);
}
void SpaceInfinityProcessor::noteOn(int note,float velocity,int source,int root,int channel){
    const Sample* sample=&sampleForSource(source);
    if(source != 0 && sample->audio.getNumSamples() < 2)return;
    auto* found=&voices[0];
    if(voiceMode==0)for(auto& v:voices)if(!v.active){found=&v;break;}
    if(voiceMode==0&&found->active)found=&*std::min_element(voices.begin(),voices.end(),[](const Voice& a,const Voice& b){return a.age<b.age;});
    const auto retainedPhase=found->phase,retainedPhase2=found->phase2;
    const auto retainedExtraA=found->extraPhase1,retainedExtraB=found->extraPhase2;
    const auto retainedDrift=found->drift;
    const auto calibration=found->calibration;
    *found={};found->calibration=calibration;
    const bool freeRunning=source==0&&param("analogueUpgrade")>.5f;
    if(freeRunning){found->phase=retainedPhase;found->phase2=retainedPhase2;
        found->extraPhase1=retainedExtraA;found->extraPhase2=retainedExtraB;found->drift=retainedDrift;}
    found->active=true;found->note=note;found->channel=channel;found->velocity=velocity;found->age=++clock;found->noiseState=uint32_t(found->age)*747796405u+2891336453u;
    if(found->noiseState==0)found->noiseState=1;
    if(!freeRunning)found->drift.reset(found->noiseState^0x9e3779b9u);
    if(!freeRunning&&source==0&&param("analogueCharacter")>0.0f){
        const auto phase=[found]{return 0.5+0.5*double(found->drift.random());};
        found->phase=phase();
        found->phase2=phase();
        for(auto& position:found->extraPhase1)position=phase();
        for(auto& position:found->extraPhase2)position=phase();
    }
    found->source=source;
    if(source != 0){
        if(userSample(source))found->sampleRevision=sampleRevision;
        const int sampleRoot=source==1?41:source==2?48:source>=5&&source<21?extraSampleSources[size_t(source-5)].root:root;
        found->sampleStep=sample->sampleRate/sr*std::exp2((note-sampleRoot)/12.0);
        const int last=sample->audio.getNumSamples()-1;
        found->sampleStart=juce::jlimit(0,last-1,int(param("sampleStart")*float(last)));
        found->sampleEnd=juce::jlimit(found->sampleStart+1,last,int(param("sampleEnd")*float(last)));
        found->sampleLoop=param("sampleLoop")>0.5f;
        if(param("sampleReverse")>0.5f){
            found->sampleStep=-found->sampleStep;
            found->samplePosition=found->sampleEnd;
        }else found->samplePosition=found->sampleStart;
    }
}
void SpaceInfinityProcessor::selectMonoNote(int source,int root,bool retrigger){
    const auto selected=std::max_element(heldNotes.begin(),heldNotes.end(),
        [](const HeldNote& a,const HeldNote& b){return a.order<b.order;});
    auto& voice=voices.front();
    if(selected->order==0){voice.released=true;return;}
    const auto key=int(selected-heldNotes.begin());
    const int note=key%128, channel=key/128+1;
    if(voice.active&&!voice.released&&voice.note==note&&voice.channel==channel&&!retrigger)return;
    if(voiceMode==2&&voice.active&&!voice.released&&voice.source==source){
        voice.sampleStep*=std::exp2((note-voice.note)/12.0);
        voice.note=note;
        voice.channel=channel;
        return;
    }
    noteOn(note,selected->velocity,source,root,channel);
}
void SpaceInfinityProcessor::noteOff(int note,int channel){for(auto& v:voices) if(v.active&&v.note==note&&v.channel==channel)v.released=true;}
void SpaceInfinityProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer& midi){
    if(b.getNumChannels()<2){b.clear();return;}
    // Process bounded chunks if a host supplies more samples than it announced.
    for(int offset=0;offset<b.getNumSamples();){
        const int count=std::min(maximumHostBlock,b.getNumSamples()-offset);
        sidechainHigh.clear();
        if(getBusCount(true)>0&&getBus(true,0)->isEnabled()){
            const auto input=getBusBuffer(b,true,0);
            for(int i=0;i<count;++i){float v=0;for(int ch=0;ch<input.getNumChannels();++ch)v+=input.getSample(ch,offset+i)/float(input.getNumChannels());
                sidechainHigh.setSample(0,i*2,v);sidechainHigh.setSample(0,i*2+1,v);}
        }
        auto output=juce::dsp::AudioBlock<float>(b).getSubBlock(size_t(offset),size_t(count));output.clear();
        auto up=oversampling.processSamplesUp(output);
        float* channels[]{up.getChannelPointer(0),up.getChannelPointer(1)};
        juce::AudioBuffer<float> high(channels,2,int(up.getNumSamples()));
        highRateMidi.clear();
        for(const auto event:midi)if(event.samplePosition>=offset&&event.samplePosition<offset+count)
            highRateMidi.addEvent(event.getMessage(),(event.samplePosition-offset)*2);
        renderBlock(high,highRateMidi);oversampling.processSamplesDown(output);
        unsigned write=scopeWrite.load(std::memory_order_relaxed);
        for(int i=0;i<count;++i){scopeSamples[size_t(write%32768)].store((b.getSample(0,offset+i)+b.getSample(1,offset+i))*.5f,std::memory_order_relaxed);++write;}
        scopeWrite.store(write,std::memory_order_release);offset+=count;
    }
}
void SpaceInfinityProcessor::renderBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer& midi){
    juce::ScopedNoDenormals noDenormals;
    b.clear(); if(b.getNumChannels()<2)return;
    transformPattern(midi,b.getNumSamples());
    auto* left=b.getWritePointer(0);auto* right=b.getWritePointer(1);
    std::array<float,23> modulation{};
    for(size_t i=0;i<macroValues.size();++i){
        const float value=macroValues[i]->load();
        for(size_t slot=0;slot<macroRoutes[i].size();++slot){
            const auto route=size_t(juce::jlimit(0,int(modulation.size()-1),int(macroRoutes[i][slot]->load())));
            modulation[route]+=value*macroDepths[i][slot]->load();
        }
    }

    if(auto* playHead=getPlayHead())
        if(auto position=playHead->getPosition())
            if(auto bpm=position->getBpm())
                if(*bpm>0.0)delayBpm=*bpm;
    std::array<std::array<float,17>,3> curves{};
    std::array<double,3> lfoSteps{};
    std::array<int,3> lfoModes{};
    for(size_t lfo=0;lfo<3;++lfo){
        for(size_t point=0;point<17;++point)curves[lfo][point]=designerPoints[lfo][point]->load();
        const auto& control=designerControls[lfo];
        static constexpr double lengths[]{1,2,4,8,.5,.25,.125,.0625};
        const int division=juce::jlimit(0,7,int(control[2]->load()));
        lfoSteps[lfo]=control[1]->load()>.5f
            ?delayBpm/(240.0*sr*lengths[division]):control[0]->load()/sr;
        lfoModes[lfo]=int(control[3]->load());
    }
    const auto curveAt=[&curves](size_t lfo,double phase){
        const double x=std::clamp(phase,0.0,1.0)*16.0;
        const size_t point=std::min(size_t(x),size_t(15));
        return curves[lfo][point]+(curves[lfo][point+1]-curves[lfo][point])*float(x-double(point));
    };
    struct DynamicRoute {size_t target=0;float depth=0.0f;};
    std::array<std::array<DynamicRoute,3>,4> timeRoutes{};
    for(size_t source=0;source<4;++source)
        for(size_t slot=0;slot<3;++slot)
            timeRoutes[source][slot]={size_t(juce::jlimit(0,22,int(macroRoutes[source+4][slot]->load()))),
                macroDepths[source+4][slot]->load()};
    const auto routeDynamic=[&timeRoutes](auto& values,size_t source,float value){
        for(const auto& route:timeRoutes[source])
            if(route.target>0)values[route.target]+=route.depth*value;
    };
    auto fxModulation=modulation;
    for(size_t lfo=0;lfo<3;++lfo)routeDynamic(fxModulation,lfo,curveAt(lfo,designerPhase[lfo]));
    float currentEnvelope=0.0f;
    for(const auto& voice:voices)if(voice.active)currentEnvelope=std::max(currentEnvelope,voice.env);
    routeDynamic(fxModulation,3,currentEnvelope);
    const int speedTargets[]{18,21,22};
    for(size_t lfo=0;lfo<3;++lfo)
        lfoSteps[lfo]*=std::exp2(fxModulation[size_t(speedTargets[lfo])]*2.0f);

    const float attack=juce::jlimit(0.0f,1.0f,param("attack")*std::exp2(modulation[4]*3));
    const float release=juce::jlimit(0.0f,1.0f,param("release")*std::exp2(modulation[9]*2));
    const float mix=juce::jlimit(0.0f,1.0f,param("mix")+0.4f*modulation[8]);
    const float detune=juce::jlimit(0.0f,0.04f,param("detune")+0.02f*modulation[2]);
    const float cutoff=param("cutoff")*std::exp2(modulation[1]*2);
    const float res=juce::jlimit(0.0f,0.95f,param("res")+0.4f*modulation[10]);
    const float lfoRate=juce::jlimit(0.1f,12.0f,param("lforate")*std::exp2(modulation[18]*2));
    const float lfoDepth=juce::jlimit(0.0f,1.0f,param("lfodepth")+0.5f*modulation[19]);
    const float drive=juce::jlimit(0.0f,1.0f,param("drive")+0.45f*modulation[5]);
    const bool timedEnvelope=param("ampTimed")>.5f;
    const bool upgraded=param("analogueUpgrade")>.5f;
    const float sustain=juce::jlimit(0.0f,1.0f,param("ampSustain")+0.5f*modulation[16]);
    const float attackStep=float(1.0/std::max(1.0,double(attack)*sr));
    const float legacyAttackStep=attack==0.0f?1.0f:float(1.0-std::exp(-double(attackStep)));
    const float decayStep=float(1.0/std::max(1.0,juce::jlimit(0.005,8.0,double(param("ampDecay"))*std::exp2(double(modulation[15])*2))*sr));
    const float releaseFactor=release==0.0f?0.0f:float(std::exp(-1.0/(double(release)*sr)));
    const bool osc1Enabled=param("osc1Enabled")>.5f,osc2Enabled=param("osc2Enabled")>.5f;
    const int wave=int(param("wave"));
    const int waveChoice=int(param("osc2Wave"));
    const int secondWave=waveChoice==0?wave:waveChoice-1;
    const int warp1=int(param("osc1Warp")),warp2=int(param("osc2Warp"));
    const float warpAmount1=param("osc1WarpAmount"),warpAmount2=param("osc2WarpAmount");
    const double tune1=std::exp2(param("osc1Octave")+param("osc1Tune")/1200.0);
    const double tune2=std::exp2(param("osc2Octave")+param("osc2Tune")/1200.0);
    const int unison1=int(param("osc1Unison")),unison2=int(param("osc2Unison"));
    std::array<double,16> unisonRates1{},unisonRates2{};
    const auto prepareStack=[detune](auto& rates,int count){
        for(int i=0;i<count;++i){
            const double spread=count==1?0.0:2.0*i/(count-1)-1.0;
            rates[size_t(i)]=std::exp2(spread*detune*0.75);
        }
    };
    prepareStack(unisonRates1,unison1);
    prepareStack(unisonRates2,unison2);
    const float noiseLevel=juce::jlimit(0.0f,1.0f,param("noiseLevel")+0.5f*modulation[11]);
    const int noiseType=int(param("noiseType"));
    const float width1=juce::jlimit(0.0f,1.0f,param("osc1Width")+0.5f*modulation[12]);
    const float width2=juce::jlimit(0.0f,1.0f,param("osc2Width")+0.5f*modulation[13]);
    const float filterA=param("filterAttack"),filterD=param("filterDecay"),filterS=param("filterSustain"),filterR=param("filterRelease");
    const float filterAmount=param("filterEnvAmount"),tableScan=param("wavetableScan");
    const float grainPosition=param("grainPosition"),grainSize=param("grainSize"),grainDensity=param("grainDensity"),grainSpray=param("grainSpray"),grainSpread=param("grainSpread");
    const bool grainReverse=param("sampleReverse")>.5f;
    const int filterMode=int(param("filterMode"));
    const int source=int(param("source")), root=int(param("sampleRoot"));
    analogueAmount.setTargetValue(juce::jlimit(0.0f,1.0f,param("analogueCharacter")+0.5f*fxModulation[20]));
    analogueCutoff.setTargetValue(juce::jlimit(50.0f,float(sr*.4),cutoff));
    analogueResonance.setTargetValue(res);
    const float dcCoefficient=float(std::exp(-juce::MathConstants<double>::twoPi*10.0/sr));
    const float driftSlew=float(1.0-std::exp(-1.0/(sr*0.18)));
    const int driftInterval=juce::jmax(1,int(sr*0.043));
    const float hostBend=param("pitchBend");
    pitchBend.setTargetValue(juce::jlimit(-24.0f,24.0f,hostBend+midiBend));
    const juce::SpinLock::ScopedTryLockType sampleGuard(sampleLock);
    for(auto& voice:voices)if(voice.active&&voice.source!=0&&(!userSample(voice.source)||sampleGuard.isLocked())){
        const auto& sample=sampleForSource(voice.source);const int last=sample.audio.getNumSamples()-1;if(last<2)continue;
        voice.sampleStart=juce::jlimit(0,last-1,int(param("sampleStart")*last));voice.sampleEnd=juce::jlimit(voice.sampleStart+1,last,int(param("sampleEnd")*last));voice.sampleLoop=param("sampleLoop")>.5f;
        const int root=voice.source==1?41:voice.source==2?48:voice.source>=5?extraSampleSources[size_t(voice.source-5)].root:int(param("sampleRoot"));
        voice.sampleStep=sample.sampleRate/sr*std::exp2((voice.note-root)/12.0)*(param("sampleReverse")>.5f?-1:1);
        voice.samplePosition=juce::jlimit(double(voice.sampleStart),double(voice.sampleEnd),voice.samplePosition);
    }

    const float motionDepth=juce::jlimit(0.0f,1.0f,param("motionDepth")+0.5f*modulation[17]);
    const int requestedMode=int(param("voiceMode"));
    if(voiceMode!=requestedMode){
        for(auto& voice:voices){voice.active=false;voice.released=false;voice.env=0;}
        heldNotes={};
        voiceMode=requestedMode;
    }
    const double masterOctave=param("masterOctave");
    auto msg=patternMidi.begin();
    for(int s=0;s<b.getNumSamples();++s){
        while(msg!=patternMidi.end()&&(*msg).samplePosition<=s){
            const auto m=(*msg).getMessage();
            ++msg;
            const int channel=m.getChannel();
            if(m.isPitchWheel()){
                const int wheel=m.getPitchWheelValue()-8192;
                midiBend=24.0f*float(wheel)/(wheel<0?8192.0f:8191.0f);
                pitchBend.setTargetValue(juce::jlimit(-24.0f,24.0f,hostBend+midiBend));
            }else if(m.isNoteOn()){
                if(userSample(source)&&!sampleGuard.isLocked())continue;
                for(size_t lfo=0;lfo<3;++lfo)if(lfoModes[lfo]!=0)designerPhase[lfo]=0.0;
                if(voiceMode==0)noteOn(m.getNoteNumber(),m.getFloatVelocity(),source,root,channel);
                else{
                    heldNotes[size_t((channel-1)*128+m.getNoteNumber())]={++noteOrder,m.getFloatVelocity()};
                    selectMonoNote(source,root,true);
                }
            }else if(m.isNoteOff()){
                if(voiceMode==0)noteOff(m.getNoteNumber(),channel);
                else{
                    heldNotes[size_t((channel-1)*128+m.getNoteNumber())]={};
                    if(!userSample(source)||sampleGuard.isLocked())selectMonoNote(source,root,false);
                    else if(voices.front().note==m.getNoteNumber()&&voices.front().channel==channel)
                        voices.front().released=true;
                }
            }else if(m.isAllNotesOff()||m.isAllSoundOff()){
                for(auto& voice:voices)if(voice.channel==channel){
                    if(m.isAllSoundOff())voice.active=false;
                    else voice.released=true;
                }
                if(voiceMode!=0){
                    std::fill_n(heldNotes.begin()+(channel-1)*128,128,HeldNote{});
                    if(!userSample(source)||sampleGuard.isLocked())selectMonoNote(source,root,false);
                }
            }
        }
        lfoPhase+=lfoRate/sr;if(lfoPhase>=1)lfoPhase-=1;
        std::array<float,3> lfoValues{};
        std::array<float,23> movingModulation{};
        for(size_t lfo=0;lfo<3;++lfo){
            lfoValues[lfo]=curveAt(lfo,designerPhase[lfo]);
            designerPhase[lfo]+=lfoSteps[lfo];
            if(lfoModes[lfo]==2)designerPhase[lfo]=std::min(1.0,designerPhase[lfo]);
            else designerPhase[lfo]-=std::floor(designerPhase[lfo]);
            displayedLfoPhase[lfo].store(float(designerPhase[lfo]),std::memory_order_relaxed);
        }
        const float movement=lfoValues[0];
        float out=0, side=0;const float lfo=std::sin(float(juce::MathConstants<double>::twoPi*lfoPhase));
        const float character=analogueAmount.getNextValue();
        const float circuitDrive=1.0f+character*3.0f;
        const float warmthDrive=1.0f+character*1.8f;
        const float warmthBias=character*0.07f;
        const float warmthCentre=std::tanh(warmthBias);
        const float warmthScale=1.0f/std::tanh(warmthDrive);
        const double bendRatio=std::exp2(masterOctave+pitchBend.getNextValue()/12.0);
        const float circuitCutoff=analogueCutoff.getNextValue();
        const float circuitResonance=analogueResonance.getNextValue();
        const float fc=juce::jlimit(50.0f,float(sr*0.4),(filterMode==5?circuitCutoff:cutoff)*std::exp2(lfo*lfoDepth*2+movement*motionDepth*3));
        for(auto& v:voices){if(!v.active){
            if(upgraded){const auto advance=[](double& phase,double rate){phase+=rate;phase-=std::floor(phase);};
                advance(v.phase,v.idleRateA);advance(v.phase2,v.idleRateB);
                for(auto& phase:v.extraPhase1)advance(phase,v.idleRateA);
                for(auto& phase:v.extraPhase2)advance(phase,v.idleRateB);}
            continue;
        }
            if(userSample(v.source) && !sampleGuard.isLocked())continue;
            if(userSample(v.source) && v.sampleRevision!=sampleRevision){v.active=false;continue;}
            movingModulation={};
            for(size_t lfo=0;lfo<3;++lfo){
                float value=lfoValues[lfo];
                if(lfoModes[lfo]!=0){
                    value=curveAt(lfo,v.lfoPhase[lfo]);
                    v.lfoPhase[lfo]+=lfoSteps[lfo];
                    if(lfoModes[lfo]==2)v.lfoPhase[lfo]=std::min(1.0,v.lfoPhase[lfo]);
                    else v.lfoPhase[lfo]-=std::floor(v.lfoPhase[lfo]);
                }
                routeDynamic(movingModulation,lfo,value);
            }
            routeDynamic(movingModulation,3,v.env);
            const float voiceAttack=juce::jlimit(0.0f,1.0f,attack*std::exp2(movingModulation[4]*3));
            const float voiceRelease=juce::jlimit(0.0f,1.0f,release*std::exp2(movingModulation[9]*2));
            const float voiceAttackStep=movingModulation[4]==0.0f?attackStep:float(1.0/std::max(1.0,double(voiceAttack)*sr));
            const float voiceLegacyAttack=movingModulation[4]==0.0f?legacyAttackStep:
                voiceAttack==0.0f?1.0f:float(1.0-std::exp(-double(voiceAttackStep)));
            const float voiceReleaseFactor=movingModulation[9]==0.0f?releaseFactor:
                voiceRelease==0.0f?0.0f:float(std::exp(-1.0/(voiceRelease*sr)));
            const float voiceSustain=juce::jlimit(0.0f,1.0f,sustain+.5f*movingModulation[16]);
            const float voiceDecayStep=decayStep/std::exp2(movingModulation[15]*2);
            const float timing=upgraded?std::exp2(v.calibration.timing*character*.10f):1;
            const float filterEnv=upgraded?v.curvedFilter.next(v.released,sr,juce::jmin(1.0f,filterA*timing),filterD*timing,filterS,juce::jmin(1.0f,filterR*timing)):
                v.filterEnvelope.next(v.released,sr,filterA,filterD,filterS,filterR);
            const float voiceFc=juce::jlimit(50.0f,float(sr*.4),fc*std::exp2(movingModulation[1]*2+filterEnv*filterAmount+(upgraded?v.calibration.cutoff*character*.09f:0)));
            const float voiceG=std::tan(float(juce::MathConstants<double>::pi*voiceFc/sr));
            const float voiceOnePole=1.0f-std::exp(float(-juce::MathConstants<double>::twoPi*voiceFc/sr));
            const float voiceRes=juce::jlimit(0.0f,.95f,res+.4f*movingModulation[10]);
            const float voiceDetune=juce::jlimit(0.0f,.04f,detune+.02f*movingModulation[2]);
            const float voiceMix=juce::jlimit(0.0f,1.0f,mix+.4f*movingModulation[8]);
            const float voiceGain1=osc1Enabled?1.0f-voiceMix:0.0f;
            const float voiceGain2=osc2Enabled?voiceMix:0.0f;
            const float voiceWidth1=juce::jlimit(0.0f,1.0f,width1+.5f*movingModulation[12]);
            const float voiceWidth2=juce::jlimit(0.0f,1.0f,width2+.5f*movingModulation[13]);

            if(timedEnvelope&&upgraded){
                v.env=v.curvedAmp.next(v.released,sr,juce::jmin(1.0f,voiceAttack*timing),juce::jlimit(.005f,8.0f,float(1.0/(voiceDecayStep*sr))*timing),voiceSustain,juce::jmin(1.0f,voiceRelease*timing));
            }else if(timedEnvelope){
                advanceTimedEnvelope(v.env,v.decaying,v.releaseStart,v.released,voiceAttackStep,
                    voiceDecayStep,voiceSustain,float(voiceRelease*sr));
            }else if(v.released)v.env*=voiceReleaseFactor;
            else if(voiceSustain>=1.0f&&!v.decaying)v.env+=(1-v.env)*voiceLegacyAttack;
            else if(!v.decaying){
                v.env=std::min(1.0f,v.env+voiceAttackStep);
                v.decaying=v.env>=1.0f;
            }else v.env+=juce::jlimit(-voiceDecayStep,voiceDecayStep,voiceSustain-v.env);
            if(v.released&&v.env<0.0001f){v.active=false;continue;}
            float raw=0, rawSide=0;
            if(v.source==4){
                const auto grains=v.granular.next(importedSample.audio.getReadPointer(0),v.sampleStart,v.sampleEnd,
                    std::abs(v.sampleStep)*bendRatio,sr,grainPosition,grainSize,grainDensity,grainSpray,grainSpread,grainReverse);
                raw=(grains[0]+grains[1])*.5f;rawSide=(grains[0]-grains[1])*.5f;
            }else if(v.source!=0){
                const auto& sample=sampleForSource(v.source);
                if(v.samplePosition<v.sampleStart||v.samplePosition>v.sampleEnd){
                    if(!v.sampleLoop||v.released){v.active=false;continue;}
                    const double length=double(v.sampleEnd-v.sampleStart);
                    const double offset=std::fmod(v.samplePosition-v.sampleStart,length);
                    v.samplePosition=v.sampleStart+(offset<0?offset+length:offset);
                }
                const auto index=std::min(static_cast<int>(v.samplePosition),v.sampleEnd-1);
                const auto* data=sample.audio.getReadPointer(0);
                raw=data[index]+static_cast<float>(v.samplePosition-index)*(data[index+1]-data[index]);
                v.samplePosition+=v.sampleStep*bendRatio;
            }else{
            const auto drift=character>0.0f?v.drift.next(driftSlew,driftInterval):std::array<float,2>{};
            const double driftA=std::exp2(double(character)*(double(drift[0])*2.5+(upgraded?v.calibration.tuneA*2:0))/1200);
            const double driftB=std::exp2(double(character)*(double(drift[1])*2.5+(upgraded?v.calibration.tuneB*2:0))/1200);
            const double freq=440.0*bendRatio*std::pow(2.0,(v.note-69.0)/12.0);
            const auto readWave=[&](double phase,int shape){return shape==30?(sampleGuard.isLocked()?wavetable.sample(phase,tableScan):0.0f):oscillatorSample(phase,shape);};
            const auto readWarp=[&](double phase,int shape,int mode,float amount,float mod){
                if(shape!=30)return warpedSample(phase,shape,mode,amount,mod);
                if(mode==1)phase+=mod*amount*1.5;
                else if(mode==2){const double t=phase-std::floor(phase);phase=t+((1-std::abs(t*2-1))-t)*amount;}
                else if(mode==3){const double t=phase-std::floor(phase),split=.5+.45*amount;phase=t<split?.5*t/split:.5+.5*(t-split)/(1-split);}
                return readWave(phase,shape);
            };
            v.idleRateA=freq*tune1*driftA/sr;v.idleRateB=freq*tune2*driftB/sr;
            const float sourceA=warp2==1?readWave(v.phase,wave):0.0f;
            const float sourceB=warp1==1?readWave(v.phase2,secondWave):0.0f;
            const auto stack=[&](double& phase,auto& extras,double rate,const auto& rates,
                                  int count,int shape,float width,float gain,double offset,int warp,float amount,float modulator){
                if(gain==0.0f){
                    phase+=rate*rates[0];
                    phase-=std::floor(phase);
                    return;
                }
                const float level=gain/float(count);
                const float spreadDelta=voiceDetune-detune;
                for(int i=0;i<count;++i){
                    auto& position=i==0?phase:extras[size_t(i-1)];
                    const double spread=count==1?0.0:2.0*double(i)/double(count-1)-1.0;
                    const double increment=rate*rates[size_t(i)]*(1.0+0.5198603854*spreadDelta*spread);
                    position+=increment;
                    position-=std::floor(position);
                    float sample=readWarp(position,shape,warp,amount,modulator);
                    if(warp==0&&character>0.0f){
                        if(shape==0)sample-=character*analoguePolyBlep(position,increment);
                        if(shape==1)sample+=character*(analoguePolyBlep(position,increment)-analoguePolyBlep(position-.48,increment));
                    }
                    raw+=sample*level;
                    if(width>0.0f){
                        if(count==1)rawSide+=width*level*(sample-readWarp(position+offset,shape,warp,amount,modulator))*0.5f;
                        else rawSide+=sample*level*width*(2.0f*float(i)/float(count-1)-1.0f);
                    }
                }
            };
            stack(v.phase,v.extraPhase1,freq*tune1*driftA/sr,unisonRates1,unison1,wave,voiceWidth1,voiceGain1,0.19,warp1,warpAmount1,sourceB);
            stack(v.phase2,v.extraPhase2,freq*tune2*driftB*(1.0+voiceDetune+0.003*lfo*lfoDepth)/sr,
                  unisonRates2,unison2,secondWave,voiceWidth2,voiceGain2,0.31,warp2,warpAmount2,sourceA);
            }
            const float voiceNoise=juce::jlimit(0.0f,1.0f,noiseLevel+.5f*movingModulation[11]);
            if(voiceNoise>0.0f){
                auto& rng=v.noiseState;
                rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;
                const float white=float(rng>>8)*(2.0f/16777215.0f)-1.0f;
                v.noiseMemory+=0.10f*(white-v.noiseMemory);
                const float noise=noiseType==1?v.noiseMemory*2.5f:noiseType==2?white-v.noiseMemory:white;
                raw+=noise*voiceNoise*0.5f;
            }
            if(character>0.0f){
                const float warmed=(std::tanh(raw*warmthDrive+warmthBias)-warmthCentre)*warmthScale;
                raw+=character*(warmed-raw);
                rawSide+=character*(std::tanh(rawSide*warmthDrive)*warmthScale-rawSide);
            }
            float filtered;
            float filteredSide=0.0f;
            if(filterMode==5){
                const float coefficient=voiceG/(1.0f+voiceG);
                const float preDrive=upgraded?1+character*(.6f+drive*1.5f):1;
                const float left=v.ladder[0].process((raw+rawSide)*preDrive,coefficient,juce::jlimit(0.0f,.95f,circuitResonance+.4f*movingModulation[10]),circuitDrive);
                const float right=v.ladder[1].process((raw-rawSide)*preDrive,coefficient,juce::jlimit(0.0f,.95f,circuitResonance+.4f*movingModulation[10]),circuitDrive);
                filtered=0.5f*(left+right);
                filteredSide=0.5f*(left-right);
            }else if(filterMode==0){
                v.cutoff+=voiceOnePole*(raw*(1+voiceRes*0.7f)-v.cutoff);
                filtered=v.cutoff;
            }else{
                const auto stage=[g=voiceG](float input,float k,float* memory){
                    const float band=(memory[0]+g*(input-memory[1]))/(1.0f+g*(g+k));
                    const float low=memory[1]+g*band;
                    memory[0]=2.0f*band-memory[0];
                    memory[1]=2.0f*low-memory[1];
                    return std::array<float,3>{low,input-k*band-low,band};
                };
                const auto response=stage(raw,1.4142f-voiceRes*1.34f,v.filterState);
                filtered=filterMode==4 ? stage(response[0],1.4142f,v.filterCascade)[0]
                                       : response[size_t(filterMode-1)];
            }
            if(filterMode!=5){
                v.stereoSide+=voiceOnePole*(rawSide-v.stereoSide);
                filteredSide=v.stereoSide;
            }
            if(character>0.0f){
                float channels[]{filtered+filteredSide,filtered-filteredSide};
                for(size_t channel=0;channel<2;++channel){
                    const float input=channels[channel];
                    channels[channel]=input-v.dcInput[channel]+dcCoefficient*v.dcOutput[channel];
                    v.dcInput[channel]=input;
                    v.dcOutput[channel]=channels[channel];
                }
                filtered=0.5f*(channels[0]+channels[1]);
                filteredSide=0.5f*(channels[0]-channels[1]);
            }
            if(movingModulation[5]!=0.0f){
                const float ratio=(1.0f+9.0f*juce::jlimit(0.0f,1.0f,drive+.45f*movingModulation[5]))/(1.0f+9.0f*drive);
                filtered=std::tanh(filtered*ratio);
                filteredSide=std::tanh(filteredSide*ratio);
            }
            const float level=v.velocity*v.env*0.14f*(upgraded?1+v.calibration.gain*character*.025f:1);
            out+=filtered*level;
            side+=filteredSide*level;
        }
        const float driveGain=1+drive*9;
        left[s]=std::tanh((out+side)*driveGain);
        right[s]=std::tanh((out-side)*driveGain);
    }
    const auto oldTempo=displayedBpm.exchange(delayBpm,std::memory_order_relaxed);
    if(std::abs(oldTempo-delayBpm)>.001&&param("irSync")>0)impulseRequest.fetch_add(1,std::memory_order_relaxed);
    studio::Rhythm::Settings rhythmSettings;
    rhythmSettings.gate=param("gateEnabled")>.5f;rhythmSettings.pump=param("pumpEnabled")>.5f;
    rhythmSettings.split=int(param("gateSplit"));rhythmSettings.rate=int(param("gateRate"));rhythmSettings.pumpRate=int(param("pumpRate"));rhythmSettings.sidechain=int(param("sidechainSource"));
    rhythmSettings.depth=param("gateDepth");rhythmSettings.pumpDepth=param("pumpDepth");rhythmSettings.attack=param("gateAttack");rhythmSettings.release=param("gateRelease");
    rhythmSettings.pumpAttack=param("pumpAttack");rhythmSettings.pumpRelease=param("pumpRelease");rhythmSettings.threshold=param("sidechainThreshold");rhythmSettings.ratio=param("sidechainRatio");rhythmSettings.low=param("gateLow");rhythmSettings.high=param("gateHigh");
    for(int i=0;i<16;++i)rhythmSettings.steps[size_t(i)]=gateParameters[size_t(i)]->load();
    rhythm.configure(rhythmSettings,delayBpm);
    if(auto* head=getPlayHead())if(auto info=head->getPosition())if(info->getIsPlaying())if(auto ppq=info->getPpqPosition())rhythm.setBeat(*ppq);
    if(rhythmSettings.gate||rhythmSettings.pump)for(int i=0;i<b.getNumSamples();++i){const auto result=rhythm.process(left[i],right[i],sidechainHigh.getSample(0,juce::jmin(i,sidechainHigh.getNumSamples()-1)));
        if(rhythmSettings.gate||rhythmSettings.pump){left[i]=result[0];right[i]=result[1];}}
    else rhythm.advance(b.getNumSamples());
    displayedGateStep.store(rhythm.displayStep,std::memory_order_relaxed);
    const int compressorMode=int(param("compMode"));
    if(compressorMode==0){
    // Compress the dry synth in three bands before the time-based effects.
    const auto amount=param("compEnabled")>0.5f?param("compAmount"):0.0f;
    bandMix.setTargetValue(amount>0.001f?1.0f:0.0f);
    for(size_t i=0;i<bandCompressors.size();++i){
        bandCompressors[i].setThreshold(param("compThreshold")-30.0f*amount-3.0f*float(i));
        bandCompressors[i].setAttack(param("compAttack"));
        bandCompressors[i].setRelease(param("compRelease"));
        bandCompressors[i].setRatio(1.0f+(param("compRatio")-1.0f)*amount);
    }
    for(int s=0;s<b.getNumSamples();++s){
        const auto mix=bandMix.getNextValue();
        for(int ch=0;ch<2;++ch){
            auto* output=ch==0?left:right;
            const auto dry=output[s];
            float low, upper, mid, high;
            crossoverLow.processSample(ch,dry,low,upper);
            crossoverHigh.processSample(ch,upper,mid,high);
            low=alignLow.processSample(ch,low);
            const auto compressed=bandCompressors[0].processSample(ch,low)
                +bandCompressors[1].processSample(ch,mid)+bandCompressors[2].processSample(ch,high);
            output[s]=dry+mix*(compressed-dry);
        }
    }
    }else{
        studio::Compressor::Settings c;c.mode=compressorMode;c.threshold=param("compThreshold");c.ratio=param("compRatio");c.attack=param("compAttack");c.release=param("compRelease");
        c.low=param("compXLow");c.high=param("compXHigh");c.below=param("compBelow");c.gain=param("compGain");c.output=param("compOutput");c.limit=param("compLimit")>.5f;
        c.bandGain={param("compLowGain"),param("compMidGain"),param("compHighGain")};c.offset={param("compLowOffset"),param("compMidOffset"),param("compHighOffset")};
        studioCompressor.configure(c,b.getNumSamples());bandMix.setTargetValue(param("compEnabled")>.5f&&param("compMix")>.0001f?1.0f:0.0f);studioCompWet.setTargetValue(param("compMix"));
        for(int i=0;i<b.getNumSamples();++i){const auto wet=studioCompressor.process(left[i],right[i]);const float enabled=bandMix.getNextValue(),mix=studioCompWet.getNextValue();
            left[i]+=enabled*(studioCompressor.dry[0]+mix*(wet[0]-studioCompressor.dry[0])-left[i]);
            right[i]+=enabled*(studioCompressor.dry[1]+mix*(wet[1]-studioCompressor.dry[1])-right[i]);}
        for(size_t i=0;i<3;++i){compLevels[i].store(studioCompressor.input[i],std::memory_order_relaxed);compReduction[i].store(studioCompressor.reduction[i],std::memory_order_relaxed);}
    }
    chorus.setRate(param("chorusRate"));
    chorus.setDepth(param("chorusDepth"));
    chorus.setCentreDelay(param("chorusDelay"));
    chorus.setFeedback(param("chorusFeedback"));
    chorus.setMix(juce::jlimit(0.0f,1.0f,param("chorus")+0.4f*fxModulation[7]));
    juce::dsp::AudioBlock<float> block(b);
    juce::dsp::ProcessContextReplacing<float> context(block);
    if(param("chorusEnabled")>0.5f)chorus.process(context);
    const float flangeMix=param("flangerEnabled")>0.5f?param("flanger"):0.0f;
    flanger.setRate(param("flangerRate"));
    flanger.setDepth(param("flangerDepth"));
    flanger.setCentreDelay(param("flangerDelay"));
    flanger.setFeedback(param("flangerFeedback"));
    if(flangeMix>0.0f){
        if(!flangerActive)flanger.reset();
        flanger.setMix(flangeMix);
        flanger.process(context);
    }
    flangerActive=flangeMix>0.0f;
    const float phaseMix=param("phaserEnabled")>0.5f?param("phaser"):0.0f;
    phaser.setRate(param("phaserRate"));
    phaser.setDepth(param("phaserDepth"));
    phaser.setCentreFrequency(juce::jmin(float(sr*0.45),param("phaserFrequency")));
    phaser.setFeedback(param("phaserFeedback"));
    if(phaseMix>0.0f){
        if(!phaserActive)phaser.reset();
        phaser.setMix(phaseMix);
        phaser.process(context);
    }
    phaserActive=phaseMix>0.0f;


    if(int(param("echoMode"))==0){
    static constexpr double divisions[]{0,.25,.5,.75,1};
    const int division=int(param("delaySync"));
    const bool ping=param("delayMode")>=1;
    const double seconds=division==0?param("delayTime"):60.0/delayBpm*divisions[division];
    delaySamples.setTargetValue(juce::jlimit(1.0f,float(delayL.getMaximumDelayInSamples()-1),float(seconds*sr)));
    delayWet.setTargetValue(param("delayEnabled")>0.5f?juce::jlimit(0.0f,0.8f,param("delay")+0.4f*fxModulation[6]):0.0f);
    const float tone=float(1.0-std::exp(-juce::MathConstants<double>::twoPi*juce::jmin(double(sr*0.45),double(param("delayTone")))/sr));
    const float feedback=param("delayFeedback");
    for(int s=0;s<b.getNumSamples();++s){
        const auto time=delaySamples.getNextValue(), wet=delayWet.getNextValue();
        const auto dl=delayL.popSample(0,time),dr=delayR.popSample(0,time*(ping?1.0f:.75f));
        delayToneL+=tone*(dl-delayToneL);delayToneR+=tone*(dr-delayToneR);
        delayL.pushSample(0,left[s]+(ping?delayToneR:delayToneL)*feedback);
        delayR.pushSample(0,(ping?0.0f:right[s])+ (ping?delayToneL:delayToneR)*feedback);
        left[s]+=dl*wet;right[s]+=dr*wet;
    }
    }else{
        static constexpr double beats[]{0,.0625,.125,.25,.5,1,2,4};
        const bool linked=param("echoLink")>.5f;const int syncL=int(param("echoSyncL")),syncR=linked?syncL:int(param("echoSyncR"));
        studio::Delay::Settings e;e.mode=int(param("echoMode"));e.hq=param("echoHQ")>.5f;e.feedback=param("delayFeedback");e.frequency=param("echoFrequency");e.bandwidth=param("echoBandwidth");e.output=param("echoOutput");
        e.left=float((syncL?60.0/delayBpm*beats[syncL]:param("echoTimeL"))*param("echoOffsetL"));
        e.right=linked?e.left:float((syncR?60.0/delayBpm*beats[syncR]:param("echoTimeR"))*param("echoOffsetR"));
        studioDelay.configure(e);echoWet.setTargetValue(param("delayEnabled")>.5f?juce::jlimit(0.0f,1.0f,param("echoMix")+.4f*fxModulation[6]):0);
        for(int i=0;i<b.getNumSamples();++i){const auto wet=studioDelay.process(left[i],right[i]);const float mix=echoWet.getNextValue();left[i]=(1-mix)*left[i]+mix*wet[0];right[i]=(1-mix)*right[i]+mix*wet[1];}
    }
    auto params=reverb.getParameters();
    const int space=juce::jlimit(0,2,int(param("reverbSpace")));
    static constexpr float spaceSize[]{-.36f,0.0f,.17f},spaceDamping[]{.11f,0.0f,-.13f};
    params.roomSize=juce::jlimit(0.0f,1.0f,param("reverbSize")+spaceSize[space]);
    params.damping=juce::jlimit(0.0f,1.0f,param("reverbDamping")+spaceDamping[space]);
    params.wetLevel=param("reverbEnabled")>0.5f?juce::jlimit(0.0f,0.8f,param("reverb")+0.4f*fxModulation[3])*.65f:0.0f;
    params.dryLevel=1-params.wetLevel*.35f;params.width=param("reverbWidth");
    reverb.setParameters(params);reverb.processStereo(left,right,b.getNumSamples());
    if(sampleGuard.isLocked()&&impulsePending){
        convolver.loadImpulseResponse(std::move(pendingImpulse),pendingImpulseRate,juce::dsp::Convolution::Stereo::yes,juce::dsp::Convolution::Trim::no,juce::dsp::Convolution::Normalise::no);
        impulsePending=false;
    }
    convolutionWet.setTargetValue(param("convolverEnabled")>.5f?param("convolverMix"):0.0f);
    irLevel.setTargetValue(juce::Decibels::decibelsToGain(param("irOutput")));
    for(int offset=0;offset<b.getNumSamples();){
        const int n=std::min(convolutionDry.getNumSamples(),b.getNumSamples()-offset);
        for(int channel=0;channel<2;++channel)convolutionDry.copyFrom(channel,0,b,channel,offset,n);
        auto chunk=juce::dsp::AudioBlock<float>(b).getSubBlock(size_t(offset),size_t(n));
        juce::dsp::ProcessContextReplacing<float> convContext(chunk);convolver.process(convContext);
        for(int i=0;i<n;++i){const float wet=convolutionWet.getNextValue(),level=irLevel.getNextValue();for(int channel=0;channel<2;++channel){
            const float dry=convolutionDry.getSample(channel,i);b.setSample(channel,offset+i,dry+wet*(b.getSample(channel,offset+i)*level-dry));}}
        offset+=n;
    }
    if(param("eqEnabled")>0.5f){
        for(size_t band=0;band<eqParameters.size();++band){
            const auto& values=eqParameters[band];
            const float gain=values[1]->load();
            const int mode=int(eqTypes[band]->load());
            const int stages=(mode==3||mode==4)?int(eqSlopes[band]->load())+1:1;
            if(mode==0&&std::abs(gain)<0.0001f){
                eqState[0][band]={};
                eqState[1][band]={};
                continue;
            }
            for(int stage=0;stage<stages;++stage){
                const auto co=studio::eqCoefficients(sr,values[0]->load(),values[2]->load(),gain,mode,stages,stage);const float scale=1/co[3];
                for(int channel=0;channel<2;++channel){auto* samples=b.getWritePointer(channel);auto& memory=eqState[size_t(channel)][band][size_t(stage)];
                    for(int i=0;i<b.getNumSamples();++i){const float in=samples[i],out=co[0]*scale*in+memory.z1;memory.z1=co[1]*scale*in-co[4]*scale*out+memory.z2;memory.z2=co[2]*scale*in-co[5]*scale*out;samples[i]=out;}}
            }
        }
    }else eqState={};
    const float outputGain=juce::jlimit(0.0f,1.2f,param("gain")+0.5f*fxModulation[14]);
    b.applyGainRamp(0,b.getNumSamples(),previousOutputGain,outputGain);
    previousOutputGain=outputGain;
    const float peak=b.getMagnitude(0,b.getNumSamples());
    auto previous=outputPeak.load(std::memory_order_relaxed);
    while(peak>previous&&!outputPeak.compare_exchange_weak(previous,peak,std::memory_order_relaxed)) {}
    for(int c=0;c<2;++c){
        auto* dst=b.getWritePointer(c);
        for(int s=0;s<b.getNumSamples();++s)dst[s]=std::tanh(dst[s]);
    }
    displayedSamplePosition.store(-1,std::memory_order_relaxed);
    for(const auto& voice:voices)if(voice.active&&voice.source!=0&&voice.source==int(param("source"))&&(!userSample(voice.source)||sampleGuard.isLocked())){const auto& sample=sampleForSource(voice.source);
        double at=voice.samplePosition;if(voice.source==4)for(const auto& grain:voice.granular.grains)if(grain.active){const double length=juce::jmax(1,voice.sampleEnd-voice.sampleStart);double offset=std::fmod(grain.position-voice.sampleStart,length);if(offset<0)offset+=length;at=voice.sampleStart+offset;break;}
        displayedSamplePosition.store(float(at/juce::jmax(1,sample.audio.getNumSamples()-1)),std::memory_order_relaxed);break;}
}
void SpaceInfinityProcessor::getStateInformation(juce::MemoryBlock& m){auto t=state.copyState();std::unique_ptr<juce::XmlElement> xml(t.createXml());{
    const juce::SpinLock::ScopedLockType lock(sampleLock);
    if(importedBytes.getSize()!=0)xml->setAttribute("sampleWav",importedBytes.toBase64Encoding());
    xml->setAttribute("tableFrames",wavetable.count);
    xml->setAttribute("tableData",juce::MemoryBlock(wavetable.frames.data(),sizeof(wavetable.frames)).toBase64Encoding());

}{const juce::ScopedLock lock(impulseSourceLock);if(impulseBytes.getSize())xml->setAttribute("impulseWav",impulseBytes.toBase64Encoding());}copyXmlToBinary(*xml,m);}
void SpaceInfinityProcessor::setStateInformation(const void* data,int size){auto xml=getXmlFromBinary(data,size);if(!xml||!xml->hasTagName(state.state.getType()))return;
    Sample next;
    juce::MemoryBlock bytes;
    const auto encoded=xml->getStringAttribute("sampleWav");
    if(encoded.isNotEmpty()&&encoded.length()<=23*1024*1024){
        if(!bytes.fromBase64Encoding(encoded)||bytes.getSize()>16*1024*1024||decodeSample(bytes,next).failed())
            bytes.reset();
    }
    xml->removeAttribute("sampleWav");
    infinity::Wavetable nextTable;juce::MemoryBlock tableData;
    const auto encodedTable=xml->getStringAttribute("tableData");
    if(encodedTable.length()<300000&&tableData.fromBase64Encoding(encodedTable)&&tableData.getSize()==sizeof(nextTable.frames)){
        std::memcpy(nextTable.frames.data(),tableData.getData(),sizeof(nextTable.frames));
        nextTable.count=juce::jlimit(1,infinity::Wavetable::maxFrames,xml->getIntAttribute("tableFrames",1));
        for(auto& frame:nextTable.frames)for(auto& value:frame)value=std::isfinite(value)?juce::jlimit(-1.0f,1.0f,value):0.0f;
    }
    juce::MemoryBlock ir;const auto encodedIr=xml->getStringAttribute("impulseWav");
    if(encodedIr.length()<6000000)ir.fromBase64Encoding(encodedIr);
    Sample decodedIr;if(ir.getSize()&&decodeSample(ir,decodedIr).failed())ir.reset();
    xml->removeAttribute("tableData");xml->removeAttribute("tableFrames");xml->removeAttribute("impulseWav");
    {
        const juce::SpinLock::ScopedLockType lock(sampleLock);
        std::swap(importedSample,next);
        std::swap(importedBytes,bytes);
        wavetable=nextTable;
        ++sampleRevision;
    }
    queueImpulse(ir);
    for(auto* parameter:getParameters()){
        if(auto* ranged=dynamic_cast<juce::RangedAudioParameter*>(parameter))
            if(xml->getChildByAttribute("id",ranged->paramID)==nullptr)
                ranged->setValueNotifyingHost(ranged->getDefaultValue());
    }
    state.replaceState(juce::ValueTree::fromXml(*xml));}
namespace {
struct FactoryPatch {
    const char* name;
    std::array<float,16> values;
    int source, division, delayMode, space, motionShape, motionBars;
    float motionDepth;
    int filterMode;
};
constexpr FactoryPatch factoryBank[]={
{"HARD TRANCE LEAD",{3925.37215f,0.16000f,0.04200f,1.14000f,0.00320f,0.44000f,0.12000f,0.18000f,0.22000f,0.17000f,4.86000f,0.13000f,0.60000f,0.00000f,0.62500f,0.82000f},0,0,0,1,0,2,0.00000f,1},
{"JV CHOIR INSPIRED",{1160.49677f,0.08000f,0.68200f,2.74000f,0.00320f,0.37838f,0.12000f,0.18000f,0.09000f,0.49000f,4.86000f,0.13000f,0.60000f,0.00000f,0.86500f,0.78000f},0,0,0,1,0,2,0.00000f,1},
{"RAVE STAB 1996",{4364.16776f,0.22000f,0.01200f,0.74000f,0.00320f,0.41667f,0.12000f,0.18000f,0.16000f,0.20000f,4.86000f,0.13000f,0.60000f,0.00000f,0.32500f,0.05000f},0,0,0,1,0,2,0.00000f,1},
{"ETHEREAL STRINGS",{1773.07186f,0.07000f,0.90200f,3.06000f,0.00320f,0.47500f,0.12000f,0.18000f,0.16000f,0.55000f,4.86000f,0.13000f,0.60000f,0.00000f,1.12500f,0.88000f},0,0,0,1,0,2,0.00000f,1},
{"ACID SIGNAL",{524.19085f,0.74000f,0.02200f,0.58000f,0.00320f,0.10526f,0.12000f,0.18000f,0.16000f,0.08000f,4.86000f,0.13000f,0.60000f,0.00000f,0.22500f,0.70000f},0,0,0,1,0,2,0.00000f,1},
{"INFINITY PAD",{844.46594f,0.10000f,1.34200f,3.30000f,0.00320f,0.48718f,0.12000f,0.18000f,0.28000f,0.63000f,4.86000f,0.13000f,0.60000f,0.00000f,1.18500f,0.90000f},0,0,0,1,0,2,0.00000f,1},
{"ORBITAL VOX",{683.18911f,0.26000f,0.40200f,2.26000f,0.01120f,0.45652f,0.12000f,0.54000f,0.06000f,0.49000f,4.86000f,0.13000f,0.60000f,3.00000f,0.78500f,0.81000f},1,0,0,1,0,2,0.00000f,1},
{"ETHEREAL CHOIR",{1160.49677f,0.12000f,0.92200f,2.90000f,0.00960f,0.35632f,0.12000f,0.62000f,0.12000f,0.67000f,4.86000f,0.13000f,0.60000f,2.00000f,1.10500f,0.89000f},2,0,0,1,0,2,0.00000f,1},
{"NEON STRINGS",{2310.88236f,0.09000f,0.66200f,2.34000f,0.02480f,0.48980f,0.12000f,0.63000f,0.12000f,0.39000f,4.86000f,0.13000f,0.60000f,0.00000f,0.92500f,0.82000f},0,0,0,1,0,2,0.00000f,1},
{"VOLTAGE BASS",{720.36273f,0.17000f,0.00400f,0.11600f,0.00400f,0.25253f,0.12000f,0.02000f,0.00000f,0.01000f,4.86000f,0.13000f,0.60000f,0.00000f,0.18500f,0.66000f},0,0,0,1,0,2,0.00000f,4},
{"REDLINE ANTHEM",{6667.81952f,0.09000f,0.00600f,0.54000f,0.02880f,0.45865f,0.12000f,0.39000f,0.24000f,0.22000f,4.86000f,0.13000f,0.60000f,0.00000f,0.34500f,0.82000f},0,0,0,1,0,2,0.00000f,1},
{"WAREHOUSE PULSE",{2436.62187f,0.23000f,0.00400f,0.18000f,0.01600f,0.36792f,0.12000f,0.22000f,0.17000f,0.15000f,4.86000f,0.13000f,0.60000f,1.00000f,0.13700f,0.03000f},0,0,0,1,0,2,0.00000f,1},
{"Apex Meridian",{5687.91289f,0.09000f,0.03800f,0.58000f,0.02384f,0.47115f,0.12000f,0.30000f,0.29000f,0.25000f,4.86000f,0.13000f,0.47000f,0.00000f,0.58500f,0.86000f},0,3,1,2,0,2,0.12000f,1},
{"Solar Flare",{471.48604f,0.15000f,0.00800f,0.34000f,0.02014f,0.38462f,0.28000f,0.23000f,0.21000f,0.14000f,4.86000f,0.13000f,0.47000f,0.00000f,0.30500f,0.72000f},0,2,0,1,0,2,0.00000f,2},
{"Glacier Wire",{2310.88236f,0.36000f,0.05200f,0.78000f,0.03229f,0.38462f,0.08000f,0.46000f,0.29000f,0.33000f,4.50300f,0.41000f,0.47000f,3.00000f,0.48500f,0.76000f},0,3,1,2,0,2,0.00000f,3},
{"Iron Cathedral",{2310.88236f,0.22000f,0.01800f,0.74000f,0.02806f,0.38462f,0.49000f,0.38000f,0.14000f,0.39000f,4.86000f,0.13000f,0.47000f,1.00000f,0.54500f,0.84000f},0,4,1,2,0,2,0.00000f,1},
{"Blue Horizon",{3175.70123f,0.06000f,0.11200f,0.90000f,0.02806f,0.47115f,0.05000f,0.38000f,0.31000f,0.38000f,4.86000f,0.13000f,0.47000f,0.00000f,0.68500f,0.88000f},0,3,1,2,0,2,0.00000f,1},
{"Riot Alarm",{3530.69528f,0.62000f,0.00400f,0.18000f,0.01170f,0.38462f,0.53000f,0.07000f,0.16000f,0.06000f,1.88500f,0.41000f,0.47000f,1.00000f,0.24500f,0.92000f},0,1,0,0,0,2,0.00000f,3},
{"Ivory Signal",{4138.95912f,0.11000f,0.01000f,0.26000f,0.01486f,0.25275f,0.15000f,0.13000f,0.28000f,0.20000f,4.86000f,0.13000f,0.47000f,2.00000f,0.22500f,0.21000f},0,3,1,1,0,2,0.00000f,1},
{"Red Mercury",{1434.44908f,0.48000f,0.00400f,0.14800f,0.01275f,0.38462f,0.55000f,0.09000f,0.15000f,0.10000f,4.86000f,0.13000f,0.47000f,0.00000f,0.19500f,0.46000f},0,2,0,0,0,2,0.00000f,1},
{"Orbit Brass",{1594.79820f,0.15000f,0.07200f,0.42000f,0.01962f,0.38462f,0.32000f,0.22000f,0.11000f,0.15000f,4.86000f,0.13000f,0.47000f,0.00000f,0.50500f,0.66000f},0,2,0,0,0,2,0.00000f,1},
{"Chromed Glass",{361.75733f,0.10000f,0.00800f,0.74000f,0.01803f,0.25275f,0.04000f,0.19000f,0.35000f,0.36000f,4.86000f,0.13000f,0.47000f,2.00000f,0.30500f,0.35000f},0,3,1,2,0,2,0.00000f,2},
{"Pulse Beacon",{2569.20310f,0.18000f,0.00400f,0.38000f,0.02331f,0.38462f,0.21000f,0.29000f,0.23000f,0.12000f,4.86000f,0.13000f,0.47000f,1.00000f,0.28500f,0.64000f},0,2,0,1,0,2,0.00000f,1},
{"Black Static",{1290.22228f,0.49000f,0.00600f,0.22000f,0.01328f,0.38462f,0.66000f,0.10000f,0.08000f,0.07000f,4.86000f,0.13000f,0.47000f,1.00000f,0.22500f,0.50000f},0,1,0,0,0,2,0.00000f,3},
{"Afterimage",{2310.88236f,0.08000f,0.15600f,1.38000f,0.03520f,0.47115f,0.16000f,0.53000f,0.34000f,0.39000f,4.50300f,0.41000f,0.47000f,3.00000f,0.70500f,0.90000f},0,4,1,2,1,2,0.00000f,1},
{"Rave Telegraph",{800.88809f,0.23000f,0.00400f,0.20800f,0.01434f,0.38462f,0.26000f,0.12000f,0.33000f,0.08000f,4.86000f,0.13000f,0.47000f,1.00000f,0.11500f,0.17000f},0,1,0,0,0,2,0.00000f,2},
{"Acid Crown",{759.55904f,0.68000f,0.00400f,0.22000f,0.01011f,0.38462f,0.41000f,0.04000f,0.15000f,0.09000f,4.86000f,0.13000f,0.47000f,0.00000f,0.14100f,0.48000f},0,2,0,1,0,2,0.00000f,1},
{"Eclipse Reed",{938.86411f,0.41000f,0.03800f,0.50000f,0.02226f,0.38462f,0.19000f,0.27000f,0.21000f,0.19000f,1.88500f,0.41000f,0.47000f,1.00000f,0.40500f,0.87000f},0,3,1,1,0,2,0.00000f,3},
{"Starlight Keys",{1869.54807f,0.11000f,0.01000f,0.54000f,0.02120f,0.25275f,0.05000f,0.25000f,0.26000f,0.29000f,4.86000f,0.13000f,0.47000f,3.00000f,0.32500f,0.12000f},0,2,0,2,0,2,0.00000f,1},
{"Pressure Valve",{614.49781f,0.30000f,0.00800f,0.14800f,0.00958f,0.38462f,0.56000f,0.03000f,0.17000f,0.07000f,4.86000f,0.13000f,0.47000f,0.00000f,0.17100f,0.55000f},0,1,0,0,0,2,0.00000f,2},
{"Twin Engines",{9163.16772f,0.07000f,0.01800f,0.46000f,0.02226f,0.47115f,0.32000f,0.27000f,0.11000f,0.18000f,4.86000f,0.13000f,0.47000f,0.00000f,0.40500f,0.79000f},0,2,0,1,0,2,0.00000f,1},
{"Crimson Spiral",{1773.07186f,0.29000f,0.03000f,0.86000f,0.03070f,0.38462f,0.20000f,0.43000f,0.29000f,0.25000f,7.12100f,0.41000f,0.47000f,0.00000f,0.46500f,0.70000f},0,3,1,1,0,2,0.00000f,3},
{"Zero Gravity",{2708.99833f,0.02000f,0.09200f,0.98000f,0.01962f,0.25275f,0.11000f,0.22000f,0.37000f,0.38000f,4.86000f,0.13000f,0.47000f,2.00000f,0.56500f,0.92000f},0,4,1,2,0,2,0.00000f,1},
{"Burnt Copper",{1160.49677f,0.38000f,0.01400f,0.30000f,0.01539f,0.38462f,0.63000f,0.14000f,0.13000f,0.13000f,4.86000f,0.13000f,0.47000f,1.00000f,0.32500f,0.59000f},0,2,0,1,0,2,0.00000f,1},
{"Hyper Relay",{890.41494f,0.17000f,0.00400f,0.30000f,0.02173f,0.38462f,0.26000f,0.26000f,0.31000f,0.11000f,4.86000f,0.13000f,0.47000f,3.00000f,0.19300f,0.36000f},0,3,1,1,0,2,0.00000f,2},
{"Dreamline",{3530.69528f,0.06000f,0.07200f,0.98000f,0.03070f,0.47115f,0.07000f,0.43000f,0.30000f,0.34000f,4.86000f,0.13000f,0.47000f,3.00000f,0.58500f,0.85000f},0,3,1,2,0,2,0.00000f,1},
{"Circuit Bent",{3175.70123f,0.45000f,0.00400f,0.14400f,0.01381f,0.38462f,0.49000f,0.11000f,0.18000f,0.08000f,7.12100f,0.41000f,0.47000f,1.00000f,0.12100f,0.30000f},0,1,0,0,0,2,0.00000f,3},
{"Titan Horn",{1223.64161f,0.12000f,0.04800f,0.34000f,0.01698f,0.38462f,0.38000f,0.17000f,0.12000f,0.13000f,4.86000f,0.13000f,0.47000f,0.00000f,0.40500f,0.77000f},0,2,0,1,0,2,0.00000f,1},
{"Prism Drops",{402.19618f,0.21000f,0.00400f,0.78000f,0.02173f,0.25275f,0.06000f,0.26000f,0.37000f,0.29000f,4.86000f,0.13000f,0.47000f,2.00000f,0.19500f,0.06000f},0,3,1,2,0,2,0.00000f,2},
{"Rush Hour",{3925.37215f,0.31000f,0.00600f,0.22800f,0.01645f,0.38462f,0.47000f,0.16000f,0.22000f,0.08000f,4.86000f,0.13000f,0.47000f,0.00000f,0.22500f,0.68000f},0,2,0,0,0,2,0.00000f,1},
{"Moon Quartz",{1160.49677f,0.20000f,0.05200f,1.02000f,0.02595f,0.38462f,0.10000f,0.34000f,0.35000f,0.32000f,5.81200f,0.41000f,0.47000f,3.00000f,0.44500f,0.72000f},0,4,1,2,0,2,0.00000f,3},
{"Ion Stutter",{2708.99833f,0.36000f,0.00400f,0.18400f,0.01222f,0.38462f,0.23000f,0.08000f,0.29000f,0.12000f,4.86000f,0.13000f,0.47000f,1.00000f,0.09500f,0.08000f},0,1,0,0,0,2,0.00000f,1},
{"Phantom Vowel",{844.46594f,0.61000f,0.04600f,0.82000f,0.03018f,0.38462f,0.13000f,0.42000f,0.14000f,0.29000f,1.88500f,0.41000f,0.47000f,1.00000f,0.40500f,0.82000f},0,3,1,2,6,1,0.35000f,3},
{"Neon Needle",{1160.49677f,0.22000f,0.00400f,0.24000f,0.01381f,0.38462f,0.16000f,0.11000f,0.23000f,0.14000f,4.86000f,0.13000f,0.47000f,2.00000f,0.21100f,0.71000f},0,2,0,1,0,2,0.00000f,2},
{"Detonation",{3011.82225f,0.24000f,0.00400f,0.16000f,0.01856f,0.38462f,0.61000f,0.20000f,0.09000f,0.10000f,4.86000f,0.13000f,0.47000f,0.00000f,0.12900f,0.14000f},0,1,0,0,0,2,0.00000f,1},
{"Aether Bell",{2078.53450f,0.31000f,0.00600f,0.98000f,0.02067f,0.25275f,0.09000f,0.24000f,0.32000f,0.41000f,4.86000f,0.13000f,0.47000f,2.00000f,0.50500f,0.04000f},0,3,1,2,0,2,0.00000f,3},
{"Night Mission",{938.86411f,0.24000f,0.02200f,0.31200f,0.02542f,0.38462f,0.31000f,0.33000f,0.19000f,0.16000f,4.86000f,0.13000f,0.47000f,1.00000f,0.36500f,0.75000f},0,2,0,1,0,2,0.00000f,1},
{"Saturn Sweep",{800.88809f,0.43000f,0.23200f,1.10000f,0.02648f,0.38462f,0.24000f,0.35000f,0.25000f,0.31000f,1.88500f,0.41000f,0.47000f,0.00000f,0.70500f,0.82000f},0,4,1,2,19,2,0.31000f,1},
{"Digital Orchid",{308.59326f,0.15000f,0.02200f,0.50000f,0.02437f,0.38462f,0.13000f,0.31000f,0.29000f,0.25000f,4.86000f,0.13000f,0.47000f,3.00000f,0.34500f,0.43000f},0,3,1,1,0,2,0.00000f,2},
{"Razor Octave",{5687.91289f,0.26000f,0.00800f,0.28000f,0.01064f,0.38462f,0.58000f,0.05000f,0.20000f,0.15000f,4.86000f,0.13000f,0.47000f,1.00000f,0.30500f,0.82000f},0,2,0,1,0,2,0.00000f,1},
{"Final Approach",{1434.44908f,0.35000f,0.08400f,0.94000f,0.02965f,0.47115f,0.14000f,0.41000f,0.33000f,0.36000f,5.81200f,0.41000f,0.47000f,0.00000f,0.54500f,0.81000f},0,3,1,2,0,2,0.00000f,3},
{"Orbital Uplink",{3722.80713f,0.18000f,0.01400f,0.62000f,0.03387f,0.47115f,0.24000f,0.49000f,0.28000f,0.21000f,4.86000f,0.13000f,0.47000f,0.00000f,0.44500f,0.92000f},0,3,1,1,11,2,0.00000f,1},
{"Kick Gap",{497.14049f,0.10000f,0.00200f,0.06800f,0.00080f,0.25000f,0.21000f,0.00000f,0.00000f,0.00000f,4.86000f,0.13000f,0.52000f,0.00000f,0.11100f,0.51000f},0,0,0,0,0,2,0.00000f,4},
{"Bounce Circuit",{647.93380f,0.22000f,0.00400f,0.08000f,0.00080f,0.25000f,0.26000f,0.00000f,0.00000f,0.00000f,4.86000f,0.13000f,0.52000f,1.00000f,0.12500f,0.45000f},0,0,0,0,0,2,0.00000f,4},
{"Rubber Driver",{552.71307f,0.37000f,0.00200f,0.09600f,0.00080f,0.25000f,0.27000f,0.00000f,0.00000f,0.00000f,4.86000f,0.13000f,0.52000f,3.00000f,0.10700f,0.63000f},0,0,0,0,0,2,0.00000f,4},
{"Sub Relay",{308.59326f,0.06000f,0.00400f,0.07200f,0.00080f,0.25000f,0.09000f,0.00000f,0.00000f,0.00000f,4.86000f,0.13000f,0.52000f,2.00000f,0.14100f,0.62000f},0,0,0,0,0,2,0.00000f,4},
{"Warehouse Offbeat",{890.41494f,0.14000f,0.00200f,0.07200f,0.00080f,0.25000f,0.40000f,0.00000f,0.00000f,0.00000f,4.86000f,0.13000f,0.52000f,0.00000f,0.12900f,0.37000f},0,0,0,0,0,2,0.00000f,4},
{"Triplet Teeth",{402.19618f,0.16000f,0.00200f,0.04400f,0.00080f,0.25000f,0.23000f,0.00000f,0.00000f,0.00000f,4.86000f,0.13000f,0.52000f,0.00000f,0.06700f,0.12000f},0,0,0,0,0,2,0.00000f,4},
{"Tight Machine",{614.49781f,0.23000f,0.00200f,0.05200f,0.00080f,0.25000f,0.32000f,0.00000f,0.00000f,0.00000f,4.86000f,0.13000f,0.52000f,1.00000f,0.07700f,0.10000f},0,0,0,0,0,2,0.00000f,4},
{"Low Conveyor",{424.08044f,0.08000f,0.00400f,0.05200f,0.00080f,0.25000f,0.14000f,0.00000f,0.00000f,0.00000f,4.86000f,0.13000f,0.52000f,3.00000f,0.09100f,0.20000f},0,0,0,0,0,2,0.00000f,4},
{"Acid Runner",{683.18911f,0.49000f,0.00200f,0.04800f,0.00080f,0.25000f,0.37000f,0.00000f,0.00000f,0.00000f,4.86000f,0.13000f,0.52000f,0.00000f,0.07500f,0.13000f},0,0,0,0,0,2,0.00000f,4},
{"Sixteenth Spine",{471.48604f,0.19000f,0.00200f,0.04800f,0.00080f,0.25000f,0.25000f,0.00000f,0.00000f,0.00000f,4.86000f,0.13000f,0.52000f,3.00000f,0.08700f,0.15000f},0,0,0,0,0,2,0.00000f,4},
{"Cathedral Strike",{292.66861f,0.14000f,0.00400f,0.13600f,0.00320f,0.35644f,0.08000f,0.04000f,0.10000f,0.12000f,4.86000f,0.13000f,0.60000f,0.00000f,0.14700f,0.05000f},1,2,0,1,0,2,0.00000f,2},
{"Redline Kyrie",{2191.63152f,0.21000f,0.00600f,0.20800f,0.00320f,0.35644f,0.08000f,0.13000f,0.19000f,0.16000f,4.86000f,0.13000f,0.60000f,0.00000f,0.19700f,0.28000f},1,1,0,0,0,2,0.00000f,1},
{"Vault Chant",{683.18911f,0.29000f,0.01000f,0.23600f,0.00320f,0.35644f,0.08000f,0.05000f,0.15000f,0.18000f,4.86000f,0.13000f,0.60000f,0.00000f,0.17500f,0.12000f},2,2,0,1,0,2,0.00000f,3},
{"Eclipse Choir",{582.78725f,0.19000f,0.03800f,0.66000f,0.00320f,0.35644f,0.08000f,0.22000f,0.12000f,0.23000f,0.81400f,0.27000f,0.60000f,0.00000f,0.34500f,0.58000f},1,3,1,2,0,3,0.29000f,1},
{"Halo Ooh",{1971.27374f,0.07000f,0.30200f,1.30000f,0.00320f,0.35644f,0.08000f,0.32000f,0.15000f,0.34000f,4.86000f,0.13000f,0.60000f,0.00000f,0.62500f,0.84000f},2,4,1,2,0,2,0.00000f,1},
{"White Chapel",{343.08919f,0.08000f,0.24200f,1.18000f,0.00320f,0.35644f,0.08000f,0.25000f,0.20000f,0.32000f,4.86000f,0.13000f,0.60000f,0.00000f,0.56500f,0.77000f},1,3,1,2,0,2,0.00000f,2},
{"Midnight Vespers",{497.14049f,0.23000f,0.18200f,1.06000f,0.00320f,0.35644f,0.08000f,0.16000f,0.08000f,0.24000f,0.81400f,0.27000f,0.60000f,0.00000f,0.48500f,0.80000f},2,2,0,1,6,2,0.30000f,1},
{"Choral Signal",{1100.61046f,0.32000f,0.00800f,0.30000f,0.00320f,0.35644f,0.08000f,0.10000f,0.24000f,0.12000f,4.86000f,0.13000f,0.60000f,0.00000f,0.17300f,0.39000f},2,1,0,0,0,2,0.00000f,3},
{"Reverse Procession",{249.65786f,0.10000f,0.64200f,0.86000f,0.00320f,0.35644f,0.08000f,0.11000f,0.18000f,0.36000f,4.86000f,0.13000f,0.60000f,0.00000f,0.50500f,0.92000f},1,3,1,2,0,2,0.00000f,2},
{"Final Benediction",{1512.50019f,0.12000f,0.08200f,1.06000f,0.00320f,0.35644f,0.08000f,0.28000f,0.22000f,0.34000f,0.81400f,0.27000f,0.60000f,0.00000f,0.42500f,0.72000f},2,3,1,2,19,3,0.28000f,1},
{"Black Orbit",{552.71307f,0.29000f,0.44200f,1.54000f,0.01616f,0.39326f,0.26000f,0.28000f,0.16000f,0.25000f,1.17100f,0.52000f,0.46000f,1.00000f,0.81900f,0.72000f},0,2,0,1,11,2,0.29000f,1},
{"Subterranean Choir",{720.36273f,0.37000f,0.66200f,1.94000f,0.02064f,0.42553f,0.14000f,0.42000f,0.18000f,0.36000f,0.81400f,0.52000f,0.46000f,3.00000f,0.89600f,0.76500f},0,3,1,2,0,2,0.00000f,3},
{"Pressure Chamber",{424.08044f,0.45000f,0.28200f,0.98000f,0.01328f,0.45455f,0.31000f,0.19000f,0.24000f,0.20000f,2.59900f,0.52000f,0.46000f,0.00000f,0.76300f,0.81000f},0,4,1,1,2,2,0.32000f,1},
{"Iron Mist",{938.86411f,0.31000f,0.58200f,1.74000f,0.01776f,0.39326f,0.23000f,0.33000f,0.10000f,0.34000f,1.52800f,0.52000f,0.46000f,1.00000f,0.86800f,0.85500f},0,3,1,2,0,2,0.00000f,3},
{"Cold Reactor",{844.46594f,0.28000f,0.40200f,1.22000f,0.01424f,0.42553f,0.36000f,0.22000f,0.18000f,0.25000f,2.00400f,0.52000f,0.46000f,0.00000f,0.80500f,0.72000f},0,2,0,1,0,2,0.00000f,1},
{"Shadow Vector",{647.93380f,0.33000f,0.76200f,2.22000f,0.01840f,0.45455f,0.19000f,0.35000f,0.29000f,0.41000f,0.69500f,0.52000f,0.46000f,3.00000f,0.93100f,0.76500f},0,3,1,2,6,3,0.24000f,1},
{"Afterhours Static",{224.55599f,0.29000f,0.32200f,1.26000f,0.01616f,0.39326f,0.23000f,0.28000f,0.32000f,0.27000f,3.07500f,0.52000f,0.46000f,1.00000f,0.77700f,0.81000f},0,4,1,1,0,2,0.00000f,2},
{"Deep Warning",{402.19618f,0.24000f,0.84200f,2.10000f,0.01712f,0.42553f,0.19000f,0.31000f,0.15000f,0.34000f,0.93300f,0.52000f,0.46000f,2.00000f,0.95900f,0.85500f},0,3,0,2,0,2,0.00000f,1},
{"Obsidian Sweep",{759.55904f,0.42000f,1.02200f,2.34000f,0.01936f,0.45455f,0.29000f,0.38000f,0.22000f,0.42000f,0.57600f,0.52000f,0.46000f,0.00000f,1.02200f,0.72000f},0,2,1,2,3,3,0.31000f,3},
{"Night Transmission",{1043.81453f,0.23000f,0.50200f,1.54000f,0.01744f,0.39326f,0.25000f,0.32000f,0.25000f,0.31000f,1.76600f,0.52000f,0.46000f,1.00000f,0.84000f,0.76500f},0,3,1,1,8,2,0.30000f,1},
{"Airborne Silk",{3925.37215f,0.07000f,0.68200f,1.74000f,0.02160f,0.42553f,0.04000f,0.45000f,0.23000f,0.41000f,0.69500f,0.38000f,0.43000f,0.00000f,0.90300f,0.81000f},0,4,1,2,1,3,0.18000f,1},
{"Morning Horizon",{2569.20310f,0.09000f,0.84200f,2.30000f,0.02032f,0.45455f,0.03000f,0.41000f,0.26000f,0.44000f,0.57600f,0.38000f,0.43000f,0.00000f,0.95900f,0.85500f},0,3,1,2,0,2,0.00000f,1},
{"Ivory Clouds",{1773.07186f,0.08000f,0.92200f,2.26000f,0.01904f,0.39326f,0.02500f,0.37000f,0.13000f,0.47000f,0.81400f,0.38000f,0.43000f,2.00000f,0.98700f,0.72000f},0,2,0,2,0,2,0.00000f,1},
{"Blue Refraction",{325.38441f,0.12000f,0.56200f,1.94000f,0.01968f,0.42553f,0.03500f,0.39000f,0.31000f,0.34000f,1.05200f,0.38000f,0.43000f,3.00000f,0.86100f,0.76500f},0,3,1,1,0,2,0.00000f,2},
{"Summer Meridian",{4601.63043f,0.07000f,0.46200f,1.54000f,0.02192f,0.45455f,0.05500f,0.46000f,0.28000f,0.37000f,0.93300f,0.38000f,0.43000f,0.00000f,0.82600f,0.81000f},0,4,1,2,0,2,0.00000f,1},
{"Ocean Glass",{1434.44908f,0.16000f,0.70200f,2.10000f,0.01840f,0.39326f,0.02000f,0.35000f,0.33000f,0.48000f,0.69500f,0.38000f,0.43000f,2.00000f,0.91000f,0.85500f},0,3,1,2,0,2,0.00000f,3},
{"First Light Strings",{3011.82225f,0.10000f,0.38200f,1.62000f,0.02064f,0.42553f,0.04500f,0.42000f,0.14000f,0.34000f,1.17100f,0.38000f,0.43000f,0.00000f,0.79800f,0.72000f},0,2,0,1,0,2,0.00000f,1},
{"Ethereal Lift",{263.24222f,0.09000f,1.02200f,2.58000f,0.02128f,0.45455f,0.02500f,0.44000f,0.25000f,0.51000f,0.57600f,0.38000f,0.43000f,0.00000f,1.02200f,0.76500f},0,3,1,2,0,2,0.00000f,2},
{"Silver Current",{2310.88236f,0.12000f,0.64200f,1.90000f,0.01936f,0.39326f,0.05000f,0.38000f,0.25000f,0.37000f,1.40900f,0.38000f,0.43000f,3.00000f,0.88900f,0.81000f},0,4,1,2,0,2,0.00000f,1},
{"Open Skies",{5687.91289f,0.06000f,0.72200f,2.22000f,0.02288f,0.42553f,0.03500f,0.49000f,0.31000f,0.46000f,0.69500f,0.38000f,0.43000f,0.00000f,0.91700f,0.85500f},0,3,1,2,5,2,0.15000f,1},
{"Gate Communion",{3175.70123f,0.11000f,0.05200f,0.86000f,0.02320f,0.35484f,0.12000f,0.37000f,0.24000f,0.24000f,1.17100f,0.60000f,0.46000f,0.00000f,0.46500f,0.83000f},0,3,1,1,1,1,0.23000f,1},
{"Strobelight Chord",{1512.50019f,0.24000f,0.02800f,0.54000f,0.02320f,0.35484f,0.31000f,0.19000f,0.30000f,0.17000f,1.17100f,0.60000f,0.46000f,1.00000f,0.34500f,0.75000f},0,2,0,0,8,1,0.25000f,3},
{"Rave Chapel Organ",{3925.37215f,0.07000f,0.00400f,0.15600f,0.00160f,0.35484f,0.12000f,0.08000f,0.13000f,0.14000f,1.17100f,0.60000f,0.46000f,2.00000f,0.20500f,0.86000f},0,2,0,0,0,2,0.00000f,1},
{"Warehouse Drawbars",{2191.63152f,0.13000f,0.00400f,0.18800f,0.00160f,0.35484f,0.12000f,0.11000f,0.10000f,0.15000f,1.17100f,0.60000f,0.46000f,2.00000f,0.16500f,0.81000f},0,1,0,0,0,2,0.00000f,1},
{"Airlock Mallet",{1160.49677f,0.18000f,0.00400f,0.28800f,0.00160f,0.35484f,0.12000f,0.08000f,0.28000f,0.34000f,1.17100f,0.60000f,0.46000f,2.00000f,0.22500f,0.01000f},0,3,1,2,0,2,0.00000f,3},
{"Meteor Lift",{447.15546f,0.38000f,1.08200f,0.78000f,0.00160f,0.35484f,0.12000f,0.28000f,0.23000f,0.29000f,1.17100f,0.60000f,0.46000f,0.00000f,0.46500f,0.87000f},0,4,1,2,3,1,0.27000f,1},
{"Gravity Drop",{1971.27374f,0.27000f,0.00400f,0.74000f,0.00160f,0.35484f,0.12000f,0.11000f,0.18000f,0.29000f,1.17100f,0.60000f,0.46000f,1.00000f,0.48500f,0.09000f},0,2,0,2,12,0,0.00000f,1},
{"Reentry Siren",{1434.44908f,0.44000f,0.06000f,0.94000f,0.00160f,0.35484f,0.31000f,0.10000f,0.18000f,0.31000f,1.40900f,0.60000f,0.46000f,1.00000f,0.52500f,0.82000f},0,3,1,1,0,2,0.00000f,3},
{"Sub Impact",{720.36273f,0.08000f,0.00400f,0.70000f,0.00160f,0.35484f,0.12000f,0.01000f,0.03000f,0.22000f,1.17100f,0.60000f,0.46000f,2.00000f,0.26500f,0.02000f},0,2,0,2,0,2,0.00000f,1},
{"Frozen Interlude",{249.65786f,0.12000f,0.15200f,1.30000f,0.02320f,0.35484f,0.12000f,0.42000f,0.38000f,0.48000f,1.17100f,0.60000f,0.46000f,3.00000f,0.58500f,0.69000f},0,4,1,2,5,3,0.19000f,2},
{"Acid Ghost",{800.88809f,0.60000f,0.00400f,0.37200f,0.00160f,0.35484f,0.31000f,0.04000f,0.36000f,0.18000f,1.17100f,0.60000f,0.46000f,0.00000f,0.19500f,0.22000f},0,3,1,1,0,2,0.00000f,3},
{"Soprano Ah Ensemble",{8000.00000f,0.08000f,0.12000f,0.80000f,0.00200f,0.45000f,0.05000f,0.28000f,0.13000f,0.34000f,1.00000f,0.04000f,0.58000f,0.00000f,0.55000f,0.78000f},5,2,1,2,0,2,0.00000f,1},
{"Soprano Ih Air",{8000.00000f,0.08000f,0.12000f,0.80000f,0.00200f,0.45000f,0.05000f,0.28000f,0.13000f,0.34000f,1.00000f,0.04000f,0.58000f,0.00000f,0.55000f,0.78000f},6,2,1,2,0,2,0.00000f,1},
{"Soprano Oh Halo",{8000.00000f,0.08000f,0.12000f,0.80000f,0.00200f,0.45000f,0.05000f,0.28000f,0.13000f,0.34000f,1.00000f,0.04000f,0.58000f,0.00000f,0.55000f,0.78000f},7,2,1,2,0,2,0.00000f,1},
{"Tenor Ah Cathedral",{8000.00000f,0.08000f,0.12000f,0.80000f,0.00200f,0.45000f,0.05000f,0.28000f,0.13000f,0.34000f,1.00000f,0.04000f,0.58000f,0.00000f,0.55000f,0.78000f},8,2,1,2,0,2,0.00000f,1},
{"Tenor Oh Velvet",{8000.00000f,0.08000f,0.12000f,0.80000f,0.00200f,0.45000f,0.05000f,0.28000f,0.13000f,0.34000f,1.00000f,0.04000f,0.58000f,0.00000f,0.55000f,0.78000f},9,2,1,2,0,2,0.00000f,1},
{"Bass Ah Chamber",{8000.00000f,0.08000f,0.12000f,0.80000f,0.00200f,0.45000f,0.05000f,0.28000f,0.13000f,0.34000f,1.00000f,0.04000f,0.58000f,0.00000f,0.55000f,0.78000f},10,2,1,2,0,2,0.00000f,1},
{"Bass Ih Shadow",{8000.00000f,0.08000f,0.12000f,0.80000f,0.00200f,0.45000f,0.05000f,0.28000f,0.13000f,0.34000f,1.00000f,0.04000f,0.58000f,0.00000f,0.55000f,0.78000f},11,2,1,2,0,2,0.00000f,1},
{"Bass Oh Ritual",{8000.00000f,0.08000f,0.12000f,0.80000f,0.00200f,0.45000f,0.05000f,0.28000f,0.13000f,0.34000f,1.00000f,0.04000f,0.58000f,0.00000f,0.55000f,0.78000f},12,2,1,2,0,2,0.00000f,1},
{"Vibraphone Glow",{8000.00000f,0.08000f,0.00300f,0.35000f,0.00200f,0.45000f,0.05000f,0.10000f,0.13000f,0.18000f,1.00000f,0.04000f,0.58000f,0.00000f,0.25000f,0.20000f},13,2,1,2,0,2,0.00000f,1},
{"Crystal Glock",{8000.00000f,0.08000f,0.00300f,0.35000f,0.00200f,0.45000f,0.05000f,0.10000f,0.13000f,0.18000f,1.00000f,0.04000f,0.58000f,0.00000f,0.25000f,0.20000f},14,2,1,2,0,2,0.00000f,1},
{"Marimba Pulse",{8000.00000f,0.08000f,0.00300f,0.35000f,0.00200f,0.45000f,0.05000f,0.10000f,0.13000f,0.18000f,1.00000f,0.04000f,0.58000f,0.00000f,0.25000f,0.20000f},15,2,1,2,0,2,0.00000f,1},
{"Lute Pluck Echo",{8000.00000f,0.08000f,0.00300f,0.35000f,0.00200f,0.45000f,0.05000f,0.10000f,0.13000f,0.18000f,1.00000f,0.04000f,0.58000f,0.00000f,0.25000f,0.20000f},16,2,1,2,0,2,0.00000f,1},
{"FM Clavi Bounce",{8000.00000f,0.08000f,0.00300f,0.35000f,0.00200f,0.45000f,0.05000f,0.10000f,0.13000f,0.18000f,1.00000f,0.04000f,0.58000f,0.00000f,0.25000f,0.20000f},17,2,1,2,0,2,0.00000f,1},
{"FM Piano Horizon",{8000.00000f,0.08000f,0.00300f,0.35000f,0.00200f,0.45000f,0.05000f,0.10000f,0.13000f,0.18000f,1.00000f,0.04000f,0.58000f,0.00000f,0.25000f,0.20000f},18,2,1,2,0,2,0.00000f,1},
{"Bowed Psaltery Mist",{8000.00000f,0.08000f,0.00300f,0.35000f,0.00200f,0.45000f,0.05000f,0.10000f,0.13000f,0.18000f,1.00000f,0.04000f,0.58000f,0.00000f,0.25000f,0.20000f},19,2,1,2,0,2,0.00000f,1},
{"Recorder Stab Rave",{8000.00000f,0.08000f,0.00300f,0.35000f,0.00200f,0.45000f,0.05000f,0.10000f,0.13000f,0.18000f,1.00000f,0.04000f,0.58000f,0.00000f,0.25000f,0.20000f},20,2,1,2,0,2,0.00000f,1}
};
static_assert(std::size(factoryBank)==119);
}
const juce::StringArray& SpaceInfinityProcessor::presetNames(){
    static const auto names=[]{
        juce::StringArray result;
        for(const auto& patch:factoryBank)result.add(patch.name);
        return result;
    }();
    return names;
}
juce::String SpaceInfinityProcessor::presetCategory(int index){
    static constexpr const char* categories[]={
"Original",
"Original",
"Original",
"Original",
"Original",
"Original",
"Choirs",
"Choirs",
"Original",
"Original",
"Original",
"Original",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Leads",
"Offbeat Bass",
"Offbeat Bass",
"Offbeat Bass",
"Offbeat Bass",
"Offbeat Bass",
"Rolling Bass",
"Rolling Bass",
"Rolling Bass",
"Rolling Bass",
"Rolling Bass",
"Choirs",
"Choirs",
"Choirs",
"Choirs",
"Choirs",
"Choirs",
"Choirs",
"Choirs",
"Choirs",
"Choirs",
"Dark Pads",
"Dark Pads",
"Dark Pads",
"Dark Pads",
"Dark Pads",
"Dark Pads",
"Dark Pads",
"Dark Pads",
"Dark Pads",
"Dark Pads",
"Light Pads",
"Light Pads",
"Light Pads",
"Light Pads",
"Light Pads",
"Light Pads",
"Light Pads",
"Light Pads",
"Light Pads",
"Light Pads",
"Trance Tools",
"Trance Tools",
"Trance Tools",
"Trance Tools",
"Trance Tools",
"Trance Tools",
"Trance Tools",
"Trance Tools",
"Trance Tools",
"Trance Tools",
"Trance Tools",
"Sampled Choirs",
"Sampled Choirs",
"Sampled Choirs",
"Sampled Choirs",
"Sampled Choirs",
"Sampled Choirs",
"Sampled Choirs",
"Sampled Choirs",
"Sampled Instruments",
"Sampled Instruments",
"Sampled Instruments",
"Sampled Instruments",
"Sampled Instruments",
"Sampled Instruments",
"Sampled Instruments",
"Sampled Instruments"
    };
    static_assert(std::size(categories)==std::size(factoryBank));
    return categories[index];
}
void SpaceInfinityProcessor::loadPreset(int index){
    if(index<0||index>=int(std::size(factoryBank)))return;
    const auto& patch=factoryBank[index];
    // New engines are opt-in when selecting the existing factory bank.
    for(const auto* id:{"filterEnvAmount","patternMode","convolverEnabled","wavetableScan"})
        state.getParameter(id)->setValueNotifyingHost(state.getParameter(id)->convertTo0to1(0));
    const auto set=[this](const char* id,float value){
        auto* parameter=state.getParameter(id);
        parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
    };
    constexpr const char* ids[]={"cutoff","res","attack","release","detune","mix","drive",
        "chorus","delay","reverb","lforate","lfodepth","gain","wave","ampDecay","ampSustain"};
    for(size_t i=0;i<std::size(ids);++i)
        set(ids[i],i==2||i==3?juce::jlimit(0.0f,1.0f,patch.values[i]):patch.values[i]);
    set("source",float(patch.source));
    set("ampTimed",1.0f);
    set("flanger",0.0f);
    set("phaser",0.0f);
    for(const auto& control:effectControls)
        if(control.added)set(control.id,control.initial);
    for(const auto* id:effectEnableIds)set(id,1.0f);
    set("reverbSize",.72f);
    set("reverbDamping",.45f);
    set("eqEnabled",0.0f);
    for(int band=0;band<7;++band){
        const auto reset=[this,band](const char* field,float value){
            auto* parameter=state.getParameter(eqId(band,field));
            parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
        };
        reset("Frequency",eqFrequencies[size_t(band)]);
        reset("Gain",0.0f);
        reset("Q",1.0f);
    }
    set("noiseLevel",0.0f);
    set("noiseType",0.0f);
    for(const auto* id:{"masterOctave","osc1Octave","osc2Octave"})set(id,0.0f);
    for(const auto* id:{"osc1Warp","osc2Warp","osc1WarpAmount","osc2WarpAmount"})set(id,0.0f);
    set("osc1Enabled",1.0f);
    set("osc1Unison",1.0f);
    set("osc2Unison",1.0f);
    for(const auto* id:{"osc1Tune","osc2Tune","osc1Width","osc2Width","osc2Wave"})set(id,0.0f);
    set("osc2Enabled",1.0f);
    set("filterMode",float(patch.filterMode));
    set("delaySync",float(patch.division));
    set("delayMode",float(patch.delayMode));
    set("reverbSpace",float(patch.space));
    set("compAmount",0.0f);
    set("compMode",0);set("echoMode",0);set("gateEnabled",0);set("pumpEnabled",0);
    for(int band=0;band<7;++band){set(("eqMode"+juce::String(band)).toRawUTF8(),0);set(("eqSlope"+juce::String(band)).toRawUTF8(),0);}
    set("sampleStart",0.0f);
    set("sampleEnd",1.0f);
    set("sampleLoop",0.0f);
    set("sampleReverse",0.0f);
    set("motionBars",float(patch.motionBars));
    set("motionDepth",patch.motionDepth);
    for(int i=0;i<4;++i){
        state.getParameter("macro"+juce::String(i))->setValueNotifyingHost(0.5f);
        for(int slot=0;slot<3;++slot){
            auto* route=state.getParameter(macroRouteId(i,slot,false));
            route->setValueNotifyingHost(route->convertTo0to1(slot==0?float(i+1):0.0f));
            state.getParameter(macroRouteId(i,slot,true))->setValueNotifyingHost(1.0f);
        }
    }
    const auto category=presetCategory(index);
    if(category=="Sampled Choirs")set("sampleLoop",1);
    const auto name=juce::String(patch.name);
    const bool lead=category=="Leads"||name=="HARD TRANCE LEAD"||name=="REDLINE ANTHEM";
    const bool bass=category.endsWith("Bass")||name=="VOLTAGE BASS";
    const bool pad=category.endsWith("Pads")||name=="INFINITY PAD"||name=="ETHEREAL STRINGS";
    set("analogueCharacter",patch.source!=0?.06f:lead?.26f:bass?.16f:pad?.34f:.12f);
    if(patch.source==0&&(lead||pad)){
        set("osc1Unison",lead?5.0f:3.0f);
        set("osc2Unison",3.0f);
        set("osc1Width",lead?.36f:.48f);
        set("osc2Width",lead?.44f:.55f);
        set("detune",juce::jlimit(.006f,.014f,patch.values[4]));
    }
    if(patch.source==0&&(bass||(lead&&patch.filterMode==1))){
        set("filterMode",5.0f);
        set("cutoff",juce::jlimit(80.0f,16000.0f,patch.values[0]*(lead?1.30f:1.10f)));
    }
    for(int lfo=0;lfo<3;++lfo){
        for(const auto* field:{"Speed","Sync","Mode"}){
            auto* parameter=state.getParameter(lfoParameterId(lfo,field));
            const float value=juce::String(field)=="Mode"?0.0f:1.0f;
            parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
        }
        if(lfo>0){
            auto* bars=state.getParameter(lfoParameterId(lfo,"Bars"));
            bars->setValueNotifyingHost(bars->convertTo0to1(2.0f));
        }
        loadLfoShape(lfo,lfo==0?patch.motionShape:lfo==1?1:0);
    }
    for(int source=4;source<8;++source)
        for(int slot=0;slot<3;++slot){
            state.getParameter(modulationRouteId(source,slot,false))->setValueNotifyingHost(0.0f);
            state.getParameter(modulationRouteId(source,slot,true))->setValueNotifyingHost(.75f);
        }
    const auto& refinement=factoryRefinements[size_t(index)];
    set("analogueUpgrade",1);
    set("analogueCharacter",refinement.character);
    set("osc1Unison",float(refinement.voicesA));set("osc2Unison",float(refinement.voicesB));
    set("osc1Width",refinement.widthA);set("osc2Width",refinement.widthB);set("detune",refinement.detune);
    if(refinement.waveA>=0)set("wave",float(refinement.waveA));
    if(refinement.waveB>=0)set("osc2Wave",float(refinement.waveB+1));
    set("voiceMode",float(refinement.voiceMode));
    set("filterAttack",bass?.001f:pad?.35f:.004f);
    set("filterDecay",refinement.filterDecay);set("filterSustain",refinement.filterSustain);
    set("filterRelease",pad?.9f:bass?.08f:.35f);set("filterEnvAmount",refinement.filterAmount);
    if(bass){set("mix",.22f);set("filterMode",5);set("res",juce::jmin(.58f,patch.values[1]));
        set("ampDecay",category=="Rolling Bass"?.09f:.13f);set("ampSustain",category=="Rolling Bass"?.24f:.36f);
        set("chorus",0);set("reverb",0);set("delay",0);set("gain",.53f);}
    if(lead){set("drive",juce::jmin(.38f,patch.values[6]));set("res",juce::jmin(.70f,patch.values[1]));set("gain",.46f);}
    if(pad){set("gain",.41f);set("drive",juce::jmin(.20f,patch.values[6]));set("attack",juce::jlimit(.25f,1.0f,patch.values[2]));set("release",1.0f);}
    if(refinement.lfoDepth>0){
        loadLfoShape(1,refinement.shape);
        set(lfoParameterId(1,"Bars").toRawUTF8(),3);
        set(modulationRouteId(5,0,false).toRawUTF8(),1);set(modulationRouteId(5,0,true).toRawUTF8(),refinement.lfoDepth);
        set(modulationRouteId(5,1,false).toRawUTF8(),13);set(modulationRouteId(5,1,true).toRawUTF8(),.06f);
    }
    if(name=="Meteor Lift"||name=="Gravity Drop"||name=="Reentry Siren"){
        loadLfoShape(1,name=="Gravity Drop"?4:0);set(lfoParameterId(1,"Mode").toRawUTF8(),2);set(lfoParameterId(1,"Bars").toRawUTF8(),4);
        set(modulationRouteId(5,0,false).toRawUTF8(),1);set(modulationRouteId(5,0,true).toRawUTF8(),.45f);
        set("noiseLevel",name=="Meteor Lift"?.16f:.06f);
    }
    if(name=="Gate Communion"||name=="Strobelight Chord"){
        set("patternMode",2);set("patternRate",2);set("patternGate",name=="Gate Communion"?.7f:.42f);set("sequenceLength",8);
        for(int step=0;step<16;++step){set(("stepPitch"+juce::String(step)).toRawUTF8(),0);set(("stepVelocity"+juce::String(step)).toRawUTF8(),step%2?.72f:.90f);set(("stepOn"+juce::String(step)).toRawUTF8(),step%8==6?0.0f:1.0f);}
    }
    const char* sampledChoirs[]{"Cathedral Strike","Redline Kyrie","Vault Chant","Eclipse Choir","Halo Ooh","White Chapel","Midnight Vespers","Choral Signal"};
    for(int i=0;i<8;++i)if(name==sampledChoirs[i]){set("source",float(5+i));set("sampleLoop",1);set("filterMode",1);set("cutoff",juce::jmax(1800.0f,patch.values[0]));set("gain",.60f);}
    if(name=="Reverse Procession"){set("source",5);set("sampleReverse",1);set("sampleLoop",1);}
    if(name=="Final Benediction"){set("source",10);set("sampleLoop",1);}
    if(name=="Airlock Mallet"){set("source",15);set("cutoff",10000);set("filterEnvAmount",0);}
    if(name=="Frozen Interlude"){set("source",13);set("cutoff",6000);set("filterEnvAmount",.3f);}
    const auto& studio=factoryStudioBank[size_t(index)];
    for(const auto& c:studioControls)set(c.id,c.value);
    set("compMode",float(studio.compressor));set("compThreshold",studio.threshold);set("compRatio",studio.ratio);set("compGain",studio.makeup);set("compMix",studio.compressionMix);
    set("compBelow",studio.compressor==2?1.08f:1.0f);set("compLimit",0);
    set("echoMode",float(studio.delay));set("echoSyncL",4);set("echoSyncR",4);set("echoOffsetL",1);set("echoOffsetR",1.5f);set("echoLink",0);set("echoHQ",1);
    set("echoMix",studio.delayMix);set("delayFeedback",studio.feedback);set("echoFrequency",bass?1800.0f:pad?3400.0f:5000.0f);set("echoBandwidth",.58f);
    set("irPreset",float(studio.impulse));set("convolverEnabled",studio.impulseMix>0?1.0f:0.0f);set("convolverMix",studio.impulseMix);set("irDecay",studio.decay);set("irDamp",studio.damping);
    set("irMinimumPhase",0);set("irReverse",0);set("irSync",0);
    if(studio.impulseMix>0)set("reverb",juce::jmin(.22f,patch.values[9]));
    set("eqEnabled",1);set("eqMode0",4);set("eqSlope0",0);set("eq0Frequency",studio.highpass);set("eq0Q",.7071f);
    set("eqMode6",2);set("eq6Frequency",7000);set("eq6Gain",bass?-.8f:lead?.7f:pad?.4f:0);set("eq6Q",.7071f);
    set("gateEnabled",float(studio.gate));set("gateDepth",studio.gateDepth);set("gateAttack",3);set("gateRelease",22);set("gateRate",2);
    set("pumpEnabled",studio.pump>0?1.0f:0.0f);set("pumpDepth",studio.pump);set("pumpAttack",3);set("pumpRelease",180);set("pumpRate",2);set("sidechainSource",0);
    set("gateSplit",bass?1.0f:0.0f);set("gateLow",180);set("gateHigh",2600);loadGatePreset(studio.gatePattern);
    const int sampleSource=int(state.getRawParameterValue("source")->load());
    if(sampleSource!=0&&!userSample(sampleSource)){
        set("sampleStart",sampleForSource(sampleSource).view.onset);
        if(name=="Frozen Interlude"||name=="Airlock Mallet")set("filterMode",1);
    }
    state.state.setProperty("presetKey",library->keyForFactory(index),nullptr);
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new SpaceInfinityProcessor();}

