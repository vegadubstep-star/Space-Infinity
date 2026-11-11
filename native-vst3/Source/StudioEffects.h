#pragma once
#include <JuceHeader.h>
#include <array>
#include <cmath>
#include <vector>

namespace studio {
// Independent stereo-linked, three-band upwards/downwards dynamics.
class Compressor {
public:
    struct Settings {
        int mode=2;float threshold=-18,ratio=4,attack=9,release=110,below=1;
        float low=180,high=2600,gain=0,output=0;bool limit=false;
        std::array<float,3> bandGain{},offset{};
    };
    void prepare(double rate){
        sr=rate;juce::dsp::ProcessSpec spec{rate,512,2};
        lo.prepare(spec);hi.prepare(spec);align.prepare(spec);
        align.setType(juce::dsp::LinkwitzRileyFilterType::allpass);envelope={};gainDb={};
        low.reset(rate,.04);high.reset(rate,.04);makeup.reset(rate,.025);
        low.setCurrentAndTargetValue(180);high.setCurrentAndTargetValue(2600);makeup.setCurrentAndTargetValue(1);
    }
    void configure(const Settings& value,int block){
        s=value;s.ratio=juce::jmax(1.0f,s.ratio);s.below=juce::jlimit(.25f,4.0f,s.below);
        low.setTargetValue(juce::jlimit(20.0f,float(sr*.2),s.low));
        high.setTargetValue(juce::jlimit(s.low*1.25f,float(sr*.44),s.high));
        const float l=low.skip(block),h=juce::jmax(l*1.25f,high.skip(block));
        lo.setCutoffFrequency(l);hi.setCutoffFrequency(h);align.setCutoffFrequency(h);
        attack=std::exp(-1.0f/(float(sr)*juce::jmax(.0001f,s.attack*.001f)));
        release=std::exp(-1.0f/(float(sr)*juce::jmax(.001f,s.release*.001f)));
        makeup.setTargetValue(juce::Decibels::decibelsToGain(s.gain+s.output));
        input={};reduction={};
    }
    std::array<float,2> process(float left,float right){
        std::array<std::array<float,2>,3> bands{};
        for(int ch=0;ch<2;++ch){
            const float dry=ch==0?left:right;float upper;
            lo.processSample(ch,dry,bands[0][size_t(ch)],upper);
            hi.processSample(ch,upper,bands[1][size_t(ch)],bands[2][size_t(ch)]);
            bands[0][size_t(ch)]=align.processSample(ch,bands[0][size_t(ch)]);
        }
        std::array<float,2> wet{};
        dry={left,right};
        if(s.mode==1){
            const float gain=detector(0,juce::jmax(std::abs(left),std::abs(right)),s.threshold);
            wet={left*gain,right*gain};
        }else {dry={};for(size_t i=0;i<3;++i){
            const float gain=detector(i,juce::jmax(std::abs(bands[i][0]),std::abs(bands[i][1])),s.threshold+s.offset[i])
                *juce::Decibels::decibelsToGain(s.bandGain[i]);
            for(size_t ch=0;ch<2;++ch){wet[ch]+=bands[i][ch]*gain;dry[ch]+=bands[i][ch];}
        }}
        const float gain=makeup.getNextValue();
        for(auto& v:wet){v*=gain;if(s.limit)v=juce::jlimit(-1.0f,1.0f,v);}
        return wet;
    }
    std::array<float,3> input{},reduction{};
    std::array<float,2> dry{};
private:
    float detector(size_t i,float peak,float threshold){
        const float c=peak>envelope[i]?attack:release;envelope[i]=peak+c*(envelope[i]-peak);
        const float db=juce::Decibels::gainToDecibels(envelope[i],-120.0f);
        float target=db>threshold?(threshold-db)*(1-1/s.ratio):
            (s.mode==2&&db>-100?(threshold-db)*(1-1/s.below):0);
        target=juce::jlimit(-80.0f,36.0f,target);
        const float smoothing=target<gainDb[i]?attack:release;
        gainDb[i]=target+smoothing*(gainDb[i]-target);
        input[i]=juce::jmax(input[i],peak);reduction[i]=juce::jmax(reduction[i],-gainDb[i]);
        return juce::Decibels::decibelsToGain(gainDb[i]);
    }
    Settings s;double sr=48000;float attack=0,release=0;
    std::array<float,3> envelope{},gainDb{};
    juce::dsp::LinkwitzRileyFilter<float> lo,hi,align;
    juce::SmoothedValue<float> low,high,makeup;
};

class Delay {
public:
    struct Settings {int mode=1;bool hq=true;float left=.375f,right=.375f,feedback=.35f,frequency=5700,bandwidth=.8f,output=0;};
    void prepare(double rate){
        sr=rate;const int n=int(rate*4)+8;
        for(auto& b:buffer)b.assign(size_t(n),0);position=0;hp={};lp={};
        left.reset(rate,.05);right.reset(rate,.05);feedback.reset(rate,.03);output.reset(rate,.025);
        frequency.reset(rate,.03);bandwidth.reset(rate,.03);
        left.setCurrentAndTargetValue(float(rate*.375));right.setCurrentAndTargetValue(float(rate*.375));
        feedback.setCurrentAndTargetValue(.35f);output.setCurrentAndTargetValue(1);
        frequency.setCurrentAndTargetValue(5700);bandwidth.setCurrentAndTargetValue(.8f);
    }
    void configure(const Settings& value){
        s=value;left.setTargetValue(juce::jlimit(1.0f,float(buffer[0].size()-5),float(sr)*s.left));
        right.setTargetValue(juce::jlimit(1.0f,float(buffer[0].size()-5),float(sr)*s.right));
        feedback.setTargetValue(juce::jlimit(0.0f,.95f,s.feedback));output.setTargetValue(juce::Decibels::decibelsToGain(s.output));
        frequency.setTargetValue(juce::jlimit(20.0f,float(sr*.4),s.frequency));bandwidth.setTargetValue(juce::jlimit(0.0f,1.0f,s.bandwidth));
    }
    std::array<float,2> process(float l,float r){
        const float freq=frequency.getNextValue(),span=std::exp2(.15f+bandwidth.getNextValue()*6);
        const float hc=float(1-std::exp(-juce::MathConstants<double>::twoPi*juce::jlimit(10.0f,float(sr*.4),freq/span)/sr));
        const float lc=float(1-std::exp(-juce::MathConstants<double>::twoPi*juce::jlimit(20.0f,float(sr*.45),freq*span)/sr));
        std::array<float,2> wet{read(0,left.getNextValue()),read(1,right.getNextValue())};
        for(size_t ch=0;ch<2;++ch){hp[ch]+=hc*(wet[ch]-hp[ch]);lp[ch]+=lc*(wet[ch]-hp[ch]-lp[ch]);wet[ch]=lp[ch];}
        const float fb=feedback.getNextValue();
        if(s.mode==2){buffer[0][position]=(l+r)*.5f+wet[1]*fb;buffer[1][position]=wet[0]*fb;}
        else if(s.mode==3){buffer[0][position]=(l+r)*.5f;buffer[1][position]=wet[0]+wet[1]*fb;}
        else{buffer[0][position]=l+wet[0]*fb;buffer[1][position]=r+wet[1]*fb;}
        position=(position+1)%buffer[0].size();const float gain=output.getNextValue();
        return {wet[0]*gain,wet[1]*gain};
    }
private:
    float read(size_t ch,float delay)const{
        const auto& b=buffer[ch];double at=double(position)-delay;while(at<0)at+=double(b.size());
        const int a=int(at);const float t=float(at-a);
        const auto get=[&](int n){return b[size_t((n+int(b.size()))%int(b.size()))];};
        if(!s.hq)return get(a)+(get(a+1)-get(a))*t;
        const float p0=get(a-1),p1=get(a),p2=get(a+1),p3=get(a+2);
        return p1+.5f*t*(p2-p0+t*(2*p0-5*p1+4*p2-p3+t*(3*(p1-p2)+p3-p0)));
    }
    Settings s;double sr=48000;size_t position=0;
    std::array<std::vector<float>,2> buffer;
    std::array<float,2> hp{},lp{};
    juce::SmoothedValue<float> left,right,feedback,output,frequency,bandwidth;
};

inline void minimumPhase(juce::AudioBuffer<float>& audio){
    const int count=audio.getNumSamples();int order=1;while((1<<order)<count*2)++order;
    const int size=1<<order;juce::dsp::FFT fft(order);
    std::vector<std::complex<float>> in(size_t(size),std::complex<float>{}),out=in;
    for(int ch=0;ch<audio.getNumChannels();++ch){
        std::fill(in.begin(),in.end(),std::complex<float>{});
        for(int i=0;i<count;++i)in[size_t(i)]={audio.getSample(ch,i),0};
        fft.perform(in.data(),out.data(),false);
        for(int i=0;i<size;++i)in[size_t(i)]={std::log(juce::jmax(1.0e-9f,std::abs(out[size_t(i)]))),0};
        fft.perform(in.data(),out.data(),true);
        for(int i=1;i<size/2;++i)out[size_t(i)]*=2;
        for(int i=size/2+1;i<size;++i)out[size_t(i)]={};
        fft.perform(out.data(),in.data(),false);
        for(auto& v:in)v=std::exp(v);
        fft.perform(in.data(),out.data(),true);
        for(int i=0;i<count;++i)audio.setSample(ch,i,std::isfinite(out[size_t(i)].real())?out[size_t(i)].real():0.0f);
    }
}
}
