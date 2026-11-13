#pragma once
#include <JuceHeader.h>
#include <array>
namespace studio {
class Distortion {
public:
    static juce::StringArray names(){return {"Tube","Soft Clip","Hard Clip","Diode 1","Diode 2","Linear Fold","Sine Fold","Zero Square","Downsample","Asymmetric","Rectify","X-Shaper","X-Shaper Asym","Sine Shaper"};}
    struct Settings {int type=0,filterPosition=0;float drive=.2f,mix=1,frequency=6000,q=.7071f,filterMorph=0;std::array<float,9> a{-1,-.75f,-.5f,-.25f,0,.25f,.5f,.75f,1},b{-1,-1,-.9f,-.6f,0,.6f,.9f,1,1};};
    void prepare(double rate){sr=rate;memory={};lastIn={};lastOut={};held={};counter=0;mix=0;drive=.2f;dc=float(std::exp(-2*juce::MathConstants<double>::pi*12/sr));}
    void configure(const Settings& next){s=next;s.type=juce::jlimit(0,13,s.type);s.q=juce::jlimit(.2f,10.0f,s.q);const float g=std::tan(juce::MathConstants<float>::pi*juce::jlimit(20.0f,float(sr*.43),s.frequency)/float(sr));k=1/s.q;g1=g;a1=1/(1+g*(g+k));slew=float(1-std::exp(-1/(sr*.008)));}
    static float shape(float x,int type,float drive,const Settings& s){
        const float gain=std::exp2(drive*6),v=x*gain;
        const auto table=[&](const std::array<float,9>& values,float at){const float pos=juce::jlimit(0.0f,8.0f,(at+1)*4);const int i=juce::jmin(7,int(pos));return values[size_t(i)]+(values[size_t(i+1)]-values[size_t(i)])*(pos-i);};
        switch(type){case 0:return (std::tanh(v+.12f)-std::tanh(.12f))/.99f;case 1:return v/(1+std::abs(v));case 2:return juce::jlimit(-1.0f,1.0f,v);case 3:return std::atan(v*1.5f)/1.5707963f;case 4:return juce::jlimit(-.8f,.8f,std::sin(v*.8f));case 5:{const float t=std::fmod(v+1,4.0f);return 1-std::abs((t<0?t+4:t)-2);}case 6:return std::sin(v);case 7:return std::abs(v)>.06f?(v>0?1.0f:-1.0f):v/.06f;case 8:return x;case 9:return v>=0?std::tanh(v):.62f*std::tanh(v*1.6f);case 10:return std::abs(std::tanh(v));case 11:case 12:{const float at=type==11?std::abs(x):x;const float a=table(s.a,at),b=table(s.b,at);return (a+(b-a)*drive)*(type==11?(x>=0?1:-1):1);}default:return std::sin(v*(1+drive*3));}
    }
    std::array<float,2> process(float left,float right){drive+=slew*(s.drive-drive);mix+=slew*(s.mix-mix);std::array<float,2> out{left,right};const int period=juce::jmax(1,int(1+drive*drive*95));const bool sample=counter++%period==0;
        for(int ch=0;ch<2;++ch){const float dry=out[size_t(ch)];float wet=dry;if(s.filterPosition==1)wet=filter(wet,ch);if(s.type==8){if(sample)held[size_t(ch)]=wet;wet=held[size_t(ch)];}else wet=shape(wet,s.type,drive,s);if(s.filterPosition==2)wet=filter(wet,ch);const float clean=wet-lastIn[size_t(ch)]+dc*lastOut[size_t(ch)];lastIn[size_t(ch)]=wet;lastOut[size_t(ch)]=clean;out[size_t(ch)]=dry+mix*(clean-dry);}return out;}
private:
    float filter(float x,int ch){auto& m=memory[size_t(ch)];const float v3=x-m[1],v1=a1*m[0]+g1*a1*v3,v2=m[1]+g1*v1;m[0]=2*v1-m[0];m[1]=2*v2-m[1];const float lp=v2,bp=k*v1,hp=x-bp-lp,t=juce::jlimit(0.0f,2.0f,s.filterMorph*2);return t<1?lp+(bp-lp)*t:bp+(hp-bp)*(t-1);}
    Settings s;double sr=48000;std::array<std::array<float,2>,2> memory{};std::array<float,2> lastIn{},lastOut{},held{};std::uint64_t counter=0;float drive=.2f,mix=0,dc=.999f,slew=.002f,g1=0,a1=1,k=1.4142f;
};
}
