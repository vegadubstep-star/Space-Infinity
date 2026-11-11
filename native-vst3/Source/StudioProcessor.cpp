#include "PluginProcessor.h"
#include <complex>

namespace {
constexpr const char* impulseIds[]{"irPreset","irSize","irTone","irPredelay","irSync","irAttack","irDecay","irDamp","irGain","irReverse","irMinimumPhase"};
}
class ImpulseWorker final : public juce::Thread,private juce::AudioProcessorValueTreeState::Listener {
public:
    explicit ImpulseWorker(SpaceInfinityProcessor& p):Thread("Infinity IR preparation"),owner(p){
        for(const auto* id:impulseIds)owner.state.addParameterListener(id,this);startThread();
    }
    ~ImpulseWorker() override {for(const auto* id:impulseIds)owner.state.removeParameterListener(id,this);signalThreadShouldExit();stopThread(-1);}
private:
    SpaceInfinityProcessor& owner;
    void parameterChanged(const juce::String&,float) override {owner.impulseRequest.fetch_add(1,std::memory_order_relaxed);}
    void run() override {while(!threadShouldExit()){
        if(owner.impulseRequest.load(std::memory_order_relaxed)!=owner.impulseCompleted.load(std::memory_order_relaxed))owner.rebuildImpulse();
        wait(20);
    }}
};
void SpaceInfinityProcessor::startImpulseWorker(){impulseWorker=std::make_unique<ImpulseWorker>(*this);}
SpaceInfinityProcessor::~SpaceInfinityProcessor(){impulseWorker.reset();}
double SpaceInfinityProcessor::getTailLengthSeconds()const{
    double tail=1;
    if(param("delayEnabled")>.5f){
        const bool modern=param("echoMode")>.5f;const float wet=modern?param("echoMix"):param("delay");
        if(wet>.001f){static constexpr double divisions[]{0,.0625,.125,.25,.5,1,2,4};
            const double seconds=modern?juce::jmax(param("echoTimeL")*param("echoOffsetL"),param("echoTimeR")*param("echoOffsetR")):param("delayTime");
            const int left=modern?int(param("echoSyncL")):0,right=modern?int(param("echoSyncR")):0;
            const double synced=juce::jmax(seconds,60.0/hostBpm()*divisions[juce::jmax(left,right)]*2);
            tail+=juce::jmin(modern?4.0:2.0,synced)*juce::jmax(1.0,std::log(.001)/std::log(juce::jlimit(.0001,.95,double(param("delayFeedback")))));
        }
    }
    if(param("reverbEnabled")>.5f&&param("reverb")>.001f)tail+=2+6*param("reverbSize");
    if(param("convolverEnabled")>.5f&&param("convolverMix")>.001f){static constexpr double lengths[]{4,.45,1.4,2.5,.75};
        tail+=lengths[juce::jlimit(0,4,int(param("irPreset")))]*param("irSize")+2;}
    return juce::jmin(600.0,tail);
}

