#pragma once
#include "WorkflowControls.h"
#include "EnvelopeControls.h"
#include "HardwareTheme.h"
class RoutePanel:public juce::Component {
    SpaceInfinityProcessor& p;juce::Viewport view;juce::Component content;std::vector<std::unique_ptr<juce::Component>> controls;std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliders;std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> choices;
public:
    RoutePanel(SpaceInfinityProcessor& processor,int source,bool generic):p(processor){const int count=generic?16:3;setSize(550,juce::jmin(440,65+count*49));addAndMakeVisible(view);view.setViewedComponent(&content,false);view.setBounds(0,0,getWidth(),getHeight());content.setSize(535,30+count*49);
        auto title=std::make_unique<juce::Label>();title->setText(generic?"Choose a control and signed amount. Drag sources onto supported knobs for quicker assignment.":"Performance source: choose destination and amount.",juce::dontSendNotification);title->setBounds(8,0,520,30);content.addAndMakeVisible(*title);controls.push_back(std::move(title));
        for(int slot=0;slot<count;++slot){auto box=std::make_unique<juce::ComboBox>();box->addItem("Unassigned",1);box->addItemList(generic?p.flexibleTargets():p.macroTargets(),2);box->setBounds(8,32+slot*49,337,32);auto amount=std::make_unique<juce::Slider>();amount->setSliderStyle(juce::Slider::LinearHorizontal);amount->setTextBoxStyle(juce::Slider::TextBoxRight,false,64,25);amount->setBounds(350,32+slot*49,178,32);
            if(generic){box->setSelectedId(p.flexibleSourceTarget(source,slot)+2,juce::dontSendNotification);box->onChange=[this,source,slot,b=box.get()]{p.assignFlexibleSource(source,slot,b->getSelectedId()-2);};const auto id=p.flexibleDepthId(source,slot);sliders.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,id,*amount));}
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
    juce::String field(const char* suffix)const{return gate?"gateCurve"+juce::String(suffix):p.lfoParameterId(lfo,(juce::String("Curve")+suffix).toRawUTF8());}
    juce::String pointId(const char* suffix,int i)const{return gate?"gateCurve"+juce::String(suffix)+juce::String(i):p.lfoParameterId(lfo,suffix)+juce::String(i);}
    int count()const{return gate?juce::jlimit(2,33,int(get(field("Count")))):p.lfoCurve(lfo).count;}
    juce::String yId(int i)const{return pointId("Y",i);}
    float xAt(int i)const{return gate?get(pointId("X",i)):p.lfoCurve(lfo).x[size_t(i)];}
    juce::Rectangle<float> graph()const{return getLocalBounds().toFloat().reduced(9,9);}
    juce::Point<float> point(int i)const{auto r=graph();return {r.getX()+xAt(i)*r.getWidth(),r.getBottom()-(gate?get(yId(i)):p.lfoCurve(lfo).y[size_t(i)])*r.getHeight()};}
    int nearest(juce::Point<float> pos)const{int best=-1;float distance=10;for(int i=0;i<count();++i){const float d=pos.getDistanceFrom(point(i));if(d<distance){distance=d;best=i;}}return best;}
    studio::EditableCurve curve()const{if(!gate)return p.lfoCurve(lfo);studio::EditableCurve c;c.count=count();for(int i=0;i<c.count;++i){c.x[size_t(i)]=xAt(i);c.y[size_t(i)]=get(yId(i));c.bend[size_t(i)]=get(pointId("Bend",i));}return c;}
    void begin(const juce::String& id){if(!gestures.contains(id)){p.state.getParameter(id)->beginChangeGesture();gestures.add(id);}}
    void finish(){for(const auto& id:gestures)p.state.getParameter(id)->endChangeGesture();gestures.clear();drag=segment=-1;painting=false;}
    void ensureCurve(){if(gate){if(get("gateMode")<.5f)p.useGateCurveFromSteps();}else p.enableLfoCurve(lfo);}
    void changed(){if(edited)edited();repaint();}
public:
    std::function<void()> edited;
    CurveEditor(SpaceInfinityProcessor& processor,bool isGate):p(processor),gate(isGate){setName(gate?"Audio gate curve":"LFO 1 curve");setMouseCursor(juce::MouseCursor::CrosshairCursor);}
    ~CurveEditor()override{finish();}
    void selectLfo(int i){finish();lfo=i;setName("LFO "+juce::String(i+1)+" curve");repaint();}
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
        if(!legacy)for(int i=0;i<c.count-1;++i){const float x=(c.x[size_t(i)]+c.x[size_t(i+1)])*.5f;const auto pt=juce::Point<float>(r.getX()+x*r.getWidth(),r.getBottom()-c.value(x)*r.getHeight());g.setColour(juce::Colour(0xff9c7cda));g.drawEllipse(pt.x-3,pt.y-3,6,6,1);}
        const float phase=gate?p.gatePhaseView():p.lfoDisplayPhase(lfo);g.setColour(juce::Colour(0x99f0dfaa));g.drawVerticalLine(juce::roundToInt(r.getX()+phase*r.getWidth()),r.getY(),r.getBottom());
        if(legacy){g.setFont(10);g.setColour(juce::Colour(0xffded5ee));g.drawText("STEP PATTERN / edit to convert to a curve",r.toNearestInt().reduced(8).withHeight(18),juce::Justification::centredLeft);}
    }
    void mouseDown(const juce::MouseEvent& e)override{
        if(!gate&&e.mods.isPopupMenu()){juce::PopupMenu menu;menu.addItem(1,"Set envelope loopback here");menu.addItem(2,"Clear envelope loopback");const float x=juce::jlimit(0.0f,.999f,(e.position.x-graph().getX())/graph().getWidth());const juce::Component::SafePointer<CurveEditor> safe(this);menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),[safe,x](int choice){if(safe&&choice>0)safe->set(safe->p.lfoParameterId(safe->lfo,"Loopback"),choice==1?x:-1);});return;}
        finish();ensureCurve();painting=paintMode||e.mods.isShiftDown();
        if(painting){const auto c=curve();set(field("Count"),17);for(int i=0;i<17;++i){set(pointId("X",i),float(i)/16);set(yId(i),c.value(float(i)/16));set(pointId("Bend",i),0);}}
        if(painting){drag=juce::jlimit(0,16,juce::roundToInt((e.position.x-graph().getX())/graph().getWidth()*16));for(int i=0;i<17;++i)begin(yId(i));}
        else{drag=nearest(e.position);if(drag>=0){begin(yId(drag));if(drag>0&&drag<count()-1)begin(pointId("X",drag));}
            else{const float x=(e.position.x-graph().getX())/graph().getWidth();segment=0;while(segment<count()-2&&xAt(segment+1)<x)++segment;const auto id=pointId("Bend",segment);startBend=get(id);begin(id);}}
        mouseDrag(e);
    }
    void mouseDrag(const juce::MouseEvent& e)override{
        const auto r=graph();float x=juce::jlimit(0.0f,1.0f,(e.position.x-r.getX())/r.getWidth()),y=1-juce::jlimit(0.0f,1.0f,(e.position.y-r.getY())/r.getHeight());
        if(snapEnabled||e.mods.isAltDown()){x=std::round(x*snap)/snap;y=std::round(y*snap)/snap;}
        if(segment>=0){write(pointId("Bend",segment),juce::jlimit(-1.0f,1.0f,startBend-float(e.getDistanceFromDragStartY())/r.getHeight()*2));}
        else if(drag>=0&&painting){const int next=juce::jlimit(0,16,juce::roundToInt(x*16)),first=drag;const float old=get(yId(first));for(int i=std::min(first,next);i<=std::max(first,next);++i){const float t=first==next?1:float(i-first)/float(next-first);write(yId(i),old+(y-old)*t);}drag=next;}
        else if(drag>=0){if(drag>0&&drag<count()-1)write(pointId("X",drag),juce::jlimit(xAt(drag-1)+.000001f,xAt(drag+1)-.000001f,x));write(yId(drag),y);}
        changed();
    }
    void mouseUp(const juce::MouseEvent&)override{finish();}
    void mouseDoubleClick(const juce::MouseEvent& e)override{
        finish();ensureCurve();
        const int old=count(),hit=nearest(e.position);
        if(hit>0&&hit<old-1){for(int i=hit;i<old-1;++i){set(pointId("X",i),xAt(i+1));set(yId(i),get(yId(i+1)));set(pointId("Bend",i),get(pointId("Bend",i+1)));}set(field("Count"),float(old-1));}
        else if(hit<0&&old<33){const auto r=graph();float x=juce::jlimit(.00001f,.99999f,(e.position.x-r.getX())/r.getWidth());if(snapEnabled||e.mods.isAltDown())x=juce::jlimit(.00001f,.99999f,std::round(x*snap)/snap);int at=1;while(at<old-1&&xAt(at)<x)++at;for(int i=old;i>at;--i){set(pointId("X",i),xAt(i-1));set(yId(i),get(yId(i-1)));set(pointId("Bend",i),get(pointId("Bend",i-1)));}set(pointId("X",at),x);set(yId(at),1-juce::jlimit(0.0f,1.0f,(e.position.y-r.getY())/r.getHeight()));set(pointId("Bend",at),0);set(field("Count"),float(old+1));}
        changed();
    }
    void mouseWheelMove(const juce::MouseEvent& e,const juce::MouseWheelDetails& wheel)override{ensureCurve();const float x=(e.position.x-graph().getX())/graph().getWidth();int at=0;while(at<count()-2&&xAt(at+1)<x)++at;const auto id=pointId("Bend",at);set(id,juce::jlimit(-1.0f,1.0f,get(id)+wheel.deltaY*.5f));changed();}
    void shape(int index){finish();ensureCurve();if(gate)set("gateMode",1);
        const int n=index==0?5:index==1?3:index==4?4:index==6?17:2;set(field("Count"),float(n));
        for(int i=0;i<n;++i){float x=float(i)/(n-1),y=0,bend=0;
            if(index==0){y=.5f-.5f*std::cos(x*juce::MathConstants<float>::twoPi);bend=i%2==0?.25f:-.25f;}
            else if(index==1)y=1-std::abs(x*2-1);
            else if(index==2)y=x;else if(index==3)y=1-x;
            else if(index==4){x=i==0?0:i==1?.4999f:i==2?.5f:1;y=i<2?1:0;}
            else if(index==5){y=x;bend=-1.0f/3;}
            else y=float(((i*37+11)%17))/16;
            set(pointId("X",i),x);set(pointId("Bend",i),bend);set(yId(i),y);
        }changed();}

};
class LfoPane:public juce::Component,private juce::Timer {
    SpaceInfinityProcessor& p;std::function<void(std::unique_ptr<juce::Component>,juce::Rectangle<int>)> open;
    int selected=0;ModSourceButton source;juce::ComboBox selector,shape,sync,rate,mode,grid;juce::ToggleButton enabled{"ON"},snap{"Snap"},draw{"Draw"};ModSlider speed;juce::TextButton routes{"ROUTES"},expand{"ADVANCED"};CurveEditor graph;
    std::array<ModSlider,4> advanced;std::array<juce::Label,4> advancedLabels;juce::ComboBox direction;std::array<juce::ToggleButton,4> advancedButtons;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> advancedBindings;std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> advancedButtonBindings;std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> directionBind;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> syncBind,rateBind,modeBind;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> speedBind;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> enabledBind;
    void timerCallback()override{if(isShowing())graph.repaint();}
public:
    LfoPane(SpaceInfinityProcessor& processor,int initial,decltype(open) opener):p(processor),open(std::move(opener)),graph(p,false){
        const char* titles[]{"PHASE","DELAY","RISE","SMOOTH"};for(int i=0;i<4;++i){auto& knob=advanced[size_t(i)];knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);knob.setTextBoxStyle(juce::Slider::TextBoxBelow,false,72,18);knob.setMouseDragSensitivity(300);advancedLabels[size_t(i)].setText(titles[i],juce::dontSendNotification);advancedLabels[size_t(i)].setJustificationType(juce::Justification::centred);addAndMakeVisible(knob);addAndMakeVisible(advancedLabels[size_t(i)]);}
        direction.addItemList({"Forward","Reverse","Ping Pong"},1);addAndMakeVisible(direction);const char* switches[]{"MONO","TRIP","DOT","HOST"};for(int i=0;i<4;++i){advancedButtons[size_t(i)].setButtonText(switches[i]);addAndMakeVisible(advancedButtons[size_t(i)]);}
        selector.addItemList({"LFO 1","LFO 2","LFO 3"},1);selector.onChange=[this]{select(selector.getSelectedId()-1);};
        source.onClick=[this]{juce::PopupMenu menu;for(int n=0;n<3;++n)menu.addItem(n+1,"LFO "+juce::String(n+1));const juce::Component::SafePointer<LfoPane> safe(this);menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&source),[safe](int id){if(safe&&id>0)safe->select(id-1);});};
        source.setTooltip("Drag this LFO onto a synth or FX control. Edit its signed amount at the destination.");
        shape.addItemList({"Sine","Triangle","Ramp up","Ramp down","Square","Sidechain","Random steps"},1);shape.setTextWhenNothingSelected("Shape");shape.onChange=[this]{if(shape.getSelectedId())graph.shape(shape.getSelectedId()-1);};
        sync.addItemList({"Hz","BPM"},1);sync.onChange=[this]{resized();};rate.addItemList({"1 bar","2 bars","4 bars","8 bars","1/2 bar","1/4 bar","1/8 bar","1/16 bar"},1);mode.addItemList({"Free","Retrigger","Envelope"},1);
        grid.addItemList({"4","8","16","32"},1);grid.setSelectedId(3);grid.onChange=[this]{graph.setSnap(grid.getText().getIntValue());};snap.onClick=[this]{graph.setSnapEnabled(snap.getToggleState());};draw.onClick=[this]{graph.setPaintMode(draw.getToggleState());};
        speed.setSliderStyle(juce::Slider::LinearHorizontal);speed.setTextBoxStyle(juce::Slider::TextBoxRight,false,44,20);
        for(juce::Component* c:std::array<juce::Component*,15>{&source,&selector,&shape,&sync,&rate,&mode,&grid,&enabled,&snap,&draw,&speed,&routes,&expand,&graph,&hint})addAndMakeVisible(c);
        hint.setText("Drag points to move / drag between points to bend",juce::dontSendNotification);hint.setFont(juce::Font(juce::FontOptions(9)));hint.setJustificationType(juce::Justification::centredLeft);
        graph.edited=[this]{shape.setSelectedId(0,juce::dontSendNotification);shape.setTextWhenNothingSelected("Custom");};routes.onClick=[this]{open(std::make_unique<RoutePanel>(p,selected+3,true),getBounds());};
        expand.onClick=[this]{auto pane=std::make_unique<LfoPane>(p,selected,open);pane->setSize(760,410);pane->expand.setVisible(false);open(std::move(pane),getBounds());};select(initial);startTimerHz(20);
    }
    ~LfoPane()override{stopTimer();}
    juce::Label hint;
    void select(int i){selected=juce::jlimit(0,2,i);setName("LFO "+juce::String(selected+1)+" editor");selector.setSelectedId(selected+1,juce::dontSendNotification);source.setButtonText("LFO "+juce::String(selected+1)+" +");source.description="INFINITY:LFO:"+juce::String(selected);graph.selectLfo(selected);
        advancedBindings.clear();advancedButtonBindings.clear();directionBind.reset();const char* fields[]{"Phase","Delay","Rise","Smooth"};const char* toggles[]{"Mono","Triplet","Dotted","Host"};for(int n=0;n<4;++n){auto& knob=advanced[size_t(n)];const auto id=p.lfoParameterId(selected,fields[n]);knob.initialiseModulation(p,id);knob.setName("LFO "+juce::String(selected+1)+" "+fields[n]);advancedBindings.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,id,knob));advancedButtonBindings.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,p.lfoParameterId(selected,toggles[n]),advancedButtons[size_t(n)]));knob.textFromValueFunction=[n](double v){return n==0?juce::String(v*360,1)+" deg":n==3?juce::String(v,1)+" ms":juce::String(v,3)+" s";};knob.valueFromTextFunction=[n](const juce::String& text){return text.getDoubleValue()/(n==0?360:1);};knob.updateText();}directionBind=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,p.lfoParameterId(selected,"Direction"),direction);
        syncBind.reset();rateBind.reset();modeBind.reset();speedBind.reset();enabledBind.reset();
        syncBind=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,p.lfoParameterId(selected,"Sync"),sync);rateBind=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,p.lfoParameterId(selected,"Bars"),rate);modeBind=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,p.lfoParameterId(selected,"Mode"),mode);speedBind=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,p.lfoParameterId(selected,"Speed"),speed);enabledBind=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,p.lfoParameterId(selected,"Enabled"),enabled);speed.initialiseModulation(p,p.lfoParameterId(selected,"Speed"));resized();repaint();
    }
    void resized()override{auto r=getLocalBounds().reduced(5);auto top=r.removeFromTop(23);enabled.setBounds(top.removeFromRight(49));const bool compact=getWidth()<220;selector.setVisible(!compact);source.setBounds(compact?top:top.removeFromLeft(76));if(!compact)selector.setBounds(top);
        top=r.removeFromTop(24);shape.setBounds(top.removeFromLeft(top.getWidth()/2).reduced(1));mode.setBounds(top.reduced(1));
        auto foot=r.removeFromBottom(23);sync.setBounds(foot.removeFromLeft(57));rate.setBounds(foot);speed.setBounds(foot);rate.setVisible(sync.getSelectedId()==2);speed.setVisible(sync.getSelectedId()==1);
        const bool full=getHeight()>320;grid.setVisible(full);snap.setVisible(full);draw.setVisible(full);
        auto tools=r.removeFromBottom(23);if(full){const int w=tools.getWidth()/5;grid.setBounds(tools.removeFromLeft(w));snap.setBounds(tools.removeFromLeft(w));draw.setBounds(tools.removeFromLeft(w));routes.setBounds(tools.removeFromLeft(w));expand.setBounds(tools);}else{routes.setBounds(tools.removeFromLeft(tools.getWidth()/2).reduced(1));expand.setBounds(tools.reduced(1));}
        direction.setVisible(full);for(auto& button:advancedButtons)button.setVisible(full);for(int i=0;i<4;++i){advanced[size_t(i)].setVisible(full);advancedLabels[size_t(i)].setVisible(full);}
        if(full){auto knobs=r.removeFromBottom(75);for(int i=0;i<4;++i){auto cell=knobs.removeFromLeft(knobs.getWidth()/(4-i));advancedLabels[size_t(i)].setBounds(cell.removeFromTop(17));advanced[size_t(i)].setBounds(cell);}auto switches=r.removeFromBottom(24);direction.setBounds(switches.removeFromLeft(switches.getWidth()/3));for(int i=0;i<4;++i)advancedButtons[size_t(i)].setBounds(switches.removeFromLeft(switches.getWidth()/(4-i)));}
        hint.setVisible(getWidth()>260);if(getWidth()>260)hint.setBounds(r.removeFromBottom(15));graph.setBounds(r);

    }
    void paint(juce::Graphics& g)override{HardwareTheme::module(g,getLocalBounds().toFloat(),true);}
};
class MainWorkbench:public juce::Component,private juce::Timer {
    SpaceInfinityProcessor& p;std::function<void(std::unique_ptr<juce::Component>,juce::Rectangle<int>)> open;
    std::array<ModSlider,11> env;std::array<juce::Label,11> envLabels;std::array<ModSlider,6> macro;std::array<ModSourceButton,6> macroLabels;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> bindings;
    std::array<EnvelopeView,2> envelopeGraphs;
    void timerCallback()override{for(auto& graph:envelopeGraphs)graph.repaint();}
    juce::TextButton contours{"CURVES"};
    juce::ToggleButton longEnv{"LONG x3"},declick{"DE-CLICK"},vibrato{"VIBRATO"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> longBind,declickBind,vibratoBind;
    std::array<std::unique_ptr<LfoPane>,2> lfos;juce::Rectangle<int> macroArea,envArea,lfoArea;
    void attach(ModSlider& slider,const juce::String& id){slider.initialiseModulation(p,id);slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,75,17);bindings.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,id,slider));auto* parameter=p.state.getParameter(id);slider.setDoubleClickReturnValue(true,parameter->convertFrom0to1(parameter->getDefaultValue()));}
