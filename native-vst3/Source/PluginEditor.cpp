#include "PluginEditor.h"
#include <array>
#include <cmath>
namespace {
constexpr int controlTop=260;
struct Panel { const char* title; std::array<int,4> knobs; int count; };
constexpr Panel panels[]={
    {"01   VOICE",{4,5,6,-1},3},{"02   FILTER",{0,1,-1,-1},2},
    {"03   AMP ENVELOPE",{2,13,14,3},4},{"04   LFO",{10,11,-1,-1},2},
    {"05   FX / OUTPUT",{7,8,9,12},4}};
juce::Rectangle<int> panelBounds(int width,int height,int index){
    const int topHeight=(height-494)/2;
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
    addAndMakeVisible(osc1);
    addAndMakeVisible(osc2);
    oscAttachments[0]=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.state,"osc1Enabled",osc1);
    oscAttachments[1]=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.state,"osc2Enabled",osc2);
    waveform.addItemList({"Saw","Pulse","Sine","Triangle"},1);addAndMakeVisible(waveform);
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
    motionDepth.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
    motionDepth.setTooltip("Motion amount to filter cutoff");
    addAndMakeVisible(motionDepth);
    motionDepthAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,"motionDepth",motionDepth);
    for(size_t i=0;i<motionPoints.size();++i){
        auto& point=motionPoints[i];
        point.setSliderStyle(juce::Slider::LinearVertical);
        point.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
        point.setTooltip("Motion point "+juce::String(int(i+1)));
        point.setColour(juce::Slider::thumbColourId,juce::Colour(0xffd78e3f));
        point.setColour(juce::Slider::trackColourId,juce::Colours::transparentBlack);
        point.setColour(juce::Slider::backgroundColourId,juce::Colours::transparentBlack);
        point.onValueChange=[this]{repaint();};
        addAndMakeVisible(point);
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
        fxChoices.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.state,"macroRoute"+suffix,macroRoutes[i]));
    }
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
        trim.setNumDecimalPlacesToDisplay(2);
    }
    bindChoice(delaySync,"delaySync",{"Free delay","1/16","1/8","Dotted 1/8","1/4"});
    bindChoice(delayMode,"delayMode",{"Stereo","Ping-pong"});
    bindChoice(reverbSpace,"reverbSpace",{"Room","Plate","Hall"});
    compAmount.setSliderStyle(juce::Slider::LinearHorizontal);
    compAmount.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
    compAmount.setTooltip("Multiband compression amount");
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
    pitchWheel.setNumDecimalPlacesToDisplay(1);
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
        slider.setTextValueSuffix(units[i]);
        if(i==2 || i==3 || i==13 || i==14){
            slider.setSliderStyle(juce::Slider::LinearVertical);
            slider.setColour(juce::Slider::thumbColourId,juce::Colour(0xffd48d42));
            slider.setColour(juce::Slider::trackColourId,juce::Colour(0xff10100f));
            slider.setColour(juce::Slider::backgroundColourId,juce::Colour(0xff4b4742));
        }
        slider.onValueChange=[this]{repaint();};
    }
}
SpaceInfinityEditor::~SpaceInfinityEditor(){setLookAndFeel(nullptr);}
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
    g.drawText("TRIM START",34,getHeight()-65,66,22,juce::Justification::centredLeft);
    g.drawText("END",245,getHeight()-65,35,22,juce::Justification::centredLeft);
    g.drawText("PITCH",getWidth()-316,116,51,22,juce::Justification::right);
    const auto motionPanel=juce::Rectangle<int>(26,getHeight()-243,getWidth()-52,119);
    g.setColour(juce::Colour(0xff0b0b0d));
    g.fillRoundedRectangle(motionPanel.toFloat(),5.0f);
    g.setColour(juce::Colour(0xff4d4243));
    g.drawRoundedRectangle(motionPanel.toFloat(),5.0f,1.0f);
    g.setColour(juce::Colour(0xffd9a6a1));
    g.drawText("MOTION  /  FILTER CUTOFF",34,motionPanel.getY()+2,155,16,juce::Justification::centredLeft);
    juce::Path motionTrace;
    for(size_t i=0;i<motionPoints.size();++i){
        const auto bounds=motionPoints[i].getBounds();
        const float x=float(bounds.getCentreX());
        const float y=float(bounds.getBottom()-9)-float(motionPoints[i].getValue())*float(bounds.getHeight()-18);
        g.setColour(juce::Colour(0xff342123));
        g.drawVerticalLine(int(x),float(bounds.getY()+9),float(bounds.getBottom()-9));
        if(i==0)motionTrace.startNewSubPath(x,y);else motionTrace.lineTo(x,y);
    }
    g.setColour(juce::Colour(0xffe6a14e));
    g.strokePath(motionTrace,juce::PathStrokeType(1.5f));
    g.setColour(juce::Colour(0xffc3c1bd));
    const auto macroNames=SpaceInfinityProcessor::macroNames();
    for(size_t i=0;i<macros.size();++i){
        const auto slider=macros[i].getBounds();
        g.drawText(macroNames[int(i)],slider.translated(0,-13).withHeight(14),juce::Justification::centred);
    }
    g.drawText("SIGNAL PATH",34,116,350,20,juce::Justification::left);
    drawInset(g,juce::Rectangle<float>(float(getWidth()-344),22.0f,318.0f,81.0f));
    g.setFont(10.0f);
    g.setColour(juce::Colour(0xffb5a6a0));
    g.drawText("FACTORY PROGRAM",getWidth()-330,25,110,13,juce::Justification::centredLeft);
    for(int i=0;i<int(std::size(panels));++i){
        const auto panel=panelBounds(getWidth(),getHeight(),i);
        drawInset(g,panel.toFloat());
        g.setColour(juce::Colour(0xffb7b1a8));
        g.drawText(panels[i].title,panel.reduced(8,2).withHeight(16),juce::Justification::centredLeft);
    }
    g.setFont(12.0f);
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
                const float waveforms[]={phase*2.0f-1.0f,phase<0.48f?1.0f:-1.0f,
                    std::sin(juce::MathConstants<float>::twoPi*phase),4.0f*std::abs(phase-0.5f)-1.0f};
                y-=0.43f*waveforms[juce::jlimit(0,3,waveform.getSelectedItemIndex())];
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
    nextPreset.setBounds(w-67,42,29,25);waveform.setBounds(w-330,72,292,23);
    source.setBounds(34,getHeight()-37,152,24);
    importButton.setBounds(194,getHeight()-37,104,24);
    sampleRoot.setBounds(306,getHeight()-37,116,24);
    auto fx=panelBounds(w,getHeight(),4).reduced(8,5).removeFromBottom(24);
    const int fxWidth=fx.getWidth()/4;
    delaySync.setBounds(fx.removeFromLeft(fxWidth).reduced(2,0));
    delayMode.setBounds(fx.removeFromLeft(fxWidth).reduced(2,0));
    reverbSpace.setBounds(fx.removeFromLeft(fxWidth).reduced(2,0));
    compAmount.setBounds(fx.reduced(2,0));
    const auto voice=panelBounds(w,getHeight(),0);
    osc1.setBounds(voice.getRight()-112,voice.getY()+1,54,18);
    osc2.setBounds(voice.getRight()-56,voice.getY()+1,54,18);
    pitchWheel.setBounds(w-261,116,220,22);
    auto motionRow=juce::Rectangle<int>(34,getHeight()-227,w-68,98);
    auto controls=motionRow.removeFromLeft(155);
    motionShape.setBounds(controls.removeFromTop(26).reduced(2));
    motionBars.setBounds(controls.removeFromTop(26).reduced(2));
    motionDepth.setBounds(controls.removeFromTop(26).reduced(2));
    const int spacing=motionRow.getWidth()/int(motionPoints.size());
    for(auto& point:motionPoints)point.setBounds(motionRow.removeFromLeft(spacing).reduced(1));
    auto macroRow=juce::Rectangle<int>(34,getHeight()-108,w-68,36);
    const int macroWidth=macroRow.getWidth()/int(macros.size());
    for(size_t i=0;i<macros.size();++i){
        auto cell=macroRow.removeFromLeft(macroWidth).reduced(4,0);
        macros[i].setBounds(cell.removeFromTop(16));
        macroRoutes[i].setBounds(cell);
    }
    sampleTrim[0].setBounds(100,getHeight()-65,130,22);
    sampleTrim[1].setBounds(282,getHeight()-65,130,22);
    sampleLoop.setBounds(425,getHeight()-65,100,22);
    sampleReverse.setBounds(532,getHeight()-65,100,22);
    filterMode.setBounds(642,getHeight()-65,120,22);
    for(int i=0;i<int(std::size(panels));++i){
        const auto& panel=panels[i];
        auto area=panelBounds(w,getHeight(),i).reduced(3,3).withTrimmedTop(17);
        if(i==4) area.removeFromBottom(28);
        for(int j=0;j<panel.count;++j){
            auto cell=area.removeFromLeft(area.getWidth()/(panel.count-j)).reduced(2,0);
            const auto index=size_t(panel.knobs[j]);
            labels[index].setBounds(cell.removeFromTop(17));
            knobs[index].setBounds(cell);
        }
    }
}
juce::AudioProcessorEditor* SpaceInfinityProcessor::createEditor(){return new SpaceInfinityEditor(*this);}