void SpaceInfinityProcessor::rebuildImpulse(){
    // Serialise preparation jobs, never the audio callback. FFTs, decoding,
    // allocation and transformations all happen on the caller/worker thread.
    const juce::ScopedLock job(impulsePreparationLock);
    const unsigned request=impulseRequest.load(std::memory_order_relaxed);
    if(request==impulseCompleted.load(std::memory_order_relaxed))return;
    irBusy.store(true,std::memory_order_relaxed);
    juce::MemoryBlock bytes;{const juce::ScopedLock lock(impulseSourceLock);bytes=impulseBytes;}
    juce::AudioBuffer<float> source;double rate=48000;
    const int preset=int(param("irPreset"));
    if(preset==0&&bytes.getSize()){
        juce::WavAudioFormat format;std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(new juce::MemoryInputStream(bytes,false),true));
        if(reader){rate=reader->sampleRate;source.setSize(int(reader->numChannels),int(reader->lengthInSamples));reader->read(&source,0,source.getNumSamples(),0,true,true);}
    }else if(preset>0){
        const float duration=preset==1?.45f:preset==2?1.4f:preset==3?2.5f:.75f;
        source.setSize(2,int(rate*duration));source.clear();juce::Random random(7201+preset*97);
        for(int ch=0;ch<2;++ch)for(int i=0;i<source.getNumSamples();++i){const float t=float(i/rate),tail=std::exp(-7*t/duration);
            const float noise=random.nextFloat()*2-1;const float metal=std::sin(float(i)*.072f)+.5f*std::sin(float(i)*.1197f);
            float v=(preset==4?metal*.15f:noise*.2f)*tail;
            if(preset==1&&i%int(rate*.017+ch*43)<3)v+=.5f*tail;
            source.setSample(ch,i,v);}
        for(int ch=0;ch<2;++ch)source.addSample(ch,0,.7f);
    }
    if(source.getNumSamples()==0){source.setSize(1,1);source.setSample(0,0,1);}
    const int length=juce::jmax(1,int(double(source.getNumSamples())*param("irSize")));
    const int sync=int(param("irSync"));static constexpr double divisions[]{0,.0625,.125,.25,.5,1};
    const double preSeconds=sync?60.0/hostBpm()*divisions[sync]:param("irPredelay")*.001;
    const int pre=int(juce::jlimit(0.0,2.0,preSeconds)*rate);
    juce::AudioBuffer<float> prepared(source.getNumChannels(),length);
    const bool reverse=param("irReverse")>.5f;const float attack=param("irAttack")*.001f,decay=param("irDecay"),damp=param("irDamp"),tone=param("irTone");
    const float coeff=float(1-std::exp(-juce::MathConstants<double>::twoPi*juce::jmin(double(tone),rate*.45)/rate));
    for(int ch=0;ch<prepared.getNumChannels();++ch){float lp=0;
        for(int i=0;i<length;++i){double at=double(i)*source.getNumSamples()/length;if(reverse)at=source.getNumSamples()-1-at;
            at=juce::jlimit(0.0,double(source.getNumSamples()-1),at);const int a=int(at),b=juce::jmin(a+1,source.getNumSamples()-1);
            float v=source.getSample(ch,a)+float(at-a)*(source.getSample(ch,b)-source.getSample(ch,a));
            lp+=coeff*(v-lp);if(tone<19999)v=lp;
            const float t=float(i/rate),fade=attack>0?juce::jlimit(0.0f,1.0f,t/attack):1;
            const float envelope=fade*(decay<7.999f?std::exp(-6.9078f*t/decay):1);
            const float highLoss=std::exp(-t*damp*12);v=lp+(v-lp)*highLoss;
            prepared.setSample(ch,i,v*envelope);
        }
    }
    if(param("irMinimumPhase")>.5f)studio::minimumPhase(prepared);
    const float peak=prepared.getMagnitude(0,prepared.getNumSamples());
    prepared.applyGain(juce::Decibels::decibelsToGain(param("irGain"))/(juce::jmax(1.0f,peak)));
    juce::AudioBuffer<float> delayed(prepared.getNumChannels(),length+pre);delayed.clear();
    for(int ch=0;ch<prepared.getNumChannels();++ch)delayed.copyFrom(ch,pre,prepared,ch,0,length);
    if(request==impulseRequest.load(std::memory_order_relaxed)){
        for(int bin=0;bin<512;++bin){const int begin=bin*delayed.getNumSamples()/512,end=juce::jmax(begin+1,(bin+1)*delayed.getNumSamples()/512);float high=0;
            for(int ch=0;ch<delayed.getNumChannels();++ch)for(int i=begin;i<juce::jmin(end,delayed.getNumSamples());++i)high=juce::jmax(high,std::abs(delayed.getSample(ch,i)));
            displayedImpulse[size_t(bin)].store(high,std::memory_order_relaxed);}
        {const juce::SpinLock::ScopedLockType lock(sampleLock);pendingImpulse=std::move(delayed);pendingImpulseRate=rate;impulsePending=true;}
        impulseCompleted.store(request,std::memory_order_release);
    }
    irBusy.store(false,std::memory_order_relaxed);
}

bool SpaceInfinityProcessor::sampleView(int source,SampleView& view){
    if(source==0){view={};return true;}
    const juce::SpinLock::ScopedTryLockType lock(sampleLock);if(!lock.isLocked())return false;
    view=sampleForSource(source).view;return true;
}
std::array<float,1024> SpaceInfinityProcessor::outputScope(bool fullRate)const{
    std::array<float,1024> result{};const unsigned end=scopeWrite.load(std::memory_order_acquire);
    const float hostRate=float(getSampleRate()>0?getSampleRate():48000);
    const unsigned span=fullRate?1024u:juce::jlimit(32u,32768u,unsigned(hostRate*state.getRawParameterValue("scopeWindow")->load()));
    for(unsigned i=0;i<1024;++i)result[i]=scopeSamples[size_t((end-span+i*span/1024)%32768)].load(std::memory_order_relaxed);
    return result;
}
std::array<float,3> SpaceInfinityProcessor::compressorLevels()const{std::array<float,3> v{};for(size_t i=0;i<3;++i)v[i]=compLevels[i].load();return v;}
std::array<float,3> SpaceInfinityProcessor::compressorReduction()const{std::array<float,3> v{};for(size_t i=0;i<3;++i)v[i]=compReduction[i].load();return v;}
std::array<float,512> SpaceInfinityProcessor::impulseView()const{std::array<float,512> v{};for(size_t i=0;i<512;++i)v[i]=displayedImpulse[i].load();return v;}
juce::String SpaceInfinityProcessor::impulseName()const{
    static constexpr const char* names[]{"CUSTOM / IMPORTED","SMALL ROOM","BRIGHT PLATE","LARGE HALL","METAL CHAMBER"};
    const int preset=int(state.getRawParameterValue("irPreset")->load());return preset==0?state.state.getProperty("impulseName","DROP OR LOAD AN IMPULSE").toString():names[juce::jlimit(0,4,preset)];
}
void SpaceInfinityProcessor::loadGatePreset(int index){
    if(index<0||index>=15)return;for(int i=0;i<16;++i){auto* p=state.getParameter("gateStep"+juce::String(i));p->beginChangeGesture();p->setValueNotifyingHost(studio::gatePatterns[size_t(index)][size_t(i)]);p->endChangeGesture();}
    state.state.setProperty("gatePresetName",studio::gateNames[size_t(index)],nullptr);
}
