#pragma once
#include <JuceHeader.h>
#include <array>
#include <vector>
#include <cmath>
namespace infinity {
// Four-times oversampling is used for peak detection only: below the ceiling
// the audio is an exact, latency-compensated copy. Stereo gain stays linked.
class TruePeakSafety {
    juce::dsp::Oversampling<float> meter{2,2,juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple,true,true};
    std::vector<std::array<float,2>> delay;
    std::vector<uint8_t> cleanFlags;
    size_t position=0;float detector=0,gain=1,peakDecay=0,attack=1,release=1;int latency=0;
public:
    void prepare(double sampleRate,int maximumBlock){
        meter.initProcessing(size_t(std::max(1,maximumBlock)));
        latency=int(std::ceil(meter.getLatencyInSamples()))+std::max(1,int(std::ceil(sampleRate*.001)));
        delay.resize(size_t(latency));cleanFlags.resize(size_t(latency));const double highRate=sampleRate*4;
        peakDecay=float(std::exp(-1/(highRate*.05)));attack=float(1-std::exp(-1/(highRate*.0001)));release=float(1-std::exp(-1/(highRate*.005)));reset();
    }
    int latencySamples()const{return latency;}
    void reset(){meter.reset();std::fill(delay.begin(),delay.end(),std::array<float,2>{});std::fill(cleanFlags.begin(),cleanFlags.end(),0);position=0;detector=0;gain=1;}
    void process(juce::dsp::AudioBlock<float> audio,bool clean){
        const auto high=meter.processSamplesUp(audio);
        for(size_t n=0;n<audio.getNumSamples();++n){float frameGain=1;
            for(size_t phase=0;phase<4;++phase){const auto i=n*4+phase;const float peak=std::max(std::abs(high.getSample(0,int(i))),std::abs(high.getSample(1,int(i))));detector=std::max(peak,detector*peakDecay);const float desired=std::min(1.f,.92f/std::max(.92f,detector));gain+=(desired-gain)*(desired<gain?attack:release);frameGain=std::min(frameGain,gain);}
            const std::array<float,2> input{audio.getSample(0,int(n)),audio.getSample(1,int(n))},old=delay[position];const bool protect=cleanFlags[position]!=0;delay[position]=input;cleanFlags[position]=clean?1:0;position=(position+1)%delay.size();
            if(protect){const float peak=std::max(std::abs(old[0]),std::abs(old[1]));if(peak>.94f)frameGain=std::min(frameGain,.94f/peak);}else frameGain=1;
            audio.setSample(0,int(n),old[0]*frameGain);audio.setSample(1,int(n),old[1]*frameGain);
        }
    }
};
}
