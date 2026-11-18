#pragma once
#include <JuceHeader.h>
#include <array>
#include <vector>

namespace studio {
// Original stereo dimension algorithm: opposed slow delay modulation, a
// filtered/saturated wet path and a dry-preserving stereo crossmix. No noise.
class DimensionChorus {
public:
 void prepare(double rate){sr=rate;const auto size=size_t(sr*.06)+8;for(auto& b:buffer)b.assign(size,0);position=0;phase=phase2=0;hpIn={};hpOut={};lp={};hp=std::exp(-juce::MathConstants<double>::twoPi*120/sr);
  for(auto* value:{&speed,&delay,&depth,&width,&tone,&mix})value->reset(sr,.04);
  speed.setCurrentAndTargetValue(.25f);delay.setCurrentAndTargetValue(8);depth.setCurrentAndTargetValue(.6f);width.setCurrentAndTargetValue(1);tone.setCurrentAndTargetValue(float(1-std::exp(-juce::MathConstants<double>::twoPi*7000/sr)));mix.setCurrentAndTargetValue(0);
 }
 void configure(int mode,float spread,float colour,float wet){mode=juce::jlimit(0,3,mode);constexpr float rates[]{.25f,.4f,.55f,.7f},delays[]{8,10,12,14},depths[]{.6f,1,1.5f,2.2f};speed.setTargetValue(rates[mode]);delay.setTargetValue(delays[mode]);depth.setTargetValue(depths[mode]);width.setTargetValue(juce::jlimit(0.0f,1.5f,spread));const double hz=2000*std::pow(6.0,juce::jlimit(0.0f,1.0f,colour));tone.setTargetValue(float(1-std::exp(-juce::MathConstants<double>::twoPi*juce::jmin(hz,sr*.4)/sr)));mix.setTargetValue(juce::jlimit(0.0f,1.0f,wet));}
 std::array<float,2> process(float left,float right){const float rate=speed.getNextValue(),base=delay.getNextValue(),amount=depth.getNextValue(),spread=width.getNextValue(),co=tone.getNextValue(),wet=mix.getNextValue();phase+=rate/sr;phase-=std::floor(phase);phase2+=(rate*.77+.031)/sr;phase2-=std::floor(phase2);std::array<float,2> dry{left,right},high{},echo{};
  for(int ch=0;ch<2;++ch){high[size_t(ch)]=float(hp)*(hpOut[size_t(ch)]+dry[size_t(ch)]-hpIn[size_t(ch)]);hpIn[size_t(ch)]=dry[size_t(ch)];hpOut[size_t(ch)]=high[size_t(ch)];buffer[size_t(ch)][position]=std::tanh(high[size_t(ch)]*1.1f)/1.1f;
   const float sign=ch==0?1.0f:-1.0f;const double motion=sign*(.8*std::sin(juce::MathConstants<double>::twoPi*phase)+.2*std::sin(juce::MathConstants<double>::twoPi*phase2));const double samples=sr*.001*(base+amount*motion);double at=double(position)-samples;if(at<0)at+=buffer[0].size();const auto i=size_t(at);const float read=buffer[size_t(ch)][i]+float(at-i)*(buffer[size_t(ch)][(i+1)%buffer[0].size()]-buffer[size_t(ch)][i]);lp[size_t(ch)]+=co*(read-lp[size_t(ch)]);echo[size_t(ch)]=lp[size_t(ch)];}
  position=(position+1)%buffer[0].size();if(wet==0)return dry;
  const float mid=(echo[0]+echo[1])*.5f,side=(echo[0]-echo[1])*.5f*spread,correction=.15f*(mid-(high[0]+high[1])*.5f);return {left+wet*(correction+.8f*side),right+wet*(correction-.8f*side)};
 }
private:
 double sr=48000,phase=0,phase2=0,hp=0;size_t position=0;std::array<std::vector<float>,2> buffer;std::array<float,2> hpIn{},hpOut{},lp{};juce::SmoothedValue<float> speed,delay,depth,width,tone,mix;
};
}
