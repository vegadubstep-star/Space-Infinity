# Acid cutoff regression — 0.6.2

The original six filter implementations and cutoff host parameter ID/units/mapping are retained. The seventh Acid Resonant response is selected by all ten revised factory acid patches. The other150 design hashes match0.6.1.

Small-signal impulse measurements at44.1/48/96kHz place the resonant peak near180/800/3200Hz when those cutoffs are requested (approximately179–180/806–807/3219–3229Hz). Fast80Hz–16kHz sweeps at maximum resonance and three drive levels remain finite and decay after the input stops. Stereo routing is exercised across all seven filters and four sources.

Actual driven factory patches are measured at160Hz and8kHz cutoff. Every patch has at least5x more relative energy above2kHz when opened, with a nontrivial open high-frequency band. This is independent of loudness: tests use spectral energy fractions. A full accented-note automation render then sweeps80Hz–16kHz and back. Positive envelope excursions approach the available high-frequency headroom smoothly.

Final maximum raw full-sweep peak: 0.1908718. Per-patch values are in ACID_SWEEP_062.json. The complete160-preset audition audit measures -29.60..-27.78 LUFS, spread1.82LU, maximum chord/raw automation peak0.7800004. Exact results: TRANCE_LOUDNESS_062.json. Unchanged150 auditions are reused from verified0.6.1; all ten revised acid auditions and automation files were rendered afresh.

The RMS-matched [old/new spectrogram](../images/Acid-Cutoff-Comparison-062.png) uses identical MIDI notes and cutoff automation. It shows the change in high-frequency content over the sweep, without relying on a louder comparison. Audio/profile evidence is in /workspace/work/acid-cutoff-062-final and /workspace/work/levels-062-final-{solo,chord}. The short shared demo uses constant gain and a final40ms fade; it adds no processing or dynamics to the synth output.

The complete18-suite native run and Python preset-edit importer are required before publication, followed by Windows native tests, installer/uninstaller and downloaded-release byte checks. Musical quality in the user's FL Studio project remains a listening evaluation; the update is not claimed as a component-exact303 reproduction.
