# SPACE INFINITY 0.7.2 — native verification

Validated 18 November 2026 on Linux using the compiled Release JUCE native editor/DSP.

- All 24 native suites passed in 235.39 seconds. See [complete results](LAYOUT_MODULATION_072_NATIVE_TESTS.txt).
- Following the final popup-layer adjustment, HardwareUI, AnalogueExpansion, PitchTravel, ModulationWorkflow and DualLFO passed again: 5/5 in 20.14 seconds. See [focused results](LAYOUT_MODULATION_072_UI_TESTS.txt).
- Factory-edit import/reject Python test passed; source diff whitespace checks passed.
- All 1108 previous parameter definitions and indices are preserved; 99 parameters are appended (1207 total). Factory 160 and Heritage 200 banks remain byte-identical. See [compatibility evidence](LAYOUT_MODULATION_072_COMPATIBILITY.json).
- Native Main/FX/Gate/Seq previews and bounds cover 800 × 760, 1000 × 790, 1180 × 820, 1280 × 760 and 1500 × 950. Sampler previews cover 800 and 1280 pixels. Default is 1280 × 760; interactive minimum is 1000 × 760.
- Keyboard testing uses a native desktop peer: clicking a displayed key produces actual audio, release stops it, and no stuck note remains. Independent pitch stops, centred spring return, host/MIDI bounds and held MIDI wheel rescaling pass.
- Modulation tests cover source selection, actual drops, ring/Alt gestures, fine adjustment, polarity, numeric/inverted amounts, route capacity/recall and audible signed-envelope modulation. Hold/Decay plots and LFO point changes follow their settings.
- Chorus/MELT checks cover all four modes at 44.1/48/96 kHz, dry bypass, bass/mono behaviour, tone, shared-harmonic interaction, single-oscillator bypass, two-oscillator native audio, UI and recall. Existing factory chorus/MELT settings are retained.

Actual screenshots: [Main](../images/Layout-MAIN-072.png), [Sampler](../images/Layout-SAMPLER-072.png), [Dimension chorus](../images/Dimension-Chorus-072.png).

Windows 0.7.2 build/installer verification and owner FL Studio listening remain pending. No 0.7.2 installer or public release has been supplied. The owner's download/publication hold remains in force. The saved verified 0.7.1 installer predates this work.
