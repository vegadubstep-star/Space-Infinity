# SPACE INFINITY 0.6.1 — stronger acid, new bass and owner choir stabs

Distortion now has a real INPUT stage, an independent TONE control, HQ anti-aliasing and three additional characters: Transistor, Tube Stack and Hard Drive. The display reports the driven signal level and whether it is reaching saturation. Quiet synth voices can now push the shaper properly; the final output trim remains separate. All ten acid patches use deliberately driven, mostly/full-wet settings, with contrasting resonance, articulation and shaping rather than a small amount of the same effect.

All 35 bass presets have been revoiced around clean fundamentals and deliberate body/sub layers. The offbeats cover rounded saws, hollow pulses, octave/fifth layers, restrained FM, organ-like body and a few driven stabs. The rolling basses use tighter envelopes and controlled harmonics; five sub presets focus on stable low end. They remain unsequenced: play your own MIDI patterns.

All ten choir stabs now use the owner's uploaded recordings. No extra detuned copies, vibrato, chorus or moving LFO are added. The bank includes tight/deep rave hits and brighter short vocal stabs. The separate Choir Stabs ZIP includes ten portable .sipreset files with their samples embedded and ten stereo WAV one-shots, ready for a DAW sampler. Source names, root-note decisions and processing are in USER_CHOIR_PROVENANCE.json. The previous choir sources remain available for older projects.

PORTAMENTO is now beside the main-page voicing selector. Dragging it from Poly switches to Legato; overlap notes to glide. Shift-drag gives finer adjustment. Incoming host automation does not silently change the voicing mode.

Analogue Lab has Calibrated, Studio and Vintage ranges. Studio/Vintage process colour at the voice's circuit level, before the quiet output scaling, so VCA drive and asymmetry can actually change the tone. Drift, tolerance and coupling have wider useful ranges. ACTIVATE LAB makes the routing explicit; NEUTRAL, WARM and VINTAGE give easy comparison settings. A status line identifies Digital-mode or Lab bypass. Oscillator/pulse controls apply to Synth; drive, memory and coupling also colour sample sources.

Saved 0.6.0 states default to zero extra input, open Tone, HQ off and the calibrated Lab range. Existing mode indices and sample-source IDs remain in place. The owner choir sources are appended, not substituted under old saved IDs. The bank remains 160 sounds plus Init; 56 voicings are revised and 104 retain their designs. New levels are measured after sound design, with chord headroom checks.

In a quiet-input sine test, the driven setting generated roughly 20.8% THD versus 0.17% for the earlier setting. In a separate high-frequency hard-clipping test, HQ reduced the measured alias fraction by about 95% at matched signal power. These demonstrate real nonlinear shaping and anti-aliasing rather than an output-volume increase; they are specific test conditions, not guarantees for every sound.

HQ uses first-order antiderivative integration for the common analytic and piecewise-linear curves, with four-point numerical integration for the non-analytic/cascaded characters, alongside the synth's existing oversampling. The implementation was checked against the principles in [Parker, Zavalishin and Le Bivic, DAFx 2016](https://www.dafx.de/paper-archive/2016/dafxpapers/20-DAFx-16_paper_41-PN.pdf). It is our DSP implementation, not a claimed copy of another plugin's proprietary circuit models.

Close FL Studio before installing. Load a fresh instance and select the new factory presets when comparing; saved projects retain their saved settings.
