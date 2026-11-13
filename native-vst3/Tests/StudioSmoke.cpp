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
    for(size_t i=0;i<studio::gatePatterns.size();++i)for(size_t j=0;j<i;++j)require(studio::gatePatterns[i]!=studio::gatePatterns[j]||studio::gatePresetRates[i]!=studio::gatePresetRates[j],"15 unique gate patterns and timings");
    studio::Rhythm sc;sc.prepare(48000);studio::Rhythm::Settings settings;settings.pump=true;settings.sidechain=1;settings.pumpDepth=1;settings.threshold=-30;sc.configure(settings,120);float duck=1;
    for(int i=0;i<12000;++i)duck=sc.process(.5f,.5f,1)[0];require(duck<.1f,"external sidechain detection");
    auto externalEnergy=[](float trigger){
        auto synth=std::make_unique<SpaceInfinityProcessor>();auto layout=synth->getBusesLayout();layout.inputBuses.set(0,juce::AudioChannelSet::stereo());require(synth->setBusesLayout(layout),"external bus layout");
        synth->loadPreset(0);auto set=[&](const char* id,float v){auto* p=synth->state.getParameter(id);p->setValueNotifyingHost(p->convertTo0to1(v));};
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
    auto synth=std::make_unique<SpaceInfinityProcessor>();synth->loadPreset(0);synth->prepareToPlay(48000,256);
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
    juce::MemoryBlock state;synth->getStateInformation(state);const auto exportFile=directory.getChildFile("Portable Sound.sipreset");require(synth->exportPreset(exportFile).wasOk(),"portable preset export");juce::MemoryBlock portable;require(exportFile.loadFileAsData(portable),"portable preset read");const auto portableXml=juce::AudioProcessor::getXmlFromBinary(portable.getData(),int(portable.getSize()));require(portableXml!=nullptr&&portableXml->getStringAttribute("sampleWav").isNotEmpty()&&portableXml->getStringAttribute("impulseWav").isNotEmpty()&&portableXml->getStringAttribute("tableData").isNotEmpty(),"portable sample, impulse and wavetable assets");auto other=std::make_unique<SpaceInfinityProcessor>();other->setStateInformation(portable.getData(),int(portable.getSize()));
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

// Sustained and retriggered pad output must not accumulate a loss of level.
int runLeadSmoke(){
 try{
    const auto capture=[](int oscillator,float ownSpread,float otherSpread,float blend,int generation){
        auto p=std::make_unique<SpaceInfinityProcessor>();p->loadPreset(0);
        const auto set=[&](const juce::String& id,float v){auto* parameter=p->state.getParameter(id);parameter->setValueNotifyingHost(parameter->convertTo0to1(v));};
        set("mix",oscillator==1?0:1);set("osc1Unison",7);set("osc2Unison",7);set("detune",.02f);set("attack",0);
        set("manualUnison",generation==2?1:0);set("modernUnison",generation!=0?1:0);const auto field=generation!=0?"Detune":"Spread";const float scale=generation!=0?.5f:1;set("osc"+juce::String(oscillator)+field,ownSpread*scale);set("osc"+juce::String(3-oscillator)+field,otherSpread*scale);set("osc"+juce::String(oscillator)+"Blend",blend);
        p->prepareToPlay(48000,256);juce::AudioBuffer<float> audio(2,256);std::vector<float> samples;
        for(int b=0;b<64;++b){juce::MidiBuffer midi;if(b==0)midi.addEvent(juce::MidiMessage::noteOn(1,69,.8f),0);p->processBlock(audio,midi);for(int i=0;i<256;++i)samples.push_back(audio.getSample(0,i));}return samples;
    };
    const auto difference=[](const auto& a,const auto& b){double sum=0;for(size_t i=0;i<a.size();++i)sum+=std::abs(a[i]-b[i]);return sum;};
    for(int generation:{0,1,2})for(int oscillator:{1,2}){const auto base=capture(oscillator,1,1,1,generation);require(difference(base,capture(oscillator,1,0,1,generation))<.00001,"other oscillator detune/spread stays independent");require(difference(base,capture(oscillator,0,1,1,generation))>1,"own detune/spread changes real output");require(difference(base,capture(oscillator,1,1,0,generation))>1,"unison blend changes real output");}
    // Measure real output pitch during an overlapping octave change.
    auto glideSynth=std::make_unique<SpaceInfinityProcessor>();glideSynth->loadPreset(0);
    const auto set=[&](const char* id,float v){auto* p=glideSynth->state.getParameter(id);p->setValueNotifyingHost(p->convertTo0to1(v));};
    set("wave",2);set("osc2Enabled",0);set("mix",0);set("attack",0);set("voiceMode",2);set("glideTime",.12f);set("cutoff",16000);
    glideSynth->prepareToPlay(48000,256);juce::AudioBuffer<float> probe(2,256);float previous=0;
    const auto crossings=[&](int blocks,int note){int count=0;for(int b=0;b<blocks;++b){juce::MidiBuffer midi;if(b==0&&note>=0)midi.addEvent(juce::MidiMessage::noteOn(1,note,.8f),0);glideSynth->processBlock(probe,midi);for(int i=0;i<256;++i){const float v=probe.getSample(0,i);if(previous<=0&&v>0)++count;previous=v;}}return count*48000.0/(blocks*256);};
    crossings(40,69);const double earlyPitch=crossings(4,81);crossings(110,-1);const double latePitch=crossings(20,-1);
    require(earlyPitch>400&&earlyPitch<700&&latePitch>820&&latePitch<930,"legato glide reaches target gradually");
    set("voiceMode",1);set("glideTime",.12f);glideSynth->prepareToPlay(48000,256);previous=0;crossings(40,69);const double monoEarly=crossings(4,81);crossings(110,-1);const double monoLate=crossings(20,-1);require(monoEarly>400&&monoEarly<700&&monoLate>820&&monoLate<930,"mono portamento retriggers envelope while gliding pitch");
    set("glideTime",0);set("voiceMode",0);set("pitchEnvAmount",12);set("pitchEnvAttack",0);set("pitchEnvDecay",.1f);glideSynth->prepareToPlay(48000,256);previous=0;
    const double pitchStart=crossings(4,69);crossings(40,-1);const double pitchEnd=crossings(20,-1);
    require(pitchStart>650&&pitchEnd>400&&pitchEnd<480,"pitch envelope returns to played pitch");
    set("pitchEnvAmount",0);set("midiBendRange",2);glideSynth->prepareToPlay(48000,256);previous=0;crossings(40,69);
    juce::MidiBuffer bend;bend.addEvent(juce::MidiMessage::pitchWheel(1,16383),0);glideSynth->processBlock(probe,bend);crossings(10,-1);const double bentPitch=crossings(30,-1);
    require(bentPitch>480&&bentPitch<510,"MIDI bend range gives two semitones");
    set("pitchBend",12);glideSynth->prepareToPlay(48000,256);previous=0;crossings(40,69);const double automatedPitch=crossings(30,-1);require(automatedPitch>850&&automatedPitch<920,"host pitch wheel parameter automates octave riser");set("pitchBend",0);
    set("osc1Enabled",0);set("osc2Enabled",1);set("mix",1);set("osc2Octave",2);set("osc2Semitone",7);set("detune",0);glideSynth->prepareToPlay(48000,256);previous=0;crossings(40,69);const double coarsePitch=crossings(30,-1);
    require(coarsePitch>2600&&coarsePitch<2675,"octave plus semitone gives the 31-semitone interval");
    set("osc1Enabled",1);set("osc2Enabled",0);set("mix",0);set("osc2Octave",0);set("osc2Semitone",0);
    set(glideSynth->lfoParameterId(0,"Sync").toRawUTF8(),0);set(glideSynth->lfoParameterId(0,"Speed").toRawUTF8(),2);set(glideSynth->modulationRouteId(4,0,false).toRawUTF8(),25);set(glideSynth->modulationRouteId(4,0,true).toRawUTF8(),1);
    for(int point=0;point<17;++point)set(glideSynth->lfoPointId(0,point).toRawUTF8(),.5f+.5f*std::sin(float(point)*juce::MathConstants<float>::twoPi/16));
    glideSynth->prepareToPlay(48000,256);previous=0;const double pitchUp=crossings(23,69);crossings(23,-1);const double pitchDown=crossings(23,-1);
    require(pitchUp>465&&pitchDown<420,"LFO pitch modulation moves above and below the played note");
    std::cout<<"MIDI wheel / coarse tuning / bipolar pitch: "<<bentPitch<<" / "<<coarsePitch<<" / "<<pitchUp<<" -> "<<pitchDown<<std::endl;
    std::cout<<"Glide output pitch: "<<earlyPitch<<" -> "<<latePitch<<" Hz"<<std::endl;
    std::cout<<"Replacement leads: repeated output, headroom, previews and recall passed"<<std::endl;return 0;
 }catch(const std::exception& e){std::cerr<<"Lead test: "<<e.what()<<std::endl;return 140;}
}
