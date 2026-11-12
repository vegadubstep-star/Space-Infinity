# v0.4.0 candidate — 12 November 2026

The analogue/manual-audit update and latest main-page envelope/LFO/FX fixes are complete locally. All **15 native suites passed** at a 1 MiB stack limit. The parameter inventory verifies **1,068 unique IDs**, finite ranges and valid defaults/Init values. Release scripts parse and publisher mocks pass success, failed-upload and wrong-size cases, including audit/inventory ZIP contents and absolute release links.

This candidate preserves the existing analogue/filter controls and adds separate component controls, matched stereo processing, source pan/A4 tuning, Hold/contours/editable envelope graphs, clean base VCO correction and an FX display matching the existing audio order. The logo is smaller and fully visible at all four native QA sizes. See [update notes](ANALOGUE_AND_PARITY_UPDATE.md) and [honest Serum manual comparison](SERUM2_FEATURE_AUDIT.md). Full Serum parity is **not** claimed.

Latest steering also moves Character/pre-filter drive/analogue switch beside the filter, removes pitch from ANALOGUE (new PITCH tab), exposes main PORTAMENTO and vertical automatable Pitch Wheel, and adds Mono overlap glide. All parameter IDs/ranges remain unchanged.

Latest steering: live editable amp/filter graphs are now on MAIN; simpler few-anchor LFO shapes and bend handles with Grid/Draw under Advanced; high-quality logo resampling; smooth full-stage FX mute at the existing output minimum (display MUTE), post-gain meter/clip clear at silence; convolver primary controls and Advanced; clearer reverb controls. Old output values at −24 dB now mute; other values/ranges remain unchanged.

Pending: Windows build, all native suites, installation/uninstallation and archive verification, release publishing, actual installer ZIP/VST3 downloads and hash checks. Do not describe v0.4.0 as ready until these complete. v0.3.0 remains the last verified release, recorded below.

The isolated checkpoint branch was used for local work. The final change is being squashed onto codex/windows-build so the WIP skip-ci commit does not suppress the release workflow. Generated audition/ remains untracked and reproducible. Local evidence: /workspace/work/workflow-update/v04-final-all-tests.log and v04-tail-probe.log. Do not edit production sources during compilation.

---

# Verified release — 12 November 2026

**v0.3.0 is published and ready to test.**

