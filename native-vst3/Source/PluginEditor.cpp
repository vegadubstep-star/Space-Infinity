#include "PluginEditor.h"
#include "EffectControls.h"
#include <array>
#include <cmath>
#include <complex>
class ModulationBadge final : public juce::Component, public juce::SettableTooltipClient {
public:
    ~ModulationBadge() override {if(dragging)dragging->endChangeGesture();}
    std::function<std::pair<juce::RangedAudioParameter*,juce::RangedAudioParameter*>(bool)> getRoute;
    std::function<juce::Colour()> sourceColour;
    std::function<void()> changed;
    void paint(juce::Graphics& g) override {
        const auto route=getRoute(false);
        if(!route.first)return;
        const float amount=route.first->convertFrom0to1(route.first->getValue());
        auto circle=getLocalBounds().toFloat().reduced(2);
        g.setColour(juce::Colour(0xff141a1a));g.fillEllipse(circle);
        g.setColour(juce::Colour(0xff626c6b));g.drawEllipse(circle,1);
        auto colour=sourceColour();
        if(amount<0)colour=colour.brighter(.5f);
        g.setColour(colour);
        juce::Path arc;
        arc.addCentredArc(circle.getCentreX(),circle.getCentreY(),circle.getWidth()*.5f,circle.getHeight()*.5f,
            0.0f,-2.5f,-2.5f+5.0f*std::abs(amount),true);
        g.strokePath(arc,juce::PathStrokeType(2));
        g.drawText(amount<0?"-":"+",getLocalBounds(),juce::Justification::centred);
    }
    void mouseDown(const juce::MouseEvent& event) override {
        const auto route=getRoute(true);
        if(!route.first)return;
        if(event.mods.isPopupMenu()){
            juce::PopupMenu menu;
            menu.addItem(1,"Invert amount");
            menu.addItem(2,"Set amount to zero");
            menu.addItem(3,"Remove assignment");
            const juce::Component::SafePointer<ModulationBadge> safe(this);
            menu.showMenuAsync(juce::PopupMenu::Options{}.withTargetComponent(this),[safe,route](int action){
                if(safe==nullptr||action==0)return;
                auto* parameter=action==3?route.second:route.first;
                const float value=action==1?-route.first->convertFrom0to1(route.first->getValue()):0.0f;
                parameter->beginChangeGesture();
                parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
                parameter->endChangeGesture();
                safe->changed();
            });
            return;
        }
        dragging=route.first;
        initial=dragging->convertFrom0to1(dragging->getValue());
        dragging->beginChangeGesture();
        setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
    }
    void mouseDrag(const juce::MouseEvent& event) override {
        if(!dragging)return;
        const float sensitivity=event.mods.isShiftDown()?450.0f:140.0f;
        const float value=juce::jlimit(-1.0f,1.0f,initial-float(event.getDistanceFromDragStartY())/sensitivity);
        dragging->setValueNotifyingHost(dragging->convertTo0to1(value));
        setTooltip("Modulation depth "+juce::String(value*100.0f,0)+"%");
        changed();
    }
    void mouseUp(const juce::MouseEvent&) override {
        if(dragging)dragging->endChangeGesture();
        dragging=nullptr;
        setMouseCursor(juce::MouseCursor::NormalCursor);
    }
private:
    juce::RangedAudioParameter* dragging=nullptr;
    float initial=0.0f;
};


