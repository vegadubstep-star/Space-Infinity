#include "../Source/PluginProcessor.h"
#include <iostream>
#include <stdexcept>
#include <vector>
namespace {
void need(bool x,const char* why){if(!x)throw std::runtime_error(why);}
void set(SpaceInfinityProcessor& p,const juce::String& id,float v){auto* a=p.state.getParameter(id);need(a!=nullptr,"bass parameter exists");a->setValueNotifyingHost(a->convertTo0to1(v));}
bool bass(SpaceInfinityProcessor& p,int i){return p.presetCategory(i).contains("Bass");}
void checkMidiPassThrough(SpaceInfinityProcessor& p,const juce::MidiBuffer& input){
 need(p.state.getRawParameterValue("patternMode")->load()==0,"bass has no note generator");
 for(const auto generated:p.generatedNotes())if(generated.getMessage().isNoteOn()){
  bool found=false;for(const auto original:input)if(original.getMessage().isNoteOn()&&original.getMessage().getNoteNumber()==generated.getMessage().getNoteNumber()&&original.getMessage().getChannel()==generated.getMessage().getChannel())found=true;
  need(found,"bass adds no notes beyond customer MIDI");
 }
}
float rangePeak(SpaceInfinityProcessor& p,float* measuredTruePeak=nullptr){
 float peak=0;juce::AudioBuffer<float> audio(2,128);juce::dsp::Oversampling<float> meter(2,2,juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple);meter.initProcessing(128);
 for(int key:{36,43,48,55}){p.prepareToPlay(48000,128);p.takeOutputPeak();for(int block=0;block<144;++block){juce::MidiBuffer midi;if(block==0)midi.addEvent(juce::MidiMessage::noteOn(1,key,1.0f),7);if(block==90)midi.addEvent(juce::MidiMessage::noteOff(1,key),11);const juce::MidiBuffer input(midi);p.processBlock(audio,midi);checkMidiPassThrough(p,input);if(measuredTruePeak){const auto high=meter.processSamplesUp(juce::dsp::AudioBlock<const float>(audio));for(size_t ch=0;ch<high.getNumChannels();++ch)for(size_t n=0;n<high.getNumSamples();++n)*measuredTruePeak=std::max(*measuredTruePeak,std::abs(high.getSample(int(ch),int(n))));}for(int i=0;i<128;++i)need(std::isfinite(audio.getSample(0,i)),"bass register sweep finite");}peak=std::max(peak,p.takeOutputPeak());}return peak;
}
void groove(SpaceInfinityProcessor& p,const juce::File& file,bool rolling){
 constexpr int sr=48000,blockSize=128;const int step=juce::roundToInt(sr*60.0/140/(rolling?4:1)),count=rolling?64:16,length=count*step+sr/3;
 struct Event{int at;juce::MidiMessage message;};std::vector<Event> events;constexpr int notes[]{40,40,40,43,40,40,47,40,40,40,38,40,40,43,40,40};
 for(int n=0;n<count;++n){if(n%16==15)continue;const int note=notes[n%16],start=n*step+(rolling?0:step/2);events.push_back({start,juce::MidiMessage::noteOn(1,note,n%4==0?.95f:.76f)});events.push_back({start+juce::roundToInt(step*(rolling?.69:.35)),juce::MidiMessage::noteOff(1,note)});}
 std::stable_sort(events.begin(),events.end(),[](const auto& a,const auto& b){return a.at<b.at;});size_t event=0;p.prepareToPlay(sr,blockSize);p.takeOutputPeak();juce::AudioBuffer<float> audio(2,blockSize);auto stream=file.createOutputStream();need(stream!=nullptr,"bass audition stream");juce::WavAudioFormat format;std::unique_ptr<juce::AudioFormatWriter> writer(format.createWriterFor(stream.get(),sr,2,24,{},0));need(writer!=nullptr,"bass audition writer");stream.release();
 for(int start=0;start<length;start+=blockSize){juce::MidiBuffer midi;while(event<events.size()&&events[event].at<start+blockSize){midi.addEvent(events[event].message,events[event].at-start);++event;}const juce::MidiBuffer input(midi);p.processBlock(audio,midi);checkMidiPassThrough(p,input);need(writer->writeFromAudioSampleBuffer(audio,0,blockSize),"bass audition write");}
}
}
int runBassSmoke(){try{
 auto p=std::make_unique<SpaceInfinityProcessor>();int count=0;
 for(int index=1;index<p->presetNames().size();++index)if(bass(*p,index)){
  ++count;p->loadPreset(index);p->prepareToPlay(48000,128);juce::AudioBuffer<float> audio(2,128);double energy=0,mono=0,body=0,attack=0,tail=0;float peak=0;
  for(int block=0;block<450;++block){juce::MidiBuffer midi;if(block==0)midi.addEvent(juce::MidiMessage::noteOn(1,40,.82f),0);if(block==160)midi.addEvent(juce::MidiMessage::noteOff(1,40),0);const juce::MidiBuffer input(midi);p->processBlock(audio,midi);checkMidiPassThrough(*p,input);for(int i=0;i<128;++i){const float l=audio.getSample(0,i),r=audio.getSample(1,i);need(std::isfinite(l)&&std::isfinite(r),"bass output finite");const double e=.5*(l*l+r*r);peak=std::max(peak,std::max(std::abs(l),std::abs(r)));energy+=e;mono+=(l+r)*(l+r)*.25;if(block>=15&&block<45)attack+=e;if(block>=60&&block<120)body+=e*.5;if(block>=400)tail+=e;}}
  need(peak>.02&&peak<.99,"bass audible with headroom");need(mono>energy*.98,"bass fundamental and body remain mono compatible");need(body>attack*.003,"bass keeps body behind its transient");need(tail<energy*.0001,"bass releases cleanly without a lingering effect tail");
  p->prepareToPlay(48000,128);std::array<double,12> hit{};for(int block=0;block<12*40;++block){const int note=block/40,pos=block%40;juce::MidiBuffer midi;if(pos==0)midi.addEvent(juce::MidiMessage::noteOn(1,40,.82f),0);if(pos==28)midi.addEvent(juce::MidiMessage::noteOff(1,40),0);p->processBlock(audio,midi);for(int i=0;i<128;++i){const float v=audio.getSample(0,i);hit[size_t(note)]+=v*v;}}double early=0,late=0;for(int i=2;i<6;++i)early+=hit[size_t(i)];for(int i=8;i<12;++i)late+=hit[size_t(i)];need(late>early*.8&&late<early*1.25,"twelve bass retriggers retain their level");
  // Static loudness matching uses a reference phrase. Resonant registers must
  // remain safe at the actual output, including reconstructed intersample peaks.
  float truePeak=0;const float rawPeak=rangePeak(*p,&truePeak);need(std::isfinite(rawPeak)&&truePeak<.995f,"four bass registers retain true-peak output headroom");
  std::cout<<p->presetNames()[index]<<": body/attack "<<body/attack<<", tail "<<tail/energy<<", repeat ratio "<<late/early<<std::endl;
 }
 need(count==35,"all 35 authored basses checked");std::cout<<"Bass: mono weight, body, clean release and repeated-note stability passed\n";return 0;
}catch(const std::exception& e){std::cerr<<"Bass: "<<e.what()<<std::endl;return 178;}}
int exportBassAuditions(const juce::File& folder,const juce::File& oldFile){try{
 folder.createDirectory();auto p=std::make_unique<SpaceInfinityProcessor>();const auto old=oldFile.existsAsFile()?juce::JSON::parse(oldFile.loadFileAsString()):juce::var{};juce::Array<juce::var> rows;
 for(int index=1;index<p->presetNames().size();++index)if(bass(*p,index)){
  p->loadPreset(index);const auto name=p->presetNames()[index];const bool rolling=p->presetCategory(index)=="Rolling Bass";groove(*p,folder.getChildFile(name+" - 063.wav"),rolling);auto* row=new juce::DynamicObject;row->setProperty("name",name);row->setProperty("raw_peak",p->takeOutputPeak());row->setProperty("preset_trim",p->state.getRawParameterValue("presetTrim")->load());rows.add(juce::var(row));
  if(old.isObject())for(const auto& patch:*old["presets"].getArray())if(patch["name"].toString()==name){p->loadPreset(index);for(const auto& v:patch["settings"].getDynamicObject()->getProperties())set(*p,v.name.toString(),float(v.value));groove(*p,folder.getChildFile(name+" - 062.wav"),rolling);}
  std::cout<<"Rendered bass groove "<<name<<std::endl;
 }
 need(folder.getChildFile("groove-profile.json").replaceWithText(juce::JSON::toString(rows)),"bass groove profile");return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<std::endl;return 179;}}

int exportBassRangeProfile(const juce::File& file){try{
 auto p=std::make_unique<SpaceInfinityProcessor>();juce::Array<juce::var> rows;
 for(int index=1;index<p->presetNames().size();++index)if(bass(*p,index)){p->loadPreset(index);auto* row=new juce::DynamicObject;row->setProperty("name",p->presetNames()[index]);row->setProperty("raw_peak",rangePeak(*p));row->setProperty("preset_trim",p->state.getRawParameterValue("presetTrim")->load());rows.add(juce::var(row));}
 file.getParentDirectory().createDirectory();need(file.replaceWithText(juce::JSON::toString(rows)),"bass range profile");return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<std::endl;return 180;}}
