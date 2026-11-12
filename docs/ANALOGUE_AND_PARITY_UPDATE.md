# SPACE INFINITY 0.4.0 — analogue controls and manual audit

This update keeps the existing analogue filters and controls and adds more direct control over hardware-like variation. It also includes a [module-by-module Serum 2 audit](SERUM2_FEATURE_AUDIT.md). The audit finds real feature gaps; this release is not represented as a full Serum 2 replacement or an identical Xfer sound engine.

## Changes

- **Logo fit:** smaller INFINITY logo with more top/side clearance, checked at the supported window sizes.

- **ANALOG LAB:** ENGINES → ANALOG LAB contains separate VCO wander, slow thermal drift, filter/envelope/voice tolerances, pulse drift, VCA drive/asymmetry, circuit memory and AC coupling. The component switch bypasses the new variation/colour controls. Fresh non-Init presets use gentle settings; Init is neutral.
- **Original analogue section retained:** Character, analogue voices/curved envelopes, pre-filter drive, pitch envelope, velocity controls and all six existing filters remain available.
- **Matched stereo filters:** high-pass and band-pass now apply their selected response to the stereo side too. Channel-wise warmth preserves panning. The matched-stereo switch is independent of the component-colour switch. Old saved states retain their legacy settings. Enable MATCHED STEREO FILTERS on older sounds before assessing the new pan controls.
- **Envelope controls:** amp and filter now have Hold plus independent attack/decay/release contours. CURVES on MAIN opens both editable graphs; ENGINES also provides AMP ENV and FILTER ENV pages. Drag nodes to edit times/levels and drag segments to bend curves.
- **Pan and tuning:** separate oscillator pan on MAIN, sub pan in its main panel, noise pan and A4 tuning in ANALOG LAB.
- **Clean base VCO option:** updated saw/pulse edge correction is independent of Character, with base sub saw/pulse support. Imported/morphed/warped waveforms do not gain full Serum-style band-limiting from this option.
- **FX rack order:** the fixed rack is displayed in actual audio order without changing saved stage IDs or the processing chain.

Two visible LFOs, broad macro routing, WT POS, 3D frame views, the six original acid waveform families, wet EQs and the separate gate/sequencer from v0.3.0 remain included.

## Test it

Download the **v0.4.0 Windows installer ZIP** from its release page, extract it, close FL Studio, run Setup.exe and reopen a fresh synth instance. Keep older installers if you want to compare versions.

1. Compare the new component switch on/off on a factory preset. Turn up one control at a time. VCA drive can compress peaks; it is a tone control, not an automatic loudness-matching switch.
2. Check that old saved sounds recall and the original analogue/filter controls still work.
3. Try Hold on a short-decay sound, then drag the amp/filter envelope graphs and contours.
4. Pan each source and try high-pass/band-pass/ladder modes. Check delay/reverb tails separately when assessing stereo position.
5. Compare the clean-VCO option on a high saw/pulse note with Character low. It is a targeted correction, not an alias-free guarantee for every sound.
6. Play fresh and held notes for several minutes and report any remaining clicks, fade or excessive CPU use with the preset name.

The audit and complete [native control inventory](audit/NATIVE_PARAMETER_INVENTORY.json) are included with the installer ZIP. The implementation draws on the official [Serum 2 manual](https://xferrecords.com/manual/serum-2/docs) and the [Diva manual’s Trimmers section](https://u-he.com/downloads/manuals/plugins/diva/Diva-user-guide.pdf). Numerical sound equivalence and overall superiority are not established by those manuals.

## Native screenshots

[Main and smaller logo](images/Rhythm-Main.png) / [Analogue Lab](images/Analogue-Lab.png) / [Amp envelope](images/Analogue-Amp-Envelope.png) / [Filter envelope](images/Analogue-Filter-Envelope.png)
