# Active bass update — 0.6.3 pending Windows publication

All35basses rebuilt in scripts/refine-bass-063.py (15offbeat,5sub,15rolling);125other settings/trims/macros exactly match0.6.2. No synth DSP changes. Clean full-wave oscillator foundations, octave/FM/PWM roles, firmer amp body, shorter filter contours, selective HQ high-passed wet saturation, mono phase-stable low end. Tone/Punch/Character mappings tailored. Pure/triangle/pulse/FM/octave subs. Version0.6.3;1106params,39sources,60zones,7filters,160sounds+Init unchanged. Added19th BassVoicing suite; all19 native suites and Python importer passed. Register/groove raw peak max0.615566, fullbank loudness -29.60..-27.78LUFS/1.82LU, maximum raw chord peak0.7800004. Sub roots measured41.2Hz atMIDI40. See docs/audit/BASS_VOICING_063_VALIDATION.md and BASS_VOICING_UPDATE.md.

Old bank /workspace/work/Bank-062-before-bass.json. Final35held-note WAVs /workspace/work/levels-063-final-bass; complete160mergedreports /workspace/work/levels-063-final-{solo,chord}. Same-MIDI old/new groovedemos and range/groove profiles /workspace/work/bass-063-final. Shared preview SPACE_INFINITY_0.6.3_Bass_Preview.wav and PREVIEW_NAMES.txt there. Rendering helper --bass-audition folder oldBank; register helper --bass-range-profile JSON. MIDI test matches incoming noteOns because internal patternMidi also contains pass-through notes. No engine change was needed for that helper fix. Calibration --category option now preserves unselected rows; all125other presets kept exactly.

Next: commit/push source, Windows19checks/install/publication, download/verify0.6.3release. Keep production fixed during builds, no subagents authorized. GH_TOKEN currently works. Exclude audition/ and Python caches. Musical quality remains owner's listening evaluation, no exact-artist patch claims.

## Previous verified release

# Verified release — SPACE INFINITY 0.6.2

Ready for the owner's FL Studio listening test. [Installer ZIP](https://github.com/vegadubstep-star/Space-Infinity/releases/download/v0.6.2/SPACE_INFINITY_0.6.2_Windows_x64_Installer.zip). Close FL Studio, Extract All, run Setup, then load a revised factory Acid preset. MAIN CUTOFF is the automation target; the filter should read Acid Resonant. Existing saved projects keep their old patch settings until a revised factory sound is loaded.

