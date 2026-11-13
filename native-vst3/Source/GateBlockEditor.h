#pragma once
#include "PluginProcessor.h"
#include "HardwareTheme.h"
class GateBlockEditor final:public juce::Component {
    SpaceInfinityProcessor& p;int dragging=-1;juce::RangedAudioParameter* gesture=nullptr;
    float get(const juce::String& id)const{return p.state.getRawParameterValue(id)->load();}
    void set(const juce::String& id,float value){auto* a=p.state.getParameter(id);a->setValueNotifyingHost(a->convertTo0to1(value));}
    void finish(){if(gesture)gesture->endChangeGesture();gesture=nullptr;dragging=-1;}
    juce::Rectangle<float> graph()const{return getLocalBounds().toFloat().reduced(8,18);}
    int stepAt(float x)const{const auto r=graph();const int n=juce::jlimit(1,16,int(get("gateStepCount")));return juce::jlimit(0,n-1,int((x-r.getX())*n/r.getWidth()));}
public:
    std::function<void()> edited;
    explicit GateBlockEditor(SpaceInfinityProcessor& processor):p(processor){setName("Trance gate blocks");}
    ~GateBlockEditor()override{finish();}
    void paint(juce::Graphics& g)override{HardwareTheme::display(g,getLocalBounds().toFloat());auto r=graph();const int n=juce::jlimit(1,16,int(get("gateStepCount"))),active=p.gateStep();const float width=r.getWidth()/n;
        for(int i=0;i<n;++i){const float value=get("gateStep"+juce::String(i));auto tile=juce::Rectangle<float>(r.getX()+width*i,r.getY(),width-1,r.getHeight());g.setColour(juce::Colour(0xff090b10));g.fillRect(tile);for(int channel=0;channel<2;++channel){auto lane=juce::Rectangle<float>(tile.getX(),tile.getY()+channel*tile.getHeight()/2,tile.getWidth(),tile.getHeight()/2-1);const float duty=get("gateStepWidth");auto fill=lane.withWidth(lane.getWidth()*duty).withTrimmedTop(lane.getHeight()*(1-value));g.setColour(juce::Colour(0xffbc87ff).withAlpha(i==active?.64f:.35f));g.fillRect(fill);g.setColour(juce::Colour(0xffd9b3ff));if(value>.001f)g.fillRect(fill.removeFromTop(2));}g.setColour(juce::Colour(i==active?0xfff0dbb0:0xff747082));g.setFont(9);g.drawText(juce::String(i+1),int(tile.getX()),int(r.getBottom()),int(width),16,juce::Justification::centred);}
        g.setColour(juce::Colour(0xffb7a9c8));g.setFont(9);g.drawText("STEREO LINKED  /  CLICK ON-OFF  /  DRAG LEVEL",8,1,getWidth()-16,15,juce::Justification::centredLeft);}
    void mouseDown(const juce::MouseEvent& e)override{finish();dragging=stepAt(e.position.x);gesture=p.state.getParameter("gateStep"+juce::String(dragging));gesture->beginChangeGesture();set("gateStep"+juce::String(dragging),get("gateStep"+juce::String(dragging))>.5f?0:1);if(edited)edited();repaint();}
    void mouseDrag(const juce::MouseEvent& e)override{if(e.getDistanceFromDragStart()<5)return;const int next=stepAt(e.position.x);if(next!=dragging){finish();dragging=next;gesture=p.state.getParameter("gateStep"+juce::String(dragging));gesture->beginChangeGesture();}set("gateStep"+juce::String(dragging),juce::jlimit(0.0f,1.0f,1-(e.position.y-graph().getY())/graph().getHeight()));if(edited)edited();repaint();}
    void mouseUp(const juce::MouseEvent&)override{finish();}
};
