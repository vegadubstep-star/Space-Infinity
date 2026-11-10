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
    decodeSample(juce::MemoryBlock{BinaryData::Orbital_Choir_Ah_Chord_F2_wav,
                  BinaryData::Orbital_Choir_Ah_Chord_F2_wavSize}, choirAh);
    decodeSample(juce::MemoryBlock{BinaryData::Ethereal_Choir_Ooh_C3_wav,
                  BinaryData::Ethereal_Choir_Ooh_C3_wavSize}, choirOoh);
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
    for(auto& v:voices)v={}; lfoPhase=0; clock=0;
}
void SpaceInfinityProcessor::noteOn(int note,float velocity,int source,int root){
    const Sample* sample = source == 1 ? &choirAh : source == 2 ? &choirOoh : &importedSample;
    if(source != 0 && sample->audio.getNumSamples() < 2)return;
    auto* found=&voices[0]; for(auto& v:voices) if(!v.active){found=&v;break;}
    if(found->active)found=&*std::min_element(voices.begin(),voices.end(),[](const Voice& a,const Voice& b){return a.age<b.age;});
    *found={};found->active=true;found->note=note;found->velocity=velocity;found->age=++clock;
    found->source=source;
    if(source != 0){
        if(source==3)found->sampleRevision=sampleRevision;
        const int sampleRoot = source == 1 ? 41 : source == 2 ? 48 : root;
        found->sampleStep=sample->sampleRate/sr*std::exp2((note-sampleRoot)/12.0);
    }
}
void SpaceInfinityProcessor::noteOff(int note){for(auto& v:voices) if(v.active&&v.note==note)v.released=true;}
void SpaceInfinityProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer& midi){
    juce::ScopedNoDenormals noDenormals;
    b.clear(); if(b.getNumChannels()<2)return;
    auto* left=b.getWritePointer(0);auto* right=b.getWritePointer(1);
    const float attack=param("attack"),release=param("release"),mix=param("mix"),detune=param("detune"),cutoff=param("cutoff"),res=param("res"),drive=param("drive"),lfoRate=param("lforate"),lfoDepth=param("lfodepth");
    const int wave=int(param("wave"));
    const int source=int(param("source")), root=int(param("sampleRoot"));
    const float hostBend=param("pitchBend");
    pitchBend.setTargetValue(juce::jlimit(-24.0f,24.0f,hostBend+midiBend));
    const juce::SpinLock::ScopedTryLockType sampleGuard(sampleLock);
    auto msg=midi.begin();
    for(int s=0;s<b.getNumSamples();++s){
        while(msg!=midi.end()&&(*msg).samplePosition<=s){const auto m=(*msg).getMessage(); if(m.isNoteOn()){if(source!=3||sampleGuard.isLocked())noteOn(m.getNoteNumber(),m.getFloatVelocity(),source,root);}else if(m.isPitchWheel()){const int wheel=m.getPitchWheel()-8192;midiBend=24.0f*float(wheel)/(wheel<0?8192.0f:8191.0f);pitchBend.setTargetValue(juce::jlimit(-24.0f,24.0f,hostBend+midiBend));}else if(m.isNoteOff())noteOff(m.getNoteNumber());else if(m.isAllNotesOff()||m.isAllSoundOff()){for(auto& v:voices)v.released=true;}++msg;}
        lfoPhase+=lfoRate/sr;if(lfoPhase>=1)lfoPhase-=1;
        float out=0;const float lfo=std::sin(float(juce::MathConstants<double>::twoPi*lfoPhase));
        const double bendRatio=std::exp2(pitchBend.getNextValue()/12.0);
        for(auto& v:voices){if(!v.active)continue;
            if(v.source==3 && !sampleGuard.isLocked())continue;
            if(v.source==3 && v.sampleRevision!=sampleRevision){v.active=false;continue;}
            if(v.released)v.env*=std::exp(-1.0/(std::max(0.001f,release)*sr));else v.env+=(1-v.env)*(1-std::exp(-1.0/(std::max(0.001f,attack)*sr)));
            if(v.released&&v.env<0.0001f){v.active=false;continue;}
            float raw=0;
            if(v.source!=0){
                const auto& sample = v.source==1 ? choirAh : v.source==2 ? choirOoh : importedSample;
                const auto index=static_cast<int>(v.samplePosition);
                if(index+1>=sample.audio.getNumSamples()){v.active=false;continue;}
                const auto* data=sample.audio.getReadPointer(0);
                raw=data[index]+static_cast<float>(v.samplePosition-index)*(data[index+1]-data[index]);
                v.samplePosition+=v.sampleStep*bendRatio;
            }else{
            const double freq=440.0*bendRatio*std::pow(2.0,(v.note-69.0)/12.0);
            v.phase+=freq/sr;v.phase2+=freq*(1.0+detune+0.003*lfo*lfoDepth)/sr;
            if(v.phase>=1)v.phase-=std::floor(v.phase);if(v.phase2>=1)v.phase2-=std::floor(v.phase2);
            raw=osc(v.phase,wave)*(1-mix)+osc(v.phase2,wave)*mix;
            }
            // One-pole lowpass with resonance-dependent input emphasis. Not a model of JV hardware filters.
            const float fc=juce::jlimit(50.0f,float(sr*0.4),cutoff*std::pow(2.0f,lfo*lfoDepth*2));
            const float a=1-std::exp(float(-juce::MathConstants<double>::twoPi*fc/sr));
            v.cutoff+=a*(raw*(1+res*0.7f)-v.cutoff);
            out+=v.cutoff*v.velocity*v.env*0.14f;
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
    chorus.setMix(param("chorus"));
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
    delayWet.setTargetValue(param("delay"));
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
    params.wetLevel=param("reverb")*.65f;params.dryLevel=1-params.wetLevel*.35f;params.width=1;
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
const juce::StringArray& SpaceInfinityProcessor::presetNames(){static const juce::StringArray names{"Classic Rave Saw","German HT Lead","Wide Anthem","Dark Hoover","Soft Dream Pad","Bright Choir-Like","Acid Stab","Rolling Bass","Trance Pluck","Epic Breakdown","Hard Dance Hook","Retro Synth Brass", "Orbital Vox", "Ethereal Choir", "Neon Strings", "Voltage Bass", "Redline Anthem", "Warehouse Pulse"};return names;}
void SpaceInfinityProcessor::loadPreset(int index){
    // Original factory-style sound-design presets; not Roland patch data.
    static constexpr float data[][14]={
      {8500,.1,.006,.32,.010,.54,.12,.24,.17,.18,4,.07,.68,0},
      {10500,.28,.004,.23,.021,.60,.26,.38,.23,.27,5.2,.11,.70,0},
      {9200,.13,.012,.75,.032,.58,.2,.62,.32,.41,3.4,.15,.58,0},
      {3500,.44,.003,.39,.040,.55,.54,.48,.17,.23,6.1,.24,.65,1},
      {2800,.08,1.25,3.6,.014,.43,.01,.48,.11,.62,.3,.31,.49,2},
      {4700,.08,.65,2.6,.007,.49,.06,.75,.05,.74,1.4,.10,.49,2},
      {1600,.6,.003,.16,.018,.35,.62,.05,.08,.09,7.1,.12,.74,1},
      {950,.25,.003,.15,.004,.23,.23,.02,.01,.04,2,.0,.79,0},
      {7100,.09,.003,.19,.014,.57,.09,.28,.33,.32,5.8,.08,.66,3},
      {5500,.1,.17,2.7,.022,.59,.12,.64,.44,.73,.19,.32,.55,0},
      {12400,.17,.006,.32,.027,.6,.30,.48,.25,.45,6.0,.13,.68,0},
      {3000,.32,.008,.41,.016,.42,.31,.3,.16,.21,1.2,.09,.69,1},
      {7600,.10,.012,.45,.006,.43,.06,.15,.05,.17,4.5,.02,.58,3},
      {6500,.08,.022,.70,.009,.46,.04,.18,.08,.23,3.2,.02,.58,3},
      {4100,.08,.34,1.40,.015,.51,.06,.59,.10,.39,2.4,.05,.51,0},
      {750,.13,.003,.065,.004,.28,.29,.015,0,.01,2,.02,.72,0},
      {11200,.10,.004,.22,.025,.57,.22,.41,.24,.22,5.5,.09,.61,0},
      {3700,.21,.003,.095,.014,.38,.24,.22,.17,.15,4,.06,.61,1}
    };
    const char* ids[]={"cutoff","res","attack","release","detune","mix","drive","chorus","delay","reverb","lforate","lfodepth","gain","wave"};
    if(index<0||index>=int(sizeof(data)/sizeof(data[0])))return;
    for(int i=0;i<14;i++)if(auto* p=state.getParameter(ids[i]))p->setValueNotifyingHost(p->convertTo0to1(data[index][i]));
    // Factory recall also replaces the FX mode, so settings from a previous
    // patch cannot carry into a bass, choir or lead unexpectedly.
    const auto setChoice=[this](const char* id,float value){
        auto* parameter=state.getParameter(id);
        parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
    };
    static constexpr int divisions[]{2,3,3,2,4,2,1,0,3,4,2,2,2,3,4,0,3,1};
    static constexpr int spaces[]{1,1,2,1,2,2,0,0,1,2,1,0,0,2,2,0,1,0};
    static_assert(sizeof(divisions)/sizeof(divisions[0])==sizeof(data)/sizeof(data[0]));
    static_assert(sizeof(spaces)/sizeof(spaces[0])==sizeof(data)/sizeof(data[0]));
    setChoice("source",index==12?1.0f:index==13?2.0f:0.0f);
    setChoice("delaySync",float(divisions[index]));
    setChoice("delayMode",divisions[index]>=3?1.0f:0.0f);
    setChoice("reverbSpace",float(spaces[index]));
    setChoice("compAmount",0.0f);
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new SpaceInfinityProcessor();}
