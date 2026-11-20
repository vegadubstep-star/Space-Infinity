# SPACE INFINITY 0.9.0 — filter and sampler character

Eight additional filter models bring the main filter selector to 15: Notch,
All-pass, 24 dB High-pass, 24 dB Band-pass, Formant vowels, positive and negative
feedback Comb, and continuous LP/BP/HP Morph. The original seven remain in their
original slots, including the nonlinear analogue and acid circuits. Their host
automation range stays unchanged; an appended Character Filter Override parameter
provides the extra types behind the combined 15-item MAIN selector.

The main filter has a pink/teal response display. Drag horizontally for cutoff
and vertically for resonance. EDIT opens the larger display and two shape knobs:
Vowel (A/E/I/O/U) for Formant, and Morph for LP/BP/HP. Both accept LFO and macro
assignments with the usual destination amount controls. Cutoff shifts vowel
resonances or comb tuning; Resonance sets vowel focus or comb feedback. The bright
curve follows the latest rendered voice, including its filter envelope and LFO;
the faint curve shows the unmodulated knob setting. Analogue/acid response plots
are small-signal approximations, not exact predictions of nonlinear distortion.
This is a filter response view, not an input/output spectrum analyser.

Sampled sources now show two dedicated panels on MAIN, replacing oscillator
controls in those slots. LOOP & TEXTURE has start, end, root, loop crossfade,
diffusion and warmth. GRANULAR BLEND has mix, position, size, density, spray and
width, with Freeze. EDIT provides larger knobs. Loop and Reverse remain next to
Import WAV. Crossfade works forward and backward and clamps safely for short
trim regions. Grain mixing works with imported WAVs and factory sampled sources;
window-sum normalisation and minimum overlap avoid density-related gaps. Freeze
captures the current playback position; enable Loop to sustain a one-shot sample.

The octave fields above sampled sources now bind to Sampler Octave and Granular
Layer Octave, rather than the inactive synth oscillators. Both span -4 to +4;
the granular pitch is relative to sampler pitch. Master octave and pitch wheel
still apply globally. Returning to Synth restores the oscillator octave controls.

LIFE opens a full-size assignable Voice Life knob: subtle independent pitch wander
and per-voice filter variation, with no amplitude LFO. It is optional; it does not
silently revoice existing factory/user patches. Performance macros now use two
rows of three to give their knobs more space. Envelope graphs reserve more space
for their knobs, and the filter grid uses two rows of three.

New controls default to bypass/zero so existing saved projects retain their sound.
Seven previously quiet uplifting factory leads have revised sustain, slightly drier
returns and macro gain compensation, followed by native peak calibration. This
raises their usable starting level without simply pushing their macro extremes
into clipping. Their IDs and names are retained. The other 153 factory patches
and the separate Heritage 200 recipes/trim data are unchanged. The 0.8.0 bank
audio report remains historical evidence; the new report covers the 20 additional
leads and seven revised ones. Output protection remains active, but changing
resonance, voice count or effects can still change loudness and headroom.

Validation is recorded in docs/audit/CHARACTER_090_NATIVE_TESTS.txt when complete.

## Oscillator journey and additional leads

A > B opens a per-note oscillator transition: amount, seconds, curve and A-to-B
or B-to-A direction. Enable both oscillators. At 100% it starts on the first
oscillator and ends on the other; lower amounts blend the journey with Layer.
The amp envelope still sets note length. Zero preserves existing Layer behaviour.
This transition is for the synth oscillators, not sample crossfading.

Lush Trance 20 is an additional importable bank. It explores pulse/parabolic,
supersaw, string/reed, organ/saw, bell-air, vowel and morph-filter architectures,
with original names and fast attacks. OPEN progressively dries the spatial returns
as the filter opens, BODY adds oscillator interaction, and HORIZON adjusts width
and voice character. Specific presets also use the per-note oscillator journey.
