# Ten control updates — Windows 0.2.0

This batch contains the following requested changes:

1. **Oscillator gain:** a separate dB gain knob on A and B, before the filter and FX.
2. **Independent unison:** Voices, Detune, Blend and Width per oscillator. Factory detune amounts are derived from their former spread settings. This is an independent implementation, not Xfer's proprietary algorithm.
3. **Main-page envelopes:** clearly labelled AMP ENVELOPE and FILTER ENVELOPE with separate ADSR controls and filter amount.
4. **Fine envelope timing:** more knob travel at short times, Shift-drag, double-click reset and typed values such as `0.25 ms`. LONG x3 readouts include the multiplier.
5. **Filter sources:** A, B, SUB and NOISE switches; deselected sources bypass the filter.
6. **Main-page sub:** power, six waveforms, octave and level. Sampler trim/root/reverse controls appear when a sampled source is selected.
7. **Delay wet EQ:** low shelf, mid bell and high shelf, each with frequency, gain and Q. This EQ is on the return, outside the feedback loop, and does not EQ the dry signal.
8. **Reverb wet EQ:** an independent three-band return EQ, with the existing dry gain and its smoothing preserved.
9. **FX gain and metering:** every rack module has an output gain, stage peak meter and clipping indication. The main output also has a dBFS/clipping readout.
10. **Smoother stereo flanger:** smoothed rate, depth, delay, feedback, stereo phase and mix controls; slower knob travel and stable bounded feedback. Factory patches use Smooth stereo. Older sessions retain Legacy mode.

The lower main-page area also consolidates the LFO controls and six performance macros. Shape is hidden without removing its legacy parameter. Three new macros start unassigned. LFO tabs and new macro labels can be dragged to supported synth/FX knobs; right-click an assigned knob to change/remove its signed amount. Hover the MAIN/FX tab during a drag to switch page. Routes are saved by parameter ID in presets/projects.

This batch does **not** complete the requested LFOTool-style gate interface/separate sequencer page or the further purple-metal/logo redesign. De-click protection is included, but a claim that every reported click is eliminated still requires listening in the user's FL Studio setup.

## Validation

The workflow test checks actual wet-EQ frequency response, exact flat bypass and dry isolation; extreme flanger feedback; independent oscillator gain/detune; all four filter source paths and six sub waveforms; FX dB gain/peak/clipping; fine millisecond entry; route and parameter state recall; and old-session algorithm defaults. The existing native suites cover the 34-entry factory bank, long-term pad/lead playback, UI at four sizes, embedded assets, dynamics, delay timing, convolution, EQ and gate/sidechain behavior.

Windows CI builds the VST3 and installer, runs all eleven native suites, installs/uninstalls the package and checks that the installed VST3 matches the archive. FL Studio listening and host-side automation remain manual tests.
