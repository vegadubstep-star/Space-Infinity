# Skywave Pad

First replacement factory sound, programmed from the owner's Rank 1 Airwave-inspired recipe. It is an independent approximation for INFINITY, not a claim to reproduce the original recording or Serum's engine exactly.

- A: eight saw voices; B: seven saw voices, +1 octave and +4 cents. Moderate shared detune, random/free-running starting phases, wide stereo, B at 32% of the oscillator mix.
- Bright noise at 0.8% adds air. This patch does not use an extra granular oscillator.
- Amp: 0.96-second attack, full sustain, 2.52-second release. **LONG x3** multiplies the existing attack/release controls; their displayed seconds reflect that multiplier. Defaults and older saved states retain the original range.
- LFO 1: four-bar triangle, subtle Osc A width movement. LFO 2: eight-bar triangle, low-pass cutoff rising gently from 2.6 kHz. Different periods still repeat after eight bars; this is not random/non-repeating modulation. This patch retains width movement; the later custom-table scan destination is available for other patches.
- Chorus at 32%, wide long hall reverb, subtle phaser. EQ: 200 Hz high-pass, 3.9 kHz low-pass. Compressor, gate, pumping and delay are disabled.

Each LFO has an **LFO ON** switch for the selected tab. Turning it off removes its voice and effects modulation, including the direct LFO 1 motion-to-cutoff path, while retaining the curve/routes. **VIBRATO** separately bypasses the older sine LFO's pitch/filter movement. Both are saved and automatable. They do not disable chorus or phaser's internal modulation.

The bank now contains Init, Skywave Pad, Aurelia Lead, Azure Horizon Lead, Prism Drive Lead, Orbital Expedition Lead, Ionwire Gate Lead, Expanse Motion Pluck, Solo Afterglow Lead, Nebula Pulse Pad, Caustic Circuit Bass, Velvet Transit Chords, Drawbar Ember Bass and Copper Bounce Bass. Previous F0–F119 presets remain retired; Init is F120 and Skywave Pad is F121. Existing bank manifests gain the new sound without discarding user presets or restoring deliberately deleted sounds on every reload.

Automated checks cover roughly a minute of repeated chords, a held chord over multiple LFO cycles, the extended release, LFO bypass and saved-state recall. This establishes bounded output and guards against cumulative attenuation in this patch; it does not yet explain the owner's reported fading in the previous bank or replace FL Studio listening checks.

See [groove bank additions](GROOVE_BANK.md) for the rhythmic pad, acid, house, organ and bounce patches and their new controls.
