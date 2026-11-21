# SPACE INFINITY 0.9.3 — Windows release verification pending

All requested sampler/bank/portamento/menu fixes are implemented and native-tested.
Latest steering fixed: preset selector and arrows follow saved/manual bank order,
anchored to the last selected sound. A–Z is explicit only.

Changes: clean stereo/full-range sample import; independently controlled OSC B
layer; stereo granular; finer finite glide with new-phrase reset; popup ownership;
automatically installed Heritage200, Lush20 and new48 Orbit Voices sounds from
40 actual Infinity_*.wav files. Seven close Heritage variants archived with recovery
ZIP; local sound and metadata edits retained. Existing160factoryIDs and1223host
parameter IDs/order preserved, four appended (1227total).

Verification complete locally:
-28native tests PASS (278.31s), plus4 focused tests on final embedded bank/curation.
-48new presets verified at3registers x9macro corners, fullvelocity notes/chords,
4xtruepeak: maxpre-safety0.900000572, maxtruepeak-0.630338735dBFS, noerrors.
Measurement source hashes match all48 shipped files and calibrated trims.
-All428 presets audio-feature screened; no silent sounds, no new-bank close pairs.
Eight existingclose pairs confirmed at3registers;7archived, distinctacidsretained.
-40actual renamed WAV file hashes match source uploads/provenance.
-Packagepreflight: actualbank/hash/audits/1227params pass; missing evidence and
failednativeaudits rejected. PowerShellsyntax andfactoryeditPython tests PASS.

Evidence docs/audit/SAMPLER_FULL_RANGE_093_{NATIVE_TESTS.txt,AUDIO_LEVELS.json,CURATION.json}.
Portable48bank and7recoverybank in native-vst3/Presets/UserVoices093.
No0.9.3publicdownloadsent. README remains0.9.2 until newpublicverification.

NEXT: commit/push [skip ci], dispatch space-infinity-windows.yml on
codex/windows-build with publish_release=true. All28Windows, install/uninstall,
archivevsinstalledbyteidentity andpackagecontents mustpass beforedelivery.
Then run /tmp/verify-space-infinity093-release.py SOURCE_SHA RUN_ID JOB_ID to
independentlydownload publicinstaller/manual ZIPs, CRC/SetupMZ/hash/x64PE/module093,
sourcecommit/bank/calibration/curation/currentevidence. CaptureWindowsverification
log+publicproofJSON in docs/audit, updateREADME/handoverstocomplete, docs-only
[skip ci]commit/push, finalconciseinstallerlink+ExtractAll/closeFL/Setupsteps.

Rawreports/helper source retained in /workspace/work/space093; generatedRuntimeLibrary
and/tmp/Profile093 can be removed after completedworkers to free~210MB.
No subagents allowed or used. Preserve owneruntracked audition/ andallcaches.

# SPACE INFINITY 0.9.2 — completed and publicly verified

User requested preset arrows to continue from last clicked sound, sample
portamento and granular EDIT dismissal on preset/plugin selection. All fixes,
checks and public release verification are complete; delivery is ready.

Release: https://github.com/vegadubstep-star/Space-Infinity/releases/tag/v0.9.2
Installer: https://github.com/vegadubstep-star/Space-Infinity/releases/download/v0.9.2/SPACE_INFINITY_0.9.2_Windows_x64_Installer.zip
Binary source: a107fd696c29b548da2573d63cb45a387699444a, branch codex/windows-build.
Windows run44903124623, package job136843028697: success.
ZIP SHA256: d8fc81def6d1bf6ecd914c0824112c752a98798df8a9deba3d2605fabf3769c6
Setup SHA256: 57e0d0951bdf99c7615f433980906948f7c918cc5c126bf37db37c1664ad5f9f
Manual VST3 ZIP SHA256: b261b10ed49824079ed49110aec5bc7d16a0376f3715043a0214c7c5cd1b5f2f
Final docs-only commit records proof; binary source remains the feature SHA.

## Fixes

BANK single-click loads; Return/double-click supported. Filters alone do not
change sound. Header arrows follow the displayed category-grouped order, keyed
to the loaded sound, including project recall/reordering/reopen and wrapping.

Sample PORTAMENTO adjustment from Poly selects Mono. Mono glides across note gaps
and finished samples, Legato only on overlaps. Remembered pitch clears on
preparation, preset load, voice-mode change and panic; channel/source guards.
No loop is forced. Oscillator UI retains automatic Legato selection from Poly.

Granular, sample-loop and filter EDIT use editor-owned popups. BANK replaces the
panel; preset dropdown and page navigation close it. Editor destruction is safe.

## Verification

All27 native tests PASS123.96s, focused3/3PASS4.75s; all27 Windows tests PASS
564.66s. Windows actual install/uninstall and installed/manual
VST3 byte equality passed. Public ZIP/manual downloaded independently: hashes,
CRC, Setup, x64 PE, module0.9.2, 1223parameter inventory, required docs/current
native report and optional bank hashes/preset bytes passed. Factory160 recipes
and optional Heritage200/Lush20 archives are unchanged from0.9.1. Earlier091
audio evidence is explicitly historical; no new preset calibration claimed.

Evidence: docs/audit/BROWSING_SAMPLE_GLIDE_092_{NATIVE_TESTS,WINDOWS_TESTS}.txt
and BROWSING_SAMPLE_GLIDE_092_PUBLISHED_VERIFICATION.json. New notes and test guide:
docs/BROWSING_SAMPLE_GLIDE_092_UPDATE.md and docs/TESTING_STUDIO_BUILD.md.

The initial full run had26/27: Smoke.cpp still expected raw storage order at
wraparound. Assertion now checks the selector's displayed last item; no sound
or safety check weakened. Final full27native and Windows passes verify the fix.

## Testing and workspace

Close FL Studio, Extract All, run Setup, reopen a fresh instance. Click a middle
preset, use arrows; test sampled glide in Mono/Legato; open granular EDIT then
BANK/dropdown/FX and verify dismissal. Saved user sounds/replacements preserved.

Checkout /workspace/work/native-design, JUCE /workspace/work/JUCE, tools
/workspace/work/build-tools/cmake/data/bin. Native build native-vst3/build-linux;
DISPLAY=:99 XDG_CACHE_HOME=/workspace/work/runtime-cache. No local builds/tests
active. Public downloads/proof /workspace/work/release-verification/092.
Temporary scripts /tmp/verify-space-infinity092-release.py and close-space092.py.
Package-preflight092 fixture is NOT executable and must never be distributed.
Preserve owner untracked audition/ and __pycache__ folders. Do not bulk-add them.
Generated audio helper binaries removed for disk space; source/evidence retained.
No subagents used. Future binary work needs a new version and full verification.
