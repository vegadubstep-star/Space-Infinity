#pragma once
#include <JuceHeader.h>
#include <array>
namespace studio {
inline std::array<float,6> eqCoefficients(double rate,float frequency,float q,float gain,int mode,int stages,int stage){
    using C=juce::dsp::IIR::ArrayCoefficients<float>;
    frequency=juce::jlimit(20.0f,float(rate*.45),frequency);q=juce::jlimit(.2f,12.0f,q);
    const float level=juce::Decibels::decibelsToGain(gain);
    if(mode==1)return C::makeLowShelf(rate,frequency,q,level);
    if(mode==2)return C::makeHighShelf(rate,frequency,q,level);
    if(mode==3||mode==4){
        const float butterworth=1.0f/(2*std::cos(juce::MathConstants<float>::pi*float(2*stage+1)/float(4*stages)));
        const float resonance=juce::jlimit(.15f,18.0f,butterworth*q);
        return mode==3?C::makeLowPass(rate,frequency,resonance):C::makeHighPass(rate,frequency,resonance);
    }
    if(mode==5)return C::makeNotch(rate,frequency,q);
    if(mode==6)return C::makeBandPass(rate,frequency,q);
    return C::makePeakFilter(rate,frequency,q,level);
}
}
