#include "../Source/PluginProcessor.h"
#include "../Source/ExtendedPanel.h"
#include <iostream>
#include <cmath>
#include <stdexcept>
int runExtendedSmoke(){
    juce::ScopedJuceInitialiser_GUI gui;
    auto synthInstance=std::make_unique<SpaceInfinityProcessor>();auto& synth=*synthInstance;
    constexpr int block=256;juce::AudioBuffer<float> audio(2,block);
    auto set=[&](const juce::String& id,float value){auto* p=synth.state.getParameter(id);if(!p)throw std::runtime_error(id.toStdString());p->setValueNotifyingHost(p->convertTo0to1(value));};
    auto render=[&](int blocks){double energy=0;for(int n=0;n<blocks;++n){juce::MidiBuffer midi;if(n==0)midi.addEvent(juce::MidiMessage::noteOn(1,60,.8f),0);if(n==blocks-5)midi.addEvent(juce::MidiMessage::noteOff(1,60),0);synth.processBlock(audio,midi);for(int c=0;c<2;++c)for(int i=0;i<block;++i){const float v=audio.getSample(c,i);if(!std::isfinite(v)||std::abs(v)>1)throw std::runtime_error("invalid output");energy+=v*v;}}return energy;};
    auto makeWav=[](const juce::File& file,const juce::AudioBuffer<float>& buffer,double rate){
        auto stream=file.createOutputStream();if(!stream)return false;juce::WavAudioFormat format;
        std::unique_ptr<juce::AudioFormatWriter> writer(format.createWriterFor(stream.get(),rate,juce::uint32(buffer.getNumChannels()),24,{},0));
        if(!writer)return false;stream.release();return writer->writeFromAudioSampleBuffer(buffer,0,buffer.getNumSamples());
    };
    const auto directory=juce::File::getSpecialLocation(juce::File::tempDirectory).getNonexistentChildFile("SpaceInfinityExtended",{},false);directory.createDirectory();
    synth.loadPreset(0);synth.prepareToPlay(48000,block);
    set("chorus",0);set("delay",0);set("reverb",0);set("cutoff",200);set("filterEnvAmount",0);
    const double baseline=render(40);synth.prepareToPlay(48000,block);set("filterEnvAmount",5);set("filterSustain",1);const double envelopeEnergy=render(40);
    if(baseline<=0||std::abs(envelopeEnergy-baseline)<.01)return 50;
    juce::AudioBuffer<float> sample(1,48000);for(int i=0;i<48000;++i)sample.setSample(0,i,float(.7*std::sin(2*infinity::pi*i/96)));
    const auto wav=directory.getChildFile("source.wav");if(!makeWav(wav,sample,48000)||synth.importSample(wav).failed())return 51;
    set("source",4);set("filterEnvAmount",0);set("cutoff",8000);synth.prepareToPlay(48000,block);if(render(80)<.01)return 52;
    juce::AudioBuffer<float> bank(1,4096);for(int i=0;i<2048;++i){bank.setSample(0,i,float(std::sin(2*infinity::pi*i/2048)));bank.setSample(0,i+2048,2*float(i)/2048-1);}
    const auto table=directory.getChildFile("table.wav");if(!makeWav(table,bank,44100)||synth.importWavetable(table,2048).failed()||synth.tableFrameCount()!=2)return 53;
    auto edited=synth.tableFrame(0);edited[30]=-.6f;synth.editTableFrame(0,edited);const auto expected=synth.tableFrame(0);
    set("source",0);set("wave",30);set("osc2Enabled",0);set("mix",0);set("wavetableScan",.4f);synth.prepareToPlay(48000,block);if(render(40)<.01)return 54;
    const auto exported=directory.getChildFile("export.wav");if(synth.exportWavetable(exported).failed())return 55;
    synth.addTableFrame(0);if(synth.tableFrameCount()!=3)return 56;synth.removeTableFrame(2);if(synth.tableFrameCount()!=2)return 57;
    juce::AudioBuffer<float> ir(2,2400);ir.clear();ir.setSample(0,0,.8f);ir.setSample(1,0,.8f);ir.setSample(0,1200,.4f);ir.setSample(1,1800,.3f);
    const auto impulse=directory.getChildFile("impulse.wav");if(!makeWav(impulse,ir,48000)||synth.importImpulse(impulse).failed())return 58;
    juce::MemoryBlock state;synth.getStateInformation(state);auto restoredInstance=std::make_unique<SpaceInfinityProcessor>();auto& restored=*restoredInstance;restored.setStateInformation(state.getData(),int(state.getSize()));
    if(restored.tableFrameCount()!=2||std::abs(restored.tableFrame(0)[30]-expected[30])>.00001f||restored.state.getRawParameterValue("convolverEnabled")->load()!=1)return 59;
    synth.prepareToPlay(48000,block);
    // Allow the background FFT engine to install the decoded test impulse.
    for(int i=0;i<8;++i){juce::MidiBuffer empty;synth.processBlock(audio,empty);juce::Thread::sleep(10);}
    set("convolverMix",0);synth.prepareToPlay(48000,block);const double dryConvolution=render(80);
    set("convolverMix",1);synth.prepareToPlay(48000,block);const double wetConvolution=render(80);
    if(wetConvolution<.01||std::abs(wetConvolution-dryConvolution)<.01)return 60;
    synth.prepareToPlay(48000,block);set("patternMode",1);set("patternRate",3);int on=0,off=0;
    for(int b=0;b<160;++b){juce::MidiBuffer midi;if(b==0){midi.addEvent(juce::MidiMessage::noteOn(1,60,.7f),0);midi.addEvent(juce::MidiMessage::noteOn(1,64,.7f),0);}if(b==130)midi.addEvent(juce::MidiMessage::allSoundOff(1),0);synth.processBlock(audio,midi);for(const auto m:synth.generatedNotes()){on+=m.getMessage().isNoteOn()?1:0;off+=m.getMessage().isNoteOff()?1:0;}}
    if(on<5||on!=off)return 61;
    std::unique_ptr<juce::AudioProcessorEditor> editor(synth.createEditor());
    for(int oscillator=1;oscillator<=2;++oscillator){
        juce::TextButton* next=nullptr;juce::TextButton* previous=nullptr;
        for(auto* child:editor->getChildren())if(auto* button=dynamic_cast<juce::TextButton*>(child)){
            if(button->getName()=="Osc "+juce::String(oscillator)+" next waveform")next=button;
            if(button->getName()=="Osc "+juce::String(oscillator)+" previous waveform")previous=button;
        }
        if(!next||!previous)return 62;const auto id=oscillator==1?"wave":"osc2Wave";set(id,0);previous->onClick();
        const int last=oscillator==1?30:31;if(int(synth.state.getRawParameterValue(id)->load())!=last)return 63;
        next->onClick();if(synth.state.getRawParameterValue(id)->load()!=0)return 64;
    }
    ExtendedPanel panel(synth);panel.setSize(730,465);
    const auto audition=juce::File::getCurrentWorkingDirectory().getChildFile("audition");audition.createDirectory();
    juce::PNGImageFormat png;
    for(auto* child:panel.getChildren())if(auto* tabs=dynamic_cast<juce::TabbedComponent*>(child))for(int tab=0;tab<tabs->getNumTabs();++tab){
        tabs->setCurrentTabIndex(tab);auto snapshot=panel.createComponentSnapshot(panel.getLocalBounds());
        auto image=audition.getChildFile("Extended-"+juce::String(tab+1)+".png").createOutputStream();if(image)png.writeImageToStream(snapshot,*image);
    }
    directory.deleteRecursively();std::cout<<"Extended engine audio, assets/state, arp MIDI and oscillator navigation passed\n";return 0;
}
