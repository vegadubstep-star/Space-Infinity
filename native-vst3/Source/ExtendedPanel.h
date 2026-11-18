#pragma once
#include "PluginProcessor.h"
#include "WorkflowControls.h"
#include "EnvelopeControls.h"
class ExtendedPanel final : public juce::Component,private juce::Timer {
    using SliderBinding=juce::AudioProcessorValueTreeState::SliderAttachment;
    using ChoiceBinding=juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonBinding=juce::AudioProcessorValueTreeState::ButtonAttachment;
    class TableCanvas final : public juce::Component {
    public:
        SpaceInfinityProcessor& processor;int frame=0,last=-1;float lastValue=0;
        std::array<float,infinity::Wavetable::points> data{};
        explicit TableCanvas(SpaceInfinityProcessor& p):processor(p){reload();}
        void reload(){frame=juce::jlimit(0,processor.tableFrameCount()-1,frame);data=processor.tableFrame(frame);repaint();}
        void paint(juce::Graphics& g) override {
            g.fillAll(juce::Colour(0xff10171a));g.setColour(juce::Colour(0xff344047));g.drawHorizontalLine(getHeight()/2,0,float(getWidth()));
            juce::Path path;for(size_t i=0;i<data.size();++i){const float x=float(i)*getWidth()/float(data.size()-1),y=(1-data[i])*getHeight()*.5f;if(i==0)path.startNewSubPath(x,y);else path.lineTo(x,y);}
            g.setColour(juce::Colour(0xff74d9d0));g.strokePath(path,juce::PathStrokeType(1.5f));
        }
        void mouseDown(const juce::MouseEvent& e) override {last=-1;draw(e);}
        void mouseDrag(const juce::MouseEvent& e) override {draw(e);}
        void mouseUp(const juce::MouseEvent&) override {processor.editTableFrame(frame,data);reload();}
        void draw(const juce::MouseEvent& e){
            const int point=juce::jlimit(0,int(data.size())-1,int(e.position.x/getWidth()*float(data.size()-1)));
            const float value=juce::jlimit(-1.0f,1.0f,1-2*e.position.y/getHeight());
            if(last>=0&&last!=point){const int lo=std::min(point,last),hi=std::max(point,last);for(int i=lo;i<=hi;++i)data[size_t(i)]=lastValue+(value-lastValue)*float(i-last)/float(point-last);}
            data[size_t(point)]=value;last=point;lastValue=value;repaint();
        }
    };
    class EnvelopeCanvas final:public EnvelopeView {public:explicit EnvelopeCanvas(SpaceInfinityProcessor& p):EnvelopeView(p,false){}};
    SpaceInfinityProcessor& processor;
    std::array<juce::Component,9> pages;
    juce::TabbedComponent tabs{juce::TabbedButtonBar::TabsAtTop};
    std::vector<std::unique_ptr<juce::Component>> controls;
    std::vector<std::unique_ptr<SliderBinding>> sliders;
    std::vector<std::unique_ptr<ChoiceBinding>> choices;
    std::vector<std::unique_ptr<ButtonBinding>> buttons;
    TableCanvas table;
    EnvelopeCanvas envelope;EnvelopeView ampEnvelope;
    juce::Slider frame;
    juce::ComboBox frameSize;
    juce::Label status;juce::Label* labState=nullptr;
    std::unique_ptr<juce::FileChooser> chooser;
    template<class T> T& own(int page){auto item=std::make_unique<T>();auto& ref=*item;pages[size_t(page)].addAndMakeVisible(ref);controls.push_back(std::move(item));return ref;}
    void label(int page,const juce::String& text,juce::Rectangle<int> area){auto& l=own<juce::Label>(page);l.setText(text,juce::dontSendNotification);l.setBounds(area);l.setColour(juce::Label::textColourId,juce::Colour(0xffd2d4d8));}
    void slider(int page,const char* id,const char* name,int x,int y,int width=350){
        label(page,name,{x,y,145,26});auto& s=own<ModSlider>(page);s.initialiseModulation(processor,id);s.setBounds(x+145,y,width-145,27);
        s.setSliderStyle(juce::Slider::LinearHorizontal);s.setTextBoxStyle(juce::Slider::TextBoxRight,false,65,23);
        sliders.push_back(std::make_unique<SliderBinding>(processor.state,id,s));
        s.setNumDecimalPlacesToDisplay(3);
        const auto* p=processor.state.getParameter(id);s.setDoubleClickReturnValue(true,p->convertFrom0to1(p->getDefaultValue()));if(juce::String(id).containsIgnoreCase("attack")||juce::String(id).containsIgnoreCase("release"))configureTimeKnob(s);
    }
    void knob(int page,const char* id,const char* name,int x,int y,int width,const char* unit=""){
        auto& caption=own<juce::Label>(page);caption.setText(name,juce::dontSendNotification);caption.setFont(juce::Font(juce::FontOptions(11)));caption.setBounds(x,y,width,20);caption.setJustificationType(juce::Justification::centred);caption.setColour(juce::Label::textColourId,juce::Colour(0xffd2d4d8));
        auto& s=own<ModSlider>(page);s.initialiseModulation(processor,id);s.setBounds(x+(width-80)/2,y+21,80,page==7?68:78);
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);s.setTextBoxStyle(juce::Slider::TextBoxBelow,false,76,18);
        sliders.push_back(std::make_unique<SliderBinding>(processor.state,id,s));const auto suffix=juce::String(unit);
        s.textFromValueFunction=[suffix](double value){return suffix=="%"?juce::String(value*100,0)+"%":juce::String(value,suffix=="s"?3:2)+(suffix.isEmpty()?juce::String{}:" "+suffix);};
        s.valueFromTextFunction=[suffix](const juce::String& text){return text.getDoubleValue()/(suffix=="%"?100:1);};s.updateText();
        const auto* p=processor.state.getParameter(id);s.setDoubleClickReturnValue(true,p->convertFrom0to1(p->getDefaultValue()));if(juce::String(id).containsIgnoreCase("attack")||juce::String(id).containsIgnoreCase("release"))configureTimeKnob(s);
    }
    void choice(int page,const char* id,juce::StringArray names,int x,int y,int width){auto& c=own<juce::ComboBox>(page);c.addItemList(names,1);c.setBounds(x,y,width,26);choices.push_back(std::make_unique<ChoiceBinding>(processor.state,id,c));}
    juce::TextButton& action(int page,const char* text,juce::Rectangle<int> area,std::function<void()> callback){auto& b=own<juce::TextButton>(page);b.setButtonText(text);b.setBounds(area);b.onClick=std::move(callback);return b;}
    void showResult(const juce::Result& result,const juce::String& success){status.setText(result.wasOk()?success:result.getErrorMessage(),juce::dontSendNotification);}
    void updateFrames(){const int count=processor.tableFrameCount();frame.setRange(1,std::max(2,count),1);frame.setEnabled(count>1);frame.setValue(table.frame+1,juce::dontSendNotification);table.reload();}
    void chooseTable(bool save){
        chooser=std::make_unique<juce::FileChooser>(save?"Export wavetable (512 samples per frame)":"Import wavetable WAV",juce::File{},"*.wav");
        const int flags=(save?juce::FileBrowserComponent::saveMode|juce::FileBrowserComponent::warnAboutOverwriting:juce::FileBrowserComponent::openMode)|juce::FileBrowserComponent::canSelectFiles;
        chooser->launchAsync(flags,[safe=juce::Component::SafePointer<ExtendedPanel>(this),save](const juce::FileChooser& c){
            if(!safe||c.getResult()==juce::File{})return;
            const auto result=save?safe->processor.exportWavetable(c.getResult()):safe->processor.importWavetable(c.getResult(),safe->frameSize.getSelectedId());
            safe->showResult(result,save?"Wavetable exported (512 samples/frame).":"Wavetable loaded into oscillator A.");safe->table.frame=0;safe->updateFrames();
        });
    }
    void chooseImpulse(){
        chooser=std::make_unique<juce::FileChooser>("Load impulse response",juce::File{},"*.wav");
        const juce::Component::SafePointer<ExtendedPanel> safe(this);
        chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,
            [safe](const juce::FileChooser& c){
                if(safe!=nullptr&&c.getResult()!=juce::File{})
                    safe->showResult(safe->processor.importImpulse(c.getResult()),"Impulse loaded: "+c.getResult().getFileName());
            });
    }
    void timerCallback() override {envelope.repaint();ampEnvelope.repaint();if(labState){const bool digital=processor.state.getRawParameterValue("engineMode")->load()>.5f,on=processor.state.getRawParameterValue("analogueLabEnabled")->load()>.5f;labState->setText(digital?"BYPASSED: Digital mode - press ACTIVATE LAB to hear these controls.":!on?"LAB OFF - press ACTIVATE LAB to hear component variation and colour.":"LAB ACTIVE - Studio / Vintage expand the range; WARM and VINTAGE are audible starting points.",juce::dontSendNotification);}}
