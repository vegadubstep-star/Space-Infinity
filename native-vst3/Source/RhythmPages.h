#pragma once
#include "MainWorkbench.h"
#include "GateBlockEditor.h"
namespace rhythmui {
class Knob:public juce::Component {
public:
    ModSlider slider;juce::Label label;
    juce::AudioProcessorValueTreeState::SliderAttachment binding;
    Knob(SpaceInfinityProcessor& p,const char* id,const char* text,const char* unit=""):binding(p.state,id,slider){
        slider.initialiseModulation(p,id);slider.setName(text);slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,76,17);slider.setMouseDragSensitivity(320);
        const auto suffix=juce::String(unit);slider.textFromValueFunction=[suffix](double v){return juce::String(suffix=="%"?v*100:v,suffix=="%"||suffix=="Hz"?0:2)+(suffix.isEmpty()?"":" "+suffix);};slider.valueFromTextFunction=[suffix](const juce::String& t){return t.getDoubleValue()/(suffix=="%"?100:1);};
        auto* a=p.state.getParameter(id);if(dynamic_cast<juce::AudioParameterInt*>(a)){slider.textFromValueFunction=[](double v){return juce::String(int(v));};slider.valueFromTextFunction=[](const juce::String& text){return double(text.getIntValue());};}slider.setDoubleClickReturnValue(true,a->convertFrom0to1(a->getDefaultValue()));slider.updateText();label.setText(text,juce::dontSendNotification);label.setJustificationType(juce::Justification::centred);label.setFont(juce::Font(juce::FontOptions(10)));addAndMakeVisible(slider);addAndMakeVisible(label);
    }
    void resized()override{auto r=getLocalBounds();label.setBounds(r.removeFromTop(16));slider.setBounds(r);}
};
class GatePage:public juce::Component,private juce::Timer {
    SpaceInfinityProcessor& p;CurveEditor graph;GateBlockEditor blocks;
    juce::ToggleButton enabled{"GATE"},pump{"SIDECHAIN"},snap{"SNAP"},draw{"DRAW"},retrigger{"NOTE RETRIGGER"},noteGate{"NOTE GATE"};
    juce::ComboBox mode,presets,shapes,cycle,stepRate,grid,split,sidechain,pumpRate;
    juce::TextButton copySteps{"USE STEP PATTERN"};
    std::vector<std::unique_ptr<Knob>> timing,routing,basic;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> choices;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttons;
    juce::Rectangle<int> left,right;
    void choice(juce::ComboBox& box,const char* id){if(auto* a=dynamic_cast<juce::AudioParameterChoice*>(p.state.getParameter(id)))box.addItemList(a->choices,1);addAndMakeVisible(box);choices.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,id,box));}
    void toggle(juce::ToggleButton& b,const char* id){addAndMakeVisible(b);buttons.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,id,b));}
    void row(juce::Rectangle<int> r,std::vector<std::unique_ptr<Knob>>& knobs,int from,int n){for(int i=0;i<n;++i)knobs[size_t(from+i)]->setBounds(r.getX()+r.getWidth()*i/n,r.getY(),r.getWidth()/n,r.getHeight());}
    void timerCallback()override{if(isShowing()){graph.repaint();blocks.repaint();}}
