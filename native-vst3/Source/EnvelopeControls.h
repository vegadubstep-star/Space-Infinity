#pragma once
#include "WorkflowControls.h"
class EnvelopeView:public juce::Component {
    SpaceInfinityProcessor& p;bool amp;int drag=-1,bend=-1;float startCurve=0;double fixedTotal=0;juce::StringArray gestures;
    juce::String id(int n)const{const char* a[]{"attack","ampHold","ampDecay","ampSustain","release"};const char* f[]{"filterAttack","filterHold","filterDecay","filterSustain","filterRelease"};return amp?a[n]:f[n];}
    juce::String curveId(int n)const{return juce::String(amp?"amp":"filter")+(n==0?"AttackCurve":n==1?"DecayCurve":"ReleaseCurve");}
    float get(const juce::String& key)const{return p.state.getRawParameterValue(key)->load();}
    float scale()const{return amp&&get("ampLong")>.5f?3:1;}
    std::array<float,5> values()const{return {get(id(0))*scale(),get(id(1)),get(id(2)),get(id(3)),get(id(4))*scale()};}
    juce::Rectangle<float> graph()const{return getLocalBounds().toFloat().reduced(12,getHeight()<80?6:14);}
    double total()const{const auto v=values();return fixedTotal>0?fixedTotal:juce::jmax(.01,double(v[0]+v[1]+v[2]+v[4]+.4f));}
    juce::Point<float> point(int n)const{const auto v=values();const float times[]{0,v[0],v[0]+v[1],v[0]+v[1]+v[2],v[0]+v[1]+v[2]+.4f,v[0]+v[1]+v[2]+.4f+v[4]},levels[]{0,1,1,v[3],v[3],0};const auto r=graph();return {r.getX()+float(times[n]/total())*r.getWidth(),r.getBottom()-levels[n]*r.getHeight()};}
    int segment(float x)const {if(x<point(1).x)return 0;if(x<point(3).x&&x>=point(2).x)return 1;if(x>=point(4).x)return 2;return -1;}
    void begin(const juce::String& key){if(!gestures.contains(key)){p.state.getParameter(key)->beginChangeGesture();gestures.add(key);}}
    void set(const juce::String& key,float v){auto* a=p.state.getParameter(key);a->setValueNotifyingHost(a->convertTo0to1(v));}
    void finish(){for(const auto& key:gestures)p.state.getParameter(key)->endChangeGesture();gestures.clear();drag=bend=-1;fixedTotal=0;}
public:
    EnvelopeView(SpaceInfinityProcessor& processor,bool isAmp):p(processor),amp(isAmp){setName(amp?"Amp envelope graph":"Filter envelope graph");}
    ~EnvelopeView()override{finish();}
    void paint(juce::Graphics& g)override {
        const auto r=graph();g.setColour(juce::Colour(0xff10111c));g.fillRoundedRectangle(getLocalBounds().toFloat(),4);g.setColour(juce::Colour(0xff30233e));for(int i=1;i<8;++i)g.drawVerticalLine(int(r.getX()+r.getWidth()*i/8),r.getY(),r.getBottom());
        const auto v=values();const float base=get("analogueUpgrade")>.5f?0:-.5f;juce::Path path;path.startNewSubPath(point(0));
        for(int n=0;n<5;++n){const auto a=point(n),b=point(n+1);for(int i=1;i<=40;++i){const float x=float(i)/40;const int c=n==0?0:n==2?1:n==4?2:-1;const float t=c>=0?AnalogueEnvelope::charge(x,get(curveId(c))+base):x;path.lineTo(a.x+(b.x-a.x)*x,a.y+(b.y-a.y)*t);}}
        g.setColour(juce::Colour(0xffd1b5ff));g.strokePath(path,juce::PathStrokeType(1.8f));for(int n:{1,2,3,5}){const auto pt=point(n);g.fillEllipse(pt.x-3,pt.y-3,6,6);}g.setFont(10);if(getHeight()>=80)g.drawText(amp?"AMP — drag nodes / bend segments":"FILTER — drag nodes / bend segments",getLocalBounds().reduced(7).withHeight(12),juce::Justification::centredLeft);if(getHeight()>=80)g.drawText(juce::String(total(),2)+" s",getLocalBounds().reduced(7).removeFromBottom(12),juce::Justification::centredRight);juce::ignoreUnused(v);
    }
    void mouseDown(const juce::MouseEvent& e)override {finish();fixedTotal=total();if(amp)set("ampTimed",1);float distance=11;for(int n:{1,2,3,5})if(e.position.getDistanceFrom(point(n))<distance){distance=e.position.getDistanceFrom(point(n));drag=n;}if(drag>=0){begin(id(drag==1?0:drag==2?1:drag==3?2:4));if(drag==3)begin(id(3));}else {bend=segment(e.position.x);if(bend>=0){startCurve=get(curveId(bend));begin(curveId(bend));}}mouseDrag(e);}
    void mouseDrag(const juce::MouseEvent& e)override {const auto v=values();const auto r=graph();const float seconds=juce::jlimit(0.0f,float(fixedTotal),(e.position.x-r.getX())/r.getWidth()*float(fixedTotal));if(drag==1)set(id(0),seconds/scale());else if(drag==2)set(id(1),juce::jmax(0.0f,seconds-v[0]));else if(drag==3){set(id(2),juce::jmax(0.0f,seconds-v[0]-v[1]));set(id(3),juce::jlimit(0.0f,1.0f,1-(e.position.y-r.getY())/r.getHeight()));}else if(drag==5)set(id(4),juce::jmax(0.0f,seconds-v[0]-v[1]-v[2]-.4f)/scale());else if(bend>=0)set(curveId(bend),juce::jlimit(-1.0f,1.0f,startCurve-float(e.getDistanceFromDragStartY())/r.getHeight()*2));repaint();}
    void mouseUp(const juce::MouseEvent&)override{finish();repaint();}
};
class EnvelopeContourPanel:public juce::Component,private juce::Timer {
    SpaceInfinityProcessor& p;std::array<EnvelopeView,2> graphs;std::array<ModSlider,6> knobs;std::array<juce::Label,6> labels;std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> bindings;
    void timerCallback()override{for(auto& graph:graphs)graph.repaint();}
public:
    EnvelopeContourPanel(SpaceInfinityProcessor& processor):p(processor),graphs{EnvelopeView(p,true),EnvelopeView(p,false)}{setSize(730,330);for(int row=0;row<2;++row){addAndMakeVisible(graphs[size_t(row)]);graphs[size_t(row)].setBounds(330,8+row*160,390,145);for(int col=0;col<3;++col){const int n=row*3+col;const char* names[]{"ATTACK","DECAY","RELEASE"};const auto id=juce::String(row==0?"amp":"filter")+(col==0?"AttackCurve":col==1?"DecayCurve":"ReleaseCurve");auto& knob=knobs[size_t(n)];knob.initialiseModulation(p,id);knob.setName((row==0?"Amp ":"Filter ")+juce::String(names[col])+" contour");knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);knob.setTextBoxStyle(juce::Slider::TextBoxBelow,false,90,20);knob.setBounds(8+col*105,35+row*160,100,100);knob.setMouseDragSensitivity(350);addAndMakeVisible(knob);labels[size_t(n)].setText(names[col],juce::dontSendNotification);labels[size_t(n)].setJustificationType(juce::Justification::centred);labels[size_t(n)].setBounds(8+col*105,10+row*160,100,22);addAndMakeVisible(labels[size_t(n)]);bindings.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,id,knob));knob.setDoubleClickReturnValue(true,0);}}startTimerHz(15);}
    ~EnvelopeContourPanel()override{stopTimer();}
};
