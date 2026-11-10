#include "../Source/PluginProcessor.h"
#include <cmath>
#include <iostream>

int main()
{
    juce::ScopedJuceInitialiser_GUI gui;
    SpaceInfinityProcessor synth;
    constexpr int blockSize=256;
    juce::AudioBuffer<float> audio(2,blockSize);
    const auto& names=synth.presetNames();
    if(names.size()!=104 || synth.motionNames().size()!=20) return 1;

    float quietest=1.0f, loudest=0.0f;
    for(int index=0;index<names.size();++index){
        if(names[index].isEmpty() || synth.presetCategory(index).isEmpty()) return 2;
        synth.loadPreset(index);
        synth.prepareToPlay(48000.0,blockSize);
        float peak=0.0f;
        for(int block=0;block<75;++block){
            juce::MidiBuffer midi;
            if(block==0) midi.addEvent(juce::MidiMessage::noteOn(1,48,0.8f),0);
            if(block==60) midi.addEvent(juce::MidiMessage::noteOff(1,48),0);
            synth.processBlock(audio,midi);
            for(int channel=0;channel<audio.getNumChannels();++channel)
                for(int i=0;i<blockSize;++i){
                    const float sample=audio.getSample(channel,i);
                    if(!std::isfinite(sample)) return 3;
                    peak=std::max(peak,std::abs(sample));
                }
        }
        std::cout << names[index] << " peak " << peak << '\n';
        if(peak<0.0001f || peak>1.0f) return 4;
        quietest=std::min(quietest,peak);
        loudest=std::max(loudest,peak);
    }
    auto* cutoff=synth.state.getParameter("cutoff");
    auto* point=synth.state.getParameter(synth.motionPointId(3));
    cutoff->setValueNotifyingHost(0.45f);
    point->setValueNotifyingHost(0.73f);
    juce::MemoryBlock saved;
    synth.getStateInformation(saved);
    synth.loadPreset(0);
    synth.setStateInformation(saved.getData(),int(saved.getSize()));
    if(std::abs(cutoff->getValue()-0.45f)>0.001f || std::abs(point->getValue()-0.73f)>0.001f) return 5;

    std::unique_ptr<juce::AudioProcessorEditor> editor(synth.createEditor());
    if(!editor) return 6;
    for(const auto size : {juce::Point<int>(800,760),juce::Point<int>(1000,790),juce::Point<int>(1500,950)}){
        editor->setSize(size.x,size.y);
        for(auto* child : editor->getChildren())
            if(child->isVisible() && !editor->getLocalBounds().contains(child->getBounds())) return 7;
        const auto snapshot=editor->createComponentSnapshot(editor->getLocalBounds());
        if(!snapshot.isValid() || snapshot.getWidth()!=size.x || snapshot.getHeight()!=size.y) return 8;
    }
    std::cout << "Factory peaks " << quietest << " to " << loudest << '\n';
}
