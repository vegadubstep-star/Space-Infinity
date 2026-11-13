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
    if(mode==0){displayedPatternStep.store(-1);patternMidi.addEvents(midi,0,samples,0);return;}
    pattern.order=int(param("arpOrder"));pattern.octaves=int(param("arpOctaves"));
    pattern.length=int(param("sequenceLength"));pattern.gate=param("patternGate");
    for(int step=0;step<16;++step){
        pattern.pitch[size_t(step)]=int(readParameter(stepParameters[0][size_t(step)]));
        pattern.velocity[size_t(step)]=readParameter(stepParameters[1][size_t(step)]);
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
    displayedPatternStep.store(pattern.index>0?(pattern.index-1)%juce::jmax(1,pattern.length):-1);

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
    state.getParameter("irPreset")->setValueNotifyingHost(0);
    state.state.setProperty("impulseName",file.getFileName(),nullptr);
    const auto result=queueImpulse(bytes);if(result.failed())return result;
    state.getParameter("convolverEnabled")->setValueNotifyingHost(1);return juce::Result::ok();
}
juce::Result SpaceInfinityProcessor::queueImpulse(const juce::MemoryBlock& bytes){
    if(bytes.getSize()){juce::WavAudioFormat format;std::unique_ptr<juce::AudioFormatReader> r(format.createReaderFor(new juce::MemoryInputStream(bytes,false),true));
        if(!r||r->numChannels<1||r->numChannels>2||r->sampleRate<8000||r->sampleRate>192000||r->lengthInSamples<1||r->lengthInSamples>r->sampleRate*4)return juce::Result::fail("Invalid impulse response.");}
    {const juce::ScopedLock lock(impulseSourceLock);impulseBytes=bytes;}
    impulseRequest.fetch_add(1,std::memory_order_relaxed);rebuildImpulse();return juce::Result::ok();
}

juce::Result SpaceInfinityProcessor::loadBankPreset(int index){
    const auto& entries=library->entries();if(index<0||index>=int(entries.size()))return juce::Result::fail("Select a preset.");
    const auto entry=entries[size_t(index)];
    if(entry.factory>=0&&entry.file.isEmpty())loadPreset(entry.factory);
    else {juce::MemoryBlock bytes;const auto result=library->read(index,bytes);if(result.failed())return result;setStateInformation(bytes.getData(),int(bytes.getSize()));}
    state.state.setProperty("creator",entry.creator,nullptr);state.state.setProperty("tags",entry.tags,nullptr);state.state.setProperty("presetDisplayName",entry.name,nullptr);state.state.setProperty("presetKey",entry.key,nullptr);return juce::Result::ok();
}
juce::Result SpaceInfinityProcessor::saveNewPreset(const juce::String& name){
    state.state.setProperty("presetDisplayName",name,nullptr);juce::MemoryBlock bytes;getStateInformation(bytes);juce::String key;
    const auto result=library->add(name,bytes,key);if(result.wasOk())state.state.setProperty("presetKey",key,nullptr);return result;
}
juce::Result SpaceInfinityProcessor::replaceBankPreset(int index){
    juce::MemoryBlock bytes;getStateInformation(bytes);const auto result=library->replace(index,bytes);if(result.wasOk())state.state.setProperty("presetKey",library->entries()[size_t(index)].key,nullptr);return result;
}

juce::Result SpaceInfinityProcessor::exportPreset(const juce::File& file,bool overwrite){
    if(!file.hasFileExtension("sipreset")||file.isDirectory())return juce::Result::fail("Choose a .sipreset filename.");
    if(!file.getParentDirectory().isDirectory())return juce::Result::fail("The selected folder does not exist.");
    if(file.existsAsFile()&&!overwrite)return juce::Result::fail("A preset with that filename already exists.");
    juce::MemoryBlock bytes;getStateInformation(bytes);juce::TemporaryFile temporary(file);
    if(!temporary.getFile().replaceWithData(bytes.getData(),bytes.getSize())||!temporary.overwriteTargetFileWithTemporary())
        return juce::Result::fail("Cannot save the preset in that folder. Check the folder permissions and free space.");
    lastPresetExportFolder=file.getParentDirectory();return juce::Result::ok();
}

juce::Result SpaceInfinityProcessor::hybridisePresets(int a,int b,float blend){
    juce::MemoryBlock original,first,second;getStateInformation(original);auto r=loadBankPreset(a);if(r.failed())return r;getStateInformation(first);r=loadBankPreset(b);if(r.failed()){setStateInformation(original.getData(),int(original.getSize()));return r;}getStateInformation(second);auto xa=juce::AudioProcessor::getXmlFromBinary(first.getData(),int(first.getSize())),xb=juce::AudioProcessor::getXmlFromBinary(second.getData(),int(second.getSize()));setStateInformation(first.getData(),int(first.getSize()));blend=juce::jlimit(0.0f,1.0f,blend);const float parentGain=param("gain"),parentTrim=param("presetTrim");
    for(auto* parameter:getParameters())if(auto* p=dynamic_cast<juce::RangedAudioParameter*>(parameter)){const auto id=p->paramID;if(id=="source"||id.startsWith("sample")||id.startsWith("grain")||id.startsWith("ir")||id=="convolverEnabled"||id.startsWith("osc1")||id=="wave"||id=="wavetableScan"||id=="engineMode")continue;auto* left=xa->getChildByAttribute("id",id);auto* right=xb->getChildByAttribute("id",id);if(!left||!right)continue;const float x=float(left->getDoubleAttribute("value")),y=float(right->getDoubleAttribute("value"));float value=x+(y-x)*blend;if(dynamic_cast<juce::AudioParameterChoice*>(p)||dynamic_cast<juce::AudioParameterBool*>(p)||dynamic_cast<juce::AudioParameterInt*>(p))value=blend>.5f?y:x;if(id=="osc2Wave"&&int(value)==31)value=x;if((id.containsIgnoreCase("frequency")||id=="cutoff"||id.containsIgnoreCase("decay")||id.containsIgnoreCase("release")||id.containsIgnoreCase("attack"))&&x>0&&y>0)value=std::exp(std::log(x)*(1-blend)+std::log(y)*blend);p->setValueNotifyingHost(p->convertTo0to1(value));}
    const auto set=[this](const char* id,float value){auto* p=state.getParameter(id);p->setValueNotifyingHost(p->convertTo0to1(value));};set("patternMode",0);set("gateEnabled",0);set("pumpEnabled",0);set("gain",parentGain);set("presetTrim",juce::jlimit(-24.0f,24.0f,parentTrim-6));set("levelMatch",0);state.state.setProperty("presetKey","HYBRID",nullptr);state.state.setProperty("presetDisplayName","Hybrid - "+library->entries()[size_t(a)].name+" + "+library->entries()[size_t(b)].name,nullptr);state.state.setProperty("tags","Hybrid, "+library->entries()[size_t(a)].tags+", "+library->entries()[size_t(b)].tags,nullptr);resetVoicesRequested.store(true);return juce::Result::ok();
}
