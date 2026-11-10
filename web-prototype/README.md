# SPACE INFINITY v0.2 — interactive synthesis prototype

This is an original, playable **Web Audio** synthesizer with a realistic, hardware-inspired interface. It is NOT a Windows VST3 binary, does NOT load or emulate JV-880 firmware/ROMs, and is not yet a production plugin. It does not include Roland material.

## Run
Open `index.html` in Chrome or Edge. Click the piano keys or play A W S E D F T G Y H U J K on your keyboard. The first interaction starts the audio engine, as required by browser autoplay rules. Drag knobs vertically or scroll on them, switch oscillator shapes, browse presets, and save a JSON patch snapshot.

## Implemented
- Real polyphonic Web Audio oscillator engine (three oscillators per note), amp ADSR and Biquad low/high/band-pass filter
- Real chorus/delay/convolution-reverb audio paths and compression
- Six patch starting points and functional interactive knobs
- Live time-domain audio monitor, oscillator wave visualisers, AMP/FILTER/MOD envelope shapes, filter and LFO graphs
- On-screen keyboard, computer keyboard, simple arpeggiator, mute/panic
- Preset JSON export (import and VST3 state recall not yet implemented)

## Limitations / next development
- Authentic JV-880 sound would require a properly licensed JV-880 emulation engine and integration with your own legally obtained ROMs. No ROM content is redistributed here.
- This build is not a compiled VST3 and has not been tested in FL Studio.
- Current LFO and mod matrix displays are visual prototypes; actual parameter modulation/routing is not yet fully implemented. The sequence step buttons currently edit pattern visually only.
- Additional filter-envelope DSP, per-voice filtering, CPU handling, audio rendering optimization and production QA are required.