- [Windows installer ZIP](https://github.com/vegadubstep-star/Space-Infinity/releases/download/v0.3.0/SPACE_INFINITY_0.3.0_Windows_x64_Installer.zip)
- [Release page](https://github.com/vegadubstep-star/Space-Infinity/releases/tag/v0.3.0)
- [Successful Windows workflow](https://github.com/vegadubstep-star/Space-Infinity/actions/runs/43201726777)
- Release source: `f52c79723805b2c1faf2dbb435dc3ef87793e7c9` on `codex/windows-build`.

Windows compilation, all **14 native suites**, install/uninstall and VST3 archive verification, publishing and optional artifact/cache steps passed. The actual published installer ZIP and VST3 ZIP were downloaded, integrity-checked and matched their published checksums. The installer inside its ZIP matched the external installer checksum, and the VST3 module was confirmed to be Windows x64 PE. Extract All, close FL Studio, run Setup.exe, reopen FL Studio and test a fresh instance.

- Installer ZIP SHA-256: `9cd9f5ec63c899d023287ea520f6311e7dd9584c010168132e9a176e94bf5f7a`
- Installer SHA-256: `34896ea3f978fc79d5858206e0869baa08f384b14e1054db5a999b895a18119b`
- VST3 ZIP SHA-256: `41a62e28f3e417a33af0d892526ca80e9ae67dd894544368eadec61340d0437b`

This release includes the ten rhythm/metal updates plus two visible LFO editors, free LFO points/bends and timing controls, all six macros with 16 destinations each and target-ring amount dragging, independent WT POS controls, actual 3D frame previews, six original 303/acid waveform families and manual-documented unison controls. See [release notes](RHYTHM_AND_METAL_UPDATE.md) for behavior and exact-parity limitations, and [testing instructions](TESTING_STUDIO_BUILD.md).

The official Serum 2 PDF manual was consulted (oscillators pp. 41–48, LFOs pp. 189–194). Its unpublished detune curves were not presented as exact Xfer values. Normal LFO controls are implemented independently; Path/Chaos/S&H and certain advanced editor gestures are not included. Phase-memory/phase-legato and a second simultaneous warp stage are not included. The release notes make these limitations explicit. User's latest Copilot detune explanation agrees with the core unison-pitch/Blend/modulation behavior; it does not supply a numerical tuning formula.

Local evidence: all 14 suites passed with a 1 MiB stack limit; the last B-position assignment compatibility fix also passed four targeted suites. The previous Windows compiler capture failure was fixed; a superseded run was cancelled before publication. No unresolved build or publication blocker remains. FL Studio listening and host integration remain the user's next checks.

The working checkout is `/workspace/work/native-design`. Generated `audition/` is intentionally untracked and reproducible. Logs are in `/workspace/work/workflow-update/`; downloaded release proof is `/workspace/work/release-verification/v0.3.0/DOWNLOAD_VERIFIED.json`. All source changes are committed and pushed. GitHub builds run remotely; the user's PC need not stay powered on.

---

# Session handover — 11 November 2026

Repository: `vegadubstep-star/Space-Infinity`  
Working branch: `codex/windows-build`  
Native version: **0.2.0**

## Latest status — download now verified

The outage has cleared. Workflow [43104018106](https://github.com/vegadubstep-star/Space-Infinity/actions/runs/43104018106) completed successfully: Windows build, all eleven native suites, installer/archive verification and release publishing.

**v0.2.0 is published and ready to test:**

- [Windows setup](https://github.com/vegadubstep-star/Space-Infinity/releases/download/v0.2.0/SPACE_INFINITY_0.2.0_Windows_x64_Setup.exe)
- [Release page / manual VST3 archive](https://github.com/vegadubstep-star/Space-Infinity/releases/tag/v0.2.0)

Both assets were actually downloaded and matched `SHA256SUMS.txt`. The archive passed ZIP integrity verification and contains a Windows x64 PE VST3 module.

- Setup SHA-256: `dea033f2f92c6b87a7fe926e2718381dd444235bed531de949ae580880b103f4`
- VST3 ZIP SHA-256: `d2e273fcf1726d1ecfdda9348da03920d765b2f2965140d3dff110abe5e39663`

Next: user testing in FL Studio. Close FL Studio before running the setup, reopen and test a fresh instance. The remaining gate/sequencer and visual work is listed below. Earlier outage notes describe the previous shutdown state; the release blocker is now resolved.

## Saved work

- `8deb548`: ten synth control/FX updates.
- `18f6039`: legacy/new unison regression coverage and actual interface screenshots.
- `d7acf15`: publishing fix using versioned GitHub Releases, verified asset sizes/checksums, draft-before-publish, retries, optional bounded artifact uploads and manual workflow dispatch.
- All source changes, documentation and selected screenshots are committed and pushed. The only untracked local directory is generated `audition/` test output; it is reproducible and is not needed to build the synth.

The ten-item batch is described in [TEN_CONTROL_UPDATES.md](TEN_CONTROL_UPDATES.md): oscillator gains; separate oscillator detune; main-page amp/filter envelopes; finer envelope timing; A/B/sub/noise filter routing; main-page sub controls; delay wet EQ; reverb wet EQ; FX gain/metering; smoother stereo flanger. It also contains the consolidated LFO/macros interface and saved modulation routes.

All eleven native suites passed locally. Windows build, native tests and installer installation/uninstallation/archive verification passed on run `43094870299`. That run failed to publish its Actions artifact. Its follow-up upload also stalled; cancellation was requested. Do not present this run as a downloadable v0.2.0 release.

The new publishing script passed local PowerShell syntax and mocked success, failed-upload and mismatched-size scenarios. Actual release publishing still needs confirmation from the queued Windows run.

## External blocker at shutdown

GitHub's public status API reported a **major Actions outage**. The repaired workflow was still **queued** at the final status check. The user's PC does not need to stay on: GitHub-hosted workflows run remotely.

- Repaired workflow: https://github.com/vegadubstep-star/Space-Infinity/actions/runs/43098621648
- GitHub status: https://www.githubstatus.com/
- Expected release, once published: https://github.com/vegadubstep-star/Space-Infinity/releases/tag/v0.2.0
- Expected installer: `SPACE_INFINITY_0.2.0_Windows_x64_Setup.exe`
- Expected manual archive: `SPACE_INFINITY_0.2.0_Windows_x64_VST3.zip`

No v0.2.0 download was confirmed at shutdown. Do not substitute an older v0.1.0 installer.

## Resume tomorrow

1. Check GitHub status and workflow `43098621648`, then check whether release `v0.2.0` exists and is published rather than a draft.
2. Verify the setup/archive assets are available. Download the installer and `SHA256SUMS.txt` and compare the SHA-256 checksum before telling the user it is ready.
3. If publishing failed, inspect that run's logs and repair/retry the publishing step. The primary download now uses GitHub Releases; optional Actions artifacts must not block it.
4. Send the verified download link. Ask the user to close FL Studio, install, reopen and test in a fresh project. See [TESTING_STUDIO_BUILD.md](TESTING_STUDIO_BUILD.md).
5. Prioritize listening for clicks and fading with repeated notes, plus main-page envelopes, routing/sub, both oscillator gains/detune and wet EQ/FX meters. Host-side FL Studio testing has not been performed here.

## Still outside this completed batch

- Finish the LFOTool-style gate editor and separate note-sequencer presentation.
- Further vivid purple-metal material and logo-fit redesign.
- Continue checking the reported clicks across presets and the usability/coverage of modulation dragging. De-click protection is implemented, but no claim that every click is eliminated has been made.

Preserve user presets and legacy parameter IDs/algorithm compatibility. User testing is the next priority once a real installer download is available.
