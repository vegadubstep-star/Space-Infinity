#pragma once
#include "TimingReadout.h"

namespace graphui {
inline juce::String number(double v){return juce::String(v,2).trimCharactersAtEnd("0").trimCharactersAtEnd(".");}
inline juce::String time(double seconds,double span=-1){const bool ms=(span<0?seconds:span)<1;return number(seconds*(ms?1000:1))+(ms?"ms":"s");}
inline juce::Rectangle<float> plot(juce::Rectangle<int> bounds){return bounds.toFloat().withTrimmedLeft(30).withTrimmedRight(9).withTrimmedTop(5).withTrimmedBottom(14);}
inline void levels(juce::Graphics& g,juce::Rectangle<float> r){
    g.setFont(juce::Font(juce::FontOptions(9)));g.setColour(juce::Colour(0xffb0a8c7));
    for(int n=0;n<3;++n){if(n==1&&r.getHeight()<32)continue;const float y=r.getY()+r.getHeight()*n/2;
        g.drawText(juce::String(100-n*50)+"%",juce::Rectangle<float>(0,y-5,r.getX()-4,10),juce::Justification::centredRight);
        g.setColour(juce::Colour(0xff62516f));g.drawLine(r.getX()-3,y,r.getX(),y,.7f);g.setColour(juce::Colour(0xffb0a8c7));}
}
inline void times(juce::Graphics& g,juce::Rectangle<float> r,double seconds){
    g.setFont(juce::Font(juce::FontOptions(9)));g.setColour(juce::Colour(0xffb5c9d4));const int divisions=r.getWidth()<210?2:4;
    const float width=r.getWidth()/divisions;
    for(int i=0;i<=divisions;++i){const float x=r.getX()+r.getWidth()*i/divisions;g.drawLine(x,r.getBottom(),x,r.getBottom()+2,.7f);
        auto label=juce::Rectangle<float>(x-width*.5f,r.getBottom()+3,width,10);if(i==0)label.setX(r.getX());if(i==divisions)label.setX(r.getRight()-width);
        g.drawText(time(seconds*i/divisions,seconds),label,i==0?juce::Justification::centredLeft:i==divisions?juce::Justification::centredRight:juce::Justification::centred);}
}
inline void badge(juce::Graphics& g,juce::Rectangle<float> plot,const juce::String& text){
    const float w=juce::jmin(plot.getWidth(),float(text.length())*5.1f+10);auto r=juce::Rectangle<float>(plot.getRight()-w,plot.getY()+2,w,14);
    g.setColour(juce::Colour(0xf20e0a17));g.fillRoundedRectangle(r,2);g.setColour(juce::Colour(0xffaff9eb));g.setFont(juce::Font(juce::FontOptions(9)));g.drawText(text,r.reduced(3,0),juce::Justification::centred);
}
}
