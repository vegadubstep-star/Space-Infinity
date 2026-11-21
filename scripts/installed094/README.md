# Complete installed-bank review

ReviewInstalled.cpp links against the actual native engine. It loads the three
embedded banks, applies the production curation and measures all 421 active
sounds. Init is retained as the neutral reset patch.

The meter uses ITU-R BS.1770 K weighting at 48 kHz, stereo channel weights 1/1,
400 ms windows with 100 ms hops, a -70 LUFS absolute gate and -10 LU relative
gate. FFmpeg ebur128 agrees within 0.04 LU on steady and gated fixtures.
Four-times oversampling measures reconstructed peaks.

Each sound receives a 6.4-second articulated/held phrase. A separate stress case
covers three registers, full velocity, chords for non-bass sounds and all six
performance macros at maximum. Twenty-one previously dry keyboard/organ sounds
receive a small room; bass and other programmed effect chains are retained.

match-bank.py plans a static presetReferenceGain independently of presetTrim,
so existing macro compensation keeps its range. It bakes the measured settings
into the factory JSON and all active extra-bank files. It retains original
snapshots in the supplied work directory, preserves the seven archived bytes,
and records previous shipped hashes so updates protect user edits. Use a new
work directory for a new tone revision. Do not treat a gain prediction as proof:
rebuild the embedded data and rerender all sounds before verify-matching.py.

ReviewInstalled arguments: output-folder repository [changes-JSON-or-dash]
[first-index] [last-exclusive] [shared-runtime-folder] [--changed-only]. Separate
output folders allow two render jobs to share the installed library safely.
Never overwrite a running renderer binary.

verify-matching.py checks actual final loudness, reference gains, source preset
hashes, ZIP hashes, update histories, default headroom and stress true peaks. Its
release audit identifies the exact renderer binary. The native InstalledBank
integration test exercises the same constructor bootstrap as the plug-in, with
an isolated writable library: 421 sounds plus Init, 48 embedded sampled presets,
repeat startup, calibrated settings and preservation of a personal replacement.
