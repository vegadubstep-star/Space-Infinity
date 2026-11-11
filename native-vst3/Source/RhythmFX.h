#pragma once
#include <JuceHeader.h>
#include <array>
#include <cmath>
namespace studio {
inline constexpr std::array<const char*,15> gateNames{{"Classic 16ths","Offbeat Drive","Gallop","Double Gallop","Triplet Feel","Hard Trance","Anthem Lift","Rolling Bass","Syncopated","Open Eight","Stutter Rise","Chopper","Half Time","Dotted Pulse","Final Rush"}};
inline constexpr std::array<std::array<float,16>,15> gatePatterns{{
    {{1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0}},{{0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1}},
    {{1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1}},{{1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0}},
    {{1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1}},{{1,0,1,.5f,0,1,0,1,1,0,1,0,0,1,1,0}},
    {{1,.7f,.4f,0,1,.7f,.4f,0,1,.7f,.4f,0,1,1,.6f,0}},{{0,1,.5f,1,0,1,.5f,1,0,1,.5f,1,0,1,1,0}},
    {{1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1}},{{1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0}},
    {{.15f,0,.25f,0,.4f,0,.55f,0,.7f,0,.8f,0,1,0,1,0}},{{1,0,.3f,0,1,0,.3f,0,1,0,.3f,0,1,0,.3f,0}},
    {{1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0}},{{1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,0}},
    {{1,0,1,0,1,.4f,1,.4f,1,.7f,1,.7f,1,1,1,0}}
}};
class Rhythm {
public:
    struct Settings {bool gate=false,pump=false;int split=0,rate=2,pumpRate=2,sidechain=0;
        float depth=1,pumpDepth=.7f,attack=3,release=35,pumpAttack=3,pumpRelease=180,threshold=-24,ratio=8,low=180,high=2600;
        std::array<float,16> steps=gatePatterns[0];};
    void prepare(double rate){sr=rate;juce::dsp::ProcessSpec spec{sr,512,2};lo.prepare(spec);hi.prepare(spec);align.prepare(spec);align.setType(juce::dsp::LinkwitzRileyFilterType::allpass);gateGain=1;pumpGain=1;detector=0;beat=0;}
    void configure(const Settings& value,double tempo){s=value;bpm=juce::jlimit(20.0,400.0,tempo);
        lo.setCutoffFrequency(juce::jlimit(20.0f,float(sr*.2),s.low));
        const float high=juce::jlimit(s.low*1.25f,float(sr*.44),s.high);hi.setCutoffFrequency(high);align.setCutoffFrequency(high);}
    void setBeat(double ppq){beat=ppq;}
    void advance(int samples){beat+=samples*bpm/(60*sr);if(beat>1000000)beat-=1000000;}
    std::array<float,2> process(float l,float r,float external){
        static constexpr double lengths[]{.0625,.125,.25,.5,1,2,4};
        const double step=lengths[size_t(juce::jlimit(0,6,s.rate))];
        const int index=int(std::floor(beat/step))%16;displayStep=juce::jlimit(0,15,index);
        const float target=s.gate?1-s.depth+s.depth*s.steps[size_t(displayStep)]:1;
        const float gc=std::exp(-1.0f/(float(sr)*juce::jmax(.0002f,(target<gateGain?s.release:s.attack)*.001f)));
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
    int displayStep=0;float gateGain=1,pumpGain=1;
private:
    Settings s;double sr=48000,bpm=120,beat=0;float detector=0;
    juce::dsp::LinkwitzRileyFilter<float> lo,hi,align;
};
}
