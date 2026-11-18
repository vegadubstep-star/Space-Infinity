#pragma once
#include <array>
#include <cmath>
namespace studio {
// Shared nonlinear oscillator bus: cross-products between the two signals
// add cohesion. Static compensation avoids any level-following gain fade.
inline std::array<float,2> oscillatorMelt(float mid,float side,float amount){
 if(amount<=0)return {mid,side};const float drive=1+2*amount,compensation=1+.15f*amount;
 const float l=std::tanh((mid+side)*drive)/drive*compensation,r=std::tanh((mid-side)*drive)/drive*compensation;
 return {mid+amount*((l+r)*.5f-mid),side+amount*((l-r)*.5f*(1-.12f*amount)-side)};
}
}
