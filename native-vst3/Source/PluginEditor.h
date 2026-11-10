#pragma once
#include "PluginProcessor.h"
class SpaceInfinityEditor : public juce::AudioProcessorEditor {
public:
    explicit SpaceInfinityEditor(SpaceInfinityProcessor&);
    ~SpaceInfinityEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    class HardwareLook final : public juce::LookAndFeel_V4 {
        void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
        void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
        void drawComboBox(juce::Graphics&, int, int, bool, int, int, int, int, juce::ComboBox&) override;
    };
    HardwareLook hardwareLook;
    SpaceInfinityProcessor& processor;
    juce::ComboBox presets, waveform, source, motionShape, motionBars;
    juce::Slider motionDepth;
    std::array<juce::Slider,4> macros;
    std::array<juce::ComboBox,4> macroRoutes;
    std::array<juce::Slider,17> motionPoints;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> motionDepthAttachment;
    juce::ComboBox delaySync, delayMode, reverbSpace, sampleLoop, sampleReverse, filterMode;
    std::array<juce::Slider,2> sampleTrim;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,2> trimAttachments;
    juce::Slider sampleRoot, compAmount, pitchWheel;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> fxChoices;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compAttachment, pitchAttachment;
    juce::ToggleButton osc1{"OSC 1"}, osc2{"OSC 2"};
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>,2> oscAttachments;
    juce::TextButton previousPreset{"<"}, nextPreset{">"};
    juce::TextButton importButton{"Import WAV"};
    std::unique_ptr<juce::FileChooser> sampleChooser;
    juce::Label title, subtitle;
    std::array<juce::Slider,15> knobs;
    std::array<juce::Label,15> labels;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveAttachment, sourceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rootAttachment;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpaceInfinityEditor)
};
