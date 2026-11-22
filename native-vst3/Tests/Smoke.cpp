#include "../Source/PluginProcessor.h"
#include <cmath>
#include <iostream>
#include <fstream>
#if JUCE_WINDOWS
 #ifndef NOMINMAX
  #define NOMINMAX
 #endif
 #include <windows.h>
#endif

int runBankZipSmoke(const juce::File&);
int runSoundCharacterSmoke();
int runLushBankSmoke(const juce::File&);
int runBassSmoke();
int exportBassRangeProfile(const juce::File&);
int exportBassAuditions(const juce::File&,const juce::File&);
int exportOwnerChoirStabs(const juce::File& folder);
int exportAcidCutoffSweeps(const juce::File&,const juce::File&);
int runAcidSmoke(const juce::File& folder={},const juce::File& previousBank={});
int runCreativeSmoke();
int runUsabilitySmoke();
int runAnalogueTailProbe();
int runAnalogueExpansionSmoke();
int runOscillatorWorkflowSmoke();
int runDualLfoSmoke();
int runNoiseModeSmoke();
int runModulationWorkflowSmoke();
int runChorusMeltSmoke();
int runPitchTravelSmoke();
int runRhythmWorkflowSmoke();
int runWorkflowSmoke();
int runExtendedSmoke();
int runPresetSmoke();
int runStudioSmoke();
int runTranceBankSmoke(int group,const juce::File& profile={});
int runLeadSmoke();


int writeParameterInventory(const juce::File& file){
    auto p=std::make_unique<SpaceInfinityProcessor>();auto* root=new juce::DynamicObject;juce::var result(root);root->setProperty("native_version",JucePlugin_VersionString);root->setProperty("manual_reference","https://xferrecords.com/manual/serum-2/docs");root->setProperty("notes","Host defaults preserve omitted-field legacy recall. Init values reflect a fresh Init preset. Ranges are native parameter units; UI formatting may apply units and scaling.");juce::Array<juce::var> rows;
    for(auto* parameter:p->getParameters())if(auto* a=dynamic_cast<juce::RangedAudioParameter*>(parameter)){auto* row=new juce::DynamicObject;row->setProperty("id",a->paramID);row->setProperty("name",a->getName(120));row->setProperty("type",dynamic_cast<juce::AudioParameterBool*>(a)?"boolean":dynamic_cast<juce::AudioParameterChoice*>(a)?"choice":dynamic_cast<juce::AudioParameterInt*>(a)?"integer":"float");row->setProperty("minimum",a->convertFrom0to1(0));row->setProperty("maximum",a->convertFrom0to1(1));row->setProperty("host_default",a->convertFrom0to1(a->getDefaultValue()));row->setProperty("init_value",p->state.getRawParameterValue(a->paramID)->load());if(auto* choice=dynamic_cast<juce::AudioParameterChoice*>(a)){juce::Array<juce::var> names;for(const auto& name:choice->choices)names.add(name);row->setProperty("choices",names);}rows.add(juce::var(row));}root->setProperty("parameter_count",rows.size());root->setProperty("parameters",rows);if(file.getParentDirectory().createDirectory().failed()||!file.replaceWithText(juce::JSON::toString(result)))return 125;std::cout<<"Exported "<<rows.size()<<" native parameters to "<<file.getFullPathName()<<'\n';return 0;
}

