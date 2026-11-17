#include "../Source/PluginEditor.h"
#include <iostream>
#include <stdexcept>
#include <vector>
namespace {
void need(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
void set(SpaceInfinityProcessor& p,const juce::String& id,float v){auto* a=p.state.getParameter(id);need(a!=nullptr,"noise parameter exists");a->setValueNotifyingHost(a->convertTo0to1(v));}
juce::Component* named(juce::Component& c,const juce::String& name){if(c.getName()==name)return &c;for(auto* child:c.getChildren())if(auto* found=named(*child,name))return found;return nullptr;}
std::vector<float> burst(int mode,int type,int idle,bool repeat=false){
 auto p=std::make_unique<SpaceInfinityProcessor>();p->loadPreset(0);
 for(auto id:{"osc1Enabled","osc2Enabled","subEnabled","routeNoise","compEnabled","delayEnabled","reverbEnabled","chorusEnabled","flangerEnabled","phaserEnabled","gateEnabled","pumpEnabled","eqEnabled","convolverEnabled","distEnabled","analogueUpgrade","declickEnabled"})set(*p,id,0);
 set(*p,"engineMode",1);set(*p,"analogueCharacter",0);set(*p,"noiseLevel",.6f);set(*p,"noiseMode",float(mode));set(*p,"noiseType",float(type));set(*p,"gain",.2f);set(*p,"attack",0);set(*p,"ampSustain",1);set(*p,"release",.002f);set(*p,"subLevel",0);set(*p,"voiceMode",1);
 p->prepareToPlay(48000,128);juce::AudioBuffer<float> audio(2,128);std::vector<float> result;
 const int second=idle+432;for(int block=0;block<(repeat?second+32:idle+32);++block){juce::MidiBuffer midi;if(block==idle||block==second)midi.addEvent(juce::MidiMessage::noteOn(1,60,.8f),0);if(block==idle+32)midi.addEvent(juce::MidiMessage::noteOff(1,60),0);p->processBlock(audio,midi);if((block>=idle&&block<idle+32)||(repeat&&block>=second))for(int i=0;i<128;++i){const float x=audio.getSample(0,i);need(std::isfinite(x),"noise finite");result.push_back(x);}}
 return result;
}
double difference(const std::vector<float>& a,const std::vector<float>& b){double e=0;for(size_t n=0;n<a.size();++n)e+=std::abs(a[n]-b[n]);return e/a.size();}
}
int runNoiseModeSmoke(){try{
 for(int type=0;type<3;++type){const auto twice=burst(2,type,400,true);need(difference(std::vector<float>(twice.begin(),twice.begin()+4096),std::vector<float>(twice.begin()+4096,twice.end()))<1e-6,"successive notes restart the same noise burst");const auto retrigger=burst(2,type,400),retrigLater=burst(2,type,800),free=burst(1,type,400),freeLater=burst(1,type,800);double energy=0;for(float x:retrigger)energy+=x*x;need(energy>.01,"isolated noise audible");need(difference(retrigger,retrigLater)<1e-6,"Retrigger repeats identical colour burst after different idle gaps");need(difference(free,freeLater)>.001,"Free stream continues advancing through idle gaps");}
 auto p=std::make_unique<SpaceInfinityProcessor>();p->loadPreset(0);need(p->state.getRawParameterValue("noiseMode")->load()==1,"fresh Init uses Free noise");set(*p,"noiseMode",2);juce::MemoryBlock saved;p->getStateInformation(saved);auto recalled=std::make_unique<SpaceInfinityProcessor>();recalled->setStateInformation(saved.getData(),int(saved.getSize()));need(recalled->state.getRawParameterValue("noiseMode")->load()==2,"noise mode saved and recalled");
 auto legacy=p->state.copyState();for(int n=legacy.getNumChildren();--n>=0;)if(legacy.getChild(n).getProperty("id").toString()=="noiseMode")legacy.removeChild(n,nullptr);juce::MemoryBlock old;juce::AudioProcessor::copyXmlToBinary(*legacy.createXml(),old);recalled->setStateInformation(old.getData(),int(old.getSize()));need(recalled->state.getRawParameterValue("noiseMode")->load()==0,"omitted legacy mode recalls Random per note");
 auto editor=std::unique_ptr<juce::AudioProcessorEditor>(p->createEditor());editor->setSize(800,760);auto* control=dynamic_cast<juce::ComboBox*>(named(*editor,"Noise playback mode"));need(control&&control->isVisible()&&control->getNumItems()==3,"noise modes accessible at smallest size");need(editor->getLocalBounds().contains(editor->getLocalArea(control,control->getLocalBounds())),"noise control inside main window");
 std::cout<<"Noise: repeatable White/Dark/Bright bursts, advancing Free stream, legacy recall and compact UI passed\n";return 0;
}catch(const std::exception& e){std::cerr<<"Noise modes: "<<e.what()<<'\n';return 181;}}
