#pragma once
#include "PluginProcessor.h"
class SpaceInfinityEditor : public juce::AudioProcessorEditor {
public:
    explicit SpaceInfinityEditor(SpaceInfinityProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    SpaceInfinityProcessor& processor;
    juce::ComboBox presets, waveform;
    juce::Label title, subtitle;
    std::array<juce::Slider,13> knobs;
    std::array<juce::Label,13> labels;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveAttachment;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpaceInfinityEditor)
};