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

    for(int i=0;i<names.size();++i)
        if(names.indexOf(names[i])!=i) return 9;
    std::unique_ptr<juce::AudioProcessorEditor> editor(synth.createEditor());
    if(!editor) return 6;
    juce::ComboBox* factoryMenu=nullptr;
    for(auto* child:editor->getChildren()){
        auto* box=dynamic_cast<juce::ComboBox*>(child);
        if(box && box->getTextWhenNothingSelected()=="Select a sound") factoryMenu=box;
    }
    if(!factoryMenu) return 10;
    auto* cutoff=synth.state.getParameter("cutoff");

    float quietest=1.0f, loudest=0.0f;
    for(int index=0;index<names.size();++index){
        if(names[index].isEmpty() || synth.presetCategory(index).isEmpty()) return 2;
        synth.loadPreset(index);
        const auto expectedCutoff=cutoff->getValue();
        cutoff->setValueNotifyingHost(expectedCutoff>0.5f?0.0f:1.0f);
        factoryMenu->setSelectedId(index+1,juce::sendNotificationSync);
        if(factoryMenu->getText()!=names[index] || std::abs(cutoff->getValue()-expectedCutoff)>0.001f)
            return 11;
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
    auto* point=synth.state.getParameter(synth.motionPointId(3));
    cutoff->setValueNotifyingHost(0.45f);
    point->setValueNotifyingHost(0.73f);
    juce::MemoryBlock saved;
    synth.getStateInformation(saved);
    synth.loadPreset(0);
    synth.setStateInformation(saved.getData(),int(saved.getSize()));
    if(std::abs(cutoff->getValue()-0.45f)>0.001f || std::abs(point->getValue()-0.73f)>0.001f) return 5;

    for(const auto rate:{44100.0,96000.0}){
        for(const auto frames:{64,512}){
            synth.loadPreset(0);
            synth.prepareToPlay(rate,frames);
            juce::AudioBuffer<float> buffer(2,frames);
            juce::MidiBuffer events;
            events.addEvent(juce::MidiMessage::noteOn(1,60,0.8f),0);
            synth.processBlock(buffer,events);
            const auto peak=buffer.getMagnitude(0,frames);
            if(!std::isfinite(peak)||peak<=0.0f||peak>1.0f) return 12;
        }
    }
    for(int shape=0;shape<20;++shape){
        synth.loadMotionShape(shape);
        for(int i=0;i<17;++i){
            const auto value=synth.state.getParameter(synth.motionPointId(i))->getValue();
            if(!std::isfinite(value)||value<0.0f||value>1.0f) return 13;
        }
    }
    for(const auto size : {juce::Point<int>(800,760),juce::Point<int>(1000,790),juce::Point<int>(1500,950)}){
        editor->setSize(size.x,size.y);
        for(auto* child : editor->getChildren())
            if(child->isVisible() && !editor->getLocalBounds().contains(child->getBounds())) return 7;
        const auto snapshot=editor->createComponentSnapshot(editor->getLocalBounds());
        if(!snapshot.isValid() || snapshot.getWidth()!=size.x || snapshot.getHeight()!=size.y) return 8;
    }
    std::cout << "Factory peaks " << quietest << " to " << loudest << '\n';
}
