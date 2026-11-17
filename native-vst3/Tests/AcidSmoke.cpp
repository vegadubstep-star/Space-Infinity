#include "../Source/PluginProcessor.h"
#include "../Source/DistortionFX.h"
#include "fixtures/DistortionLegacy063.h"
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
std::array<double,2> filterPeak(float cutoff,float resonance,double sr){
 AcidResonantFilter filter;std::vector<float> data(size*2);juce::dsp::FFT fft(15);
 const float g=std::tan(float(juce::MathConstants<double>::pi*cutoff/(sr*1.15)));
 for(int i=0;i<size;++i)data[size_t(i)]=filter.process(i==0?.001f:0,g,resonance,.08f);
 fft.performFrequencyOnlyForwardTransform(data.data());int peak=1;double maximum=0;
 for(int bin=1;bin<size/2;++bin)if(data[size_t(bin)]>maximum){maximum=data[size_t(bin)];peak=bin;}
 return {peak*sr/size,maximum};
}
std::array<double,3> patchSpectrum(SpaceInfinityProcessor& p,float cutoff){
 set(p,"cutoff",cutoff);p.prepareToPlay(48000,256);juce::AudioBuffer<float> audio(2,256);std::vector<float> data(size*2);double total=0,high=0,weighted=0;float peak=0;
 for(int block=0;block<256;++block){juce::MidiBuffer midi;if(block==0)midi.addEvent(juce::MidiMessage::noteOn(1,45,.85f),0);p.processBlock(audio,midi);peak=std::max(peak,audio.getMagnitude(0,256));if(block>=128)for(int i=0;i<256;++i)data[size_t((block-128)*256+i)]=audio.getSample(0,i)*(.5f-.5f*std::cos(juce::MathConstants<float>::twoPi*((block-128)*256+i)/(size-1)));}
 juce::dsp::FFT fft(15);fft.performFrequencyOnlyForwardTransform(data.data());
 for(int i=1;i<size/2;++i){const double energy=data[size_t(i)]*double(data[size_t(i)]),hz=i*48000.0/size;total+=energy;weighted+=energy*hz;if(hz>2000)high+=energy;}
 need(total>1e-5&&peak<.98f,"acid preset audible with sweep headroom");return {weighted/total,high/total,peak};
}
void cutoffChecks(){
 for(double sr:{44100.0,48000.0,96000.0})for(float cutoff:{180.0f,800.0f,3200.0f}){const auto flat=filterPeak(cutoff,0,sr),res=filterPeak(cutoff,.92f,sr);std::cout<<"Acid peak "<<sr<<" Hz: cutoff "<<cutoff<<", peak "<<res[0]<<", resonance gain "<<res[1]/flat[1]<<std::endl;need(res[0]>cutoff*.7&&res[0]<cutoff*1.4&&res[1]>flat[1]*1.8,"resonant peak follows cutoff across audio rates");}
 for(double sr:{44100.0,48000.0,96000.0})for(float res:{0.0f,.6f,.95f})for(float drive:{0.0f,.5f,1.0f}){AcidResonantFilter filter;float previous=0;double tail=0;for(int i=0;i<int(sr*1.2);++i){const float t=float(i%(int(sr)/4))/(sr*.25f),fc=80*std::pow(200.0f,t);const float g=std::tan(float(juce::MathConstants<double>::pi*fc/(sr*1.15)));const float x=i<sr*.7?.6f*std::sin(float(i)*.08f):0;const float y=filter.process(x,g,res,drive);need(std::isfinite(y)&&std::abs(y)<12,"fast resonant sweeps stable at all drive amounts");previous=y;if(i>sr)tail+=y*y;}need(tail<.01,"acid feedback decays after input stops");}
 auto p=std::make_unique<SpaceInfinityProcessor>();for(int index=1;index<p->presetNames().size();++index)if(p->presetCategory(index)=="Acid"){
  p->loadPreset(index);need(p->state.getRawParameterValue("filterMode")->load()==6,"factory acids use the cutoff-focused response");const auto low=patchSpectrum(*p,160),high=patchSpectrum(*p,8000);std::cout<<p->presetNames()[index]<<" sweep centroid "<<low[0]<<" -> "<<high[0]<<", high-band "<<low[1]<<" -> "<<high[1]<<std::endl;need(high[1]>.001&&high[1]>low[1]*5,"actual driven acid preset opens clearly with cutoff");
 }
}
float renderCutoffSweep(SpaceInfinityProcessor& p,const juce::File& file){
 p.prepareToPlay(48000,256);p.takeOutputPeak();juce::WavAudioFormat format;auto stream=file.createOutputStream();need(stream!=nullptr,"cutoff audition stream");std::unique_ptr<juce::AudioFormatWriter> writer(format.createWriterFor(stream.get(),48000,2,24,{},0));need(writer!=nullptr,"cutoff audition writer");stream.release();juce::AudioBuffer<float> audio(2,256);
 constexpr int notes[]{45,45,57,45,48,45,55,45,45,57,55,48,45,43,45,45};int next=0,off=-1;
 for(int block=0;block<1500;++block){const int start=block*256;juce::MidiBuffer midi;const float t=float(start)/384000,travel=t<.5f?t*2:2-t*2;set(p,"cutoff",80*std::pow(200.0f,travel));
  if(off>=start&&off<start+256){midi.addEvent(juce::MidiMessage::noteOff(1,notes[(next-1)%16]),off-start);off=-1;}
  if(next*6000<start+256){const int at=next*6000;if(at>=start){midi.addEvent(juce::MidiMessage::noteOn(1,notes[next%16],next%4==0?1.0f:.72f),at-start);off=at+4500;}++next;}
  p.processBlock(audio,midi);need(writer->writeFromAudioSampleBuffer(audio,0,256),"cutoff sweep write");
 }
 return p.takeOutputPeak();
}
}
int runAcidSmoke(const juce::File& folder={},const juce::File& previousBank={}){try{
 cutoffChecks();
 double legacyError=0;
 for(int type=0;type<17;++type)for(int position=0;position<3;++position){
  studio_legacy::Distortion before;studio::Distortion after;before.prepare(48000);after.prepare(48000);studio_legacy::Distortion::Settings left;studio::Distortion::Settings right;
  left.type=right.type=type;left.filterPosition=right.filterPosition=position;left.drive=right.drive=.55f;left.inputDb=right.inputDb=12;left.hq=right.hq=true;left.tone=right.tone=8700;left.frequency=right.frequency=3500;left.q=right.q=2;left.filterMorph=right.filterMorph=.38f;left.mix=right.mix=.8f;before.configure(left);after.configure(right);
  for(int n=0;n<6000;++n){if(n==3000){left.drive=right.drive=.8f;left.inputDb=right.inputDb=24;before.configure(left);after.configure(right);}const float in=.03f*std::sin(n*.031f)+.012f*std::sin(n*.013f);const auto a=before.process(in,-in*.7f),b=after.process(in,-in*.7f);for(int ch=0;ch<2;++ch)legacyError=std::max(legacyError,double(std::abs(a[size_t(ch)]-b[size_t(ch)])));}
 }
 std::cout<<"Standard-range compatibility against 0.6.3: maximum sample difference "<<legacyError<<std::endl;need(legacyError<1e-6,"existing distortion sounds retain their standard-range response");
 studio::Distortion::Settings standard;standard.drive=.55f;standard.inputDb=12;standard.hq=true;
 const auto gentle=probe(standard,150,.01f);standard.hot=true;const auto hot=probe(standard,150,.01f);
 const double gentleRatio=gentle.harmonics/gentle.fundamental,hotRatio=hot.harmonics/hot.fundamental;
 std::cout<<"Quiet-note THD power at same Drive/Input: standard "<<gentleRatio<<", HOT "<<hotRatio<<std::endl;
 need(hotRatio>.01&&hotRatio>gentleRatio*8,"HOT adds actual harmonic distortion on quiet notes, not just output gain");
 for(double rate:{44100.0,48000.0,96000.0})for(int type=0;type<studio::Distortion::names().size();++type){
  studio::Distortion effect;effect.prepare(rate);auto settings=standard;settings.type=type;settings.inputDb=18;settings.drive=.68f;settings.tone=11000;effect.configure(settings);double early=0,late=0;
  for(int n=0;n<int(rate*2);++n){const float input=.012f*std::sin(float(juce::MathConstants<double>::twoPi*190*n/rate));const auto out=effect.process(input,-input);for(float v:out)need(std::isfinite(v)&&std::abs(v)<25,"all HOT characters stay finite at supported sample rates");if(n>rate*.5&&n<rate)early+=out[0]*out[0];if(n>rate*1.5)late+=out[0]*out[0];}
  need(early>.001&&late>early*.85&&late<early*1.15,"HOT sustains its character without a slow gain fade");
  settings.mix=0;effect.configure(settings);for(int n=0;n<20000;++n)effect.process(.2f,-.1f);const auto dry=effect.process(.2f,-.1f);need(std::abs(dry[0]-.2f)<1e-5&&std::abs(dry[1]+.1f)<1e-5,"HOT zero mix returns dry signal");
 }
 studio::Distortion::Settings s;s.type=0;s.drive=.25f;s.mix=1;auto old=probe(s,150,.01f);s.inputDb=24;s.drive=.65f;s.hq=true;auto driven=probe(s,150,.01f);
 const double before=old.harmonics/old.fundamental,after=driven.harmonics/driven.fundamental;std::cout<<"Low-level sine THD power: legacy "<<before<<", driven "<<after<<std::endl;need(after>.015&&after>before*32,"quiet input receives genuine harmonic distortion, independent of output loudness");
 s.type=2;s.drive=.85f;s.inputDb=18;s.hq=false;auto aliased=probe(s,4781,.4f);s.hq=true;auto hq=probe(s,4781,.4f);const double aliasBefore=aliased.alias/aliased.total,aliasAfter=hq.alias/hq.total;std::cout<<"Matched-power alias fraction: plain "<<aliasBefore<<", HQ "<<aliasAfter<<std::endl;need(aliasAfter<aliasBefore*.8,"HQ reduces folded high-frequency energy at equal signal power");
 s.hot=true;s.hq=false;const auto hotAliased=probe(s,4781,.4f);s.hq=true;const auto hotHq=probe(s,4781,.4f);const double hotAliasBefore=hotAliased.alias/hotAliased.total,hotAliasAfter=hotHq.alias/hotHq.total;std::cout<<"HOT alias fraction: plain "<<hotAliasBefore<<", HQ "<<hotAliasAfter<<std::endl;need(hotAliasAfter<hotAliasBefore*.8,"HQ also reduces folded energy in the expanded range");
 for(int type=0;type<studio::Distortion::names().size();++type)for(int pos=0;pos<3;++pos){studio::Distortion effect;effect.prepare(96000);s.type=type;s.filterPosition=pos;s.frequency=4000;s.q=4;s.filterMorph=.5;s.inputDb=36;s.drive=1;s.mix=1;s.hq=true;s.tone=8500;effect.configure(s);double energy=0;for(int i=0;i<24000;++i){const float x=.02f*std::sin(float(i)*.041f);const auto out=effect.process(x,-x*.7f);for(float v:out){need(std::isfinite(v)&&std::abs(v)<25,"driven HQ mode/filter output finite");energy+=v*v;}}need(energy>.00001,"driven modes remain audible");}
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

int exportAcidCutoffSweeps(const juce::File& folder,const juce::File& previousBank){try{
 folder.createDirectory();juce::Array<juce::var> rows;auto p=std::make_unique<SpaceInfinityProcessor>();const auto oldBank=previousBank.existsAsFile()?juce::JSON::parse(previousBank.loadFileAsString()):juce::var{};
 for(int index=1;index<p->presetNames().size();++index)if(p->presetCategory(index)=="Acid"){
  const auto name=p->presetNames()[index];p->loadPreset(index);const float peak=renderCutoffSweep(*p,folder.getChildFile(name+" - 062 Sweep.wav"));auto* row=new juce::DynamicObject;row->setProperty("name",name);row->setProperty("raw_peak",peak);row->setProperty("preset_trim",p->state.getRawParameterValue("presetTrim")->load());rows.add(juce::var(row));std::cout<<name<<" full cutoff sweep raw peak "<<peak<<std::endl;
  if(oldBank.isObject())for(const auto& old:*oldBank["presets"].getArray())if(old["name"].toString()==name){p->loadPreset(index);for(const auto& value:old["settings"].getDynamicObject()->getProperties())set(*p,value.name.toString(),float(value.value));renderCutoffSweep(*p,folder.getChildFile(name+" - 061 Sweep.wav"));}
 }
 need(folder.getChildFile("sweep-profile.json").replaceWithText(juce::JSON::toString(rows)),"sweep profile export");
 std::cout<<"Rendered ten automated cutoff comparisons\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<std::endl;return 177;}}
