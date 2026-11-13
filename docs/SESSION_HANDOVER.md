# 0.6.0 release candidate — Windows publication pending

All requested remaining synth work implemented: 14-mode distortion with pre/post filtering and X-shapers, revised acid/air/choir/lead voices, optional LEVEL MATCH, measured static loudness trims, Analogue/Digital mode, novice macro defaults, ±24 spring wheel, bass LFO drag activation, duplicate engine controls removed, classic gates/block UI, brighter purple/pink-teal displays/logo meter. Preset name/creator/tags, tag search/filter, metadata-only Save Details, local factory replacement/backups/export ZIP, import-factory-edits.py and hybrid creation are implemented. No user confirmation is needed for the already-authorized git push/CI/release verification.

Native 17/17 CTests passed on final tuned/normalised bank (/tmp/ctest060-release.log). Final counter-width and imported-override metadata corrections passed targeted Creative/Preset checks (2/2) via shell34015, logs /tmp/build060-import-metadata.log and /tmp/test060-import-metadata.log. No further sound edits are needed. All160 actual final auditions audited: -29.60..-27.91 LUFS, spread1.69LU, chord raw peak0.7800004, docs/audit/TRANCE_LOUDNESS_060.json. Source /workspace/work/levels-060-final-{solo,chord}/bank-profile.json and Previews. 9 choir zones tuned before layering; final steady fundamental within~8cents,51 total multisample zones,30 source choices,1102 parameters. Python importer tests passed. MAIN/FX native screenshots reviewed and copied into docs/images/creative-*-060.png.

Release candidate fe70c8841d450d46e8b0c0b31d88c522616fc93f was pushed. Its Windows run43456475142 was deliberately cancelled before publication to correct hybrid preview gain: hybrid now keeps parent A gain and calibrated trim minus6dB instead of fixed -6dB. Quiet-source choir hybrid audio regression plus Creative/Preset/Studio targeted suites passed3/3 (/tmp/test060-hybrid-level.log). Factory audio and final loudness audit are unchanged.

NEXT: commit/push this small hybrid-level correction, then monitor the new Windows workflow; all17 suites plus importer test/install/uninstall/publication must pass. Download actual0.6.0 release ZIP +VST3/checksums using gh release download (do not retry the previously denied Azure Actions-artifact route). Independent verifier prepared at /workspace/work/workflow-update/verify-v060-download.py, expects1102 parameters,30 sources andall51WAVs plus exact bank/logo/docs. It tolerates the SDK moduleinfo trailing commas. Release URL will be https://github.com/vegadubstep-star/Space-Infinity/releases/download/v0.6.0/SPACE_INFINITY_0.6.0_Windows_x64_Installer.zip . Do NOT call installer ready until actual download verification completes. Final handover-only commit can use [skip ci]. Last verified shipped release is0.5.0 below.

No subagents authorized. Do not modify production sources while a native/Windows build is running. User requested to continue through all remaining tasks without stopping. Finish publication and downloaded-byte verification, then send concise ready-to-test link/features; don't merely offer to continue.

## Historical release evidence

# Verified release — SPACE INFINITY 0.5.0

Published and ready for the user's DAW test. [Installer ZIP](https://github.com/vegadubstep-star/Space-Infinity/releases/download/v0.5.0/SPACE_INFINITY_0.5.0_Windows_x64_Installer.zip). Source: `4a2957a2eeac8a20126bb4b3b2f03c6a4580501d`. [Windows workflow 43414906507](https://github.com/vegadubstep-star/Space-Infinity/actions/runs/43414906507) completed successfully, including all 15 native suites, install/uninstall, archive verification, publication and optional artifacts/cache.

Downloaded the actual published installer ZIP and VST3 archive. Verified ZIP integrity, published ZIP/Setup/VST3 hashes, x64 PE, version 0.5.0, exact embedded factory JSON, **all 42 WAV zones byte-for-byte**, approved logo, packaged research/catalogue/provenance and the 1,068-parameter inventory (27 source choices). The VST3 SDK's generated moduleinfo.json has trailing commas; the independent verifier accepts that SDK syntax while checking version/identity. No release files were altered to pass verification.

- Installer ZIP SHA-256: `88b4917e15bc4c94bab0d25a1e16c6423366d91b034df3355eb86ccffed1ba4e`
- Setup SHA-256: `53c476ae821486b08c5db4f4e7eb4aea495b714f7c176d0cfeee648f7f29260d`
- VST3 ZIP SHA-256: `13786b45125789486cf7dc4d3c907237bfe973da48d8bb44a1fa1032adfa427c`