public:
    void selectSource(int i){lfos[size_t(i==1?1:0)]->select(juce::jlimit(0,2,i));}
    ~MainWorkbench() override =default;
    MainWorkbench(SpaceInfinityProcessor& processor,decltype(open) opener):p(processor),open(std::move(opener)),envelopeGraphs{EnvelopeView(p,true),EnvelopeView(p,false)}{for(auto& graph:envelopeGraphs)addAndMakeVisible(graph);startTimerHz(24);setName("Main envelopes, performance macros and LFO editor");
        const char* ids[]{"attack","ampHold","ampDecay","ampSustain","release","filterAttack","filterHold","filterDecay","filterSustain","filterRelease","filterEnvAmount"};const char* labels[]{"ATTACK","HOLD","DECAY","SUSTAIN","RELEASE","ATTACK","HOLD","DECAY","SUSTAIN","RELEASE","AMOUNT"};for(int i=0;i<11;++i){attach(env[size_t(i)],ids[i]);env[size_t(i)].setName(juce::String(i<5?"Amp ":"Filter ")+labels[i]);envLabels[size_t(i)].setText(labels[i],juce::dontSendNotification);envLabels[size_t(i)].setJustificationType(juce::Justification::centred);envLabels[size_t(i)].setFont(juce::Font(juce::FontOptions(9)));addAndMakeVisible(env[size_t(i)]);addAndMakeVisible(envLabels[size_t(i)]);if(i==0||i==1||i==2||i==4||i==5||i==6||i==7||i==9)configureTimeKnob(env[size_t(i)]);if(i<5)env[size_t(i)].onDragStart=[this]{p.state.getParameter("ampTimed")->setValueNotifyingHost(1);};if(i==10){env[size_t(i)].textFromValueFunction=[](double v){return juce::String(v,2)+" oct";};env[size_t(i)].valueFromTextFunction=[](const juce::String& t){return t.getDoubleValue();};env[size_t(i)].updateText();}if(i==3||i==8){env[size_t(i)].textFromValueFunction=[](double v){return juce::String(v*100,0)+"%";};env[size_t(i)].valueFromTextFunction=[](const juce::String& v){return v.getDoubleValue()/100;};env[size_t(i)].updateText();}}
        for(int i=0;i<6;++i){const auto id=i<3?"macro"+juce::String(i):"freeMacro"+juce::String(i-3);attach(macro[size_t(i)],id);macroLabels[size_t(i)].setButtonText(i<3?p.macroNames()[i]+" +":"MACRO "+juce::String(i+1)+" +");if(i<3)macroLabels[size_t(i)].description="INFINITY:PERFORMANCE:"+juce::String(i);if(i>=3)macroLabels[size_t(i)].description="INFINITY:MACRO:"+juce::String(i-3);macroLabels[size_t(i)].onClick=[this,i]{open(std::make_unique<RoutePanel>(p,i<3?i+6:i-3,true),getBounds());};macro[size_t(i)].textFromValueFunction=[](double v){return juce::String(v*100,0)+"%";};macro[size_t(i)].valueFromTextFunction=[](const juce::String& t){return t.getDoubleValue()/100;};macro[size_t(i)].updateText();addAndMakeVisible(macro[size_t(i)]);addAndMakeVisible(macroLabels[size_t(i)]);}
        for(int i=0;i<2;++i){lfos[size_t(i)]=std::make_unique<LfoPane>(p,i,open);addAndMakeVisible(*lfos[size_t(i)]);}for(juce::Component* control:std::array<juce::Component*,3>{&longEnv,&declick,&vibrato})addAndMakeVisible(control);
        vibratoBind=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,"vibratoEnabled",vibrato);vibrato.setTooltip("Bypass the separate legacy vibrato/filter LFO.");longEnv.onStateChange=[this]{for(int i:{0,4})env[size_t(i)].updateText();};for(int i:{0,4}){env[size_t(i)].textFromValueFunction=[this](double raw){double v=raw*(longEnv.getToggleState()?3:1);return v<.1?juce::String(v*1000,2)+" ms":juce::String(v,3)+" s";};env[size_t(i)].valueFromTextFunction=[this](const juce::String& text){return text.getDoubleValue()/(text.containsIgnoreCase("ms")?1000:1)/(longEnv.getToggleState()?3:1);};}
        addAndMakeVisible(contours);contours.setTooltip("Edit amp/filter envelope contours and graphs");contours.onClick=[this]{open(std::make_unique<EnvelopeContourPanel>(p),getBounds());};
        longBind=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,"ampLong",longEnv);declickBind=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,"declickEnabled",declick);
    }
    void resized()override{auto area=getLocalBounds();const int mw=juce::jlimit(132,180,getWidth()/6),ew=juce::jlimit(300,460,getWidth()*39/100);macroArea=area.removeFromLeft(mw);area.removeFromLeft(8);envArea=area.removeFromLeft(ew);area.removeFromLeft(8);lfoArea=area;
        for(int i=0;i<6;++i){const int col=i<3?0:1,row=i%3;auto cell=juce::Rectangle<int>(macroArea.getX()+col*mw/2,24+row*(getHeight()-24)/3,mw/2,(getHeight()-24)/3);macroLabels[size_t(i)].setBounds(cell.removeFromTop(17).reduced(2,0));macro[size_t(i)].setBounds(cell.reduced(3,0));}
        const int eh=getHeight()/2;longEnv.setBounds(envArea.getRight()-165,2,77,20);declick.setBounds(envArea.getRight()-88,2,88,20);
        const int graphHeight=juce::jlimit(32,65,eh-85);for(int row=0;row<2;++row)envelopeGraphs[size_t(row)].setBounds(envArea.getX()+6,row*eh+24,ew-12,graphHeight);
        for(int i=0;i<11;++i){const int row=i<5?0:1,index=i<5?i:i-5,n=row==0?5:6;auto cell=juce::Rectangle<int>(envArea.getX()+index*ew/n,row*eh+26+graphHeight,ew/n,eh-28-graphHeight);envLabels[size_t(i)].setBounds(cell.removeFromTop(12));env[size_t(i)].setBounds(cell);}
        auto r=lfoArea.reduced(2);const bool stacked=r.getWidth()<420&&r.getHeight()>450;
        auto first=stacked?r.removeFromTop(r.getHeight()/2):r.removeFromLeft(r.getWidth()/2);lfos[0]->setBounds(first.reduced(2));lfos[1]->setBounds(r.reduced(2));vibrato.setBounds(envArea.getRight()-82,eh+1,80,20);contours.setBounds(envArea.getRight()-150,eh+1,65,20);}

    void paint(juce::Graphics& g)override{g.fillAll(juce::Colour(0xff101117));for(auto r:{macroArea,envArea,lfoArea})HardwareTheme::module(g,r.toFloat(),true);g.setColour(juce::Colour(0xffe5d7ee));g.setFont(juce::Font(juce::FontOptions(10,juce::Font::bold)));g.drawText("PERFORMANCE",macroArea.reduced(7).withHeight(15),juce::Justification::centredLeft);g.drawText("AMP ENVELOPE",envArea.reduced(6,3).withHeight(17),juce::Justification::centredLeft);g.drawText("FILTER ENVELOPE",envArea.withTrimmedTop(getHeight()/2).reduced(6,3).withHeight(17),juce::Justification::centredLeft);}
};
class UnisonPanel:public juce::Component {
    SpaceInfinityProcessor& p;juce::ComboBox mode,stack;juce::ToggleButton updated{"Updated unison"},reset{"Reset phase to zero"};std::array<ModSlider,5> knobs;std::array<juce::Label,5> labels;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> bindings;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeBind,stackBind;std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> updatedBind,resetBind;
public:
    UnisonPanel(SpaceInfinityProcessor& processor,int oscillator):p(processor){setSize(620,235);setName("Oscillator "+juce::String(oscillator+1)+" unison settings");const auto prefix="osc"+juce::String(oscillator+1);mode.addItemList({"Linear","Super","Exp","Inv","Random"},1);stack.addItemList({"Off","12 (1x)","12 (2x)","12 (3x)","12+7 (1x)","12+7 (2x)","12+7 (3x)","Center -12","Center -24"},1);mode.setBounds(12,12,135,28);stack.setBounds(155,12,145,28);updated.setBounds(310,12,145,28);reset.setBounds(460,12,150,28);for(juce::Component* c:std::array<juce::Component*,4>{&mode,&stack,&updated,&reset})addAndMakeVisible(c);
        modeBind=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,prefix+"UnisonMode",mode);stackBind=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,prefix+"Stack",stack);updatedBind=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,"manualUnison",updated);resetBind=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,prefix+"PhaseReset",reset);
        const char* fields[]{"Range","Phase","Rand","WTSpread","WarpSpread"};const char* captions[]{"RANGE (cents)","PHASE","RAND","WT POS SPREAD","WARP SPREAD"};for(int i=0;i<5;++i){auto& knob=knobs[size_t(i)];knob.initialiseModulation(p,prefix+fields[i]);knob.setName(prefix+" "+fields[i]);knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);knob.setTextBoxStyle(juce::Slider::TextBoxBelow,false,100,22);knob.setBounds(12+i*120,70,115,115);knob.setMouseDragSensitivity(350);labels[size_t(i)].setText(captions[i],juce::dontSendNotification);labels[size_t(i)].setJustificationType(juce::Justification::centred);labels[size_t(i)].setBounds(12+i*120,48,115,20);addAndMakeVisible(knob);addAndMakeVisible(labels[size_t(i)]);bindings.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,prefix+fields[i],knob));knob.textFromValueFunction=[i](double v){return i==0?juce::String(v,1)+" ct":i==1?juce::String(v*360,1)+" deg":juce::String(v*100,1)+"%";};knob.valueFromTextFunction=[i](const juce::String& t){return t.getDoubleValue()/(i==0?1:i==1?360:100);};knob.updateText();knob.onDragStart=[this,i,prefix]{p.state.getParameter("manualUnison")->setValueNotifyingHost(1);if(i==1||i==2)p.state.getParameter(prefix+"PhaseReset")->setValueNotifyingHost(0);};}
    }
    void paint(juce::Graphics& g)override {g.setColour(juce::Colour(0xffd0bde3));g.setFont(11);g.drawText("75% Blend = equal voice levels. Detune needs >1 voice; Blend needs >2.",12,199,596,20,juce::Justification::centred);}
};
class SubPanel:public juce::Component {
    SpaceInfinityProcessor& p;juce::ToggleButton on{"SUB"};juce::ComboBox wave;ModSlider level,octave,pan;juce::Label label;juce::AudioProcessorValueTreeState::ButtonAttachment onBind;juce::AudioProcessorValueTreeState::ComboBoxAttachment waveBind;juce::AudioProcessorValueTreeState::SliderAttachment levelBind,octaveBind,panBind;
public:
    SubPanel(SpaceInfinityProcessor& processor):p(processor),onBind(p.state,"subEnabled",on),waveBind(p.state,"subWave",wave),levelBind(p.state,"subLevel",level),octaveBind(p.state,"subOctave",octave),panBind(p.state,"subPan",pan){setName("Sub oscillator");pan.initialiseModulation(p,"subPan");pan.setName("Sub pan");pan.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);pan.setTextBoxStyle(juce::Slider::TextBoxBelow,false,56,16);pan.textFromValueFunction=[](double value){return std::abs(value)<.005?juce::String("C"):juce::String(std::abs(value)*100,0)+(value<0?" L":" R");};pan.valueFromTextFunction=[](const juce::String& text){return text.containsIgnoreCase("C")?0:text.getDoubleValue()*(text.containsIgnoreCase("L")?-1:1)/100;};pan.updateText();pan.setTooltip("Sub stereo pan");addAndMakeVisible(pan);wave.addItemList({"Sine","Triangle","Saw","Pulse","Narrow pulse","Rounded saw"},1);wave.setSelectedItemIndex(int(p.state.getRawParameterValue("subWave")->load()),juce::dontSendNotification);level.initialiseModulation(p,"subLevel");octave.initialiseModulation(p,"subOctave");level.setName("Sub level");level.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);level.setTextBoxStyle(juce::Slider::TextBoxBelow,false,56,16);level.textFromValueFunction=[](double v){return juce::String(v*100,0)+"%";};level.valueFromTextFunction=[](const juce::String& t){return t.getDoubleValue()/100;};level.updateText();octave.setSliderStyle(juce::Slider::IncDecButtons);octave.setTextBoxStyle(juce::Slider::TextBoxLeft,false,34,22);octave.textFromValueFunction=[](double v){return juce::String(int(v));};octave.valueFromTextFunction=[](const juce::String& t){return double(t.getIntValue());};octave.updateText();label.setText("OCTAVE",juce::dontSendNotification);label.setFont(juce::Font(juce::FontOptions(9)));for(juce::Component* c:std::array<juce::Component*,5>{&on,&wave,&level,&octave,&label})addAndMakeVisible(c);}
    void resized()override{on.setBounds(4,20,70,25);wave.setBounds(78,23,juce::jmin(125,juce::jmax(75,getWidth()-300)),24);const int x=wave.getRight()+8;label.setBounds(x,6,65,15);octave.setBounds(x,25,68,24);level.setBounds(getWidth()-69,2,64,60);pan.setBounds(getWidth()-139,2,64,60);}
    void paint(juce::Graphics& g)override{HardwareTheme::module(g,getLocalBounds().toFloat());}
};
class MasterReadout:public juce::Component {
public:float peak=0;bool clip=false;
    void paint(juce::Graphics& g)override{HardwareTheme::display(g,getLocalBounds().toFloat().reduced(2));g.setColour(juce::Colour(clip?0xffff6b6b:0xffd4c6ec));g.setFont(juce::Font(juce::FontOptions(10,juce::Font::bold)));g.drawText(clip?"CLIP":(peak>.00001f?juce::String(juce::Decibels::gainToDecibels(peak),1):"-inf")+" dB",getLocalBounds().reduced(3),juce::Justification::centred);}
};
