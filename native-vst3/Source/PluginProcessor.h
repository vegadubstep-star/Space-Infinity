#pragma once
#include <JuceHeader.h>
class SpaceInfinityProcessor : public juce::AudioProcessor {
public:
    SpaceInfinityProcessor();
    void prepareToPlay(double,int) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override {return true;}
    const juce::String getName() const override {return "SPACE INFINITY";}
    bool acceptsMidi() const override {return true;}
    bool producesMidi() const override {return false;}
    bool isMidiEffect() const override {return false;}
    double getTailLengthSeconds() const override {return 4.0;}
    int getNumPrograms() override {return 1;}
    int getCurrentProgram() override {return 0;}
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override {return "Custom";}
    void changeProgramName(int,const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*,int) override;
    juce::AudioProcessorValueTreeState state;
    void loadPreset(int index);
    static const juce::StringArray& presetNames();
private:
    struct Voice { bool active=false, released=false; int note=-1; float velocity=0, env=0, cutoff=0; double phase=0, phase2=0; uint64_t age=0; };
    std::array<Voice,24> voices{};
    double sr=44100, lfoPhase=0; uint64_t clock=0;
    juce::dsp::Chorus<float> chorus;
    juce::dsp::Reverb reverb;
    juce::dsp::DelayLine<float,juce::dsp::DelayLineInterpolationTypes::Linear> delayL{192000},delayR{192000};
    void noteOn(int,float); void noteOff(int);
    float param(const char* id) const {return state.getRawParameterValue(id)->load();}
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpaceInfinityProcessor)
};