Evidence: `/workspace/work/release-verification/v0.5.0/DOWNLOAD_VERIFIED.json`; local native audit and 160 six-second audition WAVs: `/workspace/work/trance-research/`. Do not commit the untracked `audition/` or Python cache directory. These are track-inspired authored sounds, not artist originals or proven identical replicas; the user's listening test remains necessary. Close FL Studio, Extract All, run Setup.exe, then load a fresh instance.

## Implementation handover

User requested a complete trance/90s German hard-trance factory replacement and explicitly authorised importing online samples. Exact requested counts total **160**, corrected from an earlier arithmetic error of 170. Built 160 new sounds plus Init; personal user presets and saved project parameter states are retained. New factory generation F1000 retires old factory entries.

Added 42 CC0 stereo pitch/velocity zones (VCSL / VSCO 2 CE): grand, upright, violin/cello sections, spiccato/pizzicato. Native per-note key/layer selection, stereo playback and crossfaded sustain loops. Pinned source/output hashes and roots in `native-vst3/Samples/Trance/PROVENANCE.json`. Research, catalogue and validation are in `docs/TRANCE_FACTORY_BANK*.md`, `docs/TRANCE_PRESET_CATALOGUE.md`, `docs/audit/TRANCE_BANK_VALIDATION.md`. Blueprint, sample preparation and measured calibration scripts are committed. No copyrighted record samples, commercial ROM/library rips or song MIDI.

All 15 local native suites passed, including bank headroom, dry unsequenced basses, arp stopping, sample loops, personal-bank migration and six 24-note fade checks. Full profile output/160 audition WAVs are outside Git at `/workspace/work/trance-research/`; no audition directory should be committed. Version 0.5.0 has now passed Windows publication and actual download verification above.

## Earlier verified release

# Verified hotfix — SPACE INFINITY 0.4.2

