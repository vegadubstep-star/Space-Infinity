#pragma once
#include "DimensionChorus.h"
#include "CharacterFilters.h"
#include "SampleBlend.h"
#include "OscillatorMelt.h"
#include <JuceHeader.h>
#include "AnalogueCircuit.h"
#include "ExtendedEngine.h"
#include "PresetLibrary.h"
#include "SampleSources.h"
#include "FactorySampleZones.h"
#include "StudioEffects.h"
#include "RhythmFX.h"
#include "DistortionFX.h"
#include "StudioEQ.h"
#include "WorkflowDSP.h"
#include "ManualUnison.h"
class ImpulseWorker;
class SpaceInfinityProcessor : public juce::AudioProcessor {
public:
    juce::MidiKeyboardState keyboardState;
    explicit SpaceInfinityProcessor(const juce::File& presetDirectory={});
    ~SpaceInfinityProcessor() override;
    bool isBusesLayoutSupported(const BusesLayout& layout) const override {
        const auto input=layout.getMainInputChannelSet();
        return layout.getMainOutputChannelSet()==juce::AudioChannelSet::stereo()
            &&(input.isDisabled()||input==juce::AudioChannelSet::mono()||input==juce::AudioChannelSet::stereo());
    }
    void prepareToPlay(double,int) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override {return true;}
    const juce::String getName() const override {return "SPACE INFINITY";}
    bool acceptsMidi() const override {return true;}
    bool producesMidi() const override {return false;}
    bool isMidiEffect() const override {return false;}
    double getTailLengthSeconds() const override;
    int getNumPrograms() override {return 1;}
    int getCurrentProgram() override {return 0;}
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override {return "Custom";}
    void changeProgramName(int,const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*,int) override;
    juce::AudioProcessorValueTreeState state;
    void loadPreset(int index);
    PresetLibrary& presetBank() noexcept {return *library;}
    juce::Result loadBankPreset(int index);
    juce::Result saveNewPreset(const juce::String& name);
    juce::Result replaceBankPreset(int index);
    juce::Result exportPreset(const juce::File&,bool overwrite=false);
    juce::File presetExportFolder() const {return lastPresetExportFolder.isDirectory()?lastPresetExportFolder:library->folder();}
    juce::String selectedPresetKey() const {return state.state.getProperty("presetKey").toString();}