Source commit **f2784318c5c5a7e99f200c4ee5667867065dcb8b**. [Windows workflow43514138881](https://github.com/vegadubstep-star/Space-Infinity/actions/runs/43514138881), package job132191617043 completed successfully, including all18 native suites, Python importer, installer/uninstaller, binary identity and publication. Optional artifacts and cache completed. Local18/18 suites plus final four affected suites and Python importer passed.

Fixed the reported acid/cutoff interaction: appended Acid Resonant response with internal headroom and voiced nonlinear feedback; smooth positive-envelope headroom avoids upper-limit plateaus. All ten acid presets use useful2.1–3.2oct envelope excursions and rebalanced distortion/dry blend, replacing folded/FM baked tones with saw/square/pulse acid foundations. Original six filters, cutoff ID/units/host mapping retained. Other150 presets' designs/settings/trims exactly match0.6.1. Counts remain160sounds+Init,1106 parameters,39sources,60samplezones; filter choices now7. This is an authored response, not a circuit-exact303 claim.

Small-signal peaks follow180/800/3200Hz at44.1/48/96k; maximum-resonance/drive sweep stability and feedback decay passed. Every actual driven acid patch opens spectrally with cutoff. Accented-note80Hz–16kHz up/down automation rendered for all ten. Per-patch peaks: audit/ACID_SWEEP_062.json. Completebank auditions -29.60..-27.78LUFS,1.82LU spread,max raw chord/automation peak0.7800004 (audit/TRANCE_LOUDNESS_062.json). Source/audition evidence described in audit/ACID_CUTOFF_062_VALIDATION.md. Audio/profile sets /workspace/work/levels-062-final-{solo,chord}; old bank /workspace/work/Bank-061-before-acid-sweep.json. All20 same-MIDI old/new comparisons /workspace/work/acid-cutoff-062-final, including shared SPACE_INFINITY_0.6.2_Acid_Cutoff_Demo.wav (constant audition gain,40ms tail fade). RMS-matched spectrogram docs/images/Acid-Cutoff-Comparison-062.png.

Downloaded all25 actual GitHub Release assets. Every file matches published SHA-256/size metadata and release points to the source commit above. Independent readback checked installer ZIP/Setup hash, x64VST3/version, exact bank, approved logo, all60samplezones, current docs/provenance,1106-parameter inventory and seven filter choices. Evidence /workspace/work/release-verification/v0.6.2/{DOWNLOAD_VERIFIED.json,PUBLISHED_ASSET_DIGESTS.json,WINDOWS_WORKFLOW.log}.

- installer_zip_sha256: `6b34a6d9917b3ec8e6e9c22f01dc36629ce11babe6de14b59203ed30e2317270`
- installer_sha256: `1e7118ab062c3894cd7344fd866daae79644d6b6b4eb09d183522020dfd90e24`
- vst3_zip_sha256: `ed3a41c2ce498cdb60b8a9114533a26f9a7885b621947a647bb6249384427c29`

No pending source/release work. Musical quality in the user's actual project is the next listening evaluation. GH_TOKEN worked throughout; connected GitHub tools are fallback if credentials expire. No subagents authorized. Keep production fixed during builds. Do not commit untracked audition/ or Python caches.

## Previous verified release

# Verified release — SPACE INFINITY 0.6.1

Ready for the owner's FL Studio listening test. [Installer ZIP](https://github.com/vegadubstep-star/Space-Infinity/releases/download/v0.6.1/SPACE_INFINITY_0.6.1_Windows_x64_Installer.zip) and [Choir Stabs ZIP](https://github.com/vegadubstep-star/Space-Infinity/releases/download/v0.6.1/SPACE_INFINITY_0.6.1_Choir_Stabs.zip). Close FL Studio, Extract All, run Setup, then load a fresh instance and select the new factory sounds. The separate pack has ten embedded-sample presets and ten stereo WAV hits.

Source commit **45e7fcc3a440de77bb4fe4e37164e93f907cae7d**. [Windows workflow43495801570](https://github.com/vegadubstep-star/Space-Infinity/actions/runs/43495801570) completed successfully, including all18 native suites, Python importer, installer/uninstaller, binary identity and publication. Both optional artifacts and the build cache completed.

Completed stronger17-mode distortion (Input/Tone/HQ), all10 acids and35 basses revoiced, ten choir stabs using all nine owner uploads, MAIN portamento and audible Studio/Vintage Analogue Lab. 160 presets plus Init;1106 parameters,39 source choices,60 sample zones.56 designs revised and104 retained. Original owner WAVs and provenance retained separately from CC0 materials. See ACID_BASS_AND_CHOIR_UPDATE.md and audit/ACID_BASS_CHOIR_061_VALIDATION.md.

Final native auditions: -29.60..-27.86 LUFS,1.74LU spread,max raw chord peak0.7800004. Evidence docs/audit/TRANCE_LOUDNESS_061.json; full profiles/WAVs /workspace/work/levels-061-final-{solo,chord}. Local native18/18 and Python importer passed.

Downloaded all24 actual GitHub Release assets with working CLI credentials. Every file matches published SHA-256/size metadata. Independent readback verified installer ZIP/Setup hash, x64 PE/version, approved logo, exact bank, all60 sample zones, updated docs/provenance and1106-parameter inventory. The downloaded choir pack was separately decoded/validated:10 embedded-sample presets and10 stereo48k WAV hits. Evidence /workspace/work/release-verification/v0.6.1/{DOWNLOAD_VERIFIED.json,PUBLISHED_ASSET_DIGESTS.json,WINDOWS_WORKFLOW.log}.

- installer_zip_sha256: `13d197d45effb21fbaa3908cd6c045b35cdcfb5d9a79e1e62ba0e93360fe4fe4`
- installer_sha256: `a072f725bb0e94c72ceed1328f5417f46744bc1255babe91b67b95a68c4b0d41`
- vst3_zip_sha256: `ce668c88fdb7f9cfedb4bad97a85e6583d31033f58bbaa00af33a444353fea15`
- choir_pack_sha256: `c370a62b9a7a4b1a14e9b02635e98007a1ec654e2600419d400068236d776465`

No pending source or release work. Musical/DAW listening is the owner's next evaluation. These are authored sounds, not claimed identical artist patches. No subagents authorized. Do not commit untracked audition/ or scripts/__pycache__/ (including tests cache). GH_TOKEN worked throughout publication; connected GitHub tools remain fallback if it expires. Keep production fixed during builds.

## Previous verified release

# Verified release — SPACE INFINITY 0.6.0

Ready for the owner's FL Studio test. [Installer ZIP](https://github.com/vegadubstep-star/Space-Infinity/releases/download/v0.6.0/SPACE_INFINITY_0.6.0_Windows_x64_Installer.zip). Close FL Studio, Extract All, run the Setup executable, then load a fresh plugin instance.

Source commit **3c4c2df1a4633d3b2043e512f7c456dd1900bc6b**. [Windows workflow43458350184](https://github.com/vegadubstep-star/Space-Infinity/actions/runs/43458350184) completed successfully: full17 native suites, Python preset-edit importer, installer/uninstaller and archived/installed binary identity, publication and optional artifacts/cache. Earlier run43456475142 was deliberately cancelled to fix quiet hybrid previews, not a service outage.

All remaining requested work is implemented: dedicated14-mode distortion; remade acid, airy/emotional pads and choir stabs; more varied leads; measured preset levels; LEVEL MATCH switch for opening plucks; overall Analogue/Digital mode; Tone/Punch/Character starting macros; spring ±24 wheel; direct bass/sub LFO drops; duplicate engine controls removed; classic gate timing/block view; brighter purple hardware and pink/teal displays/reactive logo meter. Presets have editable name/creator/custom tags, search/tag filter, metadata-only Save Details, local factory replacement/backups/export ZIP and a developer import script. Hybrid creation preserves parents/assets and keeps parent A's calibrated gain/trim with a6dB safety reduction; its quiet-source choir regression passed.

Actual160 final native auditions: **−29.60..−27.91 LUFS**, spread1.69LU, maximum raw chord peak0.7800004. Results: docs/audit/TRANCE_LOUDNESS_060.json and CREATIVE_WORKFLOW_VALIDATION.md. Audio/profile evidence outside Git: /workspace/work/levels-060-final-{solo,chord}/bank-profile.json and Previews. Nine new CC0 choir zones are tuned before layering; measured steady fundamentals within~8cents. Total51 multisample zones,30 source choices,1102 native parameters.

Downloaded the workflow's published release files through the connected GitHub artifact tool after the workspace CLI credential expired. Every downloaded release file was matched against GitHub's published release-asset SHA-256/size metadata. This proves the installer ZIP is byte-identical to the published download. The independent verifier additionally checked ZIP integrity, Setup hash/header, x64 VST3 PE, SDK version/identity, exact factory JSON, approved logo, all51 WAV zones byte-for-byte, documentation/provenance and the1102-parameter inventory. SDK-generated moduleinfo trailing commas are accepted without modifying the file.

- installer_zip_sha256: `6b5ce75ffc008328fdce0ed5aff248f3319cea4a9e8c3bb54c517519ed5cfbfd`
- installer_sha256: `940b55a44a17a4676eeab0459f65bbf1be2326a2e4ed49ce827c622265852803`
- vst3_zip_sha256: `463a8c28b1dc42931177b5b548ba87dd5a0b8e814b57a30eefe309588c04cb70`

Evidence: /workspace/work/release-verification/v0.6.0/DOWNLOAD_VERIFIED.json and PUBLISHED_ASSET_DIGESTS.json. Source/artifacts are verified; musical/DAW listening is the owner's next test. These are authored track-inspired sounds, not claimed identical artist originals.

Do not commit untracked audition/ or scripts/__pycache__/. No subagents were authorized. Workspace GH_TOKEN expired during CI monitoring; connected GitHub tools remain authenticated and completed publication/readback verification. Use those tools for GitHub reads/writes if the CLI still returns401. No further code changes are pending. Keep production sources fixed during any native/Windows build.

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