#if JUCE_WINDOWS
int verifyNativeLogo(juce::AudioProcessorEditor& editor,const juce::File& folder){
    editor.setSize(1000,760);editor.setOpaque(true);editor.addToDesktop(0);editor.setVisible(true);
    auto& desktop=juce::Desktop::getInstance();const float previousScale=desktop.getGlobalScaleFactor();
    struct RestoreDesktop {juce::AudioProcessorEditor& editor;juce::Desktop& desktop;float scale;~RestoreDesktop(){desktop.setGlobalScaleFactor(scale);editor.removeFromDesktop();}} restore{editor,desktop,previousScale};
    const auto engines=editor.getPeer()->getAvailableRenderingEngines();int captures=0;
    for(int engine=0;engine<engines.size();++engine)for(float scale:{1.0f,1.25f,1.5f,2.0f}){
        desktop.setGlobalScaleFactor(scale);
        // Global scaling preserves physical window bounds. Reapply logical size
        // so this remains a supported 1000x760 editor, rather than a shrunken editor.
        editor.setSize(1000,760);editor.setTopLeftPosition(desktop.getDisplays().getPrimaryDisplay()->userArea.getTopLeft());
        auto* peer=editor.getPeer();peer->setCurrentRenderingEngine(engine);editor.toFront(false);editor.repaint();
        for(int tick=0;tick<30;++tick){MSG message;const auto deadline=juce::Time::getMillisecondCounterHiRes()+5;while(juce::Time::getMillisecondCounterHiRes()<deadline&&PeekMessageW(&message,nullptr,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}peer->performAnyPendingRepaintsNow();juce::Thread::sleep(15);}
        const auto actual=juce::createSnapshotOfNativeWindow(peer->getNativeHandle());
        if(!actual.isValid()){std::cout<<"Native logo capture unavailable: "<<engines[engine]<<"\n";continue;}
        const auto image=actual.rescaled(editor.getWidth(),editor.getHeight(),juce::Graphics::highResamplingQuality);
        const auto reference=editor.createComponentSnapshot(editor.getLocalBounds());juce::Image::BitmapData expectedPixels(reference,juce::Image::BitmapData::readOnly),actualPixels(image,juce::Image::BitmapData::readOnly);int count=0,matching=0;
        for(int y=32;y<63;++y)for(int x=38;x<262;++x){const auto expected=expectedPixels.getPixelColour(x,y),background=expectedPixels.getPixelColour(266,y);if(int(expected.getGreen())-int(background.getGreen())>35){++count;const auto pixel=actualPixels.getPixelColour(x,y);if(std::abs(int(pixel.getRed())-int(expected.getRed()))<35&&std::abs(int(pixel.getGreen())-int(expected.getGreen()))<35&&std::abs(int(pixel.getBlue())-int(expected.getBlue()))<35)++matching;}}
        std::cout<<"Native logo "<<engines[engine]<<" scale "<<scale<<" logical "<<editor.getWidth()<<"x"<<editor.getHeight()<<" native "<<actual.getWidth()<<"x"<<actual.getHeight()<<": "<<matching<<"/"<<count<<" lower-stroke pixels match\n";
        auto file=folder.getChildFile("Logo-Native-"+juce::String(engine)+"-"+juce::String(int(scale*100))+".png");file.deleteFile();auto stream=file.createOutputStream();if(!stream||!juce::PNGImageFormat{}.writeImageToStream(actual,*stream))return 106;
        if(editor.getWidth()!=1000||editor.getHeight()!=760||count<150||matching<count*.9)return 107;++captures;
    }
    return captures>0?0:108;
}
#endif

int renderHardwarePreview()
{
    auto synth=std::make_unique<SpaceInfinityProcessor>();
    synth->loadPreset(0);
    auto editor=std::unique_ptr<juce::AudioProcessorEditor>(synth->createEditor());
    const auto folder=juce::File::getCurrentWorkingDirectory().getChildFile("audition");
    if(folder.createDirectory().failed())return 101;
    for(const auto* page:{"MAIN","FX","GATE","SEQ"}){
        juce::TextButton* tab=nullptr;
        for(auto* child:editor->getChildren())
            if(auto* button=dynamic_cast<juce::TextButton*>(child))
                if(button->getButtonText()==page)tab=button;
        if(!tab)return 102;
        tab->onClick();
        const auto parameter=[&](const juce::String& id,float value){auto* a=synth->state.getParameter(id);a->setValueNotifyingHost(a->convertTo0to1(value));};
        if(juce::String(page)=="GATE"){parameter("gateEnabled",1);parameter("gateMode",1);parameter("gateCurveRate",4);parameter("gateCurveCount",3);for(int i=0;i<3;++i){parameter("gateCurveX"+juce::String(i),float(i)/2);parameter("gateCurveY"+juce::String(i),i==0?.08f:i==1?.8f:1);parameter("gateCurveBend"+juce::String(i),i==0?.6f:-.45f);}}
        if(juce::String(page)=="SEQ"){parameter("patternMode",2);for(int i=0;i<16;++i){parameter("stepPitch"+juce::String(i),i%4==0?0:i%4==1?7:i%4==2?12:3);parameter("stepVelocity"+juce::String(i),i%4==0?1:.6f);}}
        for(const auto size:{juce::Point<int>(800,760),juce::Point<int>(1000,790),juce::Point<int>(1180,820),juce::Point<int>(1280,760),juce::Point<int>(1500,950)}){
            editor->setSize(size.x,size.y);
            for(auto* child:editor->getChildren())
                if(child->isVisible()&&!editor->getLocalBounds().contains(child->getBounds()))return 103;
            const auto image=editor->createComponentSnapshot(editor->getLocalBounds(),true,1.0f);
            const auto file=folder.getChildFile(juce::String("Hardware-")+page+"-"+juce::String(size.x)+".png");
            file.deleteFile();auto stream=file.createOutputStream();
            if(!stream||!juce::PNGImageFormat{}.writeImageToStream(image,*stream))return 104;
        }
        if(juce::String(page)=="GATE"){synth->loadGatePreset(2);editor->setSize(1000,790);const auto file=folder.getChildFile("Hardware-GATE-BLOCKS-1000.png");file.deleteFile();auto stream=file.createOutputStream();if(!stream||!juce::PNGImageFormat{}.writeImageToStream(editor->createComponentSnapshot(editor->getLocalBounds()),*stream))return 109;}
    }
    auto* sampleSource=synth->state.getParameter("source");sampleSource->setValueNotifyingHost(sampleSource->convertTo0to1(1));
    for(auto* child:editor->getChildren())if(auto* tab=dynamic_cast<juce::TextButton*>(child))if(tab->getButtonText()=="MAIN")tab->onClick();
    for(const auto size:{juce::Point<int>(800,760),juce::Point<int>(1280,760)}){
        editor->setSize(size.x,size.y);
        for(auto* child:editor->getChildren())if(child->isVisible()&&!editor->getLocalBounds().contains(child->getBounds()))return 110;
        const auto file=folder.getChildFile("Hardware-SAMPLER-"+juce::String(size.x)+".png");file.deleteFile();auto stream=file.createOutputStream();
        if(!stream||!juce::PNGImageFormat{}.writeImageToStream(editor->createComponentSnapshot(editor->getLocalBounds()),*stream))return 111;
    }
    for(float scale:{1.0f,1.25f,1.5f,2.0f,3.0f,4.0f}){const auto image=editor->createComponentSnapshot({30,8,240,60},true,scale);const auto file=folder.getChildFile("Logo-DPI-"+juce::String(int(scale*100))+".png");file.deleteFile();auto stream=file.createOutputStream();if(!stream||!juce::PNGImageFormat{}.writeImageToStream(image,*stream))return 105;}
   #if JUCE_WINDOWS
    if(const int result=verifyNativeLogo(*editor,folder))return result;
   #endif
    std::cout<<"Native hardware editor: Main/FX/Gate/Seq screenshots and bounds passed at 800, 1000, 1180, 1280 (wide default) and 1500 pixels\n";
    return 0;
}

int runSamplerHybridSmoke();
int runBundled093Probe(const juce::File&);
int runInstalledBankSmoke(const juce::File&);
int main(int argc,char* argv[])
{
    if(argc>2&&juce::String(argv[1])=="--bank-zip-tests"){juce::ScopedJuceInitialiser_GUI init;return runBankZipSmoke(juce::File(argv[2]));}

    juce::ScopedJuceInitialiser_GUI gui;
    if(argc>2&&juce::String(argv[1])=="--bass-range-profile")return exportBassRangeProfile(juce::File(argv[2]));
    if(argc>2&&juce::String(argv[1])=="--lush-bank-tests")return runLushBankSmoke(juce::File(argv[2]));
    if(argc>2&&juce::String(argv[1])=="--bundled093-probe")return runBundled093Probe(juce::File(argv[2]));
    if(argc>1&&juce::String(argv[1])=="--sampler-hybrid-tests")return runSamplerHybridSmoke();
    if(argc>1&&juce::String(argv[1])=="--sound-character-tests")return runSoundCharacterSmoke();
    if(argc>1&&juce::String(argv[1])=="--bass-tests")return runBassSmoke();
    if(argc>3&&juce::String(argv[1])=="--bass-audition")return exportBassAuditions(juce::File(argv[2]),juce::File(argv[3]));
    if(argc>2&&juce::String(argv[1])=="--export-choir-stabs")return exportOwnerChoirStabs(juce::File(argv[2]));
    if(argc>3&&juce::String(argv[1])=="--acid-cutoff-audition")return exportAcidCutoffSweeps(juce::File(argv[2]),juce::File(argv[3]));
    if(argc>1&&juce::String(argv[1])=="--acid-tests")return runAcidSmoke();
    if(argc>2&&juce::String(argv[1])=="--acid-audition")return runAcidSmoke(juce::File(argv[2]),argc>3?juce::File(argv[3]):juce::File{});
    if(argc>1&&juce::String(argv[1])=="--creative-tests")return runCreativeSmoke();
    if(argc>1&&juce::String(argv[1])=="--usability-tests")return runUsabilitySmoke();
    if(argc>2&&juce::String(argv[1])=="--trance-bank-solo-profile")return runTranceBankSmoke(3,juce::File(argv[2]));
    if(argc>2&&juce::String(argv[1])=="--trance-bank-profile")return runTranceBankSmoke(2,juce::File(argv[2]));
    if(argc>2&&juce::String(argv[1])=="--installed-bank-tests")return runInstalledBankSmoke(juce::File(argv[2]));
    if(argc>2&&juce::String(argv[1])=="--parameter-inventory")return writeParameterInventory(juce::File(argv[2]));
    if(argc>1&&juce::String(argv[1])=="--analogue-tail-probe")return runAnalogueTailProbe();
    if(argc>1&&juce::String(argv[1])=="--analogue-expansion-tests")return runAnalogueExpansionSmoke();
    if(argc>1&&juce::String(argv[1])=="--oscillator-workflow-tests")return runOscillatorWorkflowSmoke();
    if(argc>1&&juce::String(argv[1])=="--pitch-travel-tests")return runPitchTravelSmoke();
    if(argc>1&&juce::String(argv[1])=="--chorus-melt-tests")return runChorusMeltSmoke();
    if(argc>1&&juce::String(argv[1])=="--modulation-workflow-tests")return runModulationWorkflowSmoke();
    if(argc>1&&juce::String(argv[1])=="--noise-mode-tests")return runNoiseModeSmoke();
    if(argc>1&&juce::String(argv[1])=="--dual-lfo-tests")return runDualLfoSmoke();
    if(argc>1&&juce::String(argv[1])=="--rhythm-workflow-tests")return runRhythmWorkflowSmoke();
    if(argc>1&&juce::String(argv[1])=="--workflow-tests")return runWorkflowSmoke();
    if(argc>1&&juce::String(argv[1])=="--ui-only")return renderHardwarePreview();
    if(argc>1&&juce::String(argv[1])=="--preset-tests")return runPresetSmoke();
    if(argc>1&&juce::String(argv[1])=="--extended-tests")return runExtendedSmoke();
    if(argc>1&&juce::String(argv[1])=="--groove-tests")return runTranceBankSmoke(0);
    if(argc>1&&juce::String(argv[1])=="--lead-tests")return runLeadSmoke();
    if(argc>1&&juce::String(argv[1])=="--pad-tests")return runTranceBankSmoke(1);
    if(argc>1&&juce::String(argv[1])=="--studio-tests")return runStudioSmoke();
    // Hosts allocate processors on the heap. Keep test dispatch's main stack
    // small as well, including when it calls another smoke-test entry point.
    auto synthInstance=std::make_unique<SpaceInfinityProcessor>();
    auto& synth=*synthInstance;
    constexpr int blockSize=256;
    juce::AudioBuffer<float> audio(2,blockSize);
    const auto& names=synth.presetNames();
    if(names.size()!=161 || synth.motionNames().size()!=20) return 1;

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
    // Wrap in the order visible in the saved-order selector.
    const int lastDisplayedId=factoryMenu->getItemId(factoryMenu->getNumItems()-1);
    if(synth.loadBankPreset(lastDisplayedId-1).failed())return 23;
    next->onClick();
    if(factoryMenu->getSelectedId()!=1) return 23;
    previous->onClick();
    if(factoryMenu->getSelectedId()!=lastDisplayedId) return 24;
    auto* cutoff=synth.state.getParameter("cutoff");

    const auto audition=juce::File::getCurrentWorkingDirectory().getChildFile("audition");
    if(audition.exists()&&!audition.deleteRecursively()) return 17;
    if(audition.createDirectory().failed()) return 17;
    std::ofstream levels(audition.getChildFile("levels.tsv").getFullPathName().toStdString());
    if(!levels) return 18;
    levels << "Preset\tPeak dBFS\tRMS dBFS\n";
    for(const auto* id:{"attack","release"}){
        auto* parameter=synth.state.getParameter(id);
        if(parameter->convertFrom0to1(0.0f)!=0.0f || parameter->convertFrom0to1(1.0f)!=1.0f) return 26;
    }
    float quietest=1.0f, loudest=0.0f;
    for(int index=0;index<names.size();++index){
        if(names[index].isEmpty() || synth.presetCategory(index).isEmpty()) return 2;
        synth.loadPreset(index);
        const auto expectedCutoff=cutoff->getValue();
        // Archived factory definitions remain loadable for old projects, but
        // only retained entries have a selector item. IDs use the bank cursor.
        const int bankIndex=synth.presetBank().indexForKey(synth.presetBank().keyForFactory(index));
        if(bankIndex>=0){
            cutoff->setValueNotifyingHost(expectedCutoff>0.5f?0.0f:1.0f);
            factoryMenu->setSelectedId(0,juce::dontSendNotification);
            factoryMenu->setSelectedId(bankIndex+1,juce::sendNotificationSync);
            if(factoryMenu->getText()!=names[index] || std::abs(cutoff->getValue()-expectedCutoff)>0.001f){std::cerr<<"Selector failed for retained slot "<<names[index]<<std::endl;return 11;}
        }
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
            // Small buffers can be shorter than the announced look-ahead.
            // Require audible, bounded output after the reported latency.
            float peak=0;const int blocks=(synth.getLatencySamples()+frames-1)/frames+2;
            for(int block=0;block<blocks;++block){
                synth.processBlock(buffer,events);events.clear();
                const auto current=buffer.getMagnitude(0,frames);
                if(!std::isfinite(current)||current>1.0f)return 12;
                peak=std::max(peak,current);
            }
            if(peak<=0.0f)return 12;
        }
    }
    for(int shape=0;shape<20;++shape){
        synth.loadMotionShape(shape);
        for(int i=0;i<17;++i){
            const auto value=synth.state.getParameter(synth.motionPointId(i))->getValue();
            if(!std::isfinite(value)||value<0.0f||value>1.0f) return 13;
        }
    }
    for(const auto size : {juce::Point<int>(800,760),juce::Point<int>(1000,790),juce::Point<int>(1180,820),juce::Point<int>(1500,950)}){
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
    for(const auto size:{juce::Point<int>(800,760),juce::Point<int>(1000,790),juce::Point<int>(1180,820)}){
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
        auto* division=synth.state.getParameter(synth.lfoParameterId(lfo,"Bars"));
        if(division->convertFrom0to1(1.0f)!=7.0f)return 43;
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
        if(id.startsWith("modRoute")||id.startsWith("modDepth")||id.startsWith("designerLfo")||(id=="analogueCharacter"||id=="ampTimed"))
            legacyState.removeChild(child,nullptr);
    }
    auto legacyXml=legacyState.createXml();
    juce::MemoryBlock legacyBytes;
    juce::AudioProcessor::copyXmlToBinary(*legacyXml,legacyBytes);
    synth.setStateInformation(legacyBytes.getData(),int(legacyBytes.getSize()));
    if(synth.state.getRawParameterValue("analogueCharacter")->load()!=0.0f)return 41;
    if(synth.state.getRawParameterValue("ampTimed")->load()!=0.0f)return 44;
    for(int source=4;source<8;++source)
        for(int slot=0;slot<3;++slot)
            if(synth.state.getRawParameterValue(synth.modulationRouteId(source,slot,false))->load()!=0.0f)return 42;
    std::cout << "Factory peaks " << quietest << " to " << loudest << '\n';
    const int extended=runExtendedSmoke();if(extended)return extended;
    return runPresetSmoke();
}
