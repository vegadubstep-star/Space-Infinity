#pragma once
#include <JuceHeader.h>

// Presentation helpers only: durations use the same four-quarter-note bar and
// triplet/dotted factors as the engine. They do not alter automation or DSP.
namespace timingui {
inline juce::String duration(double seconds){
    if(seconds<1)return juce::String(seconds*1000,seconds<.01?2:1)+" ms";
    return juce::String(seconds,2)+" s";
}
inline double secondsFromText(const juce::String& text){return text.getDoubleValue()/(text.containsIgnoreCase("ms")?1000.0:1.0);}
inline double frequencyFromText(const juce::String& text){
    const auto t=text.trim().toLowerCase();const double value=t.getDoubleValue();
    if(t.contains("ms")||(!t.contains("hz")&&(t.endsWith("s")||t.contains("sec"))))return value>0?1/secondsFromText(t):.01;
    return value;
}
inline double cycleSeconds(bool sync,double hz,int division,double bpm,bool triplet,bool dotted,int direction){
    constexpr double bars[]{1,2,4,8,.5,.25,.125,.0625};
    double result=sync?240*bars[juce::jlimit(0,7,division)]/juce::jmax(1.0,bpm):1/juce::jmax(.01,hz);
    if(sync){if(triplet)result/=1.5;if(dotted)result*=1.5;}
    return result*(direction==2?2:1);
}
}
