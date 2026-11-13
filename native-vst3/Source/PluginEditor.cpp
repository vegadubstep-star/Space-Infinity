#include "PluginEditor.h"
#include "ExtendedPanel.h"
#include "PresetPanel.h"
#include "EffectControls.h"
#include "StudioRack.h"
#include "SamplerDisplay.h"
#include <Artwork.h>
#include "LogoArtwork.h"
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
          amountBinding(processor.state,"osc"+juce::String(oscillator)+"WarpAmount",amount),
          semitoneBinding(processor.state,"osc"+juce::String(oscillator)+"Semitone",semitone),
          envelopeBinding(processor.state,"osc"+juce::String(oscillator)+"WarpEnvelope",envelope),phaseBinding(processor.state,"osc"+juce::String(oscillator)+"PhaseReset",phaseReset){
        amount.initialiseModulation(processor,"osc"+juce::String(oscillator)+"WarpAmount");semitone.initialiseModulation(processor,"osc"+juce::String(oscillator)+"Semitone");
        mode.addItemList({"Off",oscillator==1?"FM from B":"FM from A","Mirror","PWM","FM from Sub"},1);
        mode.setSelectedItemIndex(int(processor.state.getRawParameterValue("osc"+juce::String(oscillator)+"Warp")->load()),juce::dontSendNotification);
        amount.setTextBoxStyle(juce::Slider::TextBoxRight,false,48,22);
        amount.setTooltip("Warp amount. Double-click to reset.");
        amount.setDoubleClickReturnValue(true,0.0);
        showPercent(amount);
        addAndMakeVisible(mode);
        addAndMakeVisible(amount);
        semitone.setTextBoxStyle(juce::Slider::TextBoxRight,false,60,22);semitone.setTooltip("Semitone offset, in addition to octave and fine tuning.");semitone.setNumDecimalPlacesToDisplay(0);semitone.setDoubleClickReturnValue(true,0);addAndMakeVisible(semitone);
        envelope.addItemList({"No warp envelope","Amp envelope","Filter envelope"},1);envelope.setSelectedItemIndex(int(processor.state.getRawParameterValue("osc"+juce::String(oscillator)+"WarpEnvelope")->load()),juce::dontSendNotification);addAndMakeVisible(envelope);phaseReset.setButtonText("RETRIGGER PHASE");addAndMakeVisible(phaseReset);
        setSize(240,184);
    }
    void resized() override {mode.setBounds(10,9,220,26);amount.setBounds(10,43,220,25);semitone.setBounds(10,78,220,25);envelope.setBounds(10,112,220,25);phaseReset.setBounds(10,147,220,25);}
private:
    juce::ComboBox mode;
    ModSlider amount,semitone;
    juce::ComboBox envelope;juce::ToggleButton phaseReset;
    juce::ComboBoxParameterAttachment modeBinding;
    juce::AudioProcessorValueTreeState::SliderAttachment amountBinding,semitoneBinding;
    juce::AudioProcessorValueTreeState::ComboBoxAttachment envelopeBinding;
    juce::AudioProcessorValueTreeState::ButtonAttachment phaseBinding;
};
class EqPanel final : public juce::Component, private juce::Timer {
public:
    explicit EqPanel(SpaceInfinityProcessor& p):processor(p),enabled(p.state,"eqEnabled",power){
        setName("Seven-band EQ");
        addAndMakeVisible(power);
        for(int band=0;band<7;++band){
            modes[size_t(band)].addItemList({"Bell","Low shelf","High shelf","Low-pass","High-pass","Notch","Band-pass"},1);
            slopes[size_t(band)].addItemList({"12 dB/oct","24 dB/oct","36 dB/oct","48 dB/oct"},1);
            for(auto* box:{&modes[size_t(band)],&slopes[size_t(band)]})addAndMakeVisible(*box);
            bindings.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.state,"eqMode"+juce::String(band),modes[size_t(band)]));
            bindings.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.state,"eqSlope"+juce::String(band),slopes[size_t(band)]));
        }
        bandPicker.addItemList({"Band 1","Band 2","Band 3","Band 4","Band 5","Band 6","Band 7"},1);bandPicker.setSelectedId(1);bandPicker.onChange=[this]{bindBand(bandPicker.getSelectedId()-1);};addAndMakeVisible(bandPicker);
        for(auto& slider:bandValues){slider.setSliderStyle(juce::Slider::LinearHorizontal);slider.setTextBoxStyle(juce::Slider::TextBoxRight,false,66,22);addAndMakeVisible(slider);}bindBand(0);
        setSize(760,440);
        startTimerHz(24);
    }
    void resized() override {power.setBounds(12,4,120,24);bandPicker.setBounds(12,getHeight()-30,85,24);const int controlWidth=(getWidth()-112)/3;for(int i=0;i<3;++i)bandValues[size_t(i)].setBounds(105+i*controlWidth,getHeight()-30,controlWidth-5,24);const int width=(getWidth()-24)/7;
        for(int i=0;i<7;++i){modes[size_t(i)].setBounds(12+i*width,getHeight()-91,width-4,23);slopes[size_t(i)].setBounds(12+i*width,getHeight()-63,width-4,23);}}

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
            double magnitude=1.0;
            for(int band=0;band<7;++band){
                const int mode=int(processor.state.getRawParameterValue("eqMode"+juce::String(band))->load());
                const int count=(mode==3||mode==4)?int(processor.state.getRawParameterValue("eqSlope"+juce::String(band))->load())+1:1;
                const double sampleRate=processor.getSampleRate()>0?processor.getSampleRate()*2:96000;
                const auto z=std::polar(1.0,-juce::MathConstants<double>::twoPi*frequency/sampleRate);
                for(int stage=0;stage<count;++stage){const auto c=studio::eqCoefficients(sampleRate,value(band,"Frequency"),value(band,"Q"),value(band,"Gain"),mode,count,stage);
                    magnitude*=std::abs((double(c[0])+double(c[1])*z+double(c[2])*z*z)/(double(c[3])+double(c[4])*z+double(c[5])*z*z));}

            }
            const auto y=yAt(float(juce::jlimit(-18.0,18.0,20.0*std::log10(magnitude))));
            if(x==0)curve.startNewSubPath(area.getX(),y);else curve.lineTo(area.getX()+float(x),y);
        }
        g.setColour(juce::Colour(0xffdca55c));
        g.strokePath(curve,juce::PathStrokeType(1.6f));
        for(int band=0;band<7;++band){
            g.setColour(juce::Colour(0xffcdb2ed));g.setFont(10);g.drawText("BAND "+juce::String(band+1),12+band*(getWidth()-24)/7,getHeight()-111,(getWidth()-24)/7-4,18,juce::Justification::centred);
            const auto x=xAt(value(band,"Frequency")),y=yAt(value(band,"Gain"));
            g.setColour(juce::Colour(band==selected?0xfff6c180:0xffa66d42));
            g.fillEllipse(x-9,y-9,18,18);
            g.setColour(juce::Colours::black);
            g.drawText(juce::String(band+1),int(x)-9,int(y)-9,18,18,juce::Justification::centred);
        }
        g.setColour(juce::Colour(0xffbdb3a6));

    }
    void mouseDown(const juce::MouseEvent& e) override{
        selected=bandAt(e.position);
        if(selected<0)return;
        bandPicker.setSelectedId(selected+1,juce::dontSendNotification);bindBand(selected);
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
    std::array<juce::ComboBox,7> modes,slopes;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> bindings;
    juce::ComboBox bandPicker;std::array<ModSlider,3> bandValues;std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,3> bandBindings;
    void bindBand(int band){if(band<0||band>6)return;const char* fields[]{"Frequency","Gain","Q"};for(int i=0;i<3;++i){auto& slider=bandValues[size_t(i)];bandBindings[size_t(i)].reset();const auto id=processor.eqId(band,fields[i]);slider.initialiseModulation(processor,id);slider.setName("EQ "+juce::String(band+1)+" "+fields[i]);bandBindings[size_t(i)]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,id,slider);slider.setDoubleClickReturnValue(true,processor.state.getParameter(id)->convertFrom0to1(processor.state.getParameter(id)->getDefaultValue()));slider.textFromValueFunction=[i](double v){return juce::String(v,i==0?0:2)+(i==0?" Hz":i==1?" dB":" Q");};slider.valueFromTextFunction=[](const juce::String& text){return text.getDoubleValue();};slider.updateText();}}
    int selected=-1;
    void timerCallback() override {repaint();}
    juce::Rectangle<float> graph() const {return getLocalBounds().toFloat().reduced(12).withTrimmedTop(23).withTrimmedBottom(107);}
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
    const int margin=30,gap=8;
    const int total=width-margin*2;
    const int topHeight=juce::jlimit(300,350,int(float(height)*0.40f));
    const int oscillatorWidth=total*35/100;
    const int top=153;
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

