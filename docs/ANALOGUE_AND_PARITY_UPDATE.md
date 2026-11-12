# SPACE INFINITY 0.4.2 — hard-trance badge and output light

Version 0.4.2 adds the approved sharper hard-trance badge and its output-reactive purple light, retaining the complete 0.4.0 controls update. [Light active](images/Logo-Active.png) / [Idle metal finish](images/Logo-Idle.png).

This update keeps the existing analogue filters and controls and adds more direct control over hardware-like variation. It also includes a [module-by-module Serum 2 audit](SERUM2_FEATURE_AUDIT.md). The audit finds real feature gaps; this release is not represented as a full Serum 2 replacement or an identical Xfer sound engine.

## Logo fit hotfix

The Windows alpha-mask path can use the parent texture of a cropped image, making only the top of the wordmark visible. The badge is now baked into complete, zero-origin software RGBA images before native drawing. Windows receives ordinary fitted images for the metal, shadow and reactive light; the approved artwork is unchanged. The backing is 4x resolution and layout checks cover 100%, 125%, 150%, 200%, 300% and 400% scale. The regression fixture has a large source offset and verifies that lower strokes remain visible.

## Changes

- **Output-reactive badge:** the new wordmark also acts as a purple level light. Illumination travels inside the letter shapes with final output level, then fades smoothly back to the idle metal finish. It follows post-output audio, not MIDI note activity.

- **Hard-trance identity:** the approved INFINITY badge uses cleaner, sharper forward-leaning lettering and a minimal orbital accent. The native renderer turns its monochrome master into a clean anti-aliased mask and applies the purple metal finish. The smaller fitted badge clears the pitch wheel at every supported window size.

- **Logo fit:** smaller INFINITY logo with more top/side clearance, checked at the supported window sizes.

- **ANALOG LAB:** ENGINES → ANALOG LAB contains separate VCO wander, slow thermal drift, filter/envelope/voice tolerances, pulse drift, VCA drive/asymmetry, circuit memory and AC coupling. The component switch bypasses the new variation/colour controls. Fresh non-Init presets use gentle settings; Init is neutral.
- **Analogue controls on MAIN:** Character, pre-filter drive and the analogue-voice switch sit beside the filter. LAB opens component settings. Pitch controls have moved out of ANALOGUE into a separate PITCH page; all original parameters remain available.
- **Portamento and pitch:** PORTAMENTO is now a main-page time knob. Overlapping notes glide in Mono and Legato; zero gives instant pitch. A vertical hardware-style pitch wheel on MAIN exposes the existing **Pitch Wheel** parameter (±24 semitones) for FL Studio automation and macro/LFO assignments. Click PITCH above it for pitch envelopes, incoming MIDI wheel range and A4 tuning.
- **Matched stereo filters:** high-pass and band-pass now apply their selected response to the stereo side too. Channel-wise warmth preserves panning. The matched-stereo switch is independent of the component-colour switch. Old saved states retain their legacy settings. Enable MATCHED STEREO FILTERS on older sounds before assessing the new pan controls.
- **Envelope controls:** amp and filter now have Hold plus independent attack/decay/release contours. MAIN now shows both envelope graphs above their knobs, updating live as times, sustain or curves change; drag their nodes and segments directly. CURVES opens the larger view; ENGINES also provides AMP ENV and FILTER ENV pages. Drag nodes to edit times/levels and drag segments to bend curves.
- **Pan and tuning:** separate oscillator pan on MAIN, sub pan in its main panel, noise pan in ANALOG LAB and A4 tuning in PITCH.
- **Clean base VCO option:** updated saw/pulse edge correction is independent of Character, with base sub saw/pulse support. Imported/morphed/warped waveforms do not gain full Serum-style band-limiting from this option.
- **FX rack order:** the fixed rack is displayed in actual audio order without changing saved stage IDs or the processing chain.

LFO shape presets now use fewer anchors (two for ramps, three for triangle, four for square and five for the curved sine-style shape). Small outlined handles show where to bend each segment. Grid and Draw controls live in Advanced; existing detailed curves remain editable.

FX OUTPUT GAIN now reads **MUTE** at its fully left position and smoothly silences the full stage, including dry audio. Other positions keep their existing dB values and automation range. Previously saved −24 dB output values now mean mute; raise them slightly if you intended quiet output. Meters remain after stage gain and clear their held clip indication once a muted stage is silent. The convolver opens with Size, Tone, Pre-delay, Decay and Dry/Wet, with all additional controls retained behind Advanced. Wet Level affects the convolver return only; OUTPUT GAIN affects the whole stage. The logo retains its smaller fit with high-quality image resampling.

Two visible LFOs, broad macro routing, WT POS, 3D frame views, the six original acid waveform families, wet EQs and the separate gate/sequencer from v0.3.0 remain included.

## Test it

Download the **v0.4.2 Windows installer ZIP** from its release page, extract it, close FL Studio, run Setup.exe and reopen a fresh synth instance. Keep older installers if you want to compare versions.

1. Compare the new component switch on/off on a factory preset. Turn up one control at a time. VCA drive can compress peaks; it is a tone control, not an automatic loudness-matching switch.
2. Check that old saved sounds recall and the original analogue/filter controls still work.
3. Try Hold on a short-decay sound, then drag the amp/filter envelope graphs and contours.
4. Pan each source and try high-pass/band-pass/ladder modes. Check delay/reverb tails separately when assessing stereo position.
5. Compare the clean-VCO option on a high saw/pulse note with Character low. It is a targeted correction, not an alias-free guarantee for every sound.
6. Play fresh and held notes for several minutes and report any remaining clicks, fade or excessive CPU use with the preset name.

The audit and complete [native control inventory](audit/NATIVE_PARAMETER_INVENTORY.json) are included with the installer ZIP. The implementation draws on the official [Serum 2 manual](https://xferrecords.com/manual/serum-2/docs) and the [Diva manual’s Trimmers section](https://u-he.com/downloads/manuals/plugins/diva/Diva-user-guide.pdf). Numerical sound equivalence and overall superiority are not established by those manuals.

## Native screenshots

[Main controls and envelope graphs](images/Rhythm-Main.png) / [Simple reverb and convolver](images/FX-Spaces.png) / [Pitch settings](images/Pitch-Settings.png) / [Analogue Lab](images/Analogue-Lab.png) / [Amp envelope](images/Analogue-Amp-Envelope.png) / [Filter envelope](images/Analogue-Filter-Envelope.png)

### FL Studio pitch risers

Move the main-page pitch wheel, then use **Tools → Last tweaked → Create automation clip** in FL Studio (or the wrapper parameter browser → Pitch Wheel → Create automation clip). Draw the rise in the clip; centre is zero, the maximum is +24 semitones. Double-click the wheel to centre it. The PITCH page’s MIDI Wheel Range controls incoming MIDI pitch-wheel messages; the automation wheel displays its pitch directly in semitones.
