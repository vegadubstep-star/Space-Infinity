#pragma once
#include "WorkflowDSP.h"
#include <JuceHeader.h>
#include <array>
#include <cmath>
namespace studio {
inline constexpr std::array<const char*,15> gateNames{{"Classic 16ths","Classic 8ths","Offbeat Trance","Offbeat Stabs","Trance Gallop","Double Gallop","Anthem Hold and Chop","Syncopated Anthem","Hard Trance Chopper","Dotted Trance","Triplet Trance","Half-Bar Hold","Alternating Accents","Breakdown to Stutter","Quarter Pump"}};
inline constexpr std::array<int,15> gatePresetRates{{1,2,2,2,2,2,2,2,1,2,2,2,1,2,2}};
inline constexpr std::array<std::array<float,16>,15> gatePatterns{{
    {{1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0}},
    {{1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0}},
    {{0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1}},
    {{0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0}},
    {{1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0}},
    {{1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0}},
    {{1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0}},
    {{1,0,0,1,0,1,0,1,1,0,1,0,0,1,0,1}},
    {{1,0,1,0,1,1,0,1,1,0,1,0,0,1,0,1}},
    {{1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,0}},
    {{1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1}},
    {{1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0}},
    {{1,0,0.55f,0,1,0,0.55f,0,1,0,0.55f,0,1,0,0.55f,0}},
    {{1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,0}},
    {{0,0.1f,0.4f,0.8f,0,0.1f,0.4f,0.8f,0,0.1f,0.4f,0.8f,0,0.1f,0.4f,0.8f}},
}};
class Rhythm {
public:
    struct Settings {bool gate=false,pump=false,curveMode=false,retrigger=false,noteGate=false;float phase=0,pwm=.5f,smooth=2;int curveRate=4;studio::EditableCurve curve;int split=0,rate=2,pumpRate=2,sidechain=0;
        float stepWidth=1,delay=0,fade=0;int stepCount=16;float swing=0,depth=1,pumpDepth=.7f,attack=3,release=35,pumpAttack=3,pumpRelease=180,threshold=-24,ratio=8,low=180,high=2600;
        std::array<float,16> steps=gatePatterns[0];};
    void prepare(double rate){sr=rate;juce::dsp::ProcessSpec spec{sr,512,2};lo.prepare(spec);hi.prepare(spec);align.prepare(spec);align.setType(juce::dsp::LinkwitzRileyFilterType::allpass);gateGain=1;pumpGain=1;detector=0;beat=0;age=0;held.fill(false);heldCount=0;}
    void configure(const Settings& value,double tempo){s=value;s.pwm=juce::jlimit(.1f,.9f,s.pwm);s.smooth=juce::jlimit(.1f,50.0f,s.smooth);s.curve.count=juce::jlimit(2,33,s.curve.count);bpm=juce::jlimit(20.0,400.0,tempo);
        if(s.gate&&!lastGate)age=0;lastGate=s.gate;
        lo.setCutoffFrequency(juce::jlimit(20.0f,float(sr*.2),s.low));
        const float high=juce::jlimit(s.low*1.25f,float(sr*.44),s.high);hi.setCutoffFrequency(high);align.setCutoffFrequency(high);}
    void setBeat(double ppq){beat=ppq;}
    void midi(const juce::MidiMessage& m){if(m.isAllNotesOff()||m.isAllSoundOff()){held.fill(false);heldCount=0;}else if(m.isNoteOnOrOff()){const int key=(m.getChannel()-1)*128+m.getNoteNumber();if(m.isNoteOn()){if(!held[size_t(key)])++heldCount;held[size_t(key)]=true;if(heldCount==1)age=0;if(s.retrigger)beat=0;}else if(held[size_t(key)]){held[size_t(key)]=false;--heldCount;}}}
    void advance(int samples){beat+=samples*bpm/(60*sr);if(beat>1000000)beat-=1000000;displayPhase=phaseAtBeat();}
    std::array<float,2> process(float l,float r,float external){
        static constexpr double lengths[]{.0625,.125,.25,.5,1,2,4};
        const double step=lengths[size_t(juce::jlimit(0,6,s.rate))];
        const double pair=std::floor(beat/(step*2));
        const int index=(int(pair)*2+(beat-pair*step*2>=step*(1+juce::jlimit(0.0f,.5f,s.swing))?1:0))%juce::jlimit(1,16,s.stepCount);displayStep=juce::jlimit(0,15,index);
        const float cycle=phaseAtBeat();displayPhase=cycle;
        const double stepPhase=(beat-pair*step*2)/step;const double local=stepPhase<1+s.swing?stepPhase/(1+s.swing):(stepPhase-1-s.swing)/(1-s.swing);
        const float level=s.curveMode?s.curve.value(float(cycle)):(local<s.stepWidth?s.steps[size_t(displayStep)]:0);
        const float fade=age<s.delay?0:s.fade>0?juce::jlimit(0.0f,1.0f,float((age-s.delay)/s.fade)):1;age+=1/sr;const float depth=s.depth*fade;
        const float target=s.gate?1-depth+depth*(s.noteGate&&heldCount==0?0:level):1;
        const float gc=std::exp(-1.0f/(float(sr)*juce::jmax(.0002f,(s.curveMode?s.smooth:(target<gateGain?s.release:s.attack))*.001f)));
        gateGain=target+gc*(gateGain-target);
        float desired=1;
        if(s.pump){
            if(s.sidechain==1){
                const float peak=std::abs(external),c=std::exp(-1.0f/(float(sr)*juce::jmax(.0001f,(peak>detector?s.pumpAttack:s.pumpRelease)*.001f)));
                detector=peak+c*(detector-peak);const float db=juce::Decibels::gainToDecibels(detector,-120.0f);
                desired=juce::Decibels::decibelsToGain(juce::jmin(0.0f,(s.threshold-db)*(1-1/juce::jmax(1.0f,s.ratio))));
                desired=1-s.pumpDepth+s.pumpDepth*desired;
            }else{
                const double cycle=lengths[size_t(juce::jlimit(0,6,s.pumpRate))]*4;
                double phase=std::fmod(beat,cycle);if(phase<0)phase+=cycle;
                desired=1-s.pumpDepth*std::exp(-float(phase*60/bpm)/juce::jmax(.01f,s.pumpRelease*.001f));
            }
        }
        const float pc=std::exp(-1.0f/(float(sr)*juce::jmax(.0002f,s.pumpAttack*.001f)));
        pumpGain=desired+pc*(pumpGain-desired);const float gain=gateGain*pumpGain;
        beat+=bpm/(60*sr);if(beat>1000000)beat-=1000000;
        if(s.split==0)return {l*gain,r*gain};
        std::array<float,2> out{};
        for(int ch=0;ch<2;++ch){float low,upper,mid,high;lo.processSample(ch,ch==0?l:r,low,upper);hi.processSample(ch,upper,mid,high);low=align.processSample(ch,low);
            out[size_t(ch)]=low*(s.split==1?gain:1)+mid*(s.split==2?gain:1)+high*(s.split==3?gain:1);}
        return out;
    }
    float displayPhase=0;int displayStep=0;float gateGain=1,pumpGain=1;
private:
    float phaseAtBeat()const{static constexpr double cycles[]{.0625,.125,.25,.5,1,2,4,8,16,32,64};const double length=cycles[size_t(juce::jlimit(0,10,s.curveRate))];double phase=std::fmod(beat/length+s.phase,1.0);if(phase<0)phase+=1;const double split=.5+.5*juce::jlimit(0.0f,.5f,s.swing);phase=phase<split?phase*.5/split:.5+(phase-split)*.5/(1-split);return float(phase<s.pwm?phase*.5/s.pwm:.5+(phase-s.pwm)*.5/(1-s.pwm));}
    std::array<bool,2048> held{};int heldCount=0;
    Settings s;double sr=48000,bpm=120,beat=0,age=0;bool lastGate=false;float detector=0;
    juce::dsp::LinkwitzRileyFilter<float> lo,hi,align;
};
}
