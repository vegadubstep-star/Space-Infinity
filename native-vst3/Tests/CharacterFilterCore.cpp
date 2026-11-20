#include "../Source/CharacterFilters.h"
#include "../Source/SampleBlend.h"
#include <iostream>
#include <stdexcept>
using Filter=infinity::CharacterFilter;
void need(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
int main(){
    // Compare the displayed response to an independently measured impulse DFT.
    for(double sr:{44100.,96000.,192000.})for(int mode=7;mode<=14;++mode){
        Filter filter;Filter::Settings s{mode,937,.65,1.35,.32,sr};
        std::array<double,7> frequencies{83,350,800,937,1700,3500,11000};std::array<std::complex<double>,7> measured{};
        for(int n=0;n<32768;++n){const float y=filter.process(n==0?1.f:0.f,s);need(std::isfinite(y)&&std::abs(y)<3,"unstable impulse response");for(size_t i=0;i<frequencies.size();++i)measured[i]+=double(y)*std::polar(1.0,-2*Filter::pi*frequencies[i]*n/sr);}
        for(size_t i=0;i<frequencies.size();++i)need(std::abs(measured[i]-Filter::response(frequencies[i],s))<.0001,"graph differs from measured filter audio");
    }
    Filter::Settings s;s.cutoff=1000;s.mode=7;need(std::abs(Filter::response(1000,s))<1.e-8,"notch does not reject cutoff");s.mode=8;for(double hz:{40.,1000.,18000.})need(std::abs(std::abs(Filter::response(hz,s))-1)<1.e-8,"all-pass changes gain");s.mode=9;need(std::abs(Filter::response(100,s))<.001,"24dB HP low-frequency attenuation");s.mode=10;need(std::abs(Filter::response(1000,s))>.9,"bandpass lost centre");
    s.mode=11;s.resonance=.7;s.vowel=0;const auto a=Filter::response(800,s);s.vowel=2;need(std::abs(Filter::response(800,s)-a)>.2,"vowel morph has no character");
    // Stress real automation at maximum resonance, including comb delay motion.
    for(int mode=7;mode<=14;++mode){Filter f;s.mode=mode;s.resonance=.95;for(int n=0;n<192000;++n){s.cutoff=50*std::pow(320.0,(.5-.5*std::cos(n*.00012)));s.vowel=2+2*std::sin(n*.0002);s.morph=.5+.5*std::sin(n*.0003);const float y=f.process(.2f*std::sin(n*.11f),s);need(std::isfinite(y)&&std::abs(y)<12,"automation produces unstable output");}}
    std::array<float,4096> ramp{},constantL{},constantR{};for(int n=0;n<4096;++n){ramp[size_t(n)]=float(n)/2047.5f-1;constantL[size_t(n)]=.7f;constantR[size_t(n)]=.2f;}
    for(bool reverse:{false,true}){double position=reverse?4095:0;float previous=0;double jump=0;for(int n=0;n<20000;++n){const auto y=infinity::SampleCloud::loop(ramp.data(),ramp.data(),0,4095,position,512,reverse);if(n>0)jump=std::max(jump,double(std::abs(y[0]-previous)));previous=y[0];position+=reverse?-1:1;}need(jump<.01,"crossfaded loop seam clicks");}
    for(float size:{5.f,90.f,500.f}){infinity::SampleCloud cloud;for(int n=0;n<96000;++n){const auto y=cloud.next(constantL.data(),constantR.data(),0,4095,1,48000,.4f,size,1,.5f,.5f,false);if(n>1000)need(std::abs(y[0]-.7f)<.00001&&std::abs(y[1]-.2f)<.00001,"granular density creates gaps or stereo gain pumping");}}
    std::cout<<"Eight character filters: measured response, vowel contrast, attenuation, maximum resonance and moving cutoff passed\n";
}
