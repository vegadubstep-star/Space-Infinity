#pragma once
#include <JuceHeader.h>
#include <array>
#include <atomic>
namespace studio {
class Distortion {
public:
    // Existing indices are retained for saved 0.6.0 projects.
    static juce::StringArray names(){return {"Tube","Soft Clip","Hard Clip","Diode 1","Diode 2","Linear Fold","Sine Fold","Zero Square","Downsample","Asymmetric","Rectify","X-Shaper","X-Shaper Asym","Sine Shaper","Transistor","Tube Stack","Hard Drive"};}
    struct Settings {
        int type=0,filterPosition=0;
        float drive=.2f,mix=1,frequency=6000,q=.7071f,filterMorph=0,inputDb=0,tone=20000;
        bool hq=false;
        std::array<float,9> a{-1,-.75f,-.5f,-.25f,0,.25f,.5f,.75f,1},b{-1,-1,-.9f,-.6f,0,.6f,.9f,1,1};
    };
    void prepare(double rate){
        sr=rate;memory={};toneMemory={};lastIn={};lastOut={};wavePrevious={};held={};counter=0;mix=0;drive=.2f;inputGain=1;toneBlend=0;hqBlend=0;configured=false;
        dc=float(std::exp(-2*juce::MathConstants<double>::pi*12/sr));slew=float(1-std::exp(-1/(sr*.008)));slow=float(1-std::exp(-1/(sr*.02)));meter.store(0);blockPeak=0;
    }
    void configure(const Settings& next){
        s=next;s.type=juce::jlimit(0,16,s.type);s.q=juce::jlimit(.2f,10.0f,s.q);
        targetInput=juce::Decibels::decibelsToGain(juce::jlimit(-24.0f,36.0f,s.inputDb));
        targetG=std::tan(juce::MathConstants<float>::pi*juce::jlimit(20.0f,float(sr*.43),s.frequency)/float(sr));targetK=1/s.q;
        targetTone=std::tan(juce::MathConstants<float>::pi*juce::jlimit(500.0f,float(sr*.4),s.tone)/float(sr));
        if(!configured){g1=targetG;k=targetK;toneG=targetTone;morph=s.filterMorph;toneBlend=s.tone<19999?1.0f:0.0f;hqBlend=s.hq?1.0f:0.0f;if(s.hq||s.inputDb!=0||s.tone<19999){drive=s.drive;mix=s.mix;inputGain=targetInput;}configured=true;}
        if(!s.hq){g1=targetG;k=targetK;morph=s.filterMorph;}
    }
    static float shape(float x,int type,float amount,const Settings& settings){
        const float gain=std::exp2(amount*6),v=x*gain;
        const auto table=[&](const std::array<float,9>& values,float at){const float pos=juce::jlimit(0.0f,8.0f,(at+1)*4);const int i=juce::jmin(7,int(pos));return values[size_t(i)]+(values[size_t(i+1)]-values[size_t(i)])*(pos-i);};
        switch(type){
            case 0:return (std::tanh(v+.12f)-std::tanh(.12f))/.99f;
            case 1:return v/(1+std::abs(v));
            case 2:return juce::jlimit(-1.0f,1.0f,v);
            case 3:return std::atan(v*1.5f)/1.5707963f;
            case 4:return juce::jlimit(-.8f,.8f,std::sin(v*.8f));
            case 5:{const float t=std::fmod(v+1,4.0f);return 1-std::abs((t<0?t+4:t)-2);}
            case 6:return std::sin(v);
            case 7:return std::abs(v)>.06f?(v>0?1.0f:-1.0f):v/.06f;
            case 8:return x;
            case 9:return v>=0?std::tanh(v):.62f*std::tanh(v*1.6f);
            case 10:return std::abs(std::tanh(v));
            case 11:case 12:{const float at=type==11?std::abs(x):x;const float a=table(settings.a,at),b=table(settings.b,at);return (a+(b-a)*amount)*(type==11?(x>=0?1:-1):1);}
            case 13:return std::sin(v*(1+amount*3));
            case 14:return std::tanh(v+.22f*v*v/(1+std::abs(v)));
            case 15:return std::tanh(1.7f*(std::tanh(v*.7f+.18f)-std::tanh(.18f)));
            default:return juce::jlimit(-.72f,.72f,v+.20f*std::sin(2*v))/.72f;
        }
    }
    void beginBlock(){blockPeak=0;}
    void endBlock(){meter.store(blockPeak,std::memory_order_relaxed);}
    float drivenInputPeak()const{return meter.load(std::memory_order_relaxed);}
    std::array<float,2> process(float left,float right){
        drive+=slew*(s.drive-drive);mix+=slew*(s.mix-mix);inputGain+=slow*(targetInput-inputGain);toneG+=slow*(targetTone-toneG);toneBlend+=slow*((s.tone<19999?1.0f:0.0f)-toneBlend);hqBlend+=slew*((s.hq?1.0f:0.0f)-hqBlend);
        if(s.hq){g1+=slow*(targetG-g1);k+=slow*(targetK-k);morph+=slow*(s.filterMorph-morph);}
        a1=1/(1+g1*(g1+k));
        const int period=juce::jmax(1,int(1+drive*drive*95));const bool sample=counter++%period==0;
        std::array<float,2> out{left,right};
        for(int ch=0;ch<2;++ch){
            const float dry=out[size_t(ch)];float wet=dry*inputGain;if(s.filterPosition==1)wet=filter(wet,ch);
            blockPeak=std::max(blockPeak,std::abs(wet)*(s.type==11||s.type==12||s.type==8?1:std::exp2(drive*6)));
            if(s.type==8){if(sample)held[size_t(ch)]=wet;wet=held[size_t(ch)];}
            else {const float input=wet;wet=shape(input,s.type,drive,s);if(hqBlend>.000001f)wet+=hqBlend*(integratedShape(input,wavePrevious[size_t(ch)])-wet);wavePrevious[size_t(ch)]=input;}
            if(s.filterPosition==2)wet=filter(wet,ch);
            float clean=wet-lastIn[size_t(ch)]+dc*lastOut[size_t(ch)];lastIn[size_t(ch)]=wet;lastOut[size_t(ch)]=clean;
            const float softened=lowpass(clean,toneG,1.41421356f,toneMemory[size_t(ch)]);clean+=toneBlend*(softened-clean);
            out[size_t(ch)]=dry+mix*(clean-dry);
        }
        return out;
    }
private:
    static double logCosh(double x){const double a=std::abs(x);return a+std::log1p(std::exp(-2*a))-.6931471805599453;}
    static double hardPrimitive(double x,double gain){const double a=std::abs(x);return a*gain<1?.5*gain*x*x:a-.5/gain;}
    static double tablePrimitive(const std::array<float,9>& curve,double x){
        if(x<=-1)return (x+1)*curve[0];double area=0;
        for(int i=0;i<8;++i){const double start=-1+i*.25,length=juce::jlimit(0.0,.25,x-start);area+=curve[size_t(i)]*length+(curve[size_t(i+1)]-curve[size_t(i)])*length*length*2;if(x<=start+.25)return area;}
        return area+(x-1)*curve[8];
    }
    double primitive(double x)const{
        const double gain=std::exp2(double(drive)*6),v=x*gain;
        switch(s.type){
            case 0:return (logCosh(v+.12)/gain-std::tanh(.12)*x)/.99;
            case 1:return (std::abs(v)-std::log1p(std::abs(v)))/gain;
            case 2:return hardPrimitive(x,gain);
            case 3:{const double a=gain*1.5;return (x*std::atan(a*x)-std::log1p(a*a*x*x)/(2*a))/1.5707963;}
            case 5:{double t=std::fmod(v+1,4.0);if(t<0)t+=4;return (t<=2?.5*t*t-t:3*t-.5*t*t-4)/gain;}
            case 6:return -std::cos(v)/gain;
            case 7:return hardPrimitive(x,gain/.06);
            case 9:return x>=0?logCosh(v)/gain:.62*logCosh(v*1.6)/(gain*1.6);
            case 10:return (x>=0?1:-1)*logCosh(v)/gain;
            case 11:case 12:{const double at=s.type==11?std::abs(x):x;const auto area=[&](const auto& c){return tablePrimitive(c,at)-(s.type==11?tablePrimitive(c,0):0);};const double a=area(s.a),b=area(s.b);return a+(b-a)*drive;}
            case 13:{const double a=gain*(1+double(drive)*3);return -std::cos(a*x)/a;}
            default:return 0;
        }
    }
    float integratedShape(float x,float previous)const{
        const double dx=double(x)-previous;
        if(std::abs(dx)<1e-5*(1+std::max(std::abs(x),std::abs(previous))))return shape((x+previous)*.5f,s.type,drive,s);
        if(s.type!=4&&s.type<14)return float((primitive(x)-primitive(previous))/dx);
        // Four-point integration for the cascaded/non-analytic transfer curves.
        constexpr double nodes[]{.0694318442029737,.330009478207572,.669990521792428,.930568155797026};
        constexpr double weights[]{.173927422568727,.326072577431273,.326072577431273,.173927422568727};
        double result=0;for(int i=0;i<4;++i)result+=weights[i]*shape(float(previous+nodes[i]*dx),s.type,drive,s);return float(result);
    }
    static float lowpass(float x,float g,float damping,std::array<float,2>& m){const float a=1/(1+g*(g+damping)),v1=a*m[0]+g*a*(x-m[1]),v2=m[1]+g*v1;m[0]=2*v1-m[0];m[1]=2*v2-m[1];return v2;}
    float filter(float x,int ch){auto& m=memory[size_t(ch)];const float v3=x-m[1],v1=a1*m[0]+g1*a1*v3,v2=m[1]+g1*v1;m[0]=2*v1-m[0];m[1]=2*v2-m[1];const float lp=v2,bp=k*v1,hp=x-bp-lp,t=juce::jlimit(0.0f,2.0f,morph*2);return t<1?lp+(bp-lp)*t:bp+(hp-bp)*(t-1);}
    Settings s;double sr=48000;std::array<std::array<float,2>,2> memory{},toneMemory{};std::array<float,2> lastIn{},lastOut{},wavePrevious{},held{};std::uint64_t counter=0;
    float drive=.2f,mix=0,inputGain=1,toneBlend=0,hqBlend=0,targetInput=1,dc=.999f,slew=.002f,slow=.001f,g1=0,k=1.4142f,a1=1,targetG=0,targetK=1.4142f,toneG=0,targetTone=0,morph=0,blockPeak=0;bool configured=false;std::atomic<float> meter{0};
};
}
