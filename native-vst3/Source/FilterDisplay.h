#pragma once
#include "PluginProcessor.h"
#include "HardwareTheme.h"
#include "WorkflowControls.h"

namespace filterui {
class ResponsePlot final : public juce::Component, public juce::SettableTooltipClient, private juce::Timer {
public:
    explicit ResponsePlot(SpaceInfinityProcessor& p):processor(p){setTooltip("Drag horizontally for cutoff, vertically for resonance. The bright curve follows the latest sounding voice; the faint curve shows the knob setting.");startTimerHz(25);}
    ~ResponsePlot() override {endGesture();}
    void paint(juce::Graphics& g) override {
        auto bounds=getLocalBounds().toFloat();HardwareTheme::display(g,bounds);
        const auto plot=bounds.reduced(5,4);const bool large=getHeight()>90;
        g.saveState();g.reduceClipRegion(plot.toNearestInt());
        const auto xFor=[&](double hz){return plot.getX()+float(std::log(hz/20)/std::log(1000.0))*plot.getWidth();};
        for(double hz:{100.,1000.,10000.}){g.setColour(juce::Colour(0x2268d9dc));g.drawVerticalLine(int(xFor(hz)),plot.getY(),plot.getBottom());if(large){g.setFont(10);g.drawText(hz==100?"100":hz==1000?"1k":"10k",int(xFor(hz))+3,int(plot.getBottom())-13,30,12,juce::Justification::centredLeft);}}
        infinity::CharacterFilter::Settings settings;settings.mode=processor.filterSelection();settings.cutoff=value("cutoff");settings.resonance=value("res");settings.vowel=value("filterVowel");settings.morph=value("filterMorph");settings.sampleRate=std::max(44100.0,processor.getSampleRate())*2;
        const auto draw=[&](bool live){
            auto s=settings;if(live){s.cutoff=processor.filterDisplayCutoff();s.resonance=processor.filterDisplayResonance();s.vowel=processor.filterDisplayVowel();s.morph=processor.filterDisplayMorph();}
            juce::Path curve;for(int i=0;i<=256;++i){const double hz=20*std::pow(1000.0,i/256.0),m=std::abs(infinity::CharacterFilter::response(hz,s));const float db=float(20*std::log10(std::max(1.e-6,m)));const float x=plot.getX()+plot.getWidth()*i/256.f,y=plot.getY()+(12-std::clamp(db,-48.f,12.f))/60*plot.getHeight();if(!i)curve.startNewSubPath(x,y);else curve.lineTo(x,y);}
            if(live){g.setGradientFill(juce::ColourGradient(juce::Colour(0xffff75d9),plot.getX(),0,juce::Colour(0xff51eff1),plot.getRight(),0,false));g.strokePath(curve,juce::PathStrokeType(1.7f));}
            else{g.setColour(juce::Colour(0x667ab4d4));g.strokePath(curve,juce::PathStrokeType(1));}
        };draw(false);draw(true);
        if(large){g.setColour(juce::Colour(0xffb5d5da));g.setFont(11);g.drawText(juce::String(processor.filterDisplayCutoff(),0)+" Hz   "+(settings.mode==5||settings.mode==6?"Circuit response (approx.)":"Filter response"),plot.toNearestInt().removeFromTop(17),juce::Justification::centredLeft);}
        g.restoreState();
    }
    void mouseDown(const juce::MouseEvent& e) override {endGesture();cutoff=processor.state.getParameter("cutoff");res=processor.state.getParameter("res");cutoff->beginChangeGesture();res->beginChangeGesture();apply(e);}
    void mouseDrag(const juce::MouseEvent& e) override {apply(e);}
    void mouseUp(const juce::MouseEvent&) override {endGesture();}
private:
    SpaceInfinityProcessor& processor;juce::RangedAudioParameter *cutoff=nullptr,*res=nullptr;
    float value(const char* id) const{return processor.state.getRawParameterValue(id)->load();}
    void timerCallback() override {if(isShowing())repaint();}
    void apply(const juce::MouseEvent& e){if(!cutoff)return;const auto b=getLocalBounds().reduced(5,4);const float x=juce::jlimit(0.f,1.f,float(e.x-b.getX())/juce::jmax(1,b.getWidth()));const float hz=juce::jlimit(80.f,16000.f,20*std::pow(1000.f,x));cutoff->setValueNotifyingHost(cutoff->convertTo0to1(hz));res->setValueNotifyingHost(juce::jlimit(0.f,1.f,1-float(e.y-b.getY())/juce::jmax(1,b.getHeight())));}
    void endGesture(){if(cutoff)cutoff->endChangeGesture();if(res)res->endChangeGesture();cutoff=res=nullptr;}
};
class Details final : public juce::Component {
public:
    explicit Details(SpaceInfinityProcessor& p):plot(p){setLookAndFeel(&theme);addAndMakeVisible(plot);const char* ids[]{"filterVowel","filterMorph"};const char* names[]{"VOWEL  A · E · I · O · U","MORPH  LP · BP · HP"};for(int i=0;i<2;++i){auto& k=knobs[size_t(i)];k.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);k.setTextBoxStyle(juce::Slider::TextBoxBelow,false,80,18);k.initialiseModulation(p,ids[i]);addAndMakeVisible(k);labels[size_t(i)].setText(names[i],juce::dontSendNotification);labels[size_t(i)].setJustificationType(juce::Justification::centred);addAndMakeVisible(labels[size_t(i)]);attachments[size_t(i)]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,ids[i],k);k.setEnabled(p.filterSelection()==(i==0?11:14));}help.setText("Cutoff shifts the formants / comb tuning. Resonance controls vowel focus / comb feedback. Drag an LFO or macro onto either active shape knob.",juce::dontSendNotification);help.setFont(juce::Font(juce::FontOptions(12)));addAndMakeVisible(help);setSize(460,340);}
    ~Details() override {setLookAndFeel(nullptr);}
    void paint(juce::Graphics& g) override {g.fillAll(juce::Colour(0xff231a32));}
    void resized() override {plot.setBounds(10,10,getWidth()-20,175);for(int i=0;i<2;++i){labels[size_t(i)].setBounds(10+i*220,191,220,20);knobs[size_t(i)].setBounds(50+i*220,211,140,80);}help.setBounds(14,294,getWidth()-28,42);}
private:
    HardwareTheme theme;ResponsePlot plot;std::array<ModSlider,2> knobs;std::array<juce::Label,2> labels;juce::Label help;std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,2> attachments;
};
class FilterDisplay final : public juce::Component {
public:
    explicit FilterDisplay(SpaceInfinityProcessor& p,std::function<void(std::unique_ptr<juce::Component>,juce::Rectangle<int>)> open={}):processor(p),plot(p){setName("Main filter response");addAndMakeVisible(plot);addAndMakeVisible(expand);expand.setButtonText("EDIT");expand.setTooltip("Large response graph and formant / morph shape controls");expand.onClick=[this,open=std::move(open)]{if(open)open(std::make_unique<Details>(processor),expand.getBounds());};}
    void resized() override {plot.setBounds(getLocalBounds());expand.setBounds(getWidth()-43,2,41,18);}
private:SpaceInfinityProcessor& processor;ResponsePlot plot;juce::TextButton expand;
};
}
