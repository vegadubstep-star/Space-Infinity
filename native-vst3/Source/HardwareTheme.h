#pragma once
#include <JuceHeader.h>
#include <cmath>

// Original HiDPI material artwork. Studio lighting stays fixed to the chassis
// while engraved pointers and value rings follow each parameter.
class HardwareTheme : public juce::LookAndFeel_V4 {
public:
    HardwareTheme() {
        const auto ink=juce::Colour(0xffe0d7c7), dark=juce::Colour(0xff151615);
        setColour(juce::Label::textColourId,ink);
        setColour(juce::Slider::textBoxTextColourId,ink);
        setColour(juce::Slider::textBoxBackgroundColourId,juce::Colours::transparentBlack);
        setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
        setColour(juce::ComboBox::textColourId,ink);
        setColour(juce::ComboBox::backgroundColourId,dark);
        setColour(juce::ComboBox::outlineColourId,juce::Colours::transparentBlack);
        setColour(juce::PopupMenu::backgroundColourId,juce::Colour(0xff202220));
        setColour(juce::PopupMenu::textColourId,ink);
        setColour(juce::PopupMenu::highlightedBackgroundColourId,juce::Colour(0xff713caa));
        setColour(juce::TextButton::textColourOffId,ink);
        setColour(juce::TextButton::textColourOnId,juce::Colour(0xffffe2be));
        setColour(juce::TextEditor::backgroundColourId,dark);
        setColour(juce::TextEditor::textColourId,ink);
        setColour(juce::ListBox::backgroundColourId,juce::Colour(0xff1b1d1c));
    }

    static void signalGradient(juce::Graphics& g,juce::Rectangle<float> area,float alpha=1){juce::ColourGradient light(juce::Colour(0xffff65ba).withMultipliedAlpha(alpha),area.getX(),area.getY(),juce::Colour(0xff48e7dc).withMultipliedAlpha(alpha),area.getRight(),area.getBottom(),false);light.addColour(.48,juce::Colour(0xffc0b9ff).withMultipliedAlpha(alpha));g.setGradientFill(light);}
    static void texture(juce::Graphics& g,juce::Rectangle<float> r,float alpha=.055f) {
        juce::Graphics::ScopedSaveState save(g);
        g.reduceClipRegion(r.toNearestInt());
        g.setTiledImageFill(grain(),0,0,alpha);
        g.fillRect(r);
    }

    static void screw(juce::Graphics& g,float x,float y,float radius=3.8f) {
        auto r=juce::Rectangle<float>(radius*2,radius*2).withCentre({x,y});
        g.setColour(juce::Colour(0x99000000));g.fillEllipse(r.expanded(1).translated(0,1));
        juce::ColourGradient metal(juce::Colour(0xffa29c8c),x-radius,y-radius,
            juce::Colour(0xff252624),x+radius,y+radius,false);
        metal.addColour(.48,juce::Colour(0xff66675f));
        g.setGradientFill(metal);g.fillEllipse(r);
        g.setColour(juce::Colour(0xff121512));g.drawEllipse(r,.6f);
        g.drawLine(x-radius*.56f,y+radius*.35f,x+radius*.56f,y-radius*.35f,1.4f);
        g.setColour(juce::Colour(0x508b8d7e));
        g.drawLine(x-radius*.5f,y+radius*.35f+1,x+radius*.5f,y-radius*.35f+1,.5f);
    }

