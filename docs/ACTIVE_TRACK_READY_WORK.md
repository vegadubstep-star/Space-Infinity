# SPACE INFINITY 0.9.2 — local fixes complete, Windows delivery pending

Latest request: preset navigation continues from last clicked sound; portamento
on samples; granular EDIT closes when opening preset/plugin selector.

Implemented and verified locally:
- BANK single-click loads the row; Return/double-click still work. Filtering alone
  does not load a sound. Header arrows follow the category-grouped dropdown order
  by current stable key, including recall/reorder/reopen and wraparound.
- Mono glide memory survives note gaps and finished samples; resets on prepare,
  preset load, mode changes and panic. MIDI channel/source guard prevents leakage.
  Legato retains overlap-only glide. Sample PORTAMENTO adjustment from Poly
  selects Mono automatically; oscillator workflow still selects Legato.
- Granular, sample-loop and filter EDIT delegate popup ownership to the editor.
  BANK replaces the panel; opening preset dropdown or changing pages dismisses it.

CHECKS COMPLETE: all27 native tests PASS123.96s; focused3/3PASS4.75s. Native inventory
exported0.9.2,1223parameters. Preset-edit Python test/PowerShell syntax/installer
preflight pass; CRLF recipe hash accepted and missing/failed audit rejected.
First full run26/27 had only outdated Smoke.cpp wrap expectation exit23 (raw bank
last vs displayed last). Test updated to assert grouped menu wrap, then all27 pass.
Native build complete; no active local builds/tests. Do not edit native source
while any future compilation runs.

Factory recipes, Heritage200 and Lush20 ZIPs unchanged. Old091 sound reports are
historical, retained separately; current092native report is required in installer.
New notes docs/BROWSING_SAMPLE_GLIDE_092_UPDATE.md; testing guide current0.9.2.
README intentionally still links verified0.9.1 until new public ZIP is verified.

NEXT: commit explicit task files [skip ci], push codex/windows-build; dispatch
space-infinity-windows.yml publish_release=true. Follow all Windows native and
install/uninstall/ZIP/publish checks. Do not send a new download until independently
verified. Prepared verifier:
python /tmp/verify-space-infinity092-release.py SOURCE_SHA RUN_ID PACKAGE_JOB_ID
It downloads PUBLIC ZIP/manual/checksums, tests contents/version/x64 PE/banks/source
recipe hash. Output /workspace/work/release-verification/092/VERIFIED.json.
After success capture Windows log to docs/audit/BROWSING_SAMPLE_GLIDE_092_WINDOWS_TESTS.txt,
copy proof to BROWSING_SAMPLE_GLIDE_092_PUBLISHED_VERIFICATION.json, update README
and handovers COMPLETE, docs-only commit [skip ci] and push, then deliver user ZIP.
Temporary package fixture outside repo is NON-EXECUTABLE; never distribute it.

Workspace /workspace/work/native-design; JUCE /workspace/work/JUCE; CMake/CTest
/workspace/work/build-tools/cmake/data/bin. Native test env DISPLAY=:99 and
XDG_CACHE_HOME=/workspace/work/runtime-cache. Logs /tmp/space092-{native-final,
focused,final-build}.log. Public verifier scripts under /tmp. Preflight helper
/tmp/space092-preflight.py; output /workspace/work/preset-research091/packaging-preflight092.
Owner untracked audition/ and __pycache__ must stay untracked. Generated helper
executables /tmp/{RenderLush,RenderResearch,RenderFactory,BassProbe} were removed
only to reclaim disk; source/evidence preserved. ~400MB free.

Previous verified0.9.1: binary source4f7ee13b6033d4d603014a02b19630425a36610a,
Windows44897593118/job136826462106, docs-only closeout52881b4. All27 Windows tests
PASS554.78s; public ZIP/manual x64/version/hashes and banks verified. Audit evidence
under docs/audit/STUDIO_RESEARCH_091_*. Installer ZIP SHA c1f3d109c165f6af44941542db63e6f88e8b5cb4eca6725d79a66afac6f6c76c.
Research/calibrated bank evidence /workspace/work/preset-research091 preserved.
No subagents used. User authorization includes repository updates/build delivery.
