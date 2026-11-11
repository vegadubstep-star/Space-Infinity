#pragma once
#include <array>
struct FactoryRefinement {
    float character;int voicesA,voicesB;float widthA,widthB,detune,filterAmount,filterDecay,filterSustain,lfoDepth;
    int shape,waveA,waveB,voiceMode;
};
inline constexpr std::array<FactoryRefinement,119> factoryRefinements{{
    {0.270f,7,5,0.480f,0.540f,0.00900f,0.350f,0.700f,0.650f,0.000f,1,-1,-1,0}, // HARD TRANCE LEAD: Wide anthem lead
    {0.200f,3,2,0.300f,0.380f,0.00700f,0.500f,0.320f,0.250f,0.000f,1,-1,-1,0}, // JV CHOIR INSPIRED: Textured original
    {0.220f,3,2,0.240f,0.320f,0.00600f,1.100f,0.230f,0.100f,0.000f,1,-1,-1,0}, // RAVE STAB 1996: Fast pluck / stab
    {0.405f,3,3,0.480f,0.580f,0.00880f,0.650f,2.500f,0.550f,0.094f,1,-1,10,0}, // ETHEREAL STRINGS: Evolving analogue pad
    {0.210f,1,1,0.080f,0.120f,0.00200f,1.600f,0.110f,0.100f,0.000f,1,-1,-1,1}, // ACID SIGNAL: Focused resonant / acid
    {0.355f,3,3,0.480f,0.580f,0.00760f,0.450f,1.900f,0.550f,0.078f,11,-1,10,0}, // INFINITY PAD: Evolving analogue pad
    {0.084f,1,1,0.000f,0.000f,0.00200f,0.250f,0.650f,0.650f,0.000f,1,-1,-1,0}, // ORBITAL VOX: Breathing sampled choir
    {0.096f,1,1,0.000f,0.000f,0.00200f,0.250f,0.650f,0.650f,0.000f,1,-1,-1,0}, // ETHEREAL CHOIR: Breathing sampled choir
    {0.330f,3,3,0.480f,0.580f,0.00700f,0.350f,1.600f,0.550f,0.070f,11,-1,10,0}, // NEON STRINGS: Evolving analogue pad
    {0.175f,1,1,0.000f,0.000f,0.00150f,1.380f,0.082f,0.120f,0.000f,1,-1,29,1}, // VOLTAGE BASS: Tight mono bass
    {0.320f,7,5,0.480f,0.540f,0.00900f,0.350f,0.700f,0.650f,0.000f,1,-1,-1,0}, // REDLINE ANTHEM: Wide anthem lead
    {0.220f,3,2,0.240f,0.320f,0.00600f,1.100f,0.255f,0.100f,0.000f,1,-1,-1,0}, // WAREHOUSE PULSE: Fast pluck / stab
    {0.270f,7,5,0.480f,0.540f,0.00900f,0.350f,0.700f,0.650f,0.000f,1,-1,-1,0}, // Apex Meridian: Wide anthem lead
    {0.295f,5,3,0.360f,0.440f,0.00860f,0.650f,0.400f,0.400f,0.000f,1,-1,-1,0}, // Solar Flare: Analogue lead
    {0.320f,5,3,0.360f,0.440f,0.00920f,0.650f,0.400f,0.400f,0.000f,1,-1,-1,0}, // Glacier Wire: Analogue lead
    {0.345f,5,3,0.360f,0.440f,0.00980f,0.650f,0.400f,0.400f,0.000f,1,-1,-1,0}, // Iron Cathedral: Analogue lead
    {0.270f,7,5,0.480f,0.540f,0.00900f,0.350f,0.700f,0.650f,0.000f,1,-1,-1,0}, // Blue Horizon: Wide anthem lead
    {0.225f,1,1,0.080f,0.120f,0.00200f,1.600f,0.130f,0.100f,0.000f,1,-1,-1,1}, // Riot Alarm: Focused resonant / acid
    {0.220f,3,2,0.240f,0.320f,0.00600f,1.100f,0.230f,0.100f,0.000f,1,-1,-1,0}, // Ivory Signal: Fast pluck / stab
    {0.255f,1,1,0.080f,0.120f,0.00200f,1.600f,0.170f,0.100f,0.000f,1,-1,-1,1}, // Red Mercury: Focused resonant / acid
    {0.270f,5,3,0.360f,0.440f,0.00800f,0.650f,0.400f,0.400f,0.000f,1,15,16,0}, // Orbit Brass: Analogue lead
    {0.295f,5,3,0.360f,0.440f,0.00860f,0.650f,0.400f,0.400f,0.000f,1,19,8,0}, // Chromed Glass: Analogue lead
    {0.320f,5,3,0.360f,0.440f,0.00920f,0.650f,0.400f,0.400f,0.000f,1,-1,-1,0}, // Pulse Beacon: Analogue lead
    {0.255f,1,1,0.080f,0.120f,0.00200f,1.600f,0.170f,0.100f,0.000f,1,-1,-1,1}, // Black Static: Focused resonant / acid
    {0.270f,7,5,0.480f,0.540f,0.00900f,0.350f,0.700f,0.650f,0.000f,1,-1,-1,0}, // Afterimage: Wide anthem lead
    {0.295f,5,3,0.360f,0.440f,0.00860f,0.650f,0.400f,0.400f,0.000f,1,-1,-1,0}, // Rave Telegraph: Analogue lead
    {0.240f,1,1,0.080f,0.120f,0.00200f,1.600f,0.150f,0.100f,0.000f,1,-1,-1,1}, // Acid Crown: Focused resonant / acid
    {0.345f,5,3,0.360f,0.440f,0.00980f,0.650f,0.400f,0.400f,0.000f,1,13,14,0}, // Eclipse Reed: Analogue lead
    {0.220f,3,2,0.240f,0.320f,0.00600f,1.100f,0.180f,0.100f,0.000f,1,-1,-1,0}, // Starlight Keys: Fast pluck / stab
    {0.295f,5,3,0.360f,0.440f,0.00860f,0.650f,0.400f,0.400f,0.000f,1,-1,-1,0}, // Pressure Valve: Analogue lead
    {0.320f,5,3,0.360f,0.440f,0.00920f,0.650f,0.400f,0.400f,0.000f,1,-1,-1,0}, // Twin Engines: Analogue lead
    {0.345f,5,3,0.360f,0.440f,0.00980f,0.650f,0.400f,0.400f,0.000f,1,-1,-1,0}, // Crimson Spiral: Analogue lead
    {0.270f,7,5,0.480f,0.540f,0.00900f,0.350f,0.700f,0.650f,0.000f,1,-1,-1,0}, // Zero Gravity: Wide anthem lead
    {0.295f,5,3,0.360f,0.440f,0.00860f,0.650f,0.400f,0.400f,0.000f,1,-1,-1,0}, // Burnt Copper: Analogue lead
    {0.320f,5,3,0.360f,0.440f,0.00920f,0.650f,0.400f,0.400f,0.000f,1,-1,-1,0}, // Hyper Relay: Analogue lead
    {0.345f,7,5,0.480f,0.540f,0.00900f,0.350f,0.700f,0.650f,0.000f,1,-1,-1,0}, // Dreamline: Wide anthem lead
    {0.210f,1,1,0.080f,0.120f,0.00200f,1.600f,0.110f,0.100f,0.000f,1,-1,-1,1}, // Circuit Bent: Focused resonant / acid
    {0.295f,5,3,0.360f,0.440f,0.00860f,0.650f,0.400f,0.400f,0.000f,1,15,16,0}, // Titan Horn: Analogue lead
    {0.220f,3,2,0.240f,0.320f,0.00600f,1.100f,0.230f,0.100f,0.000f,1,19,20,0}, // Prism Drops: Fast pluck / stab
    {0.345f,5,3,0.360f,0.440f,0.00980f,0.650f,0.400f,0.400f,0.000f,1,-1,-1,0}, // Rush Hour: Analogue lead
    {0.270f,5,3,0.360f,0.440f,0.00800f,0.650f,0.400f,0.400f,0.000f,1,-1,-1,0}, // Moon Quartz: Analogue lead
    {0.220f,3,2,0.240f,0.320f,0.00600f,1.100f,0.205f,0.100f,0.000f,1,-1,-1,0}, // Ion Stutter: Fast pluck / stab
    {0.320f,5,3,0.360f,0.440f,0.00920f,0.650f,0.400f,0.400f,0.000f,1,10,11,0}, // Phantom Vowel: Analogue lead
    {0.220f,3,2,0.240f,0.320f,0.00600f,1.100f,0.255f,0.100f,0.000f,1,-1,-1,0}, // Neon Needle: Fast pluck / stab
    {0.270f,5,3,0.360f,0.440f,0.00800f,0.650f,0.400f,0.400f,0.000f,1,-1,-1,0}, // Detonation: Analogue lead
    {0.220f,3,2,0.240f,0.320f,0.00600f,1.100f,0.205f,0.100f,0.000f,1,20,19,0}, // Aether Bell: Fast pluck / stab
    {0.320f,5,3,0.360f,0.440f,0.00920f,0.650f,0.400f,0.400f,0.000f,1,-1,-1,0}, // Night Mission: Analogue lead
    {0.345f,5,3,0.360f,0.440f,0.00980f,0.650f,0.400f,0.400f,0.000f,1,-1,-1,0}, // Saturn Sweep: Analogue lead
    {0.270f,5,3,0.360f,0.440f,0.00800f,0.650f,0.400f,0.400f,0.000f,1,12,19,0}, // Digital Orchid: Analogue lead
    {0.295f,5,3,0.360f,0.440f,0.00860f,0.650f,0.400f,0.400f,0.000f,1,-1,-1,0}, // Razor Octave: Analogue lead
    {0.320f,5,3,0.360f,0.440f,0.00920f,0.650f,0.400f,0.400f,0.000f,1,-1,-1,0}, // Final Approach: Analogue lead
    {0.345f,7,5,0.480f,0.540f,0.00900f,0.350f,0.700f,0.650f,0.000f,1,-1,-1,0}, // Orbital Uplink: Wide anthem lead
    {0.160f,1,1,0.000f,0.000f,0.00150f,1.250f,0.070f,0.120f,0.000f,1,-1,29,1}, // Kick Gap: Tight mono bass
    {0.175f,1,1,0.000f,0.000f,0.00150f,1.380f,0.082f,0.120f,0.000f,1,-1,29,1}, // Bounce Circuit: Tight mono bass
    {0.190f,1,1,0.000f,0.000f,0.00150f,1.510f,0.094f,0.120f,0.000f,1,-1,29,1}, // Rubber Driver: Tight mono bass
    {0.205f,1,1,0.000f,0.000f,0.00150f,1.640f,0.106f,0.120f,0.000f,1,-1,29,1}, // Sub Relay: Tight mono bass
    {0.160f,1,1,0.000f,0.000f,0.00150f,1.250f,0.070f,0.120f,0.000f,1,-1,29,1}, // Warehouse Offbeat: Tight mono bass
    {0.175f,1,1,0.000f,0.000f,0.00150f,1.380f,0.082f,0.120f,0.000f,1,-1,29,1}, // Triplet Teeth: Tight mono bass
    {0.190f,1,1,0.000f,0.000f,0.00150f,1.510f,0.094f,0.120f,0.000f,1,-1,29,1}, // Tight Machine: Tight mono bass
    {0.205f,1,1,0.000f,0.000f,0.00150f,1.640f,0.106f,0.120f,0.000f,1,-1,29,1}, // Low Conveyor: Tight mono bass
    {0.210f,1,1,0.080f,0.120f,0.00200f,1.600f,0.110f,0.100f,0.000f,1,-1,29,1}, // Acid Runner: Focused resonant / acid
    {0.175f,1,1,0.000f,0.000f,0.00150f,1.380f,0.082f,0.120f,0.000f,1,-1,29,1}, // Sixteenth Spine: Tight mono bass
    {0.084f,1,1,0.000f,0.000f,0.00200f,0.700f,0.230f,0.150f,0.000f,1,-1,-1,0}, // Cathedral Strike: Percussive choir shot
    {0.096f,1,1,0.000f,0.000f,0.00200f,0.700f,0.260f,0.150f,0.000f,1,-1,-1,0}, // Redline Kyrie: Percussive choir shot
    {0.060f,1,1,0.000f,0.000f,0.00200f,0.700f,0.170f,0.150f,0.000f,1,-1,-1,0}, // Vault Chant: Percussive choir shot
    {0.072f,1,1,0.000f,0.000f,0.00200f,0.250f,0.650f,0.650f,0.000f,1,-1,-1,0}, // Eclipse Choir: Breathing sampled choir
    {0.084f,1,1,0.000f,0.000f,0.00200f,0.250f,0.650f,0.650f,0.000f,1,-1,-1,0}, // Halo Ooh: Breathing sampled choir
    {0.096f,1,1,0.000f,0.000f,0.00200f,0.250f,0.650f,0.650f,0.000f,1,-1,-1,0}, // White Chapel: Breathing sampled choir
    {0.060f,1,1,0.000f,0.000f,0.00200f,0.250f,0.650f,0.650f,0.000f,1,-1,-1,0}, // Midnight Vespers: Breathing sampled choir
    {0.072f,1,1,0.000f,0.000f,0.00200f,0.700f,0.200f,0.150f,0.000f,1,-1,-1,0}, // Choral Signal: Percussive choir shot
    {0.084f,1,1,0.000f,0.000f,0.00200f,0.250f,0.650f,0.650f,0.000f,1,-1,-1,0}, // Reverse Procession: Breathing sampled choir
    {0.096f,1,1,0.000f,0.000f,0.00200f,0.250f,0.650f,0.650f,0.000f,1,-1,-1,0}, // Final Benediction: Breathing sampled choir
    {0.330f,3,3,0.480f,0.580f,0.00700f,0.350f,1.600f,0.550f,0.070f,1,-1,10,0}, // Black Orbit: Evolving analogue pad
    {0.355f,3,3,0.480f,0.580f,0.00760f,0.450f,1.900f,0.550f,0.078f,6,10,11,0}, // Subterranean Choir: Evolving analogue pad
    {0.380f,3,3,0.480f,0.580f,0.00820f,0.550f,2.200f,0.550f,0.086f,11,-1,10,0}, // Pressure Chamber: Evolving analogue pad
    {0.405f,3,3,0.480f,0.580f,0.00880f,0.650f,2.500f,0.550f,0.094f,1,-1,10,0}, // Iron Mist: Evolving analogue pad
    {0.330f,3,3,0.480f,0.580f,0.00700f,0.350f,1.600f,0.550f,0.070f,6,-1,10,0}, // Cold Reactor: Evolving analogue pad
    {0.355f,3,3,0.480f,0.580f,0.00760f,0.450f,1.900f,0.550f,0.078f,11,-1,10,0}, // Shadow Vector: Evolving analogue pad
    {0.380f,3,3,0.480f,0.580f,0.00820f,0.550f,2.200f,0.550f,0.086f,1,-1,10,0}, // Afterhours Static: Evolving analogue pad
    {0.405f,3,3,0.480f,0.580f,0.00880f,0.650f,2.500f,0.550f,0.094f,6,-1,10,0}, // Deep Warning: Evolving analogue pad
    {0.330f,3,3,0.480f,0.580f,0.00700f,0.350f,1.600f,0.550f,0.070f,11,-1,10,0}, // Obsidian Sweep: Evolving analogue pad
    {0.355f,3,3,0.480f,0.580f,0.00760f,0.450f,1.900f,0.550f,0.078f,1,-1,10,0}, // Night Transmission: Evolving analogue pad
    {0.380f,3,3,0.480f,0.580f,0.00820f,0.550f,2.200f,0.550f,0.086f,6,-1,6,0}, // Airborne Silk: Evolving analogue pad
    {0.405f,3,3,0.480f,0.580f,0.00880f,0.650f,2.500f,0.550f,0.094f,11,-1,6,0}, // Morning Horizon: Evolving analogue pad
    {0.330f,3,3,0.480f,0.580f,0.00700f,0.350f,1.600f,0.550f,0.070f,1,-1,6,0}, // Ivory Clouds: Evolving analogue pad
    {0.355f,3,3,0.480f,0.580f,0.00760f,0.450f,1.900f,0.550f,0.078f,6,-1,6,0}, // Blue Refraction: Evolving analogue pad
    {0.380f,3,3,0.480f,0.580f,0.00820f,0.550f,2.200f,0.550f,0.086f,11,-1,6,0}, // Summer Meridian: Evolving analogue pad
    {0.405f,3,3,0.480f,0.580f,0.00880f,0.650f,2.500f,0.550f,0.094f,1,-1,6,0}, // Ocean Glass: Evolving analogue pad
    {0.330f,3,3,0.480f,0.580f,0.00700f,0.350f,1.600f,0.550f,0.070f,6,-1,6,0}, // First Light Strings: Evolving analogue pad
    {0.355f,3,3,0.480f,0.580f,0.00760f,0.450f,1.900f,0.550f,0.078f,11,-1,6,0}, // Ethereal Lift: Evolving analogue pad
    {0.380f,3,3,0.480f,0.580f,0.00820f,0.550f,2.200f,0.550f,0.086f,1,-1,6,0}, // Silver Current: Evolving analogue pad
    {0.405f,3,3,0.480f,0.580f,0.00880f,0.650f,2.500f,0.550f,0.094f,6,-1,6,0}, // Open Skies: Evolving analogue pad
    {0.200f,3,2,0.300f,0.380f,0.00700f,0.500f,0.320f,0.250f,0.000f,1,-1,-1,0}, // Gate Communion: Textured original
    {0.200f,3,2,0.300f,0.380f,0.00700f,0.500f,0.320f,0.250f,0.000f,1,-1,-1,0}, // Strobelight Chord: Textured original
    {0.200f,3,2,0.300f,0.380f,0.00700f,0.500f,0.320f,0.250f,0.000f,1,17,18,0}, // Rave Chapel Organ: Textured original
    {0.200f,3,2,0.300f,0.380f,0.00700f,0.500f,0.320f,0.250f,0.000f,1,18,17,0}, // Warehouse Drawbars: Textured original
    {0.220f,3,2,0.240f,0.320f,0.00600f,1.100f,0.205f,0.100f,0.000f,1,20,2,0}, // Airlock Mallet: Fast pluck / stab
    {0.200f,3,2,0.300f,0.380f,0.00700f,0.500f,0.320f,0.250f,0.000f,1,-1,-1,0}, // Meteor Lift: Textured original
    {0.200f,3,2,0.300f,0.380f,0.00700f,0.500f,0.320f,0.250f,0.000f,1,-1,-1,0}, // Gravity Drop: Textured original
    {0.200f,3,2,0.300f,0.380f,0.00700f,0.500f,0.320f,0.250f,0.000f,1,-1,-1,0}, // Reentry Siren: Textured original
    {0.200f,3,2,0.300f,0.380f,0.00700f,0.500f,0.320f,0.250f,0.000f,1,-1,-1,0}, // Sub Impact: Textured original
    {0.380f,3,3,0.480f,0.580f,0.00820f,0.550f,2.200f,0.550f,0.086f,1,-1,10,0}, // Frozen Interlude: Evolving analogue pad
    {0.255f,1,1,0.080f,0.120f,0.00200f,1.600f,0.170f,0.100f,0.000f,1,-1,-1,1}, // Acid Ghost: Focused resonant / acid
    {.07f,1,1,0,0,.002f,.2f,.7f,.6f,0,1,-1,-1,0}, // Soprano Ah Ensemble
    {.07f,1,1,0,0,.002f,.2f,.7f,.6f,0,1,-1,-1,0}, // Soprano Ih Air
    {.07f,1,1,0,0,.002f,.2f,.7f,.6f,0,1,-1,-1,0}, // Soprano Oh Halo
    {.07f,1,1,0,0,.002f,.2f,.7f,.6f,0,1,-1,-1,0}, // Tenor Ah Cathedral
    {.07f,1,1,0,0,.002f,.2f,.7f,.6f,0,1,-1,-1,0}, // Tenor Oh Velvet
    {.07f,1,1,0,0,.002f,.2f,.7f,.6f,0,1,-1,-1,0}, // Bass Ah Chamber
    {.07f,1,1,0,0,.002f,.2f,.7f,.6f,0,1,-1,-1,0}, // Bass Ih Shadow
    {.07f,1,1,0,0,.002f,.2f,.7f,.6f,0,1,-1,-1,0}, // Bass Oh Ritual
    {.07f,1,1,0,0,.002f,.2f,.7f,.6f,0,1,-1,-1,0}, // Vibraphone Glow
    {.07f,1,1,0,0,.002f,.2f,.7f,.6f,0,1,-1,-1,0}, // Crystal Glock
    {.07f,1,1,0,0,.002f,.2f,.7f,.6f,0,1,-1,-1,0}, // Marimba Pulse
    {.07f,1,1,0,0,.002f,.2f,.7f,.6f,0,1,-1,-1,0}, // Lute Pluck Echo
    {.07f,1,1,0,0,.002f,.2f,.7f,.6f,0,1,-1,-1,0}, // FM Clavi Bounce
    {.07f,1,1,0,0,.002f,.2f,.7f,.6f,0,1,-1,-1,0}, // FM Piano Horizon
    {.07f,1,1,0,0,.002f,.2f,.7f,.6f,0,1,-1,-1,0}, // Bowed Psaltery Mist
    {.07f,1,1,0,0,.002f,.2f,.7f,.6f,0,1,-1,-1,0}, // Recorder Stab Rave
}};
