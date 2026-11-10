#pragma once
#include <array>
struct EffectControl {
    const char* id;
    const char* label;
    float minimum,maximum,initial;
    int row;
    const char* unit;
    bool added;
};
inline constexpr std::array<const char*,6> effectRows{"COMPRESSOR","CHORUS","FLANGER","PHASER","DELAY","REVERB"};
inline constexpr std::array<const char*,6> effectEnableIds{"compEnabled","chorusEnabled","flangerEnabled","phaserEnabled","delayEnabled","reverbEnabled"};
inline constexpr std::array<EffectControl,28> effectControls{{
    {"compAmount","Amount",0.0000f,1.0000f,0.0000f,0,"%",false},
    {"compAttack","Attack",0.1000f,100.0000f,9.0000f,0,"ms",true},
    {"compRelease","Release",10.0000f,1000.0000f,110.0000f,0,"ms",true},
    {"compThreshold","Threshold",-48.0000f,0.0000f,-10.0000f,0,"dB",true},
    {"compRatio","Ratio",1.0000f,20.0000f,8.0000f,0,":1",true},
    {"chorusRate","Rate",0.0100f,10.0000f,0.2500f,1,"Hz",true},
    {"chorusDepth","Depth",0.0000f,1.0000f,0.3800f,1,"%",true},
    {"chorusDelay","Delay",1.0000f,20.0000f,9.0000f,1,"ms",true},
    {"chorusFeedback","Feedback",-0.9000f,0.9000f,0.1000f,1,"%",true},
    {"chorus","Mix",0.0000f,1.0000f,0.2500f,1,"%",false},
    {"flangerRate","Rate",0.0100f,10.0000f,0.2300f,2,"Hz",true},
    {"flangerDepth","Depth",0.0000f,1.0000f,0.2800f,2,"%",true},
    {"flangerDelay","Delay",0.1000f,10.0000f,1.2000f,2,"ms",true},
    {"flangerFeedback","Feedback",-0.9500f,0.9500f,0.5800f,2,"%",true},
    {"flanger","Mix",0.0000f,1.0000f,0.0000f,2,"%",false},
    {"phaserRate","Rate",0.0100f,10.0000f,0.3200f,3,"Hz",true},
    {"phaserDepth","Depth",0.0000f,1.0000f,0.6600f,3,"%",true},
    {"phaserFrequency","Centre",40.0000f,8000.0000f,750.0000f,3,"Hz",true},
    {"phaserFeedback","Feedback",-0.9500f,0.9500f,0.4200f,3,"%",true},
    {"phaser","Mix",0.0000f,1.0000f,0.0000f,3,"%",false},
    {"delayTime","Time",0.0100f,2.0000f,0.3750f,4,"s",true},
    {"delayFeedback","Feedback",0.0000f,0.9500f,0.3500f,4,"%",true},
    {"delayTone","Tone",200.0000f,16000.0000f,5700.0000f,4,"Hz",true},
    {"delay","Mix",0.0000f,0.8000f,0.1700f,4,"%",false},
    {"reverbSize","Size",0.0000f,1.0000f,0.7200f,5,"%",true},
    {"reverbDamping","Damping",0.0000f,1.0000f,0.4500f,5,"%",true},
    {"reverbWidth","Width",0.0000f,1.0000f,1.0000f,5,"%",true},
    {"reverb","Mix",0.0000f,0.8000f,0.2500f,5,"%",false}
}};
