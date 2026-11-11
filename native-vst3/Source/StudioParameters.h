#pragma once
#include <array>
struct StudioControl {const char* id;const char* label;float low,high,value;const char* unit;};
inline constexpr std::array<StudioControl,29> studioControls{{
    {"compGain","GAIN",-24,36,0,"dB"},{"compMix","MIX",0,1,1,"%"},
    {"compXLow","X-LOW",20,2000,180,"Hz"},{"compXHigh","X-HIGH",400,16000,2600,"Hz"},
    {"compBelow","BELOW",.25f,4,1,":1"},{"compLowGain","LOW",-24,24,0,"dB"},
    {"compMidGain","MID",-24,24,0,"dB"},{"compHighGain","HIGH",-24,24,0,"dB"},
    {"compOutput","LEVEL",-24,12,0,"dB"},
    {"compLowOffset","Low threshold offset",-24,24,0,"dB"},
    {"compMidOffset","Mid threshold offset",-24,24,0,"dB"},
    {"compHighOffset","High threshold offset",-24,24,0,"dB"},
    {"echoTimeL","TIME L",.001f,4,.375f,"s"},{"echoTimeR","TIME R",.001f,4,.375f,"s"},
    {"echoOffsetL","OFFSET L",.25f,2,1,"x"},{"echoOffsetR","OFFSET R",.25f,2,1,"x"},
    {"echoFrequency","FREQ",20,16000,5700,"Hz"},{"echoBandwidth","Q / WIDTH",0,1,.8f,"%"},
    {"echoMix","MIX",0,1,.25f,"%"},{"echoOutput","LEVEL",-24,12,0,"dB"},
    {"irSize","SIZE",.25f,2,1,"x"},{"irTone","TONE",100,20000,20000,"Hz"},
    {"irPredelay","PRE-DELAY",0,500,0,"ms"},{"irAttack","ATTACK",0,1000,0,"ms"},
    {"irDecay","DECAY",.05f,8,8,"s"},{"irDamp","DAMP",0,1,0,"%"},
    {"irGain","IR GAIN",-24,24,0,"dB"},{"irOutput","LEVEL",-24,12,0,"dB"},
    {"scopeWindow","Scope window",.005f,.15f,.03f,"s"}
}};
