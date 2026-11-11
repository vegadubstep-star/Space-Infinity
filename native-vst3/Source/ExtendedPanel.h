#pragma once
#include "PluginProcessor.h"
#include "WorkflowControls.h"
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
    class EnvelopeCanvas final : public juce::Component {
        SpaceInfinityProcessor& processor;
    public:
        explicit EnvelopeCanvas(SpaceInfinityProcessor& p):processor(p){}
        void paint(juce::Graphics& g) override {
            g.fillAll(juce::Colour(0xff10171a));const auto value=[&](const char* id){return processor.state.getRawParameterValue(id)->load();};
            const float a=value("filterAttack"),d=value("filterDecay"),s=value("filterSustain"),r=value("filterRelease"),total=a+d+r+.4f;
            const auto x=[&](float t){return 10+(getWidth()-20)*t/total;};const auto y=[&](float v){return getHeight()-10-v*(getHeight()-20);};
            juce::Path path;path.startNewSubPath(x(0),y(0));const bool curved=value("analogueUpgrade")>.5f;
            for(int segment=0;segment<3;++segment)for(int i=1;i<=40;++i){const float t=float(i)/40,c=curved?AnalogueEnvelope::charge(t):t;
                if(segment==0)path.lineTo(x(a*t),y(c));
                if(segment==1)path.lineTo(x(a+d*t),y(1+(s-1)*c));
                if(segment==2)path.lineTo(x(a+d+.4f+r*t),y(s*(1-c)));
                if(segment==1&&i==40)path.lineTo(x(a+d+.4f),y(s));}

            g.setColour(juce::Colour(0xffd1a4ed));g.strokePath(path,juce::PathStrokeType(2));
        }
    };
    SpaceInfinityProcessor& processor;
    std::array<juce::Component,6> pages;
    juce::TabbedComponent tabs{juce::TabbedButtonBar::TabsAtTop};
    std::vector<std::unique_ptr<juce::Component>> controls;
    std::vector<std::unique_ptr<SliderBinding>> sliders;
    std::vector<std::unique_ptr<ChoiceBinding>> choices;
    std::vector<std::unique_ptr<ButtonBinding>> buttons;
    TableCanvas table;
    EnvelopeCanvas envelope;
    juce::Slider frame;
    juce::ComboBox frameSize;
    juce::Label status;
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
        auto& caption=own<juce::Label>(page);caption.setText(name,juce::dontSendNotification);caption.setBounds(x,y,width,20);caption.setJustificationType(juce::Justification::centred);caption.setColour(juce::Label::textColourId,juce::Colour(0xffd2d4d8));
        auto& s=own<ModSlider>(page);s.initialiseModulation(processor,id);s.setBounds(x+(width-80)/2,y+21,80,78);
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
    void timerCallback() override {envelope.repaint();}
