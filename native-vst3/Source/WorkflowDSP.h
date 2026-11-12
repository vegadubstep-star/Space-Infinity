#pragma once
#include "StudioEQ.h"
namespace studio {
class WetEQ {
public:
    void prepare(double rate){sr=rate;for(auto& band:bands)band=Band{};}
    void configure(const std::array<float,9>& values){
        active=false;for(int i=0;i<3;++i){active|=std::abs(values[size_t(i*3+1)])>.00001f;auto co=eqCoefficients(sr,values[size_t(i*3)],values[size_t(i*3+2)],values[size_t(i*3+1)],i==0?1:i==2?2:0,1,0);
            std::array<float,5> next{co[0]/co[3],co[1]/co[3],co[2]/co[3],co[4]/co[3],co[5]/co[3]};auto& band=bands[size_t(i)];if(next!=band.target){band.target=next;band.remaining=64;for(int k=0;k<5;++k)band.step[size_t(k)]=(next[size_t(k)]-band.co[size_t(k)])/64;}}
        if(!active)for(auto& band:bands){band.co=band.target;band.remaining=0;band.z={};}
    }
    std::array<float,2> process(float l,float r){if(!active)return {l,r};std::array<float,2> result{l,r};for(auto& b:bands){if(b.remaining>0){for(int k=0;k<5;++k)b.co[size_t(k)]+=b.step[size_t(k)];if(--b.remaining==0)b.co=b.target;}for(int ch=0;ch<2;++ch){auto& z=b.z[size_t(ch)];const float in=result[size_t(ch)],out=b.co[0]*in+z[0];z[0]=b.co[1]*in-b.co[3]*out+z[1];z[1]=b.co[2]*in-b.co[4]*out;result[size_t(ch)]=out;}}return result;}
private:
    struct Band{std::array<float,5> co{1,0,0,0,0},target{1,0,0,0,0},step{};std::array<std::array<float,2>,2> z{};int remaining=0;};
    double sr=48000;bool active=false;std::array<Band,3> bands;
};
class SmoothFlanger {
public:
    void prepare(double sampleRate){sr=sampleRate;for(auto& b:buffer)b.assign(size_t(sr*.05)+8,0);position=0;phase=0;for(auto* s:{&rate,&depth,&delay,&feedback,&stereo,&mix})s->reset(sr,.035);rate.setCurrentAndTargetValue(.23f);depth.setCurrentAndTargetValue(.28f);delay.setCurrentAndTargetValue(1.2f);feedback.setCurrentAndTargetValue(0);stereo.setCurrentAndTargetValue(.25f);mix.setCurrentAndTargetValue(0);}
    void configure(float hz,float amount,float milliseconds,float fb,float degrees,float wet){rate.setTargetValue(hz);depth.setTargetValue(amount);delay.setTargetValue(milliseconds);feedback.setTargetValue(juce::jlimit(-.90f,.90f,fb));stereo.setTargetValue(degrees/360);mix.setTargetValue(wet);}
    std::array<float,2> process(float l,float r){const float d=delay.getNextValue(),a=depth.getNextValue(),fb=feedback.getNextValue(),spread=stereo.getNextValue(),wet=mix.getNextValue();std::array<float,2> out{l,r};phase+=rate.getNextValue()/sr;phase-=std::floor(phase);
        for(int ch=0;ch<2;++ch){const float modulation=.5f+.5f*std::sin(float((phase+ch*spread)*juce::MathConstants<double>::twoPi));const double samples=juce::jlimit(1.0,double(buffer[0].size()-3),sr*.001*(d+modulation*a*6));double at=double(position)-samples;if(at<0)at+=buffer[0].size();const size_t i=size_t(at);const float echo=buffer[size_t(ch)][i]+float(at-i)*(buffer[size_t(ch)][(i+1)%buffer[0].size()]-buffer[size_t(ch)][i]);buffer[size_t(ch)][position]=std::tanh(out[size_t(ch)]+echo*fb);out[size_t(ch)]=(1-wet)*out[size_t(ch)]+wet*echo;}
        position=(position+1)%buffer[0].size();return out;}
private:
    double sr=48000,phase=0;size_t position=0;std::array<std::vector<float>,2> buffer;juce::SmoothedValue<float> rate,depth,delay,feedback,stereo,mix;
};
struct EditableCurve {
    int count=2;std::array<float,33> x{},y{},bend{};
    float value(float phase)const{phase=juce::jlimit(0.0f,1.0f,phase);int b=1;while(b<count-1&&x[size_t(b)]<phase)++b;const int a=b-1;float t=juce::jlimit(0.0f,1.0f,(phase-x[size_t(a)])/juce::jmax(.0001f,x[size_t(b)]-x[size_t(a)]));t=std::pow(t,std::exp2(bend[size_t(a)]*3));return y[size_t(a)]+t*(y[size_t(b)]-y[size_t(a)]);}
};
struct LfoTiming {
    float phase=0,delay=0,rise=0,smooth=0;int direction=0;
    float position(double cycle)const {
        if(direction==2){cycle=std::fmod(cycle+phase,2.0);return float(cycle<=1?cycle:2-cycle);}
        cycle=juce::jlimit(0.0,1.0,cycle)+phase;if(cycle>1)cycle-=std::floor(cycle);
        return direction==1?1-float(cycle):float(cycle);
    }
    float output(const EditableCurve& curve,double cycle,double age,double sr,float& memory)const {
        const float start=curve.value(position(0));
        const float depth=age<delay?0:rise<=0?1:juce::jlimit(0.0f,1.0f,float((age-delay)/rise));
        const float target=start+(curve.value(position(cycle))-start)*depth;
        if(age==0||smooth<=0)memory=target;
        else memory+=(target-memory)*float(1-std::exp(-1.0/(sr*smooth*.001)));
        return memory;
    }
};

}