namespace {
void showPercent(juce::Slider& slider,bool signedValue=false){
    slider.textFromValueFunction=[signedValue](double value){
        return juce::String(signedValue&&value>0.0?"+":"")+juce::String(value*100.0,0)+"%";
    };
    slider.valueFromTextFunction=[](const juce::String& text){return text.getDoubleValue()/100.0;};
    slider.updateText();
}

class WarpPanel final : public juce::Component {
public:
    WarpPanel(SpaceInfinityProcessor& processor,int oscillator)
        : modeBinding(*processor.state.getParameter("osc"+juce::String(oscillator)+"Warp"),mode,nullptr),
          amountBinding(processor.state,"osc"+juce::String(oscillator)+"WarpAmount",amount){
        mode.addItemList({"Off",oscillator==1?"FM from B":"FM from A","Mirror","PWM"},1);
        mode.setSelectedItemIndex(int(processor.state.getRawParameterValue("osc"+juce::String(oscillator)+"Warp")->load()),juce::dontSendNotification);
        amount.setTextBoxStyle(juce::Slider::TextBoxRight,false,48,22);
        amount.setTooltip("Warp amount. Double-click to reset.");
        amount.setDoubleClickReturnValue(true,0.0);
        showPercent(amount);
        addAndMakeVisible(mode);
        addAndMakeVisible(amount);
        setSize(240,82);
    }
    void resized() override {mode.setBounds(10,9,220,26);amount.setBounds(10,43,220,25);}
private:
    juce::ComboBox mode;
    juce::Slider amount;
    juce::ComboBoxParameterAttachment modeBinding;
    juce::AudioProcessorValueTreeState::SliderAttachment amountBinding;
};
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

struct MainLayout {
    juce::Rectangle<int> oscillators[2], filter, macros, envelope, motion;
};
MainLayout mainLayout(int width,int height){
    const int margin=28,gap=8;
    const int total=width-margin*2;
    const int topHeight=juce::jlimit(238,335,int(float(height)*0.35f));
    const int oscillatorWidth=total*35/100;
    const int top=146;
    MainLayout result;
    result.oscillators[0]={margin,top,oscillatorWidth-gap,topHeight};
    result.oscillators[1]={margin+oscillatorWidth,top,oscillatorWidth-gap,topHeight};
    result.filter={margin+oscillatorWidth*2,top,total-oscillatorWidth*2,topHeight};
    const int lower=top+topHeight+gap;
    const int lowerHeight=height-80-lower;
    const int macroWidth=juce::jlimit(132,180,total/5);
    const int envelopeWidth=juce::jlimit(200,290,total*29/100);
    result.macros={margin,lower,macroWidth,lowerHeight};
    result.envelope={margin+macroWidth+gap,lower,envelopeWidth,lowerHeight};
    result.motion={result.envelope.getRight()+gap,lower,
        width-margin-result.envelope.getRight()-gap,lowerHeight};
    return result;
}
juce::Rectangle<int> effectRowBounds(int width,int height,int row){
    const int rowHeight=(height-216)/6;
    return {28,150+row*rowHeight,width-56,rowHeight-5};
}

constexpr int controlTop=328;
struct Panel { const char* title; std::array<int,6> knobs; int count; };
constexpr Panel panels[]={
    {"01   VOICE",{4,5,6,-1},3},{"02   FILTER",{0,1,-1,-1},2},
    {"03   AMP ENVELOPE",{2,13,14,3},4},{"04   LFO",{10,11,-1,-1},2},
    {"05   FX / OUTPUT",{7,15,16,8,9,12},6}};
juce::Rectangle<int> panelBounds(int width,int height,int index){
    const int topHeight=(height-562)/2;
    const int row=index>=3?1:0;
    const int start=index==0||index==3?0:index==1?3:index==2?5:2;
    const int columns=row==0?9:8;
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
    const auto& properties=slider.getProperties();
    if(properties.contains("modDepth")){
        const float depth=float(double(properties["modDepth"]));
        const float first=start+pos*(end-start);
        const float last=start+juce::jlimit(0.0f,1.0f,pos+depth*.5f)*(end-start);
        juce::Path halo;
        halo.addCentredArc(c.x,c.y,r*.98f,r*.98f,0.0f,std::min(first,last),std::max(first,last),true);
        g.setColour(juce::Colour(juce::uint32(juce::int64(properties["modColour"]))));
        g.strokePath(halo,juce::PathStrokeType(2.3f));
    }
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
    subtitle.setText("DRAG A SOURCE TO A CONTROL. DRAG ITS BADGE FOR DEPTH.",juce::dontSendNotification);subtitle.setColour(juce::Label::textColourId,juce::Colour(0xffc0c0c8));addAndMakeVisible(subtitle);
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
    addAndMakeVisible(warpA);
    addAndMakeVisible(warpB);
    warpA.onClick=[this]{juce::CallOutBox::launchAsynchronously(std::make_unique<WarpPanel>(processor,1),warpA.getBounds(),this);};
    warpB.onClick=[this]{juce::CallOutBox::launchAsynchronously(std::make_unique<WarpPanel>(processor,2),warpB.getBounds(),this);};
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
    motionShape.setTextWhenNothingSelected("Choose a shape");
    motionShape.setTooltip("Load a starting shape, then drag across the graph to edit it.");
    motionShape.onChange=[this]{processor.loadLfoShape(selectedLfo,motionShape.getSelectedId()-1);};
    addAndMakeVisible(motionShape);
    const char* lengths[]={"1 bar","2 bars","4 bars","8 bars","1/2 bar","1/4 bar","1/8 bar","1/16 bar"};
    for(const int choice:{7,6,5,4,0,1,2,3})motionBars.addItem(lengths[choice],choice+1);
    addAndMakeVisible(motionBars);
    lfoSync.addItemList({"Hz","BPM"},1);
    lfoSync.onChange=[this]{updateLfoRateVisibility();};
    lfoMode.addItemList({"Free","Retrigger","One-shot"},1);
    addAndMakeVisible(lfoSync);
    addAndMakeVisible(lfoMode);
    lfoSpeed.setSliderStyle(juce::Slider::LinearHorizontal);
    lfoSpeed.setTextBoxStyle(juce::Slider::TextBoxRight,false,59,21);
    addAndMakeVisible(lfoSpeed);
    motionDepth.setSliderStyle(juce::Slider::LinearHorizontal);
    motionDepth.setTextBoxStyle(juce::Slider::TextBoxRight,false,43,20);
    motionDepth.setTooltip("Motion amount to filter cutoff");
    addAndMakeVisible(motionDepth);
    motionDepthAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,"motionDepth",motionDepth);
    motionDepth.setDoubleClickReturnValue(true,0.0);
    showPercent(motionDepth);
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
        lfoPointBindings[i]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,SpaceInfinityProcessor::lfoPointId(0,int(i)),point);
    }
    const auto names=SpaceInfinityProcessor::macroNames();
    for(size_t i=0;i<macros.size();++i){
        const auto suffix=juce::String(int(i));
        auto& macro=macros[i];
        macro.setName(names[int(i)]);
        macro.setTooltip(names[int(i)]);
        macro.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        macro.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
        macro.setDoubleClickReturnValue(true,0.0);
        macro.onDragStart=[this,i]{selectedSource=int(i);refreshModulationIndicators();repaint();};
        addAndMakeVisible(macro);
        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,"macro"+suffix,macro));

    }
    bindLfoControls();
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
        slider.setDoubleClickReturnValue(true,0.0);
        if(i%2!=0)showPercent(slider);
        else{
            slider.textFromValueFunction=[](double value){return juce::String(value,1)+" ct";};
            slider.valueFromTextFunction=[](const juce::String& text){return text.getDoubleValue();};
            slider.updateText();
        }
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
    bindChoice(filterMode,"filterMode",{"Classic LP","Resonant LP","High-pass","Band-pass","24 dB LP","Analogue ZDF"});
    filterMode.setTooltip("Analogue ZDF is a four-pole low-pass with nonlinear, zero-delay resonance feedback.");
    analogueCharacter.setSliderStyle(juce::Slider::LinearHorizontal);
    analogueCharacter.setTextBoxStyle(juce::Slider::TextBoxRight,false,46,20);
    analogueCharacter.setDoubleClickReturnValue(true,0.0);
    analogueCharacter.setTooltip("Analogue character: gentle drift, soft saturation and smoother oscillator edges.");
    analogueCharacter.setName("Analogue character");
    addAndMakeVisible(analogueCharacter);
    attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.state,"analogueCharacter",analogueCharacter));
    showPercent(analogueCharacter);
    analogueLabel.setText("ANALOGUE CHARACTER",juce::dontSendNotification);
    analogueLabel.setJustificationType(juce::Justification::centred);
    analogueLabel.setColour(juce::Label::textColourId,juce::Colour(0xffc0c8b0));
    addAndMakeVisible(analogueLabel);
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
    pitchWheel.setDoubleClickReturnValue(true,0.0);
    pitchWheel.setName("Pitch Wheel");
    pitchWheel.setTooltip("Pitch Wheel ±24 semitones. Double-click to centre.");
    addAndMakeVisible(pitchWheel);
    pitchAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,"pitchBend",pitchWheel);
    pitchWheel.textFromValueFunction=[](double value){return juce::String(value,1)+" st";};
    pitchWheel.updateText();
    const char* ids[]={"cutoff","res","attack","release","detune","mix","drive","chorus","delay","reverb","lforate","lfodepth","gain","ampDecay","ampSustain","flanger","phaser"};
    const int decimals[]={0,2,3,3,3,2,2,2,2,2,2,2,2,3,2,2,2};
    const char* units[]={" Hz",""," s"," s","","","","","",""," Hz","",""," s","","",""};
    const char* texts[]={"CUTOFF","RESONANCE","ATTACK","RELEASE","DETUNE","LAYER","DRIVE","CHORUS","DELAY","REVERB","LFO RATE","LFO DEPTH","OUTPUT","DECAY","SUSTAIN","FLANGER","PHASER"};
    for(int i=0;i<17;i++){
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
        const auto* parameter=processor.state.getParameter(ids[i]);
        slider.setDoubleClickReturnValue(true,parameter->convertFrom0to1(parameter->getDefaultValue()));
        if(i==1||i==5||i==6||i==7||i==8||i==9||i==11||i==12||i==14||i==15||i==16)
            showPercent(slider);
        if(i==2||i==3||i==13||i==14)
            slider.onDragStart=[this]{processor.state.getParameter("ampTimed")->setValueNotifyingHost(1.0f);};
        if(i==0 || i==13){
            juce::NormalisableRange<double> travel(slider.getMinimum(),slider.getMaximum(),slider.getInterval());
            travel.setSkewForCentre(i==0?1000.0:0.45);
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
    for(size_t i=0;i<effectControls.size();++i){
        const auto& definition=effectControls[i];
        auto& control=rackControls[i];
        control.setName(definition.label);
        control.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        control.setTextBoxStyle(juce::Slider::TextBoxBelow,false,74,18);
        control.setTooltip(juce::String(effectRows[size_t(definition.row)])+" "+definition.label);
        addAndMakeVisible(control);
        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.state,definition.id,control));
        const auto* parameter=processor.state.getParameter(definition.id);
        control.setDoubleClickReturnValue(true,parameter->convertFrom0to1(parameter->getDefaultValue()));
        if(juce::String(definition.unit)=="%")showPercent(control,definition.minimum<0);
        else{
            const auto unit=juce::String(definition.unit);
            control.textFromValueFunction=[unit](double value){
                return juce::String(value,unit=="Hz"?0:unit=="s"?3:1)+" "+unit;
            };
            control.valueFromTextFunction=[](const juce::String& text){return text.getDoubleValue();};
            control.updateText();
        }
        if(definition.maximum>100){
            auto range=juce::NormalisableRange<double>(control.getMinimum(),control.getMaximum());
            range.setSkewForCentre(juce::String(definition.unit)=="Hz"?1000.0:
                juce::jlimit(control.getMinimum()+0.001,control.getMaximum()-0.001,double(definition.initial)));
            control.setNormalisableRange(range);
        }
        auto& label=rackLabels[i];
        label.setText(definition.label,juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId,juce::Colour(0xffd1c5b6));
        addAndMakeVisible(label);
    }
    for(size_t row=0;row<rackEnabled.size();++row){
        rackEnabled[row].setButtonText(effectRows[row]);
        rackEnabled[row].setTooltip("Enable or bypass "+juce::String(effectRows[row]));
        addAndMakeVisible(rackEnabled[row]);
        rackPower[row]=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            processor.state,effectEnableIds[row],rackEnabled[row]);
    }
    eqButton.setButtonText("OPEN 7-BAND EQ");
    for(auto* child:getChildren())
        if(child->isVisible()&&child!=&title&&child!=&subtitle&&child!=&presets
           &&child!=&previousPreset&&child!=&nextPreset&&child!=&noiseType&&child!=&noiseLevel)mainControls.push_back(child);
    addAndMakeVisible(mainTab);
    addAndMakeVisible(fxTab);
    mainTab.setTooltip("Oscillators, envelopes, Motion, macros and sampler");
    fxTab.setTooltip("Open the effects and equaliser page");
    mainTab.onClick=[this]{fxPage=false;updatePage();};
    fxTab.onClick=[this]{fxPage=true;updatePage();};
    for(size_t target=1;target<modulationBadges.size();++target){
        auto badge=std::make_unique<ModulationBadge>();
        badge->setName("Modulation depth "+processor.macroTargets()[int(target)]);
        badge->getRoute=[this,target](bool select){return routeForTarget(int(target),select);};
        badge->sourceColour=[this,target]{
            const int source=routeSourceForTarget(int(target));
            return juce::Colour(source!=selectedSource?0xff7b8583:source<4?0xffd3a363:source==7?0xffb29ed3:0xff69b6cc);
        };
        badge->changed=[this]{refreshModulationIndicators();repaint();};
        addChildComponent(*badge);
        modulationBadges[target]=std::move(badge);
    }
    updatePage();
    startTimerHz(30);
}
SpaceInfinityEditor::~SpaceInfinityEditor(){
    stopTimer();
    for(size_t point=0;point<motionGestures.size();++point)
        if(motionGestures[point])
            processor.state.getParameter(processor.lfoPointId(selectedLfo,int(point)))->endChangeGesture();
    if(envelopeDrag>=0){
        const char* ids[]={"attack","ampDecay","release"};
        processor.state.getParameter(ids[envelopeDrag])->endChangeGesture();
        if(envelopeDrag==1)processor.state.getParameter("ampSustain")->endChangeGesture();
    }
    setLookAndFeel(nullptr);
}
void SpaceInfinityEditor::timerCallback(){
    const float peak=processor.takeOutputPeak();
    meterPeak=juce::jmax(peak,meterPeak*0.88f);
    clipHold=peak>=1.0f?30:juce::jmax(0,clipHold-1);
    refreshModulationIndicators();
    repaint();
}
void SpaceInfinityEditor::updatePage(){
    for(auto* control:mainControls)control->setVisible(!fxPage);
    for(const int index:{7,8,9,15,16}){
        knobs[size_t(index)].setVisible(fxPage);
        labels[size_t(index)].setVisible(fxPage);
    }
    for(juce::Component* effect:std::array<juce::Component*,5>{&delaySync,&delayMode,&reverbSpace,&compAmount,&eqButton})
        effect->setVisible(fxPage);
    for(size_t i=0;i<rackControls.size();++i){
        rackControls[i].setVisible(fxPage);
        rackLabels[i].setVisible(fxPage);
    }
    for(auto& power:rackEnabled)power.setVisible(fxPage);
    for(const int index:{7,8,9,15,16}){
        knobs[size_t(index)].setVisible(false);
        labels[size_t(index)].setVisible(false);
    }
    compAmount.setVisible(false);
    knobs[12].setSliderStyle(fxPage?juce::Slider::LinearHorizontal:juce::Slider::LinearVertical);
    knobs[12].setVisible(true);
    labels[12].setVisible(true);
    knobs[10].setVisible(false);labels[10].setVisible(false);
    knobs[11].setVisible(false);labels[11].setVisible(false);
    motionDepth.setVisible(false);
    updateLfoRateVisibility();
    mainTab.setToggleState(!fxPage,juce::dontSendNotification);
    fxTab.setToggleState(fxPage,juce::dontSendNotification);
    resized();
    refreshModulationIndicators();
    repaint();
}
void SpaceInfinityEditor::bindLfoControls(){
    lfoBarsBinding.reset();lfoSyncBinding.reset();lfoModeBinding.reset();lfoSpeedBinding.reset();
    for(auto& binding:lfoPointBindings)binding.reset();
    lfoBarsBinding=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        processor.state,processor.lfoParameterId(selectedLfo,"Bars"),motionBars);
    lfoSyncBinding=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        processor.state,processor.lfoParameterId(selectedLfo,"Sync"),lfoSync);
    lfoModeBinding=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        processor.state,processor.lfoParameterId(selectedLfo,"Mode"),lfoMode);
    lfoSpeedBinding=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.state,processor.lfoParameterId(selectedLfo,"Speed"),lfoSpeed);
    lfoSpeed.textFromValueFunction=[](double value){return juce::String(value,2)+" Hz";};
    lfoSpeed.valueFromTextFunction=[](const juce::String& text){return text.getDoubleValue();};
    lfoSpeed.updateText();
    lfoSpeed.setDoubleClickReturnValue(true,1.0);
    for(size_t point=0;point<motionPoints.size();++point)
        lfoPointBindings[point]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.state,processor.lfoPointId(selectedLfo,int(point)),motionPoints[point]);
    motionShape.setSelectedId(0,juce::dontSendNotification);
    updateLfoRateVisibility();
    repaint();
}
void SpaceInfinityEditor::updateLfoRateVisibility(){
    const bool sync=lfoSync.getSelectedId()==2;
    motionBars.setVisible(!fxPage&&sync);
    lfoSpeed.setVisible(!fxPage&&!sync);
}
int SpaceInfinityEditor::routeSourceForTarget(int target) const{
    const auto hasRoute=[this,target](int source){
        for(int slot=0;slot<3;++slot)
            if(int(processor.state.getRawParameterValue(processor.modulationRouteId(source,slot,false))->load())==target)
                return true;
        return false;
    };
    if(hasRoute(selectedSource))return selectedSource;
    for(int source=0;source<8;++source)if(hasRoute(source))return source;
    return -1;
}
std::pair<juce::RangedAudioParameter*,juce::RangedAudioParameter*> SpaceInfinityEditor::routeForTarget(int target,bool select){
    const int source=routeSourceForTarget(target);
    if(source<0)return {nullptr,nullptr};
    if(select)selectedSource=source;
    for(int slot=0;slot<3;++slot){
        auto* destination=processor.state.getParameter(processor.modulationRouteId(source,slot,false));
        if(int(destination->convertFrom0to1(destination->getValue()))==target)
            return {processor.state.getParameter(processor.modulationRouteId(source,slot,true)),destination};
    }
    return {nullptr,nullptr};
}
void SpaceInfinityEditor::refreshModulationIndicators(){
    for(auto* child:getChildren())if(auto* slider=dynamic_cast<juce::Slider*>(child)){
        slider->getProperties().remove("modDepth");
        slider->getProperties().remove("modColour");
    }
    for(size_t target=1;target<modulationBadges.size();++target){
        auto* badge=modulationBadges[target].get();
        if(!badge)continue;
        const auto area=targetBounds(int(target));
        const auto route=routeForTarget(int(target),false);
        const bool visible=route.first!=nullptr&&!area.isEmpty();
        badge->setVisible(visible);
        if(visible){
            badge->setBounds(area.getX()+1,area.getY()+1,17,17);
            badge->toFront(false);
            const int source=routeSourceForTarget(int(target));
            const auto sourceName=source<4?processor.macroNames()[source]:
                source==7?juce::String("AMP ENV"):"LFO "+juce::String(source-3);
            const float depth=route.first->convertFrom0to1(route.first->getValue());
            if(source==selectedSource){
                auto colour=juce::Colour(source<4?0xffd3a363:source==7?0xffb29ed3:0xff69b6cc);
                if(depth<0)colour=colour.brighter(.5f);
                for(auto* child:getChildren())if(auto* slider=dynamic_cast<juce::Slider*>(child))
                    if(slider->isVisible()&&slider->getBounds()==area){
                        slider->getProperties().set("modDepth",double(depth));
                        slider->getProperties().set("modColour",juce::int64(colour.getARGB()));
                    }
            }
            badge->setTooltip(sourceName+" > "+processor.macroTargets()[int(target)]+" "+
                juce::String(depth*100.0f,0)+"%. Drag up/down to adjust. Right-click to remove.");
            badge->repaint();
        }
    }
}
juce::Rectangle<int> SpaceInfinityEditor::targetBounds(int target) const{
    if(target==18||target==21||target==22){
        const int lfo=target==18?0:target-20;
        return !fxPage&&lfo==selectedLfo?(lfoSpeed.isVisible()?lfoSpeed.getBounds():motionBars.getBounds()):juce::Rectangle<int>{};
    }
    const juce::Component* controls[]={nullptr,&knobs[0],&knobs[4],&knobs[9],&knobs[2],&knobs[6],
        &knobs[8],&knobs[7],&knobs[5],&knobs[3],&knobs[1],&noiseLevel,&oscillatorSliders[1],
        &oscillatorSliders[3],&knobs[12],&knobs[13],&knobs[14],&motionDepth,&knobs[10],&knobs[11],&analogueCharacter};
    if(target<=0||target>=int(std::size(controls)))return {};
    if(fxPage){
        const char* id=target==3?"reverb":target==6?"delay":target==7?"chorus":nullptr;
        if(id)for(size_t i=0;i<effectControls.size();++i)
            if(juce::String(effectControls[i].id)==id)return rackControls[i].getBounds();
    }
    return controls[target]->isVisible()?controls[target]->getBounds():juce::Rectangle<int>{};
}
int SpaceInfinityEditor::targetAt(juce::Point<int> cursor) const{
    for(int target=1;target<SpaceInfinityProcessor::macroTargets().size();++target)
        if(targetBounds(target).contains(cursor))return target;
    return 0;
}
void SpaceInfinityEditor::assignMacro(int source,int target,bool inverted){
    int available=-1;
    for(int slot=0;slot<3;++slot){
        auto* route=processor.state.getParameter(processor.modulationRouteId(source,slot,false));
        const int destination=int(route->convertFrom0to1(route->getValue()));
        if(destination==target){available=slot;break;}
        if(destination==0&&available<0)available=slot;
    }
    if(available<0){
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,"Source assignments full",
            "Right-click a modulation badge on an assigned control and remove one destination before adding another.");
        return;
    }
    auto* route=processor.state.getParameter(processor.modulationRouteId(source,available,false));
    auto* depth=processor.state.getParameter(processor.modulationRouteId(source,available,true));
    route->beginChangeGesture();
    route->setValueNotifyingHost(route->convertTo0to1(float(target)));
    route->endChangeGesture();
    depth->beginChangeGesture();
    depth->setValueNotifyingHost(depth->convertTo0to1(inverted?-0.5f:0.5f));
    depth->endChangeGesture();
    selectedSource=source;
    refreshModulationIndicators();
    repaint();
}
float SpaceInfinityEditor::envelopeDuration() const{
    return envelopeDrag>=0?envelopeDragDuration:juce::jmax(.5f,float(knobs[2].getValue()+knobs[13].getValue()+knobs[3].getValue())+.4f);
}
std::array<juce::Point<float>,3> SpaceInfinityEditor::envelopeNodes() const{
    const float duration=envelopeDuration();
    const float attack=float(knobs[2].getValue()),decay=float(knobs[13].getValue());
    const float sustain=float(knobs[14].getValue());
    const auto point=[this,duration](float time,float level){
        return juce::Point<float>(envelopeGraph.getX()+envelopeGraph.getWidth()*time/duration,
            envelopeGraph.getBottom()-4-(envelopeGraph.getHeight()-8)*level);
    };
    return {point(attack,1.0f),point(attack+decay,sustain),point(duration,0.0f)};
}
void SpaceInfinityEditor::mouseDown(const juce::MouseEvent& e){
    if(fxPage)return;
    if(e.mods.isLeftButtonDown()){
        for(size_t i=0;i<macros.size();++i){
            const auto handle=macros[i].getBounds().translated(0,-17).withHeight(17);
            if(handle.contains(e.getPosition())){
                draggedMacro=int(i);selectedSource=draggedMacro;
                refreshModulationIndicators();repaint();return;
            }
        }
        for(size_t i=0;i<lfoTabs.size();++i)if(lfoTabs[i].contains(e.getPosition())){
            selectedLfo=int(i);selectedSource=int(i)+4;draggedMacro=selectedSource;
            bindLfoControls();refreshModulationIndicators();return;
        }
        if(envelopeTab.contains(e.getPosition())){
            draggedMacro=selectedSource=7;refreshModulationIndicators();repaint();return;
        }
        const auto nodes=envelopeNodes();
        for(size_t i=0;i<nodes.size();++i)if(nodes[i].getDistanceFrom(e.position)<11.0f){
            processor.state.getParameter("ampTimed")->setValueNotifyingHost(1.0f);
            const float duration=envelopeDuration();
            envelopeDrag=int(i);envelopeDragDuration=duration;
            const char* ids[]={"attack","ampDecay","release"};
            processor.state.getParameter(ids[i])->beginChangeGesture();
            if(i==1)processor.state.getParameter("ampSustain")->beginChangeGesture();
            return;
        }
    }
    if(!e.mods.isLeftButtonDown()||!motionGraph.expanded(4).contains(e.getPosition()))return;
    drawMotionAt(e.getPosition());
}
void SpaceInfinityEditor::mouseDrag(const juce::MouseEvent& e){
    if(draggedMacro>=0){
        if(fxTab.getBounds().contains(e.getPosition())&&!fxPage){fxPage=true;updatePage();}
        if(mainTab.getBounds().contains(e.getPosition())&&fxPage){fxPage=false;updatePage();}
        hoverTarget=targetAt(e.getPosition());
        repaint();
    }else if(envelopeDrag>=0){
        const auto set=[this](const char* id,float value){
            auto* p=processor.state.getParameter(id);
            p->setValueNotifyingHost(p->convertTo0to1(value));
        };
        const float time=juce::jlimit(0.0f,envelopeDragDuration,
            float(e.x-envelopeGraph.getX())/float(envelopeGraph.getWidth())*envelopeDragDuration);
        if(envelopeDrag==0)set("attack",juce::jlimit(0.0f,1.0f,time));
        if(envelopeDrag==1){
            set("ampDecay",juce::jlimit(.005f,8.0f,time-float(knobs[2].getValue())));
            set("ampSustain",juce::jlimit(0.0f,1.0f,
                float(envelopeGraph.getBottom()-4-e.y)/float(envelopeGraph.getHeight()-8)));
        }
        if(envelopeDrag==2)
            set("release",juce::jlimit(0.0f,1.0f,time-float(knobs[2].getValue()+knobs[13].getValue())-.4f));
    }else if(lastMotionPoint>=0)drawMotionAt(e.getPosition());
}
void SpaceInfinityEditor::mouseUp(const juce::MouseEvent& e){
    if(draggedMacro>=0){
        const int target=targetAt(e.getPosition());
        if(target>0)assignMacro(draggedMacro,target,e.mods.isShiftDown());
        draggedMacro=-1;hoverTarget=0;repaint();return;
    }
    if(envelopeDrag>=0){
        const char* ids[]={"attack","ampDecay","release"};
        processor.state.getParameter(ids[envelopeDrag])->endChangeGesture();
        if(envelopeDrag==1)processor.state.getParameter("ampSustain")->endChangeGesture();
        envelopeDrag=-1;
    }
    for(size_t i=0;i<motionGestures.size();++i){
        if(motionGestures[i]){
            processor.state.getParameter(processor.lfoPointId(selectedLfo,int(i)))->endChangeGesture();
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
        auto* parameter=processor.state.getParameter(processor.lfoPointId(selectedLfo,i));
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
    g.fillAll(juce::Colour(0xff2b1712));
    for(int side:{0,getWidth()-20}){
        const auto wood=juce::Rectangle<float>(float(side),0.0f,20.0f,float(getHeight()));
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff4d281c),wood.getX(),0,
            juce::Colour(0xff8c5433),wood.getRight(),0,false));
        g.fillRect(wood);
        g.setColour(juce::Colour(0x26301008));
        for(int x=side+2;x<side+20;x+=3)g.drawVerticalLine(x,0.0f,float(getHeight()));
    }
    const auto face=getLocalBounds().toFloat().reduced(20.0f,4.0f);
    g.setColour(juce::Colour(0xff242625));
    g.fillRoundedRectangle(face,4.0f);
    g.setColour(juce::Colour(0xff67584b));
    g.drawRoundedRectangle(face,4.0f,1.0f);
    const auto header=juce::Rectangle<float>(25,9,float(getWidth()-50),96);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff842e26),0,9,
        juce::Colour(0xff48201d),0,105,false));
    g.fillRoundedRectangle(header,4.0f);
    for(const float x:{33.0f,float(getWidth()-34)}){
        g.setColour(juce::Colour(0xff0d0d0c));g.fillEllipse(x-3,17,6,6);
        g.setColour(juce::Colour(0xff716a5d));g.drawLine(x-2,21,x+2,18,1.0f);
    }
    g.setFont(10.0f);
    g.setColour(juce::Colour(0xffb7ab9b));
    g.drawText("FACTORY PROGRAM",getWidth()-330,24,200,15,juce::Justification::centredLeft);
    g.drawText("NOISE",getWidth()-330,73,58,22,juce::Justification::centredLeft);
    g.drawText("MASTER OCT",getWidth()-466,116,77,22,juce::Justification::right);
    g.drawText("PITCH",getWidth()-316,116,51,22,juce::Justification::right);
    if(fxPage){
        const juce::Colour colours[]{juce::Colour(0xffd1b065),juce::Colour(0xff72a2ac),
            juce::Colour(0xffbf9d72),juce::Colour(0xff839d75),juce::Colour(0xffa28fb7),juce::Colour(0xff7ea6ae)};
        for(int row=0;row<6;++row){
            auto bounds=effectRowBounds(getWidth(),getHeight(),row);
            drawInset(g,bounds.toFloat());
            g.setColour(colours[row]);
            g.fillRect(bounds.getX()+2,bounds.getY()+3,3,bounds.getHeight()-6);
        }
        g.setColour(juce::Colour(0xffc2b5a4));
        g.drawText("TIMING",delaySync.getBounds().translated(0,-15).withHeight(13),juce::Justification::centredLeft);
        g.drawText("SPACE",reverbSpace.getBounds().translated(0,-15).withHeight(13),juce::Justification::centredLeft);
    }else{
        const auto layout=mainLayout(getWidth(),getHeight());
        for(const auto region:{layout.oscillators[0],layout.oscillators[1],layout.filter,layout.macros,layout.envelope,layout.motion})
            drawInset(g,region.toFloat());
        const auto parameter=[this](const char* id){return processor.state.getRawParameterValue(id)->load();};
        const int waveA=int(parameter("wave"));
        const int waveBChoice=int(parameter("osc2Wave"));
        const int waves[]{waveA,waveBChoice==0?waveA:waveBChoice-1};
        const int modes[]{int(parameter("osc1Warp")),int(parameter("osc2Warp"))};
        const float amounts[]{parameter("osc1WarpAmount"),parameter("osc2WarpAmount")};
        const double tune[]{std::exp2(parameter("osc1Octave")+parameter("osc1Tune")/1200.0),
            std::exp2(parameter("osc2Octave")+parameter("osc2Tune")/1200.0)*(1.0+parameter("detune"))};
        for(int oscillator=0;oscillator<2;++oscillator){
            const auto bounds=layout.oscillators[oscillator];
            const auto screen=juce::Rectangle<int>(bounds.getX()+8,bounds.getY()+31,bounds.getWidth()-16,94);
            g.setColour(juce::Colour(0xff090f0e));g.fillRect(screen);
            g.setColour(juce::Colour(0xff20322c));
            g.drawHorizontalLine(screen.getCentreY(),float(screen.getX()),float(screen.getRight()));
            for(int i=1;i<5;++i)g.drawVerticalLine(screen.getX()+screen.getWidth()*i/5,float(screen.getY()),float(screen.getBottom()));
            juce::Path shape;
            for(int point=0;point<=256;++point){
                const double phase=double(point)/128.0;
                const auto modulator=SpaceInfinityProcessor::oscillatorSample(
                    phase*tune[1-oscillator]/tune[oscillator],waves[1-oscillator]);
                const auto value=SpaceInfinityProcessor::warpedSample(phase,waves[oscillator],modes[oscillator],amounts[oscillator],modulator);
                const float x=float(screen.getX())+float(point)*float(screen.getWidth())/256.0f;
                const float y=float(screen.getCentreY())-value*float(screen.getHeight())*.42f;
                if(point==0)shape.startNewSubPath(x,y);else shape.lineTo(x,y);
            }
            const bool active=parameter(oscillator==0?"osc1Enabled":"osc2Enabled")>.5f;
            g.setColour(juce::Colour(oscillator==0?0xffa8c378:0xff77b3a5).withAlpha(active?1.0f:.3f));
            g.strokePath(shape,juce::PathStrokeType(1.6f));
            g.setColour(juce::Colour(0xffb4c0ad));
            const char* modesText[]{"NORMAL","FM","MIRROR","PWM"};
            g.drawText(modesText[juce::jlimit(0,3,modes[oscillator])],screen.reduced(4).withHeight(12),juce::Justification::centredRight);
            const auto label=[&g](const juce::Component& control,const char* text){
                g.setColour(juce::Colour(0xffb9b1a2));
                g.drawText(text,control.getBounds().translated(0,-15).withHeight(13),juce::Justification::centred);
            };
            label(oscillatorSliders[size_t(oscillator*2)],"FINE");
            label(oscillatorSliders[size_t(oscillator*2+1)],"WIDTH");
            label(unisonSliders[size_t(oscillator)],"UNISON");
            const auto octave=octaveControls[size_t(oscillator+1)].getBounds();
        g.drawText("OCT",octave.getX()-28,octave.getY(),25,octave.getHeight(),juce::Justification::right);
        }
        g.setColour(juce::Colour(0xffb9b1a2));
        g.drawText("FILTER / VOICING",layout.filter.reduced(9,4).withHeight(17),juce::Justification::centredLeft);
        g.drawText("MACROS",layout.macros.reduced(8,3).withHeight(17),juce::Justification::centredLeft);
        g.setColour(juce::Colour(selectedSource==7?0xff665172:0xff353b3c));
        g.fillRoundedRectangle(envelopeTab.toFloat(),2.0f);
        g.setColour(juce::Colour(0xffdfd2eb));
        int envelopeRoutes=0;
        for(int slot=0;slot<3;++slot)if(processor.state.getRawParameterValue(processor.modulationRouteId(7,slot,false))->load()>.5f)++envelopeRoutes;
        g.drawText("AMP ENV  ("+juce::String(envelopeRoutes)+") +",envelopeTab,juce::Justification::centredLeft);
        for(size_t lfo=0;lfo<lfoTabs.size();++lfo){
            g.setColour(juce::Colour(selectedLfo==int(lfo)?0xff294f59:0xff303738));
            g.fillRoundedRectangle(lfoTabs[lfo].toFloat(),2.0f);
            g.setColour(juce::Colour(0xffc9e0e5));
            int count=0;
            for(int slot=0;slot<3;++slot)if(processor.state.getRawParameterValue(processor.modulationRouteId(int(lfo)+4,slot,false))->load()>.5f)++count;
            g.drawText("LFO "+juce::String(int(lfo+1))+" ("+juce::String(count)+") +",lfoTabs[lfo],juce::Justification::centred);
        }
        const auto names=SpaceInfinityProcessor::macroNames();
        for(size_t i=0;i<macros.size();++i){
            const auto handle=macros[i].getBounds().translated(0,-17).withHeight(17);
            g.setColour(juce::Colour(draggedMacro==int(i)?0xff735344:0xff363b39));
            g.fillRoundedRectangle(handle.toFloat(),2.0f);
            g.setColour(juce::Colour(0xffd8cdbb));
            g.drawText(names[int(i)]+" +",handle,juce::Justification::centred);
            int count=0;
            for(int slot=0;slot<3;++slot)
                if(processor.state.getRawParameterValue(processor.modulationRouteId(int(i),slot,false))->load()>0.5f)++count;
            g.setColour(juce::Colour(0xffa5b8ad));
            const auto knob=macros[i].getBounds();
            g.drawText(juce::String(int(i+1))+" ("+juce::String(count)+")",knob.getRight()-30,knob.getBottom()-18,29,16,juce::Justification::centredRight);
        }
        const auto envelope=envelopeGraph;
        g.setColour(juce::Colour(0xff0b1418));g.fillRect(envelope);
        const float attack=float(knobs[2].getValue()),decay=float(knobs[13].getValue());
        const float sustain=float(knobs[14].getValue()),release=float(knobs[3].getValue());
        const float total=juce::jmax(.5f,attack+decay+release+.4f);
        const auto envPoint=[&envelope,total](float time,float level){
            return juce::Point<float>(envelope.getX()+envelope.getWidth()*time/total,envelope.getBottom()-4-(envelope.getHeight()-8)*level);
        };
        juce::Path env;
        env.startNewSubPath(envPoint(0,0));env.lineTo(envPoint(attack,1));
        env.lineTo(envPoint(attack+decay,sustain));env.lineTo(envPoint(attack+decay+.4f,sustain));
        env.lineTo(envPoint(total,0));
        g.setColour(juce::Colour(0xffb3a2d6));g.strokePath(env,juce::PathStrokeType(1.6f));
        for(const auto point:envelopeNodes())g.fillEllipse(point.x-3,point.y-3,6,6);
        g.setColour(juce::Colour(0xff263d43));
        for(int i=1;i<5;++i)g.drawVerticalLine(motionGraph.getX()+motionGraph.getWidth()*i/5,float(motionGraph.getY()),float(motionGraph.getBottom()));
        juce::Path motion;
        for(size_t i=0;i<motionPoints.size();++i){
            const auto point=juce::Point<float>(float(motionGraph.getX())+float(motionGraph.getWidth())*float(i)/16.0f,
                float(motionGraph.getBottom())-float(motionPoints[i].getValue())*float(motionGraph.getHeight()));
            if(i==0)motion.startNewSubPath(point);else motion.lineTo(point);
            g.setColour(juce::Colour(0xff82bec9));g.fillEllipse(point.x-2,point.y-2,4,4);
        }
        g.setColour(juce::Colour(0xff82bec9));g.strokePath(motion,juce::PathStrokeType(1.5f));
        const int playhead=motionGraph.getX()+int(float(motionGraph.getWidth())*processor.lfoDisplayPhase(selectedLfo));
        g.setColour(juce::Colour(0xffa5d8df));
        g.drawVerticalLine(playhead,float(motionGraph.getY()),float(motionGraph.getBottom()));
        for(int target=1;target<int(modulationBadges.size());++target){
            const auto area=targetBounds(target);
            const auto route=routeForTarget(target,false);
            if(area.isEmpty()||!route.first||routeSourceForTarget(target)!=selectedSource)continue;
            const float amount=route.first->convertFrom0to1(route.first->getValue());
            g.setColour(juce::Colour(selectedSource<4?0xffd3a363:selectedSource==7?0xffb29ed3:0xff69b6cc));
            bool rotary=false;
            for(auto* child:getChildren())if(auto* slider=dynamic_cast<juce::Slider*>(child))
                if(slider->isVisible()&&slider->getBounds()==area
                   &&slider->getSliderStyle()==juce::Slider::RotaryHorizontalVerticalDrag)rotary=true;
            if(!rotary)g.drawRoundedRectangle(area.toFloat().reduced(1.0f),3.0f,1.0f+std::abs(amount));
        }
        if(hoverTarget>0){
            g.setColour(juce::Colour(0xffe1b87d));g.drawRoundedRectangle(targetBounds(hoverTarget).toFloat(),3,2);
        }
        const auto sampler=juce::Rectangle<int>(28,getHeight()-69,getWidth()-56,60);
        drawInset(g,sampler.toFloat());
        g.setColour(juce::Colour(0xffb9b1a2));
        g.drawText("START",34,getHeight()-66,49,20,juce::Justification::centredLeft);
        g.drawText("END",242,getHeight()-66,37,20,juce::Justification::centredLeft);
        g.drawText("SAMPLER",getWidth()-134,getHeight()-66,99,20,juce::Justification::centredRight);
    }
    const float db=juce::Decibels::gainToDecibels(meterPeak,-60.0f);
    for(int segment=0;segment<12;++segment){
        const int y=meterBounds.getBottom()-(segment+1)*meterBounds.getHeight()/12;
        const bool lit=segment==11?clipHold>0:db>=-54.0f+4.8f*float(segment);
        const auto colour=juce::Colour(segment==11?0xffee5d4c:segment>=8?0xffcfa44a:0xff72b06e);
        g.setColour(lit?colour:colour.darker(3));
        g.fillRect(meterBounds.getX(),y,meterBounds.getWidth(),juce::jmax(1,meterBounds.getHeight()/12-1));
    }
}
void SpaceInfinityEditor::resized(){
    const int w=getWidth(),h=getHeight();
    title.setBounds(38,23,juce::jmin(450,w-390),40);
    subtitle.setBounds(39,65,juce::jmin(450,w-390),24);
    presets.setBounds(w-330,42,226,25);
    previousPreset.setBounds(w-100,42,29,25);
    nextPreset.setBounds(w-67,42,29,25);
    noiseType.setBounds(w-266,72,90,23);
    noiseLevel.setBounds(w-172,72,134,23);
    mainTab.setBounds(34,111,68,25);
    fxTab.setBounds(107,111,68,25);
    octaveControls[0].setBounds(w-385,116,64,22);
    pitchWheel.setBounds(w-261,116,220,22);
    const auto layout=mainLayout(w,h);
    for(int i=0;i<2;++i){
        auto region=layout.oscillators[i].reduced(8);
        auto& power=i==0?osc1:osc2;
        power.setBounds(region.getX(),region.getY(),72,20);
        (i==0?warpA:warpB).setBounds(region.getX()+78,region.getY(),53,20);
        octaveControls[size_t(i+1)].setBounds(region.getRight()-62,region.getY(),62,20);
        auto& selector=i==0?waveform:waveform2;
        selector.setBounds(region.getX(),region.getY()+123,region.getWidth(),24);
        auto controls=juce::Rectangle<int>(region.getX(),region.getY()+169,region.getWidth(),23);
        unisonSliders[size_t(i)].setBounds(controls.removeFromRight(62).reduced(2,0));
        oscillatorSliders[size_t(i*2)].setBounds(controls.removeFromLeft(controls.getWidth()/2).reduced(2,0));
        oscillatorSliders[size_t(i*2+1)].setBounds(controls.reduced(2,0));
    }
    const auto characterBounds=layout.oscillators[1];
    analogueLabel.setBounds(characterBounds.getX()+8,characterBounds.getBottom()-60,characterBounds.getWidth()-16,15);
    analogueCharacter.setBounds(characterBounds.getX()+16,characterBounds.getBottom()-35,characterBounds.getWidth()-32,23);
    const auto oscBottom=layout.oscillators[0];
    labels[4].setBounds(oscBottom.getX()+8,oscBottom.getBottom()-60,oscBottom.getWidth()-16,15);
    labels[4].setText("UNISON SPREAD / DETUNE",juce::dontSendNotification);
    knobs[4].setBounds(oscBottom.getX()+20,oscBottom.getBottom()-47,oscBottom.getWidth()-40,43);
    auto filterControls=layout.filter.reduced(7).withTrimmedTop(21);
    filterControls.removeFromRight(52);
    const int gridHeight=(layout.filter.getHeight()-100)/2;
    const int indices[]{0,1,6,5};
    for(int i=0;i<4;++i){
        auto row=juce::Rectangle<int>(filterControls.getX(),filterControls.getY()+(i/2)*gridHeight,filterControls.getWidth(),gridHeight);
        const int half=row.getWidth()/2;
        auto cell=juce::Rectangle<int>(row.getX()+(i%2)*half,row.getY(),half,row.getHeight()).reduced(2,0);
        const auto index=size_t(indices[i]);
        labels[index].setBounds(cell.removeFromTop(17));
        knobs[index].setBounds(cell);
    }
    filterMode.setBounds(layout.filter.getX()+8,layout.filter.getBottom()-71,layout.filter.getWidth()-70,22);
    voiceMode.setBounds(layout.filter.getX()+8,layout.filter.getBottom()-32,juce::jmax(95,layout.filter.getWidth()-70),23);
    labels[12].setText("OUT",juce::dontSendNotification);
    labels[12].setBounds(layout.filter.getRight()-58,layout.filter.getY()+24,45,13);
    knobs[12].setBounds(layout.filter.getRight()-59,layout.filter.getY()+42,44,layout.filter.getHeight()-50);
    meterBounds={layout.filter.getRight()-12,layout.filter.getY()+48,6,layout.filter.getHeight()-65};
    const auto macroNames=SpaceInfinityProcessor::macroNames();
    const auto macroArea=layout.macros.reduced(7).withTrimmedTop(18);
    const int macroWidth=macroArea.getWidth()/2,macroHeight=macroArea.getHeight()/2;
    for(size_t i=0;i<macros.size();++i){
        const int column=int(i)%2,row=int(i)/2;
        auto cell=juce::Rectangle<int>(macroArea.getX()+column*macroWidth,macroArea.getY()+row*macroHeight,macroWidth,macroHeight).reduced(2,1);
        macros[i].setBounds(cell.withTrimmedTop(18));
    }
    const auto envelope=layout.envelope.reduced(7);
    const int envHeight=juce::jmax(55,layout.envelope.getHeight()/2-22);
    auto envelopeKnobs=juce::Rectangle<int>(envelope.getX(),envelope.getY()+envHeight+29,envelope.getWidth(),envelope.getBottom()-envelope.getY()-envHeight-29);
    const int envIndices[]{2,13,14,3};
    for(int i=0;i<4;++i){
        auto cell=envelopeKnobs.removeFromLeft(envelopeKnobs.getWidth()/(4-i)).reduced(1,0);
        const auto index=size_t(envIndices[i]);
        labels[index].setText(index==14?"SUSTAIN":index==2?"ATTACK":index==3?"RELEASE":"DECAY",juce::dontSendNotification);
        labels[index].setFont(juce::Font(juce::FontOptions(11.0f)));
        labels[index].setBounds(cell.removeFromTop(15));
        knobs[index].setBounds(cell);
    }
    envelopeTab=layout.envelope.reduced(8,3).withHeight(18);
    envelopeGraph=layout.envelope.reduced(8).withTrimmedTop(19).withHeight(juce::jmax(55,layout.envelope.getHeight()/2-22));
    auto tabs=layout.motion.reduced(8,3).withHeight(19);
    const int tabWidth=tabs.getWidth()/3;
    for(auto& tab:lfoTabs)tab=tabs.removeFromLeft(tabWidth).reduced(2,0);
    auto motion=layout.motion.reduced(8).withTrimmedTop(26);
    motionShape.setBounds(motion.removeFromTop(24).reduced(2,0));
    motion.removeFromTop(8);
    auto options=motion.removeFromBottom(28);
    lfoMode.setBounds(options.removeFromLeft(99).reduced(2,0));
    lfoSync.setBounds(options.removeFromLeft(76).reduced(2,0));
    motionBars.setBounds(options.reduced(2,0));
    lfoSpeed.setBounds(options.reduced(2,0));
    motionGraph=motion.reduced(4,6);
    source.setBounds(34,h-37,152,24);
    importButton.setBounds(194,h-37,104,24);
    sampleRoot.setBounds(306,h-37,116,24);
    sampleTrim[0].setBounds(84,h-65,150,22);
    sampleTrim[1].setBounds(279,h-65,133,22);
    sampleLoop.setBounds(425,h-65,100,22);
    sampleReverse.setBounds(532,h-65,100,22);
    for(int row=0;row<6;++row){
        auto region=effectRowBounds(w,h,row).reduced(9,5);
        rackEnabled[size_t(row)].setBounds(region.removeFromLeft(118).withSizeKeepingCentre(118,27));
        int count=0;
        for(const auto& control:effectControls)if(control.row==row)++count;
        const int slots=count<5?5:count;
        const int cellWidth=region.getWidth()/slots;
        for(size_t i=0;i<effectControls.size();++i){
            if(effectControls[i].row!=row)continue;
            auto cell=region.removeFromLeft(cellWidth).reduced(3,0);
            rackLabels[i].setBounds(cell.removeFromTop(17));
            rackControls[i].setBounds(cell);
        }
        if(row==4){
            auto cell=region.reduced(4,0);
            cell.removeFromTop(16);
            delaySync.setBounds(cell.removeFromTop(24));
            cell.removeFromTop(4);
            delayMode.setBounds(cell.removeFromTop(24));
        }
        if(row==5)reverbSpace.setBounds(region.reduced(4,0).withSizeKeepingCentre(region.getWidth()-8,26));
    }
    eqButton.setBounds(36,h-53,155,29);
    if(fxPage){
        labels[12].setText("OUTPUT",juce::dontSendNotification);
        labels[12].setBounds(w-285,h-52,65,22);
        knobs[12].setBounds(w-205,h-56,155,35);
        meterBounds={w-43,h-56,8,35};
    }
    refreshModulationIndicators();
}
juce::AudioProcessorEditor* SpaceInfinityProcessor::createEditor(){return new SpaceInfinityEditor(*this);}