public:
    explicit ExtendedPanel(SpaceInfinityProcessor& p):processor(p),table(p),envelope(p){
        setSize(730,465);addAndMakeVisible(tabs);
        const char* names[]={"FILTER ENV","GRANULAR","ARP / SEQ","WAVETABLE","CONVOLVER","ANALOGUE"};
        for(int i=0;i<6;++i)tabs.addTab(names[i],juce::Colour(0xff252b31),&pages[size_t(i)],false);
        pages[0].addAndMakeVisible(envelope);envelope.setBounds(15,15,690,140);
        knob(0,"filterAttack","ATTACK",15,168,165,"s");knob(0,"filterDecay","DECAY",190,168,165,"s");
        knob(0,"filterSustain","SUSTAIN",365,168,165,"%");knob(0,"filterRelease","RELEASE",540,168,165,"s");
        slider(0,"filterEnvAmount","Amount (octaves)",15,285,690);
        slider(0,"filterVelocity","Velocity to cutoff (octaves)",15,330,690);
        slider(0,"glideTime","Legato glide (s)",15,365,690);
        slider(1,"grainPosition","Position",15,40);slider(1,"grainSize","Grain size (ms)",365,40);
        slider(1,"grainDensity","Grains / second",15,100);slider(1,"grainSpray","Position spray",365,100);
        slider(1,"grainSpread","Stereo spread",15,160);
        action(1,"USE GRANULAR WAV",{15,230,200,30},[this]{auto* source=processor.state.getParameter("source");source->setValueNotifyingHost(source->convertTo0to1(4));});
        label(1,"Import a WAV on the Main page, then select Granular WAV. Uses sample trim, root note and reverse.\nUp to 32 overlapping, windowed grains per note; release follows the amp envelope.",{15,280,690,65});
        choice(2,"patternMode",{"Off","Arpeggiator","Sequencer","Arp + Seq"},15,15,170);
        choice(2,"arpOrder",{"Up","Down","Up-Down","Random"},195,15,145);
        choice(2,"patternRate",{"1/4","1/8","1/16","1/32","1/8 triplet","1/16 triplet"},350,15,130);
        slider(2,"arpOctaves","Octaves",15,55,330);slider(2,"patternGate","Gate",365,55,340);
        slider(2,"sequenceLength","Steps",15,95,330);
        label(2,"Hold notes to play. Sequence pitches are semitones relative to the held root / arpeggiated note.",{15,130,690,30});
        for(int i=0;i<16;++i){const int x=80+i*39;label(2,juce::String(i+1),{x,170,36,22});
            auto& pitch=own<juce::Slider>(2);pitch.setSliderStyle(juce::Slider::LinearVertical);pitch.setTextBoxStyle(juce::Slider::TextBoxBelow,false,37,21);pitch.setBounds(x,197,37,100);
            sliders.push_back(std::make_unique<SliderBinding>(processor.state,"stepPitch"+juce::String(i),pitch));
            auto& vel=own<juce::Slider>(2);vel.setSliderStyle(juce::Slider::LinearVertical);vel.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);vel.setBounds(x,305,37,45);
            vel.setTooltip("Step "+juce::String(i+1)+" velocity");sliders.push_back(std::make_unique<SliderBinding>(processor.state,"stepVelocity"+juce::String(i),vel));
            auto& on=own<juce::ToggleButton>(2);on.setBounds(x+4,355,29,25);buttons.push_back(std::make_unique<ButtonBinding>(processor.state,"stepOn"+juce::String(i),on));
        }
        label(2,"Pitch",{8,235,64,23});label(2,"Velocity",{8,315,64,23});label(2,"On/rest",{8,355,64,23});
        pages[3].addAndMakeVisible(table);table.setBounds(15,100,690,180);
        frame.setSliderStyle(juce::Slider::LinearHorizontal);frame.setTextBoxStyle(juce::Slider::TextBoxRight,false,60,24);frame.setBounds(80,55,230,27);
        frame.onValueChange=[this]{table.frame=juce::jlimit(0,processor.tableFrameCount()-1,int(frame.getValue())-1);table.reload();};pages[3].addAndMakeVisible(frame);
        label(3,"Frame",{15,55,60,27});pages[3].addAndMakeVisible(frameSize);frameSize.addItem("512 samples/frame",512);frameSize.addItem("1024 samples/frame",1024);frameSize.addItem("2048 samples/frame",2048);frameSize.setSelectedId(2048);frameSize.setBounds(335,55,185,27);
        action(3,"IMPORT",{15,10,105,28},[this]{chooseTable(false);});action(3,"EXPORT",{130,10,105,28},[this]{chooseTable(true);});
        action(3,"DUPLICATE",{245,10,110,28},[this]{processor.addTableFrame(table.frame);table.frame=processor.tableFrameCount()-1;updateFrames();});
        action(3,"DELETE FRAME",{365,10,130,28},[this]{processor.removeTableFrame(table.frame);table.frame=juce::jlimit(0,processor.tableFrameCount()-1,table.frame);updateFrames();});
        action(3,"SINE",{535,55,75,27},[this]{for(size_t i=0;i<table.data.size();++i)table.data[i]=float(std::sin(2*infinity::pi*double(i)/table.data.size()));processor.editTableFrame(table.frame,table.data);table.reload();});
        action(3,"SAW",{620,55,75,27},[this]{for(size_t i=0;i<table.data.size();++i)table.data[i]=2*float(i)/float(table.data.size())-1;processor.editTableFrame(table.frame,table.data);table.reload();});
        slider(3,"wavetableScan","Frame scan",15,305,690);
        action(3,"USE ON OSC A",{15,350,155,28},[this]{auto* wave=processor.state.getParameter("wave");wave->setValueNotifyingHost(wave->convertTo0to1(30));processor.state.getParameter("source")->setValueNotifyingHost(0);});
        label(3,"Draw, then release to apply. Up to 64 frames, 512 points each.\nChoose Custom Wavetable on either oscillator. Saves with project.",{185,345,520,50});updateFrames();
        auto& enabled=own<juce::ToggleButton>(4);enabled.setButtonText("CONVOLVER ON");enabled.setBounds(15,25,200,28);buttons.push_back(std::make_unique<ButtonBinding>(processor.state,"convolverEnabled",enabled));
        action(4,"IMPORT IMPULSE WAV",{15,90,230,34},[this]{chooseImpulse();});
        slider(4,"convolverMix","Dry / wet",15,170,690);
        label(4,"Load a mono or stereo WAV impulse response (up to 4 seconds / 4 MB).\nConvolution runs after reverb and before EQ. The impulse is embedded in project state.",{15,235,690,70});
        auto& analogue=own<juce::ToggleButton>(5);analogue.setButtonText("ANALOGUE VOICES + CURVED ENVELOPES");analogue.setBounds(15,25,660,30);
        buttons.push_back(std::make_unique<ButtonBinding>(processor.state,"analogueUpgrade",analogue));
        knob(5,"midiBendRange","MIDI BEND",15,65,215,"st");knob(5,"analogueCharacter","CHARACTER",245,65,215,"%");knob(5,"filterInputDrive","PRE-FILTER DRIVE",475,65,215,"%");
        knob(5,"ampVelocity","VELOCITY TO VOLUME",15,170,215,"%");knob(5,"subLevel","SUB SINE",245,170,215,"%");knob(5,"pitchEnvVelocity","PITCH VELOCITY",475,170,215,"%");
        knob(5,"pitchEnvAmount","PITCH ENVELOPE",15,275,215,"st");knob(5,"pitchEnvAttack","PITCH ATTACK",245,275,215,"s");knob(5,"pitchEnvDecay","PITCH DECAY",475,275,215,"s");
        addAndMakeVisible(status);status.setBounds(15,433,700,27);status.setColour(juce::Label::textColourId,juce::Colour(0xffd8c8a7));startTimerHz(15);
    }
    void resized() override {tabs.setBounds(0,0,730,425);}
};
