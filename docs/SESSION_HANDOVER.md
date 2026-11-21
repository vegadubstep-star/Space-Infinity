# SPACE INFINITY 0.9.1 — completed and publicly verified

User request: extensive online research and more polished trance/hard-trance presets.
All implementation, audio validation, Windows tests, installation checks and public
release verification are complete. No implementation or delivery work is pending.

## Download and source

- Release: https://github.com/vegadubstep-star/Space-Infinity/releases/tag/v0.9.1
- Installer: https://github.com/vegadubstep-star/Space-Infinity/releases/download/v0.9.1/SPACE_INFINITY_0.9.1_Windows_x64_Installer.zip
- Binary source: `4f7ee13b6033d4d603014a02b19630425a36610a`, branch `codex/windows-build`.
- Windows workflow: https://github.com/vegadubstep-star/Space-Infinity/actions/runs/44897593118
- Package job: `136826462106`; all required steps passed.
- ZIP SHA-256: `c1f3d109c165f6af44941542db63e6f88e8b5cb4eca6725d79a66afac6f6c76c`.
- Setup SHA-256: `296dc1a77317e359f203ac363f3d5977471415ec8318ff7752897c2cc27d9a4d`.
- Manual VST3 ZIP SHA-256: `c95dd75a6b028855d358c57373e1a18be4d28702593136704a8bcffd4a416399`.

The final documentation-only commit records these results; binary source remains
the feature commit above. Only the public GitHub ZIP is approved for delivery.

## What changed

Ninety existing factory slots refined: 30 basses, 20 hard-trance leads, 20 plucks,
10 airy pads and 10 acids. Stable IDs/names; the other 70 factory recipes and the
optional Heritage 200/Lush Trance 20 ZIPs are unchanged. Total remains 160 factory
sounds, or 380 with both optional banks imported, plus Init.

Individual bass contours and harmonic colours; firmer, tight lead attacks;
filter-articulated plucks with OPEN/LEAD wet-FX reduction and gain compensation;
longer airy swells and gentle timbral motion; distinct acid wave/filter/drive
paths with audible cutoff contrast, velocity accents and legato glide.

Untouched factory tags refresh to expose Research 091 while preserving user
metadata edits and local replacement bytes. No DSP algorithm changed in 0.9.1.

## Verification

All 27 Linux native tests passed (154.01 seconds), and all 27 Windows native tests
passed (554.78 seconds). Actual Windows install/uninstall and installed/manual VST3 byte equality
passed. The public release ZIP and manual ZIP were independently downloaded:
CRC, published hashes, Setup hash, x64 VST3 PE, module version 0.9.1, required audit
contents, source recipe hashes and both optional banks passed.

Actual embedded exports match all 90 authored recipes, active routes and metadata;
native inventory: 1,223 parameters. Audio checks cover repeated notes/chords,
all 8 free-macro corners/all 6 macros, three-register stress in Analogue/Digital,
and 35 basses at MIDI 36/43/48/55 in both modes. Maximum measured true peak:
-0.362745 dBTP. Maximum pluck OPEN change:
0.63 LU. No threshold similarity candidates or
level errors remain. These establish technical behaviour; listening in FL Studio
on the user's PC remains the user's next step.

Evidence: docs/audit/STUDIO_RESEARCH_091_{AUDIO_LEVELS,RECIPES,FACTORY_PARITY,
PUBLISHED_VERIFICATION}.json and {NATIVE_TESTS,WINDOWS_TESTS}.txt.
Research sources/design decisions: docs/research/STUDIO_RESEARCH_091.md.
Update/test guide: docs/STUDIO_RESEARCH_091_UPDATE.md and TESTING_STUDIO_BUILD.md.

## User testing

Close FL Studio, Extract All, run Setup, then reopen the synth. Select Research 091
or Studio Research in BANK; no bank import is needed for these 90 revisions.
Existing local factory replacements may override updates: save a copy before
restoring factory sounds for comparison. Optional banks import as before.

## Workspace continuity

Checkout: /workspace/work/native-design. No active local processes/builds.
JUCE: /workspace/work/JUCE. CMake/CTest: /workspace/work/build-tools/cmake/data/bin.
Native build: native-vst3/build-linux. Tests: DISPLAY=:99 and writable XDG cache.
Public verification files: /workspace/work/release-verification/091.
Preserve owner untracked audition/ and __pycache__/ folders; do not bulk-add them.

Calibrated source and STUDIO_RESEARCH_091_RECIPES.json are authoritative. Do not
regenerate from initial seed scripts: late acid/Digital/bass corrections matter.
Rich authoring evidence remains in /workspace/work/preset-research091, especially
FinalDesigns.json, final-review/Verified.json, StressVerified.json,
DigitalVerified.json and BassRegisterReferences.json. Temporary package-preflight
fixtures deliberately contain a non-executable installer; never distribute them.

User listening feedback can guide the next revision. Bump the version for future
binary changes; do not overwrite this verified release. No subagents were used.
