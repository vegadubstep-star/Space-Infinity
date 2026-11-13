#include "../Source/PluginProcessor.h"
#include "../Source/DistortionFX.h"
#include <BinaryData.h>
#include <iostream>
#include <stdexcept>
#include <vector>
namespace {
void need(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
void set(SpaceInfinityProcessor& p,const juce::String& id,float value){auto* a=p.state.getParameter(id);need(a!=nullptr,"acid parameter exists");a->setValueNotifyingHost(a->convertTo0to1(value));}
constexpr int size=32768;
struct Spectrum {double total=0,fundamental=0,harmonics=0,alias=0;};
Spectrum probe(studio::Distortion::Settings settings,int bin,float amplitude){
 studio::Distortion effect;effect.prepare(48000);effect.configure(settings);std::vector<float> data(size*2);Spectrum result;
 for(int i=0;i<size*2;++i){const float x=amplitude*std::sin(juce::MathConstants<float>::twoPi*float(bin)*float(i)/size);const auto out=effect.process(x,x);need(std::isfinite(out[0]),"distortion probe finite");if(i>=size){data[size_t(i-size)]=out[0];result.total+=out[0]*out[0]/size;}}
 juce::dsp::FFT fft(15);fft.performFrequencyOnlyForwardTransform(data.data());const double scale=2.0/(double(size)*size);result.fundamental=data[size_t(bin)]*double(data[size_t(bin)])*scale;
 for(int n=2;n<=20&&bin*n<size/2;++n)result.harmonics+=data[size_t(bin*n)]*double(data[size_t(bin*n)])*scale;
 result.alias=std::max(0.0,result.total-result.fundamental-result.harmonics);return result;
}
void render(SpaceInfinityProcessor& p,const juce::File& file){
 p.prepareToPlay(48000,256);juce::WavAudioFormat format;auto stream=file.createOutputStream();need(stream!=nullptr,"acid audition stream");std::unique_ptr<juce::AudioFormatWriter> writer(format.createWriterFor(stream.get(),48000,2,24,{},0));need(writer!=nullptr,"acid audition writer");stream.release();
 struct Event {int frame;juce::MidiMessage message;};std::vector<Event> events;constexpr int notes[]{45,45,57,45,48,45,55,45,45,57,55,48,45,43,45,45};
 const double step=48000*60.0/140/4;
 for(int n=0;n<64;++n){if(n%16==15)continue;const int start=juce::roundToInt(n*step),end=juce::roundToInt((n+(n%7==3?1.12:.72))*step);events.push_back({start,juce::MidiMessage::noteOn(1,notes[n%16],n%4==0?1.0f:n%3==0?.87f:.65f)});events.push_back({end,juce::MidiMessage::noteOff(1,notes[n%16])});}
 std::stable_sort(events.begin(),events.end(),[](const auto& a,const auto& b){return a.frame<b.frame;});size_t event=0;juce::AudioBuffer<float> audio(2,256);float peak=0;
 const int length=juce::roundToInt(64*step+48000*.6);
 for(int start=0;start<length;start+=256){juce::MidiBuffer midi;while(event<events.size()&&events[event].frame<start+256){midi.addEvent(events[event].message,events[event].frame-start);++event;}p.processBlock(audio,midi);need(writer->writeFromAudioSampleBuffer(audio,0,256),"acid audition write");peak=std::max(peak,audio.getMagnitude(0,256));}need(peak>.00001f&&peak<=1,"acid audition audible and bounded");
}
}
int runAcidSmoke(const juce::File& folder={},const juce::File& previousBank={}){try{
 studio::Distortion::Settings s;s.type=0;s.drive=.25f;s.mix=1;auto old=probe(s,150,.01f);s.inputDb=24;s.drive=.65f;s.hq=true;auto driven=probe(s,150,.01f);
 const double before=old.harmonics/old.fundamental,after=driven.harmonics/driven.fundamental;std::cout<<"Low-level sine THD power: legacy "<<before<<", driven "<<after<<std::endl;need(after>.015&&after>before*32,"quiet input receives genuine harmonic distortion, independent of output loudness");
 s.type=2;s.drive=.85f;s.inputDb=18;s.hq=false;auto aliased=probe(s,4781,.4f);s.hq=true;auto hq=probe(s,4781,.4f);const double aliasBefore=aliased.alias/aliased.total,aliasAfter=hq.alias/hq.total;std::cout<<"Matched-power alias fraction: plain "<<aliasBefore<<", HQ "<<aliasAfter<<std::endl;need(aliasAfter<aliasBefore*.8,"HQ reduces folded high-frequency energy at equal signal power");
 for(int type=0;type<17;++type)for(int pos=0;pos<3;++pos){studio::Distortion effect;effect.prepare(96000);s.type=type;s.filterPosition=pos;s.frequency=4000;s.q=4;s.filterMorph=.5;s.inputDb=36;s.drive=1;s.mix=1;s.hq=true;s.tone=8500;effect.configure(s);double energy=0;for(int i=0;i<24000;++i){const float x=.02f*std::sin(float(i)*.041f);const auto out=effect.process(x,-x*.7f);for(float v:out){need(std::isfinite(v)&&std::abs(v)<25,"driven HQ mode/filter output finite");energy+=v*v;}}need(energy>.00001,"driven modes remain audible");}
 if(folder!=juce::File{}){
  folder.createDirectory();auto p=std::make_unique<SpaceInfinityProcessor>();const auto oldBank=previousBank.existsAsFile()?juce::JSON::parse(previousBank.loadFileAsString()):juce::var{};
  for(int index=1;index<p->presetNames().size();++index)if(p->presetCategory(index)=="Acid"){
   const auto name=p->presetNames()[index];p->loadPreset(index);render(*p,folder.getChildFile(name+" - NEW.wav"));
   if(oldBank.isObject())for(const auto& oldPatch:*oldBank["presets"].getArray())if(oldPatch["name"].toString()==name){p->loadPreset(index);if(auto* settings=oldPatch["settings"].getDynamicObject())for(const auto& v:settings->getProperties())set(*p,v.name.toString(),float(v.value));set(*p,"distInput",0);set(*p,"distHQ",0);set(*p,"distTone",20000);render(*p,folder.getChildFile(name+" - 060.wav"));}
   p->loadPreset(index);set(*p,"distEnabled",0);render(*p,folder.getChildFile(name+" - DRY.wav"));std::cout<<"Rendered acid comparison: "<<name<<std::endl;
  }
 }
 std::cout<<"Acid: low-level saturation, HQ alias reduction and all driven modes passed\n";return 0;
}catch(const std::exception& e){std::cerr<<"Acid: "<<e.what()<<std::endl;return 175;}}

int exportOwnerChoirStabs(const juce::File& folder){try{
 folder.createDirectory();folder.getChildFile("Presets").createDirectory();folder.getChildFile("One Shots").createDirectory();const auto temp=juce::File::getSpecialLocation(juce::File::tempDirectory).getNonexistentChildFile("InfinityOwnerChoirExport",{},false);temp.createDirectory();auto p=std::make_unique<SpaceInfinityProcessor>();int count=0;
 for(int index=1;index<p->presetNames().size();++index)if(p->presetCategory(index)=="Choir Stabs"){
  p->loadPreset(index);const int source=int(p->state.getRawParameterValue("source")->load());const FactorySampleZone* zone=nullptr;for(const auto& z:factorySampleZones)if(z.instrument==source-21){zone=&z;break;}need(zone!=nullptr,"uploaded choir zone available");int size=0;const char* data=BinaryData::getNamedResource(zone->resource,size);need(data&&size>0,"uploaded choir sample embedded");const auto wav=temp.getChildFile(p->presetNames()[index]+".wav");need(wav.replaceWithData(data,size_t(size)),"export source temp");need(p->importSample(wav).wasOk(),"portable choir sample import");set(*p,"source",3);set(*p,"sampleRoot",float(zone->root));const auto name=p->presetNames()[index];need(p->exportPreset(folder.getChildFile("Presets").getChildFile(name+".sipreset"),true).wasOk(),"portable choir preset export");
  p->prepareToPlay(48000,256);juce::AudioBuffer<float> audio(2,256),clip(2,72192);for(int block=0;block<282;++block){juce::MidiBuffer midi;if(block==0)midi.addEvent(juce::MidiMessage::noteOn(1,60,.85f),0);if(block==24)midi.addEvent(juce::MidiMessage::noteOff(1,60),0);p->processBlock(audio,midi);for(int ch=0;ch<2;++ch)clip.copyFrom(ch,block*256,audio,ch,0,256);}const float peak=clip.getMagnitude(0,clip.getNumSamples());need(peak>1e-6f,"choir stab preview audible");clip.applyGain(.89f/peak);clip.applyGainRamp(clip.getNumSamples()-480,480,1,0);auto file=folder.getChildFile("One Shots").getChildFile(name+".wav");file.deleteFile();auto stream=file.createOutputStream();juce::WavAudioFormat format;std::unique_ptr<juce::AudioFormatWriter> writer(format.createWriterFor(stream.get(),48000,2,24,{},0));need(writer!=nullptr,"choir stab WAV writer");stream.release();need(writer->writeFromAudioSampleBuffer(clip,0,clip.getNumSamples()),"choir stab WAV export");++count;
 }
 p.reset();temp.deleteRecursively();need(count==10,"ten uploaded choir stabs exported");std::cout<<"Exported ten portable choir presets with embedded samples and ten peak-normalised C4 one-shots\n";return 0;
}catch(const std::exception& e){std::cerr<<"Choir export: "<<e.what()<<std::endl;return 176;}}
