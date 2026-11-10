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
