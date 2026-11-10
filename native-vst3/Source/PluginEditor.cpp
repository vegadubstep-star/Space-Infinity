#include "PluginEditor.h"
#include <array>
#include <cmath>
#include <complex>
namespace {
class EqPanel final : public juce::Component, private juce::Timer {
public:
    explicit EqPanel(SpaceInfinityProcessor& p):processor(p),enabled(p.state,"eqEnabled",power){
        addAndMakeVisible(power);
        setSize(540,300);
        startTimerHz(24);
    }
    void resized() override {power.setBounds(12,4,120,24);}
    void paint(juce::Graphics& g) override{
        g.fillAll(juce::Colour(0xff181817));
        const auto area=graph();
        g.setColour(juce::Colour(0xff080909));
        g.fillRect(area);
        g.setColour(juce::Colour(0xff34302c));
        for(float f:{20,50,100,200,500,1000,2000,5000,10000,20000})
            g.drawVerticalLine(int(xAt(f)),area.getY(),area.getBottom());
        for(float db:{-18,-12,-6,0,6,12,18})
            g.drawHorizontalLine(int(yAt(db)),area.getX(),area.getRight());
        juce::Path curve;
        for(int x=0;x<=int(area.getWidth());++x){
            const double frequency=20.0*std::pow(1000.0,double(x)/area.getWidth());
            const auto z=std::polar(1.0,-juce::MathConstants<double>::twoPi*frequency/48000.0);
            double magnitude=1.0;
            for(int band=0;band<7;++band){
                const auto c=juce::dsp::IIR::ArrayCoefficients<double>::makePeakFilter(
                    48000.0,value(band,"Frequency"),value(band,"Q"),std::pow(10.0,value(band,"Gain")/20.0));
                magnitude*=std::abs((c[0]+c[1]*z+c[2]*z*z)/(c[3]+c[4]*z+c[5]*z*z));
            }
            const auto y=yAt(float(juce::jlimit(-18.0,18.0,20.0*std::log10(magnitude))));
            if(x==0)curve.startNewSubPath(area.getX(),y);else curve.lineTo(area.getX()+float(x),y);
        }
        g.setColour(juce::Colour(0xffdca55c));
        g.strokePath(curve,juce::PathStrokeType(1.6f));
        for(int band=0;band<7;++band){
            const auto x=xAt(value(band,"Frequency")),y=yAt(value(band,"Gain"));
            g.setColour(juce::Colour(band==selected?0xfff6c180:0xffa66d42));
            g.fillEllipse(x-9,y-9,18,18);
            g.setColour(juce::Colours::black);
            g.drawText(juce::String(band+1),int(x)-9,int(y)-9,18,18,juce::Justification::centred);
        }
        g.setColour(juce::Colour(0xffbdb3a6));
        g.drawText("Drag frequency / gain    •    Scroll over a band to change Q",
                   12,getHeight()-26,getWidth()-24,20,juce::Justification::centredLeft);
    }
    void mouseDown(const juce::MouseEvent& e) override{
        selected=bandAt(e.position);
        if(selected<0)return;
        for(const auto* field:{"Frequency","Gain"})parameter(selected,field)->beginChangeGesture();
    }
    void mouseDrag(const juce::MouseEvent& e) override{
        if(selected<0)return;
        const auto area=graph();
        const float x=juce::jlimit(0.0f,1.0f,(e.position.x-area.getX())/area.getWidth());
        const float y=juce::jlimit(0.0f,1.0f,(e.position.y-area.getY())/area.getHeight());
        set(selected,"Frequency",20.0f*std::pow(1000.0f,x));
        set(selected,"Gain",18.0f-36.0f*y);
        repaint();
    }
    void mouseUp(const juce::MouseEvent&) override{
        if(selected>=0)for(const auto* field:{"Frequency","Gain"})parameter(selected,field)->endChangeGesture();
        selected=-1;
    }
    void mouseWheelMove(const juce::MouseEvent& e,const juce::MouseWheelDetails& wheel) override{
        const int band=bandAt(e.position);
        if(band<0)return;
        auto* q=parameter(band,"Q");
        q->beginChangeGesture();
        set(band,"Q",juce::jlimit(0.2f,12.0f,value(band,"Q")*std::exp(wheel.deltaY)));
        q->endChangeGesture();
    }
private:
    SpaceInfinityProcessor& processor;
    juce::ToggleButton power{"EQ enabled"};
    juce::AudioProcessorValueTreeState::ButtonAttachment enabled;
    int selected=-1;
    void timerCallback() override {repaint();}
    juce::Rectangle<float> graph() const {return getLocalBounds().toFloat().reduced(12).withTrimmedTop(20).withTrimmedBottom(16);}
    juce::RangedAudioParameter* parameter(int band,const char* field) const {return processor.state.getParameter(processor.eqId(band,field));}
    float value(int band,const char* field) const {auto* p=parameter(band,field);return p->convertFrom0to1(p->getValue());}
    void set(int band,const char* field,float v){auto* p=parameter(band,field);p->setValueNotifyingHost(p->convertTo0to1(v));}
    float xAt(float hz) const {return graph().getX()+graph().getWidth()*std::log(hz/20.0f)/std::log(1000.0f);}
    float yAt(float db) const {return graph().getY()+graph().getHeight()*(18.0f-db)/36.0f;}
    int bandAt(juce::Point<float> point) const{
        int result=-1;
        float best=15.0f;
        for(int band=0;band<7;++band){
            const auto distance=point.getDistanceFrom({xAt(value(band,"Frequency")),yAt(value(band,"Gain"))});
            if(distance<best){best=distance;result=band;}
        }
        return result;
    }
};
constexpr int controlTop=306;
struct Panel { const char* title; std::array<int,4> knobs; int count; };
constexpr Panel panels[]={
    {"01   VOICE",{4,5,6,-1},3},{"02   FILTER",{0,1,-1,-1},2},
    {"03   AMP ENVELOPE",{2,13,14,3},4},{"04   LFO",{10,11,-1,-1},2},
    {"05   FX / OUTPUT",{7,8,9,12},4}};
juce::Rectangle<int> panelBounds(int width,int height,int index){
    const int topHeight=(height-540)/2;
    const int row=index>=3?1:0;
    const int start=index==0||index==3?0:index==1?3:index==2?5:2;
    const int columns=row==0?9:6;
    const int available=width-58;
    const int left=29+available*start/columns;
    const int right=29+available*(start+panels[index].count)/columns;
    return {left+2,controlTop+row*topHeight,right-left-4,topHeight-5};
}
void drawInset(juce::Graphics& g,juce::Rectangle<float> bounds){
    g.setColour(juce::Colour(0xff222220));
    g.fillRoundedRectangle(bounds,4.0f);
    g.setColour(juce::Colour(0xff55504a));
    g.drawRoundedRectangle(bounds.reduced(.5f),4.0f,1.0f);
    g.setColour(juce::Colour(0xff242629));
    g.drawHorizontalLine(int(bounds.getBottom()-2),bounds.getX()+5,bounds.getRight()-5);
}

void drawSwitchBody(juce::Graphics& g, juce::Rectangle<float> r, bool down){
    g.setColour(juce::Colour(0xff090a0c));
    g.fillRoundedRectangle(r,3.0f);
    r=r.reduced(1.0f);
    g.setGradientFill(juce::ColourGradient(
        juce::Colour(down ? 0xff20252c : 0xff4a505a),0,r.getY(),
        juce::Colour(0xff15181e),0,r.getBottom(),false));
    g.fillRoundedRectangle(r,2.5f);
    g.setColour(juce::Colour(0xff69727e));
    g.drawHorizontalLine(juce::roundToInt(r.getY()),r.getX()+2,r.getRight()-2);
}
}
void SpaceInfinityEditor::HardwareLook::drawRotarySlider(
    juce::Graphics& g,int x,int y,int width,int height,float pos,float start,float end,juce::Slider& slider){
    auto bounds=juce::Rectangle<float>(float(x),float(y),float(width),float(height));
    const auto c=bounds.getCentre();
    const float r=juce::jmin(35.0f,juce::jmin(bounds.getWidth(),bounds.getHeight())*0.46f);
    if(r<4.0f)return;
    const auto radial=[c](float angle,float radius){
        return c.getPointOnCircumference(radius,angle);
    };
    for(int i=0;i<=18;++i){
        const float angle=start+(end-start)*float(i)/18.0f;
        g.setColour(juce::Colour(i%3==0 ? 0xffe0e2e5 : 0xff9299a2));
        g.drawLine({radial(angle,r*(i%3==0 ? 0.85f : 0.91f)),radial(angle,r)},1.2f);
    }
    const auto cap=juce::Rectangle<float>(r*1.53f,r*1.53f).withCentre(c);
    g.setColour(juce::Colour(0xb0000000));
    g.fillEllipse(cap.translated(0,3));
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff616977),c.x-r*.35f,c.y-r*.65f,
        juce::Colour(0xff090a0d),c.x+r*.42f,c.y+r*.60f,true));
    g.fillEllipse(cap);
    g.setColour(juce::Colour(0xff101216));
    g.drawEllipse(cap,1.3f);
    const auto legend=slider.getName();
    const bool metal=legend=="ATTACK" || legend=="RELEASE";
    if(metal){
        const auto insert=cap.reduced(r*.20f);
        juce::ColourGradient sheen(juce::Colour(0xff686f7a),insert.getTopLeft().x,insert.getTopLeft().y,
            juce::Colour(0xffaab0ba),insert.getBottomRight().x,insert.getBottomRight().y,false);
        sheen.addColour(0.43,juce::Colour(0xffe4e6e9));
        sheen.addColour(0.58,juce::Colour(0xff9299a4));
        g.setGradientFill(sheen);
        g.fillEllipse(insert);
    }
    const float angle=start+pos*(end-start);
    g.setColour(metal ? juce::Colour(0xff222833) : juce::Colour(0xfff4f6f7));
    g.drawLine({radial(angle,r*.42f),radial(angle,r*.69f)},2.1f);
}
void SpaceInfinityEditor::HardwareLook::drawButtonBackground(
    juce::Graphics& g,juce::Button& b,const juce::Colour&,bool hover,bool down){
    drawSwitchBody(g,b.getLocalBounds().toFloat(),down);
    if(hover || b.getToggleState()){
        g.setColour(juce::Colour(0xffff665f));
        g.fillEllipse(5.0f,float(b.getHeight())*.5f-2.0f,4.0f,4.0f);
    }
}
void SpaceInfinityEditor::HardwareLook::drawComboBox(
    juce::Graphics& g,int w,int h,bool down,int,int,int,int,juce::ComboBox& box){
    drawSwitchBody(g,box.getLocalBounds().toFloat(),down);
    juce::Path chevron;
    chevron.startNewSubPath(float(w)-18,float(h)*.5f-2);
    chevron.lineTo(float(w)-14,float(h)*.5f+2);
    chevron.lineTo(float(w)-10,float(h)*.5f-2);
    g.setColour(juce::Colour(0xffff8c80));
    g.strokePath(chevron,juce::PathStrokeType(1.4f));
}
SpaceInfinityEditor::SpaceInfinityEditor(SpaceInfinityProcessor& p):AudioProcessorEditor(&p),processor(p){
    setLookAndFeel(&hardwareLook);
    setResizable(true,true);setResizeLimits(800,760,1500,950);setSize(1000,790);
    title.setText("SPACE  INFINITY",juce::dontSendNotification);title.setFont(juce::Font(juce::FontOptions(30.0f,juce::Font::bold)));title.setColour(juce::Label::textColourId,juce::Colours::white);addAndMakeVisible(title);
    subtitle.setText("HYBRID HARDWARE-INSPIRED SYNTHESIS",juce::dontSendNotification);subtitle.setColour(juce::Label::textColourId,juce::Colour(0xffc0c0c8));addAndMakeVisible(subtitle);
    const auto& factoryNames=SpaceInfinityProcessor::presetNames();
    juce::StringArray banks;
    for(int i=0;i<factoryNames.size();++i)
        banks.addIfNotAlreadyThere(SpaceInfinityProcessor::presetCategory(i));
    for(const auto& bank:banks){
        juce::PopupMenu items;
        for(int i=0;i<factoryNames.size();++i)
            if(SpaceInfinityProcessor::presetCategory(i)==bank)
                items.addItem(i+1,factoryNames[i]);
        presets.getRootMenu()->addSubMenu(bank,items);
    }
    presets.setTextWhenNothingSelected("Select a sound");presets.onChange=[this]{processor.loadPreset(presets.getSelectedId()-1);};addAndMakeVisible(presets);
    const auto stepPreset=[this](int direction){
        const int count=SpaceInfinityProcessor::presetNames().size();
        const int current=presets.getSelectedId();
        const int next=current==0 ? (direction>0?1:count) : (current-1+direction+count)%count+1;
        presets.setSelectedId(next,juce::sendNotificationSync);
    };
    previousPreset.setName("Previous preset");
    nextPreset.setName("Next preset");
    previousPreset.setTooltip("Previous preset");
    nextPreset.setTooltip("Next preset");
    previousPreset.onClick=[stepPreset]{stepPreset(-1);};
    nextPreset.onClick=[stepPreset]{stepPreset(1);};
    addAndMakeVisible(previousPreset);
    addAndMakeVisible(nextPreset);
    addAndMakeVisible(eqButton);
    eqButton.setTooltip("Open the seven-band parametric EQ");
    eqButton.onClick=[this]{
        juce::CallOutBox::launchAsynchronously(std::make_unique<EqPanel>(processor),eqButton.getBounds(),this);
    };
    addAndMakeVisible(osc1);
    addAndMakeVisible(osc2);
    oscAttachments[0]=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.state,"osc1Enabled",osc1);
    oscAttachments[1]=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.state,"osc2Enabled",osc2);
    waveform.addItemList(SpaceInfinityProcessor::oscillatorWaves(),1);addAndMakeVisible(waveform);
    waveform.onChange=[this]{repaint();};
    waveAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.state,"wave",waveform);
    source.addItemList(SpaceInfinityProcessor::sourceNames(),1);
    addAndMakeVisible(source);
    sourceAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.state,"source",source);
    sampleRoot.setSliderStyle(juce::Slider::IncDecButtons);
    sampleRoot.setTextBoxStyle(juce::Slider::TextBoxLeft,false,68,24);
    sampleRoot.setTooltip("Root MIDI note for imported WAV");
    addAndMakeVisible(sampleRoot);
    rootAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,"sampleRoot",sampleRoot);
    addAndMakeVisible(importButton);
    importButton.onClick=[this]{
        sampleChooser=std::make_unique<juce::FileChooser>("Choose an audio sample",juce::File{},"*.wav");
        const juce::Component::SafePointer<SpaceInfinityEditor> editor(this);
        sampleChooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,
            [editor](const juce::FileChooser& chooser){
                if(editor==nullptr||chooser.getResult()==juce::File{})return;
                const auto result=editor->processor.importSample(chooser.getResult());
                if(result.failed())juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,"Sample import",result.getErrorMessage());
                else editor->importButton.setTooltip(chooser.getResult().getFileName());
            });
    };
    auto bindChoice=[this](juce::ComboBox& box,const char* id,const juce::StringArray& labels){
        box.addItemList(labels,1);
        addAndMakeVisible(box);
        fxChoices.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.state,id,box));
    };
    motionShape.addItemList(SpaceInfinityProcessor::motionNames(),1);
    motionShape.setTextWhenNothingSelected("Motion shapes");
    motionShape.onChange=[this]{processor.loadMotionShape(motionShape.getSelectedItemIndex());};
    addAndMakeVisible(motionShape);
    bindChoice(motionBars,"motionBars",{"1 bar","2 bars","4 bars","8 bars"});
    motionDepth.setSliderStyle(juce::Slider::LinearHorizontal);
    motionDepth.setTextBoxStyle(juce::Slider::TextBoxRight,false,43,20);
    motionDepth.setTooltip("Motion amount to filter cutoff");
    addAndMakeVisible(motionDepth);
    motionDepthAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,"motionDepth",motionDepth);
    motionDepth.textFromValueFunction=[](double value){return juce::String(value,2);};
    motionDepth.updateText();
    for(size_t i=0;i<motionPoints.size();++i){
        auto& point=motionPoints[i];
        point.setSliderStyle(juce::Slider::LinearVertical);
        point.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
        point.setTooltip("Motion point "+juce::String(int(i+1)));
        point.setColour(juce::Slider::thumbColourId,juce::Colour(0xffd78e3f));
        point.setColour(juce::Slider::trackColourId,juce::Colours::transparentBlack);
        point.setColour(juce::Slider::backgroundColourId,juce::Colours::transparentBlack);
        point.onValueChange=[this]{repaint();};
        point.setVisible(false);
        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,SpaceInfinityProcessor::motionPointId(int(i)),point));
    }
    const auto names=SpaceInfinityProcessor::macroNames();
    for(size_t i=0;i<macros.size();++i){
        const auto suffix=juce::String(int(i));
        auto& macro=macros[i];
        macro.setName(names[int(i)]);
        macro.setTooltip(names[int(i)]);
        macro.setSliderStyle(juce::Slider::LinearHorizontal);
        macro.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
        macro.setDoubleClickReturnValue(true,0.0);
        addAndMakeVisible(macro);
        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,"macro"+suffix,macro));
        macroRoutes[i].addItemList(SpaceInfinityProcessor::macroTargets(),1);
        addAndMakeVisible(macroRoutes[i]);
        routeDepth[i].setSliderStyle(juce::Slider::LinearHorizontal);
        routeDepth[i].setTextBoxStyle(juce::Slider::TextBoxRight,false,33,17);
        routeDepth[i].setTooltip("Signed route depth. Negative values reverse the modulation.");
        addAndMakeVisible(routeDepth[i]);
    }
    macroSlot.addItemList({"Macro route 1","Macro route 2","Macro route 3"},1);
    macroSlot.setSelectedId(1,juce::dontSendNotification);
    macroSlot.onChange=[this]{bindMacroPage();};
    addAndMakeVisible(macroSlot);
    bindMacroPage();
    bindChoice(waveform2,"osc2Wave",SpaceInfinityProcessor::oscillatorWaves(true));
    bindChoice(noiseType,"noiseType",{"White","Dark","Bright"});
    noiseLevel.setSliderStyle(juce::Slider::LinearHorizontal);
    noiseLevel.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
    noiseLevel.setTooltip("Noise level (zero turns noise off)");
    addAndMakeVisible(noiseLevel);
    attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,"noiseLevel",noiseLevel));
    const char* oscIds[]={"osc1Tune","osc1Width","osc2Tune","osc2Width"};
    for(size_t i=0;i<oscillatorSliders.size();++i){
        auto& slider=oscillatorSliders[i];
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::TextBoxRight,false,41,18);
        slider.setTooltip(i%2==0?"Fine tune in cents":"Stereo width");
        addAndMakeVisible(slider);
        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,oscIds[i],slider));
        slider.textFromValueFunction=[i](double value){return juce::String(value,i%2==0?1:2);};
        slider.updateText();
    }
    for(size_t i=0;i<unisonSliders.size();++i){
        auto& slider=unisonSliders[i];
        slider.setSliderStyle(juce::Slider::IncDecButtons);
        slider.setTextBoxStyle(juce::Slider::TextBoxLeft,false,24,18);
        slider.setTooltip("Unison voices (1–16)");
        addAndMakeVisible(slider);
        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.state,i==0?"osc1Unison":"osc2Unison",slider));
    }
    const char* octaveIds[]={"masterOctave","osc1Octave","osc2Octave"};
    const char* octaveNames[]={"Master octave","Osc 1 octave","Osc 2 octave"};
    for(size_t i=0;i<octaveControls.size();++i){
        auto& control=octaveControls[i];
        control.setSliderStyle(juce::Slider::IncDecButtons);
        control.setTextBoxStyle(juce::Slider::TextBoxLeft,false,23,17);
        control.setName(octaveNames[i]);
        control.setTooltip(octaveNames[i]);
        addAndMakeVisible(control);
        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.state,octaveIds[i],control));
    }
    bindChoice(voiceMode,"voiceMode",{"Poly","Mono","Legato"});
    bindChoice(filterMode,"filterMode",{"Classic LP","Resonant LP","High-pass","Band-pass","24 dB LP"});
    bindChoice(sampleLoop,"sampleLoop",{"Loop off","Loop on"});
    bindChoice(sampleReverse,"sampleReverse",{"Forward","Reverse"});
    const char* trimIds[]={"sampleStart","sampleEnd"};
    const char* trimNames[]={"Sample start","Sample end"};
    for(size_t i=0;i<sampleTrim.size();++i){
        auto& trim=sampleTrim[i];
        trim.setName(trimNames[i]);
        trim.setTooltip(trimNames[i]);
        trim.setSliderStyle(juce::Slider::LinearHorizontal);
        trim.setTextBoxStyle(juce::Slider::TextBoxRight,false,44,20);
        addAndMakeVisible(trim);
        trimAttachments[i]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,trimIds[i],trim);
        trim.textFromValueFunction=[](double value){return juce::String(value,2);};
        trim.updateText();
    }
    bindChoice(delaySync,"delaySync",{"Free delay","1/16","1/8","Dotted 1/8","1/4"});
    bindChoice(delayMode,"delayMode",{"Stereo","Ping-pong"});
    bindChoice(reverbSpace,"reverbSpace",{"Room","Plate","Hall"});
    compAmount.setSliderStyle(juce::Slider::LinearHorizontal);
    compAmount.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
    compAmount.setTooltip("Multiband compression amount");
    compAmount.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
    compAmount.setName("Multiband amount");
    addAndMakeVisible(compAmount);
    compAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,"compAmount",compAmount);
    pitchWheel.setSliderStyle(juce::Slider::LinearHorizontal);
    pitchWheel.setTextBoxStyle(juce::Slider::TextBoxRight,false,56,20);
    pitchWheel.setTextValueSuffix(" st");
    pitchWheel.setDoubleClickReturnValue(true,0.0);
    pitchWheel.setName("Pitch Wheel");
    pitchWheel.setTooltip("Pitch Wheel ±24 semitones. Double-click to centre.");
    addAndMakeVisible(pitchWheel);
    pitchAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,"pitchBend",pitchWheel);
    pitchWheel.textFromValueFunction=[](double value){return juce::String(value,1)+" st";};
    pitchWheel.updateText();
    const char* ids[]={"cutoff","res","attack","release","detune","mix","drive","chorus","delay","reverb","lforate","lfodepth","gain","ampDecay","ampSustain"};
    const int decimals[]={0,2,3,3,3,2,2,2,2,2,2,2,2,3,2};
    const char* units[]={" Hz",""," s"," s","","","","","",""," Hz","",""," s",""};
    const char* texts[]={"CUTOFF","RESONANCE","ATTACK","RELEASE","DETUNE","LAYER","DRIVE","CHORUS","DELAY","REVERB","LFO RATE","LFO DEPTH","OUTPUT","DECAY","SUSTAIN"};
    for(int i=0;i<15;i++){
        auto& slider=knobs[i];slider.setName(texts[i]);slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,70,19);
        slider.setColour(juce::Slider::rotarySliderFillColourId,juce::Colour(0xffe84953));slider.setColour(juce::Slider::thumbColourId,juce::Colours::white);
        slider.setColour(juce::Slider::textBoxTextColourId,juce::Colours::white);slider.setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
        addAndMakeVisible(slider);
        auto& label=labels[i];label.setText(texts[i],juce::dontSendNotification);label.setJustificationType(juce::Justification::centred);label.setColour(juce::Label::textColourId,juce::Colour(0xffc0c0ca));addAndMakeVisible(label);
        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,ids[i],slider));
        slider.setNumDecimalPlacesToDisplay(decimals[i]);
        const auto suffix=juce::String(units[i]);
        const int precision=decimals[i];
        slider.textFromValueFunction=[suffix,precision](double value){
            return juce::String(value,precision)+suffix;
        };
        slider.updateText();
        if(i==13){
            juce::NormalisableRange<double> travel(slider.getMinimum(),slider.getMaximum(),slider.getInterval());
            travel.setSkewForCentre(0.45);
            slider.setNormalisableRange(travel);
        }
        if(i==2 || i==3 || i==13 || i==14 || i==12){
            slider.setSliderStyle(juce::Slider::LinearVertical);
            slider.setColour(juce::Slider::thumbColourId,juce::Colour(0xffd48d42));
            slider.setColour(juce::Slider::trackColourId,juce::Colour(0xff10100f));
            slider.setColour(juce::Slider::backgroundColourId,juce::Colour(0xff4b4742));
        }
        slider.onValueChange=[this]{repaint();};
    }
    startTimerHz(30);
}
SpaceInfinityEditor::~SpaceInfinityEditor(){stopTimer();setLookAndFeel(nullptr);}
void SpaceInfinityEditor::timerCallback(){
    const float peak=processor.takeOutputPeak();
    meterPeak=juce::jmax(peak,meterPeak*0.88f);
    clipHold=peak>=1.0f?30:juce::jmax(0,clipHold-1);
    repaint(meterBounds);
}
void SpaceInfinityEditor::bindMacroPage(){
    const int slot=juce::jlimit(0,2,macroSlot.getSelectedId()-1);
    for(size_t i=0;i<macroRoutes.size();++i){
        routeAttachments[i].reset();
        depthAttachments[i].reset();
        routeAttachments[i]=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            processor.state,SpaceInfinityProcessor::macroRouteId(int(i),slot,false),macroRoutes[i]);
        depthAttachments[i]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.state,SpaceInfinityProcessor::macroRouteId(int(i),slot,true),routeDepth[i]);
        routeDepth[i].textFromValueFunction=[](double value){return juce::String(value,2);};
        routeDepth[i].updateText();
    }
}
juce::Rectangle<int> SpaceInfinityEditor::targetBounds(int target) const{
    const juce::Component* controls[]={nullptr,&knobs[0],&knobs[4],&knobs[9],&knobs[2],&knobs[6],
        &knobs[8],&knobs[7],&knobs[5],&knobs[3],&knobs[1],&noiseLevel,&oscillatorSliders[1],
        &oscillatorSliders[3],&knobs[12],&knobs[13],&knobs[14],&motionDepth,&knobs[10],&knobs[11]};
    if(target<=0||target>=int(std::size(controls)))return {};
    return controls[target]->getBounds();
}
int SpaceInfinityEditor::targetAt(juce::Point<int> cursor) const{
    for(int target=1;target<SpaceInfinityProcessor::macroTargets().size();++target)
        if(targetBounds(target).contains(cursor))return target;
    return 0;
}
void SpaceInfinityEditor::assignMacro(int macro,int target,bool inverted){
    int available=-1;
    for(int slot=0;slot<3;++slot){
        auto* route=processor.state.getParameter(SpaceInfinityProcessor::macroRouteId(macro,slot,false));
        const int destination=int(route->convertFrom0to1(route->getValue()));
        if(destination==target){available=slot;break;}
        if(destination==0&&available<0)available=slot;
    }
    if(available<0){
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,"Macro routes full",
            "Set an existing route to Off before adding another destination.");
        return;
    }
    auto* route=processor.state.getParameter(SpaceInfinityProcessor::macroRouteId(macro,available,false));
    auto* depth=processor.state.getParameter(SpaceInfinityProcessor::macroRouteId(macro,available,true));
    route->beginChangeGesture();
    route->setValueNotifyingHost(route->convertTo0to1(float(target)));
    route->endChangeGesture();
    depth->beginChangeGesture();
    depth->setValueNotifyingHost(depth->convertTo0to1(inverted?-0.5f:0.5f));
    depth->endChangeGesture();
    macroSlot.setSelectedId(available+1,juce::sendNotificationSync);
}
void SpaceInfinityEditor::mouseDown(const juce::MouseEvent& e){
    if(e.mods.isLeftButtonDown()){
        for(size_t i=0;i<macros.size();++i){
            const auto handle=macros[i].getBounds().translated(0,-13).withHeight(13);
            if(handle.contains(e.getPosition())){
                draggedMacro=int(i);
                repaint();
                return;
            }
        }
    }
    if(!e.mods.isLeftButtonDown()||!motionGraph.expanded(8).contains(e.getPosition()))return;
    drawMotionAt(e.getPosition());
}
void SpaceInfinityEditor::mouseDrag(const juce::MouseEvent& e){
    if(draggedMacro>=0){
        hoverTarget=targetAt(e.getPosition());
        repaint();
    }else if(lastMotionPoint>=0)drawMotionAt(e.getPosition());
}
void SpaceInfinityEditor::mouseUp(const juce::MouseEvent& e){
    if(draggedMacro>=0){
        const int target=targetAt(e.getPosition());
        if(target>0)assignMacro(draggedMacro,target,e.mods.isShiftDown());
        draggedMacro=-1;
        hoverTarget=0;
        repaint();
        return;
    }
    for(size_t i=0;i<motionGestures.size();++i){
        if(motionGestures[i]){
            processor.state.getParameter(SpaceInfinityProcessor::motionPointId(int(i)))->endChangeGesture();
            motionGestures[i]=false;
        }
    }
    lastMotionPoint=-1;
}
void SpaceInfinityEditor::drawMotionAt(juce::Point<int> cursor){
    const auto index=juce::jlimit(0,16,juce::roundToInt(16.0f*float(cursor.x-motionGraph.getX())/float(motionGraph.getWidth())));
    const auto value=juce::jlimit(0.0f,1.0f,float(motionGraph.getBottom()-cursor.y)/float(motionGraph.getHeight()));
    const int from=lastMotionPoint<0?index:lastMotionPoint;
    for(int i=std::min(from,index);i<=std::max(from,index);++i){
        auto* parameter=processor.state.getParameter(SpaceInfinityProcessor::motionPointId(i));
        if(!motionGestures[size_t(i)]){
            parameter->beginChangeGesture();
            motionGestures[size_t(i)]=true;
        }
        const float point=from==index?value:lastMotionValue+(value-lastMotionValue)*float(i-from)/float(index-from);
        parameter->setValueNotifyingHost(point);
    }
    lastMotionPoint=index;
    lastMotionValue=value;
}
void SpaceInfinityEditor::paint(juce::Graphics& g){
    g.fillAll(juce::Colour(0xff21100c));
    for(const int side:{0,getWidth()-22}){
        const juce::Rectangle<float> cheek(float(side),0.0f,22.0f,float(getHeight()));
        juce::ColourGradient wood(juce::Colour(0xff38150d),cheek.getX(),0.0f,
                                  juce::Colour(0xff512419),cheek.getRight(),0.0f,false);
        wood.addColour(0.42,juce::Colour(0xffa65e37));
        wood.addColour(0.72,juce::Colour(0xff78371f));
        g.setGradientFill(wood);
        g.fillRect(cheek);
        for(int grain=0;grain<24;++grain){
            juce::Path fibre;
            const float x=cheek.getX()+float(grain)*0.92f;
            fibre.startNewSubPath(x,0.0f);
            for(int y=12;y<getHeight()+12;y+=12)
                fibre.lineTo(x+0.42f*std::sin(float(y)*0.034f+float(grain)),float(y));
            g.setColour(juce::Colour(grain%3==0?0x378f4629:0x2b301309));
            g.strokePath(fibre,juce::PathStrokeType(0.65f));
        }
    }
    const auto chassis=getLocalBounds().toFloat().reduced(19.0f,4.0f);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xffa02c24),0,chassis.getY(),
        juce::Colour(0xff5d1818),0,chassis.getBottom(),false));
    g.fillRoundedRectangle(chassis,4.0f);
    g.setColour(juce::Colour(0xff53565b));
    g.drawRoundedRectangle(chassis,4.0f,1.0f);
    auto rail=chassis.reduced(8.0f).withHeight(85.0f);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff9c2c25),0,rail.getY(),
        juce::Colour(0xff1a1c20),0,rail.getBottom(),false));
    g.fillRoundedRectangle(rail,3.0f);
    g.setColour(juce::Colour(0xffbd5446));
    g.drawRoundedRectangle(rail,3.0f,1.0f);
    const auto tray=juce::Rectangle<float>(23,112,float(getWidth()-46),float(getHeight()-156));
    g.setColour(juce::Colour(0xff292827));
    g.fillRoundedRectangle(tray,3.0f);
    g.setColour(juce::Colour(0xff111215));
    g.drawRoundedRectangle(tray,3.0f,1.0f);
    g.setColour(juce::Colour(0x035e6469));
    for(int y=114;y<int(tray.getBottom())-2;y+=3)
        g.drawHorizontalLine(y,tray.getX()+2,tray.getRight()-2);
    for(const float x:{rail.getX()+7,rail.getRight()-7}){
        const float y=rail.getY()+7;
        g.setColour(juce::Colour(0xff15161a));
        g.fillEllipse(x-4,y-4,8,8);
        g.setColour(juce::Colour(0xff777a80));
        g.fillEllipse(x-3,y-3,6,6);
        g.setColour(juce::Colour(0xff25272c));
        g.drawLine(x-2,y+2,x+2,y-2,1.0f);
    }
    g.setColour(juce::Colour(0xffc3c1bd));
    g.setFont(12.0f);
    drawInset(g,juce::Rectangle<float>(26.0f,float(getHeight()-69),float(getWidth()-52),60.0f));
    g.setColour(juce::Colour(0xffc3c1bd));
    g.drawText("SAMPLER",getWidth()-132,getHeight()-65,95,22,juce::Justification::centredRight);
    g.drawText("TRIM START",34,getHeight()-65,66,22,juce::Justification::centredLeft);
    g.drawText("END",245,getHeight()-65,35,22,juce::Justification::centredLeft);
    g.drawText("MASTER OCT",getWidth()-466,116,77,22,juce::Justification::right);
    g.drawText("PITCH",getWidth()-316,116,51,22,juce::Justification::right);
    const auto motionPanel=juce::Rectangle<int>(26,getHeight()-243,getWidth()-52,119);
    g.setColour(juce::Colour(0xff0b0b0d));
    g.fillRoundedRectangle(motionPanel.toFloat(),5.0f);
    g.setColour(juce::Colour(0xff4d4243));
    g.drawRoundedRectangle(motionPanel.toFloat(),5.0f,1.0f);
    g.setColour(juce::Colour(0xffd9a6a1));
    g.drawText("MOTION  /  FILTER CUTOFF",34,motionPanel.getY()+2,155,16,juce::Justification::centredLeft);
    juce::Path motionTrace;
    std::array<juce::Point<float>,17> handles;
    for(size_t i=0;i<motionPoints.size();++i){
        const float x=float(motionGraph.getX())+float(motionGraph.getWidth())*float(i)/16.0f;
        const float y=float(motionGraph.getBottom())-float(motionPoints[i].getValue())*float(motionGraph.getHeight());
        handles[i]={x,y};
        g.setColour(juce::Colour(0xff342123));
        g.drawVerticalLine(int(x),float(motionGraph.getY()),float(motionGraph.getBottom()));
        if(i==0)motionTrace.startNewSubPath(x,y);else motionTrace.lineTo(x,y);
    }
    g.setColour(juce::Colour(0xffe6a14e));
    g.strokePath(motionTrace,juce::PathStrokeType(1.5f));
    for(const auto point:handles)g.fillEllipse(point.x-2.5f,point.y-2.5f,5.0f,5.0f);
    g.setColour(juce::Colour(0xffc3c1bd));
    const auto macroNames=SpaceInfinityProcessor::macroNames();
    for(size_t i=0;i<macros.size();++i){
        const auto slider=macros[i].getBounds();
        const auto handle=slider.translated(0,-13).withHeight(13);
        g.setColour(juce::Colour(draggedMacro==int(i)?0xff8c402b:0xff33302e));
        g.fillRoundedRectangle(handle.toFloat(),2.0f);
        g.setColour(juce::Colour(0xffe1c9ae));
        g.drawText(macroNames[int(i)]+"  +",handle,juce::Justification::centred);
    }
    g.drawText("SIGNAL PATH",34,116,350,20,juce::Justification::left);
    drawInset(g,juce::Rectangle<float>(float(getWidth()-344),22.0f,318.0f,81.0f));
    g.setFont(10.0f);
    g.setColour(juce::Colour(0xffb5a6a0));
    g.drawText("NOISE",getWidth()-330,72,60,23,juce::Justification::centredLeft);
    g.drawText("FACTORY PROGRAM",getWidth()-330,25,110,13,juce::Justification::centredLeft);
    for(int i=0;i<int(std::size(panels));++i){
        const auto panel=panelBounds(getWidth(),getHeight(),i);
        drawInset(g,panel.toFloat());
        g.setColour(juce::Colour(0xffb7b1a8));
        g.drawText(panels[i].title,panel.reduced(8,2).withHeight(16),juce::Justification::centredLeft);
    }
    const float db=juce::Decibels::gainToDecibels(meterPeak,-60.0f);
    for(int segment=0;segment<12;++segment){
        const int y=meterBounds.getBottom()-(segment+1)*meterBounds.getHeight()/12;
        const bool lit=segment==11?clipHold>0:db>=-54.0f+4.8f*float(segment);
        const auto colour=juce::Colour(segment==11?0xfff05243:segment>=8?0xffd4a33d:0xff5dbb68);
        g.setColour(lit?colour:colour.darker(3.0f));
        g.fillRect(meterBounds.getX(),y,meterBounds.getWidth(),juce::jmax(1,meterBounds.getHeight()/12-1));
    }
    for(int i=0;i<2;++i){
        auto strip=juce::Rectangle<int>(30+i*(getWidth()-60)/2,263,(getWidth()-60)/2,40).reduced(2,0);
        drawInset(g,strip.toFloat());
        g.setColour(juce::Colour(0xffc8bcb0));
        g.drawText("WAVE",strip.getX()+65,264,42,14,juce::Justification::centredLeft);
        g.drawText("FINE TUNE",oscillatorSliders[size_t(i*2)].getX(),264,75,14,juce::Justification::centredLeft);
        g.drawText("OCT",octaveControls[size_t(i+1)].getX()-27,264,25,14,juce::Justification::right);
        g.drawText("UNISON",unisonSliders[size_t(i)].getX(),264,55,14,juce::Justification::centredLeft);
        g.drawText("WIDTH",oscillatorSliders[size_t(i*2+1)].getX(),264,65,14,juce::Justification::centredLeft);
    }
    g.drawText("COMP",compAmount.getX(),compAmount.getY()-13,compAmount.getWidth(),13,juce::Justification::centred);
    g.setFont(12.0f);
    if(hoverTarget>0){
        g.setColour(juce::Colour(0xffe9a357));
        g.drawRoundedRectangle(targetBounds(hoverTarget).toFloat(),3.0f,2.0f);
    }
    const int displayWidth=(getWidth()-84)/3;
    const char* headings[]={"OSCILLATOR","AMP RESPONSE","LFO"};
    for(int i=0;i<3;++i){
        const auto frame=juce::Rectangle<int>(34+i*(displayWidth+8),148,displayWidth,102);
        g.setColour(juce::Colour(0xff050608));
        g.fillRoundedRectangle(frame.toFloat(),5.0f);
        g.setColour(juce::Colour(0xff55565b));
        g.drawRoundedRectangle(frame.toFloat(),5.0f,1.0f);
        g.setColour(juce::Colour(0xffbd9b95));
        g.drawText(headings[i],frame.reduced(10,4).withHeight(18),juce::Justification::centredLeft);
        const auto area=frame.reduced(10,8).withTrimmedTop(19).toFloat();
        g.setColour(juce::Colour(0xff2a1d20));
        for(int line=1;line<8;++line)
            g.drawVerticalLine(int(area.getX()+area.getWidth()*float(line)/8.0f),area.getY(),area.getBottom());
        g.drawHorizontalLine(int(area.getCentreY()),area.getX(),area.getRight());
        juce::Path trace;
        for(int k=0;k<=128;++k){
            const float x=float(k)/128.0f;
            float y=0.5f;
            if(i==0){
                const float phase=x*2.0f-std::floor(x*2.0f);
                y-=0.43f*SpaceInfinityProcessor::oscillatorSample(phase,juce::jmax(0,waveform.getSelectedItemIndex()));
            }else if(i==1){
                const float t=x*4.0f;
                const float attack=float(knobs[2].getValue());
                const float release=float(knobs[3].getValue());
                const float held=juce::jmin(t,2.0f);
                const float sustain=float(knobs[14].getValue());
                const float decay=float(knobs[13].getValue());
                const float level=sustain>=1.0f && attack==0.0f ? 1.0f : sustain>=1.0f ? 1.0f-std::exp(-held/juce::jmax(attack,0.002f))
                    : juce::jlimit(sustain,1.0f,1.0f-juce::jmax(0.0f,held-attack)/juce::jmax(decay,0.005f))
                        *juce::jlimit(0.0f,1.0f,held/juce::jmax(attack,0.002f));
                const float tail=t<=2.0f?1.0f:release==0.0f?0.0f:std::exp(-(t-2.0f)/release);
                y=0.94f-0.86f*level*tail;
            }else{
                const float cycles=float(knobs[10].getValue())*0.5f;
                y-=0.44f*float(knobs[11].getValue())*std::sin(juce::MathConstants<float>::twoPi*cycles*x);
            }
            const auto point=juce::Point<float>(area.getX()+x*area.getWidth(),area.getY()+y*area.getHeight());
            if(k==0)trace.startNewSubPath(point);else trace.lineTo(point);
        }
        g.setColour(juce::Colour(0xffdf9a53));
        g.strokePath(trace,juce::PathStrokeType(1.5f));
    }
}
void SpaceInfinityEditor::resized(){
    const int w=getWidth();title.setBounds(34,23,300,40);subtitle.setBounds(35,64,450,24);
    presets.setBounds(w-330,42,226,25);
    previousPreset.setBounds(w-100,42,29,25);
    nextPreset.setBounds(w-67,42,29,25);
    noiseType.setBounds(w-266,72,90,23);
    noiseLevel.setBounds(w-172,72,134,23);
    auto oscillatorRow=juce::Rectangle<int>(30,265,w-60,36);
    for(int i=0;i<2;++i){
        auto strip=oscillatorRow.removeFromLeft((w-60)/2).reduced(5,0);
        auto& toggle=i==0?osc1:osc2;
        toggle.setBounds(strip.getX(),strip.getY()-4,60,18);
        octaveControls[size_t(i+1)].setBounds(strip.getRight()-123,strip.getY()-4,62,17);
        strip.removeFromTop(14);
        unisonSliders[size_t(i)].setBounds(strip.removeFromRight(62).reduced(2,0));
        auto& selector=i==0?waveform:waveform2;
        selector.setBounds(strip.removeFromLeft(strip.getWidth()/3).reduced(2,0));
        oscillatorSliders[size_t(i*2)].setBounds(strip.removeFromLeft(strip.getWidth()/2).reduced(2,0));
        oscillatorSliders[size_t(i*2+1)].setBounds(strip.reduced(2,0));
    }
    source.setBounds(34,getHeight()-37,152,24);
    importButton.setBounds(194,getHeight()-37,104,24);
    macroSlot.setBounds(w-176,getHeight()-37,140,24);
    sampleRoot.setBounds(306,getHeight()-37,116,24);
    auto fx=panelBounds(w,getHeight(),4).reduced(8,5).removeFromBottom(24);
    const int fxWidth=fx.getWidth()/4;
    delaySync.setBounds(fx.removeFromLeft(fxWidth).reduced(2,0));
    delayMode.setBounds(fx.removeFromLeft(fxWidth).reduced(2,0));
    reverbSpace.setBounds(fx.removeFromLeft(fxWidth).reduced(2,0));
    eqButton.setBounds(fx.removeFromRight(36).reduced(2,0));
    compAmount.setBounds(fx.reduced(2,0));
    auto voice=panelBounds(w,getHeight(),0).reduced(8,5);
    voiceMode.setBounds(voice.removeFromBottom(21).withWidth(108));
    auto filter=panelBounds(w,getHeight(),1).reduced(8,5);
    filterMode.setBounds(filter.removeFromBottom(21));
    octaveControls[0].setBounds(w-385,116,64,22);
    pitchWheel.setBounds(w-261,116,220,22);
    auto motionRow=juce::Rectangle<int>(34,getHeight()-227,w-68,98);
    auto controls=motionRow.removeFromLeft(155);
    motionShape.setBounds(controls.removeFromTop(26).reduced(2));
    motionBars.setBounds(controls.removeFromTop(26).reduced(2));
    motionDepth.setBounds(controls.removeFromTop(26).reduced(2));
    motionGraph=motionRow.reduced(12,13);
    auto macroRow=juce::Rectangle<int>(34,getHeight()-108,w-68,36);
    const int macroWidth=macroRow.getWidth()/int(macros.size());
    for(size_t i=0;i<macros.size();++i){
        auto cell=macroRow.removeFromLeft(macroWidth).reduced(4,0);
        macros[i].setBounds(cell.removeFromTop(16));
        routeDepth[i].setBounds(cell.removeFromRight(56).reduced(2,0));
        macroRoutes[i].setBounds(cell);
    }
    sampleTrim[0].setBounds(100,getHeight()-65,130,22);
    sampleTrim[1].setBounds(282,getHeight()-65,130,22);
    sampleLoop.setBounds(425,getHeight()-65,100,22);
    sampleReverse.setBounds(532,getHeight()-65,100,22);
    for(int i=0;i<int(std::size(panels));++i){
        const auto& panel=panels[i];
        auto area=panelBounds(w,getHeight(),i).reduced(3,3).withTrimmedTop(17);
        if(i==0 || i==1 || i==4) area.removeFromBottom(28);
        for(int j=0;j<panel.count;++j){
            auto cell=area.removeFromLeft(area.getWidth()/(panel.count-j)).reduced(2,0);
            const auto index=size_t(panel.knobs[j]);
            labels[index].setBounds(cell.removeFromTop(17));
            if(index==12)meterBounds=cell.removeFromRight(10).reduced(1,2);
            knobs[index].setBounds(cell);
        }
    }
}
juce::AudioProcessorEditor* SpaceInfinityProcessor::createEditor(){return new SpaceInfinityEditor(*this);}