public:
    explicit GatePage(SpaceInfinityProcessor& processor):p(processor),graph(p,true),blocks(p){
        setName("Dedicated audio gate and sidechain page");addAndMakeVisible(graph);addAndMakeVisible(blocks);blocks.edited=[this]{presets.setSelectedId(0,juce::dontSendNotification);p.state.state.setProperty("gatePresetName","Custom blocks",nullptr);};toggle(enabled,"gateEnabled");toggle(pump,"pumpEnabled");toggle(retrigger,"gateRetrigger");toggle(noteGate,"gateNoteGate");choice(mode,"gateMode");mode.changeItemText(1,"BLOCKS");mode.changeItemText(2,"CURVE");choice(cycle,"gateCurveRate");choice(stepRate,"gateRate");mode.onChange=[this]{resized();graph.repaint();};choice(split,"gateSplit");choice(sidechain,"sidechainSource");choice(pumpRate,"pumpRate");
        presets.addItemList(juce::StringArray(studio::gateNames.data(),int(studio::gateNames.size())),1);presets.setTextWhenNothingSelected("15 gate presets");presets.onChange=[this]{p.loadGatePreset(presets.getSelectedId()-1);resized();graph.repaint();blocks.repaint();};addAndMakeVisible(presets);
        shapes.addItemList({"Sine","Triangle","Ramp up","Ramp down","Square","Sidechain pump","Random steps"},1);shapes.setTextWhenNothingSelected("Curve shapes...");shapes.onChange=[this]{if(shapes.getSelectedId()>0)graph.shape(shapes.getSelectedId()-1);};addAndMakeVisible(shapes);
        grid.addItemList({"4","8","16","32","64"},1);grid.setSelectedId(3);grid.onChange=[this]{graph.setSnap(1<<(grid.getSelectedId()+1));};addAndMakeVisible(grid);
        snap.onClick=[this]{graph.setSnapEnabled(snap.getToggleState());};draw.onClick=[this]{graph.setPaintMode(draw.getToggleState());};addAndMakeVisible(snap);addAndMakeVisible(draw);addAndMakeVisible(copySteps);copySteps.onClick=[this]{p.useGateCurveFromSteps();graph.repaint();};
        graph.edited=[this]{shapes.setSelectedId(0,juce::dontSendNotification);presets.setSelectedId(0,juce::dontSendNotification);presets.setTextWhenNothingSelected("Custom curve / 15 presets");p.state.state.setProperty("gatePresetName","Custom",nullptr);resized();};
        const char* tids[]{"gateDepth","gatePhase","gatePWM","gateSwing","gateSmooth"};const char* names[]{"GATE DEPTH","PHASE","PWM","SWING","SMOOTH"};for(int i=0;i<5;++i){auto k=std::make_unique<Knob>(p,tids[i],names[i],i==4?"ms":"%");addAndMakeVisible(*k);timing.push_back(std::move(k));}
        const char* basicIds[]{"gateDepth","gateRelease","gateStepWidth","gateSwing","gateStepCount","gateDelay","gateFade"};const char* basicNames[]{"MIX","CONTOUR","WIDTH","SHUFFLE","LENGTH","DELAY","FADE IN"};const char* basicUnits[]{"%","ms","%","%","","s","s"};for(int i=0;i<7;++i){auto k=std::make_unique<Knob>(p,basicIds[i],basicNames[i],basicUnits[i]);addAndMakeVisible(*k);basic.push_back(std::move(k));}
        for(const auto* id:{"gateAttack","gateRelease"}){auto k=std::make_unique<Knob>(p,id,juce::String(id)=="gateAttack"?"ATTACK":"RELEASE","ms");addAndMakeVisible(*k);timing.push_back(std::move(k));}
        const char* rids[]{"gateLow","gateHigh","pumpAttack","pumpRelease","sidechainThreshold","sidechainRatio","pumpDepth","fxOutput3"};const char* rn[]{"SPLIT LOW","SPLIT HIGH","SC ATTACK","SC RELEASE","SC THRESH","SC RATIO","DUCK DEPTH","OUTPUT GAIN"};const char* units[]{"Hz","Hz","ms","ms","dB",":1","%","dB"};for(int i=0;i<8;++i){auto k=std::make_unique<Knob>(p,rids[i],rn[i],units[i]);addAndMakeVisible(*k);routing.push_back(std::move(k));}
        mode.setTooltip("Simple blocks or a freely editable curve.");cycle.setTooltip("BPM-synchronised full curve duration.");split.setTooltip("Process only the chosen frequency band; other frequencies pass through.");sidechain.setTooltip("Internal rhythmic pump or audio from the host's optional sidechain input.");retrigger.setTooltip("Restart the curve from its phase offset on MIDI note-on.");noteGate.setTooltip("Close the audio gate when no MIDI notes are held.");stepRate.setTooltip("BPM-synced speed of each block. CURVE mode shows the full curve duration instead.");startTimerHz(30);
    }
    ~GatePage()override{stopTimer();}
    void resized()override{
        auto r=getLocalBounds().reduced(8);right=r.removeFromRight(210);r.removeFromRight(10);left=r;
        auto top=r.removeFromTop(30);enabled.setBounds(top.removeFromLeft(72));mode.setBounds(top.removeFromLeft(115).reduced(2));presets.setBounds(top.removeFromLeft(juce::jmin(190,top.getWidth()/2)).reduced(2));shapes.setBounds(top.reduced(2));
        auto help=r.removeFromBottom(20);juce::ignoreUnused(help);auto edit=r.removeFromBottom(28);snap.setBounds(edit.removeFromLeft(65));grid.setBounds(edit.removeFromLeft(55).reduced(1));draw.setBounds(edit.removeFromLeft(68));copySteps.setBounds(edit.removeFromRight(137).reduced(2));
        auto midi=r.removeFromBottom(29);cycle.setBounds(midi.removeFromLeft(juce::jmax(96,midi.getWidth()-275)).reduced(2));stepRate.setBounds(cycle.getBounds());cycle.setVisible(mode.getSelectedItemIndex()==1);stepRate.setVisible(mode.getSelectedItemIndex()==0);retrigger.setBounds(midi.removeFromLeft(150));noteGate.setBounds(midi);
        row(r.removeFromBottom(90),timing,0,5);const bool curve=mode.getSelectedItemIndex()==1;for(int i:{1,2})timing[size_t(i)]->setVisible(curve);timing[4]->setEnabled(curve);timing[5]->setBounds(timing[1]->getBounds());timing[6]->setBounds(timing[2]->getBounds());timing[5]->setVisible(!curve);timing[6]->setVisible(!curve);graph.setBounds(r.reduced(2,6));blocks.setBounds(graph.getBounds());graph.setVisible(curve);blocks.setVisible(!curve);for(auto& k:timing)if(!curve)k->setVisible(false);row(juce::Rectangle<int>(r.getX(),r.getBottom(),r.getWidth(),90),basic,0,7);for(auto& k:basic)k->setVisible(!curve);snap.setVisible(curve);draw.setVisible(curve);grid.setVisible(curve);copySteps.setVisible(!curve);
        auto side=right.reduced(5);side.removeFromTop(20);split.setBounds(side.removeFromTop(27));row(side.removeFromTop(78),routing,0,2);pump.setBounds(side.removeFromTop(27));sidechain.setBounds(side.removeFromTop(27));pumpRate.setBounds(side.removeFromTop(27));for(int i=2;i<8;i+=2)row(side.removeFromTop(78),routing,i,2);
    }
    void paint(juce::Graphics& g)override{HardwareTheme::module(g,left.toFloat(),true);HardwareTheme::module(g,right.toFloat());g.setFont(juce::Font(juce::FontOptions(10,juce::Font::bold)));g.setColour(juce::Colour(0xffe9d9fb));g.drawText("AFFECTED FREQUENCIES",right.reduced(10,2).withHeight(18),juce::Justification::centredLeft);g.setFont(9);g.setColour(juce::Colour(0xffbdaccd));g.drawText(mode.getSelectedItemIndex()==0?"Click blocks on/off; drag height. Speed is synced to the host tempo.":"Double-click: point / drag: move / wheel: bend / Shift: draw / Alt: snap",left.reduced(10).withTrimmedTop(left.getHeight()-28).withHeight(18),juce::Justification::centredLeft);}
};
class SequencerPage:public juce::Component,private juce::Timer {
    SpaceInfinityProcessor& p;
    juce::ComboBox mode,order,rate;juce::TextButton reset{"RESET STEPS"};
    std::array<std::unique_ptr<Knob>,3> settings;
    std::array<ModSlider,16> pitch,velocity;std::array<juce::ToggleButton,16> on;std::array<juce::Label,16> numbers;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliders;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttons;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> choices;
    juce::Rectangle<int> steps;int selectedStep=-1;
    void choice(juce::ComboBox& b,const char* id){auto* a=dynamic_cast<juce::AudioParameterChoice*>(p.state.getParameter(id));if(a)b.addItemList(a->choices,1);addAndMakeVisible(b);choices.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,id,b));}
    void timerCallback()override{if(isShowing()){selectedStep=p.sequenceStep();repaint();}}
