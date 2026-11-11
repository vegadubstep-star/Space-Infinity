#include "PluginProcessor.h"
#include <cstring>
void SpaceInfinityProcessor::transformPattern(juce::MidiBuffer& midi,int samples){
    const int mode=int(param("patternMode"));
    patternMidi.clear();
    const auto emitAt=[this](bool on,int channel,int note,float velocity,int offset){
        patternMidi.addEvent(on?juce::MidiMessage::noteOn(channel,note,velocity):juce::MidiMessage::noteOff(channel,note),offset);
    };
    if(mode!=pattern.mode){
        pattern.stop([&](bool on,int ch,int n,float v){emitAt(on,ch,n,v,0);});
        for(int ch=1;ch<=16;++ch)patternMidi.addEvent(juce::MidiMessage::allSoundOff(ch),0);
        pattern.held={};pattern.index=0;pattern.mode=mode;
    }
    bool stopped=false;
    double bpm=delayBpm;
    if(auto* head=getPlayHead())if(auto position=head->getPosition()){
        if(auto tempo=position->getBpm())if(*tempo>0)bpm=*tempo;
        const bool playing=position->getIsPlaying();
        if(transportKnown&&transportPlaying&&!playing)stopped=true;
        if(auto ppq=position->getPpqPosition()){
            const double expected=previousPpq+double(samples)*bpm/(60*sr);
            if(transportKnown&&playing&&transportPlaying&&std::abs(*ppq-expected)>.25){
                pattern.stop([&](bool on,int ch,int n,float v){emitAt(on,ch,n,v,0);});pattern.index=0;
            }
            previousPpq=*ppq;
        }
        transportKnown=true;transportPlaying=playing;
    }
    if(stopped){pattern.stop([&](bool on,int ch,int n,float v){emitAt(on,ch,n,v,0);});pattern.held={};}
    if(mode==0){patternMidi.addEvents(midi,0,samples,0);return;}
    pattern.order=int(param("arpOrder"));pattern.octaves=int(param("arpOctaves"));
    pattern.length=int(param("sequenceLength"));pattern.gate=param("patternGate");
    for(int step=0;step<16;++step){
        pattern.pitch[size_t(step)]=int(stepParameters[0][size_t(step)]->load());
        pattern.velocity[size_t(step)]=stepParameters[1][size_t(step)]->load();
        pattern.enabled[size_t(step)]=stepParameters[2][size_t(step)]->load()>.5f;
    }
    static constexpr double beats[]{1,.5,.25,.125,1.0/3,1.0/6};
    const double interval=sr*60/std::max(1.0,bpm)*beats[juce::jlimit(0,5,int(param("patternRate")))];
    auto event=midi.begin();
    for(int offset=0;offset<samples;++offset){
        const auto emit=[&](bool on,int ch,int n,float v){emitAt(on,ch,n,v,offset);};
        while(event!=midi.end()&&(*event).samplePosition<=offset){
            const auto message=(*event).getMessage();++event;
            if(message.isNoteOnOrOff())pattern.input(message.isNoteOn(),message.getChannel(),message.getNoteNumber(),message.getFloatVelocity(),emit);
            else if(message.isAllNotesOff()||message.isAllSoundOff()){
                const int ch=message.getChannel();
                for(int note=0;note<128;++note)pattern.held[size_t((ch-1)*128+note)]={};
                if(pattern.activeChannel==ch)pattern.stop(emit);
                patternMidi.addEvent(message,offset);
            }else patternMidi.addEvent(message,offset);
        }
        pattern.next(interval,emit);
    }

}
std::array<float,infinity::Wavetable::points> SpaceInfinityProcessor::tableFrame(int index){
    const juce::SpinLock::ScopedLockType lock(sampleLock);
    return wavetable.frames[size_t(juce::jlimit(0,wavetable.count-1,index))];
}
int SpaceInfinityProcessor::tableFrameCount(){const juce::SpinLock::ScopedLockType lock(sampleLock);return wavetable.count;}
void SpaceInfinityProcessor::editTableFrame(int index,const std::array<float,infinity::Wavetable::points>& frame){
    const juce::SpinLock::ScopedLockType lock(sampleLock);
    if(index<0||index>=wavetable.count)return;
    auto& target=wavetable.frames[size_t(index)];float mean=0;
    for(size_t i=0;i<target.size();++i){target[i]=std::isfinite(frame[i])?juce::jlimit(-1.0f,1.0f,frame[i]):0;mean+=target[i]/float(target.size());}
    for(auto& value:target)value=juce::jlimit(-1.0f,1.0f,value-mean);
}
void SpaceInfinityProcessor::addTableFrame(int copyFrom){
    const juce::SpinLock::ScopedLockType lock(sampleLock);
    if(wavetable.count>=infinity::Wavetable::maxFrames)return;
    wavetable.frames[size_t(wavetable.count)]=wavetable.frames[size_t(juce::jlimit(0,wavetable.count-1,copyFrom))];++wavetable.count;
}
void SpaceInfinityProcessor::removeTableFrame(int index){
    const juce::SpinLock::ScopedLockType lock(sampleLock);
    if(wavetable.count<=1||index<0||index>=wavetable.count)return;
    for(int i=index;i<wavetable.count-1;++i)wavetable.frames[size_t(i)]=wavetable.frames[size_t(i+1)];--wavetable.count;
}
juce::Result SpaceInfinityProcessor::importWavetable(const juce::File& file,int frameSize){
    if(frameSize!=512&&frameSize!=1024&&frameSize!=2048)return juce::Result::fail("Select 512, 1024 or 2048 samples per frame.");
    if(file.getSize()>16*1024*1024)return juce::Result::fail("Wavetable WAV must be under 16 MB.");
    juce::WavAudioFormat format;std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(file.createInputStream().release(),true));
    if(!reader||reader->numChannels!=1||reader->lengthInSamples<frameSize||reader->lengthInSamples%frameSize!=0||reader->lengthInSamples/frameSize>64)
        return juce::Result::fail("Choose a mono WAV of 1–64 complete frames matching the selected frame size.");
    juce::AudioBuffer<float> audio(1,int(reader->lengthInSamples));
    if(!reader->read(&audio,0,audio.getNumSamples(),0,true,false))return juce::Result::fail("Cannot decode wavetable.");
    infinity::Wavetable next;next.count=audio.getNumSamples()/frameSize;
    for(int frame=0;frame<next.count;++frame){
        float mean=0,peak=0;auto& table=next.frames[size_t(frame)];
        for(int i=0;i<infinity::Wavetable::points;++i){
            const float x=float(i)*frameSize/infinity::Wavetable::points;const int a=int(x),b=(a+1)%frameSize;
            const float v=audio.getSample(0,frame*frameSize+a)+(x-a)*(audio.getSample(0,frame*frameSize+b)-audio.getSample(0,frame*frameSize+a));
            if(!std::isfinite(v))return juce::Result::fail("Wavetable contains invalid samples.");
            table[size_t(i)]=v;mean+=v/infinity::Wavetable::points;
        }
        for(auto& v:table){v-=mean;peak=std::max(peak,std::abs(v));}
        if(peak>0)for(auto& v:table)v/=peak;
    }
    {const juce::SpinLock::ScopedLockType lock(sampleLock);wavetable=next;}
    auto* wave=state.getParameter("wave");wave->setValueNotifyingHost(wave->convertTo0to1(30));
    state.getParameter("source")->setValueNotifyingHost(0);return juce::Result::ok();
}
juce::Result SpaceInfinityProcessor::exportWavetable(const juce::File& file){
    infinity::Wavetable snapshot;{const juce::SpinLock::ScopedLockType lock(sampleLock);snapshot=wavetable;}
    juce::AudioBuffer<float> audio(1,snapshot.count*infinity::Wavetable::points);
    for(int frame=0;frame<snapshot.count;++frame)audio.copyFrom(0,frame*infinity::Wavetable::points,snapshot.frames[size_t(frame)].data(),infinity::Wavetable::points);
    juce::TemporaryFile temporary(file);auto stream=temporary.getFile().createOutputStream();
    if(!stream)return juce::Result::fail("Cannot create WAV.");
    juce::WavAudioFormat format;std::unique_ptr<juce::AudioFormatWriter> writer(format.createWriterFor(stream.get(),44100,1,24,{},0));
    if(!writer)return juce::Result::fail("Cannot create WAV writer.");stream.release();
    if(!writer->writeFromAudioSampleBuffer(audio,0,audio.getNumSamples()))return juce::Result::fail("Cannot write WAV.");
    writer.reset();return temporary.overwriteTargetFileWithTemporary()?juce::Result::ok():juce::Result::fail("Cannot save WAV.");
}
juce::Result SpaceInfinityProcessor::importImpulse(const juce::File& file){
    juce::MemoryBlock bytes;
    if(file.getSize()>4*1024*1024||!file.loadFileAsData(bytes))return juce::Result::fail("Choose a WAV impulse response up to 4 MB / 4 seconds.");
    juce::WavAudioFormat format;std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(new juce::MemoryInputStream(bytes,false),true));
    if(!reader||reader->numChannels<1||reader->numChannels>2||reader->sampleRate<8000||reader->sampleRate>192000||reader->lengthInSamples<1||reader->lengthInSamples>reader->sampleRate*4)
        return juce::Result::fail("Choose a mono/stereo WAV impulse response up to 4 seconds.");
    const auto result=queueImpulse(bytes);if(result.failed())return result;
    state.getParameter("convolverEnabled")->setValueNotifyingHost(1);return juce::Result::ok();
}
juce::Result SpaceInfinityProcessor::queueImpulse(const juce::MemoryBlock& bytes){
    juce::AudioBuffer<float> decoded;double rate=44100;
    if(bytes.getSize()){
        juce::WavAudioFormat format;std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(new juce::MemoryInputStream(bytes,false),true));
        if(!reader||reader->numChannels<1||reader->numChannels>2||reader->sampleRate<8000||reader->sampleRate>192000||reader->lengthInSamples<1||reader->lengthInSamples>reader->sampleRate*4)
            return juce::Result::fail("Invalid impulse response.");
        rate=reader->sampleRate;decoded.setSize(int(reader->numChannels),int(reader->lengthInSamples));
        if(!reader->read(&decoded,0,decoded.getNumSamples(),0,true,true))return juce::Result::fail("Cannot decode impulse response.");
    }else {decoded.setSize(1,1);decoded.setSample(0,0,1);}
    {const juce::SpinLock::ScopedLockType lock(sampleLock);pendingImpulse=std::move(decoded);pendingImpulseRate=rate;impulsePending=true;impulseBytes=bytes;}
    return juce::Result::ok();
}
