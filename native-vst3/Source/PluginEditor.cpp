#include "PluginEditor.h"
#include <array>
SpaceInfinityEditor::SpaceInfinityEditor(SpaceInfinityProcessor& p):AudioProcessorEditor(&p),processor(p){
    setSize(940,550);setResizable(true,true);setResizeLimits(760,450,1500,950);
    title.setText("SPACE  INFINITY",juce::dontSendNotification);title.setFont(juce::Font(juce::FontOptions(30.0f,juce::Font::bold)));title.setColour(juce::Label::textColourId,juce::Colours::white);addAndMakeVisible(title);
    subtitle.setText("HYBRID HARDWARE-INSPIRED SYNTHESIS",juce::dontSendNotification);subtitle.setColour(juce::Label::textColourId,juce::Colour(0xffc0c0c8));addAndMakeVisible(subtitle);
    for(int i=0;i<SpaceInfinityProcessor::presetNames().size();++i)presets.addItem(SpaceInfinityProcessor::presetNames()[i],i+1);
    presets.setTextWhenNothingSelected("Select a sound");presets.onChange=[this]{processor.loadPreset(presets.getSelectedItemIndex());};addAndMakeVisible(presets);
    waveform.addItemList({"Saw","Pulse","Sine","Triangle"},1);addAndMakeVisible(waveform);
    waveAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.state,"wave",waveform);
    const char* ids[]={"cutoff","res","attack","release","detune","mix","drive","chorus","delay","reverb","lforate","lfodepth","gain"};
    const char* texts[]={"CUTOFF","RESONANCE","ATTACK","RELEASE","DETUNE","LAYER","DRIVE","CHORUS","DELAY","REVERB","LFO RATE","LFO DEPTH","OUTPUT"};
    for(int i=0;i<13;i++){
        auto& slider=knobs[i];slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,70,19);
        slider.setColour(juce::Slider::rotarySliderFillColourId,juce::Colour(0xffe84953));slider.setColour(juce::Slider::thumbColourId,juce::Colours::white);
        slider.setColour(juce::Slider::textBoxTextColourId,juce::Colours::white);slider.setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
        addAndMakeVisible(slider);
        auto& label=labels[i];label.setText(texts[i],juce::dontSendNotification);label.setJustificationType(juce::Justification::centred);label.setColour(juce::Label::textColourId,juce::Colour(0xffc0c0ca));addAndMakeVisible(label);
        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,ids[i],slider));
    }
}
void SpaceInfinityEditor::paint(juce::Graphics& g){
    g.fillAll(juce::Colour(0xff15161b));
    auto r=getLocalBounds().toFloat().reduced(14);
    g.setColour(juce::Colour(0xff24252d));g.fillRoundedRectangle(r,12);
    g.setColour(juce::Colour(0xffe84953));g.fillRoundedRectangle(r.removeFromTop(5),2);
    g.setColour(juce::Colour(0xff343640));g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(14),12,1);
    g.setColour(juce::Colour(0xff101116));g.fillRoundedRectangle(juce::Rectangle<float>(28,125,float(getWidth()-56),float(getHeight()-155)),9);
    g.setColour(juce::Colour(0xffe84953));g.setFont(12);g.drawText("SYNTH  /  MODULATION  /  FX",34,111,350,20,juce::Justification::left);
}
void SpaceInfinityEditor::resized(){
    const int w=getWidth();title.setBounds(34,23,300,40);subtitle.setBounds(35,64,450,24);
    presets.setBounds(w-290,31,252,28);waveform.setBounds(w-290,72,252,26);
    auto area=juce::Rectangle<int>(32,150,w-64,getHeight()-190);const int cols=7,rows=2;
    const int cellW=area.getWidth()/cols,cellH=area.getHeight()/rows;
    for(int i=0;i<13;i++){
        int col=i%cols,row=i/cols;auto cell=juce::Rectangle<int>(area.getX()+col*cellW,area.getY()+row*cellH,cellW,cellH);
        labels[i].setBounds(cell.getX()+4,cell.getY(),cell.getWidth()-8,22);
        knobs[i].setBounds(cell.getX()+5,cell.getY()+22,cell.getWidth()-10,std::max(62,cell.getHeight()-25));
    }
}
juce::AudioProcessorEditor* SpaceInfinityProcessor::createEditor(){return new SpaceInfinityEditor(*this);}