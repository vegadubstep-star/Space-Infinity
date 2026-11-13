#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "EffectControls.h"
#include "StudioParameters.h"
#include <BinaryData.h>
#include <FactoryBankData.h>
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
    p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"ampLong",1},"Long Amp Envelope (x3)",false));
    p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"vibratoEnabled",1},"Legacy Vibrato Enabled",true));
    f("ampSustain","Amp Sustain",0.0f,1.0f,1.0f);
    p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"osc1Enabled",1},"Oscillator 1 Enabled",true));
    p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"osc2Enabled",1},"Oscillator 2 Enabled",true));
    p.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"osc1Unison",1},"Oscillator 1 Unison",1,16,1));
    p.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"osc2Unison",1},"Oscillator 2 Unison",1,16,1));
    f("pitchEnvAmount","Pitch Envelope Amount",-24,24,0);f("pitchEnvAttack","Pitch Envelope Attack",0,.5f,.02f);f("pitchEnvDecay","Pitch Envelope Decay",.005f,2,.09f);
    f("pitchEnvVelocity","Velocity to Pitch Envelope",0,1,0);
    f("midiBendRange","MIDI Bend Range",1,24,24);
    for(int oscillator=1;oscillator<=2;++oscillator)p.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"osc"+juce::String(oscillator)+"Semitone",1},"Oscillator "+juce::String(oscillator)+" Semitone",-12,12,0));
    for(int oscillator=1;oscillator<=2;++oscillator){const auto prefix="osc"+juce::String(oscillator);
        p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{prefix+"PhaseReset",1},"Oscillator "+juce::String(oscillator)+" Retrigger Phase",false));
        p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{prefix+"WarpEnvelope",1},"Oscillator "+juce::String(oscillator)+" Warp Envelope",juce::StringArray{"Off","Amp envelope","Filter envelope"},0));}
    f("filterInputDrive","Pre-filter Drive",0,1,0);
    f("filterVelocity","Velocity to Cutoff",0,4,0);f("ampVelocity","Velocity to Volume",0,1,1);f("subLevel","Sub Sine Level",0,1,0);f("gateSwing","Gate Swing",0,.5f,0);f("gateStepWidth","Gate Width",.05f,1,1);f("gateDelay","Gate Delay",0,4,0);f("gateFade","Gate Fade In",0,8,0);p.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"gateStepCount",1},"Gate Length",1,16,16));
    f("glideTime","Legato Glide",0,.5f,0);
    f("osc1Spread","Oscillator 1 Detune Scale",0,2,1);f("osc2Spread","Oscillator 2 Detune Scale",0,2,1);
    f("osc1Blend","Oscillator 1 Unison Blend",0,1,1);f("osc2Blend","Oscillator 2 Unison Blend",0,1,1);
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
            juce::StringArray{"Off",oscillator==1?"FM from B":"FM from A","Mirror","PWM","FM from Sub"},0));
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
    f("gain","Output",0,1.2f,0.65f);f("presetTrim","Preset Loudness Trim",-24,24,0);p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"levelMatch",1},"Filter Level Match",false));p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"engineMode",1},"Overall Engine Mode",juce::StringArray{"Analogue","Digital"},0));
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
        p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{SpaceInfinityProcessor::lfoParameterId(lfo,"Enabled"),1},title+" Enabled",true));
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
    flag("distEnabled","Distortion Enabled",false);choice("distType","Distortion Type",studio::Distortion::names(),0);choice("distFilterPosition","Distortion Filter",{"Off","Pre","Post"},0);f("distDrive","Distortion Drive",0,1,.2f);f("distMix","Distortion Mix",0,1,1);f("distFrequency","Distortion Filter Frequency",20,20000,6000);f("distQ","Distortion Bandwidth",.2f,10,.7071f);f("distFilterMorph","Distortion LP BP HP",0,1,0);
    f("distInput","Distortion Input",-24,36,0);f("distTone","Distortion Tone",500,20000,20000);flag("distHQ","Distortion HQ",false);
    for(int curve=0;curve<2;++curve)for(int i=0;i<9;++i)f(("distShape"+juce::String(curve)+"_"+juce::String(i)).toRawUTF8(),("Distortion shape "+juce::String(curve+1)+" point "+juce::String(i)).toRawUTF8(),-1,1,curve==0?float(i)/4-1:std::tanh((float(i)/4-1)*2)/std::tanh(2.0f));
    f("pitchBend","Pitch Wheel",-24,24,0);
    for(const char* prefix:{"delayWet","reverbWet"})for(int band=0;band<3;++band){const auto id=juce::String(prefix)+(band==0?"Low":band==1?"Mid":"High");f((id+"Freq").toRawUTF8(),(id+" Frequency").toRawUTF8(),20,20000,band==0?200:band==1?1500:6000);f((id+"Gain").toRawUTF8(),(id+" Gain").toRawUTF8(),-18,18,0);f((id+"Q").toRawUTF8(),(id+" Q").toRawUTF8(),.2f,4,band==1?1:.7071f);}
    for(int stage=0;stage<9;++stage)f(("fxOutput"+juce::String(stage)).toRawUTF8(),("FX "+juce::String(stage+1)+" Output").toRawUTF8(),-24,12,0);
    flag("declickEnabled","De-click protection",true);flag("routeA","Filter oscillator A",true);flag("routeB","Filter oscillator B",true);flag("routeSub","Filter sub",false);flag("routeNoise","Filter noise",true);flag("subEnabled","Sub enabled",true);
    menu("subWave","Sub waveform",{"Sine","Triangle","Saw","Pulse","Narrow pulse","Rounded saw"},0);p.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"subOctave",1},"Sub octave",-3,1,-1));
    menu("flangerMode","Flanger algorithm",{"Legacy","Smooth stereo"},0);f("flangerStereo","Flanger Stereo Phase",0,180,90);
    menu("gateMode","Gate editor",{"Step pattern","Curve"},0);menu("gateCurveRate","Gate cycle",{"1/64","1/32","1/16","1/8","1/4","1/2","1 bar","2 bars","4 bars","8 bars","16 bars"},4);
    f("gatePhase","Gate phase",0,1,0);f("gatePWM","Gate pulse width",.1f,.9f,.5f);f("gateSmooth","Gate smoothing",.1f,50,2);flag("gateRetrigger","Gate note retrigger",false);flag("gateNoteGate","Gate follows held notes",false);
    p.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"gateCurveCount",1},"Gate curve point count",2,33,2));
    for(int i=0;i<33;++i){f(("gateCurveX"+juce::String(i)).toRawUTF8(),"Gate point time",0,1,i==1?1:float(i)/32);f(("gateCurveY"+juce::String(i)).toRawUTF8(),"Gate point level",0,1,1);f(("gateCurveBend"+juce::String(i)).toRawUTF8(),"Gate segment curve",-1,1,0);}
    for(int lfo=0;lfo<3;++lfo)for(int slot=0;slot<16;++slot)f(("lfoFlexDepth"+juce::String(lfo)+"_"+juce::String(slot)).toRawUTF8(),"LFO assignment amount",-1,1,.25f);
    f("osc1Gain","Oscillator A Gain",-60,6,0);f("osc2Gain","Oscillator B Gain",-60,6,0);f("osc1Detune","Oscillator A Detune",0,1,.2f);f("osc2Detune","Oscillator B Detune",0,1,.2f);flag("modernUnison","Independent Unison",false);
    for(int macro=0;macro<3;++macro){f(("freeMacro"+juce::String(macro)).toRawUTF8(),("Assignable macro "+juce::String(macro+1)).toRawUTF8(),0,1,0);for(int slot=0;slot<3;++slot)f(("freeMacroDepth"+juce::String(macro)+"_"+juce::String(slot)).toRawUTF8(),"Assignable macro depth",-1,1,1);}
    for(int source=0;source<9;++source)for(int slot=0;slot<16;++slot)if(source>=6||(source<3&&slot>=3))f(SpaceInfinityProcessor::flexibleDepthId(source,slot).toRawUTF8(),"Macro assignment amount",-1,1,1);
    for(int lfo=0;lfo<3;++lfo){
        const auto id=[lfo](const char* field){return SpaceInfinityProcessor::lfoParameterId(lfo,field);};
        flag(id("CurveEnabled").toRawUTF8(),"LFO free point editor",false);
        f(id("Phase").toRawUTF8(),"LFO phase",0,1,0);f(id("Delay").toRawUTF8(),"LFO delay",0,8,0);f(id("Rise").toRawUTF8(),"LFO rise",0,8,0);f(id("Smooth").toRawUTF8(),"LFO smoothing ms",0,1000,0);
        menu(id("Direction").toRawUTF8(),"LFO direction",{"Forward","Reverse","Ping Pong"},0);flag(id("Mono").toRawUTF8(),"LFO mono",false);flag(id("Triplet").toRawUTF8(),"LFO triplet",false);flag(id("Dotted").toRawUTF8(),"LFO dotted",false);flag(id("Host").toRawUTF8(),"LFO host anchor",false);f(id("Loopback").toRawUTF8(),"LFO envelope loopback",-1,1,-1);

        p.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{id("CurveCount"),1},"LFO point count",2,33,17));
        for(int i=0;i<33;++i){f((id("X")+juce::String(i)).toRawUTF8(),"LFO point time",0,1,juce::jmin(1.0f,float(i)/16));f((id("Y")+juce::String(i)).toRawUTF8(),"LFO point level",0,1,juce::jmin(1.0f,float(i)/16));f((id("Bend")+juce::String(i)).toRawUTF8(),"LFO segment curve",-1,1,0);}
    }
    f("osc2WTPos","Oscillator B wavetable position",0,1,0);flag("independentWTPos","Independent wavetable position",false);
    flag("manualUnison","Manual aligned unison controls",false);
    for(int osc=1;osc<=2;++osc){const auto prefix="osc"+juce::String(osc);menu((prefix+"UnisonMode").toRawUTF8(),"Unison mode",{"Linear","Super","Exp","Inv","Random"},0);menu((prefix+"Stack").toRawUTF8(),"Unison stack",{"Off","12 (1x)","12 (2x)","12 (3x)","12+7 (1x)","12+7 (2x)","12+7 (3x)","Center -12","Center -24"},0);f((prefix+"Range").toRawUTF8(),"Unison range cents",0,120,36);f((prefix+"Phase").toRawUTF8(),"Oscillator phase",0,1,0);f((prefix+"Rand").toRawUTF8(),"Oscillator random phase",0,1,1);f((prefix+"WTSpread").toRawUTF8(),"Unison wavetable spread",0,1,0);f((prefix+"WarpSpread").toRawUTF8(),"Unison warp spread",0,1,0);}
    flag("cleanVCO","Clean base saw/pulse VCO",false);flag("analogueLabEnabled","Analogue component controls",false);flag("stereoFilterUpgrade","Matched stereo filter response",false);
    const char* detailIds[]{"analogueOscDrift","analogueThermal","analogueFilterSpread","analogueEnvSpread","analogueVoiceSpread","analoguePulseDrift","analogueVCADrive","analogueAsymmetry","analogueMemory","analogueCoupling"};
    const char* detailNames[]{"VCO wander","Thermal drift","Filter tolerance","Envelope tolerance","Voice tolerance","Pulse-width drift","VCA drive","VCA asymmetry","Circuit memory","AC coupling"};
    for(int i=0;i<10;++i)f(detailIds[i],detailNames[i],0,1,0);
    choice("analogueLabRange","Analogue Lab Range",{"Calibrated","Studio","Vintage"},0);
    f("ampHold","Amp envelope hold",0,8,0);f("filterHold","Filter envelope hold",0,8,0);
    for(const char* prefix:{"amp","filter"})for(const char* phase:{"Attack","Decay","Release"})f((juce::String(prefix)+phase+"Curve").toRawUTF8(),(juce::String(prefix)+" "+phase+" contour").toRawUTF8(),-1,1,0);
    f("osc1Pan","Oscillator A pan",-1,1,0);f("osc2Pan","Oscillator B pan",-1,1,0);f("subPan","Sub pan",-1,1,0);f("noisePan","Noise pan",-1,1,0);f("concertPitch","Concert pitch A4",400,480,440);
    return p;
}
float osc(double x,int wave){ x-=std::floor(x); switch(wave){case 1:return x<0.48?1.0f:-1.0f;case 2:return std::sin(float(juce::MathConstants<double>::twoPi*x));case 3:return float(4*std::abs(x-0.5)-1);default:return float(2*x-1);} }
}
juce::String SpaceInfinityProcessor::eqId(int band,const char* field){
    return "eq"+juce::String(band)+field;
}
juce::StringArray SpaceInfinityProcessor::oscillatorWaves(bool follow){
    juce::StringArray names{"Saw","Pulse","Sine","Triangle","Narrow Pulse","Wide Pulse","Rounded Saw","Folded Sine","Double Sine","Rectified","Vowel Ah","Vowel Oh","Vowel Ee","Reed","Hollow Reed","Brass","Soft Brass","Organ","Drawbar","Glass","Bell","Metal","Sync Rise","Sync Fall","Bent Ramp","Steps","Buzz","Octave Saw","Fifth Stack","Sub Harmonics","Custom Wavetable","303 Saw","303 Square","Acid Pulse","Acid Resonant Saw","Acid Squelch","Acid Bite"};
    if(follow)names.insert(0,"Follow Osc 1");
    return names;
}
float SpaceInfinityProcessor::warpedSample(double position,int shape,int mode,float amount,float modulator){
    if(amount==0.0f||mode==0)return oscillatorSample(position,shape);
    if(mode==1||mode==4)return oscillatorSample(position+modulator*amount*1.5,shape);
    const double phase=position-std::floor(position);
    if(mode==2){
        const double mirror=1.0-std::abs(phase*2.0-1.0);
        return oscillatorSample(phase+(mirror-phase)*amount,shape);
    }
    const double split=0.5+0.45*amount;
    const double mapped=phase<split?0.5*phase/split:0.5+0.5*(phase-split)/(1.0-split);
    return oscillatorSample(mapped,shape);
}
float SpaceInfinityProcessor::oscillatorFrameSample(double phase,int wave,float position){
    wave=juce::jlimit(0,36,wave);if(wave==30)return 0;position=juce::jlimit(0.0f,1.0f,position);
    if(position==0&&wave<30)return oscillatorSample(phase,wave);
    static const auto bank=[]{
        std::vector<float> data(size_t(36*17*512));
        for(int w=0;w<37;++w){if(w==30)continue;const int family=w>30?w-1:w;
            for(int frame=0;frame<17;++frame){float peak=0,mean=0;const float morph=float(frame)/16;auto* table=data.data()+(family*17+frame)*512;
                for(int i=0;i<512;++i){const double x=double(i)/512;float sample=0;
                    if(w<30){const double warped=x+.12*morph*std::sin(x*juce::MathConstants<double>::twoPi);sample=(1-morph*.45f)*oscillatorSample(warped,w)+morph*.45f*oscillatorSample(x*2+(w%3)*.07, (w+1)%30);}
                    else{const int type=w-31;const float duty=type==1?.5f:type==2?.12f+.28f*morph:.35f;
                        for(int h=1;h<=48;++h){float a=1.0f/h;if(type==1)a=h%2?1.0f/h:0;if(type==2)a=std::sin(juce::MathConstants<float>::pi*h*duty)/h;
                            const float centre=2+10*morph;const float peakBand=std::exp(-std::pow((h-centre)/1.7f,2.0f));a*=std::exp(-float(h)*(.012f+.075f*morph));
                            if(type>=3)a*=1+(type-1)*1.2f*peakBand;
                            sample+=a*std::sin(float(x*juce::MathConstants<double>::twoPi*h)+(type==5?h*.17f:0));}
                        if(type==4||type==5)sample=.8f*std::tanh(sample*(1.3f+2*morph))+.2f*morph*std::sin(float(x*juce::MathConstants<double>::twoPi*(type==4?3:5)));}
                    table[i]=sample;mean+=sample/512;}
                for(int i=0;i<512;++i){table[i]-=mean;peak=std::max(peak,std::abs(table[i]));}if(peak>0)for(int i=0;i<512;++i)table[i]/=peak;
            }
        }return data;
    }();
    const int family=wave>30?wave-1:wave;const float f=position*16;const int a=juce::jmin(15,int(f));const double x=(phase-std::floor(phase))*512;const int i=int(x)&511,j=(i+1)&511;
    const auto read=[&](int frame){const auto* table=bank.data()+(family*17+frame)*512;return table[i]+float(x-std::floor(x))*(table[j]-table[i]);};return read(a)+(read(a+1)-read(a))*(f-a);
}
float SpaceInfinityProcessor::oscillatorSample(double phase,int wave){
    if(wave>=31)return oscillatorFrameSample(phase,wave,0);
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
    oscillatorFrameSample(0.0,31,0);
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
        const char* advanced[]{"Phase","Delay","Rise","Smooth","Direction","Mono","Triplet","Dotted","Host","Loopback"};for(int j=0;j<10;++j)designerAdvanced[size_t(lfo)][size_t(j)]=state.getRawParameterValue(lfoParameterId(lfo,advanced[j]));
        designerCurveEnabled[size_t(lfo)]=state.getRawParameterValue(lfoParameterId(lfo,"CurveEnabled"));
        designerCurveCount[size_t(lfo)]=state.getRawParameterValue(lfoParameterId(lfo,"CurveCount"));
        for(int point=0;point<33;++point){const auto n=juce::String(point);designerX[size_t(lfo)][size_t(point)]=state.getRawParameterValue(lfoParameterId(lfo,"X")+n);designerY[size_t(lfo)][size_t(point)]=state.getRawParameterValue(lfoParameterId(lfo,"Y")+n);designerBend[size_t(lfo)][size_t(point)]=state.getRawParameterValue(lfoParameterId(lfo,"Bend")+n);}
        const char* fields[]={"Speed","Sync","Bars","Mode"};
        designerEnabled[size_t(lfo)]=state.getRawParameterValue(lfoParameterId(lfo,"Enabled"));
        for(size_t i=0;i<4;++i)
            designerControls[size_t(lfo)][i]=state.getRawParameterValue(lfoParameterId(lfo,fields[i]));
    }
    decodeSample(juce::MemoryBlock{BinaryData::Orbital_Choir_Ah_Chord_F2_wav,
                  BinaryData::Orbital_Choir_Ah_Chord_F2_wavSize}, choirAh);
    decodeSample(juce::MemoryBlock{BinaryData::Ethereal_Choir_Ooh_C3_wav,
                  BinaryData::Ethereal_Choir_Ooh_C3_wavSize}, choirOoh);
    for(size_t i=0;i<extraSamples.size();++i){int bytes=0;const auto* data=BinaryData::getNamedResource(extraSampleSources[i].resource,bytes);
        if(data&&bytes>0)decodeSample(juce::MemoryBlock(data,size_t(bytes)),extraSamples[i]);}
    for(size_t i=0;i<factorySamples.size();++i){int bytes=0;const auto* data=BinaryData::getNamedResource(factorySampleZones[i].resource,bytes);if(data&&bytes>0)decodeSample(juce::MemoryBlock(data,size_t(bytes)),factorySamples[i],true);}
    juce::StringArray categories,creators,tags;for(int i=0;i<presetNames().size();++i){categories.add(presetCategory(i));creators.add(presetCreator(i));tags.add(presetTags(i));}
    for(int i=0;i<7;++i){eqTypes[size_t(i)]=state.getRawParameterValue("eqMode"+juce::String(i));eqSlopes[size_t(i)]=state.getRawParameterValue("eqSlope"+juce::String(i));}
    for(int i=0;i<16;++i)gateParameters[size_t(i)]=state.getRawParameterValue("gateStep"+juce::String(i));
    library=std::make_unique<PresetLibrary>(presetNames(),categories,presetDirectory,-1,1000,creators,tags);
    initialiseWorkflow();
    startImpulseWorker();
    loadPreset(0);

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
    return {"Off","Cutoff","Detune","Reverb","Attack","Drive","Delay","Chorus","Layer mix","Release","Resonance","Noise","Osc 1 Width","Osc 2 Width","Output","Decay","Sustain","Motion Depth","LFO 1 Speed","Vibrato Depth","Analogue","LFO 2 Speed","LFO 3 Speed","Wavetable Scan","Gate Depth","Pitch","Osc 1 Warp","Osc 2 Warp"};
}
juce::StringArray SpaceInfinityProcessor::motionNames(){
    return {"Slow Rise","Rise and Fall","Rise into Gate","Late Bloom","Long Fall","Soft Pulse",
        "Double Swell","Triple Swell","Terraces","Descending Steps","Peak Hold","Dip and Return",
        "Quick Strike","Reverse Pull","Saw Rise","Saw Fall","Alternating Gates","Syncopated","Wave Train","Final Lift"};
}
studio::EditableCurve SpaceInfinityProcessor::lfoCurve(int lfo) const {
    studio::EditableCurve c;const auto n=size_t(juce::jlimit(0,2,lfo));const bool free=designerCurveEnabled[n]->load()>.5f;
    c.count=free?juce::jlimit(2,33,int(designerCurveCount[n]->load())):17;
    for(int i=0;i<c.count;++i){const auto j=size_t(i);c.x[j]=free?designerX[n][j]->load():float(i)/16;c.y[j]=free?designerY[n][j]->load():designerPoints[n][j]->load();c.bend[j]=free?designerBend[n][j]->load():0;}if(!free){for(int i=1;i<c.count-1;){const float expected=c.y[size_t(i-1)]+(c.y[size_t(i+1)]-c.y[size_t(i-1)])*(c.x[size_t(i)]-c.x[size_t(i-1)])/(c.x[size_t(i+1)]-c.x[size_t(i-1)]);if(std::abs(expected-c.y[size_t(i)])<.000001f){for(int j=i;j<c.count-1;++j){c.x[size_t(j)]=c.x[size_t(j+1)];c.y[size_t(j)]=c.y[size_t(j+1)];}--c.count;}else ++i;}}return c;
}
float SpaceInfinityProcessor::lfoCurveValue(int lfo,double phase) const {return lfoCurve(lfo).value(float(phase));}
void SpaceInfinityProcessor::enableLfoCurve(int lfo){
    const auto id=[lfo](const char* field){return lfoParameterId(lfo,field);};
    if(state.getRawParameterValue(id("CurveEnabled"))->load()>.5f)return;
    const auto set=[this](const juce::String& key,float value){auto* a=state.getParameter(key);a->beginChangeGesture();a->setValueNotifyingHost(a->convertTo0to1(value));a->endChangeGesture();};
    const auto c=lfoCurve(lfo);for(int i=0;i<c.count;++i){const auto n=juce::String(i);set(id("X")+n,c.x[size_t(i)]);set(id("Y")+n,c.y[size_t(i)]);set(id("Bend")+n,0);}set(id("CurveCount"),float(c.count));set(id("CurveEnabled"),1);
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
    state.getParameter(lfoParameterId(lfo,"CurveEnabled"))->setValueNotifyingHost(0);
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
    for(const auto& sample:extraSampleSources)names.add(sample.name);for(const auto* name:factoryInstrumentNames)names.add(name);return names;
}
int SpaceInfinityProcessor::factoryZoneForSource(int source,int note,float velocity)const{
    if(source<21)return -1;int best=-1,distance=1000;for(size_t i=0;i<factorySampleZones.size();++i){const auto& zone=factorySampleZones[i];if(zone.instrument!=source-21||zone.layer!=juce::jmin(zone.layers-1,int(velocity*zone.layers)))continue;const int delta=std::abs(note-zone.root);if(delta<distance){best=int(i);distance=delta;}}return best;
}
int SpaceInfinityProcessor::rootForSource(int source,int zone,int importedRoot)const{return zone>=0?factorySampleZones[size_t(zone)].root:source==1?41:source==2?48:source>=5&&source<21?extraSampleSources[size_t(source-5)].root:importedRoot;}
juce::Result SpaceInfinityProcessor::decodeSample(const juce::MemoryBlock& bytes, Sample& sample,bool keepStereo){
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
    if(keepStereo)sample.audio=std::move(decoded);
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
    levelMatchActive=false;levelMatchEnergy=levelMatchReference=0;levelMatchGain=1;levelMatchSamples=0;
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

    prepareWorkflow(sr);
    eqState={};
    previousOutputGain=param("gain")*juce::Decibels::decibelsToGain(param("presetTrim"));
    analogueAmount.reset(sr,0.04);
    analogueAmount.setCurrentAndTargetValue(param("engineMode")>.5f?0:param("analogueCharacter"));
    analogueCutoff.reset(sr,.02);
    analogueCutoff.setCurrentAndTargetValue(param("cutoff"));
    analogueLabRange.reset(sr,.025);analogueLabRange.setCurrentAndTargetValue(param("engineMode")<.5f&&param("analogueLabEnabled")>.5f?param("analogueLabRange"):0);
    analogueResonance.reset(sr,.02);
    analogueResonance.setCurrentAndTargetValue(param("res"));
    juce::dsp::ProcessSpec spec{sr,(juce::uint32)std::max(maxSamples,1),2};chorus.prepare(spec);chorus.setRate(0.25f);chorus.setDepth(0.38f);chorus.setCentreDelay(9);chorus.setFeedback(0.1f);chorus.setMix(param("chorus"));reverb.setSampleRate(sr);auto wetOnlyParams=reverb.getParameters();reverbDryGain.reset(sr,.01);reverbDryGain.setCurrentAndTargetValue(wetOnlyParams.dryLevel*2);wetOnlyParams.dryLevel=0;reverb.setParameters(wetOnlyParams);reverb.setSampleRate(sr);
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
    distortion.prepare(sr);studioCompressor.prepare(sr);studioDelay.prepare(sr);rhythm.prepare(sr);
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
    designerPhase={};designerAge={};designerMemory={};
    for(auto& phase:displayedLfoPhase)phase.store(0.0f);
}
void SpaceInfinityProcessor::noteOn(int note,float velocity,int source,int root,int channel){
    const int zone=factoryZoneForSource(source,note+int(param("masterOctave"))*12,velocity);
    const Sample* sample=&sampleForSource(source,zone);
    if(source != 0 && sample->audio.getNumSamples() < 2)return;
    auto* found=&voices[0];
    if(voiceMode==0)for(auto& v:voices)if(!v.active){found=&v;break;}
    if(voiceMode==0&&found->active)found=&*std::min_element(voices.begin(),voices.end(),[](const Voice& a,const Voice& b){return a.age<b.age;});
    const bool glideFromPrevious=voiceMode!=0&&found->active&&!found->released&&param("glideTime")>0;const double retainedPitch=found->pitch;
    const auto retainedPhase=found->phase,retainedPhase2=found->phase2;
    const auto retainedExtraA=found->extraPhase1,retainedExtraB=found->extraPhase2;
    const float oldL=(found->active||found->retireRemaining>0)?found->lastL:0,oldR=(found->active||found->retireRemaining>0)?found->lastR:0;
    const auto retainedDrift=found->drift;
    const auto calibration=found->calibration;
    *found={};found->calibration=calibration;
    const bool freeRunning=source==0&&param("analogueUpgrade")>.5f;
    if(freeRunning){found->phase=retainedPhase;found->phase2=retainedPhase2;
        found->extraPhase1=retainedExtraA;found->extraPhase2=retainedExtraB;found->drift=retainedDrift;}
    if(param("osc1PhaseReset")>.5f){found->phase=0;found->extraPhase1.fill(0);}
    if(param("osc2PhaseReset")>.5f){found->phase2=0;found->extraPhase2.fill(0);}
    if(param("declickEnabled")>.5f){found->tailL=oldL;found->tailR=oldR;found->declickLength=found->declickRemaining=juce::jmax(1,int(sr*(param("subLevel")>0?.004:.0015)));}
    found->active=true;found->pitch=glideFromPrevious?retainedPitch:note;found->note=note;found->channel=channel;found->velocity=velocity;found->age=++clock;found->noiseState=uint32_t(found->age)*747796405u+2891336453u;
    if(found->noiseState==0)found->noiseState=1;
    if(!freeRunning)found->drift.reset(found->noiseState^0x9e3779b9u);
    if(!freeRunning&&source==0&&param("engineMode")<.5f&&param("analogueCharacter")>0.0f){
        const auto phase=[found]{return 0.5+0.5*double(found->drift.random());};
        found->phase=phase();
        found->phase2=phase();
        for(auto& position:found->extraPhase1)position=phase();
        for(auto& position:found->extraPhase2)position=phase();
    }
    if(source==0&&param("manualUnison")>.5f){for(int osc=0;osc<2;++osc){const auto prefix="osc"+juce::String(osc+1);if(param((prefix+"PhaseReset").toRawUTF8())>.5f)continue;const double start=param((prefix+"Phase").toRawUTF8()),randomness=param((prefix+"Rand").toRawUTF8());auto phase=[&]{const double x=start+randomness*(.5+.5*found->drift.random());return x-std::floor(x);};(osc==0?found->phase:found->phase2)=phase();auto& extras=osc==0?found->extraPhase1:found->extraPhase2;for(auto& x:extras)x=phase();}}
    found->source=source;found->sampleZone=zone;
    if(source != 0){
        if(userSample(source))found->sampleRevision=sampleRevision;
        const int sampleRoot=rootForSource(source,zone,root);
        found->sampleStep=sample->sampleRate/sr*std::exp2((note-sampleRoot)/12.0);
        const int last=sample->audio.getNumSamples()-1-(zone>=0&&factorySampleZones[size_t(zone)].loopStart>0?int(sample->sampleRate*.1):0);
        found->sampleStart=juce::jlimit(0,last-1,int(param("sampleStart")*float(last)));
        found->sampleEnd=juce::jlimit(found->sampleStart+1,last,int(param("sampleEnd")*float(last)));
        found->sampleLoop=param("sampleLoop")>0.5f;found->sampleLoopStart=zone>=0?factorySampleZones[size_t(zone)].loopStart:0;
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
        voice.velocity=selected->velocity;
        voice.note=note;
        voice.channel=channel;
        return;
    }
    noteOn(note,selected->velocity,source,root,channel);
}
void SpaceInfinityProcessor::retireVoice(Voice& v){v.active=false;if(param("declickEnabled")>.5f){v.tailL=v.lastL;v.tailR=v.lastR;v.retireLength=v.retireRemaining=juce::jmax(1,int(sr*.003));}}
void SpaceInfinityProcessor::noteOff(int note,int channel){for(auto& v:voices) if(v.active&&v.note==note&&v.channel==channel)v.released=true;}
void SpaceInfinityProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer& midi){
    if(b.getNumChannels()<2){b.clear();return;}
    // Process bounded chunks if a host supplies more samples than it announced.
    for(int offset=0;offset<b.getNumSamples();){
        const bool assigned=flexibleAssigned.load(std::memory_order_relaxed);
        const int count=std::min(assigned?juce::jmin(64,maximumHostBlock):maximumHostBlock,b.getNumSamples()-offset);
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
        hostChunkSeconds=double(offset)*2/sr;renderBlock(high,highRateMidi);oversampling.processSamplesDown(output);
        unsigned write=scopeWrite.load(std::memory_order_relaxed);
        for(int i=0;i<count;++i){scopeSamples[size_t(write%32768)].store((b.getSample(0,offset+i)+b.getSample(1,offset+i))*.5f,std::memory_order_relaxed);++write;}
        scopeWrite.store(write,std::memory_order_release);offset+=count;
    }
}
void SpaceInfinityProcessor::renderBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer& midi){
    juce::ScopedNoDenormals noDenormals;
    b.clear(); if(b.getNumChannels()<2)return;
    if(resetVoicesRequested.exchange(false)){levelMatchActive=false;for(auto& v:voices)if(v.active)retireVoice(v);heldNotes={};noteOrder=0;pattern.held={};pattern.activeNote=-1;pattern.index=0;pattern.remaining=0;}
    transformPattern(midi,b.getNumSamples());
    for(const auto event:midi)if(event.samplePosition==0&&event.getMessage().isNoteOn())for(int i=0;i<3;++i)if(designerControls[size_t(i)][3]->load()>.5f){designerPhase[size_t(i)]=0;designerAge[size_t(i)]=0;}
    updateFlexibleRoutes();
    auto* left=b.getWritePointer(0);auto* right=b.getWritePointer(1);
    std::array<float,28> modulation{};
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
    std::array<studio::EditableCurve,3> curves{};
    std::array<double,3> lfoSteps{};
    std::array<studio::LfoTiming,3> timing{};std::array<bool,3> lfoMono{};std::array<float,3> loopback{};
    std::array<int,3> lfoModes{};
    std::array<bool,3> lfoEnabled{};
    for(size_t lfo=0;lfo<3;++lfo){
        curves[lfo]=lfoCurve(int(lfo));
        const auto& control=designerControls[lfo];
        static constexpr double lengths[]{1,2,4,8,.5,.25,.125,.0625};
        const int division=juce::jlimit(0,7,int(readParameter(control[2])));
        lfoSteps[lfo]=control[1]->load()>.5f
            ?delayBpm/(240.0*sr*lengths[division]):readParameter(control[0])/sr;
        lfoModes[lfo]=int(control[3]->load());
        lfoEnabled[lfo]=designerEnabled[lfo]->load()>.5f;
        const auto& advanced=designerAdvanced[lfo];timing[lfo]={readParameter(advanced[0]),readParameter(advanced[1]),readParameter(advanced[2]),readParameter(advanced[3]),int(advanced[4]->load())};lfoMono[lfo]=advanced[5]->load()>.5f;loopback[lfo]=advanced[9]->load();
        if(control[1]->load()>.5f){if(advanced[6]->load()>.5f)lfoSteps[lfo]*=1.5;if(advanced[7]->load()>.5f)lfoSteps[lfo]/=1.5;}
        if(lfoModes[lfo]==0&&advanced[8]->load()>.5f)if(auto* playHead=getPlayHead())if(auto pos=playHead->getPosition()){
            if(control[1]->load()>.5f){if(auto ppq=pos->getPpqPosition()){designerPhase[lfo]=(*ppq+hostChunkSeconds*delayBpm/60)/(lengths[division]*4);if(advanced[6]->load()>.5f)designerPhase[lfo]*=1.5;if(advanced[7]->load()>.5f)designerPhase[lfo]/=1.5;}}
            else if(auto seconds=pos->getTimeInSeconds())designerPhase[lfo]=(*seconds+hostChunkSeconds)*readParameter(control[0]);
            const double cycle=timing[lfo].direction==2?2:1;designerPhase[lfo]=std::fmod(std::max(0.0,designerPhase[lfo]),cycle);
        }

    }
    const auto advanceLfo=[&](size_t lfo,double& phase){
        const double end=timing[lfo].direction==2?2:1;phase+=lfoSteps[lfo];
        if(lfoModes[lfo]==2){if(phase>=end){if(loopback[lfo]>=0&&loopback[lfo]<1)phase=loopback[lfo]+std::fmod(phase-end,end-loopback[lfo]);else phase=end;}}
        else if(phase>=end)phase-=end;
    };
    struct DynamicRoute {size_t target=0;float depth=0.0f;};
    std::array<std::array<DynamicRoute,3>,4> timeRoutes{};
    for(size_t source=0;source<4;++source)
        for(size_t slot=0;slot<3;++slot)
            timeRoutes[source][slot]={size_t(juce::jlimit(0,27,int(macroRoutes[source+4][slot]->load()))),
                macroDepths[source+4][slot]->load()};
    const auto routeDynamic=[&timeRoutes](auto& values,size_t source,float value){
        for(const auto& route:timeRoutes[source])
            if(route.target>0)values[route.target]+=route.depth*(route.target==25&&source<3?value*2-1:value);
    };
    auto fxModulation=modulation;
    for(size_t lfo=0;lfo<3;++lfo)if(lfoEnabled[lfo]){float memory=designerMemory[lfo];const float value=designerAge[lfo]>0?memory:timing[lfo].output(curves[lfo],designerPhase[lfo],0,sr,memory);routeDynamic(fxModulation,lfo,value);}
    float currentEnvelope=0.0f;
    for(const auto& voice:voices)if(voice.active)currentEnvelope=std::max(currentEnvelope,voice.env);
    routeDynamic(fxModulation,3,currentEnvelope);
    const int speedTargets[]{18,21,22};
    for(size_t lfo=0;lfo<3;++lfo)
        lfoSteps[lfo]*=std::exp2(fxModulation[size_t(speedTargets[lfo])]*2.0f);

    const bool declick=param("declickEnabled")>.5f;
    const float ampScale=param("ampLong")>.5f?3.0f:1.0f;
    const float attack=juce::jlimit(0.0f,1.0f,param("attack")*std::exp2(modulation[4]*3))*ampScale;
    const float release=juce::jlimit(0.0f,1.0f,param("release")*std::exp2(modulation[9]*2))*ampScale;
    const float mix=juce::jlimit(0.0f,1.0f,param("mix")+0.4f*modulation[8]);
    const float detune=juce::jlimit(0.0f,0.04f,param("detune")+0.02f*modulation[2]);
    const float cutoff=param("cutoff")*std::exp2(modulation[1]*2);
    const float res=juce::jlimit(0.0f,0.95f,param("res")+0.4f*modulation[10]);
    const float lfoRate=juce::jlimit(0.1f,12.0f,param("lforate")*std::exp2(modulation[18]*2));
    const float lfoDepth=param("vibratoEnabled")>.5f?juce::jlimit(0.0f,1.0f,param("lfodepth")+0.5f*modulation[19]):0;
    const float drive=juce::jlimit(0.0f,1.0f,param("drive")+0.45f*modulation[5]);
    const bool timedEnvelope=param("ampTimed")>.5f;
    const bool upgraded=param("analogueUpgrade")>.5f;
    const float sustain=juce::jlimit(0.0f,1.0f,param("ampSustain")+0.5f*modulation[16]);
    const float attackStep=float(1.0/std::max(1.0,double(attack)*sr));
    const float legacyAttackStep=attack==0.0f?1.0f:float(1.0-std::exp(-double(attackStep)));
    const float decayStep=float(1.0/std::max(1.0,juce::jlimit(0.005,8.0,double(param("ampDecay"))*std::exp2(double(modulation[15])*2))*sr));
    const float releaseFactor=(release==0&&!declick)?0:float(std::exp(-1.0/(double(declick?juce::jmax(.006f,release):release)*sr)));
    const bool osc1Enabled=param("osc1Enabled")>.5f,osc2Enabled=param("osc2Enabled")>.5f;
    const int wave=int(param("wave"));
    const int waveChoice=int(param("osc2Wave"));
    const int secondWave=waveChoice==0?wave:waveChoice-1;
    const int warpEnvelope1=int(param("osc1WarpEnvelope")),warpEnvelope2=int(param("osc2WarpEnvelope"));
    const int warp1=int(param("osc1Warp")),warp2=int(param("osc2Warp"));
    const float warpAmount1=juce::jlimit(0.0f,1.0f,param("osc1WarpAmount")+.5f*modulation[26]),warpAmount2=juce::jlimit(0.0f,1.0f,param("osc2WarpAmount")+.5f*modulation[27]);
    const double tune1=std::exp2(param("osc1Octave")+param("osc1Semitone")/12.0+param("osc1Tune")/1200.0);
    const double tune2=std::exp2(param("osc2Octave")+param("osc2Semitone")/12.0+param("osc2Tune")/1200.0);
    const int unison1=int(param("osc1Unison")),unison2=int(param("osc2Unison"));
    const float oscGainA=juce::Decibels::decibelsToGain(param("osc1Gain")),oscGainB=juce::Decibels::decibelsToGain(param("osc2Gain"));
    std::array<double,128> unisonRates1{},unisonRates2{};
    const bool manualUnison=param("manualUnison")>.5f;std::array<infinity::UnisonSettings,2> unisonSettings;for(int i=0;i<2;++i){const auto prefix="osc"+juce::String(i+1);auto& u=unisonSettings[size_t(i)];u.mode=int(param((prefix+"UnisonMode").toRawUTF8()));u.stack=int(param((prefix+"Stack").toRawUTF8()));u.amount=param((prefix+"Detune").toRawUTF8());u.range=param((prefix+"Range").toRawUTF8());u.blend=param((prefix+"Blend").toRawUTF8());}
    const float wtSpread[]{param("osc1WTSpread"),param("osc2WTSpread")},warpSpread[]{param("osc1WarpSpread"),param("osc2WarpSpread")};
    const bool modernUnison=param("modernUnison")>.5f;
    const float detuneA=modernUnison?param("osc1Detune")*.04f:detune*param("osc1Spread"),detuneB=modernUnison?param("osc2Detune")*.04f:detune*param("osc2Spread");
    const float spreadA=modernUnison?1:param("osc1Spread"),spreadB=modernUnison?1:param("osc2Spread"),blendA=param("osc1Blend"),blendB=param("osc2Blend");
    const float pitchVelocity=param("pitchEnvVelocity"),pitchAmount=param("pitchEnvAmount"),pitchAttack=param("pitchEnvAttack"),pitchDecay=param("pitchEnvDecay");
    const double glide=param("glideTime")>0?std::exp(-1.0/(param("glideTime")*sr)):0;
    const auto prepareStack=[&](auto& rates,int count,float amount,int oscillator){
        if(manualUnison){const auto& u=unisonSettings[size_t(oscillator)];for(int group=0;group<u.groups();++group)for(int i=0;i<count;++i)rates[size_t(group*count+i)]=u.ratio(i,count,group);return;}
        for(int i=0;i<count;++i){
            const double spread=count==1?0.0:2.0*i/(count-1)-1.0;
            rates[size_t(i)]=std::exp2(spread*amount*0.75);
        }
    };
    prepareStack(unisonRates1,unison1,detuneA,0);
    prepareStack(unisonRates2,unison2,detuneB,1);
    const float noiseLevel=juce::jlimit(0.0f,1.0f,param("noiseLevel")+0.5f*modulation[11]);
    const int noiseType=int(param("noiseType"));
    const double subRatio=std::exp2(param("subOctave"));static constexpr int subShapes[]{2,3,0,1,4,6};const int subShape=subShapes[juce::jlimit(0,5,int(param("subWave")))];
    const float cutoffSmoothing=float(1-std::exp(-1/(sr*.0015)));
    const float width1=juce::jlimit(0.0f,1.0f,param("osc1Width")+0.5f*modulation[12]);
    const float width2=juce::jlimit(0.0f,1.0f,param("osc2Width")+0.5f*modulation[13]);
    const float filterA=param("filterAttack"),filterD=param("filterDecay"),filterS=param("filterSustain"),filterR=param("filterRelease");
    const bool independentWTPos=param("independentWTPos")>.5f;const float tableScanB=param("osc2WTPos");
    const float filterAmount=param("filterEnvAmount"),tableScan=juce::jlimit(0.0f,1.0f,param("wavetableScan")+modulation[23]);
    const float preFilterDrive=param("filterInputDrive");
    const float filterVelocity=param("filterVelocity"),ampVelocity=param("ampVelocity"),subLevel=param("subEnabled")>.5f?param("subLevel"):0;
    const bool routeA=param("routeA")>.5f,routeB=param("routeB")>.5f,routeSub=param("routeSub")>.5f,routeNoise=param("routeNoise")>.5f;
    const float grainPosition=param("grainPosition"),grainSize=param("grainSize"),grainDensity=param("grainDensity"),grainSpray=param("grainSpray"),grainSpread=param("grainSpread");
    const float gainTargets[]{osc1Enabled?oscGainA:0,osc2Enabled?oscGainB:0,mix,noiseLevel,subLevel,float(routeA),float(routeB),float(routeSub),float(routeNoise),res};for(int i=0;i<10;++i)liveGains[size_t(i)].setTargetValue(gainTargets[i]);
    const bool grainReverse=param("sampleReverse")>.5f;
    const int filterMode=int(param("filterMode"));
    const bool cleanVco=param("cleanVCO")>.5f;
    const bool digital=param("engineMode")>.5f;
    const bool stereoFilters=param("stereoFilterUpgrade")>.5f,analogueLab=!digital&&param("analogueLabEnabled")>.5f;
    const char* detailIds[]{"analogueOscDrift","analogueThermal","analogueFilterSpread","analogueEnvSpread","analogueVoiceSpread","analoguePulseDrift","analogueVCADrive","analogueAsymmetry","analogueMemory","analogueCoupling"};
    analogueLabRange.setTargetValue(analogueLab?param("analogueLabRange"):0);
    for(int i=0;i<10;++i)analogueDetails[size_t(i)].setTargetValue(analogueLab?param(detailIds[i]):0);
    const char* panIds[]{"osc1Pan","osc2Pan","subPan","noisePan"};for(int i=0;i<4;++i)sourcePans[size_t(i)].setTargetValue(param(panIds[i]));
    const float ampHold=param("ampHold"),filterHold=param("filterHold"),concertPitch=param("concertPitch");
    const std::array<float,3> ampCurves{param("ampAttackCurve"),param("ampDecayCurve"),param("ampReleaseCurve")},filterCurves{param("filterAttackCurve"),param("filterDecayCurve"),param("filterReleaseCurve")};
    const bool ampContour=std::any_of(ampCurves.begin(),ampCurves.end(),[](float x){return x!=0;}),filterContour=std::any_of(filterCurves.begin(),filterCurves.end(),[](float x){return x!=0;});
    const float thermalSlew=float(1-std::exp(-1/(sr*8)));const int thermalInterval=juce::jmax(1,int(sr*.67));

    const int source=int(param("source")), root=int(param("sampleRoot"));
    analogueAmount.setTargetValue(digital?0:juce::jlimit(0.0f,1.0f,param("analogueCharacter")+0.5f*fxModulation[20]));
    analogueCutoff.setTargetValue(juce::jlimit(50.0f,float(sr*.4),cutoff));
    analogueResonance.setTargetValue(res);
    const float dcCoefficient=float(std::exp(-juce::MathConstants<double>::twoPi*10.0/sr));
    const float driftSlew=float(1.0-std::exp(-1.0/(sr*0.18)));
    const int driftInterval=juce::jmax(1,int(sr*0.043));
    const float hostBend=param("pitchBend");
    pitchBend.setTargetValue(juce::jlimit(-24.0f,24.0f,hostBend+midiBend));
    const juce::SpinLock::ScopedTryLockType sampleGuard(sampleLock);
    for(auto& voice:voices)if(voice.active&&voice.source!=0&&(!userSample(voice.source)||sampleGuard.isLocked())){
        const auto& sample=sampleForSource(voice.source,voice.sampleZone);const int last=sample.audio.getNumSamples()-1-(voice.sampleZone>=0&&voice.sampleLoopStart>0?int(sample.sampleRate*.1):0);if(last<2)continue;
        voice.sampleStart=juce::jlimit(0,last-1,int(param("sampleStart")*last));voice.sampleEnd=juce::jlimit(voice.sampleStart+1,last,int(param("sampleEnd")*last));voice.sampleLoop=param("sampleLoop")>.5f;
        const int root=rootForSource(voice.source,voice.sampleZone,int(param("sampleRoot")));
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
                midiBend=param("midiBendRange")*float(wheel)/(wheel<0?8192.0f:8191.0f);
                pitchBend.setTargetValue(juce::jlimit(-24.0f,24.0f,hostBend+midiBend));
            }else if(m.isNoteOn()){
                if(userSample(source)&&!sampleGuard.isLocked())continue;
                for(size_t lfo=0;lfo<3;++lfo)if(lfoModes[lfo]!=0){designerPhase[lfo]=0.0;designerAge[lfo]=0;}
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
                    if(m.isAllSoundOff()){voice.active=false;voice.retireRemaining=0;}
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
        std::array<float,28> movingModulation{};
        for(size_t lfo=0;lfo<3;++lfo){
            if(!lfoEnabled[lfo])continue;
            lfoValues[lfo]=timing[lfo].output(curves[lfo],designerPhase[lfo],designerAge[lfo],sr,designerMemory[lfo]);designerAge[lfo]+=1/sr;
            advanceLfo(lfo,designerPhase[lfo]);
            displayedLfoPhase[lfo].store(timing[lfo].position(designerPhase[lfo]),std::memory_order_relaxed);
        }
        const float movement=lfoValues[0];
        float out=0, side=0;const float lfo=std::sin(float(juce::MathConstants<double>::twoPi*lfoPhase));
        std::array<float,10> live{};for(int i=0;i<10;++i){const float next=liveGains[size_t(i)].getNextValue();live[size_t(i)]=declick?next:gainTargets[i];}
        const float character=analogueAmount.getNextValue();
        std::array<float,10> detail{};for(int i=0;i<10;++i)detail[size_t(i)]=analogueDetails[size_t(i)].getNextValue();
        std::array<float,4> pan{};for(int i=0;i<4;++i)pan[size_t(i)]=sourcePans[size_t(i)].getNextValue();
        const auto panSignal=[](float& mid,float& side,float position){if(position==0)return;const float angle=(position+1)*juce::MathConstants<float>::pi*.25f;const float l=(mid+side)*(position>=1?0:std::cos(angle)*1.41421356237f),r=(mid-side)*(position<=-1?0:std::sin(angle)*1.41421356237f);mid=(l+r)*.5f;side=(l-r)*.5f;};
        const auto board=detail[1]>0?boardDrift.next(thermalSlew,thermalInterval):std::array<float,2>{};
        const float range=analogueLabRange.getNextValue(),voltageBlend=juce::jlimit(0.0f,1.0f,range);
        const auto ranged=[range](float calibrated,float studio,float vintage){return range<=1?calibrated+(studio-calibrated)*range:studio+(vintage-studio)*(range-1);};
        const float wanderScale=ranged(7,18,35),thermalScale=ranged(3,8,18),voicePitchScale=ranged(3,12,22),filterSpreadScale=ranged(.12f,1,1.8f),envSpreadScale=ranged(.2f,.8f,1.5f),voiceLevelScale=ranged(.08f,.35f,.55f),pulseDriftScale=ranged(.035f,.16f,.28f);
        const auto colour=AnalogueColourSettings::make(detail[6],detail[7],detail[8],detail[9],sr);
        const auto voltageColour=AnalogueColourSettings::voltage(detail[6],detail[7],detail[8],detail[9],sr,range);

        const float circuitDrive=1.0f+character*3.0f;
        const float warmthDrive=1.0f+character*1.8f;
        const float warmthBias=character*0.07f;
        const float warmthCentre=std::tanh(warmthBias);
        const float warmthScale=1.0f/std::tanh(warmthDrive);
        const double bendRatio=std::exp2(masterOctave+pitchBend.getNextValue()/12.0);
        const float circuitCutoff=analogueCutoff.getNextValue();
        const float circuitResonance=analogueResonance.getNextValue();
        const float fc=juce::jlimit(50.0f,float(sr*0.4),(filterMode==5?circuitCutoff:cutoff)*std::exp2(lfo*lfoDepth*2+movement*motionDepth*3));
        for(auto& v:voices){if(v.retireRemaining>0){const float t=float(v.retireRemaining)/v.retireLength;const float fade=.5f-.5f*std::cos(juce::MathConstants<float>::pi*t);v.lastL=v.tailL*fade;v.lastR=v.tailR*fade;out+=(v.lastL+v.lastR)*.5f;side+=(v.lastL-v.lastR)*.5f;--v.retireRemaining;continue;}if(!v.active){
            if(upgraded){const auto advance=[](double& phase,double rate){phase+=rate;phase-=std::floor(phase);};
                advance(v.phase,v.idleRateA);advance(v.phase2,v.idleRateB);
                for(int i=0;i<(manualUnison?unison1*unisonSettings[0].groups()-1:15);++i)advance(v.extraPhase1[size_t(i)],v.idleRateA);
                for(int i=0;i<(manualUnison?unison2*unisonSettings[1].groups()-1:15);++i)advance(v.extraPhase2[size_t(i)],v.idleRateB);}
            continue;
        }
            if(userSample(v.source) && !sampleGuard.isLocked())continue;
            if(userSample(v.source) && v.sampleRevision!=sampleRevision){retireVoice(v);out+=(v.lastL+v.lastR)*.5f;side+=(v.lastL-v.lastR)*.5f;continue;}
            movingModulation={};
            for(size_t lfo=0;lfo<3;++lfo){
                if(!lfoEnabled[lfo])continue;
                float value=lfoValues[lfo];
                if(lfoModes[lfo]!=0&&!lfoMono[lfo]){
                    value=timing[lfo].output(curves[lfo],v.lfoPhase[lfo],v.lfoAge[lfo],sr,v.lfoMemory[lfo]);v.lfoAge[lfo]+=1/sr;advanceLfo(lfo,v.lfoPhase[lfo]);
                }
                routeDynamic(movingModulation,lfo,value);
            }
            routeDynamic(movingModulation,3,v.env);
            const float voiceAttack=juce::jlimit(0.0f,ampScale,attack*std::exp2(movingModulation[4]*3));
            const float voiceRelease=(declick?juce::jmax(subLevel>0?.006f:.0015f,juce::jlimit(0.0f,ampScale,release*std::exp2(movingModulation[9]*2))):juce::jlimit(0.0f,ampScale,release*std::exp2(movingModulation[9]*2)));
            const float voiceAttackStep=movingModulation[4]==0.0f?attackStep:float(1.0/std::max(1.0,double(voiceAttack)*sr));
            const float voiceLegacyAttack=movingModulation[4]==0.0f?legacyAttackStep:
                voiceAttack==0.0f?1.0f:float(1.0-std::exp(-double(voiceAttackStep)));
            const float voiceReleaseFactor=movingModulation[9]==0.0f?releaseFactor:
                voiceRelease==0.0f?0.0f:float(std::exp(-1.0/(voiceRelease*sr)));
            const float voiceSustain=juce::jlimit(0.0f,1.0f,sustain+.5f*movingModulation[16]);
            const float voiceDecayStep=decayStep/std::exp2(movingModulation[15]*2);
            const float timing=std::exp2(v.calibration.timing*((upgraded?character*.10f:0)+detail[3]*envSpreadScale));
            const float curveBase=upgraded?0:-.5f;
            const float filterEnv=(upgraded||filterHold>0||filterContour)?v.curvedFilter.next(v.released,sr,juce::jmin(1.0f,filterA*timing),filterD*timing,filterS,juce::jmin(1.0f,filterR*timing),filterHold*timing,filterCurves[0]+curveBase,filterCurves[1]+curveBase,filterCurves[2]+curveBase):
                v.filterEnvelope.next(v.released,sr,filterA,filterD,filterS,filterR);
            const float targetVoiceFc=juce::jlimit(50.0f,float(sr*.4),fc*std::exp2(movingModulation[1]*2+filterEnv*filterAmount+(v.velocity-.5f)*filterVelocity+(upgraded?v.calibration.cutoff*character*.09f:0)+v.calibration.cutoff*detail[2]*filterSpreadScale));
            if(v.smoothFc<=0)v.smoothFc=targetVoiceFc;else v.smoothFc+=(declick?cutoffSmoothing:1)*(targetVoiceFc-v.smoothFc);
            const float voiceFc=v.smoothFc;
            const float voiceG=std::tan(float(juce::MathConstants<double>::pi*voiceFc/sr));
            const float voiceOnePole=1.0f-std::exp(float(-juce::MathConstants<double>::twoPi*voiceFc/sr));
            const float voiceRes=juce::jlimit(0.0f,.95f,live[9]+.4f*movingModulation[10]);
            const float voiceDetune=juce::jlimit(0.0f,.04f,detune+.02f*movingModulation[2]);
            const float voiceMix=juce::jlimit(0.0f,1.0f,live[2]+.4f*movingModulation[8]);
            const float voiceGain1=(1.0f-voiceMix)*live[0];
            const float voiceGain2=voiceMix*live[1];
            const float voiceWidth1=juce::jlimit(0.0f,1.0f,width1+.5f*movingModulation[12]);
            const float voiceWidth2=juce::jlimit(0.0f,1.0f,width2+.5f*movingModulation[13]);

            if((timedEnvelope||ampHold>0||ampContour)&&(upgraded||ampHold>0||ampContour)){
                v.env=v.curvedAmp.next(v.released,sr,juce::jmin(ampScale,voiceAttack*timing),juce::jlimit(.005f,8.0f,float(1.0/(voiceDecayStep*sr))*timing),voiceSustain,juce::jmin(ampScale,voiceRelease*timing),ampHold*timing,ampCurves[0]+curveBase,ampCurves[1]+curveBase,ampCurves[2]+curveBase);
            }else if(timedEnvelope){
                advanceTimedEnvelope(v.env,v.decaying,v.releaseStart,v.released,voiceAttackStep,
                    voiceDecayStep,voiceSustain,float(voiceRelease*sr));
            }else if(v.released)v.env*=voiceReleaseFactor;
            else if(voiceSustain>=1.0f&&!v.decaying)v.env+=(1-v.env)*voiceLegacyAttack;
            else if(!v.decaying){
                v.env=std::min(1.0f,v.env+voiceAttackStep);
                v.decaying=v.env>=1.0f;
            }else v.env+=juce::jlimit(-voiceDecayStep,voiceDecayStep,voiceSustain-v.env);
            if(v.released&&v.env<0.0001f){retireVoice(v);out+=(v.lastL+v.lastR)*.5f;side+=(v.lastL-v.lastR)*.5f;continue;}
            v.pitch=v.note+glide*(v.pitch-v.note);
            const double sampleGlide=std::exp2((v.pitch-v.note)/12.0);
            float raw=0, rawSide=0,rawSub=0,bypass=0,bypassSide=0;
            if(v.source==4){
                const auto grains=v.granular.next(importedSample.audio.getReadPointer(0),v.sampleStart,v.sampleEnd,
                    std::abs(v.sampleStep)*sampleGlide*bendRatio*(concertPitch/440.0),sr,grainPosition,grainSize,grainDensity,grainSpray,grainSpread,grainReverse);
                raw=(grains[0]+grains[1])*.5f;rawSide=(grains[0]-grains[1])*.5f;
            }else if(v.source!=0){
                const auto& sample=sampleForSource(v.source,v.sampleZone);
                const int fade=juce::jmin(int(sample.sampleRate*.025),(v.sampleEnd-v.sampleStart)/4);
                const bool smoothLoop=v.sampleLoop&&v.sampleLoopStart>0&&v.sampleStep>0&&fade>2;
                const int loopStart=juce::jlimit(v.sampleStart,juce::jmax(v.sampleStart,v.sampleEnd-2*fade),v.sampleLoopStart);
                if(v.samplePosition<v.sampleStart||v.samplePosition>v.sampleEnd){
                    if(!v.sampleLoop){retireVoice(v);out+=(v.lastL+v.lastR)*.5f;side+=(v.lastL-v.lastR)*.5f;continue;}
                    const double start=smoothLoop?loopStart+fade:v.sampleStart,length=double(v.sampleEnd)-start;
                    const double offset=std::fmod(v.samplePosition-start,juce::jmax(1.0,length));v.samplePosition=start+(offset<0?offset+length:offset);
                }
                const auto read=[&](int channel,double position){const int index=juce::jlimit(v.sampleStart,v.sampleEnd-1,int(position));const auto* data=sample.audio.getReadPointer(juce::jmin(channel,sample.audio.getNumChannels()-1));return data[index]+float(position-index)*(data[index+1]-data[index]);};
                float left=read(0,v.samplePosition),right=read(1,v.samplePosition);
                if(smoothLoop&&v.samplePosition>v.sampleEnd-fade){const float t=float((v.samplePosition-v.sampleEnd+fade)/fade);const double head=loopStart+v.samplePosition-v.sampleEnd+fade;left+=(read(0,head)-left)*t;right+=(read(1,head)-right)*t;}
                raw=(left+right)*.5f;rawSide=(left-right)*.5f;
                if(declick&&v.sampleLoop&&!smoothLoop){const float edge=juce::jlimit(1.0f,float(v.sampleEnd-v.sampleStart)/4,float(sample.sampleRate*.0015));const float distance=float(std::min(v.samplePosition-v.sampleStart,double(v.sampleEnd)-v.samplePosition));if(distance<edge){const float smooth=.5f-.5f*std::cos(juce::MathConstants<float>::pi*juce::jlimit(0.0f,1.0f,distance/edge));raw*=smooth;rawSide*=smooth;}}
                v.samplePosition+=v.sampleStep*sampleGlide*bendRatio*(concertPitch/440.0);
            }else{
            const auto drift=(character>0.0f||detail[0]>0||detail[5]>0)?v.drift.next(driftSlew,driftInterval):std::array<float,2>{};
            const double driftA=std::exp2(double(character)*(double(drift[0])*2.5+(upgraded?v.calibration.tuneA*2:0))/1200+(double(drift[0])*detail[0]*wanderScale+board[0]*detail[1]*thermalScale+v.calibration.tuneA*detail[4]*voicePitchScale)/1200);
            const double driftB=std::exp2(double(character)*(double(drift[1])*2.5+(upgraded?v.calibration.tuneB*2:0))/1200+(double(drift[1])*detail[0]*wanderScale+(board[0]*.8+board[1]*.2)*detail[1]*thermalScale+v.calibration.tuneB*detail[4]*voicePitchScale)/1200);
            const float pitchOffset=pitchAmount!=0?pitchAmount*(1-pitchVelocity+pitchVelocity*v.velocity)*v.pitchEnvelope.next(v.released,sr,pitchAttack,pitchDecay,0,.03f):0;
            const double freq=concertPitch*bendRatio*std::pow(2.0,(v.pitch+pitchOffset+4*(modulation[25]+movingModulation[25])-69.0)/12.0);
            const auto readWave=[&](double phase,int shape,float scan){if(shape==1&&detail[5]>0){const double x=phase-std::floor(phase),split=.48+detail[5]*drift[1]*pulseDriftScale;phase=x<split?.48*x/split:.48+.52*(x-split)/(1-split);}return shape==30?(sampleGuard.isLocked()?wavetable.sample(phase,scan):0.0f):oscillatorFrameSample(phase,shape,scan);};
            const float scanA=juce::jlimit(0.0f,1.0f,tableScan+movingModulation[23]),scanB=independentWTPos?tableScanB:scanA;
            const auto readWarp=[&](double phase,int shape,int mode,float amount,float mod,float scan){
                if(mode==1||mode==4)phase+=mod*amount*1.5;
                else if(mode==2){const double t=phase-std::floor(phase);phase=t+((1-std::abs(t*2-1))-t)*amount;}
                else if(mode==3){const double t=phase-std::floor(phase),split=.5+.45*amount;phase=t<split?.5*t/split:.5+.5*(t-split)/(1-split);}
                return readWave(phase,shape,scan);
            };
            float subModulator=0;
            if(live[4]>0||warp1==4||warp2==4){v.subPhase+=freq*subRatio/sr;v.subPhase-=std::floor(v.subPhase);subModulator=oscillatorSample(v.subPhase,subShape);if(cleanVco){const double increment=freq*subRatio/sr;if(subShape==0)subModulator-=analoguePolyBlep(v.subPhase,increment);if(subShape==1)subModulator+=analoguePolyBlep(v.subPhase,increment)-analoguePolyBlep(v.subPhase-.48,increment);if(std::abs(increment)>=.5)subModulator=0;}v.subEnvelope+=(declick?cutoffSmoothing:1)*((v.released?0.0f:1.0f)-v.subEnvelope);rawSub=live[4]*subModulator*v.subEnvelope;}
            v.idleRateA=freq*tune1*driftA/sr;v.idleRateB=freq*tune2*driftB/sr;
            const float sourceA=warp2==4?subModulator:warp2==1?readWave(v.phase,wave,scanA):0.0f;
            const float sourceB=warp1==4?subModulator:warp1==1?readWave(v.phase2,secondWave,scanB):0.0f;
            const auto stack=[&](double& phase,auto& extras,double rate,const auto& rates,
                                  int count,int shape,float width,float gain,float blend,float detuneScale,double offset,int warp,float amount,float modulator,float scan,int oscillator){
                if(gain==0.0f){
                    phase+=rate*rates[0];
                    phase-=std::floor(phase);
                    return;
                }
                const int centres=count==1?1:count%2==0?2:1;
                float normalisation=centres+(count-centres)*blend;const auto& settings=unisonSettings[size_t(oscillator)];if(manualUnison){normalisation=0;for(int i=0;i<count;++i)normalisation+=settings.weight(i,count);normalisation*=settings.groups();}normalisation=juce::jmax(.00001f,normalisation);
                const float spreadDelta=voiceDetune-detune;
                for(int i=0;i<count*(manualUnison?settings.groups():1);++i){
                    const int local=i%count;auto& position=i==0?phase:extras[size_t(i-1)];
                    const double spread=count==1?0.0:2.0*double(local)/double(count-1)-1.0;
                    const bool centre=std::abs(2*local-(count-1))<=1;
                    const float level=gain*(manualUnison?settings.weight(local,count):(centre?1.0f:blend))/normalisation;
                    const double increment=rate*rates[size_t(i)]*(1.0+0.5198603854*spreadDelta*detuneScale*spread);
                    position+=increment;
                    position-=std::floor(position);
                    const float frame=juce::jlimit(0.0f,1.0f,scan+(manualUnison?float(spread)*wtSpread[size_t(oscillator)]:0));const float warpAmount=juce::jlimit(0.0f,1.0f,amount+(manualUnison?float(spread)*warpSpread[size_t(oscillator)]:0));
                    float sample=readWarp(position,shape,warp,warpAmount,modulator,frame);
                    if(warp==0&&(cleanVco||character>0.0f)){const float edgeAmount=cleanVco?juce::jlimit(0.0f,1.0f,1-frame*16):character;
                        if(shape==0)sample-=edgeAmount*analoguePolyBlep(position,increment);
                        if(shape==1)sample+=edgeAmount*(analoguePolyBlep(position,increment)-analoguePolyBlep(position-(.48+detail[5]*drift[1]*pulseDriftScale),increment));
                    }
                    if(cleanVco&&std::abs(increment)>=.5)sample=0;
                    raw+=sample*level;
                    if(width>0.0f){
                        if(count==1&&!manualUnison)rawSide+=width*level*(sample-readWarp(position+offset,shape,warp,amount,modulator,scan))*0.5f;
                        else if(count>1)rawSide+=sample*level*width*(2.0f*float(local)/float(count-1)-1.0f);
                    }
                }
            };
            stack(v.phase,v.extraPhase1,freq*tune1*driftA/sr,unisonRates1,unison1,wave,voiceWidth1,voiceGain1,blendA,spreadA,0.19,warp1,juce::jlimit(0.0f,1.0f,warpAmount1+.5f*movingModulation[26])*(warpEnvelope1==1?v.env:warpEnvelope1==2?filterEnv:1),sourceB,scanA,0);
            panSignal(raw,rawSide,pan[0]);
            bypass+=raw*(1-live[5]);bypassSide+=rawSide*(1-live[5]);raw*=live[5];rawSide*=live[5];
            const float beforeB=raw,beforeSideB=rawSide;
            stack(v.phase2,v.extraPhase2,freq*tune2*driftB*(1.0+(manualUnison?0:voiceDetune)+0.003*lfo*lfoDepth)/sr,
                  unisonRates2,unison2,secondWave,voiceWidth2,voiceGain2,blendB,spreadB,0.31,warp2,juce::jlimit(0.0f,1.0f,warpAmount2+.5f*movingModulation[27])*(warpEnvelope2==1?v.env:warpEnvelope2==2?filterEnv:1),sourceA,scanB,1);
            {float mid=raw-beforeB,sidePart=rawSide-beforeSideB;panSignal(mid,sidePart,pan[1]);raw=beforeB+mid;rawSide=beforeSideB+sidePart;}
            bypass+=(raw-beforeB)*(1-live[6]);bypassSide+=(rawSide-beforeSideB)*(1-live[6]);raw=beforeB+(raw-beforeB)*live[6];rawSide=beforeSideB+(rawSide-beforeSideB)*live[6];
            }
            if(v.source!=0){bypass+=raw*(1-live[5]);bypassSide+=rawSide*(1-live[5]);raw*=live[5];rawSide*=live[5];}
            float subSide=0;panSignal(rawSub,subSide,pan[2]);raw+=rawSub*live[7];rawSide+=subSide*live[7];bypassSide+=subSide*(1-live[7]);rawSub*=1-live[7];
            const float voiceNoise=juce::jlimit(0.0f,1.0f,live[3]+.5f*movingModulation[11]);
            if(voiceNoise>0.0f){
                auto& rng=v.noiseState;
                rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;
                const float white=float(rng>>8)*(2.0f/16777215.0f)-1.0f;
                v.noiseMemory+=0.10f*(white-v.noiseMemory);
                const float noise=noiseType==1?v.noiseMemory*2.5f:noiseType==2?white-v.noiseMemory:white;
                float mid=noise*voiceNoise*.5f,sidePart=0;panSignal(mid,sidePart,pan[3]);raw+=mid*live[8];rawSide+=sidePart*live[8];bypass+=mid*(1-live[8]);bypassSide+=sidePart*(1-live[8]);
            }
            if(character>0.0f){
                if(stereoFilters){const float l=(std::tanh((raw+rawSide)*warmthDrive+warmthBias)-warmthCentre)*warmthScale,r=(std::tanh((raw-rawSide)*warmthDrive+warmthBias)-warmthCentre)*warmthScale;raw+=character*((l+r)*.5f-raw);rawSide+=character*((l-r)*.5f-rawSide);}
                else{const float warmed=(std::tanh(raw*warmthDrive+warmthBias)-warmthCentre)*warmthScale;raw+=character*(warmed-raw);rawSide+=character*(std::tanh(rawSide*warmthDrive)*warmthScale-rawSide);}
            }
            if(preFilterDrive>0){
                const float gain=1+12*preFilterDrive,normalisation=1/std::tanh(gain);
                const float l=std::tanh((raw+rawSide)*gain)*normalisation,r=std::tanh((raw-rawSide)*gain)*normalisation;
                raw=.5f*(l+r);rawSide=.5f*(l-r);
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
                const auto stage=[g=voiceG](float input,float damping,float* memory){return analogueSvf(input,g,damping,memory);};
                const auto response=stage(raw,1.4142f-voiceRes*1.34f,v.filterState);
                filtered=filterMode==4 ? stage(response[0],1.4142f,v.filterCascade)[0]
                                       : response[size_t(filterMode-1)];
            }
            if(filterMode!=5){
                if(!stereoFilters||filterMode==0){v.stereoSide+=voiceOnePole*(rawSide*(stereoFilters?1+voiceRes*.7f:1)-v.stereoSide);filteredSide=v.stereoSide;}
                else{const auto response=analogueSvf(rawSide,voiceG,1.4142f-voiceRes*1.34f,v.filterSideState);filteredSide=filterMode==4?analogueSvf(response[0],voiceG,1.4142f,v.filterSideCascade)[0]:response[size_t(filterMode-1)];}
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
            const float level=(1-ampVelocity+ampVelocity*v.velocity)*v.env*0.14f*((upgraded?1+v.calibration.gain*character*.025f:1)+v.calibration.gain*detail[4]*voiceLevelScale);
            const float voltageL=filtered+rawSub+bypass+filteredSide+bypassSide,voltageR=filtered+rawSub+bypass-filteredSide-bypassSide;
            float voiceL=voltageL*level,voiceR=voltageR*level;
            if(voltageBlend<.999999f){voiceL=v.colour.process(voiceL,colour,0);voiceR=v.colour.process(voiceR,colour,1);}
            if(voltageBlend>.000001f){const float colouredL=v.voltageColour.process(voltageL,voltageColour,0)*level,colouredR=v.voltageColour.process(voltageR,voltageColour,1)*level;voiceL+=voltageBlend*(colouredL-voiceL);voiceR+=voltageBlend*(colouredR-voiceR);}
            if(v.declickRemaining>0){const float t=1-float(v.declickRemaining)/float(v.declickLength);voiceL=voiceL*t+v.tailL*(1-t);voiceR=voiceR*t+v.tailR*(1-t);--v.declickRemaining;}
            v.lastL=voiceL;v.lastR=voiceR;out+=(voiceL+voiceR)*.5f;
            side+=(voiceL-voiceR)*.5f;
        }
        const float driveGain=1+drive*9;
        left[s]=std::tanh((out+side)*driveGain);
        right[s]=std::tanh((out-side)*driveGain);
    }
    const auto oldTempo=displayedBpm.exchange(delayBpm,std::memory_order_relaxed);
    if(std::abs(oldTempo-delayBpm)>.001&&param("irSync")>0)impulseRequest.fetch_add(1,std::memory_order_relaxed);
    const bool distort=param("distEnabled")>.5f;
    distortion.beginBlock();
    if(distort){studio::Distortion::Settings settings;settings.type=int(param("distType"));settings.inputDb=param("distInput");settings.tone=param("distTone");settings.hq=param("distHQ")>.5f;settings.drive=param("distDrive");settings.mix=param("distMix");settings.filterPosition=int(param("distFilterPosition"));settings.frequency=param("distFrequency");settings.q=param("distQ");settings.filterMorph=param("distFilterMorph");for(int i=0;i<9;++i){settings.a[size_t(i)]=param(("distShape0_"+juce::String(i)).toRawUTF8());settings.b[size_t(i)]=param(("distShape1_"+juce::String(i)).toRawUTF8());}distortion.configure(settings);for(int i=0;i<b.getNumSamples();++i){const auto value=distortion.process(left[i],right[i]);left[i]=value[0];right[i]=value[1];}}
    distortion.endBlock();finishEffect(8,b,distort);
    studio::Rhythm::Settings rhythmSettings;
    rhythmSettings.gate=param("gateEnabled")>.5f;rhythmSettings.pump=param("pumpEnabled")>.5f;
    rhythmSettings.split=int(param("gateSplit"));rhythmSettings.rate=int(param("gateRate"));rhythmSettings.pumpRate=int(param("pumpRate"));rhythmSettings.sidechain=int(param("sidechainSource"));
    rhythmSettings.stepWidth=param("gateStepWidth");rhythmSettings.stepCount=int(param("gateStepCount"));rhythmSettings.delay=param("gateDelay");rhythmSettings.fade=param("gateFade");
    rhythmSettings.depth=juce::jlimit(0.0f,1.0f,param("gateDepth")+fxModulation[24]);rhythmSettings.swing=param("gateSwing");rhythmSettings.pumpDepth=param("pumpDepth");rhythmSettings.attack=param("gateAttack");rhythmSettings.release=param("gateRelease");
    rhythmSettings.pumpAttack=param("pumpAttack");rhythmSettings.pumpRelease=param("pumpRelease");rhythmSettings.threshold=param("sidechainThreshold");rhythmSettings.ratio=param("sidechainRatio");rhythmSettings.low=param("gateLow");rhythmSettings.high=param("gateHigh");
    for(int i=0;i<16;++i)rhythmSettings.steps[size_t(i)]=gateParameters[size_t(i)]->load();
    rhythmSettings.curveMode=param("gateMode")>.5f;rhythmSettings.phase=param("gatePhase");rhythmSettings.pwm=param("gatePWM");rhythmSettings.smooth=param("gateSmooth");rhythmSettings.retrigger=param("gateRetrigger")>.5f;rhythmSettings.noteGate=param("gateNoteGate")>.5f;rhythmSettings.curveRate=int(param("gateCurveRate"));rhythmSettings.curve.count=juce::jlimit(2,33,int(param("gateCurveCount")));
    for(int i=0;i<33;++i){rhythmSettings.curve.x[size_t(i)]=gateCurveParameters[0][size_t(i)]->load();rhythmSettings.curve.y[size_t(i)]=gateCurveParameters[1][size_t(i)]->load();rhythmSettings.curve.bend[size_t(i)]=gateCurveParameters[2][size_t(i)]->load();}
    rhythm.configure(rhythmSettings,delayBpm);
    if(auto* head=getPlayHead())if(auto info=head->getPosition())if(info->getIsPlaying()&&!rhythmSettings.retrigger)if(auto ppq=info->getPpqPosition())rhythm.setBeat(*ppq);
    auto rhythmEvent=midi.begin();
    if(rhythmSettings.gate||rhythmSettings.pump)for(int i=0;i<b.getNumSamples();++i){while(rhythmEvent!=midi.end()&&(*rhythmEvent).samplePosition<=i){rhythm.midi((*rhythmEvent).getMessage());++rhythmEvent;}const auto result=rhythm.process(left[i],right[i],sidechainHigh.getSample(0,juce::jmin(i,sidechainHigh.getNumSamples()-1)));
        if(rhythmSettings.gate||rhythmSettings.pump){left[i]=result[0];right[i]=result[1];}}
    else {int previous=0;for(const auto event:midi){const int at=juce::jlimit(previous,b.getNumSamples(),event.samplePosition);rhythm.advance(at-previous);rhythm.midi(event.getMessage());previous=at;}rhythm.advance(b.getNumSamples()-previous);}
    gateViewPhase.store(rhythm.displayPhase);
    displayedGateStep.store(rhythm.displayStep,std::memory_order_relaxed);
    finishEffect(3,b,rhythmSettings.gate||rhythmSettings.pump);
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
    finishEffect(0,b,param("compEnabled")>.5f);
    chorus.setRate(param("chorusRate"));
    chorus.setDepth(param("chorusDepth"));
    chorus.setCentreDelay(param("chorusDelay"));
    chorus.setFeedback(param("chorusFeedback"));
    chorus.setMix(juce::jlimit(0.0f,1.0f,param("chorus")+0.4f*fxModulation[7]));
    juce::dsp::AudioBlock<float> block(b);
    juce::dsp::ProcessContextReplacing<float> context(block);
    if(param("chorusEnabled")>0.5f)chorus.process(context);
    finishEffect(4,b,param("chorusEnabled")>.5f);
    const float flangeMix=param("flangerEnabled")>0.5f?param("flanger"):0.0f;
    flanger.setRate(param("flangerRate"));
    flanger.setDepth(param("flangerDepth"));
    flanger.setCentreDelay(param("flangerDelay"));
    flanger.setFeedback(param("flangerFeedback"));
    if(param("flangerMode")>.5f){smoothFlanger.configure(param("flangerRate"),param("flangerDepth"),param("flangerDelay"),param("flangerFeedback"),param("flangerStereo"),flangeMix);for(int i=0;i<b.getNumSamples();++i){const auto v=smoothFlanger.process(left[i],right[i]);left[i]=v[0];right[i]=v[1];}}
    else if(flangeMix>0.0f){
        if(!flangerActive)flanger.reset();
        flanger.setMix(flangeMix);
        flanger.process(context);
    }
    flangerActive=flangeMix>0.0f;
    finishEffect(5,b,flangeMix>0);
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


    finishEffect(6,b,phaseMix>0);
    configureWetEQ();
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
        const auto shaped=delayWetEQ.process(dl,dr);left[s]+=shaped[0]*wet;right[s]+=shaped[1]*wet;
    }
    }else{
        static constexpr double beats[]{0,.0625,.125,.25,.5,1,2,4};
        const bool linked=param("echoLink")>.5f;const int syncL=int(param("echoSyncL")),syncR=linked?syncL:int(param("echoSyncR"));
        studio::Delay::Settings e;e.mode=int(param("echoMode"));e.hq=param("echoHQ")>.5f;e.feedback=param("delayFeedback");e.frequency=param("echoFrequency");e.bandwidth=param("echoBandwidth");e.output=param("echoOutput");
        e.left=float((syncL?60.0/delayBpm*beats[syncL]:param("echoTimeL"))*param("echoOffsetL"));
        e.right=linked?e.left:float((syncR?60.0/delayBpm*beats[syncR]:param("echoTimeR"))*param("echoOffsetR"));
        studioDelay.configure(e);echoWet.setTargetValue(param("delayEnabled")>.5f?juce::jlimit(0.0f,1.0f,param("echoMix")+.4f*fxModulation[6]):0);
        for(int i=0;i<b.getNumSamples();++i){const auto unshaped=studioDelay.process(left[i],right[i]);const auto wet=delayWetEQ.process(unshaped[0],unshaped[1]);const float mix=echoWet.getNextValue();left[i]=(1-mix)*left[i]+mix*wet[0];right[i]=(1-mix)*right[i]+mix*wet[1];}
    }
    finishEffect(1,b,param("delayEnabled")>.5f);
    for(int ch=0;ch<2;++ch)workflowDry.copyFrom(ch,0,b,ch,0,b.getNumSamples());
    auto params=reverb.getParameters();
    const int space=juce::jlimit(0,2,int(param("reverbSpace")));
    static constexpr float spaceSize[]{-.36f,0.0f,.17f},spaceDamping[]{.11f,0.0f,-.13f};
    params.roomSize=juce::jlimit(0.0f,1.0f,param("reverbSize")+spaceSize[space]);
    params.damping=juce::jlimit(0.0f,1.0f,param("reverbDamping")+spaceDamping[space]);
    params.wetLevel=param("reverbEnabled")>0.5f?juce::jlimit(0.0f,0.8f,param("reverb")+0.4f*fxModulation[3])*.65f:0.0f;
    reverbDryGain.setTargetValue(2*(1-params.wetLevel*.35f));params.dryLevel=0;params.width=param("reverbWidth");
    reverb.setParameters(params);reverb.processStereo(left,right,b.getNumSamples());
    for(int i=0;i<b.getNumSamples();++i){const auto wet=reverbWetEQ.process(left[i],right[i]);const float reverbDry=reverbDryGain.getNextValue();left[i]=workflowDry.getSample(0,i)*reverbDry+wet[0];right[i]=workflowDry.getSample(1,i)*reverbDry+wet[1];}
    finishEffect(7,b,param("reverbEnabled")>.5f);
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
    finishEffect(2,b,param("convolverEnabled")>.5f);
    if(param("eqEnabled")>0.5f){
        for(size_t band=0;band<eqParameters.size();++band){
            const auto& values=eqParameters[band];
            const float gain=readParameter(values[1]);
            const int mode=int(eqTypes[band]->load());
            const int stages=(mode==3||mode==4)?int(eqSlopes[band]->load())+1:1;
            if(mode==0&&std::abs(gain)<0.0001f){
                eqState[0][band]={};
                eqState[1][band]={};
                continue;
            }
            for(int stage=0;stage<stages;++stage){
                const auto co=studio::eqCoefficients(sr,readParameter(values[0]),readParameter(values[2]),gain,mode,stages,stage);const float scale=1/co[3];
                for(int channel=0;channel<2;++channel){auto* samples=b.getWritePointer(channel);auto& memory=eqState[size_t(channel)][band][size_t(stage)];
                    for(int i=0;i<b.getNumSamples();++i){const float in=samples[i],out=co[0]*scale*in+memory.z1;memory.z1=co[1]*scale*in-co[4]*scale*out+memory.z2;memory.z2=co[2]*scale*in-co[5]*scale*out;samples[i]=out;}}
            }
        }
    }else eqState={};
    const bool match=param("levelMatch")>.5f;
    if(match&&!levelMatchActive){levelMatchEnergy=levelMatchReference=0;levelMatchSamples=0;levelMatchGain=1;}
    levelMatchActive=match;
    const double energyCoefficient=1-std::exp(-1/(sr*.04)),gainCoefficient=1-std::exp(-1/(sr*.035));
    for(int sample=0;sample<b.getNumSamples();++sample){
        const double l=b.getSample(0,sample),r=b.getSample(1,sample);
        levelMatchEnergy+=energyCoefficient*(.5*(l*l+r*r)-levelMatchEnergy);
        double desired=1;
        if(match){const double rms=std::sqrt(juce::jmax(0.0,levelMatchEnergy));
            if(levelMatchSamples<int(sr*.25)){if(rms>.001){levelMatchReference=juce::jmax(levelMatchReference,rms);++levelMatchSamples;}}
            else if(rms>.0001)desired=juce::jlimit(.125,1.0,levelMatchReference/rms);
        }
        levelMatchGain+=gainCoefficient*(desired-levelMatchGain);
        if(levelMatchGain<.999999){b.setSample(0,sample,float(l*levelMatchGain));b.setSample(1,sample,float(r*levelMatchGain));}
    }
    const float outputGain=juce::jlimit(0.0f,1.2f,param("gain")+0.5f*fxModulation[14])*juce::Decibels::decibelsToGain(param("presetTrim"));
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
    for(const auto& voice:voices)if(voice.active&&voice.source!=0&&voice.source==int(param("source"))&&(!userSample(voice.source)||sampleGuard.isLocked())){const auto& sample=sampleForSource(voice.source,voice.sampleZone);
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
    state.replaceState(juce::ValueTree::fromXml(*xml));restoreFlexibleRoutes();resetVoicesRequested.store(true);}