    static void purpleMetal(juce::Graphics& g,juce::Rectangle<float> r,float radius=3){
        juce::Graphics::ScopedSaveState save(g);juce::Path clip;clip.addRoundedRectangle(r,radius);g.reduceClipRegion(clip);
        juce::ColourGradient metal(juce::Colour(0xff401064),r.getX(),r.getY(),juce::Colour(0xff3c0a65),r.getX(),r.getBottom(),false);
        metal.addColour(.07,juce::Colour(0xffa34be2));metal.addColour(.22,juce::Colour(0xff8a24d2));metal.addColour(.49,juce::Colour(0xff6416a9));metal.addColour(.78,juce::Colour(0xff400e70));metal.addColour(.96,juce::Colour(0xff7027a4));
        g.setGradientFill(metal);g.fillRect(r);texture(g,r,.21f);
        juce::ColourGradient reflection(juce::Colours::transparentWhite,r.getX(),r.getY(),juce::Colours::transparentWhite,r.getRight(),r.getBottom(),false);
        reflection.addColour(.16,juce::Colour(0x03ffffff));reflection.addColour(.30,juce::Colour(0x36efcfff));reflection.addColour(.42,juce::Colour(0x0ac9a5f0));reflection.addColour(.64,juce::Colour(0x00ffffff));g.setGradientFill(reflection);g.fillRect(r);
        g.setColour(juce::Colour(0xff1e072d));g.drawRoundedRectangle(r.reduced(.4f),radius,.8f);
        g.setColour(juce::Colour(0xa5d69aff));g.drawHorizontalLine(int(r.getY()+1),r.getX()+3,r.getRight()-3);
        g.setColour(juce::Colour(0xb00a0410));g.drawHorizontalLine(int(r.getBottom()-1),r.getX()+2,r.getRight()-2);
    }
    static void module(juce::Graphics& g,juce::Rectangle<float> r,bool violet=false){
        for(int i=6;i>0;--i){g.setColour(juce::Colours::black.withAlpha(.037f));g.fillRoundedRectangle(r.expanded(float(i)*.23f).translated(float(i)*.12f,float(i)*.65f),4);}
        juce::ColourGradient rim(juce::Colour(0xff9884a4),r.getX(),r.getY(),juce::Colour(0xff08050e),r.getRight(),r.getBottom(),false);rim.addColour(.12,juce::Colour(0xff57405f));rim.addColour(.55,juce::Colour(0xff1b1124));g.setGradientFill(rim);g.fillRoundedRectangle(r,4);
        const auto face=r.reduced(1.5f);juce::ColourGradient paint(juce::Colour(violet?0xff542174:0xff3e224e),face.getX(),face.getY(),juce::Colour(0xff160c21),face.getRight(),face.getBottom(),false);
        paint.addColour(.19,juce::Colour(violet?0xff351345:0xff2d1839));paint.addColour(.72,juce::Colour(0xff211329));paint.addColour(.97,juce::Colour(0xff2d1a39));g.setGradientFill(paint);g.fillRoundedRectangle(face,3);texture(g,face,.12f);
        g.setColour(juce::Colour(0xa89155b7));g.drawHorizontalLine(int(face.getY()+1),face.getX()+3,face.getRight()-3);
        g.setColour(juce::Colour(0x36000000));g.drawRoundedRectangle(face.reduced(.5f),3,.7f);
        g.setColour(juce::Colour(0x30000000));g.drawHorizontalLine(int(face.getBottom()-1),face.getX()+3,face.getRight()-3);
    }
    static void display(juce::Graphics& g,juce::Rectangle<float> r){
        auto bezel=r.expanded(3);
        g.setColour(juce::Colour(0x99000000));g.fillRoundedRectangle(bezel.translated(0,2),4);
        juce::ColourGradient rim(juce::Colour(0xff8c8394),bezel.getX(),bezel.getY(),juce::Colour(0xff111019),bezel.getRight(),bezel.getBottom(),false);
        rim.addColour(.2,juce::Colour(0xff292533));g.setGradientFill(rim);g.fillRoundedRectangle(bezel,4);
        g.setColour(juce::Colour(0xff03050a));g.fillRoundedRectangle(bezel.reduced(1.2f),3);
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff10131d),0,r.getY(),juce::Colour(0xff04070d),0,r.getBottom(),false));g.fillRect(r);
        juce::Graphics::ScopedSaveState save(g);g.reduceClipRegion(r.toNearestInt());
        juce::ColourGradient reflection(juce::Colour(0x16d8e3f4),r.getX(),r.getY(),juce::Colours::transparentWhite,r.getCentreX(),r.getBottom(),false);
        reflection.addColour(.30,juce::Colour(0x06ffffff));reflection.addColour(.40,juce::Colours::transparentWhite);g.setGradientFill(reflection);g.fillRect(r);
        g.setColour(juce::Colour(0x18c5c0da));g.drawHorizontalLine(int(r.getY()),r.getX(),r.getRight());
    }

    static void chassis(juce::Graphics& g,juce::Rectangle<int> area) {
        auto r=area.toFloat();
        g.fillAll(juce::Colour(0xff101210));
        for(const float x:{0.0f,r.getWidth()-23}) {
            const auto wood=juce::Rectangle<float>(x,1,23,r.getHeight()-2);
            juce::ColourGradient finish(juce::Colour(0xff140f11),x,0,juce::Colour(0xff68452e),x+23,0,false);
            finish.addColour(.16,juce::Colour(0xff4f3228));finish.addColour(.75,juce::Colour(0xff281c1b));
            g.setGradientFill(finish);g.fillRoundedRectangle(wood,4);
            juce::Graphics::ScopedSaveState save(g);g.reduceClipRegion(wood.toNearestInt());
            // Long, irregular longitudinal pores, rather than evenly spaced stripes.
            for(int i=0;i<36;++i) {
                juce::Path pore;const float base=x+float(i)*.71f;
                for(int y=0;y<int(r.getHeight());y+=8) {
                    const float px=base+.55f*std::sin(float(y)*.021f+float(i)*1.7f)
                        +.3f*std::sin(float(y)*.074f+float(i)*.8f);
                    if(y==0)pore.startNewSubPath(px,0);else pore.lineTo(px,float(y));
                }
                g.setColour(juce::Colour(i%3?0x38200d05:0x327d4b28));
                g.strokePath(pore,juce::PathStrokeType(i%4?.5f:1.1f));
            }
            texture(g,wood,.13f);
        }
        auto face=r.reduced(22,4);
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff4c4b43),0,0,juce::Colour(0xff151916),0,r.getBottom(),false));
        g.fillRoundedRectangle(face,4);
        face=face.reduced(2);
        g.setColour(juce::Colour(0xff111219));g.fillRoundedRectangle(face,3);
        texture(g,face);
        auto header=juce::Rectangle<float>(25,8,r.getWidth()-50,97);
        purpleMetal(g,header);
        g.setColour(juce::Colour(0xffa2765d));g.drawHorizontalLine(9,28,r.getWidth()-28);
        g.setColour(juce::Colour(0xff100e0b));g.drawHorizontalLine(105,25,r.getWidth()-25);
        g.setColour(juce::Colour(0xff766c56));g.drawHorizontalLine(106,25,r.getWidth()-25);
        for(const float x:{32.0f,r.getWidth()-33})for(const float y:{18.0f,r.getHeight()-18})screw(g,x,y);
        // Folded steel rail behind the mode switches.
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff42304c),0,108,juce::Colour(0xff211a2b),0,143,false));
        g.fillRect(25,108,area.getWidth()-50,35);texture(g,{25,108,r.getWidth()-50,35});
        g.setColour(juce::Colour(0xff090c09));g.drawHorizontalLine(143,25,r.getWidth()-25);
    }

    void drawRotarySlider(juce::Graphics& g,int x,int y,int width,int height,float pos,float start,float end,juce::Slider& slider) override {
        const auto bounds=juce::Rectangle<float>(float(x),float(y),float(width),float(height));const auto c=bounds.getCentre();
        const float r=juce::jmin(43.0f,juce::jmin(bounds.getWidth(),bounds.getHeight())*.44f);if(r<3)return;
        juce::Graphics::ScopedSaveState save(g);const float alpha=slider.isEnabled()?1.0f:.42f;g.setOpacity(alpha);
        const auto radial=[c](float a,float radius){return c.getPointOnCircumference(radius,a);};const float angle=start+pos*(end-start);
        // Etched calibrations are on the panel, outside the recessed socket.
        for(int i=0;i<=24;++i){const float a=start+(end-start)*float(i)/24;const auto a1=radial(a,r*(i%6?1.01f:.97f)),a2=radial(a,r*1.09f);g.setColour(juce::Colour(0xcc08040d));g.drawLine({a1.translated(0,.7f),a2.translated(0,.7f)},i%6?1.1f:1.7f);g.setColour(juce::Colour(i%6?0xff85738e:0xffdbcedf));g.drawLine({a1,a2},i%6?.5f:.9f);}
        const auto socket=juce::Rectangle<float>(r*1.89f,r*1.89f).withCentre(c);g.setGradientFill(juce::ColourGradient(juce::Colour(0xff07040d),socket.getX(),socket.getY(),juce::Colour(0xff75517f),socket.getRight(),socket.getBottom(),false));g.fillEllipse(socket);
        g.setColour(juce::Colour(0xff0c0712));g.fillEllipse(socket.reduced(1));g.setColour(juce::Colour(0x605e3977));g.drawEllipse(socket.reduced(.5f),.6f);
        const bool violet=slider.getName().containsIgnoreCase("macro")||slider.getName().containsIgnoreCase("space")||slider.getName().containsIgnoreCase("energy")||slider.getName().containsIgnoreCase("gain")||slider.getName().containsIgnoreCase("output");
        g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);g.setOpacity(alpha);const float extent=r*.82f*2*1.30f;g.drawImage(knob(violet),juce::Rectangle<float>(extent,extent).withCentre(c));
        juce::Path track;track.addCentredArc(c.x,c.y,r*.94f,r*.94f,0,start,end,true);g.setColour(juce::Colour(0xff160b21));g.strokePath(track,juce::PathStrokeType(2.1f));
        if(pos>.001f){juce::Path arc;arc.addCentredArc(c.x,c.y,r*.94f,r*.94f,0,start,angle,true);g.setColour(juce::Colour(0x2453e9dc));g.strokePath(arc,juce::PathStrokeType(4));signalGradient(g,socket,.9f);g.strokePath(arc,juce::PathStrokeType(1.15f));}
        const auto& props=slider.getProperties();if(props.contains("modDepth")){const float last=start+juce::jlimit(0.0f,1.0f,pos+float(double(props["modDepth"]))*.5f)*(end-start);juce::Path arc;arc.addCentredArc(c.x,c.y,r*1.13f,r*1.13f,0,std::min(angle,last),std::max(angle,last),true);g.setColour(juce::Colour(juce::uint32(juce::int64(props["modColour"]))));g.strokePath(arc,juce::PathStrokeType(2));}
        // A recessed index with a lit insert; the metal's studio lighting stays fixed.
        const auto p1=radial(angle,r*.26f),p2=radial(angle,r*.58f);const float line=juce::jmax(1.6f,r*.075f);g.setColour(juce::Colour(0x90000000));g.drawLine({p1.translated(.7f,1),p2.translated(.7f,1)},line+1);g.setColour(juce::Colour(violet?0xff100717:0xff252531));g.drawLine({p1,p2},line);g.setColour(juce::Colour(violet?0xffbcfff3:0xff12101a));g.drawLine({p1.translated(-.25f,-.35f),p2.translated(-.25f,-.35f)},juce::jmax(.75f,line*.48f));
    }

    void drawLinearSlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float minimum,float maximum,
        juce::Slider::SliderStyle style,juce::Slider& slider) override {
        const bool vertical=style==juce::Slider::LinearVertical;
        if(style!=juce::Slider::LinearHorizontal&&!vertical) {
            LookAndFeel_V4::drawLinearSlider(g,x,y,w,h,pos,minimum,maximum,style,slider);return;
        }
        if(vertical&&slider.getName()=="Pitch Wheel"){
            auto r=juce::Rectangle<float>(float(x+w/2-12),float(y),24,float(h));g.setColour(juce::Colour(0xff09070e));g.fillRoundedRectangle(r.expanded(3,1),4);juce::ColourGradient drum(juce::Colour(0xff32253e),r.getX(),r.getY(),juce::Colour(0xff32253e),r.getX(),r.getBottom(),false);drum.addColour(.5,juce::Colour(0xffb9a2d4));g.setGradientFill(drum);g.fillRoundedRectangle(r,4);g.setColour(juce::Colour(0xff312139));for(int line=0;line<13;++line){const float yy=r.getY()+2+(r.getHeight()-4)*line/12;g.drawHorizontalLine(int(yy),r.getX()+2,r.getRight()-2);}g.setColour(juce::Colour(0xfff4dcff));g.fillRoundedRectangle(r.getX()+2,juce::jlimit(r.getY(),r.getBottom()-3,pos-1.5f),r.getWidth()-4,3,1);g.setColour(juce::Colour(0xffb28ad9));g.drawRoundedRectangle(r,4,1);return;
        }
        const float cx=float(x)+float(w)*.5f,cy=float(y)+float(h)*.5f;
        for(int i=0;i<=10;++i) {
            const float t=float(i)/10;
            g.setColour(juce::Colour(i%5?0xff6d7163:0xffa8aa96));
            if(vertical) {
                const float yy=float(y+h)-float(h)*t;
                g.drawHorizontalLine(int(yy),cx-11,cx-5);g.drawHorizontalLine(int(yy),cx+5,cx+11);
            } else {
                const float xx=float(x)+float(w)*t;
                g.drawVerticalLine(int(xx),cy+5,cy+(i%5?7.0f:9.0f));
            }
        }
        auto slot=vertical?juce::Rectangle<float>(cx-2.5f,float(y)-1,5,float(h)+2)
            :juce::Rectangle<float>(float(x)-1,cy-2.5f,float(w)+2,5);
        g.setColour(juce::Colour(0xff080c08));g.fillRoundedRectangle(slot,2.5f);
        g.setColour(juce::Colour(0xff54594c));g.drawRoundedRectangle(slot,2.5f,.6f);
        auto cap=vertical?juce::Rectangle<float>(24,12).withCentre({cx,pos})
            :juce::Rectangle<float>(12,21).withCentre({pos,cy});
        g.setColour(juce::Colour(0x88000000));g.fillRoundedRectangle(cap.expanded(1).translated(1,2),2);
        juce::ColourGradient plastic(juce::Colour(0xff717568),0,cap.getY(),juce::Colour(0xff191e17),0,cap.getBottom(),false);
        plastic.addColour(.45,juce::Colour(0xff3f473b));plastic.addColour(.52,juce::Colour(0xff232b21));
        g.setGradientFill(plastic);g.fillRoundedRectangle(cap,1.8f);
        g.setColour(juce::Colour(0xff0d130c));g.drawRoundedRectangle(cap,1.8f,.8f);
        g.setColour(juce::Colour(0xffd29965));
        if(vertical)g.fillRect(cap.reduced(3,0).withY(pos-.8f).withHeight(1.6f));
        else g.fillRect(cap.reduced(0,3).withX(pos-.8f).withWidth(1.6f));
    }

    static void switchBody(juce::Graphics& g,juce::Rectangle<float> r,bool down,bool lit=false) {
        g.setColour(juce::Colour(0x90000000));g.fillRoundedRectangle(r.translated(.6f,1.6f),3);
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff76627e),r.getX(),r.getY(),juce::Colour(0xff08050e),r.getRight(),r.getBottom(),false));g.fillRoundedRectangle(r,3);
        auto face=r.reduced(1.2f);if(down)face=face.translated(.2f,.65f);
        juce::ColourGradient finish(juce::Colour(down?0xff21122c:lit?0xff5a2d78:0xff4e3c59),face.getX(),face.getY(),juce::Colour(0xff120b1b),face.getRight(),face.getBottom(),false);finish.addColour(.22,juce::Colour(down?0xff24152d:0xff3a2945));finish.addColour(.78,juce::Colour(0xff201429));g.setGradientFill(finish);g.fillRoundedRectangle(face,2);
        texture(g,face,.18f);g.setColour(juce::Colour(lit?0xffb780e2:0xff897091).withAlpha(down?.2f:.55f));g.drawHorizontalLine(int(face.getY()),face.getX()+2,face.getRight()-2);g.setColour(juce::Colour(0xb0050208));g.drawHorizontalLine(int(face.getBottom()-1),face.getX()+1,face.getRight()-1);
    }

    void drawButtonBackground(juce::Graphics& g,juce::Button& b,const juce::Colour&,bool hover,bool down) override {
        const bool lit=b.getToggleState();switchBody(g,b.getLocalBounds().toFloat(),down,lit);
        if(lit) {
            g.setColour(juce::Colour(0x44956bdb));g.fillRoundedRectangle(4,float(b.getHeight()-5),float(b.getWidth()-8),3,1);
            g.setColour(juce::Colour(0xffd3b6f2));g.fillRoundedRectangle(5,float(b.getHeight()-4),float(b.getWidth()-10),1.5f,.7f);
        } else if(hover) {g.setColour(juce::Colour(0x0cfff3d0));g.fillRoundedRectangle(b.getLocalBounds().toFloat().reduced(2),2);}
    }

    void drawToggleButton(juce::Graphics& g,juce::ToggleButton& b,bool hover,bool down) override {
        const float cy=float(b.getHeight())*.5f;
        switchBody(g,{1,cy-6,19,12},down);
        const float xx=b.getToggleState()?12.0f:3.0f;
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xffb7afc4),xx,cy-4,juce::Colour(0xff292131),xx,cy+4,false));
        g.fillRoundedRectangle(xx,cy-4,6,8,1);
        const bool compact=b.getWidth()<70;const auto lamp=juce::Rectangle<float>(3,3).withCentre({compact?20.0f:25.0f,cy});
        if(b.getToggleState()){for(int halo=4;halo>0;--halo){g.setColour(juce::Colour(0xffed78dc).withAlpha(.035f*float(5-halo)));g.fillEllipse(lamp.expanded(float(halo)));}}
        g.setColour(juce::Colour(b.getToggleState()?0xff64f3df:0xff35253e));g.fillEllipse(lamp);if(b.getToggleState()){g.setColour(juce::Colour(0xffeffffb));g.fillEllipse(lamp.reduced(.8f).translated(-.25f,-.25f));}
        g.setColour(juce::Colour(hover?0xfff1e7cf:0xffd3cbb7));g.setFont(juce::Font(juce::FontOptions(compact?9.5f:11.0f)));
        const int textX=compact?23:31;g.drawText(b.getButtonText(),textX,0,b.getWidth()-textX,b.getHeight(),juce::Justification::centredLeft,true);
    }

    void drawComboBox(juce::Graphics& g,int w,int h,bool down,int,int,int,int,juce::ComboBox& box) override {
        auto r=box.getLocalBounds().toFloat().reduced(.5f);
        const bool program=box.getTextWhenNothingSelected()=="Select a sound";
        g.setColour(juce::Colour(0xff100916));g.fillRoundedRectangle(r,program?4.0f:2.0f);
        g.setColour(juce::Colour(program?0xffad7ac9:0xff72537e));g.drawRoundedRectangle(r,program?4.0f:2.0f,.75f);
        r=r.reduced(1.5f);
        g.setGradientFill(juce::ColourGradient(juce::Colour(down?0xff10160f:0xff21142d),0,r.getY(),juce::Colour(0xff0e0815),0,r.getBottom(),false));
        g.fillRoundedRectangle(r,2);
        juce::Path chevron;chevron.startNewSubPath(float(w)-17,float(h)*.5f-1.5f);
        chevron.lineTo(float(w)-13,float(h)*.5f+2);chevron.lineTo(float(w)-9,float(h)*.5f-1.5f);
        g.setColour(juce::Colour(0xffc2b58f));g.strokePath(chevron,juce::PathStrokeType(1));
    }

    juce::Font getComboBoxFont(juce::ComboBox& box) override {return juce::Font(juce::FontOptions(box.getWidth()<60?10.0f:12.0f));}
    void positionComboBoxText(juce::ComboBox& box,juce::Label& label)override{label.setBounds(3,1,juce::jmax(1,box.getWidth()-24),box.getHeight()-2);label.setFont(getComboBoxFont(box));}
    juce::Font getTextButtonFont(juce::TextButton&,int h) override {return juce::Font(juce::FontOptions(juce::jmin(11.5f,float(h)*.45f),juce::Font::bold));}

    juce::Label* createSliderTextBox(juce::Slider& slider) override {
        auto* label=LookAndFeel_V4::createSliderTextBox(slider);
        label->setColour(juce::Label::outlineColourId,juce::Colours::transparentBlack);
        label->setColour(juce::Label::backgroundColourId,juce::Colours::transparentBlack);
        label->setColour(juce::Label::textColourId,juce::Colour(0xffddd3bc));
        label->setColour(juce::TextEditor::backgroundColourId,juce::Colour(0xff141b12));
        label->setColour(juce::TextEditor::outlineColourId,juce::Colour(0xffad8861));
        label->setFont(juce::Font(juce::FontOptions(11.0f)));
        return label;
    }

private:
    static const juce::Image& knob(bool violet);
    static const juce::Image& grain();
};
