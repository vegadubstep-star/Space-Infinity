#include "../Source/PluginProcessor.h"
#include "../Source/StudioRack.h"
#include <iostream>
#include <complex>
#include <stdexcept>
namespace {
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
bool screenshot(juce::Component& c,const juce::String& name){
    const auto folder=juce::File::getCurrentWorkingDirectory().getChildFile("audition");folder.createDirectory();const auto file=folder.getChildFile(name+".png");file.deleteFile();auto stream=file.createOutputStream();return stream&&juce::PNGImageFormat{}.writeImageToStream(c.createComponentSnapshot(c.getLocalBounds()),*stream);
}
juce::Component* named(juce::Component& c,const juce::String& name){if(c.getName()==name)return &c;for(auto* child:c.getChildren())if(auto* found=named(*child,name))return found;return nullptr;}
}
int runStudioSmoke(){
 try{
    // Dynamics: loud signals reduce, quiet signals lift, channels remain linked.
    auto compressor=std::make_unique<studio::Compressor>();compressor->prepare(48000);studio::Compressor::Settings c;c.mode=1;c.threshold=-24;c.ratio=4;c.attack=.1f;c.release=30;
    compressor->configure(c,256);float loud=0;
    for(int i=0;i<12000;++i){auto v=compressor->process(.8f,.8f);require(v[0]==v[1],"compressor stereo link");loud=v[0];}
    require(loud<.3f&&loud>0,"downward compression");
    compressor->prepare(48000);c.mode=2;c.below=2;c.threshold=-24;compressor->configure(c,256);double energy=0;
    for(int i=0;i<16000;++i){const float in=.005f*std::sin(float(i)*.07f);auto out=compressor->process(in,in);if(i>8000)energy+=out[0]*out[0];}
    require(energy>8000*.005*.005*.7,"upward compression");
    // Delay timings are measured from actual impulse responses after smoothing settles.
    for(int mode=1;mode<=3;++mode){auto echo=std::make_unique<studio::Delay>();echo->prepare(48000);studio::Delay::Settings d;d.mode=mode;d.left=.01f;d.right=.02f;d.feedback=.5f;d.bandwidth=1;echo->configure(d);
        for(int i=0;i<5000;++i)echo->process(0,0);
        int firstL=-1,firstR=-1;double total=0;
        for(int i=0;i<4500;++i){auto v=echo->process(i==0?1.0f:0,0);for(float sample:v)require(std::isfinite(sample),"delay finite output");if(firstL<0&&std::abs(v[0])>.00001)firstL=i;if(firstR<0&&std::abs(v[1])>.00001)firstR=i;total+=std::abs(v[0])+std::abs(v[1]);}
        require(firstL>=477&&firstL<=482,"left delay timing");require(total>0,"delay impulse response");
        if(mode==1)require(firstR<0,"normal delay channel independence");else require(firstR>=1435&&firstR<=1445,"ping-pong / tap routing");
    }
    // Bass-only processing must preserve the high-frequency part of a signal.
    auto bassEnergy=[](float frequency){studio::Rhythm fx;fx.prepare(48000);studio::Rhythm::Settings s;s.gate=true;s.depth=1;s.split=1;s.low=180;s.high=2600;s.steps.fill(0);fx.configure(s,120);double e=0;
        for(int i=0;i<48000;++i){const float v=.2f*std::sin(juce::MathConstants<float>::twoPi*frequency*i/48000);auto out=fx.process(v,v,0);if(i>24000)e+=out[0]*out[0];}return e;};
    require(bassEnergy(50)<10,"bass-only gate attenuates low band");require(bassEnergy(5000)>350,"bass-only gate preserves highs");
    for(size_t i=0;i<studio::gatePatterns.size();++i)for(size_t j=0;j<i;++j)require(studio::gatePatterns[i]!=studio::gatePatterns[j],"15 unique gate patterns");
    studio::Rhythm sc;sc.prepare(48000);studio::Rhythm::Settings settings;settings.pump=true;settings.sidechain=1;settings.pumpDepth=1;settings.threshold=-30;sc.configure(settings,120);float duck=1;
    for(int i=0;i<12000;++i)duck=sc.process(.5f,.5f,1)[0];require(duck<.1f,"external sidechain detection");
    auto externalEnergy=[](float trigger){
        auto synth=std::make_unique<SpaceInfinityProcessor>();auto layout=synth->getBusesLayout();layout.inputBuses.set(0,juce::AudioChannelSet::stereo());require(synth->setBusesLayout(layout),"external bus layout");
        synth->loadPreset(12);auto set=[&](const char* id,float v){auto* p=synth->state.getParameter(id);p->setValueNotifyingHost(p->convertTo0to1(v));};
        for(const char* id:{"compEnabled","delayEnabled","chorusEnabled","reverbEnabled","convolverEnabled","eqEnabled","gateEnabled"})set(id,0);
        set("pumpEnabled",1);set("sidechainSource",1);set("gateSplit",0);set("pumpDepth",1);set("sidechainThreshold",-30);set("sidechainRatio",8);
        synth->prepareToPlay(48000,256);juce::AudioBuffer<float> audio(2,256);double energy=0;
        for(int block=0;block<80;++block){for(int ch=0;ch<2;++ch)for(int i=0;i<256;++i)audio.setSample(ch,i,trigger);
            juce::MidiBuffer notes;if(block==0)notes.addEvent(juce::MidiMessage::noteOn(1,60,.8f),0);synth->processBlock(audio,notes);
            if(block>40)for(int i=0;i<256;++i)energy+=audio.getSample(0,i)*audio.getSample(0,i);}
        return energy;
    };
    require(externalEnergy(.8f)<externalEnergy(0)*.15,"external sidechain bus affects real synth audio");
    // Each high-pass slope is steeper at a fixed point below cutoff.
    double previous=2;
    for(int stages=1;stages<=4;++stages){double magnitude=1;auto z=std::polar(1.0,-juce::MathConstants<double>::twoPi*100/48000);
        for(int stage=0;stage<stages;++stage){const auto co=studio::eqCoefficients(48000,1000,1,0,4,stages,stage);magnitude*=std::abs((double(co[0])+double(co[1])*z+double(co[2])*z*z)/(double(co[3])+double(co[4])*z+double(co[5])*z*z));}
        require(magnitude<previous*.25,"EQ roll-off slopes");previous=magnitude;}
    // Minimum phase moves delayed energy forward and retains magnitude response.
    juce::AudioBuffer<float> impulse(1,512);impulse.clear();impulse.setSample(0,100,1);impulse.setSample(0,180,.3f);studio::minimumPhase(impulse);
    require(std::abs(impulse.getSample(0,0))>.8f,"minimum-phase energy position");
    for(int bin:{1,7,40,100}){std::complex<double> actual{},original{};for(int i=0;i<512;++i){const auto z=std::polar(1.0,-juce::MathConstants<double>::twoPi*bin*i/1024);actual+=double(impulse.getSample(0,i))*z;if(i==100)original+=z;if(i==180)original+=.3*z;}
        require(std::abs(std::abs(actual)-std::abs(original))<.01,"minimum-phase magnitude preservation");}
    auto synth=std::make_unique<SpaceInfinityProcessor>();synth->loadPreset(12);synth->prepareToPlay(48000,256);
    auto set=[&](const char* id,float v){auto* p=synth->state.getParameter(id);require(p!=nullptr,id);p->setValueNotifyingHost(p->convertTo0to1(v));};
    // A real imported waveform, trim/reverse, audio history and embedded state.
    const auto directory=juce::File::getSpecialLocation(juce::File::tempDirectory).getNonexistentChildFile("InfinityStudio",{},false);directory.createDirectory();
    const auto file=directory.getChildFile("Waveform-Test.wav");juce::AudioBuffer<float> wave(1,48000);
    for(int i=0;i<48000;++i)wave.setSample(0,i,.7f*std::sin(juce::MathConstants<float>::twoPi*440*i/48000)*(1-float(i)/60000));
    {auto stream=file.createOutputStream();juce::WavAudioFormat format;std::unique_ptr<juce::AudioFormatWriter> writer(format.createWriterFor(stream.get(),48000,1,24,{},0));require(writer!=nullptr,"fixture WAV");stream.release();require(writer->writeFromAudioSampleBuffer(wave,0,48000),"write fixture WAV");}
    require(synth->importSample(file).wasOk(),"sample import");SpaceInfinityProcessor::SampleView view;require(synth->sampleView(3,view)&&view.samples==48000&&view.high[0]>.4f,"actual sample envelope");
    set("sampleLoop",1);set("sampleStart",.15f);set("sampleEnd",.8f);set("sampleReverse",1);set("compMode",2);set("compThreshold",-35);set("compMix",.6f);set("echoMode",2);set("echoMix",.18f);
    juce::AudioBuffer<float> audio(2,256);
    for(int block=0;block<70;++block){juce::MidiBuffer notes;if(block==0)notes.addEvent(juce::MidiMessage::noteOn(1,60,.8f),0);synth->processBlock(audio,notes);}
    const auto history=synth->outputScope();float peak=0;for(float v:history)peak=juce::jmax(peak,std::abs(v));require(peak>.001f,"live processed waveform");
    require(synth->samplePlayhead()>=.15f&&synth->samplePlayhead()<=.8f,"sample trim / reverse playhead");
    const float before=synth->samplePlayhead();set("sampleReverse",0);
    for(int i=0;i<4;++i){juce::MidiBuffer notes;synth->processBlock(audio,notes);}
    require(synth->samplePlayhead()>before,"reverse changes a held sampled voice");set("sampleReverse",1);
    for(int i=0;i<4;++i){juce::MidiBuffer notes;synth->processBlock(audio,notes);}
    for(int preset=0;preset<15;++preset){synth->loadGatePreset(preset);for(int i=0;i<16;++i)require(std::abs(synth->state.getRawParameterValue("gateStep"+juce::String(i))->load()-studio::gatePatterns[size_t(preset)][size_t(i)])<.0001,"gate preset recall");}
    set("irPreset",0);set("irSize",.5f);set("irMinimumPhase",1);set("irAttack",2);set("irDecay",.4f);set("irReverse",1);set("irPredelay",30);
    require(synth->importImpulse(file).wasOk(),"convolution import / transform");const auto ir=synth->impulseView();float irPeak=0;for(float v:ir)irPeak=juce::jmax(irPeak,v);require(irPeak>0,"processed impulse waveform");
    juce::MemoryBlock state;synth->getStateInformation(state);auto other=std::make_unique<SpaceInfinityProcessor>();other->setStateInformation(state.getData(),int(state.getSize()));
    for(const char* id:{"compMode","compThreshold","echoMode","echoMix","irSize","irMinimumPhase","irReverse","sampleStart","sampleEnd","sampleReverse"})require(std::abs(synth->state.getRawParameterValue(id)->load()-other->state.getRawParameterValue(id)->load())<.001,"studio state recall");
    auto editor=std::unique_ptr<juce::AudioProcessorEditor>(synth->createEditor());editor->setSize(1180,820);require(screenshot(*editor,"Studio-Sampler-Main"),"sampler screenshot");
    juce::TextButton* fx=nullptr;for(auto* child:editor->getChildren())if(auto* b=dynamic_cast<juce::TextButton*>(child))if(b->getButtonText()=="FX")fx=b;require(fx!=nullptr,"FX tab");fx->onClick();
    StudioRack* rack=nullptr;for(auto* child:editor->getChildren())if(auto* r=dynamic_cast<StudioRack*>(child))rack=r;require(rack!=nullptr,"FX rack");
    for(int i=0;i<8;++i){auto* m=rack->module(i);require(m!=nullptr,"FX module");require(screenshot(*m,"Studio-FX-"+juce::String(i+1)),"FX screenshot");}
    require(screenshot(*editor,"Studio-FX-Rack"),"rack screenshot");
    juce::TextButton* eqButton=nullptr;
    for(auto* child:editor->getChildren())if(auto* button=dynamic_cast<juce::TextButton*>(child))if(button->getButtonText()=="OPEN 7-BAND EQ")eqButton=button;
    require(eqButton!=nullptr,"EQ button");eqButton->onClick();
    auto* eq=named(*editor,"Seven-band EQ");require(eq!=nullptr,"EQ popup");require(screenshot(*eq,"Studio-EQ"),"EQ screenshot");
    juce::Component::SafePointer<juce::Component> firstPopup(eq);
    eqButton->onClick();require(firstPopup==nullptr,"reopening releases the previous popup");
    juce::Component::SafePointer<juce::Component> lastPopup(named(*editor,"Seven-band EQ"));require(lastPopup!=nullptr,"replacement EQ popup");
    std::cout<<"Studio screenshots complete; checking editor/popup teardown"<<std::endl;
    editor.reset();require(lastPopup==nullptr,"editor owns and destroys its popup");
    std::cout<<"Editor and popup released; closing processor instances"<<std::endl;
    other.reset();synth.reset();directory.deleteRecursively();
    std::cout<<"Studio dynamics, delay modes/timing, frequency-selective gate, 15 patterns, external detector, EQ slopes, minimum phase, sampler/IR waveforms and embedded state passed\n";
    return 0;
 }catch(const std::exception& e){std::cerr<<"Studio check failed: "<<e.what()<<'\n';return 120;}
}
