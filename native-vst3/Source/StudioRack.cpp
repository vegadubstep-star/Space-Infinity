#include "StudioRack.h"
#include "StudioParameters.h"
#include "EffectControls.h"
#include <complex>
namespace {
using SliderBinding=juce::AudioProcessorValueTreeState::SliderAttachment;
using ChoiceBinding=juce::AudioProcessorValueTreeState::ComboBoxAttachment;
using ButtonBinding=juce::AudioProcessorValueTreeState::ButtonAttachment;
class Knob final : public juce::Component {
public:
    Knob(SpaceInfinityProcessor& p,const juce::String& id,const juce::String& text):binding(p.state,id,slider){
        label.setText(text,juce::dontSendNotification);label.setJustificationType(juce::Justification::centred);label.setFont(juce::Font(juce::FontOptions(10)));
        slider.setName(text);slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,78,16);
        slider.setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
        juce::String unit;
        for(const auto& c:studioControls)if(id==c.id)unit=c.unit;
        for(const auto& c:effectControls)if(id==c.id)unit=c.unit;
        if(id.containsIgnoreCase("attack")||id.containsIgnoreCase("release"))unit="ms";
        if(id=="gateLow"||id=="gateHigh")unit="Hz";
        if(id.contains("Depth")||id=="convolverMix"||id=="gateDepth")unit="%";
        if(id=="sidechainThreshold")unit="dB";
        slider.textFromValueFunction=[unit](double value){return juce::String(unit=="%"?value*100:value,unit=="Hz"||unit=="%"?0:unit=="s"?3:1)+(unit.isEmpty()?"":" "+unit);};
        slider.valueFromTextFunction=[unit](const juce::String& s){return s.getDoubleValue()/(unit=="%"?100:1);};
        auto* parameter=p.state.getParameter(id);slider.setDoubleClickReturnValue(true,parameter->convertFrom0to1(parameter->getDefaultValue()));
        slider.setTooltip(text+" — drag to adjust; Shift for fine adjustment; double-click resets.");
        slider.updateText();
        addAndMakeVisible(label);addAndMakeVisible(slider);
    }
    void resized()override{label.setBounds(0,0,getWidth(),13);slider.setBounds(0,13,getWidth(),getHeight()-13);}
    juce::Slider slider;juce::Label label;SliderBinding binding;
};
class Choice final : public juce::Component {
public:
    Choice(SpaceInfinityProcessor& p,const char* id,const char* text):binding(p.state,id,box){
        auto* parameter=dynamic_cast<juce::AudioParameterChoice*>(p.state.getParameter(id));if(parameter)box.addItemList(parameter->choices,1);
        box.setSelectedItemIndex(int(p.state.getRawParameterValue(id)->load()),juce::dontSendNotification);
        box.setTooltip(text);addAndMakeVisible(box);
    }
    void resized()override{box.setBounds(getLocalBounds());}
    juce::ComboBox box;ChoiceBinding binding;
};
class Switch final : public juce::Component {
public:
    Switch(SpaceInfinityProcessor& p,const char* id,const char* text):binding(p.state,id,button){button.setButtonText(text);addAndMakeVisible(button);}
    void resized()override{button.setBounds(getLocalBounds());}
    juce::ToggleButton button;ButtonBinding binding;
};
class Module : public juce::Component {
public:
    Module(SpaceInfinityProcessor& p,juce::String name,const char* enable):processor(p),title(std::move(name)),power(p,enable,""){addAndMakeVisible(power);setName(title);}
    void paint(juce::Graphics& g)override{
        HardwareTheme::module(g,getLocalBounds().toFloat(),false);
        juce::ColourGradient violet(juce::Colour(0xff62418b),0,0,juce::Colour(0xff251c38),0,29,false);violet.addColour(.18,juce::Colour(0xff946fb7));
        g.setGradientFill(violet);g.fillRect(1,1,getWidth()-2,28);
        g.setColour(juce::Colour(0xffeee1cf));g.setFont(juce::Font(juce::FontOptions(12,juce::Font::bold)));g.drawText(title,38,2,180,26,juce::Justification::centredLeft);
        power.setBounds(8,3,28,24);
    }
    Knob& knob(const char* id,const char* name){auto k=std::make_unique<Knob>(processor,id,name);auto& result=*k;addAndMakeVisible(*k);knobs.push_back(std::move(k));return result;}
    Choice& choice(const char* id,const char* text){auto k=std::make_unique<Choice>(processor,id,text);auto& result=*k;addAndMakeVisible(*k);choices.push_back(std::move(k));return result;}
    Switch& toggle(const char* id,const char* text){auto k=std::make_unique<Switch>(processor,id,text);auto& result=*k;addAndMakeVisible(*k);switches.push_back(std::move(k));return result;}
    void row(juce::Rectangle<int> area,int first,int count){const int w=area.getWidth()/juce::jmax(1,count);for(int i=0;i<count;++i)knobs[size_t(first+i)]->setBounds(area.getX()+i*w,area.getY(),w,area.getHeight());}
    SpaceInfinityProcessor& processor;juce::String title;Switch power;
    std::vector<std::unique_ptr<Knob>> knobs;std::vector<std::unique_ptr<Choice>> choices;std::vector<std::unique_ptr<Switch>> switches;
};
class CompGraph final : public juce::Component {
public:
    explicit CompGraph(SpaceInfinityProcessor& p):processor(p){setTooltipText();}
    void paint(juce::Graphics& g)override{
        HardwareTheme::display(g,getLocalBounds().toFloat().reduced(3));const auto levels=processor.compressorLevels(),reduction=processor.compressorReduction();
        if(processor.state.getRawParameterValue("compMode")->load()<.5f){g.setColour(juce::Colour(0xffdac6ed));g.setFont(10);g.drawText("LEGACY DEPTH MODE",getLocalBounds(),juce::Justification::centred);return;}
        for(int i=0;i<3;++i){auto r=juce::Rectangle<int>(8,10+i*(getHeight()-20)/3,getWidth()-16,(getHeight()-20)/3-5);
            const float db=juce::Decibels::gainToDecibels(levels[size_t(i)],-60.0f),fill=juce::jlimit(0.0f,1.0f,(db+60)/60);
            g.setColour(juce::Colour(0xff58b2a4));g.fillRect(r.withWidth(int(r.getWidth()*fill)));
            g.setColour(juce::Colour(0xffe5a27c));g.fillRect(r.withY(r.getBottom()-4).withHeight(3).withWidth(int(r.getWidth()*juce::jlimit(0.0f,1.0f,reduction[size_t(i)]/36))));
            const char* ids[]{"compLowOffset","compMidOffset","compHighOffset"};const float threshold=processor.state.getRawParameterValue("compThreshold")->load()+processor.state.getRawParameterValue(ids[i])->load();
            const int x=r.getX()+int(r.getWidth()*juce::jlimit(0.0f,1.0f,(threshold+60)/60));g.setColour(juce::Colour(0xfff2dfb9));g.drawVerticalLine(x,float(r.getY()),float(r.getBottom()));
            g.setFont(10);g.drawText(juce::String(i==0?"L ":i==1?"M ":"H ")+juce::String(reduction[size_t(i)],1)+" dB GR",r,juce::Justification::centredLeft);
        }
    }
    void mouseDown(const juce::MouseEvent& e)override{band=juce::jlimit(0,2,e.y*3/juce::jmax(1,getHeight()));parameter()->beginChangeGesture();initial=parameter()->convertFrom0to1(parameter()->getValue());}
    void mouseDrag(const juce::MouseEvent& e)override{if(band<0)return;parameter()->setValueNotifyingHost(parameter()->convertTo0to1(juce::jlimit(-24.0f,24.0f,initial-float(e.getDistanceFromDragStartY())*.3f)));}
    void mouseUp(const juce::MouseEvent&)override{if(band>=0)parameter()->endChangeGesture();band=-1;}
    ~CompGraph()override{if(band>=0)parameter()->endChangeGesture();}
private:
    SpaceInfinityProcessor& processor;int band=-1;float initial=0;
    juce::RangedAudioParameter* parameter(){const char* ids[]{"compLowOffset","compMidOffset","compHighOffset"};return processor.state.getParameter(ids[band]);}
    void setTooltipText(){setName("Compressor band meters — drag each lane to adjust its threshold");}
};
class CompressorModule final : public Module {
public:
    explicit CompressorModule(SpaceInfinityProcessor& p):Module(p,"MULTIBAND COMPRESSOR","compEnabled"),graph(p){
        choice("compMode","Compressor mode");toggle("compLimit","PEAK CEILING");
        const char* ids[]{"compThreshold","compRatio","compAttack","compRelease","compGain","compMix","compXLow","compBelow","compXHigh","compLowGain","compMidGain","compHighGain","compOutput","compAmount"};
        const char* names[]{"THRESH","RATIO","ATTACK","RELEASE","GAIN","MIX","X-LOW","BELOW","X-HIGH","LOW","MID","HIGH","LEVEL","LEGACY DEPTH"};
        for(int i=0;i<14;++i)knob(ids[i],names[i]);addAndMakeVisible(graph);
    }
    void resized()override{choices[0]->setBounds(getWidth()-290,4,135,22);switches[0]->setBounds(getWidth()-150,4,145,22);
        const bool legacy=processor.state.getRawParameterValue("compMode")->load()<.5f;knobs[13]->setVisible(legacy);
        row({8,38,getWidth()-250,77},0,6);row({8,126,getWidth()-16,79},6,legacy?8:7);graph.setBounds(getWidth()-235,40,222,73);}
    CompGraph graph;
};
class DelayGraph final : public juce::Component {
public:
    explicit DelayGraph(SpaceInfinityProcessor& p):processor(p){}
    void paint(juce::Graphics& g)override{
        const auto r=getLocalBounds().toFloat().reduced(3);HardwareTheme::display(g,r);
        const float centre=value("echoFrequency"),span=std::exp2(.15f+value("echoBandwidth")*6);juce::Path curve;
        for(int i=0;i<=int(r.getWidth());++i){const float hz=20*std::pow(1000.0f,float(i)/r.getWidth());const float hp=hz/std::sqrt(hz*hz+(centre/span)*(centre/span));const float lp=centre*span/std::sqrt(hz*hz+centre*span*centre*span);
            const float yy=r.getBottom()-juce::jlimit(0.0f,1.0f,hp*lp)*(r.getHeight()-12);if(i==0)curve.startNewSubPath(r.getX(),yy);else curve.lineTo(r.getX()+i,yy);}
        auto fill=curve;fill.lineTo(r.getBottomRight());fill.lineTo(r.getBottomLeft());fill.closeSubPath();
        g.setGradientFill(juce::ColourGradient(juce::Colour(0x996e70db),0,r.getY(),juce::Colour(0x104d3080),0,r.getBottom(),false));g.fillPath(fill);
        g.setColour(juce::Colour(0xffc9b1ec));g.strokePath(curve,juce::PathStrokeType(1.5f));
        if(spectrum){const auto scope=processor.outputScope(true);std::array<float,1024> bins{};for(int i=0;i<512;++i)bins[size_t(i)]=scope[size_t(i)]*(.5f-.5f*std::cos(juce::MathConstants<float>::twoPi*i/511));fft.performFrequencyOnlyForwardTransform(bins.data());
            juce::Path trace;for(int x=0;x<int(r.getWidth());++x){const float hz=20*std::pow(1000.0f,x/r.getWidth());const int bin=juce::jlimit(1,255,int(hz*512/(processor.getSampleRate()>0?processor.getSampleRate():48000.0)));
                const float db=juce::Decibels::gainToDecibels(bins[size_t(bin)]/128,-90.0f);const float y=r.getBottom()-juce::jlimit(0.0f,1.0f,(db+90)/90)*r.getHeight();if(x==0)trace.startNewSubPath(r.getX(),y);else trace.lineTo(r.getX()+x,y);}
            g.setColour(juce::Colour(0x99eecba0));g.strokePath(trace,juce::PathStrokeType(.8f));}
        g.setFont(9);g.setColour(juce::Colour(0xffd8c6e8));g.drawText(spectrum?"FILTER / OUTPUT SPECTRUM":"DRAG: FILTER FREQ / WIDTH",getLocalBounds().reduced(7).withHeight(12),juce::Justification::centredLeft);
    }
    void mouseDown(const juce::MouseEvent& e)override{for(const char* id:{"echoFrequency","echoBandwidth"})processor.state.getParameter(id)->beginChangeGesture();dragging=true;mouseDrag(e);}
    void mouseDrag(const juce::MouseEvent& e)override{if(!dragging)return;set("echoFrequency",20*std::pow(800.0f,juce::jlimit(0.0f,1.0f,float(e.x)/getWidth())));set("echoBandwidth",1-juce::jlimit(0.0f,1.0f,float(e.y)/getHeight()));repaint();}
    void mouseUp(const juce::MouseEvent&)override{finish();}
    void mouseDoubleClick(const juce::MouseEvent&)override{spectrum=!spectrum;repaint();}
    ~DelayGraph()override{finish();}
private:
    SpaceInfinityProcessor& processor;juce::dsp::FFT fft{9};bool spectrum=false,dragging=false;
    float value(const char* id){return processor.state.getRawParameterValue(id)->load();}
    void set(const char* id,float v){auto* p=processor.state.getParameter(id);p->setValueNotifyingHost(p->convertTo0to1(v));}
    void finish(){if(dragging)for(const char* id:{"echoFrequency","echoBandwidth"})processor.state.getParameter(id)->endChangeGesture();dragging=false;}
};
class DelayModule final : public Module {
public:
    explicit DelayModule(SpaceInfinityProcessor& p):Module(p,"STEREO DELAY","delayEnabled"),graph(p){
        choice("echoMode","Delay routing");choice("echoSyncL","Left BPM / milliseconds");choice("echoSyncR","Right BPM / milliseconds");toggle("echoLink","LINK");toggle("echoHQ","HQ");
        const char* ids[]{"echoTimeL","echoTimeR","echoOffsetL","echoOffsetR","delayFeedback","echoFrequency","echoBandwidth","echoMix","echoOutput","delayTime","delayTone","delay"};
        const char* names[]{"TIME L","TIME R","OFFSET L","OFFSET R","FEEDBACK","FREQ","Q / WIDTH","MIX","LEVEL","LEGACY TIME","LEGACY TONE","LEGACY MIX"};
        for(int i=0;i<12;++i)knob(ids[i],names[i]);addAndMakeVisible(graph);
    }
    void resized()override{choices[0]->setBounds(200,4,145,22);switches[0]->setBounds(getWidth()-155,4,75,22);switches[1]->setBounds(getWidth()-80,4,70,22);
        row({8,39,getWidth()-275,78},0,4);graph.setBounds(getWidth()-257,40,245,94);
        const bool legacy=processor.state.getRawParameterValue("echoMode")->load()<.5f,linked=processor.state.getRawParameterValue("echoLink")->load()>.5f;
        for(int i=9;i<12;++i)knobs[size_t(i)]->setVisible(legacy);
        knobs[0]->setEnabled(processor.state.getRawParameterValue("echoSyncL")->load()<.5f);knobs[1]->setEnabled(!linked&&processor.state.getRawParameterValue("echoSyncR")->load()<.5f);knobs[3]->setEnabled(!linked);choices[2]->setEnabled(!linked);
        choices[1]->setBounds(12,121,110,22);choices[2]->setBounds(128,121,110,22);row({8,151,getWidth()-16,76},4,legacy?8:5);}
    DelayGraph graph;
};
class ConvolverModule final : public Module,public juce::FileDragAndDropTarget {
public:
    explicit ConvolverModule(SpaceInfinityProcessor& p):Module(p,"CONVOLVER","convolverEnabled"){
        choice("irPreset","Impulse library");choice("irSync","Pre-delay BPM / milliseconds");toggle("irMinimumPhase","MIN PHASE");toggle("irReverse","REVERSE");
        const char* ids[]{"irSize","irTone","irPredelay","irAttack","irDecay","irDamp","irGain","convolverMix","irOutput"};
        const char* names[]{"SIZE","TONE","PRE-DELAY","ATTACK","DECAY","DAMP","IR GAIN","MIX","LEVEL"};for(int i=0;i<9;++i)knob(ids[i],names[i]);
        for(auto* b:{&previous,&next,&load})addAndMakeVisible(*b);
        previous.onClick=[this]{step(-1);};next.onClick=[this]{step(1);};load.onClick=[this]{open();};
        setName("CONVOLVER — drop a WAV impulse here; imported IR is embedded in presets");
    }
    bool isInterestedInFileDrag(const juce::StringArray& files)override{return files.size()==1&&files[0].endsWithIgnoreCase(".wav");}
    void filesDropped(const juce::StringArray& files,int,int)override{if(files.size())import(juce::File(files[0]));}
    void paint(juce::Graphics& g)override{Module::paint(g);const auto r=impulseArea().toFloat();HardwareTheme::display(g,r);
        const auto data=processor.impulseView();juce::Path wave;wave.startNewSubPath(r.getBottomLeft());
        for(int i=0;i<512;++i)wave.lineTo(r.getX()+r.getWidth()*i/511,r.getBottom()-juce::jlimit(0.0f,1.0f,(juce::Decibels::gainToDecibels(data[size_t(i)],-72.0f)+72)/72)*(r.getHeight()-18));
        wave.lineTo(r.getBottomRight());wave.closeSubPath();g.setGradientFill(juce::ColourGradient(juce::Colour(0xffb7d890),0,r.getY(),juce::Colour(0x303e5a4a),0,r.getBottom(),false));g.fillPath(wave);
        g.setColour(juce::Colour(0xffdedbc3));g.setFont(10);g.drawText(processor.impulseBuilding()?"PREPARING IMPULSE...":processor.impulseName(),impulseArea().reduced(5).withHeight(13),juce::Justification::centredLeft);
        g.drawText("IR embedded in saved preset / project",12,150,getWidth()/3,17,juce::Justification::centredLeft);
    }
    void resized()override{choices[0]->setBounds(150,4,175,22);previous.setBounds(330,4,24,22);next.setBounds(357,4,24,22);load.setBounds(386,4,77,22);
        switches[0]->setBounds(getWidth()-230,4,115,22);switches[1]->setBounds(getWidth()-115,4,110,22);
        row({getWidth()/3+20,42,getWidth()*2/3-30,82},0,6);row({getWidth()/3+20,137,getWidth()*2/3-30,79},6,3);choices[1]->setBounds(12,176,getWidth()/3-10,24);}
private:
    juce::TextButton previous{"<"},next{">"},load{"LOAD IR"};std::unique_ptr<juce::FileChooser> chooser;
    juce::Rectangle<int> impulseArea()const{return {12,42,getWidth()/3-5,99};}
    void step(int direction){auto& box=choices[0]->box;box.setSelectedItemIndex((box.getSelectedItemIndex()+direction+5)%5,juce::sendNotificationSync);}
    void import(const juce::File& file){const auto r=processor.importImpulse(file);if(r.failed())juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,"Impulse response",r.getErrorMessage());repaint();}
    void open(){chooser=std::make_unique<juce::FileChooser>("Load impulse response",juce::File{},"*.wav");juce::Component::SafePointer<ConvolverModule> safe(this);
        chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,[safe](const juce::FileChooser& c){if(safe&&c.getResult()!=juce::File{})safe->import(c.getResult());});}
};
class RhythmModule final : public Module {
public:
    explicit RhythmModule(SpaceInfinityProcessor& p):Module(p,"TRANCE GATE / SIDECHAIN","gateEnabled"){
        toggle("pumpEnabled","SIDECHAIN");choice("gateSplit","Affected frequency range");choice("gateRate","Gate step duration");choice("pumpRate","Pump cycle");choice("sidechainSource","Internal pump / external host input");
        presets.addItemList(juce::StringArray(studio::gateNames.data(),int(studio::gateNames.size())),1);presets.setTextWhenNothingSelected("15 gate presets");presets.onChange=[this]{processor.loadGatePreset(presets.getSelectedId()-1);};addAndMakeVisible(presets);
        for(int i=0;i<16;++i){auto slider=std::make_unique<juce::Slider>();slider->setSliderStyle(juce::Slider::LinearVertical);slider->setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);slider->setTooltip("Gate step "+juce::String(i+1));addAndMakeVisible(*slider);
            steps.push_back(std::move(slider));stepBindings.push_back(std::make_unique<SliderBinding>(processor.state,"gateStep"+juce::String(i),*steps.back()));}
        const char* ids[]{"gateDepth","gateAttack","gateRelease","pumpDepth","pumpAttack","pumpRelease","sidechainThreshold","sidechainRatio","gateLow","gateHigh"};
        const char* names[]{"GATE DEPTH","ATTACK","RELEASE","DUCK DEPTH","SC ATTACK","SC RELEASE","SC THRESH","SC RATIO","SPLIT LOW","SPLIT HIGH"};for(int i=0;i<10;++i)knob(ids[i],names[i]);
    }
    void paint(juce::Graphics& g)override{Module::paint(g);const int step=processor.gateStep();g.setColour(juce::Colour(0xffd6bbf5));if(step<int(steps.size()))g.drawRect(steps[size_t(step)]->getBounds().expanded(1),1);}
    void resized()override{presets.setBounds(230,4,180,22);switches[0]->setBounds(getWidth()-135,4,130,22);
        const int w=(getWidth()-24)/16;for(int i=0;i<16;++i)steps[size_t(i)]->setBounds(12+i*w,40,w-3,63);
        const int cw=(getWidth()-24)/4;for(int i=0;i<4;++i)choices[size_t(i)]->setBounds(12+i*cw,111,cw-6,23);
        row({8,145,getWidth()-16,80},0,10);}
