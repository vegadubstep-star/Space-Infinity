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
    static juce::String eqId(int band,const char* field);
    static constexpr std::array<float,7> eqFrequencies{60,120,250,500,1000,3000,8000};
    static juce::StringArray oscillatorWaves(bool follow=false);
    static float oscillatorSample(double phase,int wave);
    float takeOutputPeak() noexcept { return outputPeak.exchange(0.0f); }
    void loadMotionShape(int index);
    static juce::StringArray motionNames();
    static juce::StringArray macroNames();
    static juce::StringArray macroTargets();
    static juce::String macroRouteId(int macro,int slot,bool depth);
    static juce::String motionPointId(int index);
    juce::Result importSample(const juce::File& file);
    static juce::StringArray sourceNames();
    static const juce::StringArray& presetNames();
    static juce::String presetCategory(int index);
private:
    struct EqState { float z1=0.0f,z2=0.0f; };
    std::array<std::array<EqState,7>,2> eqState{};
    std::array<std::array<std::atomic<float>*,3>,7> eqParameters{};
    std::atomic<float> outputPeak{0.0f};
    struct Sample {
        juce::AudioBuffer<float> audio;
        double sampleRate = 0;
    };
    static juce::Result decodeSample(const juce::MemoryBlock&, Sample&);
    Sample choirAh, choirOoh, importedSample;
    juce::MemoryBlock importedBytes;
    juce::SpinLock sampleLock;
    unsigned sampleRevision = 0;
    struct Voice {
        int source = 0;
        double samplePosition = 0, sampleStep = 0;
        int sampleStart = 0, sampleEnd = 0;
        bool sampleLoop = false;
        float stereoSide=0.0f, noiseMemory=0.0f;
        uint32_t noiseState=1;
        std::array<double,15> extraPhase1{}, extraPhase2{};
        float filterState[2]{}, filterCascade[2]{};
        unsigned sampleRevision = 0; bool active=false, released=false, decaying=false; int note=-1, channel=1; float velocity=0, env=0, cutoff=0; double phase=0, phase2=0; uint64_t age=0; };
    std::array<Voice,24> voices{};
    std::array<std::atomic<float>*,17> motionParameters{};
    std::array<std::atomic<float>*,4> macroValues{};
    std::array<std::array<std::atomic<float>*,3>,4> macroRoutes{}, macroDepths{};
    double motionPhase=0.0;
    double sr=44100, lfoPhase=0; uint64_t clock=0;
    juce::dsp::Chorus<float> chorus;
    juce::Reverb reverb;
    juce::dsp::LinkwitzRileyFilter<float> crossoverLow, crossoverHigh, alignLow;
    std::array<juce::dsp::Compressor<float>,3> bandCompressors;
    juce::SmoothedValue<float> delaySamples, delayWet, bandMix;
    float delayToneL=0,delayToneR=0;
    double delayBpm=120;
    float midiBend=0;
    juce::SmoothedValue<float> pitchBend;
    juce::dsp::DelayLine<float,juce::dsp::DelayLineInterpolationTypes::Linear> delayL{192000},delayR{192000};
    void noteOn(int,float,int,int,int); void noteOff(int,int);
    float param(const char* id) const {return state.getRawParameterValue(id)->load();}
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpaceInfinityProcessor)
};
