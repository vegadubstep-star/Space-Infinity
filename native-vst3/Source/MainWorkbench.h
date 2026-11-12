#pragma once
#include "WorkflowControls.h"
#include "HardwareTheme.h"
class RoutePanel:public juce::Component {
    SpaceInfinityProcessor& p;juce::Viewport view;juce::Component content;std::vector<std::unique_ptr<juce::Component>> controls;std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliders;std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> choices;
public:
    RoutePanel(SpaceInfinityProcessor& processor,int source,bool generic):p(processor){const int count=generic?(source<3?3:16):3;setSize(550,juce::jmin(440,65+count*49));addAndMakeVisible(view);view.setViewedComponent(&content,false);view.setBounds(0,0,getWidth(),getHeight());content.setSize(535,30+count*49);
        auto title=std::make_unique<juce::Label>();title->setText(generic?"Choose a control and signed amount. Drag sources onto supported knobs for quicker assignment.":"Performance source: choose destination and amount.",juce::dontSendNotification);title->setBounds(8,0,520,30);content.addAndMakeVisible(*title);controls.push_back(std::move(title));
        for(int slot=0;slot<count;++slot){auto box=std::make_unique<juce::ComboBox>();box->addItem("Unassigned",1);box->addItemList(generic?p.flexibleTargets():p.macroTargets(),2);box->setBounds(8,32+slot*49,337,32);auto amount=std::make_unique<juce::Slider>();amount->setSliderStyle(juce::Slider::LinearHorizontal);amount->setTextBoxStyle(juce::Slider::TextBoxRight,false,64,25);amount->setBounds(350,32+slot*49,178,32);
            if(generic){box->setSelectedId(p.flexibleSourceTarget(source,slot)+2,juce::dontSendNotification);box->onChange=[this,source,slot,b=box.get()]{p.assignFlexibleSource(source,slot,b->getSelectedId()-2);};const auto id=juce::String(source<3?"freeMacroDepth":"lfoFlexDepth")+juce::String(source<3?source:source-3)+"_"+juce::String(slot);sliders.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,id,*amount));}
            else{box->clear(juce::dontSendNotification);box->addItemList(p.macroTargets(),1);choices.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,p.modulationRouteId(source,slot,false),*box));sliders.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,p.modulationRouteId(source,slot,true),*amount));}
            amount->textFromValueFunction=[](double v){return juce::String(v*100,0)+"%";};amount->valueFromTextFunction=[](const juce::String& t){return t.getDoubleValue()/100;};amount->updateText();content.addAndMakeVisible(*box);content.addAndMakeVisible(*amount);controls.push_back(std::move(box));controls.push_back(std::move(amount));}
    }
};
class CurveEditor:public juce::Component {
    SpaceInfinityProcessor& p;
    bool gate,paintMode=false,snapEnabled=false,painting=false;
    int lfo=0,drag=-1,segment=-1,snap=16;
    float startBend=0;
    juce::StringArray gestures;
    float get(const juce::String& id)const{return p.state.getRawParameterValue(id)->load();}
    void write(const juce::String& id,float v){auto* a=p.state.getParameter(id);a->setValueNotifyingHost(a->convertTo0to1(v));}
    void set(const juce::String& id,float v){auto* a=p.state.getParameter(id);a->beginChangeGesture();write(id,v);a->endChangeGesture();}
    int count()const{return gate?juce::jlimit(2,33,int(get("gateCurveCount"))):17;}
    juce::String yId(int i)const{return gate?"gateCurveY"+juce::String(i):p.lfoPointId(lfo,i);}
    float xAt(int i)const{return gate?get("gateCurveX"+juce::String(i)):float(i)/16;}
    juce::Rectangle<float> graph()const{return getLocalBounds().toFloat().reduced(9,9);}
    juce::Point<float> point(int i)const{auto r=graph();return {r.getX()+xAt(i)*r.getWidth(),r.getBottom()-get(yId(i))*r.getHeight()};}
    int nearest(juce::Point<float> pos)const{int best=-1;float distance=10;for(int i=0;i<count();++i){const float d=pos.getDistanceFrom(point(i));if(d<distance){distance=d;best=i;}}return best;}
    studio::EditableCurve curve()const{studio::EditableCurve c;c.count=count();for(int i=0;i<c.count;++i){c.x[size_t(i)]=xAt(i);c.y[size_t(i)]=get(yId(i));c.bend[size_t(i)]=gate?get("gateCurveBend"+juce::String(i)):0;}return c;}
    void begin(const juce::String& id){if(!gestures.contains(id)){p.state.getParameter(id)->beginChangeGesture();gestures.add(id);}}
    void finish(){for(const auto& id:gestures)p.state.getParameter(id)->endChangeGesture();gestures.clear();drag=segment=-1;painting=false;}
    void ensureCurve(){if(gate&&get("gateMode")<.5f)p.useGateCurveFromSteps();}
    void changed(){if(edited)edited();repaint();}
public:
    std::function<void()> edited;
    CurveEditor(SpaceInfinityProcessor& processor,bool isGate):p(processor),gate(isGate){setName(gate?"Audio gate curve":"LFO curve");setMouseCursor(juce::MouseCursor::CrosshairCursor);}
    ~CurveEditor()override{finish();}
    void selectLfo(int i){finish();lfo=i;repaint();}
    void setSnap(int n){snap=juce::jlimit(2,64,n);repaint();}
    void setSnapEnabled(bool enabled){snapEnabled=enabled;}
    void setPaintMode(bool enabled){paintMode=enabled;}
    void paint(juce::Graphics& g)override{
        const auto r=graph();HardwareTheme::display(g,r);
        g.setColour(juce::Colour(0xff2c2543));for(int i=1;i<snap;++i)g.drawVerticalLine(juce::roundToInt(r.getX()+r.getWidth()*i/snap),r.getY(),r.getBottom());
        for(int i=1;i<4;++i)g.drawHorizontalLine(juce::roundToInt(r.getY()+r.getHeight()*i/4),r.getX(),r.getRight());
        const bool legacy=gate&&get("gateMode")<.5f;
        juce::Path path;const auto c=curve();
        for(int i=0;i<=512;++i){const float x=float(i)/512;const float y=legacy?get("gateStep"+juce::String(juce::jmin(15,int(x*16)))):c.value(x);const float px=r.getX()+x*r.getWidth(),py=r.getBottom()-y*r.getHeight();if(i==0)path.startNewSubPath(px,py);else path.lineTo(px,py);}
        auto fill=path;fill.lineTo(r.getBottomRight());fill.lineTo(r.getBottomLeft());fill.closeSubPath();
        g.setGradientFill(juce::ColourGradient(juce::Colour(0x994f79d8),0,r.getY(),juce::Colour(0x28392777),0,r.getBottom(),false));g.fillPath(fill);
        g.setColour(juce::Colour(0xffbdb1ff));g.strokePath(path,juce::PathStrokeType(1.8f));
        if(!legacy)for(int i=0;i<count();++i){const auto pt=point(i);g.setColour(juce::Colour(i==drag?0xffffffff:0xffd6d1eb));g.fillEllipse(pt.x-3,pt.y-3,6,6);}
        const float phase=gate?p.gatePhaseView():p.lfoDisplayPhase(lfo);g.setColour(juce::Colour(0x99f0dfaa));g.drawVerticalLine(juce::roundToInt(r.getX()+phase*r.getWidth()),r.getY(),r.getBottom());
        if(legacy){g.setFont(10);g.setColour(juce::Colour(0xffded5ee));g.drawText("STEP PATTERN / edit to convert to a curve",r.toNearestInt().reduced(8).withHeight(18),juce::Justification::centredLeft);}
    }
    void mouseDown(const juce::MouseEvent& e)override{
        finish();ensureCurve();painting=!gate||paintMode||e.mods.isShiftDown();
        if(painting&&gate){const auto c=curve();set("gateCurveCount",17);for(int i=0;i<17;++i){set("gateCurveX"+juce::String(i),float(i)/16);set(yId(i),c.value(float(i)/16));set("gateCurveBend"+juce::String(i),0);}}
        if(painting){drag=juce::jlimit(0,16,juce::roundToInt((e.position.x-graph().getX())/graph().getWidth()*16));for(int i=0;i<17;++i)begin(yId(i));}
        else{drag=nearest(e.position);if(drag>=0){begin(yId(drag));if(drag>0&&drag<count()-1)begin("gateCurveX"+juce::String(drag));}
            else{const float x=(e.position.x-graph().getX())/graph().getWidth();segment=0;while(segment<count()-2&&xAt(segment+1)<x)++segment;const auto id="gateCurveBend"+juce::String(segment);startBend=get(id);begin(id);}}
        mouseDrag(e);
    }
    void mouseDrag(const juce::MouseEvent& e)override{
        const auto r=graph();float x=juce::jlimit(0.0f,1.0f,(e.position.x-r.getX())/r.getWidth()),y=1-juce::jlimit(0.0f,1.0f,(e.position.y-r.getY())/r.getHeight());
        if(snapEnabled||e.mods.isAltDown()){x=std::round(x*snap)/snap;y=std::round(y*snap)/snap;}
        if(segment>=0){write("gateCurveBend"+juce::String(segment),juce::jlimit(-1.0f,1.0f,startBend-float(e.getDistanceFromDragStartY())/r.getHeight()*2));}
        else if(drag>=0&&painting){const int next=juce::jlimit(0,16,juce::roundToInt(x*16)),first=drag;const float old=get(yId(first));for(int i=std::min(first,next);i<=std::max(first,next);++i){const float t=first==next?1:float(i-first)/float(next-first);write(yId(i),old+(y-old)*t);}drag=next;}
        else if(drag>=0){if(drag>0&&drag<count()-1)write("gateCurveX"+juce::String(drag),juce::jlimit(xAt(drag-1)+.000001f,xAt(drag+1)-.000001f,x));write(yId(drag),y);}
        changed();
    }
    void mouseUp(const juce::MouseEvent&)override{finish();}
    void mouseDoubleClick(const juce::MouseEvent& e)override{
        finish();ensureCurve();if(!gate){set(yId(juce::jlimit(0,16,juce::roundToInt((e.position.x-graph().getX())/graph().getWidth()*16))),.5f);changed();return;}
        const int old=count(),hit=nearest(e.position);
        if(hit>0&&hit<old-1){for(int i=hit;i<old-1;++i){set("gateCurveX"+juce::String(i),xAt(i+1));set(yId(i),get(yId(i+1)));set("gateCurveBend"+juce::String(i),get("gateCurveBend"+juce::String(i+1)));}set("gateCurveCount",float(old-1));}
        else if(hit<0&&old<33){const auto r=graph();float x=juce::jlimit(.00001f,.99999f,(e.position.x-r.getX())/r.getWidth());if(snapEnabled||e.mods.isAltDown())x=juce::jlimit(.00001f,.99999f,std::round(x*snap)/snap);int at=1;while(at<old-1&&xAt(at)<x)++at;for(int i=old;i>at;--i){set("gateCurveX"+juce::String(i),xAt(i-1));set(yId(i),get(yId(i-1)));set("gateCurveBend"+juce::String(i),get("gateCurveBend"+juce::String(i-1)));}set("gateCurveX"+juce::String(at),x);set(yId(at),1-juce::jlimit(0.0f,1.0f,(e.position.y-r.getY())/r.getHeight()));set("gateCurveBend"+juce::String(at),0);set("gateCurveCount",float(old+1));}
        changed();
    }
    void mouseWheelMove(const juce::MouseEvent& e,const juce::MouseWheelDetails& wheel)override{if(!gate)return;ensureCurve();const float x=(e.position.x-graph().getX())/graph().getWidth();int at=0;while(at<count()-2&&xAt(at+1)<x)++at;const auto id="gateCurveBend"+juce::String(at);set(id,juce::jlimit(-1.0f,1.0f,get(id)+wheel.deltaY*.5f));changed();}
    void shape(int index){finish();if(gate){set("gateCurveCount",17);set("gateMode",1);}for(int i=0;i<17;++i){const float x=float(i)/16;const float y=index==0?.5f-.5f*std::cos(x*juce::MathConstants<float>::twoPi):index==1?1-std::abs(x*2-1):index==2?x:index==3?1-x:index==4?(x<.5f?1:0):index==5?std::sqrt(x):float(((i*37+11)%17))/16;if(gate){set("gateCurveX"+juce::String(i),x);set("gateCurveBend"+juce::String(i),0);}set(yId(i),y);}changed();}
};
class MainWorkbench:public juce::Component,private juce::Timer {
    SpaceInfinityProcessor& p;std::function<void(std::unique_ptr<juce::Component>,juce::Rectangle<int>)> open;
    std::array<ModSlider,9> env;std::array<juce::Label,9> envLabels;std::array<ModSlider,6> macro;std::array<ModSourceButton,6> macroLabels;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> bindings;
    std::array<ModSourceButton,3> tabs;juce::ComboBox shape,sync,rate,mode;ModSlider speed;juce::ToggleButton enabled{"ON"},longEnv{"LONG x3"},declick{"DE-CLICK"},vibrato{"VIBRATO"};juce::TextButton assignments{"ROUTES"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> syncBind,rateBind,modeBind;std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> speedBind;std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> enabledBind,longBind,declickBind,vibratoBind;
    CurveEditor graph;int selected=0;juce::Rectangle<int> macroArea,envArea,lfoArea;
    void attach(ModSlider& slider,const juce::String& id){slider.initialiseModulation(p,id);slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,75,17);bindings.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,id,slider));auto* parameter=p.state.getParameter(id);slider.setDoubleClickReturnValue(true,parameter->convertFrom0to1(parameter->getDefaultValue()));}
    void select(int i){selected=i;graph.selectLfo(i);syncBind.reset();rateBind.reset();modeBind.reset();speedBind.reset();enabledBind.reset();syncBind=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,p.lfoParameterId(i,"Sync"),sync);rateBind=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,p.lfoParameterId(i,"Bars"),rate);modeBind=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,p.lfoParameterId(i,"Mode"),mode);speedBind=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,p.lfoParameterId(i,"Speed"),speed);enabledBind=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,p.lfoParameterId(i,"Enabled"),enabled);speed.initialiseModulation(p,p.lfoParameterId(i,"Speed"));for(int n=0;n<3;++n)tabs[size_t(n)].setToggleState(n==i,juce::dontSendNotification);resized();repaint();}
    void timerCallback()override{if(!isShowing())return;speed.setVisible(sync.getSelectedId()==1);rate.setVisible(sync.getSelectedId()==2);graph.repaint();}
