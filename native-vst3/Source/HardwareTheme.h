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

    static void module(juce::Graphics& g,juce::Rectangle<float> r,bool oxblood=false) {
        g.setColour(juce::Colour(0x85000000));g.fillRoundedRectangle(r.translated(0,2),2);
        juce::ColourGradient surface(juce::Colour(oxblood?0xff683331:0xff343735),0,r.getY(),
            juce::Colour(oxblood?0xff452320:0xff242725),0,r.getBottom(),false);
        surface.addColour(.2,juce::Colour(oxblood?0xff5d2c29:0xff303330));
        g.setGradientFill(surface);g.fillRoundedRectangle(r,2);
        texture(g,r,.065f);
        g.setColour(juce::Colour(0x308e8e7a));g.drawHorizontalLine(int(r.getY()),r.getX()+1,r.getRight()-1);
        g.setColour(juce::Colour(0x9b101310));g.drawHorizontalLine(int(r.getBottom()-1),r.getX()+1,r.getRight()-1);
        g.drawVerticalLine(int(r.getRight()-1),r.getY()+1,r.getBottom()-1);
    }

    static void display(juce::Graphics& g,juce::Rectangle<float> r) {
        g.setColour(juce::Colour(0xff0a0b0a));g.fillRoundedRectangle(r.expanded(3),3);
        g.setColour(juce::Colour(0xff6b6658));g.drawRoundedRectangle(r.expanded(3),3,.7f);
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff141a17),0,r.getY(),
            juce::Colour(0xff070d0b),0,r.getBottom(),false));
        g.fillRect(r);
        g.setColour(juce::Colour(0x16e4efde));g.fillRect(r.withHeight(juce::jmin(12.0f,r.getHeight()*.14f)));
    }

    static void chassis(juce::Graphics& g,juce::Rectangle<int> area) {
        auto r=area.toFloat();
        g.fillAll(juce::Colour(0xff101210));
        for(const float x:{0.0f,r.getWidth()-23}) {
            const auto wood=juce::Rectangle<float>(x,1,23,r.getHeight()-2);
            juce::ColourGradient finish(juce::Colour(0xff291b13),x,0,juce::Colour(0xff9a6741),x+23,0,false);
            finish.addColour(.16,juce::Colour(0xff75482a));finish.addColour(.75,juce::Colour(0xff4e2e1c));
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
        g.setColour(juce::Colour(0xff1c211e));g.fillRoundedRectangle(face,3);
        texture(g,face);
        auto header=juce::Rectangle<float>(25,8,r.getWidth()-50,97);
        juce::ColourGradient enamel(juce::Colour(0xff783d37),0,8,juce::Colour(0xff411e1c),0,105,false);
        enamel.addColour(.3,juce::Colour(0xff66312d));
        g.setGradientFill(enamel);g.fillRoundedRectangle(header,2);
        texture(g,header,.095f);
        g.setColour(juce::Colour(0xffa2765d));g.drawHorizontalLine(9,28,r.getWidth()-28);
        g.setColour(juce::Colour(0xff100e0b));g.drawHorizontalLine(105,25,r.getWidth()-25);
        g.setColour(juce::Colour(0xff766c56));g.drawHorizontalLine(106,25,r.getWidth()-25);
        for(const float x:{32.0f,r.getWidth()-33})for(const float y:{18.0f,r.getHeight()-18})screw(g,x,y);
        // Folded steel rail behind the mode switches.
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff343b35),0,108,juce::Colour(0xff1c211d),0,143,false));
        g.fillRect(25,108,area.getWidth()-50,35);texture(g,{25,108,r.getWidth()-50,35});
        g.setColour(juce::Colour(0xff090c09));g.drawHorizontalLine(143,25,r.getWidth()-25);
    }

    void drawRotarySlider(juce::Graphics& g,int x,int y,int width,int height,
        float pos,float start,float end,juce::Slider& slider) override {
        const auto bounds=juce::Rectangle<float>(float(x),float(y),float(width),float(height));
        const auto c=bounds.getCentre();
        const float r=juce::jmin(43.0f,juce::jmin(bounds.getWidth(),bounds.getHeight())*.44f);
        if(r<3)return;
        const auto radial=[c](float a,float radius){return c.getPointOnCircumference(radius,a);};
        const float angle=start+pos*(end-start);
        const auto& properties=slider.getProperties();
        if(properties.contains("modDepth")) {
            const float last=start+juce::jlimit(0.0f,1.0f,pos+float(double(properties["modDepth"]))* .5f)*(end-start);
            juce::Path arc;arc.addCentredArc(c.x,c.y,r*1.09f,r*1.09f,0,std::min(angle,last),std::max(angle,last),true);
            g.setColour(juce::Colour(juce::uint32(juce::int64(properties["modColour"]))));
            g.strokePath(arc,juce::PathStrokeType(2));
        }
        for(int i=0;i<=20;++i) {
            const float a=start+(end-start)*float(i)/20;
            g.setColour(juce::Colour(i%5?0xff9b998a:0xffe0d8c5).withMultipliedAlpha(slider.isEnabled()?1.0f:.4f));
            g.drawLine({radial(a,r*(i%5?.96f:.90f)),radial(a,r*1.04f)},i%5?.65f:1.2f);
        }
        auto skirt=juce::Rectangle<float>(r*1.63f,r*1.63f).withCentre(c);
        for(int i=5;i>0;--i) {
            g.setColour(juce::Colour(0x10000000));g.fillEllipse(skirt.expanded(float(i)*.65f).translated(1.2f,3.0f+float(i)*.2f));
        }
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff6b6b5f),c.x-r,c.y-r,juce::Colour(0xff080b09),c.x+r,c.y+r,false));
        g.fillEllipse(skirt);
        g.setColour(juce::Colour(0xff090c09));g.drawEllipse(skirt,.8f);
        for(int i=0;i<40;++i) {
            const float a=angle+juce::MathConstants<float>::twoPi*float(i)/40;
            const float light=.5f+.5f*std::cos(a+2.35f);
            g.setColour(juce::Colour::fromRGB(juce::uint8(25+light*37),juce::uint8(28+light*37),juce::uint8(25+light*32)));
            g.drawLine({radial(a,r*.64f),radial(a,r*.80f)},juce::jmax(.7f,r*.045f));
        }
        const auto cap=skirt.reduced(r*.17f).translated(-.3f,-.8f);
        const bool metal=slider.getName().containsIgnoreCase("fine")||slider.getName().containsIgnoreCase("width")
            ||slider.getName().containsIgnoreCase("character");
        juce::ColourGradient top(juce::Colour(metal?0xffc2bfb0:0xff60645a),cap.getX(),cap.getY(),
            juce::Colour(metal?0xff5c6159:0xff151a16),cap.getRight(),cap.getBottom(),false);
        top.addColour(.34,juce::Colour(metal?0xffe7e1cd:0xff454b41));
        top.addColour(.68,juce::Colour(metal?0xff969c8e:0xff30382e));
        g.setGradientFill(top);g.fillEllipse(cap);
        g.setColour(juce::Colour(0x786e7566));g.drawEllipse(cap,.7f);
        g.setColour(juce::Colour(0x1811130e));g.drawEllipse(cap.reduced(1.5f),.5f);
        const auto p1=radial(angle,r*.28f),p2=radial(angle,r*.64f);
        g.setColour(juce::Colour(metal?0xff31352b:0xff10150e));g.drawLine({p1.translated(1,1),p2.translated(1,1)},3);
        g.setColour(juce::Colour(metal?0xff282c23:0xffede6d1));g.drawLine({p1,p2},juce::jmax(1.6f,r*.066f));
    }

    void drawLinearSlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float minimum,float maximum,
        juce::Slider::SliderStyle style,juce::Slider& slider) override {
        const bool vertical=style==juce::Slider::LinearVertical;
        if(style!=juce::Slider::LinearHorizontal&&!vertical) {
            LookAndFeel_V4::drawLinearSlider(g,x,y,w,h,pos,minimum,maximum,style,slider);return;
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
        juce::ColourGradient plastic(juce::Colour(down?0xff272d25:0xff555b4e),0,r.getY(),
            juce::Colour(0xff242b21),0,r.getBottom(),false);
        plastic.addColour(.48,juce::Colour(down?0xff242a20:0xff393f33));
        g.setGradientFill(plastic);g.fillRoundedRectangle(r,2);
        g.setColour(juce::Colour(lit?0xffbd8c62:0xff6d7463));g.drawHorizontalLine(int(r.getY()),r.getX()+2,r.getRight()-2);
        g.setColour(juce::Colour(0xff0d120b));g.drawHorizontalLine(int(r.getBottom()-1),r.getX()+1,r.getRight()-1);
    }

    void drawButtonBackground(juce::Graphics& g,juce::Button& b,const juce::Colour&,bool hover,bool down) override {
        const bool lit=b.getToggleState();switchBody(g,b.getLocalBounds().toFloat(),down,lit);
        if(lit) {
            g.setColour(juce::Colour(0x44ff5b29));g.fillRoundedRectangle(4,float(b.getHeight()-5),float(b.getWidth()-8),3,1);
            g.setColour(juce::Colour(0xffe6b07d));g.fillRoundedRectangle(5,float(b.getHeight()-4),float(b.getWidth()-10),1.5f,.7f);
        } else if(hover) {g.setColour(juce::Colour(0x0cfff3d0));g.fillRoundedRectangle(b.getLocalBounds().toFloat().reduced(2),2);}
    }

    void drawToggleButton(juce::Graphics& g,juce::ToggleButton& b,bool hover,bool down) override {
        const float cy=float(b.getHeight())*.5f;
        switchBody(g,{1,cy-6,19,12},down);
        const float xx=b.getToggleState()?12.0f:3.0f;
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xffa3a897),xx,cy-4,juce::Colour(0xff333c2c),xx,cy+4,false));
        g.fillRoundedRectangle(xx,cy-4,6,8,1);
        const auto lamp=juce::Rectangle<float>(3,3).withCentre({25,cy});
        if(b.getToggleState()){g.setColour(juce::Colour(0x33ff5025));g.fillEllipse(lamp.expanded(2));}
        g.setColour(juce::Colour(b.getToggleState()?0xfffc945f:0xff442c20));g.fillEllipse(lamp);
        g.setColour(juce::Colour(hover?0xfff1e7cf:0xffd3cbb7));g.setFont(juce::Font(juce::FontOptions(11.0f)));
        g.drawText(b.getButtonText(),31,0,b.getWidth()-31,b.getHeight(),juce::Justification::centredLeft,true);
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

    juce::Font getComboBoxFont(juce::ComboBox&) override {return juce::Font(juce::FontOptions(12.0f));}
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
            juce::Image image(juce::Image::RGB,256,256,false);
            juce::Random random(0x51a7cafe);
            for(int y=0;y<256;++y){
                const int brushing=random.nextInt(19)-9;
                for(int x=0;x<256;++x){
                    const auto v=juce::uint8(juce::jlimit(0,255,116+brushing+random.nextInt(57)));
                    image.setPixelAt(x,y,juce::Colour(v,v,v));
                }
            }
            return image;
        }();
        return tile;
    }
};
