#pragma once
#include "PluginProcessor.h"
#include "HardwareTheme.h"
class StudioRack final : public juce::Component,private juce::Timer {
public:
    explicit StudioRack(SpaceInfinityProcessor&);
    ~StudioRack() override;
    void resized() override;
    std::function<void()> onOpenGate;
    juce::Component* module(int index)const;
private:
    struct Impl;std::unique_ptr<Impl> impl;
    void timerCallback() override;
};
