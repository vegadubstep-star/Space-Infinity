#pragma once
#include <JuceHeader.h>
#include <cmath>

// Original, resolution-independent hardware artwork. Lighting stays fixed to
// the chassis while pointers and fluting follow the parameter position.
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
        setColour(juce::PopupMenu::highlightedBackgroundColourId,juce::Colour(0xff713531));
        setColour(juce::TextButton::textColourOffId,ink);
        setColour(juce::TextButton::textColourOnId,juce::Colour(0xffffe2be));
        setColour(juce::TextEditor::backgroundColourId,dark);
        setColour(juce::TextEditor::textColourId,ink);
        setColour(juce::ListBox::backgroundColourId,juce::Colour(0xff1b1d1c));
    }

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
        juce::ColourGradient metal(juce::Colour(0xff280c4b),0,r.getY(),juce::Colour(0xff351253),0,r.getBottom(),false);
        metal.addColour(.075,juce::Colour(0xffa34ce3));metal.addColour(.12,juce::Colour(0xffedd2ff));
        metal.addColour(.18,juce::Colour(0xffa53df0));metal.addColour(.40,juce::Colour(0xff60278d));
        metal.addColour(.79,juce::Colour(0xff36114e));metal.addColour(.94,juce::Colour(0xff8e3ac9));
        g.setGradientFill(metal);g.fillRoundedRectangle(r,radius);texture(g,r,.11f);
        juce::Graphics::ScopedSaveState save(g);juce::Path clip;clip.addRoundedRectangle(r,radius);g.reduceClipRegion(clip);
        juce::ColourGradient softbox(juce::Colours::transparentWhite,r.getX(),r.getY(),juce::Colours::transparentWhite,r.getRight(),r.getBottom(),false);
        softbox.addColour(.18,juce::Colour(0x28f5e7ff));softbox.addColour(.29,juce::Colour(0x05e8d2ff));softbox.addColour(.80,juce::Colour(0x00ffffff));
        g.setGradientFill(softbox);g.fillRect(r);
        g.setColour(juce::Colour(0x50e4c9fa));g.drawHorizontalLine(int(r.getY()+1),r.getX()+3,r.getRight()-3);
    }
    static void module(juce::Graphics& g,juce::Rectangle<float> r,bool violet=false){
        for(int i=4;i>0;--i){g.setColour(juce::Colours::black.withAlpha(.035f));g.fillRoundedRectangle(r.expanded(float(i)*.35f).translated(0,float(i)),4);}
        juce::ColourGradient edge(juce::Colour(0xff716677),r.getX(),r.getY(),juce::Colour(0xff08090e),r.getRight(),r.getBottom(),false);
        edge.addColour(.18,juce::Colour(0xff27222d));g.setGradientFill(edge);g.fillRoundedRectangle(r,4);
        auto face=r.reduced(1.2f);
        juce::ColourGradient surface(juce::Colour(violet?0xff723aa0:0xff4f3068),0,face.getY(),juce::Colour(0xff15151c),0,face.getBottom(),false);
        surface.addColour(.10,juce::Colour(violet?0xff51216e:0xff372143));surface.addColour(.72,juce::Colour(0xff23182d));surface.addColour(.98,juce::Colour(0xff48305b));
        g.setGradientFill(surface);g.fillRoundedRectangle(face,3);texture(g,face,.10f);
        g.setColour(juce::Colour(violet?0xaac785ff:0x887e4cac));g.drawHorizontalLine(int(face.getY()+1),face.getX()+3,face.getRight()-3);
        g.setColour(juce::Colour(0x8804050a));g.drawRoundedRectangle(r.reduced(.5f),4,.7f);
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
        const auto radial=[c](float a,float radius){return c.getPointOnCircumference(radius,a);};const float angle=start+pos*(end-start);
        for(int i=0;i<=24;++i){const float a=start+(end-start)*float(i)/24;g.setColour(juce::Colour(i%6?0xff817b8b:0xffe0d7e3).withMultipliedAlpha(slider.isEnabled()?1.0f:.4f));g.drawLine({radial(a,r*(i%6?.98f:.94f)),radial(a,r*1.07f)},i%6?.55f:1.1f);}
        auto skirt=juce::Rectangle<float>(r*1.65f,r*1.65f).withCentre(c);
        for(int i=6;i>0;--i){g.setColour(juce::Colours::black.withAlpha(.055f));g.fillEllipse(skirt.expanded(float(i)*.45f).translated(1,2+float(i)*.35f));}
        juce::ColourGradient rim(juce::Colour(0xffb2a7bf),c.x-r,c.y-r,juce::Colour(0xff090a10),c.x+r,c.y+r,false);rim.addColour(.25,juce::Colour(0xff55515e));rim.addColour(.54,juce::Colour(0xff13141b));g.setGradientFill(rim);g.fillEllipse(skirt.expanded(.9f));
        g.setColour(juce::Colour(0xff12131a));g.fillEllipse(skirt.reduced(.7f));
        for(int i=0;i<48;++i){const float a=angle+juce::MathConstants<float>::twoPi*float(i)/48;const float light=.5f+.5f*std::cos(a+2.4f);
            g.setColour(juce::Colour::fromRGB(juce::uint8(17+light*66),juce::uint8(18+light*63),juce::uint8(25+light*69)));g.drawLine({radial(a,r*.60f),radial(a,r*.79f)},juce::jmax(.65f,r*.045f));}
        const auto cap=skirt.reduced(r*.17f).translated(-.35f,-.9f);const bool champagne=slider.getName().containsIgnoreCase("fine")||slider.getName().containsIgnoreCase("width");
        juce::ColourGradient metal(juce::Colour(champagne?0xfff1e7d4:0xffd9dbe4),cap.getX(),cap.getY(),juce::Colour(champagne?0xff5e5651:0xff444654),cap.getRight(),cap.getBottom(),false);
        metal.addColour(.20,juce::Colour(champagne?0xffb5aa95:0xff7d8291));metal.addColour(.34,juce::Colour(champagne?0xfffff3dc:0xffeef0f5));metal.addColour(.55,juce::Colour(champagne?0xffc5b89e:0xffa0a4b3));metal.addColour(.78,juce::Colour(champagne?0xff877e71:0xff666b7c));
        g.setGradientFill(metal);g.fillEllipse(cap);
        {juce::Graphics::ScopedSaveState save(g);juce::Path clip;clip.addEllipse(cap);g.reduceClipRegion(clip);
            for(float yy=cap.getY();yy<cap.getBottom();yy+=1.4f){g.setColour(juce::Colour(0x0c10121b));g.drawHorizontalLine(int(yy),cap.getX(),cap.getRight());}
            for(int i=1;i<7;++i){g.setColour(juce::Colour(0x121c1823));g.drawEllipse(cap.reduced(float(i)*r*.045f),.45f);}}
        g.setColour(juce::Colour(0xbde9e4f1));g.drawEllipse(cap,.65f);g.setColour(juce::Colour(0x9022222d));g.drawEllipse(cap.reduced(1),.65f);
        juce::Path lightArc;lightArc.addCentredArc(c.x,c.y,r*.89f,r*.89f,0,start,angle,true);g.setColour(juce::Colour(0xb9ce81ff));g.strokePath(lightArc,juce::PathStrokeType(1.3f));
        const auto& props=slider.getProperties();if(props.contains("modDepth")){const float last=start+juce::jlimit(0.0f,1.0f,pos+float(double(props["modDepth"]))*.5f)*(end-start);juce::Path arc;arc.addCentredArc(c.x,c.y,r*1.13f,r*1.13f,0,std::min(angle,last),std::max(angle,last),true);g.setColour(juce::Colour(juce::uint32(juce::int64(props["modColour"]))));g.strokePath(arc,juce::PathStrokeType(2));}
        const auto p1=radial(angle,r*.22f),p2=radial(angle,r*.62f);g.setColour(juce::Colour(0x90ffffff));g.drawLine({p1.translated(.7f,.8f),p2.translated(.7f,.8f)},juce::jmax(2.2f,r*.085f));g.setColour(juce::Colour(0xff11131d));g.drawLine({p1,p2},juce::jmax(1.7f,r*.064f));
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
        g.setColour(juce::Colour(0xff090d09));g.fillRoundedRectangle(r,3);
        r=r.reduced(1.5f);if(down)r=r.translated(0,1);
        juce::ColourGradient plastic(juce::Colour(down?0xff272d25:0xff605765),0,r.getY(),
            juce::Colour(0xff191722),0,r.getBottom(),false);
        plastic.addColour(.48,juce::Colour(down?0xff242a20:0xff36303d));
        g.setGradientFill(plastic);g.fillRoundedRectangle(r,2);
        g.setColour(juce::Colour(lit?0xffc3a1e6:0xff6d7463));g.drawHorizontalLine(int(r.getY()),r.getX()+2,r.getRight()-2);
        g.setColour(juce::Colour(0xff0d120b));g.drawHorizontalLine(int(r.getBottom()-1),r.getX()+1,r.getRight()-1);
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
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xffa3a897),xx,cy-4,juce::Colour(0xff333c2c),xx,cy+4,false));
        g.fillRoundedRectangle(xx,cy-4,6,8,1);
        const bool compact=b.getWidth()<70;const auto lamp=juce::Rectangle<float>(3,3).withCentre({compact?20.0f:25.0f,cy});
        if(b.getToggleState()){g.setColour(juce::Colour(0x339d70df));g.fillEllipse(lamp.expanded(2));}
        g.setColour(juce::Colour(b.getToggleState()?0xffd9b9ff:0xff35253e));g.fillEllipse(lamp);
        g.setColour(juce::Colour(hover?0xfff1e7cf:0xffd3cbb7));g.setFont(juce::Font(juce::FontOptions(compact?9.5f:11.0f)));
        const int textX=compact?23:31;g.drawText(b.getButtonText(),textX,0,b.getWidth()-textX,b.getHeight(),juce::Justification::centredLeft,true);
    }

    void drawComboBox(juce::Graphics& g,int w,int h,bool down,int,int,int,int,juce::ComboBox& box) override {
        auto r=box.getLocalBounds().toFloat().reduced(.5f);
        const bool program=box.getTextWhenNothingSelected()=="Select a sound";
        g.setColour(juce::Colour(0xff070b07));g.fillRoundedRectangle(r,program?4.0f:2.0f);
        g.setColour(juce::Colour(program?0xffa48160:0xff585e4e));g.drawRoundedRectangle(r,program?4.0f:2.0f,.75f);
        r=r.reduced(1.5f);
        g.setGradientFill(juce::ColourGradient(juce::Colour(down?0xff10160f:0xff1b251b),0,r.getY(),juce::Colour(0xff10180f),0,r.getBottom(),false));
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
    static const juce::Image& grain() {
        static const auto tile=[] {
            juce::Image image(juce::Image::RGB,1024,128,false);juce::Random random(0x51a7cafe);
            for(int y=0;y<128;++y){const int brushing=random.nextInt(19)-9;float streak=float(random.nextInt(9)-4);
                for(int x=0;x<1024;++x){streak=.98f*streak+.02f*float(random.nextInt(21)-10);const auto v=juce::uint8(juce::jlimit(0,255,128+brushing+int(streak)+random.nextInt(11)-5));image.setPixelAt(x,y,juce::Colour(v,v,v));}}
            return image;
        }();
        return tile;
    }
};