void drawInset(juce::Graphics& g,juce::Rectangle<float> bounds){
    HardwareTheme::module(g,bounds);
}
}
SpaceInfinityEditor::SpaceInfinityEditor(SpaceInfinityProcessor& p):AudioProcessorEditor(&p),processor(p){
    setLookAndFeel(&hardwareLook);
    const LogoArtwork badge(juce::ImageFileFormat::loadFrom(Artwork::InfinityLogo_png,Artwork::InfinityLogo_pngSize));
    infinityLogo=badge.metal;logoShadow=badge.shadow;logoLight=badge.light;

    setResizable(true,true);setResizeLimits(800,760,1500,950);setSize(1180,820);
    title.setText("SPACE INFINITY",juce::dontSendNotification);title.setFont(juce::Font(juce::FontOptions(29.0f,juce::Font::bold).withKerningFactor(.075f)));title.setColour(juce::Label::textColourId,juce::Colour(0xffefe4cb));addAndMakeVisible(title);
    subtitle.setText("ORBITAL AUDIO  /  POLYPHONIC ANALOGUE SYNTHESIZER",juce::dontSendNotification);subtitle.setFont(juce::Font(juce::FontOptions(10.0f).withKerningFactor(.09f)));subtitle.setColour(juce::Label::textColourId,juce::Colour(0xffcbb99b));addAndMakeVisible(subtitle);
    refreshPresetMenu();
    presets.setTextWhenNothingSelected("Select a sound");
    presets.onChange=[this]{const auto result=processor.loadBankPreset(presets.getSelectedId()-1);
        if(result.failed())juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,"Preset",result.getErrorMessage());
        displayedPresetKey=processor.selectedPresetKey();};addAndMakeVisible(presets);
    const auto stepPreset=[this](int direction){
        const int count=int(processor.presetBank().entries().size());
        if(count==0)return;
        // Bank loads and host restores can precede the editor's timer refresh.
        // Resolve the actual loaded sound by stable key, never a stale menu row.
        const int current=processor.presetBank().indexForKey(processor.selectedPresetKey());
        const int next=current<0 ? (direction>0?0:count-1) : (current+direction+count)%count;
        const auto result=processor.loadBankPreset(next);
        refreshPresetMenu();
        if(result.failed())juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,"Preset",result.getErrorMessage());
    };
    previousPreset.setName("Previous preset");
    nextPreset.setName("Next preset");
    previousPreset.setTooltip("Previous preset");
    nextPreset.setTooltip("Next preset");
    previousPreset.onClick=[stepPreset]{stepPreset(-1);};
    nextPreset.onClick=[stepPreset]{stepPreset(1);};
    addAndMakeVisible(bankButton);bankButton.setTooltip("Save, rename, delete and reorder presets");
    bankButton.onClick=[this]{openPresetManager();};
    addAndMakeVisible(previousPreset);
    addAndMakeVisible(nextPreset);
    addAndMakeVisible(warpA);
    addAndMakeVisible(warpB);
    warpA.onClick=[this]{openPopup(std::make_unique<WarpPanel>(processor,1),warpA.getBounds());};
    warpB.onClick=[this]{openPopup(std::make_unique<WarpPanel>(processor,2),warpB.getBounds());};
    addAndMakeVisible(eqButton);
    eqButton.setTooltip("Open the seven-band parametric EQ");
    eqButton.onClick=[this]{
        openPopup(std::make_unique<EqPanel>(processor),eqButton.getBounds());
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
    sampleRoot.initialiseModulation(processor,"sampleRoot");sampleRoot.setSliderStyle(juce::Slider::IncDecButtons);
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
    for(auto* button:{&lfoPower,&vibratoPower,&longEnvelope})addAndMakeVisible(*button);
    lfoPower.setName("Selected LFO enabled");vibratoPower.setName("Legacy vibrato enabled");longEnvelope.setName("Long envelope x3");
    lfoPower.setTooltip("Bypass the selected LFO, preserving its shape and routes.");
    vibratoPower.setTooltip("Bypass the separate legacy vibrato/filter LFO.");
    longEnvelope.setTooltip("Multiply amp attack and release times by three.");
    vibratoPowerBinding=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.state,"vibratoEnabled",vibratoPower);
    longEnvelopeBinding=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.state,"ampLong",longEnvelope);
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
    noiseLevel.initialiseModulation(processor,"noiseLevel");noiseLevel.setSliderStyle(juce::Slider::LinearHorizontal);
    noiseLevel.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
    noiseLevel.setTooltip("Noise level (zero turns noise off)");
    addAndMakeVisible(noiseLevel);
    attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,"noiseLevel",noiseLevel));
    const char* oscIds[]={"osc1Tune","osc1Width","osc2Tune","osc2Width"};
    for(size_t i=0;i<oscillatorSliders.size();++i){
        auto& slider=oscillatorSliders[i];
        slider.initialiseModulation(processor,oscIds[i]);slider.setName(i%2==0?"Fine tune":"Stereo width");
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,68,18);
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
    if(processor.state.getRawParameterValue("modernUnison")->load()<.5f)for(int i=0;i<2;++i){auto* param=processor.state.getParameter("osc"+juce::String(i+1)+"Detune");const float amount=processor.state.getRawParameterValue("detune")->load()*processor.state.getRawParameterValue("osc"+juce::String(i+1)+"Spread")->load()/.04f;param->setValueNotifyingHost(juce::jlimit(0.0f,1.0f,amount));}
    const char* toneIds[]{"osc1Detune","osc1Blend","osc2Detune","osc2Blend"};
    for(size_t i=0;i<unisonTone.size();++i){auto& slider=unisonTone[i];slider.initialiseModulation(processor,toneIds[i]);slider.setName(i%2==0?"Oscillator detune":"Unison blend");
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,56,18);addAndMakeVisible(slider);
        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,toneIds[i],slider));slider.setDoubleClickReturnValue(true,i%2?.75:.2);
        if(i%2)showPercent(slider);else{slider.textFromValueFunction=[](double v){return juce::String(v,2);};slider.valueFromTextFunction=[](const juce::String& t){return t.getDoubleValue();};slider.updateText();}slider.setMouseDragSensitivity(300);
        if(i%2==0){slider.onDragStart=[this]{processor.state.getParameter("modernUnison")->setValueNotifyingHost(1);};slider.onValueChange=[this,&slider]{if(slider.hasKeyboardFocus(true))processor.state.getParameter("modernUnison")->setValueNotifyingHost(1);};}
        slider.setTooltip(i%2?"Updated unison: 75% gives equal voice levels; only applies above two voices. Legacy sessions retain their previous blend law.":"Independent detune amount for this oscillator. Shift-drag for fine control.");
    }
    for(size_t i=0;i<unisonSliders.size();++i){
        auto& slider=unisonSliders[i];
        slider.initialiseModulation(processor,i==0?"osc1Unison":"osc2Unison");slider.setName("Unison voices");
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,42,18);
        slider.setTooltip("Unison voices (1–16)");
        addAndMakeVisible(slider);
        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.state,i==0?"osc1Unison":"osc2Unison",slider));
    }
    const char* octaveIds[]={"masterOctave","osc1Octave","osc2Octave"};
    const char* octaveNames[]={"Master octave","Osc 1 octave","Osc 2 octave"};
    for(size_t i=0;i<octaveControls.size();++i){
        auto& control=octaveControls[i];
        control.initialiseModulation(processor,octaveIds[i]);control.setSliderStyle(juce::Slider::IncDecButtons);
        control.setTextBoxStyle(juce::Slider::TextBoxLeft,false,23,17);
        control.setName(octaveNames[i]);
        control.setTooltip(octaveNames[i]);
        addAndMakeVisible(control);
        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.state,octaveIds[i],control));
    }
    bindChoice(voiceMode,"voiceMode",{"Poly","Mono","Legato"});
    bindChoice(filterMode,"filterMode",{"Classic LP","Resonant LP","High-pass","Band-pass","24 dB LP","Analogue ZDF","Acid Resonant"});
    filterMode.setTooltip("Analogue ZDF is a four-pole nonlinear low-pass. Acid Resonant keeps the moving squelch prominent for cutoff automation; adjust Resonance, Filter Env Amount and Pre Drive.");
    analogueCharacter.initialiseModulation(processor,"analogueCharacter");analogueCharacter.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    analogueCharacter.setTextBoxStyle(juce::Slider::TextBoxBelow,false,70,18);
    analogueCharacter.setDoubleClickReturnValue(true,0.0);
    analogueCharacter.setTooltip("Analogue character: gentle drift, soft saturation and smoother oscillator edges.");
    analogueCharacter.setName("Analogue character");
    addAndMakeVisible(analogueCharacter);
    attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.state,"analogueCharacter",analogueCharacter));
    showPercent(analogueCharacter);
    preDrive.initialiseModulation(processor,"filterInputDrive");preDrive.setName("Pre-filter drive");preDrive.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);preDrive.setTextBoxStyle(juce::Slider::TextBoxBelow,false,70,16);preDrive.setMouseDragSensitivity(350);preDrive.setDoubleClickReturnValue(true,0);preDrive.setTooltip("Drive into the filter before resonance; distinct from the output saturation DRIVE.");attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,"filterInputDrive",preDrive));showPercent(preDrive);addAndMakeVisible(preDrive);preDriveLabel.setText("PRE DRIVE",juce::dontSendNotification);preDriveLabel.setJustificationType(juce::Justification::centred);addAndMakeVisible(preDriveLabel);
    portamento.initialiseModulation(processor,"glideTime");portamento.setName("Portamento time");portamento.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);portamento.setTextBoxStyle(juce::Slider::TextBoxBelow,false,65,16);configureTimeKnob(portamento);portamento.setDoubleClickReturnValue(true,0);portamento.textFromValueFunction=[](double v){return juce::String(v*1000,1)+" ms";};portamento.valueFromTextFunction=[](const juce::String& t){return t.getDoubleValue()/1000;};portamento.setTooltip("Portamento / glide. Dragging in Poly switches to Legato; overlap notes for slides. Zero gives instant pitch. Shift-drag for fine control.");portamento.onDragStart=[this]{auto* mode=processor.state.getParameter("voiceMode");if(mode->convertFrom0to1(mode->getValue())<.5f)mode->setValueNotifyingHost(mode->convertTo0to1(2));};attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,"glideTime",portamento));portamento.textFromValueFunction=[](double v){return juce::String(v*1000,1)+" ms";};portamento.valueFromTextFunction=[](const juce::String& t){return t.getDoubleValue()/1000;};portamento.updateText();addAndMakeVisible(portamento);portamentoLabel.setText("PORTAMENTO",juce::dontSendNotification);portamentoLabel.setJustificationType(juce::Justification::centred);portamentoLabel.setFont(juce::Font(juce::FontOptions(9)));addAndMakeVisible(portamentoLabel);
    analogueOn.addItemList({"ANALOGUE","DIGITAL"},1);analogueOn.setName("Overall engine mode");analogueOn.setTooltip("Analogue: use character and component variation. Digital: bypass drift, tolerance and analogue colour while retaining filter and envelope settings.");fxChoices.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.state,"engineMode",analogueOn));addAndMakeVisible(analogueOn);addAndMakeVisible(analogueLabButton);analogueLabButton.onClick=[this]{auto panel=std::make_unique<ExtendedPanel>(processor);panel->selectTab(7);openPopup(std::move(panel),analogueLabButton.getBounds());};addAndMakeVisible(pitchSettings);pitchSettings.setTooltip("Pitch envelope, tuning and MIDI wheel range");pitchSettings.onClick=[this]{auto panel=std::make_unique<ExtendedPanel>(processor);panel->selectTab(8);openPopup(std::move(panel),pitchSettings.getBounds());};

    analogueLabel.setText("ANALOGUE CHARACTER",juce::dontSendNotification);
    analogueLabel.setJustificationType(juce::Justification::centred);
    analogueLabel.setColour(juce::Label::textColourId,juce::Colour(0xffd9d1bb));
    addAndMakeVisible(analogueLabel);
    bindChoice(sampleLoop,"sampleLoop",{"Loop off","Loop on"});
    bindChoice(sampleReverse,"sampleReverse",{"Forward","Reverse"});
    const char* trimIds[]={"sampleStart","sampleEnd"};
    const char* trimNames[]={"Sample start","Sample end"};
    for(size_t i=0;i<sampleTrim.size();++i){
        auto& trim=sampleTrim[i];trim.initialiseModulation(processor,i==0?"sampleStart":"sampleEnd");
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
    pitchWheel.initialiseModulation(processor,"pitchBend");pitchWheel.setSliderStyle(juce::Slider::LinearVertical);
    pitchWheel.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
    pitchWheel.setDoubleClickReturnValue(true,0.0);pitchWheel.setScrollWheelEnabled(false);
    pitchWheel.setName("Pitch Wheel");
    pitchWheel.setTooltip("Automatable Pitch Wheel ±24 semitones. In FL Studio: tweak this wheel, then Tools > Last tweaked > Create automation clip. Release the wheel to spring back to zero. Range: -24 to +24 semitones. Right-click for modulation assignments.");
    addAndMakeVisible(pitchWheel);
    pitchAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,"pitchBend",pitchWheel);
    pitchWheel.textFromValueFunction=[](double value){return juce::String(value,1)+" st";};
    pitchWheel.updateText();
    longEnvelope.onStateChange=[this]{knobs[2].updateText();knobs[3].updateText();repaint();};
    const char* ids[]={"cutoff","res","attack","release","detune","mix","drive","chorus","delay","reverb","lforate","lfodepth","gain","ampDecay","ampSustain","flanger","phaser"};
    const int decimals[]={0,2,3,3,3,2,2,2,2,2,2,2,2,3,2,2,2};
    const char* units[]={" Hz",""," s"," s","","","","","",""," Hz","",""," s","","",""};
    const char* texts[]={"CUTOFF","RESONANCE","ATTACK","RELEASE","GLOBAL DETUNE","LAYER","DRIVE","CHORUS","DELAY","REVERB","LFO RATE","LFO DEPTH","OUTPUT","DECAY","SUSTAIN","FLANGER","PHASER"};
    for(int i=0;i<17;i++){
        auto& slider=knobs[i];slider.initialiseModulation(processor,ids[i]);slider.setName(texts[i]);slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,70,19);
        if(i==0)slider.setTooltip("Automate Filter Cutoff for acid sweeps. Acid Resonant keeps the moving resonance prominent; Filter Env Amount and Decay shape each note. Shift-drag for fine adjustment.");
        if(i==4)slider.setTooltip("Shared unison detune for both oscillators, scaled by each oscillator's SPREAD control. This does not control volume.");
        slider.setColour(juce::Slider::rotarySliderFillColourId,juce::Colour(0xffe84953));slider.setColour(juce::Slider::thumbColourId,juce::Colours::white);
        slider.setColour(juce::Slider::textBoxTextColourId,juce::Colour(0xffd9d1bb));slider.setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
        addAndMakeVisible(slider);
        auto& label=labels[i];label.setText(texts[i],juce::dontSendNotification);label.setJustificationType(juce::Justification::centred);label.setColour(juce::Label::textColourId,juce::Colour(0xffd9d1bb));addAndMakeVisible(label);
        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,ids[i],slider));
        slider.setNumDecimalPlacesToDisplay(decimals[i]);
        const auto suffix=juce::String(units[i]);
        const int precision=decimals[i];
        slider.textFromValueFunction=[this,i,suffix,precision](double value){
            const double scale=(i==2||i==3)&&longEnvelope.getToggleState()?3.0:1.0;
            return juce::String(value*scale,precision)+suffix;
        };
        if(i==2||i==3)slider.valueFromTextFunction=[this](const juce::String& text){return text.getDoubleValue()/(longEnvelope.getToggleState()?3:1);};
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
        if(i==12){
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
           &&child!=&previousPreset&&child!=&nextPreset&&child!=&bankButton&&child!=&noiseType&&child!=&noiseLevel)mainControls.push_back(child);
    addAndMakeVisible(mainTab);
    addAndMakeVisible(fxTab);
    mainTab.setTooltip("Oscillators, envelopes, Motion, macros and sampler");
    fxTab.setTooltip("Open the effects and equaliser page");
    mainTab.onClick=[this]{showPage(0);};
    fxTab.onClick=[this]{showPage(1);};
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
    for(size_t osc=0;osc<2;++osc){
        for(auto* button:{&previousWave[osc],&nextWave[osc]})addAndMakeVisible(*button);
        previousWave[osc].setButtonText("<");nextWave[osc].setButtonText(">");
        previousWave[osc].setName("Osc "+juce::String(int(osc+1))+" previous waveform");
        nextWave[osc].setName("Osc "+juce::String(int(osc+1))+" next waveform");
        const auto step=[this,osc](int direction){auto& selector=osc==0?waveform:waveform2;const int count=selector.getNumItems();
            const int index=selector.getSelectedItemIndex();selector.setSelectedItemIndex((index+direction+count)%count,juce::sendNotificationSync);};
        previousWave[osc].onClick=[step]{step(-1);};nextWave[osc].onClick=[step]{step(1);};
        previousWave[osc].setTooltip("Previous oscillator waveform");nextWave[osc].setTooltip("Next oscillator waveform");
    }
    addAndMakeVisible(extendedButton);extendedButton.setTooltip("Filter ADSR, granular, arpeggiator/sequencer, wavetable editor and convolver");
    const juce::Component::SafePointer<SpaceInfinityEditor> safeEditor(this);
    extendedButton.onClick=[this,safeEditor]{openPopup(std::make_unique<ExtendedPanel>(processor,[safe=safeEditor]{juce::MessageManager::callAsync([safe]{if(safe){safe->popup.reset();safe->popupContent.reset();safe->showPage(3);}});}),extendedButton.getBounds());};
    studioRack=std::make_unique<StudioRack>(processor);addAndMakeVisible(*studioRack);studioRack->onOpenGate=[this]{showPage(2);};
    gatePage=std::make_unique<rhythmui::GatePage>(processor);sequencerPage=std::make_unique<rhythmui::SequencerPage>(processor);addAndMakeVisible(*gatePage);addAndMakeVisible(*sequencerPage);addAndMakeVisible(gateTab);addAndMakeVisible(sequenceTab);gateTab.onClick=[this]{showPage(2);};sequenceTab.onClick=[this]{showPage(3);};
    for(int i=0;i<3;++i){auto& b=pageLfoSources[size_t(i)];b.setButtonText("LFO "+juce::String(i+1)+" +");b.description="INFINITY:LFO:"+juce::String(i);b.setTooltip("Drag to a control here, or click to edit this LFO on MAIN.");b.onClick=[this,i]{showPage(0);workbench->selectSource(i);};addAndMakeVisible(b);}
    sampleDisplay=std::make_unique<SamplerDisplay>(processor,false);addAndMakeVisible(*sampleDisplay);
    liveDisplay=std::make_unique<SamplerDisplay>(processor,true);addAndMakeVisible(*liveDisplay);
    title.setVisible(false);
    workbench=std::make_unique<MainWorkbench>(processor,[this](std::unique_ptr<juce::Component> c,juce::Rectangle<int> r){openPopup(std::move(c),r);});addAndMakeVisible(*workbench);
    subPanel=std::make_unique<SubPanel>(processor);addAndMakeVisible(*subPanel);addAndMakeVisible(masterReadout);
    for(int i=0;i<2;++i){auto& button=unisonButtons[size_t(i)];button.setButtonText("UNI");button.setName("Oscillator "+juce::String(i+1)+" unison options");button.setTooltip("Unison mode, stacking, range, phase, random phase and frame/warp spread");button.onClick=[this,i]{openPopup(std::make_unique<UnisonPanel>(processor,i),unisonButtons[size_t(i)].getBounds());};addAndMakeVisible(button);}
    for(int i=0;i<2;++i){auto& knob=oscillatorPans[size_t(i)];const auto id="osc"+juce::String(i+1)+"Pan";knob.initialiseModulation(processor,id);knob.setName("Oscillator "+juce::String(i+1)+" pan");knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);knob.setTextBoxStyle(juce::Slider::TextBoxBelow,false,55,18);attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,id,knob));knob.setDoubleClickReturnValue(true,0);knob.textFromValueFunction=[](double v){return std::abs(v)<.005?juce::String("C"):juce::String(std::abs(v)*100,0)+(v<0?" L":" R");};knob.valueFromTextFunction=[](const juce::String& t){return t.containsIgnoreCase("C")?0:t.getDoubleValue()*(t.containsIgnoreCase("L")?-1:1)/100;};knob.updateText();addAndMakeVisible(knob);}
    for(int i=0;i<2;++i){auto& knob=wtPositions[size_t(i)];const juce::String id=i==0?"wavetableScan":"osc2WTPos";knob.initialiseModulation(processor,id);knob.setName("Oscillator "+juce::String(i+1)+" wavetable position");knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);knob.setTextBoxStyle(juce::Slider::TextBoxBelow,false,55,18);attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,id,knob));knob.textFromValueFunction=[this,i](double v){int wave=int(processor.state.getRawParameterValue(i==0?"wave":"osc2Wave")->load());if(i==1)wave=wave==0?int(processor.state.getRawParameterValue("wave")->load()):wave-1;return juce::String(1+v*(wave==30?juce::jmax(0,processor.tableFrameCount()-1):16),1);};knob.valueFromTextFunction=[this,i](const juce::String& text){int wave=int(processor.state.getRawParameterValue(i==0?"wave":"osc2Wave")->load());if(i==1)wave=wave==0?int(processor.state.getRawParameterValue("wave")->load()):wave-1;return (text.getDoubleValue()-1)/juce::jmax(1,wave==30?processor.tableFrameCount()-1:16);};knob.onDragStart=[this]{processor.state.getParameter("independentWTPos")->setValueNotifyingHost(1);};knob.onValueChange=[this,&knob]{if(knob.hasKeyboardFocus(true))processor.state.getParameter("independentWTPos")->setValueNotifyingHost(1);};knob.setMouseDragSensitivity(300);knob.setDoubleClickReturnValue(true,0);knob.setTooltip("Wavetable frame position, starting at frame 1. Drag an LFO or macro here to sweep frames.");knob.updateText();addAndMakeVisible(knob);}
    for(int i=0;i<2;++i){auto& gain=oscillatorGains[size_t(i)];const auto id="osc"+juce::String(i+1)+"Gain";gain.initialiseModulation(processor,id);gain.setName("Oscillator "+juce::String(i+1)+" gain");gain.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);gain.setTextBoxStyle(juce::Slider::TextBoxBelow,false,55,18);gain.setMouseDragSensitivity(300);gain.setDoubleClickReturnValue(true,0);attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,id,gain));gain.textFromValueFunction=[](double v){return juce::String(v,1)+" dB";};gain.valueFromTextFunction=[](const juce::String& t){return t.getDoubleValue();};gain.updateText();addAndMakeVisible(gain);gainLabels[size_t(i)].setText("GAIN",juce::dontSendNotification);gainLabels[size_t(i)].setJustificationType(juce::Justification::centred);addAndMakeVisible(gainLabels[size_t(i)]);}
    const char* routeIds[]{"routeA","routeB","routeSub","routeNoise"};const char* routeNames[]{"A","B","SUB","NOISE"};for(int i=0;i<4;++i){auto& b=filterRoutes[size_t(i)];b.setButtonText(routeNames[i]);b.setTooltip("Route "+juce::String(routeNames[i])+" through the filter; off bypasses the filter.");addAndMakeVisible(b);routeBindings.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.state,routeIds[i],b));}
    updatePage();
    // Text boxes are constructed before their sliders acquire this parent.
    // Refresh cached child artwork once the complete control tree is attached.
    sendLookAndFeelChange();
    startTimerHz(30);
}
SpaceInfinityEditor::~SpaceInfinityEditor(){
    stopTimer();
    popup.reset();popupContent.reset();
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
void SpaceInfinityEditor::openPopup(std::unique_ptr<juce::Component> content,juce::Rectangle<int> target){
    popup.reset();popupContent.reset();
    popupContent=std::move(content);
    popup=std::make_unique<juce::CallOutBox>(*popupContent,target,this);
    popup->enterModalState(true,nullptr,false);
}
void SpaceInfinityEditor::openPresetManager(){
    processor.presetBank().reload();refreshPresetMenu();
    const juce::Component::SafePointer<SpaceInfinityEditor> safe(this);
    openPopup(std::make_unique<PresetPanel>(processor,[safe]{if(safe!=nullptr)safe->refreshPresetMenu();}),bankButton.getBounds());
}
void SpaceInfinityEditor::refreshPresetMenu(){
    presets.clear(juce::dontSendNotification);
    const auto& entries=processor.presetBank().entries();juce::String category;for(size_t i=0;i<entries.size();++i){if(entries[i].category!=category){category=entries[i].category;presets.addSectionHeading(category);}presets.addItem(entries[i].name,int(i+1));}
    presets.setTooltip(processor.state.state.getProperty("factoryReference").toString()+"\n"+processor.state.state.getProperty("factoryDescription").toString());
    displayedPresetKey=processor.selectedPresetKey();const int index=processor.presetBank().indexForKey(displayedPresetKey);
    if(index>=0)presets.setSelectedItemIndex(index,juce::dontSendNotification);else if(displayedPresetKey=="HYBRID")presets.setText(processor.state.state.getProperty("presetDisplayName").toString(),juce::dontSendNotification);
}
void SpaceInfinityEditor::timerCallback(){
    if(displayedPresetKey!=processor.selectedPresetKey())refreshPresetMenu();
    const float peak=processor.takeOutputPeak();
    meterPeak=juce::jmax(peak,meterPeak*0.88f);
    const float targetLogoLevel=peak>.00003f?juce::jlimit(0.0f,1.0f,(juce::Decibels::gainToDecibels(peak)+54.0f)/54.0f):0.0f;
    logoLevel+=(targetLogoLevel-logoLevel)*(targetLogoLevel>logoLevel?.75f:.22f);if(logoLevel<.002f)logoLevel=0;

    clipHold=peak>=1.0f?30:juce::jmax(0,clipHold-1);
    const bool sampled=processor.state.getRawParameterValue("source")->load()>.5f;
    if(sampleDisplay)sampleDisplay->setVisible(!fxPage&&sampled);
    if(liveDisplay)liveDisplay->setVisible(!fxPage&&sampled);
    if(subPanel)subPanel->setVisible(!fxPage&&!sampled);
    for(auto* c:std::array<juce::Component*,6>{&sampleRoot,&sampleTrim[0],&sampleTrim[1],&sampleLoop,&sampleReverse,&importButton})c->setVisible(!fxPage&&sampled);
    masterReadout.peak=meterPeak;masterReadout.clip=clipHold>0;masterReadout.repaint();
    refreshModulationIndicators();
    repaint();
}
void SpaceInfinityEditor::updatePage(){
    for(auto* control:mainControls)control->setVisible(!fxPage);
    for(size_t i=0;i<2;++i){previousWave[i].setVisible(!fxPage);nextWave[i].setVisible(!fxPage);}
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
    for(size_t i=0;i<rackControls.size();++i){rackControls[i].setVisible(false);rackLabels[i].setVisible(false);}
    for(auto& power:rackEnabled)power.setVisible(false);
    delaySync.setVisible(false);delayMode.setVisible(false);reverbSpace.setVisible(false);
    if(studioRack)studioRack->setVisible(fxPage&&auxiliaryPage==0);
    if(gatePage)gatePage->setVisible(auxiliaryPage==1);if(sequencerPage)sequencerPage->setVisible(auxiliaryPage==2);for(auto& b:pageLfoSources)b.setVisible(fxPage);eqButton.setVisible(fxPage&&auxiliaryPage==0);
    const bool sampled=processor.state.getRawParameterValue("source")->load()>.5f;
    if(sampleDisplay)sampleDisplay->setVisible(!fxPage&&sampled);
    if(liveDisplay)liveDisplay->setVisible(!fxPage&&sampled);

    knobs[12].setSliderStyle(fxPage?juce::Slider::LinearHorizontal:juce::Slider::LinearVertical);
    octaveControls[0].setVisible(true);pitchWheel.setVisible(true);portamento.setVisible(!fxPage);portamentoLabel.setVisible(!fxPage);
    knobs[12].setVisible(true);
    labels[12].setVisible(true);
    knobs[10].setVisible(false);labels[10].setVisible(false);
    knobs[11].setVisible(false);labels[11].setVisible(false);
    motionDepth.setVisible(false);
    updateLfoRateVisibility();
    for(int i:{2,3,4,13,14}){knobs[size_t(i)].setVisible(false);labels[size_t(i)].setVisible(false);}for(auto& m:macros)m.setVisible(false);
    for(auto* c:std::array<juce::Component*,8>{&motionShape,&motionBars,&lfoMode,&lfoSync,&lfoSpeed,&lfoPower,&longEnvelope,&vibratoPower})c->setVisible(false);
    for(auto* c:std::array<juce::Component*,6>{&analogueCharacter,&analogueLabel,&preDrive,&preDriveLabel,&analogueOn,&analogueLabButton})c->setVisible(!fxPage);
    if(workbench){workbench->setVisible(!fxPage);workbench->toFront(false);}if(subPanel)subPanel->setVisible(!fxPage&&!sampled);
    for(auto* c:std::array<juce::Component*,6>{&sampleRoot,&sampleTrim[0],&sampleTrim[1],&sampleLoop,&sampleReverse,&importButton})c->setVisible(!fxPage&&sampled);
    for(auto& c:oscillatorGains)c.setVisible(!fxPage);for(auto& c:wtPositions)c.setVisible(!fxPage);for(auto& c:oscillatorPans)c.setVisible(!fxPage);for(auto& c:unisonButtons)c.setVisible(!fxPage);for(auto& c:gainLabels)c.setVisible(!fxPage);for(auto& c:filterRoutes)c.setVisible(!fxPage);
    mainTab.setToggleState(!fxPage,juce::dontSendNotification);
    fxTab.setToggleState(fxPage&&auxiliaryPage==0,juce::dontSendNotification);gateTab.setToggleState(auxiliaryPage==1,juce::dontSendNotification);sequenceTab.setToggleState(auxiliaryPage==2,juce::dontSendNotification);
    resized();
    refreshModulationIndicators();
    repaint();
}
void SpaceInfinityEditor::bindLfoControls(){
    lfoPowerBinding.reset();
    lfoPowerBinding=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.state,processor.lfoParameterId(selectedLfo,"Enabled"),lfoPower);
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
    if(fxPage)return {};
    if(target==25)return pitchWheel.getBounds();
    if(target==26||target==27)return !fxPage?(target==26?warpA.getBounds():warpB.getBounds()):juce::Rectangle<int>{};
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
    return envelopeDrag>=0?envelopeDragDuration:juce::jmax(.5f,float((knobs[2].getValue()+knobs[3].getValue())*(longEnvelope.getToggleState()?3:1)+knobs[13].getValue())+.4f);
}
std::array<juce::Point<float>,3> SpaceInfinityEditor::envelopeNodes() const{
    const float duration=envelopeDuration();
    const float attack=float(knobs[2].getValue())*(longEnvelope.getToggleState()?3:1),decay=float(knobs[13].getValue());
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
        if(fxTab.getBounds().contains(e.getPosition())&&!fxPage){showPage(1);}
        if(mainTab.getBounds().contains(e.getPosition())&&fxPage){showPage(0);}
        hoverTarget=targetAt(e.getPosition());
        repaint();
    }else if(envelopeDrag>=0){
        const auto set=[this](const char* id,float value){
            auto* p=processor.state.getParameter(id);
            p->setValueNotifyingHost(p->convertTo0to1(value));
        };
        const float time=juce::jlimit(0.0f,envelopeDragDuration,
            float(e.x-envelopeGraph.getX())/float(envelopeGraph.getWidth())*envelopeDragDuration);
        const float scale=longEnvelope.getToggleState()?3.0f:1.0f;
        if(envelopeDrag==0)set("attack",juce::jlimit(0.0f,1.0f,time/scale));
        if(envelopeDrag==1){
            set("ampDecay",juce::jlimit(.005f,8.0f,time-float(knobs[2].getValue())*scale));
            set("ampSustain",juce::jlimit(0.0f,1.0f,
                float(envelopeGraph.getBottom()-4-e.y)/float(envelopeGraph.getHeight()-8)));
        }
        if(envelopeDrag==2)
            set("release",juce::jlimit(0.0f,1.0f,(time-float(knobs[2].getValue())*scale-float(knobs[13].getValue())-.4f)/scale));
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
    if(hardwareFace.getWidth()!=getWidth()||hardwareFace.getHeight()!=getHeight()||cachedFxPage!=fxPage){
        hardwareFace=juce::Image(juce::Image::RGB,getWidth(),getHeight(),false);
        juce::Graphics surface(hardwareFace);
        HardwareTheme::chassis(surface,getLocalBounds());
        if(!fxPage){
            const auto layout=mainLayout(getWidth(),getHeight());
            for(const auto region:{layout.oscillators[0],layout.oscillators[1],layout.filter,layout.macros,layout.envelope,layout.motion})
                HardwareTheme::module(surface,region.toFloat(),region.getY()==layout.macros.getY());
            drawInset(surface,{28.0f,float(getHeight()-69),float(getWidth()-56),60.0f});
        }
        cachedFxPage=fxPage;
    }
    g.drawImageAt(hardwareFace,0,0);
    if(infinityLogo.isValid()){
        g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
        const int logoWidth=juce::jmin(250,getWidth()-540);const int placement=juce::RectanglePlacement::xLeft|juce::RectanglePlacement::yMid;
        g.setOpacity(1);g.drawImageWithin(logoShadow,53,24,logoWidth,58,placement,false);
        g.drawImageWithin(infinityLogo,52,22,logoWidth,58,placement,false);
        if(logoLevel>0){
            juce::Graphics::ScopedSaveState illuminated(g);
            g.reduceClipRegion(juce::Rectangle<int>(52,22,juce::roundToInt(logoWidth*logoLevel),58));
            g.setOpacity(.2f+.75f*logoLevel);g.drawImageWithin(logoLight,52,22,logoWidth,58,placement,false);
        }
    }

    {const juce::Rectangle<float> glass{54,80,float(juce::jmin(230,getWidth()-560)),3};g.setColour(juce::Colour(0xff140d20));g.fillRoundedRectangle(glass.expanded(1),2);if(logoLevel>0){HardwareTheme::signalGradient(g,glass,.25f);g.fillRoundedRectangle(glass.withWidth(glass.getWidth()*logoLevel).expanded(1),2);HardwareTheme::signalGradient(g,glass);g.fillRoundedRectangle(glass.withWidth(glass.getWidth()*logoLevel),1);}}
    g.setFont(10.0f);
    g.setColour(juce::Colour(0xffb7ab9b));
    g.drawText("PROGRAM MEMORY",getWidth()-330,24,200,15,juce::Justification::centredLeft);
    g.drawText("NOISE",getWidth()-330,73,58,22,juce::Justification::centredLeft);
    g.setFont(8);g.drawText(juce::String(pitchWheel.getValue(),1)+" st",370,58,26,14,juce::Justification::centred);g.setFont(10);
    g.drawText("MASTER OCT",getWidth()-397,116,72,22,juce::Justification::right);

    if(fxPage){
    }else{
        const auto layout=mainLayout(getWidth(),getHeight());
        const auto parameter=[this](const char* id){return processor.state.getRawParameterValue(id)->load();};
        const int waveA=int(parameter("wave"));
        const int waveBChoice=int(parameter("osc2Wave"));
        const int waves[]{waveA,waveBChoice==0?waveA:waveBChoice-1};
        const int modes[]{int(parameter("osc1Warp")),int(parameter("osc2Warp"))};
        const float amounts[]{parameter("osc1WarpAmount"),parameter("osc2WarpAmount")};
        const double tune[]{std::exp2(parameter("osc1Octave")+parameter("osc1Semitone")/12.0+parameter("osc1Tune")/1200.0),
            std::exp2(parameter("osc2Octave")+parameter("osc2Semitone")/12.0+parameter("osc2Tune")/1200.0)*(1.0+(parameter("manualUnison")>.5f?0:parameter("detune")))};
        const float scans[]{parameter("wavetableScan"),parameter("independentWTPos")>.5f?parameter("osc2WTPos"):parameter("wavetableScan")};
        for(int oscillator=0;oscillator<2;++oscillator){
            const auto bounds=layout.oscillators[oscillator];
            const auto screen=juce::Rectangle<int>(bounds.getX()+13,bounds.getY()+36,bounds.getWidth()-26,80);
            HardwareTheme::display(g,screen.toFloat());const bool active=parameter(oscillator==0?"osc1Enabled":"osc2Enabled")>.5f;
            const auto drawFrame=[&](float scan,bool selected){
                const float depth=scan*22;const int frames=waves[oscillator]==30?processor.tableFrameCount():17;const float frame=scan*juce::jmax(0,frames-1);const auto tableA=processor.tableFrame(int(frame)),tableB=processor.tableFrame(int(frame)+1);
                const auto preview=[&](double phase){if(waves[oscillator]!=30)return SpaceInfinityProcessor::oscillatorFrameSample(phase,waves[oscillator],scan);phase-=std::floor(phase);const double x=phase*infinity::Wavetable::points;const int i=int(x)%infinity::Wavetable::points,j=(i+1)%infinity::Wavetable::points;const auto read=[&](const auto& table){return table[size_t(i)]+float(x-i)*(table[size_t(j)]-table[size_t(i)]);};return read(tableA)+(read(tableB)-read(tableA))*(frame-std::floor(frame));};
                juce::Path path;for(int point=0;point<=192;++point){double phase=double(point)/192;const int mode=modes[oscillator];const float amount=amounts[oscillator];const float mod=mode==4?std::sin(float(phase*.5/tune[oscillator])*juce::MathConstants<float>::twoPi):SpaceInfinityProcessor::oscillatorFrameSample(phase*tune[1-oscillator]/tune[oscillator],waves[1-oscillator]==30?2:waves[1-oscillator],scans[1-oscillator]);
                    if(mode==1||mode==4)phase+=mod*amount*1.5;else if(mode==2){const double x=phase-std::floor(phase);phase=x+((1-std::abs(x*2-1))-x)*amount;}else if(mode==3){const double x=phase-std::floor(phase),split=.5+.45*amount;phase=x<split?.5*x/split:.5+.5*(x-split)/(1-split);}
                    const float x=screen.getX()+5+depth+float(point)/192*(screen.getWidth()-34);const float y=screen.getCentreY()+15-depth-preview(phase)*18;if(point==0)path.startNewSubPath(x,y);else path.lineTo(x,y);}
                if(selected){g.setColour(juce::Colour(0x446d39bf));g.strokePath(path,juce::PathStrokeType(5));}g.setColour(juce::Colour(selected?0xfff0ddff:0xff8b63bf).withAlpha(active?(selected?1.0f:.38f):.15f));g.strokePath(path,juce::PathStrokeType(selected?1.8f:.8f));
            };
            for(int layer=8;layer>=0;--layer)drawFrame(float(layer)/8,false);drawFrame(scans[oscillator],true);
            g.setColour(juce::Colour(0xffcdb6e3));g.setFont(9);const int frames=waves[oscillator]==30?processor.tableFrameCount():17;g.drawText("FRAME "+juce::String(1+scans[oscillator]*juce::jmax(0,frames-1),1)+" / "+juce::String(frames),screen.reduced(4).removeFromBottom(12),juce::Justification::centredLeft);
            const auto label=[&g](const juce::Component& control,const char* text){
                g.setColour(juce::Colour(0xffb9b1a2));
                g.drawText(text,control.getBounds().translated(0,-15).withHeight(13),juce::Justification::centred);
            };
            label(oscillatorSliders[size_t(oscillator*2)],"FINE");
            label(oscillatorSliders[size_t(oscillator*2+1)],"WIDTH");
            label(unisonSliders[size_t(oscillator)],"VOICES");label(oscillatorPans[size_t(oscillator)],"PAN");
            label(unisonTone[size_t(oscillator*2)],"DETUNE");label(unisonTone[size_t(oscillator*2+1)],"BLEND");label(wtPositions[size_t(oscillator)],"WT POS");
            const auto octave=octaveControls[size_t(oscillator+1)].getBounds();
        g.drawText("OCT",octave.getX()-28,octave.getY(),25,octave.getHeight(),juce::Justification::right);
        }
        g.setColour(juce::Colour(0xffb9b1a2));
        g.drawText("FILTER",layout.filter.reduced(9,4).withWidth(65).withHeight(17),juce::Justification::centredLeft);
        g.drawText("PERFORMANCE",layout.macros.reduced(8,3).withHeight(17),juce::Justification::centredLeft);
        g.setColour(juce::Colour(selectedSource==7?0xff665172:0xff353b3c));
        HardwareTheme::switchBody(g,envelopeTab.toFloat(),false,selectedSource==7);
        g.setColour(juce::Colour(0xffdfd2eb));
        int envelopeRoutes=0;
        for(int slot=0;slot<3;++slot)if(processor.state.getRawParameterValue(processor.modulationRouteId(7,slot,false))->load()>.5f)++envelopeRoutes;
        g.drawText("AMP ENV  ("+juce::String(envelopeRoutes)+") +",envelopeTab,juce::Justification::centredLeft);
        for(size_t lfo=0;lfo<lfoTabs.size();++lfo){
            g.setColour(juce::Colour(selectedLfo==int(lfo)?0xff294f59:0xff303738));
            HardwareTheme::switchBody(g,lfoTabs[lfo].toFloat(),false,selectedLfo==int(lfo));
            g.setColour(juce::Colour(0xffc9e0e5));
            int count=0;
            for(int slot=0;slot<3;++slot)if(processor.state.getRawParameterValue(processor.modulationRouteId(int(lfo)+4,slot,false))->load()>.5f)++count;
            const bool enabled=processor.state.getRawParameterValue(processor.lfoParameterId(int(lfo),"Enabled"))->load()>.5f;
            g.drawText("LFO "+juce::String(int(lfo+1))+(enabled?" ("+juce::String(count)+") +":juce::String(" OFF")),lfoTabs[lfo],juce::Justification::centred);
        }
        const auto names=SpaceInfinityProcessor::macroNames();
        for(size_t i=0;i<macros.size();++i){
            const auto handle=macros[i].getBounds().translated(0,-17).withHeight(17);
            g.setColour(juce::Colour(draggedMacro==int(i)?0xff735344:0xff363b39));
            // Silk-screened macro legend; the + remains its drag handle.
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
        HardwareTheme::display(g,envelope.toFloat());
        const float attack=float(knobs[2].getValue())*(longEnvelope.getToggleState()?3:1),decay=float(knobs[13].getValue());
        const float sustain=float(knobs[14].getValue()),release=float(knobs[3].getValue());
        const float total=juce::jmax(.5f,attack+decay+release+.4f);
        const auto envPoint=[&envelope,total](float time,float level){
            return juce::Point<float>(envelope.getX()+envelope.getWidth()*time/total,envelope.getBottom()-4-(envelope.getHeight()-8)*level);
        };
        juce::Path env;
        env.startNewSubPath(envPoint(0,0));
        const bool curved=parameter("analogueUpgrade")>.5f;
        for(int segment=0;segment<3;++segment)for(int i=1;i<=40;++i){
            const float t=float(i)/40,curve=curved?AnalogueEnvelope::charge(t):t;
            if(segment==0)env.lineTo(envPoint(attack*t,curve));
            if(segment==1)env.lineTo(envPoint(attack+decay*t,1+(sustain-1)*curve));
            if(segment==2)env.lineTo(envPoint(attack+decay+.4f+float(knobs[3].getValue())*t,sustain*(1-curve)));
            if(segment==1&&i==40)env.lineTo(envPoint(attack+decay+.4f,sustain));
        }
        g.setColour(juce::Colour(0xffe3b487));g.strokePath(env,juce::PathStrokeType(1.6f));
        for(const auto point:envelopeNodes())g.fillEllipse(point.x-3,point.y-3,6,6);
        HardwareTheme::display(g,motionGraph.toFloat());
        g.setColour(juce::Colour(0xff303d30));
        for(int i=1;i<5;++i)g.drawVerticalLine(motionGraph.getX()+motionGraph.getWidth()*i/5,float(motionGraph.getY()),float(motionGraph.getBottom()));
        juce::Path motion;
        for(size_t i=0;i<motionPoints.size();++i){
            const auto point=juce::Point<float>(float(motionGraph.getX())+float(motionGraph.getWidth())*float(i)/16.0f,
                float(motionGraph.getBottom())-float(motionPoints[i].getValue())*float(motionGraph.getHeight()));
            if(i==0)motion.startNewSubPath(point);else motion.lineTo(point);
            g.setColour(juce::Colour(0xffdcb17b));g.fillEllipse(point.x-2,point.y-2,4,4);
        }
        g.setColour(juce::Colour(0xffdcb17b));g.strokePath(motion,juce::PathStrokeType(1.5f));
        const int playhead=motionGraph.getX()+int(float(motionGraph.getWidth())*processor.lfoDisplayPhase(selectedLfo));
        g.setColour(juce::Colour(0xfff5d8a3));
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
        g.setColour(juce::Colour(0xffb9b1a2));
        if(parameter("source")>.5f){g.drawText("START",34,getHeight()-66,49,20,juce::Justification::centredLeft);
        g.drawText("END",242,getHeight()-66,37,20,juce::Justification::centredLeft);
        g.drawText("SAMPLER",getWidth()-134,getHeight()-66,99,20,juce::Justification::centredRight);}
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
    subtitle.setBounds(39,79,juce::jmin(450,w-390),24);
    presets.setBounds(w-330,42,226,25);
    previousPreset.setBounds(w-100,42,29,25);
    nextPreset.setBounds(w-67,42,29,25);
    bankButton.setBounds(w-330,72,60,23);
    noiseType.setBounds(w-266,72,90,23);
    noiseLevel.setBounds(w-172,72,134,23);
    mainTab.setBounds(34,111,58,25);fxTab.setBounds(97,111,50,25);gateTab.setBounds(152,111,60,25);sequenceTab.setBounds(217,111,60,25);extendedButton.setBounds(282,111,78,25);
    octaveControls[0].setBounds(w-320,116,52,22);pitchWheel.setBounds(326,23,45,75);pitchSettings.setBounds(327,9,48,17);
    if(gatePage)gatePage->setBounds(28,150,w-56,h-212);if(sequencerPage)sequencerPage->setBounds(28,150,w-56,h-212);
    for(int i=0;i<3;++i)pageLfoSources[size_t(i)].setBounds(210+i*60,h-53,57,28);
    if(studioRack)studioRack->setBounds(28,150,w-56,h-212);
    const auto layout=mainLayout(w,h);
    if(sampleDisplay)sampleDisplay->setBounds(layout.oscillators[0].getX()+10,layout.oscillators[0].getY()+33,layout.oscillators[0].getWidth()-20,86);
    if(liveDisplay)liveDisplay->setBounds(layout.oscillators[1].getX()+10,layout.oscillators[1].getY()+33,layout.oscillators[1].getWidth()-20,86);
    for(int i=0;i<2;++i){
        auto region=layout.oscillators[i].reduced(8);
        auto& power=i==0?osc1:osc2;
        power.setBounds(region.getX(),region.getY(),60,20);
        (i==0?warpA:warpB).setBounds(region.getX()+65,region.getY(),40,20);unisonButtons[size_t(i)].setBounds(region.getX()+109,region.getY(),40,20);
        octaveControls[size_t(i+1)].setBounds(region.getRight()-62,region.getY(),62,20);
        auto& selector=i==0?waveform:waveform2;
        selector.setBounds(region.getX()+28,region.getY()+124,region.getWidth()-56,23);
        previousWave[size_t(i)].setBounds(region.getX(),region.getY()+124,25,23);
        nextWave[size_t(i)].setBounds(region.getRight()-25,region.getY()+124,25,23);
        auto controls=juce::Rectangle<int>(region.getX()+7,region.getY()+169,region.getWidth()-14,52);
        const int controlWidth=controls.getWidth()/4;oscillatorSliders[size_t(i*2)].setBounds(controls.removeFromLeft(controlWidth).reduced(1,0));oscillatorPans[size_t(i)].setBounds(controls.removeFromLeft(controlWidth).reduced(1,0));oscillatorSliders[size_t(i*2+1)].setBounds(controls.removeFromLeft(controlWidth).reduced(1,0));unisonSliders[size_t(i)].setBounds(controls.reduced(1,0));
        auto bottom=juce::Rectangle<int>(region.getX()+2,region.getBottom()-57,region.getWidth()-4,56);const int cell=bottom.getWidth()/4;unisonTone[size_t(i*2)].setBounds(bottom.removeFromLeft(cell).reduced(1,0));unisonTone[size_t(i*2+1)].setBounds(bottom.removeFromLeft(cell).reduced(1,0));wtPositions[size_t(i)].setBounds(bottom.removeFromLeft(cell).reduced(1,0));oscillatorGains[size_t(i)].setBounds(bottom.reduced(1,0));
    }
    const auto characterBounds=layout.oscillators[1];
    analogueLabel.setBounds(characterBounds.getRight()-96,characterBounds.getBottom()-80,90,15);
    analogueLabel.setText("CHARACTER",juce::dontSendNotification);
    analogueCharacter.setBounds(characterBounds.getRight()-92,characterBounds.getBottom()-64,79,62);
    const auto oscBottom=layout.oscillators[0];
    labels[4].setBounds(oscBottom.getRight()-96,oscBottom.getBottom()-80,90,15);
    labels[4].setText("GLOBAL DETUNE",juce::dontSendNotification);
    knobs[4].setBounds(oscBottom.getRight()-92,oscBottom.getBottom()-64,79,62);
    auto filterControls=layout.filter.reduced(7).withTrimmedTop(21);
    filterControls.removeFromRight(52);
    const int gridHeight=(layout.filter.getHeight()-130)/3;
    const int indices[]{0,1,6,5};
    for(int i=0;i<4;++i){
        auto row=juce::Rectangle<int>(filterControls.getX(),filterControls.getY()+(i/2)*gridHeight,filterControls.getWidth(),gridHeight);
        const int half=row.getWidth()/2;
        auto cell=juce::Rectangle<int>(row.getX()+(i%2)*half,row.getY(),half,row.getHeight()).reduced(2,0);
        const auto index=size_t(indices[i]);
        labels[index].setFont(juce::Font(juce::FontOptions(10)));labels[index].setBounds(cell.removeFromTop(10));
        knobs[index].setBounds(cell);
    }
    {auto row=juce::Rectangle<int>(filterControls.getX(),filterControls.getY()+2*gridHeight,filterControls.getWidth(),gridHeight);auto a=row.removeFromLeft(row.getWidth()/2).reduced(2,0);analogueLabel.setFont(juce::Font(juce::FontOptions(9)));analogueLabel.setBounds(a.removeFromTop(10));analogueCharacter.setBounds(a);auto b=row.reduced(2,0);preDriveLabel.setFont(juce::Font(juce::FontOptions(9)));preDriveLabel.setBounds(b.removeFromTop(10));preDrive.setBounds(b);}
    analogueOn.setBounds(layout.filter.getRight()-141,layout.filter.getY()+2,100,18);analogueLabButton.setBounds(layout.filter.getRight()-40,layout.filter.getY()+2,35,18);
    filterMode.setBounds(layout.filter.getX()+8,layout.filter.getBottom()-71,layout.filter.getWidth()-70,22);
    const int voiceWidth=juce::jmax(65,layout.filter.getWidth()-154);
    voiceMode.setBounds(layout.filter.getX()+8,layout.filter.getBottom()-32,voiceWidth,23);
    filterMode.setBounds(layout.filter.getX()+8,layout.filter.getBottom()-71,voiceWidth,22);
    portamentoLabel.setBounds(layout.filter.getX()+voiceWidth+9,layout.filter.getBottom()-77,79,14);portamento.setBounds(layout.filter.getX()+voiceWidth+12,layout.filter.getBottom()-62,72,57);
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
    longEnvelope.setBounds(envelopeTab.removeFromRight(88));
    envelopeGraph=layout.envelope.reduced(8).withTrimmedTop(19).withHeight(juce::jmax(55,layout.envelope.getHeight()/2-22));
    auto tabs=layout.motion.reduced(8,3).withHeight(19);
    const int tabWidth=tabs.getWidth()/3;
    for(auto& tab:lfoTabs)tab=tabs.removeFromLeft(tabWidth).reduced(2,0);
    auto motion=layout.motion.reduced(8).withTrimmedTop(26);
    auto shapeRow=motion.removeFromTop(24);
    lfoPower.setBounds(shapeRow.removeFromRight(80));vibratoPower.setBounds(shapeRow.removeFromRight(90));
    motionShape.setBounds(shapeRow.reduced(2,0));
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
    if(workbench)workbench->setBounds(30,layout.macros.getY(),w-60,layout.macros.getHeight());
    if(subPanel)subPanel->setBounds(194,h-73,juce::jmax(330,w-390),64);
    for(int i=0;i<2;++i){auto r=layout.oscillators[i];gainLabels[size_t(i)].setBounds(oscillatorGains[size_t(i)].getBounds().translated(0,-15).withHeight(13));}
    const int routeWidth=(layout.filter.getWidth()-70)/4;for(int i=0;i<4;++i){const char* names[]{"A","B","SUB","NOISE"};filterRoutes[size_t(i)].setButtonText(routeWidth<52?(i==2?"S":i==3?"N":names[i]):names[i]);filterRoutes[size_t(i)].setBounds(layout.filter.getX()+5+i*routeWidth,layout.filter.getBottom()-100,routeWidth,22);}
    masterReadout.setBounds(fxPage?juce::Rectangle<int>(w-390,h-54,94,30):juce::Rectangle<int>(layout.filter.getRight()-64,layout.filter.getBottom()-28,58,22));
    if(!fxPage)knobs[12].setBounds(layout.filter.getRight()-59,layout.filter.getY()+42,44,layout.filter.getHeight()-76);
    refreshModulationIndicators();
}
juce::AudioProcessorEditor* SpaceInfinityProcessor::createEditor(){return new SpaceInfinityEditor(*this);}

void SpaceInfinityEditor::showPage(int page){auxiliaryPage=page>=2?page-1:0;fxPage=page!=0;updatePage();}
void SpaceInfinityEditor::itemDragMove(const juce::DragAndDropTarget::SourceDetails& d){if(fxTab.getBounds().contains(d.localPosition))showPage(1);else if(mainTab.getBounds().contains(d.localPosition))showPage(0);else if(gateTab.getBounds().contains(d.localPosition))showPage(2);else if(sequenceTab.getBounds().contains(d.localPosition))showPage(3);else if(eqButton.isVisible()&&eqButton.getBounds().contains(d.localPosition)&&(!popupContent||popupContent->getName()!="Seven-band EQ"))openPopup(std::make_unique<EqPanel>(processor),eqButton.getBounds());}
