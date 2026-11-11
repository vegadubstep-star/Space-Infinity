#pragma once
#include "PluginProcessor.h"
#include "HardwareTheme.h"
class SamplerDisplay final : public juce::Component,public juce::FileDragAndDropTarget {
public:
    SamplerDisplay(SpaceInfinityProcessor& p,bool live):processor(p),output(live){setName(live?"Live post-FX output waveform":"Loaded sample waveform — drag trim markers; drop WAV to load");}
    void paint(juce::Graphics& g)override{
        auto r=getLocalBounds().toFloat().reduced(3);HardwareTheme::display(g,r);
        const int source=int(value("source"));
        g.setFont(9);g.setColour(juce::Colour(0xffe8d6f8));
        const auto title=output?juce::String("LIVE OUTPUT / POST FX"):source==3||source==4?processor.state.state.getProperty("sampleName","IMPORTED WAV").toString():processor.sourceNames()[source];
        g.drawText(title,getLocalBounds().reduced(7).withHeight(12),juce::Justification::centredLeft);
        r=r.withTrimmedTop(14).withTrimmedBottom(3);
        g.setColour(juce::Colour(0xff2f3543));g.drawHorizontalLine(int(r.getCentreY()),r.getX(),r.getRight());
        if(output){
            const auto samples=processor.outputScope();juce::Path path;
            for(int i=0;i<1024;++i){const float x=r.getX()+r.getWidth()*i/1023,y=r.getCentreY()-samples[size_t(i)]*r.getHeight()*.48f;if(i==0)path.startNewSubPath(x,y);else path.lineTo(x,y);}
            g.setColour(juce::Colour(0xffcdaef3));g.strokePath(path,juce::PathStrokeType(1.25f));
            return;
        }
        processor.sampleView(source,cache);
        if(cache.samples==0){g.setColour(juce::Colour(0xffaaa0ba));g.drawText("DROP A WAV HERE",r,juce::Justification::centred);return;}
        const bool reverse=value("sampleReverse")>.5f;const float start=value("sampleStart"),end=juce::jmax(start,value("sampleEnd"));
        const float a=reverse?1-end:start,b=reverse?1-start:end;
        g.setColour(juce::Colour(0x444e3296));g.fillRect(r.withX(r.getX()+r.getWidth()*a).withWidth(r.getWidth()*(b-a)));
        for(int i=0;i<512;++i){const int index=reverse?511-i:i;const float x=r.getX()+r.getWidth()*i/511;
            g.setColour(juce::Colour(float(i)/511>=a&&float(i)/511<=b?0xffe7c391:0xff61586b));
            g.drawVerticalLine(int(x),r.getCentreY()-cache.high[size_t(index)]*r.getHeight()*.48f,r.getCentreY()-cache.low[size_t(index)]*r.getHeight()*.48f);}
        g.setColour(juce::Colour(0xffdfb5fa));for(float point:{a,b})g.drawVerticalLine(int(r.getX()+r.getWidth()*point),r.getY(),r.getBottom());
        const float playhead=processor.samplePlayhead();if(playhead>=0){g.setColour(juce::Colour(0xffc4f0bc));const float point=reverse?1-playhead:playhead;g.drawVerticalLine(int(r.getX()+r.getWidth()*point),r.getY(),r.getBottom());}
    }
    bool isInterestedInFileDrag(const juce::StringArray& files)override{return !output&&files.size()==1&&files[0].endsWithIgnoreCase(".wav");}
    void filesDropped(const juce::StringArray& files,int,int)override{if(!files.size())return;const auto result=processor.importSample(juce::File(files[0]));if(result.failed())juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,"Sample",result.getErrorMessage());repaint();}
    void mouseDown(const juce::MouseEvent& e)override{
        if(output)return;const float at=position(e);dragStart=std::abs(at-value("sampleStart"))<std::abs(at-value("sampleEnd"));dragging=true;
        processor.state.getParameter(dragStart?"sampleStart":"sampleEnd")->beginChangeGesture();mouseDrag(e);
    }
    void mouseDrag(const juce::MouseEvent& e)override{if(!dragging)return;auto* p=processor.state.getParameter(dragStart?"sampleStart":"sampleEnd");
        const float at=dragStart?juce::jlimit(0.0f,juce::jmax(0.0f,value("sampleEnd")-.001f),position(e)):juce::jlimit(juce::jmin(1.0f,value("sampleStart")+.001f),1.0f,position(e));p->setValueNotifyingHost(at);repaint();}
    void mouseUp(const juce::MouseEvent&)override{finish();}
    void mouseWheelMove(const juce::MouseEvent&,const juce::MouseWheelDetails& wheel)override{if(!output)return;auto* p=processor.state.getParameter("scopeWindow");p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(juce::jlimit(.005f,.15f,value("scopeWindow")*std::exp(-wheel.deltaY*2))));p->endChangeGesture();}
    ~SamplerDisplay()override{finish();}
private:
    SpaceInfinityProcessor& processor;bool output=false,dragging=false,dragStart=false;SpaceInfinityProcessor::SampleView cache;
    float value(const char* id)const{return processor.state.getRawParameterValue(id)->load();}
    float position(const juce::MouseEvent& e)const{const float x=juce::jlimit(0.0f,1.0f,(float(e.x)-3)/juce::jmax(1,getWidth()-6));return value("sampleReverse")>.5f?1-x:x;}
    void finish(){if(dragging)processor.state.getParameter(dragStart?"sampleStart":"sampleEnd")->endChangeGesture();dragging=false;}
};
