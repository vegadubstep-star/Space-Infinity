#include "../Source/PluginProcessor.h"
#include <cmath>
#include <iostream>
#include <fstream>

int main()
{
    juce::ScopedJuceInitialiser_GUI gui;
    SpaceInfinityProcessor synth;
    constexpr int blockSize=256;
    juce::AudioBuffer<float> audio(2,blockSize);
    const auto& names=synth.presetNames();
    if(names.size()!=104 || synth.motionNames().size()!=20) return 1;

    for(int i=0;i<names.size();++i)
        if(names.indexOf(names[i])!=i) return 9;
    std::unique_ptr<juce::AudioProcessorEditor> editor(synth.createEditor());
    if(!editor) return 6;
    juce::ComboBox* factoryMenu=nullptr;
    for(auto* child:editor->getChildren()){
        auto* box=dynamic_cast<juce::ComboBox*>(child);
        if(box && box->getTextWhenNothingSelected()=="Select a sound") factoryMenu=box;
    }
    if(!factoryMenu) return 10;
    juce::TextButton* previous=nullptr;
    juce::TextButton* next=nullptr;
    for(auto* child:editor->getChildren()){
        if(child->getName()=="Previous preset") previous=dynamic_cast<juce::TextButton*>(child);
        if(child->getName()=="Next preset") next=dynamic_cast<juce::TextButton*>(child);
    }
    if(!previous || !next) return 22;
    factoryMenu->setSelectedId(names.size(),juce::dontSendNotification);
    next->onClick();
    if(factoryMenu->getSelectedId()!=1) return 23;
    previous->onClick();
    if(factoryMenu->getSelectedId()!=names.size()) return 24;
    auto* cutoff=synth.state.getParameter("cutoff");

    const auto audition=juce::File::getCurrentWorkingDirectory().getChildFile("audition");
    if(audition.createDirectory().failed()) return 17;
    std::ofstream levels(audition.getChildFile("levels.tsv").getFullPathName().toStdString());
    if(!levels) return 18;
    levels << "Preset\tPeak dBFS\tRMS dBFS\n";
    for(const auto* id:{"attack","release"}){
        auto* parameter=synth.state.getParameter(id);
        if(parameter->convertFrom0to1(0.0f)!=0.0f || parameter->convertFrom0to1(1.0f)!=1.5f) return 26;
    }
    float quietest=1.0f, loudest=0.0f;
    for(int index=0;index<names.size();++index){
        if(names[index].isEmpty() || synth.presetCategory(index).isEmpty()) return 2;
        synth.loadPreset(index);
        const auto expectedCutoff=cutoff->getValue();
        cutoff->setValueNotifyingHost(expectedCutoff>0.5f?0.0f:1.0f);
        factoryMenu->setSelectedId(index+1,juce::sendNotificationSync);
        if(factoryMenu->getText()!=names[index] || std::abs(cutoff->getValue()-expectedCutoff)>0.001f)
            return 11;
        synth.prepareToPlay(48000.0,blockSize);
        float peak=0.0f;
        double energy=0.0;
        auto wav=audition.getChildFile(juce::String(index+1).paddedLeft('0',3)+" "+names[index].replaceCharacters("/\\:*?\"<>|","_________")+".wav");
        wav.deleteFile();
        auto stream=wav.createOutputStream();
        if(!stream) return 19;
        juce::WavAudioFormat format;
        std::unique_ptr<juce::AudioFormatWriter> writer(format.createWriterFor(stream.get(),48000.0,2,24,{},0));
        if(!writer) return 20;
        stream.release();
        for(int block=0;block<75;++block){
            juce::MidiBuffer midi;
            if(block==0) midi.addEvent(juce::MidiMessage::noteOn(1,48,0.8f),0);
            if(block==60) midi.addEvent(juce::MidiMessage::noteOff(1,48),0);
            synth.processBlock(audio,midi);
            if(!writer->writeFromAudioSampleBuffer(audio,0,blockSize)) return 21;
            for(int channel=0;channel<audio.getNumChannels();++channel)
                for(int i=0;i<blockSize;++i){
                    const float sample=audio.getSample(channel,i);
                    if(!std::isfinite(sample)) return 3;
                    peak=std::max(peak,std::abs(sample));
                    energy+=double(sample)*sample;
                }
        }
        levels << names[index] << '\t' << 20.0*std::log10(double(peak))
               << '\t' << 10.0*std::log10(energy/(75.0*blockSize*2.0)) << '\n';
        std::cout << names[index] << " peak " << peak << '\n';
        if(peak<0.0001f || peak>1.0f) return 4;
        quietest=std::min(quietest,peak);
        loudest=std::max(loudest,peak);
    }
    synth.loadPreset(0);
    synth.prepareToPlay(48000.0,blockSize);
    juce::MidiBuffer chord;
    for(const int note:{48,55,60,63,67,72,75,79})
        chord.addEvent(juce::MidiMessage::noteOn(1,note,0.85f),0);
    synth.processBlock(audio,chord);
    if(audio.getMagnitude(0,blockSize)<=0.0f) return 14;
    juce::MidiBuffer panic;
    panic.addEvent(juce::MidiMessage::allSoundOff(1),0);
    synth.processBlock(audio,panic);
    if(!std::isfinite(audio.getMagnitude(0,blockSize))) return 15;

    auto* point=synth.state.getParameter(synth.motionPointId(3));
    cutoff->setValueNotifyingHost(0.45f);
    point->setValueNotifyingHost(0.73f);
    auto* macro=synth.state.getParameter("macro0");
    auto* route=synth.state.getParameter("macroRoute0");
    macro->setValueNotifyingHost(0.82f);
    route->setValueNotifyingHost(route->convertTo0to1(5.0f));
    juce::MemoryBlock saved;
    synth.getStateInformation(saved);
    synth.loadPreset(0);
    synth.setStateInformation(saved.getData(),int(saved.getSize()));
    if(std::abs(cutoff->getValue()-0.45f)>0.001f || std::abs(point->getValue()-0.73f)>0.001f) return 5;

    if(std::abs(macro->getValue()-0.82f)>0.001f || std::abs(route->convertFrom0to1(route->getValue())-5.0f)>0.01f) return 16;
    for(const auto rate:{44100.0,96000.0}){
        for(const auto frames:{64,512}){
            synth.loadPreset(0);
            synth.prepareToPlay(rate,frames);
            juce::AudioBuffer<float> buffer(2,frames);
            juce::MidiBuffer events;
            events.addEvent(juce::MidiMessage::noteOn(1,60,0.8f),0);
            synth.processBlock(buffer,events);
            const auto peak=buffer.getMagnitude(0,frames);
            if(!std::isfinite(peak)||peak<=0.0f||peak>1.0f) return 12;
        }
    }
    for(int shape=0;shape<20;++shape){
        synth.loadMotionShape(shape);
        for(int i=0;i<17;++i){
            const auto value=synth.state.getParameter(synth.motionPointId(i))->getValue();
            if(!std::isfinite(value)||value<0.0f||value>1.0f) return 13;
        }
    }
    for(const auto size : {juce::Point<int>(800,760),juce::Point<int>(1000,790),juce::Point<int>(1500,950)}){
        editor->setSize(size.x,size.y);
        for(auto* child : editor->getChildren())
            if(child->isVisible() && !editor->getLocalBounds().contains(child->getBounds())) return 7;
        const auto snapshot=editor->createComponentSnapshot(editor->getLocalBounds());
        if(!snapshot.isValid() || snapshot.getWidth()!=size.x || snapshot.getHeight()!=size.y) return 8;
        const auto file=audition.getChildFile("Editor-"+juce::String(size.x)+".png");
        file.deleteFile();
        auto output=file.createOutputStream();
        if(!output || !juce::PNGImageFormat{}.writeImageToStream(snapshot,*output)) return 25;
    }
    juce::TextButton* effectsTab=nullptr;
    for(auto* child:editor->getChildren())
        if(auto* button=dynamic_cast<juce::TextButton*>(child))
            if(button->getButtonText()=="FX")effectsTab=button;
    if(!effectsTab)return 27;
    effectsTab->onClick();
    for(const auto size:{juce::Point<int>(800,760),juce::Point<int>(1000,790)}){
        editor->setSize(size.x,size.y);
        for(auto* child:editor->getChildren())
            if(child->isVisible()&&!editor->getLocalBounds().contains(child->getBounds()))return 28;
        const auto snapshot=editor->createComponentSnapshot(editor->getLocalBounds());
        const auto file=audition.getChildFile("Effects-"+juce::String(size.x)+".png");
        file.deleteFile();
        auto output=file.createOutputStream();
        if(!output||!juce::PNGImageFormat{}.writeImageToStream(snapshot,*output))return 29;
    }
    synth.loadPreset(0);
    for(const auto* id:{"compEnabled","chorusEnabled","flangerEnabled","phaserEnabled","delayEnabled","reverbEnabled"})
        synth.state.getParameter(id)->setValueNotifyingHost(1.0f);
    for(const auto* id:{"compAmount","chorus","flanger","phaser","delay","reverb"}){
        auto* parameter=synth.state.getParameter(id);
        parameter->setValueNotifyingHost(parameter->convertTo0to1(0.4f));
    }
    synth.prepareToPlay(48000.0,blockSize);
    for(int block=0;block<120;++block){
        juce::MidiBuffer midi;
        if(block==0)midi.addEvent(juce::MidiMessage::noteOn(1,60,0.65f),0);
        if(block==80)midi.addEvent(juce::MidiMessage::noteOff(1,60),0);
        synth.processBlock(audio,midi);
        for(int channel=0;channel<2;++channel)
            for(int sample=0;sample<blockSize;++sample)
                if(!std::isfinite(audio.getSample(channel,sample)))return 30;
    }
    synth.loadPreset(0);
    for(const auto* id:{"chorus","flanger","phaser","delay","reverb","compAmount"})
        synth.state.getParameter(id)->setValueNotifyingHost(0.0f);
    const auto set=[&synth](const char* id,float value){
        auto* parameter=synth.state.getParameter(id);
        parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
    };
    set("filterMode",5.0f);
    set("analogueCharacter",1.0f);
    set("osc1Unison",16.0f);
    set("osc2Unison",16.0f);
    set("res",.95f);
    for(const auto rate:{44100.0,48000.0,96000.0}){
        synth.prepareToPlay(rate,blockSize);
        for(int block=0;block<32;++block){
            set("cutoff",80.0f*std::pow(200.0f,float(block)/31.0f));
            juce::MidiBuffer midi;
            if(block==0)for(int note:{48,55,60,64})midi.addEvent(juce::MidiMessage::noteOn(1,note,.65f),0);
            synth.processBlock(audio,midi);
            for(int channel=0;channel<2;++channel)
                for(int sample=0;sample<blockSize;++sample)
                    if(!std::isfinite(audio.getSample(channel,sample)))return 31;
        }
    }
    synth.loadPreset(0);
    const float original= synth.state.getRawParameterValue(synth.lfoPointId(0,4))->load();
    synth.loadLfoShape(1,3);
    if(synth.state.getRawParameterValue(synth.lfoPointId(0,4))->load()!=original)return 32;
    for(int lfo=0;lfo<3;++lfo){
        auto* speed=synth.state.getParameter(synth.lfoParameterId(lfo,"Speed"));
        speed->setValueNotifyingHost(speed->convertTo0to1(20.0f));
        auto* sync=synth.state.getParameter(synth.lfoParameterId(lfo,"Sync"));
        sync->setValueNotifyingHost(0.0f);
        auto* mode=synth.state.getParameter(synth.lfoParameterId(lfo,"Mode"));
        mode->setValueNotifyingHost(mode->convertTo0to1(2.0f));
        auto* route=synth.state.getParameter(synth.modulationRouteId(lfo+4,0,false));
        route->setValueNotifyingHost(route->convertTo0to1(lfo==0?1.0f:lfo==1?10.0f:12.0f));
        auto* depth=synth.state.getParameter(synth.modulationRouteId(lfo+4,0,true));
        depth->setValueNotifyingHost(depth->convertTo0to1(lfo==1?-.4f:.4f));
    }
    auto* ampRoute=synth.state.getParameter(synth.modulationRouteId(7,0,false));
    ampRoute->setValueNotifyingHost(ampRoute->convertTo0to1(1.0f));
    auto* ampDepth=synth.state.getParameter(synth.modulationRouteId(7,0,true));
    ampDepth->setValueNotifyingHost(ampDepth->convertTo0to1(-.25f));
    juce::MemoryBlock modulationState;
    synth.getStateInformation(modulationState);
    synth.loadPreset(0);
    synth.setStateInformation(modulationState.getData(),int(modulationState.getSize()));
    if(std::abs(ampDepth->convertFrom0to1(ampDepth->getValue())+.25f)>.001f)return 33;
    synth.prepareToPlay(48000.0,blockSize);
    for(int block=0;block<32;++block){
        juce::MidiBuffer midi;
        if(block==0)midi.addEvent(juce::MidiMessage::noteOn(1,60,.65f),0);
        synth.processBlock(audio,midi);
        for(int channel=0;channel<2;++channel)
            for(int sample=0;sample<blockSize;++sample)
                if(!std::isfinite(audio.getSample(channel,sample)))return 34;
    }
    for(int lfo=0;lfo<3;++lfo)if(std::abs(synth.lfoDisplayPhase(lfo)-1.0f)>.001f)return 35;
    for(auto* child:editor->getChildren())
        if(auto* button=dynamic_cast<juce::TextButton*>(child))
            if(button->getButtonText()=="MAIN")button->onClick();
    const auto findChild=[&editor](const juce::String& name)->juce::Component*{
        for(auto* child:editor->getChildren())if(child->getName()==name)return child;
        return nullptr;
    };
    auto* energy=findChild("Energy");
    auto* cutoffControl=findChild("CUTOFF");
    if(!energy||!cutoffControl)return 36;
    const auto event=[](juce::Component* component,juce::Point<float> position,juce::Point<float> start,int flags,bool dragged){
        return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),position,juce::ModifierKeys(flags),
            1.0f,0.0f,0.0f,0.0f,0.0f,component,component,juce::Time::getCurrentTime(),
            start,juce::Time::getCurrentTime(),1,dragged);
    };
    const auto start=energy->getBounds().translated(0,-17).withHeight(17).getCentre().toFloat();
    const auto destination=cutoffControl->getBounds().getCentre().toFloat();
    editor->mouseDown(event(editor.get(),start,start,juce::ModifierKeys::leftButtonModifier,false));
    editor->mouseDrag(event(editor.get(),destination,start,juce::ModifierKeys::leftButtonModifier,true));
    editor->mouseUp(event(editor.get(),destination,start,0,true));
    auto* macroDepth=synth.state.getParameter(synth.modulationRouteId(0,0,true));
    if(std::abs(macroDepth->convertFrom0to1(macroDepth->getValue())-.5f)>.001f)return 37;
    auto* depthBadge=findChild("Modulation depth Cutoff");
    if(!depthBadge||!depthBadge->isVisible())return 38;
    const float unchangedCutoff=cutoff->getValue();
    const juce::Point<float> grab{8,8},increase{8,-90},decrease{8,190};
    depthBadge->mouseDown(event(depthBadge,grab,grab,juce::ModifierKeys::leftButtonModifier,false));
    depthBadge->mouseDrag(event(depthBadge,increase,grab,juce::ModifierKeys::leftButtonModifier,true));
    if(macroDepth->convertFrom0to1(macroDepth->getValue())<.8f)return 39;
    depthBadge->mouseDrag(event(depthBadge,decrease,grab,juce::ModifierKeys::leftButtonModifier,true));
    depthBadge->mouseUp(event(depthBadge,decrease,grab,0,true));
    if(macroDepth->convertFrom0to1(macroDepth->getValue())>=0.0f || cutoff->getValue()!=unchangedCutoff)return 40;
    auto legacyState=synth.state.copyState();
    for(int child=legacyState.getNumChildren()-1;child>=0;--child){
        const auto id=legacyState.getChild(child).getProperty("id").toString();
        if(id.startsWith("modRoute")||id.startsWith("modDepth")||id.startsWith("designerLfo")||id=="analogueCharacter")
            legacyState.removeChild(child,nullptr);
    }
    auto legacyXml=legacyState.createXml();
    juce::MemoryBlock legacyBytes;
    juce::AudioProcessor::copyXmlToBinary(*legacyXml,legacyBytes);
    synth.setStateInformation(legacyBytes.getData(),int(legacyBytes.getSize()));
    if(synth.state.getRawParameterValue("analogueCharacter")->load()!=0.0f)return 41;
    for(int source=4;source<8;++source)
        for(int slot=0;slot<3;++slot)
            if(synth.state.getRawParameterValue(synth.modulationRouteId(source,slot,false))->load()!=0.0f)return 42;
    std::cout << "Factory peaks " << quietest << " to " << loudest << '\n';
}
