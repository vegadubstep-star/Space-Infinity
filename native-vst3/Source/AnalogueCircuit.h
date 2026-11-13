#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

// Four trapezoidal one-pole stages with an implicitly solved nonlinear
// resonance loop. All state belongs to the voice; process() allocates nothing.
class AnalogueLadder {
public:
    float process(float input,float coefficient,float resonance,float drive) noexcept {
        const float g=std::clamp(coefficient,0.0f,0.9999f);
        const float feedback=4.0f*std::clamp(resonance,0.0f,0.95f);
        const float a=g*g*g*g;
        const float history=(1.0f-g)*(state[3]+g*(state[2]+g*(state[1]+g*state[0])));
        const float drivenInput=input*(1.0f+feedback*0.25f);
        const float inverseDrive=1.0f/std::max(0.25f,drive);
        float lower=-inverseDrive,upper=inverseDrive;
        float estimate=std::clamp(previousInput,lower,upper);
        for(int iteration=0;iteration<8;++iteration){
            const float saturated=std::tanh((drivenInput-feedback*(a*estimate+history))/inverseDrive);
            const float residual=estimate-saturated*inverseDrive;
            if(std::abs(residual)<0.0000005f)break;
            if(residual>0.0f)upper=estimate;else lower=estimate;
            const float derivative=1.0f+feedback*a*(1.0f-saturated*saturated);
            const float candidate=estimate-residual/derivative;
            estimate=candidate>lower&&candidate<upper?candidate:0.5f*(lower+upper);
        }
        previousInput=estimate;
        float output=estimate;
        for(auto& memory:state){
            const float stage=g*output+(1.0f-g)*memory;
            memory=2.0f*stage-memory;
            output=stage;
        }
        return output;
    }
    void reset() noexcept {state={};previousInput=0.0f;}
private:
    std::array<float,4> state{};
    float previousInput=0.0f;
};

// Acid-voiced four-stage TPT loop: a faster first pole and a third/fourth
// output tap expose the moving resonant peak. This is an authored response,
// not a component-exact TB-303 circuit. Internal headroom keeps the resonance
// from disappearing when a full-scale oscillator enters the nonlinear loop.
class AcidResonantFilter {
public:
    float process(float input,float g,float resonance,float drive) noexcept {
        const float first=2*g/(1+2*g),other=g/(1+g);
        const std::array<float,4> c{first,other,other,other};
        float a=1,history=0;
        for(size_t i=0;i<4;++i){a*=c[i];history=c[i]*history+(1-c[i])*state[i];}
        const float feedback=4.2f*std::pow(std::clamp(resonance/0.95f,0.0f,1.0f),1.25f);
        constexpr float headroom=.18f;
        const float source=input*headroom*(1+.5f*feedback);
        const float inverseDrive=1/(1+1.5f*std::clamp(drive,0.0f,1.0f));
        float lower=-inverseDrive,upper=inverseDrive,estimate=std::clamp(previousInput,lower,upper);
        for(int i=0;i<8;++i){
            const float t=std::tanh((source-feedback*(a*estimate+history))/inverseDrive);
            const float error=estimate-inverseDrive*t;
            if(std::abs(error)<1e-7f)break;
            if(error>0)upper=estimate;else lower=estimate;
            const float next=estimate-error/(1+feedback*a*(1-t*t));
            estimate=next>lower&&next<upper?next:(lower+upper)*.5f;
        }
        previousInput=estimate;float output=estimate,third=0;
        for(size_t i=0;i<4;++i){const float y=c[i]*output+(1-c[i])*state[i];state[i]=2*y-state[i];output=y;if(i==2)third=y;}
        return (.65f*output+.35f*third)/headroom;
    }
    void reset() noexcept {state={};previousInput=0;}
private:
    std::array<float,4> state{};float previousInput=0;
};

// Deterministic per-voice drift avoids a shared random generator on the audio thread.
class AnalogueDrift {
public:
    void reset(uint32_t seed) noexcept {
        rng=seed==0?1:seed;
        position={random(),random()};
        destination=position;
        remaining=0;
    }
    std::array<float,2> next(float slew,int interval) noexcept {
        if(--remaining<=0){
            destination={random(),random()};
            remaining=interval;
        }
        for(size_t i=0;i<position.size();++i)
            position[i]+=slew*(destination[i]-position[i]);
        return position;
    }
    float random() noexcept {
        rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;
        return float(rng>>8)*(2.0f/16777215.0f)-1.0f;
    }
private:
    uint32_t rng=1;
    std::array<float,2> position{},destination{};
    int remaining=0;
};

inline float analoguePolyBlep(double phase,double increment) noexcept {
    const float t=float(phase-std::floor(phase));
    const float dt=std::clamp(float(std::abs(increment)),0.0000001f,0.49f);
    if(t<dt){
        const float x=t/dt;
        return x+x-x*x-1.0f;
    }
    if(t>1.0f-dt){
        const float x=(t-1.0f)/dt;
        return x*x+x+x+1.0f;
    }
    return 0.0f;
}

inline void advanceTimedEnvelope(float& level,bool& decaying,float& releaseStart,bool released,
    float attackStep,float decayStep,float sustain,float releaseSamples) noexcept {
    if(released){
        if(releaseStart<0.0f)releaseStart=level;
        level=releaseSamples<=0.0f?0.0f:std::max(0.0f,level-releaseStart/std::max(1.0f,releaseSamples));
    }else if(!decaying){
        level=std::min(1.0f,level+attackStep);
        decaying=level>=1.0f;
    }else{
        const float step=std::max(0.0f,1.0f-sustain)*decayStep;
        level+=std::clamp(sustain-level,-step,step);
    }
}