public:
    explicit SequencerPage(SpaceInfinityProcessor& processor):p(processor){setName("Dedicated note sequencer page");choice(mode,"patternMode");choice(order,"arpOrder");choice(rate,"patternRate");
        const char* ids[]{"arpOctaves","patternGate","sequenceLength"};const char* names[]{"ARP OCTAVES","NOTE LENGTH","STEPS"};for(int i=0;i<3;++i){settings[size_t(i)]=std::make_unique<Knob>(p,ids[i],names[i],i==1?"%":"");addAndMakeVisible(*settings[size_t(i)]);}
        for(int i=0;i<16;++i){const auto suffix=juce::String(i);auto& a=pitch[size_t(i)];auto& v=velocity[size_t(i)];a.initialiseModulation(p,"stepPitch"+suffix);v.initialiseModulation(p,"stepVelocity"+suffix);a.setName("Sequence pitch "+juce::String(i+1));v.setName("Sequence velocity "+juce::String(i+1));a.setSliderStyle(juce::Slider::LinearVertical);a.setTextBoxStyle(juce::Slider::TextBoxBelow,false,38,21);a.setMouseDragSensitivity(250);v.setSliderStyle(juce::Slider::LinearVertical);v.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);v.setTooltip("Step velocity; drag an LFO here to modulate accents.");sliders.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,"stepPitch"+suffix,a));sliders.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,"stepVelocity"+suffix,v));buttons.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,"stepOn"+suffix,on[size_t(i)]));a.setDoubleClickReturnValue(true,0);v.setDoubleClickReturnValue(true,.8);numbers[size_t(i)].setText(juce::String(i+1),juce::dontSendNotification);numbers[size_t(i)].setJustificationType(juce::Justification::centred);for(juce::Component* c:std::array<juce::Component*,4>{&a,&v,&on[size_t(i)],&numbers[size_t(i)]})addAndMakeVisible(c);}
        addAndMakeVisible(reset);reset.onClick=[this]{for(int i=0;i<16;++i)for(int f=0;f<3;++f){auto* a=p.state.getParameter(juce::String(f==0?"stepPitch":f==1?"stepVelocity":"stepOn")+juce::String(i));a->beginChangeGesture();a->setValueNotifyingHost(a->convertTo0to1(f==0?0:f==1?.8f:1));a->endChangeGesture();}};startTimerHz(25);
    }
    ~SequencerPage()override{stopTimer();}
    void resized()override{auto r=getLocalBounds().reduced(14);auto head=r.removeFromTop(34);mode.setBounds(head.removeFromLeft(175).reduced(3));order.setBounds(head.removeFromLeft(150).reduced(3));rate.setBounds(head.removeFromLeft(150).reduced(3));reset.setBounds(head.removeFromRight(130).reduced(3));r.removeFromTop(10);auto controls=r.removeFromTop(88);for(auto& k:settings)k->setBounds(controls.removeFromLeft(135));r.removeFromTop(35);r.removeFromBottom(24);steps=r;const int w=r.getWidth()/16;for(int i=0;i<16;++i){auto cell=juce::Rectangle<int>(r.getX()+i*w,r.getY(),w-2,r.getHeight());numbers[size_t(i)].setBounds(cell.removeFromTop(24));on[size_t(i)].setBounds(cell.removeFromBottom(28).withSizeKeepingCentre(30,24));velocity[size_t(i)].setBounds(cell.removeFromBottom(70));pitch[size_t(i)].setBounds(cell.reduced(2));}}
    void paint(juce::Graphics& g)override{HardwareTheme::module(g,getLocalBounds().toFloat(),true);g.setColour(juce::Colour(0xffdbc6ed));g.setFont(12);g.drawText("Hold notes: arpeggiate a chord, sequence the root, or combine both. Pitch is in semitones.",18,145,getWidth()-36,24,juce::Justification::centredLeft);g.setFont(10);g.drawText("PITCH / VELOCITY / ON-REST     -     This page generates MIDI notes. The GATE page shapes audio.",18,getHeight()-27,getWidth()-36,20,juce::Justification::centredLeft);if(selectedStep>=0&&selectedStep<16){g.setColour(juce::Colour(0xffc289ff));const int w=steps.getWidth()/16;g.drawRoundedRectangle(juce::Rectangle<float>(float(steps.getX()+selectedStep*w),float(steps.getY()),float(w-2),float(steps.getHeight())),3,1.5f);}}
};
}
