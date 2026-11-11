#pragma once
#include "PluginProcessor.h"
#include "HardwareTheme.h"
class ModulationBadge;
class SpaceInfinityEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit SpaceInfinityEditor(SpaceInfinityProcessor&);
    ~SpaceInfinityEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override;
    juce::TooltipWindow controlTips{this,650};
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    void drawMotionAt(juce::Point<int>);
    int draggedMacro=-1, hoverTarget=0;
    int targetAt(juce::Point<int>) const;
    juce::Rectangle<int> targetBounds(int) const;
    void assignMacro(int macro,int target,bool inverted);
    juce::Rectangle<int> motionGraph;
    int lastMotionPoint=-1;
    float lastMotionValue=0.0f;
    std::array<bool,17> motionGestures{};
    juce::Rectangle<int> meterBounds;
    float meterPeak=0.0f;
    int clipHold=0;
    HardwareTheme hardwareLook;
    juce::Image hardwareFace;
    bool cachedFxPage=false;
    SpaceInfinityProcessor& processor;
    void refreshPresetMenu();
    void openPresetManager();
    juce::String displayedPresetKey;
    juce::TextButton bankButton{"BANK"};
    juce::ComboBox presets, waveform, waveform2, source, motionShape, motionBars;
    juce::Slider motionDepth, noiseLevel, analogueCharacter;
    juce::Label analogueLabel;
    juce::ComboBox noiseType;
    std::array<juce::Slider,4> oscillatorSliders;
    std::array<juce::Slider,3> octaveControls;
    std::array<juce::Slider,2> unisonSliders;
    std::array<juce::Slider,4> macros;

    int selectedLfo=0, selectedSource=0;
    std::array<juce::Rectangle<int>,3> lfoTabs;
    juce::Rectangle<int> envelopeTab, envelopeGraph;
    int envelopeDrag=-1;
    float envelopeDragDuration=1.0f;
    juce::ComboBox lfoSync,lfoMode;
    juce::Slider lfoSpeed;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lfoBarsBinding,lfoSyncBinding,lfoModeBinding;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfoSpeedBinding;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,17> lfoPointBindings;
    std::array<std::unique_ptr<ModulationBadge>,23> modulationBadges;
    void bindLfoControls();
    void updateLfoRateVisibility();
    void refreshModulationIndicators();
    int routeSourceForTarget(int target) const;
    std::pair<juce::RangedAudioParameter*,juce::RangedAudioParameter*> routeForTarget(int target,bool select);
    std::array<juce::Point<float>,3> envelopeNodes() const;
    float envelopeDuration() const;
    std::array<juce::Slider,17> motionPoints;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> motionDepthAttachment;
    juce::ComboBox delaySync, delayMode, reverbSpace, sampleLoop, sampleReverse, filterMode, voiceMode;
    std::array<juce::Slider,2> sampleTrim;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,2> trimAttachments;
    juce::Slider sampleRoot, compAmount, pitchWheel;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> fxChoices;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compAttachment, pitchAttachment;
    juce::ToggleButton osc1{"OSC 1"}, osc2{"OSC 2"};
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>,2> oscAttachments;
    juce::TextButton warpA{"WARP"}, warpB{"WARP"};
    std::array<juce::Slider,28> rackControls;
    std::array<juce::Label,28> rackLabels;
    std::array<juce::ToggleButton,6> rackEnabled;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>,6> rackPower;
    std::array<juce::TextButton,2> previousWave,nextWave;
    juce::TextButton extendedButton{"ENGINES"};
    juce::TextButton mainTab{"MAIN"}, fxTab{"FX"};
    std::vector<juce::Component*> mainControls;
    bool fxPage=false;
    void updatePage();
    juce::TextButton eqButton{"EQ"};
    juce::TextButton previousPreset{"<"}, nextPreset{">"};
    juce::TextButton importButton{"Import WAV"};
    std::unique_ptr<juce::FileChooser> sampleChooser;
    juce::Label title, subtitle;
    std::array<juce::Slider,17> knobs;
    std::array<juce::Label,17> labels;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveAttachment, sourceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rootAttachment;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpaceInfinityEditor)
};