namespace {
const juce::var& tranceFactoryBank(){
    static const auto bank=juce::JSON::parse(juce::String::fromUTF8(FactoryBankData::TranceFactoryBank_json,FactoryBankData::TranceFactoryBank_jsonSize));
    return bank;
}
}
const juce::StringArray& SpaceInfinityProcessor::presetNames(){
    static const auto names=[] {juce::StringArray result{"Init"};for(const auto& patch:*tranceFactoryBank()["presets"].getArray())result.add(patch["name"].toString());return result;}();
    return names;
}
juce::String SpaceInfinityProcessor::presetCategory(int index){
    return index==0?"Init":index>0&&index<presetNames().size()?tranceFactoryBank()["presets"][index-1]["category"].toString():juce::String{};
}
juce::String SpaceInfinityProcessor::presetCreator(int index){return index>0?tranceFactoryBank()["presets"][index-1].getProperty("creator","Orbital Audio").toString():juce::String{};}
juce::String SpaceInfinityProcessor::presetTags(int index){return index>0?tranceFactoryBank()["presets"][index-1].getProperty("tags",presetCategory(index)).toString():juce::String("Init");}
void SpaceInfinityProcessor::loadPreset(int index){
    if(index<0||index>=presetNames().size())return;
    const float retainedMode=param("engineMode");
    if(index>0){int size=0;const auto resource=library->keyForFactory(index)+"_sipreset";if(const auto* data=FactoryBankData::getNamedResource(resource.toRawUTF8(),size)){if(size>0){setStateInformation(data,size);state.getParameter("engineMode")->setValueNotifyingHost(retainedMode);state.state.setProperty("presetKey",library->keyForFactory(index),nullptr);state.state.setProperty("creator",presetCreator(index),nullptr);state.state.setProperty("tags",presetTags(index),nullptr);state.state.setProperty("presetDisplayName",presetNames()[index],nullptr);return;}}}
    state.state.setProperty("creator",presetCreator(index),nullptr);state.state.setProperty("tags",presetTags(index),nullptr);state.state.setProperty("presetDisplayName",presetNames()[index],nullptr);
    clearFlexibleRoutes();
    for(const auto* property:{"gatePresetName","factoryReference","factoryDescription","macroLabel0","macroLabel1","macroLabel2"})state.state.removeProperty(property,nullptr);
    for(auto* parameter:getParameters())if(auto* ranged=dynamic_cast<juce::RangedAudioParameter*>(parameter))ranged->setValueNotifyingHost(ranged->getDefaultValue());
    const auto set=[this](const juce::String& id,float value){if(auto* p=state.getParameter(id))p->setValueNotifyingHost(p->convertTo0to1(value));};
    const auto apply=[&](const juce::var& settings){if(const auto* obj=settings.getDynamicObject())for(const auto& property:obj->getProperties())set(property.name.toString(),float(property.value));};
    apply(tranceFactoryBank()["init_settings"]);
    Sample imported;juce::MemoryBlock bytes;
    if(index>0){
        const auto& patch=tranceFactoryBank()["presets"][index-1];apply(patch["settings"]);for(int m=0;m<3;++m)state.state.setProperty("macroLabel"+juce::String(m),juce::StringArray{"TONE","PUNCH","CHARACTER"}[m],nullptr);
        state.state.setProperty("factoryReference",patch["reference"],nullptr);state.state.setProperty("factoryDescription",patch["play"],nullptr);
        std::array<int,9> slots{};
        const auto assign=[&](int source,const juce::String& target,float depth){const int t=flexibleTargetIndex(target);if(t>=0&&source>=0&&source<9&&slots[size_t(source)]<16){const int slot=slots[size_t(source)]++;assignFlexibleSource(source,slot,t);set(flexibleDepthId(source,slot),depth);}};
        if(const auto* macros=patch["macros"].getArray())for(const auto& macro:*macros)assign(int(macro["source"]),macro["target"].toString(),float(macro["depth"]));
        const auto& lfo=patch["lfo"];
        if(lfo.isObject()){
            set(lfoParameterId(0,"Enabled"),1);set(lfoParameterId(0,"Sync"),lfo.hasProperty("hz")?0:1);
            set(lfoParameterId(0,lfo.hasProperty("hz")?"Speed":"Bars"),float(lfo[lfo.hasProperty("hz")?"hz":"bars"]));
            for(int point=0;point<17;++point)set(lfoPointId(0,point),1-std::abs(float(point)/8-1));
            enableLfoCurve(0);assign(3,lfo["target"].toString(),float(lfo["depth"]));
        }
        const auto resource=patch["grain_resource"].toString();
        if(resource.isNotEmpty()){int size=0;const auto* data=BinaryData::getNamedResource(resource.toRawUTF8(),size);if(data&&size>0){bytes=juce::MemoryBlock(data,size_t(size));decodeSample(bytes,imported);}}
    }
    {const juce::SpinLock::ScopedLockType lock(sampleLock);std::swap(importedSample,imported);std::swap(importedBytes,bytes);++sampleRevision;}
    set("engineMode",retainedMode);state.state.setProperty("presetKey",library->keyForFactory(index),nullptr);resetVoicesRequested.store(true);
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new SpaceInfinityProcessor();}

