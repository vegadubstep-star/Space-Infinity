#include "../Source/ExtendedEngine.h"
#include <cassert>
#include <iostream>
int main(){
    infinity::Envelope env;for(int i=0;i<48000;++i)env.next(false,48000,1,.2f,1,1);
    assert(env.level>.999f);for(int i=0;i<48001;++i)env.next(true,48000,1,.2f,1,1);assert(env.level==0);
    infinity::Wavetable table;assert(std::abs(table.sample(.25,0)-1)<.001f);
    table.count=2;table.frames[1].fill(0);assert(std::abs(table.sample(.25,.5f)-.5f)<.001f);
    infinity::Pattern pattern;pattern.mode=1;int ons=0,offs=0;std::array<int,8> notes{};
    auto emit=[&](bool on,int ch,int n,float v){assert(ch==2);if(on){notes[size_t(ons++)]=n;assert(v==.7f);}else ++offs;};
    pattern.input(true,2,60,.7f,emit);pattern.input(true,2,64,.7f,emit);pattern.input(true,2,67,.7f,emit);
    for(int i=0;i<400;++i)pattern.next(100,emit);
    assert(ons==4&&offs==4&&notes[0]==60&&notes[1]==64&&notes[2]==67&&notes[3]==60);
    pattern.input(false,2,60,0,emit);pattern.input(false,2,64,0,emit);pattern.input(false,2,67,0,emit);
    for(int i=0;i<1000;++i)pattern.next(100,emit);
    assert(ons==4&&pattern.activeNote==-1);
    infinity::Pattern seq;seq.mode=2;seq.length=2;seq.pitch[0]=7;seq.enabled[1]=false;int triggers=0;
    auto track=[&](bool on,int,int note,float){if(on){assert(note==67);++triggers;}};
    seq.input(true,1,60,1,track);for(int i=0;i<400;++i)seq.next(100,track);assert(triggers==2);
    std::array<float,4096> audio{};for(size_t i=0;i<audio.size();++i)audio[i]=float(std::sin(2*infinity::pi*i/64));
    infinity::Granulator gran;double energy=0;for(int i=0;i<10000;++i){const auto out=gran.next(audio.data(),0,4095,1,48000,.5f,90,30,.2f,.8f,false);for(auto v:out){assert(std::isfinite(v));energy+=v*v;}}
    assert(energy>1);std::cout<<"Envelope timing, table morphing, arp order/gates, sequencer rests and granular audio passed\n";
}
