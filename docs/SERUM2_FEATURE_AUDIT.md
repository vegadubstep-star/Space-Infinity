# Serum 2 manual audit — SPACE INFINITY 0.4.0

Audited against Xfer’s [official Serum 2 User Guide](https://xferrecords.com/manual/serum-2/docs), accessed 12 November 2026. Page numbers below refer to the printed guide. The supplementary analogue reference is u-he’s [Diva User Guide](https://u-he.com/downloads/manuals/plugins/diva/Diva-user-guide.pdf), especially Trimmers and Transient Mode, pp. 47–48.

**Result: SPACE INFINITY is not yet a complete like-for-like replacement for Serum 2.** Common controls and several workflows are implemented, and this update closes specific gaps. There are substantial architectural and editing features still missing. The manual does not disclose Xfer’s DSP formulas, so matching control names or ranges cannot establish identical sound, alias rejection, compression, filter response or CPU performance. “Better” below is restricted to concrete additional controls; no overall sound-quality superiority is claimed.

The [native parameter inventory](audit/NATIVE_PARAMETER_INVENTORY.json) lists all 1,068 exposed processor automation parameters, its ID, type, choices, numeric range, host default and fresh Init value. This is a complete inventory of our exposed controls, rather than an assertion that every Serum automation parameter has a counterpart. Host defaults and Init values can differ because missing fields in older project states must retain legacy behaviour.

## Module-by-module comparison

| Area / manual reference | SPACE INFINITY result | Status |
|---|---|---|
| Basic UI, keyboard, tooltips, resizing and undo, pp. 23–27 | Tooltips, precise text entry, fine dragging, reset gestures and resizable native pages. No equivalent on-screen piano keyboard or universal per-control undo/redo stack. The logo now has a smaller padded drawing area and high-quality resampling. | Partial |
| Three main oscillators with selectable synthesis engines, pp. 28–36, 58–122 | Two wavetable oscillators, a shared sample/granular source mode, separate sub and noise. No third independently selectable main oscillator or per-oscillator engine switching. | Architectural gap |
| Octave, semitone, fine tune, level and pan, pp. 29–30, 57 | All are available. This update adds independent main oscillator pan knobs and sub/noise pan. Concert A4 tuning is adjustable. Control coverage does not establish matching gain/pan curves. | Common controls covered |
| WT POS and 2D/3D waveform views, pp. 39–45 | Independent position knobs, perspective frame stacks and interpolated playback. Built-in families have 17 frames. Custom tables support 64 frames of 512 samples, versus Serum’s 256 frames of 2048 samples (p. 38). | Partial; lower table capacity |
| Wavetable audio quality, p. 37 | Fixed 2× oversampling; updated saw/pulse VCO and sub edge correction is available independently of Character. Not Serum’s complete band-limited wavetable engine; spectral/warped tables still need broader alias measurements. | Partial; independent implementation |
| Phase, Rand and phase memory, pp. 41–44 | Start phase, random phase and phase reset/free-running behaviour. No Serum-style phase-memory menu or phase-legato options. | Partial |
| Unison, detune, blend, width, range, modes and stacks, pp. 45–48 | 1–16 voices, documented five mode names, octave/fifth/central sub-octave stacks and WT/warp spreads. Updated Blend is equal-level at 75%; ignored for one/two voices. Detune is ignored for one voice. The numerical distributions and compensation are independent implementations. | Documented controls covered; DSP identity unverified |
| Two simultaneous warp stages and warp types, pp. 49–56 | One stage per oscillator: off, cross-FM, mirror, PWM and sub-FM. Not the full warp list or two-stage arrangement. | Partial |
| Multisample instruments, pp. 58–72 | Factory single-sample instruments and imported WAV transposition. No equivalent key/velocity-zone multisample instrument engine. | Partial |
| Sample engine, pp. 73–84 | Import, trim, reverse, root, looping, sample waveform and live output views. No complete Serum sample-engine modes, markers or sample-management workflow. | Partial |
| Granular engine, pp. 85–103 | Up to 32 windowed grains per voice, position/size/density/spray/spread. Shared imported source and fewer controls/modes. | Partial |
| Spectral engine, pp. 104–122 | No equivalent spectral oscillator/resynthesis engine. | Missing |
| Sub oscillator, pp. 123–127 | Six waveforms, octave, pan, level and filter routing. Filter bypass is supported; it does not bypass the entire FX chain like a dedicated direct-out route. | Common controls covered; routing gap |
| Noise oscillator, pp. 128–132 | Procedural noise choices, level, pan and filter routing. No full sampled-noise browser, phase/random/one-shot/keytracking feature set. | Partial |
| Two filter modules and filter selection, pp. 133–143 | One voice-filter chain with six existing modes retained: Classic LP, Resonant LP, High-pass, Band-pass, 24 dB LP and Analogue ZDF. No second independently routed voice filter or full Serum filter catalogue. | Architectural gap; original analogue filters retained |
| A/B/C/noise/sub filter routing, p. 134 | A/B/sub/noise switches and bypass paths. No C because there is no third oscillator. | Covered for existing sources |
| Stereo filter response, pp. 141–143 | New matched stereo processing applies the selected response to both stereo components, including warmth. Previously non-ladder side signals were always low-pass. Old states retain the legacy mode; fresh presets use the corrected mode. | Specific defect corrected and tested |
| Main/Bus 1/Bus 2 mixer and sends, pp. 144–150 | One fixed FX chain with filter bypass paths. No equivalent three-rack mixer, sends or parallel bus routing. | Missing architecture |
| FX order, rack presets and editing, pp. 151–160 | Fixed chain. Rack is displayed in actual processing order: Gate → Compressor → Chorus → Flanger → Phaser → Delay → Reverb → Convolver, then master EQ. No drag reordering or separate rack presets. | Partial; display-order mismatch corrected |
| Bode frequency shifter, p. 161 | No corresponding effect. | Missing |
| Chorus / flanger / phaser, pp. 162–177 | Independent implementations with their main timing/depth/feedback/mix controls; smoother stereo flanger option and per-stage output gain/meters. Not all Xfer algorithms and advanced controls. | Partial |
| Compressor, pp. 162–163 | Single/multiband processing, attack/release/ratio/threshold/mix and band controls. No measured Xfer transfer-function match. | Common controls; DSP unverified |
| Convolver, pp. 164–166 | WAV import, factory IRs, size/tone/predelay/envelope/damping/reverse/minimum-phase, wet/output controls. Five primary controls open by default; Advanced retains detailed settings. Imported IR duration/size is limited. No claim of identical Xfer convolution preprocessing. | Broad control overlap; independent implementation |
| Delay, pp. 165–166 | Stereo times, tempo sync, offsets, feedback, stereo modes and filter controls, plus dedicated three-band wet EQ and output gain. No full rack/bus equivalence. | Broad overlap plus wet EQ |
| Distortion, pp. 167–168 | Pre-filter drive, existing saturation and optional new VCA colour. No dedicated equivalent of the full Serum distortion module/type list. | Partial |
| EQ, pp. 169–170 | Seven-band master EQ with curve/slope choices; independent three-band wet EQs for delay and reverb. Serum’s documented module is two-band. Our extra bands are a concrete control-count advantage, not proof of better sound. | Additional bands and wet-only control |
| Hyper/Dimension, p. 173 | Chorus and unison width can provide related widening, but no equivalent Hyper/Dimension module exists. | Missing |
| L/H, L/M/H and M/S splitters, pp. 177–181 | Frequency-selective gating/ducking exists. No general split-FX group routing or corresponding mid/side splitter rack. | Partial / routing gap |
| Reverb, pp. 175–176 | Independent spatial/reverb controls and wet EQ. No measured match for Serum’s individual reverb algorithms. | Partial |
| Utility and FX filter modules, pp. 170–171, 182 | Some functions are spread across gain, pan, EQ and voice filters; no complete equivalent standalone utility/FX-filter modules. | Partial / missing modules |
| Four envelopes, pp. 183–187 | Two full amp/filter AHDSR envelopes plus a simpler pitch envelope. Portamento is exposed on MAIN alongside an automatable pitch wheel; Mono and Legato overlap glide are available. This update adds Hold, independent attack/decay/release contours and live editable graphs on MAIN. No fourth full envelope, per-envelope BPM units, full envelope drag routing or legato-invert options. | Partial; important controls added |
| Six LFOs and editor, pp. 188–205 | Three editable LFOs, two visible at once; separate legacy vibrato. Few-anchor shape presets and segment bend handles, with grid/drawing in Advanced; normal curves, timing, trigger/envelope, direction, phase, smoothing, grid and loopback controls. No Path/Chaos/S&H types, full multi-select tools, source-copy gestures or shape-file management. | Partial |
| Eight macros, pp. 206–208 | Six visible assignable macros; 16 generic destinations per macro and LFO, plus retained legacy assignments. Target rings, signed amounts, removal and recall work. No eight-macro layout or complete source system. | Partial |
| Modulation matrix, pp. 209–212 | Per-source route lists and signed depth editing. No full Serum matrix, polarity/auxiliary-source/curve/remapping system, all note-expression sources or audio-rate oscillator/filter sources. Generic destinations use shared control-rate updates; per-voice LFO triggering applies to retained voice routes, not universally to every new destination. Full polyphonic routing across all targets remains a gap. | Significant gap |
| MIDI clips / piano roll / MIDI out, pp. 213–243 | No clip bank, recording piano roll or host MIDI-output implementation. The internal sequencer generates notes for the synth only. | Missing |
| Arpeggiator and expression, pp. 244–271 | Internal arpeggiator and separate 16-step note/velocity/rest sequencer. No full Serum per-step expression and clip ecosystem. | Partial |
| Wavetable editor, pp. 272–311 | Draw/import/export/duplicate/delete and interpolated playback. No full spectral/FFT/formula/normalization/morph menu, independent full editors per oscillator or 256-frame capacity. | Partial |
| Global voice controls, quality and tuning, pp. 312–326 | 24 internal voices, mono/legato/glide, pitch bend, fixed oversampling, A4 tuning and separate component variation. No equivalent voice-sequence editor, broad quality configuration or microtuning import. | Partial |
| MIDI mapping, MPE, aftertouch and controllers, pp. 210–211, 327–343 | Notes, velocity and pitch bend work. No complete MPE/aftertouch/mod-wheel/controller-source or MIDI-learn system. | Significant gap |
| Preset browser, save/recall and asset handling, pp. 16–21 | Factory/user browsing, previous/next, chosen-folder Save As, embedded imported assets and legacy recall. No Serum preset-format compatibility, full tagging/search/ecosystem or universal per-control undo history. | Common workflows covered; ecosystem gaps |
| Dedicated gate / frequency-selective ducking | Separate LFOTool-inspired gate, 15 patterns, editable curves, frequency splitting and a separate sequencer. This is an additional integrated workflow, not an assertion that Serum lacks equivalent sounds via modulation. | Independent additional workflow |
| Analogue component controls | Existing Character, free-running voices, capacitor envelopes, input drive, per-voice calibration and nonlinear ladder retained. New independent drift/tolerance/pulse/VCA-memory/coupling controls. Serum itself also has per-voice randomization (p. 318). | Expanded control; not a unique concept or measured hardware clone |

## What this audit changed

1. Preserved every existing filter mode and analogue control; new state fields default to legacy/bypass when loading older states.
2. Corrected the non-ladder stereo response and channel-wise warmth so panned sources do not leak into the opposite channel through an unintended side low-pass response.
3. Added amp/filter Hold, separate attack/decay/release contours and editable envelope graphs. The main page shows both live editable amp/filter graphs and two visible LFOs.
4. Added independent oscillator/sub/noise pan and concert A4 tuning.
5. Added optional component variation, slow common tuning drift, pulse drift, VCA asymmetry/drive, short circuit memory and AC coupling. No deliberate background hiss is generated.
6. Separated updated base saw/pulse edge correction from the Character control, including the sub’s base saw/pulse. This is not a complete anti-aliasing solution for all imported, morphed or warped waveforms.
7. Displayed FX in processing order without changing that processing order or the saved stage IDs.

## Evidence and limits

In the high-saw fixture (1757.8125 Hz note, 48 kHz output, 2× processing, Character off), the non-harmonic spectral-energy fraction fell from 0.010046 to 0.000125644 with clean base VCO enabled—about a 98.7% reduction in that fixture. This is a comparison against our previous base saw path, not against Serum, and is not an alias-free guarantee.

The pad-tail probe shows that component variation changes the energy entering the reverb, while the decay ratio remains similar (new controls on: 87.4406 early / 0.235680 late / 0.0236793 later; bypassed: 49.6982 / 0.136973 / 0.0135258). The regression check therefore measures decay relative to release power and also requires a further drop and a quiet absolute endpoint; it does not silence or shorten the reverb just to match the previous absolute-energy threshold.

Native checks cover envelope Hold/contour endpoints at 44.1/48/96 kHz; exact zero-colour and master bypass; colour stability, decay and silence; actual audio changes; right-pan isolation through all six filter modes for oscillator A, B, sub and noise with Character enabled; interactive graph edits; route availability; new/legacy state recall; and oscillator alias measurements. The existing factory-bank, long-note, repeated-note, FX, preset, gate and UI suites remain required.

The catalogue above is a manual/source audit, not a listening comparison against a licensed Serum 2 instance. Hardware-specific transfer curves, component tolerances and thermal behaviour have not been measured from a particular synthesizer. Circuit memory is an independent bounded colour model, not a scientifically fitted magnetic-hysteresis model. Full parity would require the missing architecture and editors above, followed by reference renders and host/MPE/performance validation. Those gaps remain visible rather than being relabelled as matches.

The supplied release targets Windows x64 VST3 and Standalone. No macOS/AU/AAX release is supplied. The next parity work is architectural: universal per-voice modulation/MPE, additional independent oscillators/filters/buses, more envelope/LFO sources and engines, and fuller editors/FX/clip workflows.