Published and ready. [Installer ZIP](https://github.com/vegadubstep-star/Space-Infinity/releases/download/v0.4.2/SPACE_INFINITY_0.4.2_Windows_x64_Installer.zip). Release source: 8486d987246b0f7f641ca82ddbe8b5fc6442d8fa. Windows workflow: 43323636209. Native tests (all 15), installer/archive checks and publication passed. Actual release ZIP/VST3 downloaded: integrity, published hashes, embedded Setup hash, Windows x64 PE, approved logo master bytes, packaged notes and 1,068-parameter inventory/version 0.4.2 verified.

User reported only the logo's top visible on Windows. JUCE Direct2D's alpha-mask path used the parent bitmap of a cropped image without its area offset. LogoArtwork.h now physically copies coverage to a zero-origin software mask and bakes complete 4x RGBA metal/shadow/light images. Window painting draws ordinary fitted images with no cropped alpha-mask clipping. Approved artwork and reactive purple light are retained.

Local source-offset/lower-stroke regression checks passed at 100/125/150/200/300/400%. Windows HardwareUI now creates native windows for every available rendering engine at 100/125/150/200% app scale and compares lower logo strokes against the complete render; that check passed. Optional native screenshot artifact download was denied by the environment's Azure Blob proxy (403); do not claim manual inspection of those PNGs. The automated native capture comparison passed, and actual installer downloads were verified independently. Do not retry the denied Blob route without a legitimate environment-policy change.

Close FL Studio completely before installing, then load a fresh instance. 0.4.1 is not the clipping fix. Local evidence: /workspace/work/workflow-update/v042-logo-fit-tests.log; release verification: /workspace/work/release-verification/v0.4.2/.

- Installer ZIP SHA-256: ffee8f489f10f49c54e692e0996e1ef6b16ec0ee7282bd363d9453a966d795cc
- Setup SHA-256: 60e5220513417f12f0541abc7d66969804cea3d0a8c066ca54d1724759c27136
- VST3 ZIP SHA-256: b61c2d95fcbd6a6ec44a07acf06c77c78d8adce7d77aea1603dbe54e6f4090ef

## Previous release history

# Verified release — SPACE INFINITY 0.4.1

**Published and ready to test.**

- [Installer ZIP](https://github.com/vegadubstep-star/Space-Infinity/releases/download/v0.4.1/SPACE_INFINITY_0.4.1_Windows_x64_Installer.zip)
- [Release](https://github.com/vegadubstep-star/Space-Infinity/releases/tag/v0.4.1)
- [Windows workflow](https://github.com/vegadubstep-star/Space-Infinity/actions/runs/43317914127)
- Verified release source: `ef7ee9d64143353a1d897781dd633809318e245d` on `codex/windows-build`.

Windows compilation, all 15 native suites, installation/uninstallation, VST3/archive verification and publication passed. The actual published installer ZIP and VST3 archive were downloaded and verified. ZIP integrity/checksum and contained Setup checksum match published values; the VST3 is Windows x64 PE and contains the exact approved logo master bytes. Packaged update notes match source; the native inventory contains 1,068 unique parameters and version 0.4.1. Optional artifact/cache steps may finish after publication; they do not gate the verified download.

## Latest user requests completed

The user approved the sharper, less busy hard-trance INFINITY wordmark ("CLASS"), then requested output-reactive illumination inside the letters. The approved master is committed at `native-vst3/Assets/InfinityLogo.png`; original generated artwork remains at `/workspace/generated_images/exec-7a814659-2539-4da1-9b3b-672c2198bf47.png`. Runtime luminance coverage removes the black master background, supplies clean anti-aliased letter edges and receives the purple metal finish. The 250 px cap clears the pitch wheel at all four supported sizes. Post-output peak drives purple illumination inside the letter mask, with quick rise/smooth decay. A real-audio UI test verifies lighting and fade to idle when master output mutes held notes.

Retained 0.4 controls: live editable amp/filter envelope graphs on MAIN; simpler few-anchor LFO shapes/bend handles with Grid/Draw in Advanced; Character/pre-filter drive/analogue switch beside the filter; LAB opens component controls; pitch controls moved out of ANALOGUE into PITCH; main PORTAMENTO plus Mono/Legato overlap glide, including sample playback; a vertical automatable Pitch Wheel for FL Studio risers. Active FX output controls now show MUTE at their existing minimum and smoothly silence dry+wet stage output; other dB values/ranges are unchanged. Old saved -24 dB FX output settings now mute. Convolver has five primary controls with details in Advanced; reverb has clearer controls and wet EQ.

FL Studio pitch automation: tweak Pitch Wheel, then Tools > Last tweaked > Create automation clip. 50% is normal pitch, 75% is +12 semitones, 100% is +24. Double-click the wheel to centre. Select Mono or Legato to use PORTAMENTO. Extract All, close FL Studio, run the Setup file, then load a fresh instance.

All prior analogue controls/filters remain. See [update notes](ANALOGUE_AND_PARITY_UPDATE.md) and [Serum manual audit](SERUM2_FEATURE_AUDIT.md). Full Serum parity/proprietary DSP identity is not claimed.

## Verification

- Installer ZIP SHA-256: `3e21eb3565113150ac3a814ee6d32dfddc860a427da3af0909ea140805964e8e`
- Setup SHA-256: `1f0af5af34759e9991a4b94e868c7a5dd81fcc92ce6a44ffdcbd86843ca73492`
- VST3 ZIP SHA-256: `91cd78553e1a98540de71c8c30497113a558baeeb38695e9b966e86d4c0d3588`

Download evidence: `/workspace/work/release-verification/v0.4.1/` (metadata, archives, checksums and DOWNLOAD_VERIFIED.json). Local targeted checks: `/workspace/work/workflow-update/v041-logo-ui-tests.log` and `v041-final-logo-tests.log`. Native screenshots: docs/images/Rhythm-Main.png, Logo-Idle.png and Logo-Active.png.

## Workspace/build

Checkout: /workspace/work/native-design. JUCE 8.0.7: /workspace/work/JUCE. CMake: /workspace/work/build-tools/cmake/data/bin/cmake. Native local build: native-vst3/build-linux. Smoke executable: native-vst3/build-linux/SpaceInfinitySmoke_artefacts/Release/SpaceInfinitySmoke. Use DISPLAY=:99 and XDG_CACHE_HOME=/workspace/work/runtime-cache; local checks used a 1 MiB stack limit. Read the cloud runtime skill on resume. Do not edit production source during compilation. Generated audition/ remains untracked; do not commit the whole directory.

Pushing codex/windows-build triggers Windows build/test/install/publish. Bump the native/installer/package/exporter version for the next release; do not overwrite a verified published release. The publisher uses CI GITHUB_TOKEN (direct workspace upload previously failed); do not print credentials. Download and verify actual release assets before saying a new release is ready.

Previous verified release 0.4.0: source 4ad577fe644f1787bd20631446644de2e1a35889, workflow 43261025047; installer ZIP SHA-256 1cc5156c9f67187ed1c80918b321a3c55db6f50adf81e985bba8a0a0bc652966.