public:
    explicit ExtendedPanel(SpaceInfinityProcessor& p,std::function<void()> showSequence={}):processor(p),table(p),envelope(p),ampEnvelope(p,true){
        setSize(730,465);addAndMakeVisible(tabs);
        const int visiblePages[]{0,1,3,7,8};const char* names[]{"EXPRESSION","GRANULAR","WAVETABLE","ANALOG LAB","PITCH"};
        for(int i=0;i<5;++i)tabs.addTab(names[i],juce::Colour(0xff252b31),&pages[size_t(visiblePages[i])],false);
        knob(0,"ampVelocity","VELOCITY TO VOLUME",30,55,300,"%");knob(0,"filterVelocity","VELOCITY TO FILTER",370,55,300,"oct");
        label(0,"Amp/filter envelopes and contours are on MAIN. Portamento is beside the main-page voicing selector.\nConvolver controls and impulse import are in the FX rack.",{25,210,680,90});
        label(0,"OUTPUT SAFETY",{25,320,145,26});choice(0,"outputSafety",{"Soft saturation","Clean headroom"},175,320,255);
        slider(1,"grainPosition","Position",15,40);slider(1,"grainSize","Grain size (ms)",365,40);
        slider(1,"grainDensity","Grains / second",15,100);slider(1,"grainSpray","Position spray",365,100);
        slider(1,"grainSpread","Stereo spread",15,160);
        action(1,"USE GRANULAR WAV",{15,230,200,30},[this]{auto* source=processor.state.getParameter("source");source->setValueNotifyingHost(source->convertTo0to1(4));});
        label(1,"Import a WAV on the Main page, then select Granular WAV. Uses sample trim, root note and reverse.\nUp to 32 overlapping, windowed grains per note; release follows the amp envelope.",{15,280,690,65});
        label(2,"The note sequencer has its own SEQ page, separate from the audio gate.",{20,45,680,60});
        action(2,"OPEN SEQUENCER PAGE",{20,125,280,40},[showSequence]{if(showSequence)showSequence();});
        pages[3].addAndMakeVisible(table);table.setBounds(15,100,690,180);
        frame.setSliderStyle(juce::Slider::LinearHorizontal);frame.setTextBoxStyle(juce::Slider::TextBoxRight,false,60,24);frame.setBounds(80,55,230,27);
        frame.onValueChange=[this]{table.frame=juce::jlimit(0,processor.tableFrameCount()-1,int(frame.getValue())-1);table.reload();};pages[3].addAndMakeVisible(frame);
        label(3,"Frame",{15,55,60,27});pages[3].addAndMakeVisible(frameSize);frameSize.addItem("512 samples/frame",512);frameSize.addItem("1024 samples/frame",1024);frameSize.addItem("2048 samples/frame",2048);frameSize.setSelectedId(2048);frameSize.setBounds(335,55,185,27);
        action(3,"IMPORT",{15,10,105,28},[this]{chooseTable(false);});action(3,"EXPORT",{130,10,105,28},[this]{chooseTable(true);});
        action(3,"DUPLICATE",{245,10,110,28},[this]{processor.addTableFrame(table.frame);table.frame=processor.tableFrameCount()-1;updateFrames();});
        action(3,"DELETE FRAME",{365,10,130,28},[this]{processor.removeTableFrame(table.frame);table.frame=juce::jlimit(0,processor.tableFrameCount()-1,table.frame);updateFrames();});
        action(3,"SINE",{535,55,75,27},[this]{for(size_t i=0;i<table.data.size();++i)table.data[i]=float(std::sin(2*infinity::pi*double(i)/table.data.size()));processor.editTableFrame(table.frame,table.data);table.reload();});
        action(3,"SAW",{620,55,75,27},[this]{for(size_t i=0;i<table.data.size();++i)table.data[i]=2*float(i)/float(table.data.size())-1;processor.editTableFrame(table.frame,table.data);table.reload();});
        label(3,"Use WT POS on the oscillator to scan frames.",{15,305,690,26});
        action(3,"USE ON OSC A",{15,350,155,28},[this]{auto* wave=processor.state.getParameter("wave");wave->setValueNotifyingHost(wave->convertTo0to1(30));processor.state.getParameter("source")->setValueNotifyingHost(0);});
        label(3,"Draw, then release to apply. Up to 64 frames, 512 points each.\nChoose Custom Wavetable on either oscillator. Saves with project.",{185,345,520,50});updateFrames();
        knob(8,"midiBendRange","MIDI WHEEL RANGE",245,35,215,"st");knob(8,"concertPitch","A4 TUNING",475,35,215,"Hz");knob(8,"pitchEnvAmount","PITCH ENVELOPE",15,155,215,"st");knob(8,"pitchEnvAttack","PITCH ATTACK",245,155,215,"s");knob(8,"pitchEnvDecay","PITCH DECAY",475,155,215,"s");knob(8,"pitchEnvVelocity","PITCH VELOCITY",15,275,215,"%");
        label(8,"The main-page wheel exposes Pitch Wheel to FL Studio automation (±24 semitones).\nMIDI Wheel Range sets the range of an incoming MIDI pitch wheel.\nUse Mono or Legato and PORTAMENTO for gliding notes.",{245,290,445,90});
        auto& lab=own<juce::ToggleButton>(7);lab.setButtonText("COMPONENT VARIATION / COLOUR");lab.setBounds(15,10,310,28);buttons.push_back(std::make_unique<ButtonBinding>(processor.state,"analogueLabEnabled",lab));auto& stereo=own<juce::ToggleButton>(7);stereo.setButtonText("MATCHED STEREO FILTERS");stereo.setBounds(330,10,225,28);buttons.push_back(std::make_unique<ButtonBinding>(processor.state,"stereoFilterUpgrade",stereo));auto& clean=own<juce::ToggleButton>(7);clean.setButtonText("CLEAN BASE VCO");clean.setTooltip("Independent saw/pulse edge correction; does not cover all wavetable or warp modes.");clean.setBounds(570,10,145,28);buttons.push_back(std::make_unique<ButtonBinding>(processor.state,"cleanVCO",clean));
        labState=&own<juce::Label>(7);labState->setBounds(15,36,700,24);labState->setColour(juce::Label::textColourId,juce::Colour(0xff80e4dd));
        choice(7,"analogueLabRange",{"Calibrated / subtle","Studio / audible","Vintage / wide"},15,65,205);
        const auto labPreset=[this](int preset){const auto set=[this](const char* id,float value){auto* p=processor.state.getParameter(id);p->setValueNotifyingHost(p->convertTo0to1(value));};set("engineMode",0);set("analogueLabEnabled",1);if(preset<0)return;set("analogueLabRange",preset==2?2:1);const char* ids[]{"analogueOscDrift","analogueThermal","analogueFilterSpread","analogueEnvSpread","analogueVoiceSpread","analoguePulseDrift","analogueVCADrive","analogueAsymmetry","analogueMemory","analogueCoupling"};const float warm[]{.18f,.14f,.12f,.10f,.16f,.13f,.32f,.16f,.18f,.12f},vintage[]{.40f,.30f,.32f,.24f,.30f,.35f,.55f,.35f,.38f,.22f};for(int i=0;i<10;++i)set(ids[i],preset==0?0:preset==1?warm[i]:vintage[i]);};
        action(7,"ACTIVATE LAB",{230,65,133,26},[labPreset]{labPreset(-1);});action(7,"NEUTRAL",{373,65,93,26},[labPreset]{labPreset(0);});action(7,"WARM",{476,65,100,26},[labPreset]{labPreset(1);});action(7,"VINTAGE",{586,65,125,26},[labPreset]{labPreset(2);});
        const char* labIds[]{"analogueOscDrift","analogueThermal","analogueFilterSpread","analogueEnvSpread","analogueVoiceSpread","analoguePulseDrift","analogueVCADrive","analogueAsymmetry","analogueMemory","analogueCoupling","noisePan"};const char* labNames[]{"VCO WANDER","THERMAL DRIFT","FILTER TOLERANCE","ENVELOPE TOLERANCE","VOICE TOLERANCE","PULSE DRIFT","VCA DRIVE","ASYMMETRY","CIRCUIT MEMORY","AC COUPLING","NOISE PAN"};for(int i=0;i<11;++i)knob(7,labIds[i],labNames[i],15+(i%4)*174,100+(i/4)*94,168,"%");
        label(7,"VCO / pulse controls affect Synth.\nVCA, memory and coupling also colour samples.",{540,304,170,69});
        addAndMakeVisible(status);status.setBounds(15,433,700,27);status.setColour(juce::Label::textColourId,juce::Colour(0xffd8c8a7));startTimerHz(15);timerCallback();
    }
    void selectTab(int index){const int pages[]{0,1,3,7,8};for(int i=0;i<5;++i)if(pages[i]==index){tabs.setCurrentTabIndex(i);return;}tabs.setCurrentTabIndex(0);}
    void resized() override {tabs.setBounds(0,0,730,425);}
};