public:
    void selectSource(int i){select(juce::jlimit(0,2,i));}
    ~MainWorkbench() override {stopTimer();}
    MainWorkbench(SpaceInfinityProcessor& processor,decltype(open) opener):p(processor),open(std::move(opener)),graph(p,false){setName("Main envelopes, performance macros and LFO editor");
        const char* ids[]{"attack","ampDecay","ampSustain","release","filterAttack","filterDecay","filterSustain","filterRelease","filterEnvAmount"};const char* labels[]{"ATTACK","DECAY","SUSTAIN","RELEASE","ATTACK","DECAY","SUSTAIN","RELEASE","AMOUNT"};for(int i=0;i<9;++i){attach(env[size_t(i)],ids[i]);env[size_t(i)].setName(juce::String(i<4?"Amp ":"Filter ")+labels[i]);envLabels[size_t(i)].setText(labels[i],juce::dontSendNotification);envLabels[size_t(i)].setJustificationType(juce::Justification::centred);envLabels[size_t(i)].setFont(juce::Font(juce::FontOptions(9)));addAndMakeVisible(env[size_t(i)]);addAndMakeVisible(envLabels[size_t(i)]);if(i==0||i==1||i==3||i==4||i==5||i==7)configureTimeKnob(env[size_t(i)]);if(i<4)env[size_t(i)].onDragStart=[this]{p.state.getParameter("ampTimed")->setValueNotifyingHost(1);};if(i==8){env[size_t(i)].textFromValueFunction=[](double v){return juce::String(v,2)+" oct";};env[size_t(i)].valueFromTextFunction=[](const juce::String& t){return t.getDoubleValue();};env[size_t(i)].updateText();}if(i==2||i==6){env[size_t(i)].textFromValueFunction=[](double v){return juce::String(v*100,0)+"%";};env[size_t(i)].valueFromTextFunction=[](const juce::String& v){return v.getDoubleValue()/100;};env[size_t(i)].updateText();}}
        for(int i=0;i<6;++i){const auto id=i<3?"macro"+juce::String(i):"freeMacro"+juce::String(i-3);attach(macro[size_t(i)],id);macroLabels[size_t(i)].setButtonText(i<3?p.macroNames()[i]:"MACRO "+juce::String(i+1)+" +");if(i<3)macroLabels[size_t(i)].description="INFINITY:PERFORMANCE:"+juce::String(i);if(i>=3)macroLabels[size_t(i)].description="INFINITY:MACRO:"+juce::String(i-3);macroLabels[size_t(i)].onClick=[this,i]{open(std::make_unique<RoutePanel>(p,i<3?i:i-3,i>=3),getBounds());};macro[size_t(i)].textFromValueFunction=[](double v){return juce::String(v*100,0)+"%";};macro[size_t(i)].valueFromTextFunction=[](const juce::String& t){return t.getDoubleValue()/100;};macro[size_t(i)].updateText();addAndMakeVisible(macro[size_t(i)]);addAndMakeVisible(macroLabels[size_t(i)]);}
        for(int i=0;i<3;++i){tabs[size_t(i)].setButtonText("LFO "+juce::String(i+1)+" +");tabs[size_t(i)].description="INFINITY:LFO:"+juce::String(i);tabs[size_t(i)].setTooltip("Click to edit; drag onto a supported synth or FX knob. Hover FX while dragging to switch page.");tabs[size_t(i)].onClick=[this,i]{select(i);};addAndMakeVisible(tabs[size_t(i)]);}
        shape.addItemList({"Sine","Triangle","Ramp up","Ramp down","Square","Sidechain","Random steps"},1);shape.setTextWhenNothingSelected("Shapes...");shape.onChange=[this]{if(shape.getSelectedId()>0)graph.shape(shape.getSelectedId()-1);};sync.addItemList({"Hz","BPM"},1);sync.onChange=[this]{resized();};rate.addItemList({"1 bar","2 bars","4 bars","8 bars","1/2 bar","1/4 bar","1/8 bar","1/16 bar"},1);mode.addItemList({"Free","Retrigger","One-shot"},1);speed.setSliderStyle(juce::Slider::LinearHorizontal);speed.setTextBoxStyle(juce::Slider::TextBoxRight,false,54,22);
        for(juce::Component* c:std::array<juce::Component*,11>{&shape,&sync,&rate,&mode,&speed,&enabled,&graph,&assignments,&longEnv,&declick,&vibrato})addAndMakeVisible(c);
        vibratoBind=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,"vibratoEnabled",vibrato);vibrato.setTooltip("Bypass the separate legacy vibrato/filter LFO.");longEnv.onStateChange=[this]{for(int i:{0,3})env[size_t(i)].updateText();};for(int i:{0,3}){env[size_t(i)].textFromValueFunction=[this](double raw){double v=raw*(longEnv.getToggleState()?3:1);return v<.1?juce::String(v*1000,2)+" ms":juce::String(v,3)+" s";};env[size_t(i)].valueFromTextFunction=[this](const juce::String& text){return text.getDoubleValue()/(text.containsIgnoreCase("ms")?1000:1)/(longEnv.getToggleState()?3:1);};}
        longBind=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,"ampLong",longEnv);declickBind=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,"declickEnabled",declick);graph.edited=[this]{shape.setSelectedId(0,juce::dontSendNotification);shape.setTextWhenNothingSelected("Custom");};assignments.onClick=[this]{open(std::make_unique<RoutePanel>(p,selected+3,true),getBounds());};select(0);startTimerHz(20);
    }
    void resized()override{auto area=getLocalBounds();const int mw=juce::jlimit(132,180,getWidth()/6),ew=juce::jlimit(300,460,getWidth()*39/100);macroArea=area.removeFromLeft(mw);area.removeFromLeft(8);envArea=area.removeFromLeft(ew);area.removeFromLeft(8);lfoArea=area;
        for(int i=0;i<6;++i){const int col=i<3?0:1,row=i%3;auto cell=juce::Rectangle<int>(macroArea.getX()+col*mw/2,24+row*(getHeight()-24)/3,mw/2,(getHeight()-24)/3);macroLabels[size_t(i)].setBounds(cell.removeFromTop(17).reduced(2,0));macro[size_t(i)].setBounds(cell.reduced(3,0));}
        const int eh=getHeight()/2;longEnv.setBounds(envArea.getRight()-165,2,77,20);declick.setBounds(envArea.getRight()-88,2,88,20);
        for(int i=0;i<9;++i){const int row=i<4?0:1,index=i<4?i:i-4,n=row==0?4:5;auto cell=juce::Rectangle<int>(envArea.getX()+index*ew/n,row*eh+24,ew/n,eh-26);envLabels[size_t(i)].setBounds(cell.removeFromTop(12));env[size_t(i)].setBounds(cell);}
        auto r=lfoArea.reduced(7);auto head=r.removeFromTop(23);for(int i=0;i<3;++i)tabs[size_t(i)].setBounds(head.removeFromLeft(head.getWidth()/(3-i)).reduced(1,0));auto top=r.removeFromTop(25);enabled.setBounds(top.removeFromRight(62));vibrato.setBounds(top.removeFromRight(85));shape.setBounds(top);
        auto foot=r.removeFromBottom(25);sync.setBounds(foot.removeFromLeft(66));mode.setBounds(foot.removeFromLeft(juce::jmin(87,foot.getWidth()/2)));rate.setBounds(foot);speed.setBounds(foot);rate.setVisible(sync.getSelectedId()==2);speed.setVisible(sync.getSelectedId()==1);auto help=r.removeFromBottom(20);assignments.setBounds(help.removeFromRight(65));graph.setBounds(r.reduced(0,2));}
    void paint(juce::Graphics& g)override{g.fillAll(juce::Colour(0xff101117));for(auto r:{macroArea,envArea,lfoArea})HardwareTheme::module(g,r.toFloat(),true);g.setColour(juce::Colour(0xffe5d7ee));g.setFont(juce::Font(juce::FontOptions(10,juce::Font::bold)));g.drawText("PERFORMANCE",macroArea.reduced(7).withHeight(15),juce::Justification::centredLeft);g.drawText("AMP ENVELOPE",envArea.reduced(6,3).withHeight(17),juce::Justification::centredLeft);g.drawText("FILTER ENVELOPE",envArea.withTrimmedTop(getHeight()/2).reduced(6,3).withHeight(17),juce::Justification::centredLeft);g.setFont(9);g.setColour(juce::Colour(0xffb8abc9));g.drawText("Drag LFO tab -> knob / right-click knob: amount",lfoArea.reduced(10).withTrimmedTop(getHeight()-54).withHeight(18),juce::Justification::centredLeft);}
};
class SubPanel:public juce::Component {
    SpaceInfinityProcessor& p;juce::ToggleButton on{"SUB"};juce::ComboBox wave;ModSlider level,octave;juce::Label label;juce::AudioProcessorValueTreeState::ButtonAttachment onBind;juce::AudioProcessorValueTreeState::ComboBoxAttachment waveBind;juce::AudioProcessorValueTreeState::SliderAttachment levelBind,octaveBind;
public:
    SubPanel(SpaceInfinityProcessor& processor):p(processor),onBind(p.state,"subEnabled",on),waveBind(p.state,"subWave",wave),levelBind(p.state,"subLevel",level),octaveBind(p.state,"subOctave",octave){setName("Sub oscillator");wave.addItemList({"Sine","Triangle","Saw","Pulse","Narrow pulse","Rounded saw"},1);wave.setSelectedItemIndex(int(p.state.getRawParameterValue("subWave")->load()),juce::dontSendNotification);level.initialiseModulation(p,"subLevel");octave.initialiseModulation(p,"subOctave");level.setName("Sub level");level.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);level.setTextBoxStyle(juce::Slider::TextBoxBelow,false,56,16);level.textFromValueFunction=[](double v){return juce::String(v*100,0)+"%";};level.valueFromTextFunction=[](const juce::String& t){return t.getDoubleValue()/100;};level.updateText();octave.setSliderStyle(juce::Slider::IncDecButtons);octave.setTextBoxStyle(juce::Slider::TextBoxLeft,false,34,22);octave.textFromValueFunction=[](double v){return juce::String(int(v));};octave.valueFromTextFunction=[](const juce::String& t){return double(t.getIntValue());};octave.updateText();label.setText("OCTAVE",juce::dontSendNotification);label.setFont(juce::Font(juce::FontOptions(9)));for(juce::Component* c:std::array<juce::Component*,5>{&on,&wave,&level,&octave,&label})addAndMakeVisible(c);}
    void resized()override{on.setBounds(4,20,70,25);wave.setBounds(78,23,juce::jmin(125,juce::jmax(75,getWidth()-230)),24);const int x=wave.getRight()+8;label.setBounds(x,6,65,15);octave.setBounds(x,25,68,24);level.setBounds(getWidth()-69,2,64,60);}
    void paint(juce::Graphics& g)override{HardwareTheme::module(g,getLocalBounds().toFloat());}
};
class MasterReadout:public juce::Component {
public:float peak=0;bool clip=false;
    void paint(juce::Graphics& g)override{HardwareTheme::display(g,getLocalBounds().toFloat().reduced(2));g.setColour(juce::Colour(clip?0xffff6b6b:0xffd4c6ec));g.setFont(juce::Font(juce::FontOptions(10,juce::Font::bold)));g.drawText(clip?"CLIP":(peak>.00001f?juce::String(juce::Decibels::gainToDecibels(peak),1):"-inf")+" dB",getLocalBounds().reduced(3),juce::Justification::centred);}
};
