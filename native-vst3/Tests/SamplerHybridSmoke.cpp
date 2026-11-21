#include "../Source/PluginEditor.h"
#include <iostream>
#include <stdexcept>
#include <vector>
namespace {
void need(bool value,const char* why){if(!value)throw std::runtime_error(why);}
void set(SpaceInfinityProcessor& p,const char* id,float value){auto* a=p.state.getParameter(id);need(a!=nullptr,id);a->setValueNotifyingHost(a->convertTo0to1(value));}
juce::Component* named(juce::Component& c,const juce::String& name){if(c.getName()==name)return &c;for(auto* child:c.getChildren())if(auto* found=named(*child,name))return found;return nullptr;}
std::array<std::vector<float>,2> render(SpaceInfinityProcessor& p,int frames=48000){p.prepareToPlay(48000,128);juce::AudioBuffer<float> block(2,128);std::array<std::vector<float>,2> out;for(int i=0;i<frames;i+=128){juce::MidiBuffer midi;if(!i)midi.addEvent(juce::MidiMessage::noteOn(1,60,1.f),0);p.processBlock(block,midi);for(int n=0;n<juce::jmin(128,frames-i);++n)for(int c=0;c<2;++c){const float x=block.getSample(c,n);need(std::isfinite(x)&&std::abs(x)<=1,"sample/hybrid output clips");out[size_t(c)].push_back(x);}}return out;}
double tone(const std::vector<float>& audio,double hz,int begin=12000,int end=36000){double re=0,im=0;end=juce::jmin(end,int(audio.size()));for(int n=begin;n<end;++n){re+=audio[size_t(n)]*std::cos(2*juce::MathConstants<double>::pi*hz*n/48000);im+=audio[size_t(n)]*std::sin(2*juce::MathConstants<double>::pi*hz*n/48000);}return 2*std::hypot(re,im)/(end-begin);}
}
int runBundled093Probe(const juce::File& folder){try{auto p=std::make_unique<SpaceInfinityProcessor>(folder);const auto result=p->ensureBundledPresets();if(result.failed())throw std::runtime_error(result.getErrorMessage().toStdString());return p->presetBank().entries().size()==429?0:94;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 95;}}
int runSamplerHybridSmoke(){try{
 const auto folder=juce::File::getSpecialLocation(juce::File::tempDirectory).getNonexistentChildFile("InfinityHybrid093",{},false);folder.createDirectory();auto p=std::make_unique<SpaceInfinityProcessor>(folder.getChildFile("Library"));
 const auto wav=folder.getChildFile("StereoPhase.wav");juce::AudioBuffer<float> source(2,48000);
 for(int n=0;n<48000;++n){const float bass=.15f*std::sin(2*juce::MathConstants<double>::pi*50*n/48000),upper=.05f*std::sin(2*juce::MathConstants<double>::pi*800*n/48000);source.setSample(0,n,bass+upper);source.setSample(1,n,-bass+upper);}
 {auto stream=wav.createOutputStream();juce::WavAudioFormat format;auto writer=std::unique_ptr<juce::AudioFormatWriter>(format.createWriterFor(stream.get(),48000,2,24,{},0));need(bool(writer),"fixture writer");stream.release();need(writer->writeFromAudioSampleBuffer(source,0,48000),"fixture samples");}
 set(*p,"eqEnabled",1);set(*p,"filterMode",2);set(*p,"cutoff",3000);set(*p,"gain",.1f);
 need(p->importSample(wav,true).wasOk(),"clean sample import");set(*p,"sampleLoop",1);const auto direct=render(*p);
 for(int c=0;c<2;++c){const double bass=tone(direct[size_t(c)],50),upper=tone(direct[size_t(c)],800);std::cout<<"Clean sample channel "<<c<<": 50Hz="<<bass<<" 800Hz="<<upper<<'\n';need(std::abs(20*std::log10(bass/.15))<.4,"clean import loses bass or level");need(std::abs(20*std::log10(upper/.05))<.4,"clean import changes spectral balance");}
 double correlation=0;for(size_t n=12000;n<36000;++n)correlation+=direct[0][n]*direct[1][n];need(correlation<0,"stereo phase was collapsed to mono");
 juce::MemoryBlock saved;p->getStateInformation(saved);auto recalled=std::make_unique<SpaceInfinityProcessor>(folder.getChildFile("Recall"));recalled->setStateInformation(saved.getData(),int(saved.getSize()));const auto restored=render(*recalled);need(std::abs(tone(restored[0],50)-tone(direct[0],50))<.0001,"stereo sample state recall changes bass");
 // A right-only file must remain audible in granular mode too.
 const auto rightFile=folder.getChildFile("RightOnly.wav");source.clear(0,0,48000);
 {auto stream=rightFile.createOutputStream();juce::WavAudioFormat format;auto writer=std::unique_ptr<juce::AudioFormatWriter>(format.createWriterFor(stream.get(),48000,2,24,{},0));need(bool(writer),"right-only writer");stream.release();need(writer->writeFromAudioSampleBuffer(source,0,48000),"right-only samples");}
 need(p->importSample(rightFile,true).wasOk(),"right-only import");set(*p,"source",4);set(*p,"sampleLoop",1);set(*p,"grainSpread",0);const auto granular=render(*p);double leftEnergy=0,rightEnergy=0;for(size_t n=12000;n<36000;++n){leftEnergy+=granular[0][n]*granular[0][n];rightEnergy+=granular[1][n]*granular[1][n];}need(rightEnergy>.001&&leftEnergy<rightEnergy*.001,"granular primary discards the right input channel");
 p->setStateInformation(saved.getData(),int(saved.getSize()));
 // Oscillator B must have independent pitch/gain/routing and outlive a one-shot.
 set(*p,"sampleOscEnabled",1);set(*p,"osc2Enabled",1);set(*p,"osc2Wave",3);set(*p,"osc2Unison",1);set(*p,"manualUnison",1);set(*p,"osc2Rand",0);set(*p,"osc2Phase",0);set(*p,"osc2Octave",1);set(*p,"osc2Semitone",0);set(*p,"osc2Tune",0);set(*p,"osc2Width",0);set(*p,"osc2Gain",0);set(*p,"routeB",0);set(*p,"osc2Warp",0);set(*p,"mix",0);
 const auto hybrid=render(*p);need(tone(hybrid[0],523.251)>tone(direct[0],523.251)+.04,"sample oscillator layer is missing");need(std::abs(tone(hybrid[0],50)-tone(direct[0],50))<.002,"adding oscillator changes sample bass");
 set(*p,"osc2Gain",-12);const auto quiet=render(*p);const double gainRatio=tone(quiet[0],523.251)/tone(hybrid[0],523.251);need(gainRatio>.23&&gainRatio<.28,"oscillator layer gain control is ineffective");
 set(*p,"osc2Gain",0);set(*p,"osc2Octave",0);const auto lower=render(*p);need(tone(lower[0],261.626)>.04,"oscillator octave does not transpose independently");
 set(*p,"sampleLoop",0);const auto oneShot=render(*p,96000);need(tone(oneShot[0],261.626,70000,90000)>.04,"oscillator stops when sample ends");
 auto editor=std::unique_ptr<juce::AudioProcessorEditor>(p->createEditor());juce::Thread::sleep(40);juce::Timer::callPendingTimersSynchronously();
 for(const auto& name:juce::StringArray{"Osc 2 octave","Oscillator 2 gain","Sample gain"})need(named(*editor,name)&&named(*editor,name)->isVisible(),"hybrid main controls are hidden");
 auto* porta=dynamic_cast<juce::Slider*>(named(*editor,"Portamento time"));need(porta!=nullptr,"portamento control missing");need(porta->getNormalisableRange().convertFrom0to1(.5)<.025&&porta->getMouseDragSensitivity()>=700,"portamento fine range overwritten by attachment");
 // Assignment amount windows must close with the preset selector and editor.
 auto* sampleControl=dynamic_cast<ModSlider*>(named(*editor,"Sample gain"));need(sampleControl!=nullptr,"sample modulation control");need(p->dropModulation("INFINITY:MACRO:0","sampleGain"),"sample gain assignment");const auto depth=p->modulationDepthFor("INFINITY:MACRO:0","sampleGain");sampleControl->showAmount(depth,"Sample gain amount");
 auto* selector=dynamic_cast<PresetSelector*>(named(*editor,"Preset selector"));need(selector!=nullptr,"preset selector");selector->beforeOpen();for(auto* child:editor->getChildren())need(dynamic_cast<juce::CallOutBox*>(child)==nullptr,"modulation amount stays over preset selector");
 editor->setVisible(true);sampleControl->showAmount(depth,"Sample gain amount");editor->setVisible(false);for(auto* child:editor->getChildren())need(dynamic_cast<juce::CallOutBox*>(child)==nullptr,"editor hide leaves a floating amount window");
 editor.reset();
 // At 30ms a slide must reach the new note, rather than taking several time constants.
 auto glide=std::make_unique<SpaceInfinityProcessor>(folder.getChildFile("Glide"));
 for(auto id:{"compEnabled","delayEnabled","reverbEnabled","chorusEnabled","flangerEnabled","phaserEnabled","gateEnabled","pumpEnabled","eqEnabled","convolverEnabled","distEnabled","analogueUpgrade","osc2Enabled"})set(*glide,id,0);
 set(*glide,"engineMode",1);set(*glide,"analogueCharacter",0);set(*glide,"routeA",0);set(*glide,"wave",2);set(*glide,"noiseLevel",0);set(*glide,"subLevel",0);set(*glide,"gain",.2f);set(*glide,"ampSustain",1);set(*glide,"attack",0);set(*glide,"release",.005f);set(*glide,"motionDepth",0);set(*glide,"lfodepth",0);
 const auto hz=[](const std::vector<float>& audio,int begin,int end){std::vector<double> crossings;for(int n=begin+1;n<end;++n)if(audio[size_t(n-1)]<=0&&audio[size_t(n)]>0)crossings.push_back(n-1-audio[size_t(n-1)]/(audio[size_t(n)]-audio[size_t(n-1)]));need(crossings.size()>3,"glide pitch probe has too few cycles");return (crossings.size()-1)*48000/(crossings.back()-crossings.front());};
 for(bool down:{false,true})for(int mode:{1,2}){set(*glide,"voiceMode",float(mode));set(*glide,"glideTime",.03f);glide->prepareToPlay(48000,128);juce::AudioBuffer<float> block(2,128);std::vector<float> result;
     for(int b=0;b<120;++b){juce::MidiBuffer midi;if(!b)midi.addEvent(juce::MidiMessage::noteOn(1,down?81:69,1.f),0);if(mode==1&&b==30)midi.addEvent(juce::MidiMessage::noteOff(1,down?81:69),0);if(b==40)midi.addEvent(juce::MidiMessage::noteOn(1,down?69:81,1.f),0);glide->processBlock(block,midi);for(int n=0;n<128;++n)result.push_back(block.getSample(0,n));}
     const double early=hz(result,40*128+200,40*128+600),finished=hz(result,40*128+1800,40*128+3500);const double initial=down?880:440,target=down?440:880;
     std::cout<<"30ms "<<(down?"down":"up")<<" glide mode "<<mode<<": "<<early<<" -> "<<finished<<" Hz\n";
     need(std::abs(finished-target)<3,"portamento does not finish within displayed duration");need(down?(early>target&&early<initial):(early>initial&&early<target),"portamento must travel monotonically between notes");
 }
 // A fresh phrase must not swoop from a note played long before it.
 for(float seconds:{0.f,.03f}){set(*glide,"voiceMode",1);set(*glide,"glideTime",seconds);glide->prepareToPlay(48000,128);juce::AudioBuffer<float> block(2,128);std::vector<float> result;
     for(int b=0;b<230;++b){juce::MidiBuffer midi;if(!b)midi.addEvent(juce::MidiMessage::noteOn(1,69,1.f),0);if(b==20)midi.addEvent(juce::MidiMessage::noteOff(1,69),0);if(b==200)midi.addEvent(juce::MidiMessage::noteOn(1,81,1.f),0);glide->processBlock(block,midi);for(int n=0;n<128;++n)result.push_back(block.getSample(0,n));}
     need(std::abs(hz(result,200*128+200,200*128+1000)-880)<3,"new phrase retains an old glide pitch");
 }
 need(p->ensureBundledPresets().wasOk(),"automatic bundled banks");const auto count=p->presetBank().entries().size();need(count==429,"all factory/extra/sample sounds must be installed");need(p->ensureBundledPresets().wasOk()&&p->presetBank().entries().size()==count,"bundled bank startup duplicates sounds");
 for(size_t i=0;i<count;++i)if(p->presetBank().entries()[i].key.startsWith("Uuser_orbitvoices093_")){const auto name=p->presetBank().entries()[i].name;need(p->loadBankPreset(int(i)).wasOk(),"bundled sample preset load");const auto audio=render(*p,8192);float peak=0;for(const auto& channel:audio)for(float x:channel)peak=juce::jmax(peak,std::abs(x));std::cout<<"Bundled "<<name<<" peak "<<peak<<'\n';need(peak>.0001f,"bundled sample preset is silent or its sample was not decoded");}

 const auto executable=juce::File::getSpecialLocation(juce::File::currentExecutableFile);const auto concurrent=folder.getChildFile("Concurrent");juce::StringArray command{executable.getFullPathName(),"--bundled093-probe",concurrent.getFullPathName()};juce::ChildProcess a,b;need(a.start(command)&&b.start(command),"concurrent bundled-bank probes start");need(a.waitForProcessToFinish(30000)&&b.waitForProcessToFinish(30000),"concurrent bank installation times out");std::cout<<a.readAllProcessOutput()<<b.readAllProcessOutput();need(a.getExitCode()==0&&b.getExitCode()==0,"concurrent instances fail to install all banks");
 // Curation may hide shipped duplicates, but must retain metadata/sound edits.
 auto& bank=p->presetBank();juce::Array<juce::var> retirements;
 for(const auto& key:juce::StringArray{"Uuser_heritage200_H002","Uuser_heritage200_H005","Uuser_heritage200_H025"}){const int index=bank.indexForKey(key);need(index>=0,"curation fixture");const auto e=bank.entries()[size_t(index)];auto* row=new juce::DynamicObject;row->setProperty("key",e.key);row->setProperty("name",e.name);row->setProperty("sha256",juce::SHA256(bank.folder().getChildFile(e.file)).toHexString());retirements.add(juce::var(row));}
 const int renamed=bank.indexForKey("Uuser_heritage200_H005");need(bank.updateDetails(renamed,bank.entries()[size_t(renamed)].name,"Local producer","Edited tag").wasOk(),"local metadata edit");
 const int edited=bank.indexForKey("Uuser_heritage200_H025");need(p->loadBankPreset(edited).wasOk(),"local sound edit fixture");set(*p,"cutoff",777);juce::MemoryBlock replacement;p->getStateInformation(replacement);need(bank.replace(edited,replacement).wasOk(),"local sound replacement");
 auto* plan=new juce::DynamicObject;plan->setProperty("retired",retirements);bank.setCuration(juce::var(plan));need(bank.reload().wasOk(),"curated reload");
 need(bank.indexForKey("Uuser_heritage200_H002")<0,"shipped near duplicate remains visible");need(bank.indexForKey("Uuser_heritage200_H005")>=0&&bank.indexForKey("Uuser_heritage200_H025")>=0,"curation removed local edits");need(p->ensureBundledPresets().wasOk()&&bank.indexForKey("Uuser_heritage200_H002")<0,"bundled startup resurrects retired duplicate");
 p.reset();recalled.reset();folder.deleteRecursively();std::cout<<"Stereo bass fidelity, independent hybrid oscillator, one-shot lifetime, controls, fine portamento and bundled banks passed\n";return 0;
}catch(const std::exception& e){std::cerr<<"Sampler hybrid: "<<e.what()<<'\n';return 93;}}
