#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <BinaryData.h>
#include <algorithm>
#include <cmath>
namespace {
using APVTS=juce::AudioProcessorValueTreeState;
APVTS::ParameterLayout makeLayout(){
    APVTS::ParameterLayout p;
    auto f=[&](const char* id,const char* label,float low,float high,float def){p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1},label,juce::NormalisableRange<float>(low,high),def));};
    f("cutoff","Filter Cutoff",80,16000,8000);f("res","Resonance",0,0.95f,0.12f);
    f("attack","Attack",0.002f,4,0.01f);f("release","Release",0.02f,8,0.5f);
    f("ampDecay","Amp Decay",0.005f,8.0f,0.35f);
    f("ampSustain","Amp Sustain",0.0f,1.0f,1.0f);
    f("detune","Detune",0,0.04f,0.008f);f("mix","Layer Mix",0,1,0.55f);
    f("drive","Drive",0,1,0.15f);f("chorus","Chorus",0,1,0.25f);
    f("delay","Delay",0,0.8f,0.17f);f("reverb","Reverb",0,0.8f,0.25f);
    f("lforate","LFO Rate",0.1f,12,4.0f);f("lfodepth","LFO Depth",0,1,0.1f);
    f("gain","Output",0,1.2f,0.65f);
    p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"wave",1},"Oscillator",juce::StringArray{"Saw","Pulse","Sine","Triangle"},0));
    p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"source",1},"Sound Source",SpaceInfinityProcessor::sourceNames(),0));
    p.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"sampleRoot",1},"Imported Sample Root (MIDI)",0,127,60));
    auto choice=[&](const char* id,const char* label,juce::StringArray options,int def){
        p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{id,1},label,options,def));
    };
    f("sampleStart","Sample Trim Start",0.0f,1.0f,0.0f);
    f("sampleEnd","Sample Trim End",0.0f,1.0f,1.0f);
    f("motionDepth","Motion to Cutoff",0.0f,1.0f,0.0f);
    choice("motionBars","Motion Length",{"1 bar","2 bars","4 bars","8 bars"},2);
    for(int i=0;i<17;++i)
        p.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{SpaceInfinityProcessor::motionPointId(i),1},
            "Motion Point "+juce::String(i+1),juce::NormalisableRange<float>(0.0f,1.0f),float(i)/16.0f));
    const auto macros=SpaceInfinityProcessor::macroNames();
    for(int i=0;i<macros.size();++i){
        const auto suffix=juce::String(i);
        p.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"macro"+suffix,1},macros[i],juce::NormalisableRange<float>(-1.0f,1.0f),0.0f));
        p.add(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{"macroRoute"+suffix,1},macros[i]+" Destination",
            SpaceInfinityProcessor::macroTargets(),i+1));
    }
    choice("filterMode","Filter Mode",{"Classic LP","Resonant LP","High-pass","Band-pass","24 dB LP"},0);
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
SpaceInfinityProcessor::SpaceInfinityProcessor():juce::AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"PARAMETERS",makeLayout()){
    for(size_t i=0;i<motionParameters.size();++i)
        motionParameters[i]=state.getRawParameterValue(motionPointId(int(i)));
    for(size_t i=0;i<macroValues.size();++i){
        const auto suffix=juce::String(int(i));
        macroValues[i]=state.getRawParameterValue("macro"+suffix);
        macroRoutes[i]=state.getRawParameterValue("macroRoute"+suffix);
    }
    decodeSample(juce::MemoryBlock{BinaryData::Orbital_Choir_Ah_Chord_F2_wav,
                  BinaryData::Orbital_Choir_Ah_Chord_F2_wavSize}, choirAh);
    decodeSample(juce::MemoryBlock{BinaryData::Ethereal_Choir_Ooh_C3_wav,
                  BinaryData::Ethereal_Choir_Ooh_C3_wavSize}, choirOoh);
}
juce::String SpaceInfinityProcessor::motionPointId(int index){return "motionPoint"+juce::String(index);}
juce::StringArray SpaceInfinityProcessor::macroNames(){return {"Energy","Width","Space","Shape"};}
juce::StringArray SpaceInfinityProcessor::macroTargets(){
    return {"Off","Cutoff","Detune","Reverb","Attack","Drive","Delay","Chorus","Layer mix","Release"};
}
juce::StringArray SpaceInfinityProcessor::motionNames(){
    return {"Slow Rise","Rise and Fall","Rise into Gate","Late Bloom","Long Fall","Soft Pulse",
        "Double Swell","Triple Swell","Terraces","Descending Steps","Peak Hold","Dip and Return",
        "Quick Strike","Reverse Pull","Saw Rise","Saw Fall","Alternating Gates","Syncopated","Wave Train","Final Lift"};
}
void SpaceInfinityProcessor::loadMotionShape(int index){
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
    if(index<0||index>=int(std::size(curves)))return;
    const auto& curve=curves[index];
    for(int i=0;i<17;++i){
        const float x=float(i)*float(curve.size()-1)/16.0f;
        const auto left=std::min(size_t(x),curve.size()-2);
        const float value=curve[left]+(curve[left+1]-curve[left])*(x-float(left));
        state.getParameter(motionPointId(i))->setValueNotifyingHost(value);
    }
}
juce::StringArray SpaceInfinityProcessor::sourceNames(){
    return {"Synth", "Choir Ah", "Choir Ooh", "Imported WAV"};
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
    state.getParameter("sampleRoot")->setValueNotifyingHost(state.getParameter("sampleRoot")->convertTo0to1(60));
    state.getParameter("source")->setValueNotifyingHost(1.0f);
    return juce::Result::ok();
}
void SpaceInfinityProcessor::prepareToPlay(double sampleRate,int maxSamples){
    sr=sampleRate;
    juce::dsp::ProcessSpec spec{sampleRate,(juce::uint32)std::max(maxSamples,1),2};chorus.prepare(spec);chorus.setRate(0.25f);chorus.setDepth(0.38f);chorus.setCentreDelay(9);chorus.setFeedback(0.1f);chorus.setMix(param("chorus"));reverb.setSampleRate(sr);
    delayL.prepare({sampleRate,(juce::uint32)std::max(maxSamples,1),1});delayR.prepare({sampleRate,(juce::uint32)std::max(maxSamples,1),1});delayL.reset();delayR.reset();
    crossoverLow.prepare(spec); crossoverLow.setCutoffFrequency(180.0f);
    crossoverHigh.prepare(spec); crossoverHigh.setCutoffFrequency(2600.0f);
    alignLow.prepare(spec); alignLow.setCutoffFrequency(2600.0f);
    alignLow.setType(juce::dsp::LinkwitzRileyFilterType::allpass);
    for(auto& band:bandCompressors){band.prepare(spec);band.setAttack(9);band.setRelease(110);}
    delaySamples.reset(sr,0.07);
    delaySamples.setCurrentAndTargetValue(float(sr*.375));
    delayWet.reset(sr,0.025);delayWet.setCurrentAndTargetValue(param("delay"));
    bandMix.reset(sr,0.025);bandMix.setCurrentAndTargetValue(param("compAmount")>0.001f?1.0f:0.0f);
    delayToneL=delayToneR=0;
    delayBpm=120;
    midiBend=0;pitchBend.reset(sr,.006);pitchBend.setCurrentAndTargetValue(param("pitchBend"));
    for(auto& v:voices)v={}; lfoPhase=0; motionPhase=0; clock=0;
}
void SpaceInfinityProcessor::noteOn(int note,float velocity,int source,int root,int channel){
    const Sample* sample = source == 1 ? &choirAh : source == 2 ? &choirOoh : &importedSample;
    if(source != 0 && sample->audio.getNumSamples() < 2)return;
    auto* found=&voices[0]; for(auto& v:voices) if(!v.active){found=&v;break;}
    if(found->active)found=&*std::min_element(voices.begin(),voices.end(),[](const Voice& a,const Voice& b){return a.age<b.age;});
    *found={};found->active=true;found->note=note;found->channel=channel;found->velocity=velocity;found->age=++clock;
    found->source=source;
    if(source != 0){
        if(source==3)found->sampleRevision=sampleRevision;
        const int sampleRoot = source == 1 ? 41 : source == 2 ? 48 : root;
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
void SpaceInfinityProcessor::noteOff(int note,int channel){for(auto& v:voices) if(v.active&&v.note==note&&v.channel==channel)v.released=true;}
void SpaceInfinityProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer& midi){
    juce::ScopedNoDenormals noDenormals;
    b.clear(); if(b.getNumChannels()<2)return;
    auto* left=b.getWritePointer(0);auto* right=b.getWritePointer(1);
    std::array<float,10> modulation{};
    for(size_t i=0;i<macroValues.size();++i){
        const auto route=size_t(juce::jlimit(0,9,int(macroRoutes[i]->load())));
        modulation[route]+=macroValues[i]->load();
    }
    const float attack=juce::jlimit(0.002f,4.0f,param("attack")*std::exp2(modulation[4]*3));
    const float release=juce::jlimit(0.02f,8.0f,param("release")*std::exp2(modulation[9]*2));
    const float mix=juce::jlimit(0.0f,1.0f,param("mix")+0.4f*modulation[8]);
    const float detune=juce::jlimit(0.0f,0.04f,param("detune")+0.02f*modulation[2]);
    const float cutoff=param("cutoff")*std::exp2(modulation[1]*2);
    const float res=param("res"),lfoRate=param("lforate"),lfoDepth=param("lfodepth");
    const float drive=juce::jlimit(0.0f,1.0f,param("drive")+0.45f*modulation[5]);
    const float sustain=param("ampSustain");
    const float attackStep=float(1.0/std::max(1.0,double(attack)*sr));
    const float legacyAttackStep=float(1.0-std::exp(-double(attackStep)));
    const float decayStep=float(1.0/std::max(1.0,double(param("ampDecay"))*sr));
    const float releaseFactor=float(std::exp(-1.0/(double(release)*sr)));
    const int wave=int(param("wave"));
    const int filterMode=int(param("filterMode"));
    const float damping=1.4142f-res*1.34f;
    const int source=int(param("source")), root=int(param("sampleRoot"));
    const float hostBend=param("pitchBend");
    pitchBend.setTargetValue(juce::jlimit(-24.0f,24.0f,hostBend+midiBend));
    const juce::SpinLock::ScopedTryLockType sampleGuard(sampleLock);
    std::array<float,17> motion{};
    const float motionDepth=param("motionDepth");
    for(size_t i=0;i<motion.size();++i)
        motion[i]=motionParameters[i]->load();
    const double motionStep=delayBpm/(240.0*sr*std::exp2(param("motionBars")));
    auto msg=midi.begin();
    for(int s=0;s<b.getNumSamples();++s){
        while(msg!=midi.end()&&(*msg).samplePosition<=s){const auto m=(*msg).getMessage(); if(m.isNoteOn()){if(source!=3||sampleGuard.isLocked())noteOn(m.getNoteNumber(),m.getFloatVelocity(),source,root,m.getChannel());}else if(m.isPitchWheel()){const int wheel=m.getPitchWheelValue()-8192;midiBend=24.0f*float(wheel)/(wheel<0?8192.0f:8191.0f);pitchBend.setTargetValue(juce::jlimit(-24.0f,24.0f,hostBend+midiBend));}else if(m.isNoteOff())noteOff(m.getNoteNumber(),m.getChannel());else if(m.isAllNotesOff()||m.isAllSoundOff()){for(auto& v:voices)if(v.channel==m.getChannel()){if(m.isAllSoundOff())v.active=false;else v.released=true;}}++msg;}
        lfoPhase+=lfoRate/sr;if(lfoPhase>=1)lfoPhase-=1;
        const double point=motionPhase*16.0;
        const int segment=std::min(15,int(point));
        const float movement=motion[segment]+(motion[segment+1]-motion[segment])*float(point-segment);
        motionPhase+=motionStep;
        motionPhase-=std::floor(motionPhase);
        float out=0;const float lfo=std::sin(float(juce::MathConstants<double>::twoPi*lfoPhase));
        const double bendRatio=std::exp2(pitchBend.getNextValue()/12.0);
        const float fc=juce::jlimit(50.0f,float(sr*0.4),cutoff*std::exp2(lfo*lfoDepth*2+movement*motionDepth*3));
        const float g=std::tan(float(juce::MathConstants<double>::pi*fc/sr));
        const float onePole=1.0f-std::exp(float(-juce::MathConstants<double>::twoPi*fc/sr));
        for(auto& v:voices){if(!v.active)continue;
            if(v.source==3 && !sampleGuard.isLocked())continue;
            if(v.source==3 && v.sampleRevision!=sampleRevision){v.active=false;continue;}
            if(v.released)v.env*=releaseFactor;
            else if(sustain>=1.0f&&!v.decaying)v.env+=(1-v.env)*legacyAttackStep;
            else if(!v.decaying){
                v.env=std::min(1.0f,v.env+attackStep);
                v.decaying=v.env>=1.0f;
            }else v.env+=juce::jlimit(-decayStep,decayStep,sustain-v.env);
            if(v.released&&v.env<0.0001f){v.active=false;continue;}
            float raw=0;
            if(v.source!=0){
                const auto& sample = v.source==1 ? choirAh : v.source==2 ? choirOoh : importedSample;
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
            const double freq=440.0*bendRatio*std::pow(2.0,(v.note-69.0)/12.0);
            v.phase+=freq/sr;v.phase2+=freq*(1.0+detune+0.003*lfo*lfoDepth)/sr;
            if(v.phase>=1)v.phase-=std::floor(v.phase);if(v.phase2>=1)v.phase2-=std::floor(v.phase2);
            raw=osc(v.phase,wave)*(1-mix)+osc(v.phase2,wave)*mix;
            }
            float filtered;
            if(filterMode==0){
                v.cutoff+=onePole*(raw*(1+res*0.7f)-v.cutoff);
                filtered=v.cutoff;
            }else{
                const auto stage=[g](float input,float k,float* memory){
                    const float band=(memory[0]+g*(input-memory[1]))/(1.0f+g*(g+k));
                    const float low=memory[1]+g*band;
                    memory[0]=2.0f*band-memory[0];
                    memory[1]=2.0f*low-memory[1];
                    return std::array<float,3>{low,input-k*band-low,band};
                };
                const auto response=stage(raw,damping,v.filterState);
                filtered=filterMode==4 ? stage(response[0],1.4142f,v.filterCascade)[0]
                                       : response[size_t(filterMode-1)];
            }
            out+=filtered*v.velocity*v.env*0.14f;
        }
        out=std::tanh(out*(1+drive*9));left[s]=out;right[s]=out;
    }
    // Compress the dry synth in three bands before the time-based effects.
    const auto amount=param("compAmount");
    bandMix.setTargetValue(amount>0.001f?1.0f:0.0f);
    for(size_t i=0;i<bandCompressors.size();++i){
        bandCompressors[i].setThreshold(-10.0f-30.0f*amount-3.0f*float(i));
        bandCompressors[i].setRatio(1.0f+7.0f*amount);
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
    chorus.setMix(juce::jlimit(0.0f,1.0f,param("chorus")+0.4f*modulation[7]));
    juce::dsp::AudioBlock<float> block(b);
    juce::dsp::ProcessContextReplacing<float> context(block);
    chorus.process(context);

    if(auto* playHead=getPlayHead())
        if(auto position=playHead->getPosition())
            if(auto bpm=position->getBpm())
                if(*bpm>0.0)delayBpm=*bpm;
    static constexpr double divisions[]{0,.25,.5,.75,1};
    const int division=int(param("delaySync"));
    const bool ping=param("delayMode")>=1;
    const double seconds=division==0?.375:60.0/delayBpm*divisions[division];
    delaySamples.setTargetValue(juce::jlimit(1.0f,191999.0f,float(seconds*sr)));
    delayWet.setTargetValue(juce::jlimit(0.0f,0.8f,param("delay")+0.4f*modulation[6]));
    const float tone=float(1.0-std::exp(-juce::MathConstants<double>::twoPi*5700.0/sr));
    for(int s=0;s<b.getNumSamples();++s){
        const auto time=delaySamples.getNextValue(), wet=delayWet.getNextValue();
        const auto dl=delayL.popSample(0,time),dr=delayR.popSample(0,time*(ping?1.0f:.75f));
        delayToneL+=tone*(dl-delayToneL);delayToneR+=tone*(dr-delayToneR);
        delayL.pushSample(0,left[s]+(ping?delayToneR:delayToneL)*.35f);
        delayR.pushSample(0,(ping?0.0f:right[s])+ (ping?delayToneL:delayToneR)*.35f);
        left[s]+=dl*wet;right[s]+=dr*wet;
    }
    const auto space=int(param("reverbSpace"));
    static constexpr float roomSize[]{.36f,.72f,.89f},damping[]{.56f,.45f,.32f};
    auto params=reverb.getParameters();
    params.roomSize=roomSize[space];params.damping=damping[space];
    params.wetLevel=juce::jlimit(0.0f,0.8f,param("reverb")+0.4f*modulation[3])*.65f;params.dryLevel=1-params.wetLevel*.35f;params.width=1;
    reverb.setParameters(params);reverb.processStereo(left,right,b.getNumSamples());
    b.applyGain(param("gain"));for(int c=0;c<2;++c){auto* dst=b.getWritePointer(c);for(int s=0;s<b.getNumSamples();++s)dst[s]=std::tanh(dst[s]);}
}
void SpaceInfinityProcessor::getStateInformation(juce::MemoryBlock& m){auto t=state.copyState();std::unique_ptr<juce::XmlElement> xml(t.createXml());{
    const juce::SpinLock::ScopedLockType lock(sampleLock);
    if(importedBytes.getSize()!=0)xml->setAttribute("sampleWav",importedBytes.toBase64Encoding());
}copyXmlToBinary(*xml,m);}
void SpaceInfinityProcessor::setStateInformation(const void* data,int size){auto xml=getXmlFromBinary(data,size);if(!xml||!xml->hasTagName(state.state.getType()))return;
    Sample next;
    juce::MemoryBlock bytes;
    const auto encoded=xml->getStringAttribute("sampleWav");
    if(encoded.isNotEmpty()&&encoded.length()<=23*1024*1024){
        if(!bytes.fromBase64Encoding(encoded)||bytes.getSize()>16*1024*1024||decodeSample(bytes,next).failed())
            bytes.reset();
    }
    xml->removeAttribute("sampleWav");
    {
        const juce::SpinLock::ScopedLockType lock(sampleLock);
        std::swap(importedSample,next);
        std::swap(importedBytes,bytes);
        ++sampleRevision;
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
{"Glass Pizzicato",{263.24222f,0.14000f,0.00400f,0.17200f,0.00160f,0.35484f,0.12000f,0.05000f,0.32000f,0.23000f,1.17100f,0.60000f,0.46000f,3.00000f,0.11700f,0.02000f},0,3,1,1,0,2,0.00000f,2},
{"Airlock Mallet",{1160.49677f,0.18000f,0.00400f,0.28800f,0.00160f,0.35484f,0.12000f,0.08000f,0.28000f,0.34000f,1.17100f,0.60000f,0.46000f,2.00000f,0.22500f,0.01000f},0,3,1,2,0,2,0.00000f,3},
{"Meteor Lift",{447.15546f,0.38000f,1.08200f,0.78000f,0.00160f,0.35484f,0.12000f,0.28000f,0.23000f,0.29000f,1.17100f,0.60000f,0.46000f,0.00000f,0.46500f,0.87000f},0,4,1,2,3,1,0.27000f,1},
{"Gravity Drop",{1971.27374f,0.27000f,0.00400f,0.74000f,0.00160f,0.35484f,0.12000f,0.11000f,0.18000f,0.29000f,1.17100f,0.60000f,0.46000f,1.00000f,0.48500f,0.09000f},0,2,0,2,12,0,0.00000f,1},
{"Reentry Siren",{1434.44908f,0.44000f,0.06000f,0.94000f,0.00160f,0.35484f,0.31000f,0.10000f,0.18000f,0.31000f,1.40900f,0.60000f,0.46000f,1.00000f,0.52500f,0.82000f},0,3,1,1,0,2,0.00000f,3},
{"Sub Impact",{720.36273f,0.08000f,0.00400f,0.70000f,0.00160f,0.35484f,0.12000f,0.01000f,0.03000f,0.22000f,1.17100f,0.60000f,0.46000f,2.00000f,0.26500f,0.02000f},0,2,0,2,0,2,0.00000f,1},
{"Frozen Interlude",{249.65786f,0.12000f,0.15200f,1.30000f,0.02320f,0.35484f,0.12000f,0.42000f,0.38000f,0.48000f,1.17100f,0.60000f,0.46000f,3.00000f,0.58500f,0.69000f},0,4,1,2,5,3,0.19000f,2},
{"Acid Ghost",{800.88809f,0.60000f,0.00400f,0.37200f,0.00160f,0.35484f,0.31000f,0.04000f,0.36000f,0.18000f,1.17100f,0.60000f,0.46000f,0.00000f,0.19500f,0.22000f},0,3,1,1,0,2,0.00000f,3}
};
static_assert(std::size(factoryBank)==104);
}
const juce::StringArray& SpaceInfinityProcessor::presetNames(){
    static const auto names=[]{
        juce::StringArray result;
        for(const auto& patch:factoryBank)result.add(patch.name);
        return result;
    }();
    return names;
}
void SpaceInfinityProcessor::loadPreset(int index){
    if(index<0||index>=int(std::size(factoryBank)))return;
    const auto& patch=factoryBank[index];
    const auto set=[this](const char* id,float value){
        auto* parameter=state.getParameter(id);
        parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
    };
    constexpr const char* ids[]={"cutoff","res","attack","release","detune","mix","drive",
        "chorus","delay","reverb","lforate","lfodepth","gain","wave","ampDecay","ampSustain"};
    for(size_t i=0;i<std::size(ids);++i)set(ids[i],patch.values[i]);
    set("source",float(patch.source));
    set("filterMode",float(patch.filterMode));
    set("delaySync",float(patch.division));
    set("delayMode",float(patch.delayMode));
    set("reverbSpace",float(patch.space));
    set("compAmount",0.0f);
    set("sampleStart",0.0f);
    set("sampleEnd",1.0f);
    set("sampleLoop",0.0f);
    set("sampleReverse",0.0f);
    set("motionBars",float(patch.motionBars));
    set("motionDepth",patch.motionDepth);
    for(int i=0;i<4;++i){
        state.getParameter("macro"+juce::String(i))->setValueNotifyingHost(0.5f);
        auto* route=state.getParameter("macroRoute"+juce::String(i));
        route->setValueNotifyingHost(route->convertTo0to1(float(i+1)));
    }
    loadMotionShape(patch.motionShape);
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new SpaceInfinityProcessor();}