    static juce::String eqId(int band,const char* field);
    static constexpr std::array<float,7> eqFrequencies{60,120,250,500,1000,3000,8000};
    static juce::StringArray oscillatorWaves(bool follow=false);
    static float oscillatorFrameSample(double phase,int wave,float position,double phaseIncrement=0);
    static float oscillatorSample(double phase,int wave);
    static float warpedSample(double phase,int wave,int mode,float amount,float modulator);
    static juce::StringArray filterNames(bool originalOnly=false);
    int filterSelection() const {const int extra=int(param("characterFilterMode"));return extra>0?extra+6:int(param("filterMode"));}
    float filterDisplayCutoff() const noexcept {return displayedFilterCutoff.load(std::memory_order_relaxed);}
    float filterDisplayResonance() const noexcept {return displayedFilterResonance.load(std::memory_order_relaxed);}
    float filterDisplayVowel() const noexcept {return displayedFilterVowel.load(std::memory_order_relaxed);}
    float filterDisplayMorph() const noexcept {return displayedFilterMorph.load(std::memory_order_relaxed);}
    float takeOutputPeak() noexcept { return outputPeak.exchange(0.0f); }
    void loadMotionShape(int index);
    static juce::StringArray motionNames();
    static juce::StringArray macroNames();
    static juce::StringArray macroTargets();
    static juce::String macroRouteId(int macro,int slot,bool depth);
    static juce::String modulationRouteId(int source,int slot,bool depth);
    static juce::String lfoParameterId(int lfo,const char* field);
    static juce::String lfoPointId(int lfo,int point);
    void loadLfoShape(int lfo,int index);
    void enableLfoCurve(int lfo);
    studio::EditableCurve lfoCurve(int lfo) const;
    float lfoCurveValue(int lfo,double phase) const;
    float lfoDisplayPhase(int index) const noexcept {return displayedLfoPhase[size_t(index)].load();}
    static juce::String motionPointId(int index);
    juce::Result importSample(const juce::File& file);
    static juce::StringArray sourceNames();
    static const juce::StringArray& presetNames();
    static juce::String presetCategory(int index);
    static juce::String presetCreator(int index);
    static juce::String presetTags(int index);
    juce::Result hybridisePresets(int parentA,int parentB,float blend);
    const juce::MidiBuffer& generatedNotes() const noexcept {return patternMidi;}
    juce::Result importWavetable(const juce::File&,int frameSize);
    juce::Result exportWavetable(const juce::File&);
    std::array<float,infinity::Wavetable::points> tableFrame(int index);
    int tableFrameCount();
    void editTableFrame(int index,const std::array<float,infinity::Wavetable::points>&);
    void addTableFrame(int copyFrom);
    void removeTableFrame(int index);
    juce::Result importImpulse(const juce::File&);
    struct SampleView {std::array<float,512> low{},high{};int samples=0;double rate=0;float onset=0;};
    bool sampleView(int source,SampleView&);
    std::array<float,1024> outputScope(bool fullRate=false) const;
    float samplePlayhead() const {return displayedSamplePosition.load();}
    std::array<float,3> compressorLevels() const;
    std::array<float,3> compressorReduction() const;
    std::array<float,512> impulseView() const;
    bool impulseBuilding() const {return irBusy.load();}
    juce::String impulseName() const;
    double hostBpm() const {return displayedBpm.load();}
    void useGateCurveFromSteps();
    void loadGatePreset(int);
    float distortionInputPeak()const{return distortion.drivenInputPeak();}
    float effectPeak(int stage) {return fxPeak[size_t(stage)].exchange(0); }
    bool effectClipping(int stage) {return fxClip[size_t(stage)].exchange(false);}
    juce::StringArray flexibleTargets() const;
    void assignFlexibleMacro(int macro,int slot,int target);
    void assignFlexibleSource(int source,int slot,int target);
    static int flexibleRouteIndex(int source,int slot){return source==9?144+slot:source<3?(slot<3?source*3+slot:105+source*13+slot-3):source<6?9+(source-3)*16+slot:57+(source-6)*16+slot;}
    static juce::String flexibleDepthId(int source,int slot){if(source==9)return "ampFlexDepth_"+juce::String(slot);return juce::String(source<3?"freeMacroDepth":source<6?"lfoFlexDepth":"performanceFlexDepth")+juce::String(source<3?source:source<6?source-3:source-6)+"_"+juce::String(slot);}
    static juce::String flexibleTargetId(int source,int slot){if(source==9)return "ampFlexTarget_"+juce::String(slot);return juce::String(source<3?"freeMacroTarget":source<6?"lfoFlexTarget":"performanceFlexTarget")+juce::String(source<3?source:source<6?source-3:source-6)+"_"+juce::String(slot);}
    static juce::String modulationPolarityId(int source,int slot,bool legacy=false){if(legacy)return source>=4&&source<8?"modPolarity"+juce::String(source)+"_"+juce::String(slot):juce::String{};return source==9?"ampFlexPolarity_"+juce::String(slot):source>=3&&source<6?"lfoFlexPolarity"+juce::String(source-3)+"_"+juce::String(slot):juce::String{};}
    int flexibleSourceTarget(int source,int slot)const{return flexibleRoute[size_t(flexibleRouteIndex(source,slot))].load();}
    int flexibleTargetIndex(const juce::String& id)const {for(size_t i=0;i<flexibleTargetList.size();++i)if(flexibleTargetList[i].id==id)return int(i);return -1;}
    bool dropModulation(const juce::String&,const juce::String&);
    juce::String modulationDepthFor(const juce::String&,const juce::String&)const;
    int flexibleMacroTarget(int macro,int slot) const {return flexibleRoute[size_t(flexibleRouteIndex(macro,slot))].load();}
    float effectiveParameter(const char* id) const {return param(id);}
    float gatePhaseView()const{return gateViewPhase.load();}
    int sequenceStep() const{return displayedPatternStep.load();}
    static int legacyTargetIndex(const juce::String&);
    int gateStep() const {return displayedGateStep.load();}
private:
    juce::File lastPresetExportFolder;
    friend class ImpulseWorker;
    std::unique_ptr<juce::Thread> impulseWorker;
    void rebuildImpulse();
    void startImpulseWorker();
    juce::CriticalSection impulseSourceLock;
    juce::CriticalSection impulsePreparationLock;
    std::atomic<unsigned> impulseRequest{0},impulseCompleted{0};
    std::atomic<bool> irBusy{false};
    std::array<std::atomic<float>,512> displayedImpulse{};
    std::atomic<double> displayedBpm{120};
    studio::Compressor studioCompressor;
    studio::Delay studioDelay;
    studio::Rhythm rhythm;
    std::atomic<int> displayedGateStep{0};
    juce::AudioBuffer<float> sidechainHigh;
    std::array<std::atomic<float>,3> compLevels{},compReduction{};
    std::array<std::atomic<float>,32768> scopeSamples{};
    std::atomic<unsigned> scopeWrite{0};unsigned scopeCounter=0;
    std::atomic<float> displayedSamplePosition{-1};
    juce::SmoothedValue<float> echoWet,irLevel,studioCompWet,reverbDryGain;
    std::unique_ptr<PresetLibrary> library;
    juce::dsp::Oversampling<float> oversampling{2,1,juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,true,true};
    juce::MidiBuffer highRateMidi;
    int maximumHostBlock=512;
    void renderBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&);
    infinity::Wavetable wavetable;
    bool levelMatchActive=false;double levelMatchEnergy=0,levelMatchReference=0,levelMatchGain=1;int levelMatchSamples=0;
    studio::Distortion distortion;
    infinity::Pattern pattern;
    std::atomic<int> displayedPatternStep{-1};
    juce::MidiBuffer patternMidi;
    std::array<std::array<std::atomic<float>*,16>,3> stepParameters{};
    bool transportKnown=false,transportPlaying=false;
    double previousPpq=0;
    juce::dsp::Convolution convolver;
    juce::AudioBuffer<float> convolutionDry;
    juce::SmoothedValue<float> convolutionWet;
    juce::MemoryBlock impulseBytes;
    juce::AudioBuffer<float> pendingImpulse;
    double pendingImpulseRate=44100;
    bool impulsePending=false;
    juce::Result queueImpulse(const juce::MemoryBlock&);
    void transformPattern(juce::MidiBuffer&,int);
    struct EqState { float z1=0.0f,z2=0.0f; };
    std::array<std::array<std::array<EqState,4>,7>,2> eqState{};
    std::array<std::array<std::atomic<float>*,3>,7> eqParameters{};
    std::array<std::atomic<float>*,7> eqTypes{},eqSlopes{};
    std::array<std::atomic<float>*,16> gateParameters{};
    std::atomic<float> outputPeak{0.0f};
    uint32_t freeNoiseState=0x57a3c9e1u;float freeNoiseMemory=0;
    struct Sample {
        juce::AudioBuffer<float> audio;
        double sampleRate = 0;
        SampleView view;
    };
    static juce::Result decodeSample(const juce::MemoryBlock&, Sample&,bool keepStereo=false);
    Sample choirAh, choirOoh, importedSample;
    std::array<Sample,16> extraSamples;
    std::array<Sample,factorySampleZones.size()> factorySamples;
    int factoryZoneForSource(int source,int note,float velocity)const;
    int rootForSource(int source,int zone,int importedRoot)const;
    const Sample& sampleForSource(int source,int zone=-1) const {
        if(source>=21){const int selected=zone>=0?zone:factoryZoneForSource(source,60,.7f);if(selected>=0&&selected<int(factorySamples.size()))return factorySamples[size_t(selected)];}
        return source==1?choirAh:source==2?choirOoh:source>=5&&source<21?extraSamples[size_t(source-5)]:importedSample;
    }
    static bool userSample(int source) noexcept {return source==3||source==4;}
    juce::MemoryBlock importedBytes;
    juce::SpinLock sampleLock;
    unsigned sampleRevision = 0;
    struct Voice {
        infinity::Granulator granular;
        infinity::SampleCloud cloud;infinity::SamplePolish samplePolish;
        infinity::Envelope filterEnvelope;
        AnalogueEnvelope curvedAmp,curvedFilter,pitchEnvelope;
        VoiceCalibration calibration;
        double idleRateA=0,idleRateB=0,pitch=0,subPhase=0,lifePhase=0,noteSeconds=0;
        float subEnvelope=0,lastL=0,lastR=0,tailL=0,tailR=0,smoothFc=0;int declickRemaining=0,declickLength=1,retireRemaining=0,retireLength=1;
        int source = 0,sampleZone=-1,sampleLoopStart=0;
        double samplePosition = 0, sampleStep = 0;
        int sampleStart = 0, sampleEnd = 0;
        bool sampleLoop = false;
        float stereoSide=0.0f, noiseMemory=0.0f;
        float releaseStart=-1.0f;
        float dcInput[2]{},dcOutput[2]{};
        AnalogueDrift drift;
        std::array<double,3> lfoPhase{},lfoAge{};
        std::array<float,3> lfoMemory{};
        std::array<AnalogueLadder,2> ladder;
        std::array<AcidResonantFilter,2> acidFilter;

        uint32_t noiseState=1;
        AnalogueColour colour,voltageColour;float filterSideState[2]{},filterSideCascade[2]{};
        std::array<double,127> extraPhase1{}, extraPhase2{};
        float filterState[2]{}, filterCascade[2]{};
        unsigned sampleRevision = 0; bool active=false, released=false, decaying=false; int note=-1, channel=1; float velocity=0, env=0, cutoff=0; double phase=0, phase2=0; uint64_t age=0; };
    std::array<Voice,24> voices{};
    std::unique_ptr<std::array<std::array<infinity::CharacterFilter,2>,24>> characterFilters=std::make_unique<std::array<std::array<infinity::CharacterFilter,2>,24>>();
    std::atomic<float> displayedFilterCutoff{8000},displayedFilterResonance{.12f},displayedFilterVowel{0},displayedFilterMorph{0};
    std::array<juce::SmoothedValue<float>,10> liveGains;
    std::array<juce::SmoothedValue<float>,10> analogueDetails;
    std::array<juce::SmoothedValue<float>,4> sourcePans;AnalogueDrift boardDrift;
    std::atomic<bool> resetVoicesRequested{false};
    void retireVoice(Voice&);
    std::array<std::atomic<float>*,4> macroValues{};
    std::array<std::array<std::atomic<float>*,3>,8> macroRoutes{}, macroDepths{};
    std::array<std::array<std::atomic<float>*,3>,4> dynamicPolarities{};
    std::array<std::array<std::atomic<float>*,17>,3> designerPoints{};
    std::array<std::array<std::atomic<float>*,4>,3> designerControls{};
    std::array<std::atomic<float>*,3> designerEnabled{};
    std::array<double,3> designerPhase{},designerAge{};
    std::array<float,3> designerMemory{};
    std::array<std::array<std::atomic<float>*,10>,3> designerAdvanced{};
    std::array<std::atomic<float>*,3> designerCurveEnabled{},designerCurveCount{};
    std::array<std::array<std::atomic<float>*,33>,3> designerX{},designerY{},designerBend{};
    std::array<std::atomic<float>,3> displayedLfoPhase{};
    double sr=44100, lfoPhase=0, hostChunkSeconds=0; uint64_t clock=0;
    juce::dsp::Chorus<float> chorus, flanger;
    studio::DimensionChorus dimensionChorus;juce::SmoothedValue<float> oscillatorMeltAmount;
    juce::dsp::Phaser<float> phaser;
    bool flangerActive=false, phaserActive=false;
    juce::Reverb reverb;
    juce::dsp::LinkwitzRileyFilter<float> crossoverLow, crossoverHigh, alignLow;
    std::array<juce::dsp::Compressor<float>,3> bandCompressors;
    juce::SmoothedValue<float> delaySamples, delayWet, bandMix;
    float delayToneL=0,delayToneR=0;
    double delayBpm=120;
    float midiBendPosition=0,previousOutputGain=.65f;
    juce::SmoothedValue<float> pitchBend, analogueAmount, analogueCutoff, analogueResonance,analogueLabRange;
    juce::dsp::DelayLine<float,juce::dsp::DelayLineInterpolationTypes::Linear> delayL{192000},delayR{192000};
    struct HeldNote { uint64_t order=0; float velocity=0.0f; };
    std::array<HeldNote,16*128> heldNotes{};
    uint64_t noteOrder=0;
    int voiceMode=0;
    void selectMonoNote(int source,int root,bool retrigger);
    void noteOn(int,float,int,int,int); void noteOff(int,int);
    struct FlexibleTarget {juce::String id,name;juce::RangedAudioParameter* parameter=nullptr;std::atomic<float>* raw=nullptr;};
    std::vector<FlexibleTarget> flexibleTargetList;std::array<std::atomic<int>,160> flexibleRoute;std::atomic<bool> flexibleAssigned{false};
    struct FlexibleValue {std::atomic<int> target{-1};std::atomic<float> offset{0};};std::array<FlexibleValue,160> flexibleValue{};
    std::array<std::atomic<float>*,160> flexibleDepth{}, flexiblePolarity{};
    std::atomic<float> gateViewPhase{0};std::array<std::array<std::atomic<float>*,33>,3> gateCurveParameters{};
    studio::WetEQ delayWetEQ,reverbWetEQ;studio::SmoothFlanger smoothFlanger;juce::AudioBuffer<float> workflowDry;
    std::array<juce::SmoothedValue<float>,9> fxGain;std::array<std::atomic<float>,9> fxPeak;std::array<std::atomic<bool>,9> fxClip;
    void initialiseWorkflow();void prepareWorkflow(double);void updateFlexibleRoutes();void restoreFlexibleRoutes();void clearFlexibleRoutes();void configureWetEQ();void finishEffect(int,juce::AudioBuffer<float>&,bool);
    float param(const char* id) const {return readParameter(state.getRawParameterValue(id));}
    float readParameter(const std::atomic<float>* raw) const {float value=raw->load();if(!flexibleAssigned.load(std::memory_order_relaxed))return value;for(const auto& v:flexibleValue){const int index=v.target.load(std::memory_order_relaxed);if(index>=0){const auto& t=flexibleTargetList[size_t(index)];if(t.raw==raw){const float offset=v.offset.load(std::memory_order_relaxed);if(offset!=0)value=t.parameter->convertFrom0to1(juce::jlimit(0.0f,1.0f,t.parameter->convertTo0to1(value)+offset));}}}return value;}
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpaceInfinityProcessor)
};

