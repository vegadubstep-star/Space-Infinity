# SPACE INFINITY 0.9.4 — complete installed bank

The [Curated 300](CURATED_300_UPDATE.md) sound presets plus Init load automatically
after installation. This includes selected Heritage, Lush Trance and new
sample-based sounds. The 421-sound development palette was reduced after a
three-register comparison and individual chain review. The WAVs
are embedded in their presets; no separate sample-folder download or bank import
is required. Optional bank ZIPs remain available for backup and recovery.

The full bank is measured with K-weighted, gated perceived loudness using the
same articulated/held reference phrase. Static reference gains match the sounds
to a common level while retaining their individual envelopes and existing macro
compensation. This does not use automatic gain riding. Current measurements and
source hashes are recorded in audit/INSTALLED_BANK_094_LEVELS.json.

Existing room ambience on keyboard/organ sounds is retained. Basses stay dry; existing trance lead/pluck delays, pad reverbs, detune and articulation
keep their programmed character. Near-zero lead attacks are preserved.

Clean headroom protection now uses a linked stereo, four-times peak detector
with short look-ahead. It catches overloads between samples without the previous
post-output clipping curve. When peak reduction is idle the audio is unchanged apart from
the latency reported to the host. Overload, stereo balance and recovery checks
cover 44.1, 48 and 96 kHz; there is no lingering level reduction after a loud hit.

Automatic updates preserve personal sound replacements, names, creators and tags.
Existing projects without the new reference-gain parameter retain their original
gain setting. The original 1,227 host parameter IDs keep their order; the new
reference-gain parameter is appended. The earlier seven-sound recovery bank and the new 121-alternative bank preserve
access to retired variants. Stable project IDs remain intact.

Matching applies to the documented playing phrase. Different notes, chords,
velocity, macros and user edits change loudness; watch the output meters while
editing. Historical 0.9.3 audio reports describe the earlier bank revision.

Reimporting an already-current bank skips identical files, avoiding unnecessary
rewrites and duplicate backups while preserving personal edits.
