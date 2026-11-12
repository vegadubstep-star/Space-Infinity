#pragma once
#include <juce_graphics/juce_graphics.h>

// Bake letter masks with the software renderer. Native Windows painting then
// receives complete, zero-origin RGBA images rather than cropped alpha masks.
struct LogoArtwork {
    juce::Image shadow,metal,light;
    explicit LogoArtwork(const juce::Image& master){
        if(!master.isValid())return;
        juce::Rectangle<int> ink;
        const auto coverage=[](juce::Colour pixel){return juce::jlimit(0.0f,1.0f,(pixel.getBrightness()-.12f)/.76f)*pixel.getFloatAlpha();};
        {juce::Image::BitmapData source(master,juce::Image::BitmapData::readOnly);for(int y=0;y<master.getHeight();++y)for(int x=0;x<master.getWidth();++x)if(coverage(source.getPixelColour(x,y))>.5f)ink=ink.getUnion({x,y,1,1});}
        if(ink.isEmpty())return;
        ink=ink.expanded(2).getIntersection(master.getBounds());
        juce::Image mask(juce::Image::ARGB,ink.getWidth(),ink.getHeight(),true,juce::SoftwareImageType{});
        {juce::Image::BitmapData source(master,juce::Image::BitmapData::readOnly),destination(mask,juce::Image::BitmapData::writeOnly);
            for(int y=0;y<mask.getHeight();++y)for(int x=0;x<mask.getWidth();++x)destination.setPixelColour(x,y,juce::Colours::white.withAlpha(coverage(source.getPixelColour(x+ink.getX(),y+ink.getY()))));}
        constexpr int w=1000,h=232; // 4x backing for the 250 x 58 logical badge.
        shadow=juce::Image(juce::Image::ARGB,w,h,true,juce::SoftwareImageType{});
        metal=juce::Image(juce::Image::ARGB,w,h,true,juce::SoftwareImageType{});
        light=juce::Image(juce::Image::ARGB,w,h,true,juce::SoftwareImageType{});
        const auto draw=[&](juce::Image& target,juce::FillType fill){juce::Graphics g(target);g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);g.setFillType(fill);g.drawImageWithin(mask,0,0,w,h,juce::RectanglePlacement::xLeft|juce::RectanglePlacement::yMid,true);};
        draw(shadow,juce::FillType(juce::Colour(0xbb08050c)));
        juce::ColourGradient foil(juce::Colour(0xffaea3c5),0,0,juce::Colour(0xff50405f),0,h,false);foil.addColour(.30,juce::Colour(0xffb6a5d0));foil.addColour(.53,juce::Colour(0xff655076));foil.addColour(.57,juce::Colour(0xffd4c5ea));foil.addColour(.72,juce::Colour(0xff8b72a8));draw(metal,juce::FillType(foil));
        juce::ColourGradient glow(juce::Colour(0xffbf7aff),0,0,juce::Colour(0xfff4ddff),w,0,false);glow.addColour(.74,juce::Colour(0xffedb7ff));draw(light,juce::FillType(glow));
    }
};