private:
    juce::ComboBox presets;std::vector<std::unique_ptr<juce::Slider>> steps;std::vector<std::unique_ptr<SliderBinding>> stepBindings;
};
class BasicModule final : public Module {
public:
    BasicModule(SpaceInfinityProcessor& p,int row):Module(p,effectRows[size_t(row)],effectEnableIds[size_t(row)]){
        for(const auto& c:effectControls)if(c.row==row)knob(c.id,c.label);
        if(row==5)choice("reverbSpace","Reverb space");
    }
    void resized()override{const int width=getWidth()-(choices.empty()?12:145);row({8,37,width,75},0,int(knobs.size()));if(!choices.empty())choices[0]->setBounds(getWidth()-135,61,125,24);}
};
}
struct StudioRack::Impl {
    explicit Impl(SpaceInfinityProcessor& p){
        add(std::make_unique<CompressorModule>(p),215);add(std::make_unique<DelayModule>(p),238);add(std::make_unique<ConvolverModule>(p),226);add(std::make_unique<RhythmModule>(p),237);
        for(int row:{1,2,3,5})add(std::make_unique<BasicModule>(p,row),119);
        viewport.setViewedComponent(&content,false);viewport.setScrollBarsShown(true,false);viewport.setScrollBarThickness(10);
    }
    void add(std::unique_ptr<Module> m,int height){content.addAndMakeVisible(*m);heights.push_back(height);modules.push_back(std::move(m));}
    juce::Component content;juce::Viewport viewport;std::vector<std::unique_ptr<Module>> modules;std::vector<int> heights;
};
StudioRack::StudioRack(SpaceInfinityProcessor& p):impl(std::make_unique<Impl>(p)){addAndMakeVisible(impl->viewport);setName("Studio FX rack");startTimerHz(20);}
StudioRack::~StudioRack(){stopTimer();}
void StudioRack::resized(){impl->viewport.setBounds(getLocalBounds());int y=0;const int width=juce::jmax(1,getWidth()-13);
    for(size_t i=0;i<impl->modules.size();++i){impl->modules[i]->setBounds(0,y,width,impl->heights[i]);y+=impl->heights[i]+8;}impl->content.setSize(width,y);}
void StudioRack::timerCallback(){if(!isShowing())return;for(auto& m:impl->modules){if(auto* c=dynamic_cast<CompressorModule*>(m.get())){c->resized();c->graph.repaint();}if(auto* d=dynamic_cast<DelayModule*>(m.get())){d->resized();d->graph.repaint();}if(dynamic_cast<ConvolverModule*>(m.get())||dynamic_cast<RhythmModule*>(m.get()))m->repaint();}}
juce::Component* StudioRack::module(int index)const{return index>=0&&index<int(impl->modules.size())?impl->modules[size_t(index)].get():nullptr;}
