# SPACE INFINITY 0.9.1 — local work verified, Windows delivery pending

User requested extensive online research and better presets, then Continue.
Ninety existing factory slots are revised: 30 bass,20 hard-trance lead,20 pluck,
10 airy pad,10 acid. Other70 factory rows and both optional bank ZIPs are exactly
unchanged. Count remains160 factory; stable names/IDs and user edits are preserved.

LOCAL CHECKS COMPLETE: all27 native tests PASS154.01s. Actual embedded factory
exports match all90 recipes/active routes/labels/metadata. Inventory is native
0.9.1 with1223 parameters. Audio checks cover all8 free-macro corners/all6 macros,
three-register sweeps in both modes, plus35 basses at36/43/48/55 in both modes.
No headroom errors or .995 spectral+envelope similarity candidates remain. Pluck
OPEN endpoints differ by at most .63LU. See current audit JSON for exact peaks.
PowerShell release scripts parse. Synthetic installer-content/CRLF preflight passes
and rejects missing factory-parity evidence or failed native-test reports.

IMPORTANT VERIFIED INPUTS: docs/audit/STUDIO_RESEARCH_091_RECIPES.json is the exact
renderer snapshot; native-vst3/Presets/TranceFactoryBank.json is integrated source.
Do not regenerate these calibrated settings from seed scripts. Rich checkpoints
remain at /workspace/work/preset-research091/{FinalDesigns,StressVerified,
DigitalVerified,BassRegisterReferences}.json and final-review/Verified.json.
Audio report, native report, factory parity and recipes are required in installer.
Text hashes normalize CRLF to LF for Windows checkout. No new bank import needed.

Regression findings resolved without weakening sound/level checks: four-register
bass headroom; four acids whose overdrive masked cutoff; one overly similar acid
received a more percussive amp contour. ChorusMelt's factory-choice expectation
was updated for intentionally selected Dimension chorus on sampled airy pads;
its DSP/width/mono/recall checks remain. Factory metadata refresh now updates
untouched factory tags while preserving owner metadata/sound replacements.

NEXT: review/stage explicit task files, commit[skip ci], push codex/windows-build.
Starting HEAD1baca46; last fetch had no remote changes, v0.9.1 does not exist.
Dispatch space-infinity-windows.yml with publish_release=true. Inspect all Windows
checks/installer tests, then independently download/verify public ZIP and manual
VST3/checksums/moduleinfo0.9.1/source commit. Only then send user the download and
update README/handover with public verification in a docs-only[skip ci] commit.
No0.9.1 download has been sent. Main README still links verified0.9.0 deliberately.

All local processes/builds are complete; no active native build. Native executable
native-vst3/build-linux/SpaceInfinitySmoke_artefacts/Release/SpaceInfinitySmoke.
Helpers /tmp/RenderLush(original0.9 audio), /tmp/RenderResearch(sweep), /tmp/RenderFactory
(current0.9.1 export), /tmp/BassProbe. /tmp/AcidProbe was removed to reclaim disk;
its source and diagnostics remain in work folder. Do not overwrite running tools.
Native tests use DISPLAY=:99 XDG_CACHE_HOME=/workspace/work/runtime-cache.
PowerShell needs writable XDG cache/config/data folders. No subagents spawned.
Preserve owner untracked audition/ and all __pycache__ folders. Everything is still
uncommitted until the final review; no irreversible file cleanup was performed.

## Previous verified release — 0.9.0

Release: https://github.com/vegadubstep-star/Space-Infinity/releases/tag/v0.9.0
Installer ZIP: https://github.com/vegadubstep-star/Space-Infinity/releases/download/v0.9.0/SPACE_INFINITY_0.9.0_Windows_x64_Installer.zip
Binary source: 8410c90a250f7e3e141788f735d5cf368c9c4e06, codex/windows-build.
Windows run44839093943, job136642742044: SUCCESS. All27 tests PASS550.29s;
actual installation/removal and installed/manual VST3 byte equality PASS.
Public ZIP independently downloaded; CRC/checksums/x64 PE/moduleversion0.9.0,
Lush20 + Heritage200 bank integrity, 27-recipe audio audit, current readme and
1223-parameter inventory PASS. Public bank/content validator also PASS.

Public ZIP SHA256: 0a227f26d9a79f488b24f32129bcf465456a144920642d548971a81e54e7889f
Setup SHA256: 0b8d29fca2421edb907c5f0951cb472c86360cf177e9003516761f4b03d4fffb
Lush20 ZIP SHA256: 481ae6916cc8483f51167cff6950ba4163eb441718446426a8546500a2549932
Heritage200 ZIP SHA256: 43985d0daa8f43abd36aff0747b824931c62314c3cc1b73962ba69218b3035c0

Features: 15 filters incl. formant/notch/comb/steep HP/BP/morph and a live graph;
stereo sample cloud, smooth forward/reverse looping, diffusion/warmth/Freeze;
correct sampler/granular octave controls; optional Voice Life; A>B/B>A note
transition; larger main controls; separate 20-lead Lush Trance bank; seven quiet
uplifting factory leads gain3.84..5.05LU. Existing filter automation range0..6
is preserved via appended Character Filter Override0..8. 1223 host parameters,
52 oscillator families, 45 sources. Other153 factory + Heritage200 recipes intact.

User install: close FL Studio, Extract All, run Setup. Then BANK>IMPORT the inner
SPACE_INFINITY_Lush_Trance_20.zip without extracting it; select Lush Trance20 tag.
Factory160 + optional Heritage200 + Lush20 = 380 sounds (plus Init) when imported.
Actual listening/FL Studio testing on the user's PC is the next user step.

Levels: final27 new/revised recipes measured, maximum4xtruepeak -0.349934dBTP.
Calibration covers all8 user-macro corners plus all6 macros; final renders recheck
base/all-open/worst corner, singles/retriggers and4-note chords. New20 closest
spectral cosine .99033 (<.995 review threshold). Do not regenerate measured recipes.
Local full27 PASS351.26s before UI lifetime followups; final focused4/4 PASS14.54s.
Windows exact-source full27 confirms all final code. Presets/audio unchanged by UI fixes.

Resolved blockers: stale installer allowlist/readme fixed; octave attachments now
detach before Slider destruction; filter/sampler EDIT callouts are owned and close
with editor. Old diagnostic44837546513 confirmed all audio checks passed before
teardown0xc0000005. Superseded runs44834594534/44835072301/44838126358 are not release
candidates. Current8410c90 source and final44839093943 are verified.

Evidence: docs/audit/CHARACTER_090_{NATIVE_TESTS,WINDOWS_TESTS,PUBLISHED_VERIFICATION}.*
and LEADS_090_AUDIO_LEVELS.json. Native bank recipes/ZIP under Presets/Lush20.
Reproduction: scripts/review-lush090.py, package-lush20.py, installer-content validator.
Local helper /tmp/RenderLush; final audio data /workspace/work/lead-review090.
Downloaded verified release files /workspace/work/release-verification/090.

The previous release had no pending implementation work; the new preset request
above is now active. Preserve owner untracked
audition/ and __pycache__/ directories. Latest source changes are committed/pushed;
the final documentation-only commit records public verification. New preset work must be measured and verified before delivery.
