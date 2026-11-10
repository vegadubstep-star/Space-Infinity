#include "PluginProcessor.h"
#include "PluginEditor.h"
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
    return p;
}
float osc(double x,int wave){ x-=std::floor(x); switch(wave){case 1:return x<0.48?1.0f:-1.0f;case 2:return std::sin(float(juce::MathConstants<double>::twoPi*x));case 3:return float(4*std::abs(x-0.5)-1);default:return float(2*x-1);} }
}
SpaceInfinityProcessor::SpaceInfinityProcessor():juce::AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"PARAMETERS",makeLayout()){}
void SpaceInfinityProcessor::prepareToPlay(double sampleRate,int maxSamples){
    sr=sampleRate;
    juce::dsp::ProcessSpec spec{sampleRate,(juce::uint32)std::max(maxSamples,1),2};chorus.prepare(spec);chorus.setRate(0.25f);chorus.setDepth(0.38f);chorus.setCentreDelay(9);chorus.setFeedback(0.1f);chorus.setMix(1.0f);reverb.setSampleRate(sr);
    delayL.prepare({sampleRate,(juce::uint32)std::max(maxSamples,1),1});delayR.prepare({sampleRate,(juce::uint32)std::max(maxSamples,1),1});delayL.reset();delayR.reset();
    for(auto& v:voices)v={}; lfoPhase=0; clock=0;
}
void SpaceInfinityProcessor::noteOn(int note,float velocity){
    auto* found=&voices[0]; for(auto& v:voices) if(!v.active){found=&v;break;}
    if(found->active)found=&*std::min_element(voices.begin(),voices.end(),[](const Voice& a,const Voice& b){return a.age<b.age;});
    *found={};found->active=true;found->note=note;found->velocity=velocity;found->age=++clock;
}
void SpaceInfinityProcessor::noteOff(int note){for(auto& v:voices) if(v.active&&v.note==note)v.released=true;}
void SpaceInfinityProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer& midi){
    juce::ScopedNoDenormals noDenormals;
    b.clear(); if(b.getNumChannels()<2)return;
    auto* left=b.getWritePointer(0);auto* right=b.getWritePointer(1);
    const float attack=param("attack"),release=param("release"),mix=param("mix"),detune=param("detune"),cutoff=param("cutoff"),res=param("res"),drive=param("drive"),lfoRate=param("lforate"),lfoDepth=param("lfodepth");
    const int wave=int(param("wave"));
    auto msg=midi.begin();
    for(int s=0;s<b.getNumSamples();++s){
        while(msg!=midi.end()&&(*msg).samplePosition<=s){const auto m=(*msg).getMessage(); if(m.isNoteOn())noteOn(m.getNoteNumber(),m.getFloatVelocity());else if(m.isNoteOff())noteOff(m.getNoteNumber());else if(m.isAllNotesOff()||m.isAllSoundOff()){for(auto& v:voices)v.released=true;}++msg;}
        lfoPhase+=lfoRate/sr;if(lfoPhase>=1)lfoPhase-=1;
        float out=0;const float lfo=std::sin(float(juce::MathConstants<double>::twoPi*lfoPhase));
        for(auto& v:voices){if(!v.active)continue;
            if(v.released)v.env*=std::exp(-1.0/(std::max(0.001f,release)*sr));else v.env+=(1-v.env)*(1-std::exp(-1.0/(std::max(0.001f,attack)*sr)));
            if(v.released&&v.env<0.0001f){v.active=false;continue;}
            const double freq=440.0*std::pow(2.0,(v.note-69.0)/12.0);
            v.phase+=freq/sr;v.phase2+=freq*(1.0+detune+0.003*lfo*lfoDepth)/sr;
            if(v.phase>=1)v.phase-=std::floor(v.phase);if(v.phase2>=1)v.phase2-=std::floor(v.phase2);
            const float raw=osc(v.phase,wave)*(1-mix)+osc(v.phase2,wave)*mix;
            // One-pole lowpass with resonance-dependent input emphasis. Not a model of JV hardware filters.
            const float fc=juce::jlimit(50.0f,float(sr*0.4),cutoff*std::pow(2.0f,lfo*lfoDepth*2));
            const float a=1-std::exp(float(-juce::MathConstants<double>::twoPi*fc/sr));
            v.cutoff+=a*(raw*(1+res*0.7f)-v.cutoff);
            out+=v.cutoff*v.velocity*v.env*0.14f;
        }
        out=std::tanh(out*(1+drive*9));left[s]=out;right[s]=out;
    }
    // Modulation chorus, stereo delay, and algorithmic reverb; no external ROMs are included.
    juce::AudioBuffer<float> wet;wet.makeCopyOf(b);
    juce::dsp::AudioBlock<float> block(wet);
    juce::dsp::ProcessContextReplacing<float> context(block);chorus.process(context);
    const float ch=param("chorus");
    for(int s=0;s<b.getNumSamples();++s){left[s]=left[s]*(1-ch)+wet.getSample(0,s)*ch;right[s]=right[s]*(1-ch)+wet.getSample(1,s)*ch;}
    const int n=juce::jlimit(1,190000,int(sr*0.375));delayL.setDelay(float(n));delayR.setDelay(float(n*0.75));
    const float d=param("delay");
    for(int s=0;s<b.getNumSamples();++s){auto dl=delayL.popSample(0),dr=delayR.popSample(0);delayL.pushSample(0,left[s]+dr*0.35f);delayR.pushSample(0,right[s]+dl*0.35f);left[s]+=dl*d;right[s]+=dr*d;}
    auto params=reverb.getParameters();params.roomSize=0.72f;params.damping=0.45f;params.wetLevel=param("reverb")*0.65f;params.dryLevel=1-params.wetLevel*0.35f;params.width=1;reverb.setParameters(params);reverb.processStereo(left,right,b.getNumSamples());
    b.applyGain(param("gain"));for(int c=0;c<2;++c){auto* dst=b.getWritePointer(c);for(int s=0;s<b.getNumSamples();++s)dst[s]=std::tanh(dst[s]);}
}
void SpaceInfinityProcessor::getStateInformation(juce::MemoryBlock& m){auto t=state.copyState();std::unique_ptr<juce::XmlElement> xml(t.createXml());copyXmlToBinary(*xml,m);}
void SpaceInfinityProcessor::setStateInformation(const void* data,int size){auto xml=getXmlFromBinary(data,size);if(xml&&xml->hasTagName(state.state.getType()))state.replaceState(juce::ValueTree::fromXml(*xml));}
const juce::StringArray& SpaceInfinityProcessor::presetNames(){static const juce::StringArray names{"Classic Rave Saw","German HT Lead","Wide Anthem","Dark Hoover","Soft Dream Pad","Bright Choir-Like","Acid Stab","Rolling Bass","Trance Pluck","Epic Breakdown","Hard Dance Hook","Retro Synth Brass"};return names;}
void SpaceInfinityProcessor::loadPreset(int index){
    // Original factory-style sound-design presets; not Roland patch data.
    const float data[12][14]={
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
      {3000,.32,.008,.41,.016,.42,.31,.3,.16,.21,1.2,.09,.69,1}
    };
    const char* ids[]={"cutoff","res","attack","release","detune","mix","drive","chorus","delay","reverb","lforate","lfodepth","gain","wave"};
    if(index<0||index>=12)return;
    for(int i=0;i<14;i++)if(auto* p=state.getParameter(ids[i]))p->setValueNotifyingHost(p->convertTo0to1(data[index][i]));
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new SpaceInfinityProcessor();}