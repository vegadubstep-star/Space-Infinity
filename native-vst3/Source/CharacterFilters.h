#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <complex>

namespace infinity {
// TPT state-variable filters. Indices 0..6 belong to the original engine and
// remain untouched; new types start at 7 to preserve existing patch meanings.
struct CharacterFilter {
    static constexpr double pi=3.14159265358979323846;
    struct SVF {
        double s1=0,s2=0;
        std::array<double,3> next(double x,double g,double k) noexcept {
            const double bp=(s1+g*(x-s2))/(1+g*(g+k));
            const double lp=s2+g*bp;
            s1=2*bp-s1;s2=2*lp-s2;
            return {lp,bp,x-k*bp-lp};
        }
    };
    struct Settings {
        int mode=7;double cutoff=1000,resonance=.2,vowel=0,morph=0,sampleRate=96000;
    };
    // A, E, I, O, U: three broad vocal resonances, continuously interpolated.
    static constexpr double vowels[5][3]={{800,1150,2800},{400,1600,2700},{350,1700,2700},{450,800,2830},{325,700,2530}};
    static double damping(const Settings& s) noexcept {return 1.41421356237-1.34*std::clamp(s.resonance,0.0,.95);}
    static double gFor(double frequency,const Settings& s) noexcept {return std::tan(pi*std::clamp(frequency,10.0,s.sampleRate*.4)/s.sampleRate);}
    static double formantFrequency(const Settings& s,int band) noexcept {
        const double vowel=std::clamp(s.vowel,0.0,4.0);const int a=int(vowel),b=std::min(4,a+1);
        return (vowels[a][band]+(vowels[b][band]-vowels[a][band])*(vowel-a))*std::clamp(s.cutoff/1000.0,.125,8.0);
    }
    static double formantDamping(const Settings& s) noexcept {return 1/(2+10*std::clamp(s.resonance/.95,0.0,1.0));}
    static double combFeedback(const Settings& s) noexcept {return .05+.88*std::clamp(s.resonance/.95,0.0,1.0);}
    static double delay(const Settings& s) noexcept {return std::clamp(s.sampleRate/std::max(20.0,s.cutoff),2.0,8190.0);}
    std::array<SVF,3> stages{};
    std::array<float,8192> comb{};unsigned write=0;
    float process(float input,const Settings& s) noexcept {
        const double g=gFor(s.cutoff,s),k=damping(s);
        if(s.mode==11){
            const double fk=formantDamping(s);double out=0;constexpr double weight[]{1,.7,.35};
            for(int i=0;i<3;++i)out+=weight[i]*fk*stages[size_t(i)].next(input,gFor(formantFrequency(s,i),s),fk)[1];
            return float(out/1.35);
        }
        if(s.mode==12||s.mode==13){
            double read=double(write)-delay(s);if(read<0)read+=8192;
            const auto a=unsigned(read),b=(a+1)&8191u;
            const double wet=comb[a]+(comb[b]-comb[a])*(read-a),feedback=combFeedback(s);
            const float out=float((1-feedback)*input+(s.mode==12?feedback:-feedback)*wet);
            comb[write]=std::abs(out)<1.e-20f?0:out;write=(write+1)&8191u;return out;
        }
        const auto v=stages[0].next(input,g,k);
        switch(s.mode){
            case 7:return float(v[0]+v[2]);
            case 8:return float(v[0]+v[2]-k*v[1]);
            case 9:return float(stages[1].next(v[2],g,1.41421356237)[2]);
            case 10:return float(k*1.41421356237*stages[1].next(v[1],g,1.41421356237)[1]);
            case 14:{const double m=std::clamp(s.morph,0.0,1.0)*2;return float(m<1?v[0]*(1-m)+k*v[1]*m:k*v[1]*(2-m)+v[2]*(m-1));}
            default:return float(v[0]);
        }
    }
    // Exact small-signal response of the new linear models, shared with UI.
    static std::complex<double> response(double frequency,const Settings& s) {
        using C=std::complex<double>;const C j{0,1};
        const auto svf=[&](double centre,double k){const C z=j*std::tan(pi*frequency/s.sampleRate)/gFor(centre,s);const C den=1.0+k*z+z*z;return std::array<C,3>{1.0/den,z/den,z*z/den};};
        const double k=damping(s);const auto v=svf(s.cutoff,k);
        if(s.mode==11){C out=0;const double fk=formantDamping(s);constexpr double weight[]{1,.7,.35};for(int i=0;i<3;++i)out+=weight[i]*fk*svf(formantFrequency(s,i),fk)[1];return out/1.35;}
        if(s.mode==12||s.mode==13){const double d=delay(s),whole=std::floor(d),fraction=d-whole,fb=combFeedback(s);const C delayed=std::exp(-j*2.0*pi*frequency*whole/s.sampleRate)*((1-fraction)+fraction*std::exp(-j*2.0*pi*frequency/s.sampleRate));return (1-fb)/(1.0-(s.mode==12?fb:-fb)*delayed);}
        switch(s.mode){
            case 0:{const double a=1-std::exp(-2*pi*s.cutoff/s.sampleRate);return (1+.7*s.resonance)*a/(1.0-(1-a)*std::exp(-j*2.0*pi*frequency/s.sampleRate));}
            case 1:return v[0];case 2:return v[2];case 3:return v[1];case 4:return v[0]*svf(s.cutoff,1.41421356237)[0];
            case 5:case 6:return std::pow(v[0],2); // nonlinear circuit approximation, labelled in UI
            case 7:return v[0]+v[2];case 8:return v[0]+v[2]-k*v[1];
            case 9:return v[2]*svf(s.cutoff,1.41421356237)[2];case 10:return k*1.41421356237*v[1]*svf(s.cutoff,1.41421356237)[1];
            case 14:{const double m=std::clamp(s.morph,0.0,1.0)*2;return m<1?v[0]*(1-m)+k*v[1]*m:k*v[1]*(2-m)+v[2]*(m-1);}
            default:return v[0];
        }
    }
};
}
