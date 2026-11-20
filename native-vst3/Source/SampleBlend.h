#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
namespace infinity {
// Stereo, allocation-free overlap-add layer. Window-sum normalisation avoids
// density-driven gain pumping; even sparse settings retain overlapping grains.
struct SampleCloud {
    struct Grain {double position=0,age=0,length=1,step=1,rightOffset=0;bool active=false;};
    std::array<Grain,32> grains{};uint32_t rng=0x63bc91u;double until=0;float mix=0;
    double frozenPosition=.5;bool frozen=false;
    float random() noexcept {rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return float(rng&0xffffff)/16777216.f;}
    static float read(const float* data,int start,int end,double position) noexcept {
        const double length=std::max(1,end-start);double x=std::fmod(position-start,length);if(x<0)x+=length;x+=start;const int a=std::clamp(int(x),start,end-1);return data[a]+float(x-a)*(data[a+1]-data[a]);
    }
    static std::array<float,2> loop(const float* left,const float* right,int start,int end,double& position,double fade,bool reverse) noexcept {
        const double length=std::max(1,end-start);fade=std::clamp(fade,0.0,length*.45);
        const double low=reverse?start:start+fade,high=reverse?end-fade:end,span=std::max(1.0,high-low);
        if(position<start||position>end){double x=std::fmod(position-low,span);if(x<0)x+=span;position=low+x;}
        const auto at=[&](const float* data,double p){const int a=std::clamp(int(p),start,end-1);const float t=float(std::clamp(p-a,0.0,1.0));return data[a]+t*(data[a+1]-data[a]);};
        float l=at(left,position),r=at(right,position);
        if(fade>0&&((!reverse&&position>end-fade)||(reverse&&position<start+fade))){
            const double t=reverse?(start+fade-position)/fade:(position-end+fade)/fade;
            const double head=reverse?end-fade+position-start:start+position-end+fade;
            const float blend=float(.5-.5*std::cos(3.14159265358979323846*std::clamp(t,0.0,1.0)));
            l+=(at(left,head)-l)*blend;r+=(at(right,head)-r)*blend;
        }
        return {l,r};
    }
    std::array<float,2> next(const float* left,const float* right,int start,int end,double ratio,double sr,float centre,float sizeMs,float density,float spray,float spread,bool reverse) noexcept {
        const double length=std::max(1,end-start),duration=std::max(16.0,sizeMs*sr*.001);
        if(until<=0){for(auto& g:grains)if(!g.active){g={start+std::clamp(double(centre)+(random()-.5)*2*spray,0.0,1.0)*length,0,duration,(reverse?-1:1)*std::abs(ratio),(random()-.5)*2*sr*.003*spread*std::abs(ratio),true};break;}until+=std::clamp(sr/std::max(1.f,density),duration/24,duration*.28);}
        --until;double l=0,r=0,weight=0;
        for(auto& g:grains)if(g.active){const double w=.5-.5*std::cos(6.283185307179586*g.age/g.length);double leftPosition=g.position,rightPosition=g.position+g.rightOffset;const double fade=std::min((end-start)*.1,std::abs(ratio)*sr*.006);const auto a=loop(left,left,start,end,leftPosition,fade,reverse),b=loop(right,right,start,end,rightPosition,fade,reverse);l+=w*a[0];r+=w*b[0];weight+=w;g.position=leftPosition+g.step;if(++g.age>=g.length)g.active=false;}
        if(weight<1.e-8)return {};const float a=float(l/weight),b=float(r/weight),mid=(a+b)*.5f,side=(a-b)*.5f*std::clamp(spread*2.f,0.f,2.f);return {mid+side,mid-side};
    }
};
struct SamplePolish {
    std::array<std::array<float,1024>,2> delay{};unsigned write=0;std::array<float,2> tone{};float smear=0,dark=0;
    std::array<float,2> process(std::array<float,2> input,double sr,float smearTarget,float darkTarget) noexcept {
        const float smooth=float(1-std::exp(-1/(sr*.012)));smear+=smooth*(smearTarget-smear);dark+=smooth*(darkTarget-dark);
        const float coefficient=float(1-std::exp(-6.283185307179586*20000*std::exp2(-5*dark)/sr));
        for(int c=0;c<2;++c){const unsigned length=unsigned(std::clamp(sr*(c==0?.0031:.0047),2.0,1023.0)),read=(write+1024-length)&1023u;const float delayed=delay[size_t(c)][read],diffused=delayed-.55f*input[size_t(c)];delay[size_t(c)][write]=input[size_t(c)]+.55f*diffused;float x=input[size_t(c)]+smear*(diffused-input[size_t(c)]);tone[size_t(c)]+=coefficient*(x-tone[size_t(c)]);input[size_t(c)]=x+dark*(tone[size_t(c)]-x);}write=(write+1)&1023u;return input;
    }
};
}
