#pragma once
#include <array>
struct SampleSource {const char* name;const char* resource;int root;};
inline constexpr std::array<SampleSource,16> extraSampleSources{{
    {"Soprano Ah Choir","Choir_claudia_ah_wav",60},
    {"Soprano Ih Choir","Choir_claudia_ih_wav",60},
    {"Soprano Oh Choir","Choir_claudia_oh_wav",60},
    {"Tenor Ah Choir","Choir_mirko_ah_wav",48},
    {"Tenor Oh Choir","Choir_mirko_oh_wav",48},
    {"Bass Ah Choir","Choir_sven_ah_wav",49},
    {"Bass Ih Choir","Choir_sven_ih_wav",48},
    {"Bass Oh Choir","Choir_sven_oh_wav",48},
    {"Vibraphone","VCSL_Vibraphone_wav",84},
    {"Glockenspiel","VCSL_Glockenspiel_wav",84},
    {"Marimba","VCSL_Marimba_wav",72},
    {"Lute Pluck","VCSL_Lute_Pluck_wav",72},
    {"FM Clavi","VCSL_FM_Clavi_wav",72},
    {"FM Piano","VCSL_FM_Piano_wav",60},
    {"Bowed Psaltery","VCSL_Bowed_Psaltery_wav",84},
    {"Recorder Stab","VCSL_Recorder_Stab_wav",72},
}};
