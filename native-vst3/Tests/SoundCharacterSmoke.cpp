#include "../Source/PluginEditor.h"
#include <iostream>
#include <stdexcept>
#include <vector>
#if JUCE_WINDOWS
#include <windows.h>
#include <cstdio>
#endif
namespace {
#if JUCE_WINDOWS
LONG WINAPI reportWindowsException(EXCEPTION_POINTERS* exception){char text[160]{};const int size=std::snprintf(text,sizeof(text),"Sound character Windows exception: 0x%08lx at %p\n",exception->ExceptionRecord->ExceptionCode,exception->ExceptionRecord->ExceptionAddress);DWORD written=0;WriteFile(GetStdHandle(STD_ERROR_HANDLE),text,DWORD(juce::jlimit(0,159,size)),&written,nullptr);return EXCEPTION_EXECUTE_HANDLER;}
#endif
void need(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
void set(SpaceInfinityProcessor& p,const juce::String& id,float v){auto* a=p.state.getParameter(id);need(a!=nullptr,"parameter missing");a->setValueNotifyingHost(a->convertTo0to1(v));}
void clean(SpaceInfinityProcessor& p){p.loadPreset(0);for(auto id:{"compEnabled","delayEnabled","reverbEnabled","chorusEnabled","flangerEnabled","phaserEnabled","gateEnabled","pumpEnabled","eqEnabled","convolverEnabled","distEnabled","osc2Enabled","analogueLabEnabled","analogueUpgrade"})set(p,id,0);set(p,"engineMode",1);set(p,"analogueCharacter",0);set(p,"soundLife",0);set(p,"subLevel",0);set(p,"noiseLevel",0);set(p,"mix",0);set(p,"osc1PhaseReset",1);set(p,"osc1Rand",0);set(p,"osc1Unison",1);set(p,"attack",0);set(p,"ampSustain",1);set(p,"gain",.2f);set(p,"presetTrim",0);set(p,"filterEnvAmount",0);set(p,"motionDepth",0);set(p,"lfodepth",0);}
std::vector<float> render(SpaceInfinityProcessor& p,int blocks=240){p.prepareToPlay(48000,128);juce::AudioBuffer<float> audio(2,128);std::vector<float> out;for(int block=0;block<blocks;++block){juce::MidiBuffer midi;if(!block)midi.addEvent(juce::MidiMessage::noteOn(1,69,.8f),0);p.processBlock(audio,midi);for(int n=0;n<128;++n){const float x=audio.getSample(0,n);need(std::isfinite(x)&&std::abs(x)<1,"sound integration clips or is non-finite");out.push_back(x);}}return out;}
double frequency(const std::vector<float>& x){int count=0;for(size_t n=x.size()/3+1;n<x.size();++n)if(x[n-1]<=0&&x[n]>0)++count;return count*48000.0/(x.size()-x.size()/3);}
juce::Component* named(juce::Component& c,const juce::String& name){if(c.getName()==name)return &c;for(auto* child:c.getChildren())if(auto* found=named(*child,name))return found;return nullptr;}
}
int runSoundCharacterSmoke(){try{
 std::cout<<std::unitbuf;
#if JUCE_WINDOWS
 ULONG guarantee=65536;SetThreadStackGuarantee(&guarantee);SetUnhandledExceptionFilter(reportWindowsException);
#endif
 std::cout<<"Sound character: filter renders"<<std::endl;
 auto p=std::make_unique<SpaceInfinityProcessor>();clean(*p);set(*p,"cutoff",1000);set(*p,"res",.8f);double previous=0;
 for(int mode=7;mode<15;++mode){set(*p,"characterFilterMode",float(mode-6));need(p->filterSelection()==mode,"extended filter selection");const auto audio=render(*p,80);double energy=0;for(auto x:audio)energy+=x*x;need(energy>1.e-7,"new filter is silent in actual synth");previous+=energy;}
 need(previous>0,"no new filter audio");auto* legacyFilter=dynamic_cast<juce::AudioParameterChoice*>(p->state.getParameter("filterMode"));need(legacyFilter&&legacyFilter->choices.size()==7&&legacyFilter->convertTo0to1(6)==1,"legacy filter automation range changed");set(*p,"characterFilterMode",0);set(*p,"filterMode",5);need(p->filterSelection()==5,"original filter automation no longer controls original model");
 std::cout<<"Sound character: sample fixture/import"<<std::endl;
 // A steady sine sample gives an independent octave measurement through actual import/playback.
 const auto file=juce::File::getSpecialLocation(juce::File::tempDirectory).getNonexistentChildFile("InfinityOctave",".wav");juce::AudioBuffer<float> sine(2,48000);for(int n=0;n<48000;++n){sine.setSample(0,n,.4f*std::sin(float(n)*juce::MathConstants<float>::twoPi*440/48000));sine.setSample(1,n,sine.getSample(0,n));}
 {juce::WavAudioFormat format;std::unique_ptr<juce::FileOutputStream> stream(file.createOutputStream());auto writer=std::unique_ptr<juce::AudioFormatWriter>(format.createWriterFor(stream.get(),48000,2,24,{},0));need(bool(writer),"sample fixture writer");stream.release();need(writer->writeFromAudioSampleBuffer(sine,0,48000),"sample fixture write");}
 clean(*p);need(p->importSample(file).wasOk(),"sample import");file.deleteFile();set(*p,"source",3);set(*p,"sampleRoot",69);set(*p,"routeA",0);set(*p,"sampleLoop",1);set(*p,"sampleLoopFade",20);
 for(int octave:{-1,0,1,2}){set(*p,"sampleOctave",float(octave));const auto audio=render(*p);const double measured=frequency(audio),expected=440*std::exp2(octave);std::cout<<"Sampler octave "<<octave<<": "<<measured<<" Hz\n";need(std::abs(measured-expected)<4,"sampler octave does not transpose correctly");}
 // Exercise actual sample pitch through overlapping and separated note events.
 set(*p,"sampleOctave",0);set(*p,"grainOctave",0);set(*p,"release",.005f);
 const auto sampleGlide=[&](int mode,bool separated,bool reverse,bool panic,int channel=1){
     set(*p,"voiceMode",float(mode));set(*p,"glideTime",.12f);set(*p,"sampleReverse",reverse?1.f:0.f);p->prepareToPlay(48000,128);
     juce::AudioBuffer<float> block(2,128);std::vector<float> early,late;
     for(int b=0;b<420;++b){juce::MidiBuffer midi;
         if(b==0)midi.addEvent(juce::MidiMessage::noteOn(1,69,.8f),0);
         if(b==70&&separated)midi.addEvent(juce::MidiMessage::noteOff(1,69),0);
         if(b==90&&panic)midi.addEvent(juce::MidiMessage::allSoundOff(1),0);
         if(b==100)midi.addEvent(juce::MidiMessage::noteOn(channel,81,.8f),0);
         p->processBlock(block,midi);for(int n=0;n<128;++n){const auto x=block.getSample(0,n);need(std::isfinite(x)&&std::abs(x)<1,"sample portamento audio clips");if(b>=101&&b<109)early.push_back(x);if(b>=360)late.push_back(x);}
     }
     const double opening=frequency(early),ending=frequency(late);
     std::cout<<"Sample portamento mode "<<mode<<" gap "<<separated<<" reverse "<<reverse<<" panic "<<panic<<" channel "<<channel<<": "<<opening<<" -> "<<ending<<" Hz\n";
     const bool slides=!panic&&channel==1&&(mode==1||!separated);
     need(slides?(opening>350&&opening<720):opening>780,"sample portamento onset/mode/reset behaviour");
     need(std::abs(ending-880)<12,"sample portamento does not reach its destination");
 };
 for(bool reverse:{false,true})for(int mode:{1,2})for(bool gap:{false,true})sampleGlide(mode,gap,reverse,false);
 sampleGlide(1,true,false,true);sampleGlide(1,true,false,false,2);
 set(*p,"source",3);set(*p,"voiceMode",0);set(*p,"glideTime",0);set(*p,"sampleReverse",0);set(*p,"sampleOctave",2);
 std::cout<<"Sound character: granular blend and state recall"<<std::endl;
 set(*p,"sampleCloudMix",.7f);set(*p,"sampleSmear",.5f);set(*p,"sampleDarken",.4f);set(*p,"grainFreeze",1);set(*p,"grainOctave",-1);render(*p);juce::MemoryBlock saved;p->getStateInformation(saved);auto recalled=std::make_unique<SpaceInfinityProcessor>();recalled->setStateInformation(saved.getData(),int(saved.getSize()));need(recalled->state.getRawParameterValue("sampleOctave")->load()==2&&recalled->state.getRawParameterValue("grainOctave")->load()==-1,"sampler pitch state recall");render(*recalled);
 std::cout<<"Sound character: sampler editor creation"<<std::endl;
 auto editor=std::unique_ptr<juce::AudioProcessorEditor>(p->createEditor());juce::Thread::sleep(45);juce::Timer::callPendingTimersSynchronously();need(named(*editor,"Sampler octave")!=nullptr&&named(*editor,"Granular layer octave")!=nullptr,"visible octave controls bind to sampler");set(*p,"source",0);juce::Thread::sleep(45);juce::Timer::callPendingTimersSynchronously();need(named(*editor,"Osc 1 octave")!=nullptr,"oscillator octave restores on synth engine");
 auto* porta=dynamic_cast<juce::Slider*>(named(*editor,"Portamento time"));need(porta&&bool(porta->onDragStart),"main portamento control exists");
 set(*p,"source",3);set(*p,"voiceMode",0);porta->onDragStart();need(p->state.getRawParameterValue("voiceMode")->load()==1,"sample portamento adjustment must select Mono from Poly");
 set(*p,"source",0);set(*p,"voiceMode",0);porta->onDragStart();need(p->state.getRawParameterValue("voiceMode")->load()==2,"oscillator portamento adjustment retains Legato workflow");
 std::cout<<"Sound character: editor teardown"<<std::endl;editor.reset();
 for(const auto& panelName:juce::StringArray{"Main filter response","Sample loop controls","Granular blend controls"}){set(*p,"source",panelName=="Main filter response"?0.f:3.f);auto reopened=std::unique_ptr<juce::AudioProcessorEditor>(p->createEditor());need(named(*reopened,panelName=="Main filter response"?"Osc 1 octave":"Sampler octave")!=nullptr,"reopened editor octave binding");auto* panel=named(*reopened,panelName);need(panel!=nullptr,"new edit panel exists");juce::TextButton* edit=nullptr;for(auto* child:panel->getChildren())if(auto* button=dynamic_cast<juce::TextButton*>(child))if(button->getButtonText()=="EDIT")edit=button;need(edit&&bool(edit->onClick),"panel EDIT action exists");edit->onClick();
     juce::CallOutBox* popup=nullptr;for(auto* child:reopened->getChildren())if(auto* box=dynamic_cast<juce::CallOutBox*>(child))popup=box;
     need(popup!=nullptr,"EDIT popup must be owned by the editor");
     juce::Button* bank=nullptr;PresetSelector* selector=nullptr;juce::Button* fx=nullptr;
     for(auto* child:reopened->getChildren()){
         if(auto* button=dynamic_cast<juce::Button*>(child)){if(button->getButtonText()=="BANK")bank=button;if(button->getButtonText()=="FX")fx=button;}
         if(auto* combo=dynamic_cast<PresetSelector*>(child))selector=combo;
     }
     need(bank&&selector&&fx,"preset/page navigation controls exist");bank->onClick();
     popup=nullptr;for(auto* child:reopened->getChildren())if(auto* box=dynamic_cast<juce::CallOutBox*>(child))popup=box;
     need(popup&&named(*popup,"Preset bank")&&!named(*popup,panelName),"BANK must replace sample/granular/filter popup");
     selector->beforeOpen();for(auto* child:reopened->getChildren())need(dynamic_cast<juce::CallOutBox*>(child)==nullptr,"preset dropdown must dismiss edit popup");
     edit->onClick();fx->onClick();for(auto* child:reopened->getChildren())need(dynamic_cast<juce::CallOutBox*>(child)==nullptr,"page navigation must dismiss edit popup");
     reopened.reset();}
 std::cout<<"Sound character: close/reopen passed"<<std::endl;
 std::cout<<"Sound character: voice-life renders"<<std::endl;
 clean(*p);set(*p,"wave",2);set(*p,"filterMode",1);set(*p,"cutoff",8000);const auto dry=render(*p,1000);set(*p,"soundLife",.8f);const auto life=render(*p,1000);double change=0,first=0,last=0;for(size_t n=0;n<life.size();++n){change+=std::abs(life[n]-dry[n]);if(n>24000&&n<48000)first+=life[n]*life[n];if(n>life.size()-24000)last+=life[n]*life[n];}need(change>1,"voice life does not alter sound");need(last>first*.75&&last<first*1.25,"voice life causes unwanted long volume fade");
 std::cout<<"Sound character: oscillator journeys"<<std::endl;
 clean(*p);set(*p,"routeA",0);set(*p,"routeB",0);set(*p,"wave",2);set(*p,"osc2Wave",3);set(*p,"osc2Enabled",1);set(*p,"osc2Octave",1);set(*p,"osc2PhaseReset",1);set(*p,"osc2Rand",0);set(*p,"osc2Unison",1);set(*p,"oscTransition",1);set(*p,"oscTransitionTime",.2f);
 for(int direction=0;direction<2;++direction){set(*p,"oscTransitionDirection",float(direction));const auto journey=render(*p,240);std::vector<float> opening(journey.begin(),journey.begin()+2048),ending(journey.end()-8192,journey.end());const double startHz=frequency(opening),endHz=frequency(ending);need(std::abs(startHz-(direction?880:440))<40&&std::abs(endHz-(direction?440:880))<10,"oscillator transition does not travel between the two pitches");}
 std::cout<<"Full synth filters, sampler octave audio/UI/recall, granular blending and stable voice-life level passed\n";return 0;
}catch(const std::exception& e){std::cerr<<"Sound character: "<<e.what()<<'\n';return 1;}}
int runLushBankSmoke(const juce::File& archive){try{
 const auto folder=juce::File::getSpecialLocation(juce::File::tempDirectory).getNonexistentChildFile("InfinityLushBank","",false);folder.createDirectory();auto p=std::make_unique<SpaceInfinityProcessor>(folder);auto& bank=p->presetBank();const auto before=bank.entries().size();int imported=0;need(bank.importBank(archive,imported).wasOk()&&imported==20&&bank.entries().size()==before+20,"20-lead import");juce::StringArray names;for(size_t i=before;i<bank.entries().size();++i){need(p->loadBankPreset(int(i)).wasOk(),"lead bank native recall");const auto& row=bank.entries()[i];need(row.creator=="Orbital Audio"&&!names.contains(row.name),"lead creator or duplicate name");names.add(row.name);need(p->state.getRawParameterValue("source")->load()==0&&p->state.getRawParameterValue("attack")->load()<=.003f,"new leads have synth/fast attack");need(p->state.getRawParameterValue("soundLife")->load()>0,"new leads include voice character");}
 need(bank.importBank(archive,imported).wasOk()&&imported==0&&bank.entries().size()==before+20,"reimport duplicates lead bank");p.reset();folder.deleteRecursively();std::cout<<"20-lead ZIP native import, metadata, character recall and repeat-import passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