// Finite-duration capacitor-style curves, with exact endpoints even at short times.
struct AnalogueEnvelope {
    double phase=0;float level=0,releaseLevel=0,decayLevel=1;int stage=0;
    static float charge(double t,float curve=0) noexcept {const double k=4+8*curve;t=std::clamp(t,0.0,1.0);return std::abs(k)<.00001?float(t):float(-std::expm1(-k*t)/(-std::expm1(-k)));}
    float next(bool released,double sr,float attack,float decay,float sustain,float release,float hold=0,float attackCurve=0,float decayCurve=0,float releaseCurve=0) noexcept {
        if(released&&stage!=3){stage=3;phase=0;releaseLevel=level;}
        if(stage==3){phase+=1/std::max(1.0,double(release)*sr);level=releaseLevel*(1-charge(phase,releaseCurve));if(phase>=1)level=0;}
        else if(stage==0){phase+=1/std::max(1.0,double(attack)*sr);level=charge(phase,attackCurve);if(phase>=1){level=1;stage=hold>0?4:1;phase=0;decayLevel=1;}}
        else if(stage==4){level=1;phase+=1/std::max(1.0,double(hold)*sr);if(phase>=1){stage=1;phase=0;}}
        else if(stage==1){phase+=1/std::max(1.0,double(decay)*sr);level=sustain+(decayLevel-sustain)*(1-charge(phase,decayCurve));if(phase>=1){stage=2;level=sustain;}}
        else level+=.01f*(sustain-level);
        return level;
    }
};
struct VoiceCalibration {
    float tuneA=0,tuneB=0,cutoff=0,timing=0,gain=0;
    static VoiceCalibration forVoice(int index) noexcept {
        AnalogueDrift random;random.reset(0x75a37du+uint32_t(index)*9719u);
        return {random.random(),random.random(),random.random(),random.random(),random.random()};
    }
};

// Independent, bounded VCA/colour model. A short feedback memory provides
// rate-dependent colour; it is not a measured recreation of a specific circuit.
struct AnalogueColourSettings {
    float gain=1,bias=0,centre=0,normalisation=1,memory=0,memorySlew=1,dcCoefficient=0,wet=0,shapeWet=1;
    bool active=false,distort=false;
    static AnalogueColourSettings make(float drive,float asymmetry,float memory,float coupling,double sr) noexcept {
        AnalogueColourSettings s;s.active=drive>0||asymmetry>0||memory>0||coupling>0;if(!s.active)return s;
        s.gain=1+9*drive;s.bias=.12f*asymmetry;s.centre=std::tanh(s.bias*s.gain);s.normalisation=1/std::max(.1f,s.gain*(1-s.centre*s.centre));
        s.distort=drive>0||asymmetry>0;s.memory=.15f*memory;s.memorySlew=float(1-std::exp(-1/(sr*(.0004+.002*memory))));
        s.dcCoefficient=float(std::exp(-6.283185307179586*(2+18*coupling)/sr));s.wet=std::max({drive,asymmetry,memory,coupling});return s;
    }
    static AnalogueColourSettings voltage(float drive,float asymmetry,float memory,float coupling,double sr,float range) noexcept {
        AnalogueColourSettings s;s.active=drive>0||asymmetry>0||memory>0||coupling>0;if(!s.active)return s;
        const float vintage=std::max(0.0f,std::min(1.0f,range-1));s.gain=1+(3+5*vintage)*drive;s.bias=(.04f+.06f*vintage)*asymmetry;s.centre=std::tanh(s.bias*s.gain);s.normalisation=1/std::tanh(s.gain);
        s.distort=drive>0||asymmetry>0;s.shapeWet=std::sqrt(std::max(drive,asymmetry));s.memory=(.08f+.10f*vintage)*memory;s.memorySlew=float(1-std::exp(-1/(sr*(.0006+.003*memory))));s.dcCoefficient=float(std::exp(-6.283185307179586*(2+(28+50*vintage)*coupling)/sr));s.wet=std::max({drive,asymmetry,memory,coupling});return s;
    }
};
struct AnalogueColour {
    std::array<float,2> memory{},input{},output{};
    float process(float x,const AnalogueColourSettings& s,int channel) noexcept {
        if(!s.active)return x;const auto ch=size_t(channel);memory[ch]+=s.memorySlew*(x+s.memory*memory[ch]-memory[ch]);
        const float fed=(x+s.memory*memory[ch])*(1-s.memory);
        float shaped=fed;if(s.distort){const float saturated=(std::tanh((fed+s.bias)*s.gain)-s.centre)*s.normalisation;shaped=s.shapeWet>=1?saturated:fed+s.shapeWet*(saturated-fed);}
        const float ac=shaped-input[ch]+s.dcCoefficient*output[ch];input[ch]=shaped;output[ch]=ac;
        return shaped+s.wet*(ac-shaped);
    }
};
inline std::array<float,3> analogueSvf(float input,float g,float damping,float* state) noexcept {
    const float band=(state[0]+g*(input-state[1]))/(1+g*(g+damping)),low=state[1]+g*band;
    state[0]=2*band-state[0];state[1]=2*low-state[1];return {low,input-damping*band-low,band};
}
