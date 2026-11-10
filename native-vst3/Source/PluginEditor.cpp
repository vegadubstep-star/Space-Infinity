#include "PluginEditor.h"
#include <array>
namespace {
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
        g.setColour(juce::Colour(0xff60d9ff));
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
    g.setColour(juce::Colour(0xff72d5f0));
    g.strokePath(chevron,juce::PathStrokeType(1.4f));
}
SpaceInfinityEditor::SpaceInfinityEditor(SpaceInfinityProcessor& p):AudioProcessorEditor(&p),processor(p){
    setLookAndFeel(&hardwareLook);
    setSize(940,550);setResizable(true,true);setResizeLimits(760,450,1500,950);
    title.setText("SPACE  INFINITY",juce::dontSendNotification);title.setFont(juce::Font(juce::FontOptions(30.0f,juce::Font::bold)));title.setColour(juce::Label::textColourId,juce::Colours::white);addAndMakeVisible(title);
    subtitle.setText("HYBRID HARDWARE-INSPIRED SYNTHESIS",juce::dontSendNotification);subtitle.setColour(juce::Label::textColourId,juce::Colour(0xffc0c0c8));addAndMakeVisible(subtitle);
    for(int i=0;i<SpaceInfinityProcessor::presetNames().size();++i)presets.addItem(SpaceInfinityProcessor::presetNames()[i],i+1);
    presets.setTextWhenNothingSelected("Select a sound");presets.onChange=[this]{processor.loadPreset(presets.getSelectedItemIndex());};addAndMakeVisible(presets);
    waveform.addItemList({"Saw","Pulse","Sine","Triangle"},1);addAndMakeVisible(waveform);
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
    const char* ids[]={"cutoff","res","attack","release","detune","mix","drive","chorus","delay","reverb","lforate","lfodepth","gain"};
    const int decimals[]={0,2,3,3,3,2,2,2,2,2,2,2,2};
    const char* units[]={" Hz",""," s"," s","","","","","",""," Hz","",""};
    const char* texts[]={"CUTOFF","RESONANCE","ATTACK","RELEASE","DETUNE","LAYER","DRIVE","CHORUS","DELAY","REVERB","LFO RATE","LFO DEPTH","OUTPUT"};
    for(int i=0;i<13;i++){
        auto& slider=knobs[i];slider.setName(texts[i]);slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,70,19);
        slider.setColour(juce::Slider::rotarySliderFillColourId,juce::Colour(0xffe84953));slider.setColour(juce::Slider::thumbColourId,juce::Colours::white);
        slider.setColour(juce::Slider::textBoxTextColourId,juce::Colours::white);slider.setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
        addAndMakeVisible(slider);
        auto& label=labels[i];label.setText(texts[i],juce::dontSendNotification);label.setJustificationType(juce::Justification::centred);label.setColour(juce::Label::textColourId,juce::Colour(0xffc0c0ca));addAndMakeVisible(label);
        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,ids[i],slider));
        slider.setNumDecimalPlacesToDisplay(decimals[i]);
        slider.setTextValueSuffix(units[i]);
    }
}
SpaceInfinityEditor::~SpaceInfinityEditor(){setLookAndFeel(nullptr);}
void SpaceInfinityEditor::paint(juce::Graphics& g){
    g.fillAll(juce::Colour(0xff101114));
    const auto chassis=getLocalBounds().toFloat().reduced(8.0f);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff36393e),0,chassis.getY(),
        juce::Colour(0xff1a1c20),0,chassis.getBottom(),false));
    g.fillRoundedRectangle(chassis,4.0f);
    g.setColour(juce::Colour(0xff53565b));
    g.drawRoundedRectangle(chassis,4.0f,1.0f);
    auto rail=chassis.reduced(8.0f).withHeight(85.0f);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff823331),0,rail.getY(),
        juce::Colour(0xff4d1d1c),0,rail.getBottom(),false));
    g.fillRoundedRectangle(rail,3.0f);
    g.setColour(juce::Colour(0xffa15a54));
    g.drawRoundedRectangle(rail,3.0f,1.0f);
    const auto tray=juce::Rectangle<float>(23,112,float(getWidth()-46),float(getHeight()-156));
    g.setColour(juce::Colour(0xff202226));
    g.fillRoundedRectangle(tray,3.0f);
    g.setColour(juce::Colour(0xff111215));
    g.drawRoundedRectangle(tray,3.0f,1.0f);
    g.setColour(juce::Colour(0x07ffffff));
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
    g.drawText("PITCH",getWidth()-316,116,51,22,juce::Justification::right);
    g.drawText("SYNTH  /  MODULATION  /  FX",34,116,350,20,juce::Justification::left);
}
void SpaceInfinityEditor::resized(){
    const int w=getWidth();title.setBounds(34,23,300,40);subtitle.setBounds(35,64,450,24);
    presets.setBounds(w-290,31,252,28);waveform.setBounds(w-290,72,252,26);
    source.setBounds(34,getHeight()-37,152,24);
    importButton.setBounds(194,getHeight()-37,104,24);
    sampleRoot.setBounds(306,getHeight()-37,116,24);
    delaySync.setBounds(426,getHeight()-37,91,24);
    delayMode.setBounds(521,getHeight()-37,76,24);
    reverbSpace.setBounds(601,getHeight()-37,64,24);
    compAmount.setBounds(672,getHeight()-37,55,24);
    pitchWheel.setBounds(w-261,116,220,22);
    auto area=juce::Rectangle<int>(32,150,w-64,getHeight()-190);const int cols=7,rows=2;
    const int cellW=area.getWidth()/cols,cellH=area.getHeight()/rows;
    for(int i=0;i<13;i++){
        int col=i%cols,row=i/cols;auto cell=juce::Rectangle<int>(area.getX()+col*cellW,area.getY()+row*cellH,cellW,cellH);
        labels[i].setBounds(cell.getX()+4,cell.getY(),cell.getWidth()-8,22);
        knobs[i].setBounds(cell.getX()+5,cell.getY()+22,cell.getWidth()-10,std::max(62,cell.getHeight()-25));
    }
}
juce::AudioProcessorEditor* SpaceInfinityProcessor::createEditor(){return new SpaceInfinityEditor(*this);}
