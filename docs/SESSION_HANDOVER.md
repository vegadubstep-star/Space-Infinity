# Current work — 0.7.1 hardware/timing/noise preview, 17 November 2026

**Owner now requested aesthetic upgrades, clearer timing and repeatable versus free-running noise. Download delivery remains held.** This is the active scope; the earlier SOUND-only direction below is historical. No subagents were authorised or used.

Implemented original brushed-metal silver/violet knob assets, fixed studio lighting, recessed sockets, raised switches, finer striking-purple panels and HiDPI chassis caching. Actual native Main/FX screenshots are saved as docs/images/Hardware-{MAIN,FX}-071.png after final rendering. Approved logo fitting/output response is retained.

Both visible LFOs have tempo-aware cycle duration labels; Hz fields accept durations. Advanced Delay/Rise/Smooth and FX time/frequency parsers convert explicit units. Sub-Hz FX rates retain decimals. Noise has Random/Free/Retrigger: separate continuous generator, fixed seed and colour memory on Retrigger. Init uses Free; omitted legacy field defaults to Random. The new noiseMode parameter is appended, preserving all 1107 prior definitions and indices; native inventory is 1108 parameters/version0.7.1. Existing160 bank and Heritage200 ZIP are unchanged (compatibility evidence in audit/HARDWARE_TIMING_NOISE_071_COMPATIBILITY.json).

Native focused noise/LFO tests and 800/1000/1180/1500 UI bounds passed during implementation. Additional tests check successive bursts, host-tempo readout and actual attached FX/LFO parsers. Final full native suite and Windows preview verification are being completed; replace this status with their actual results before handoff. CMake, installer and package filenames are0.7.1. The Windows workflow now supports publish_release=false and verifies an installer ZIP even when publication is held. Push source with [skip ci], then dispatch this preview mode to avoid auto publication. Do not overwrite the verified0.7.0 assets or describe them as containing this update.

Source/theme details: docs/HARDWARE_TIMING_NOISE_UPDATE.md, scripts/artwork/render-hardware.py, native-vst3/Assets/Hardware/PROVENANCE.json. Native logs in /tmp/hardware-*.log. Build in native-vst3/build-linux with /workspace/work/build-tools/cmake/data/bin/cmake; DISPLAY=:99, XDG_CACHE_HOME=/workspace/work/runtime-cache. Preserve the existing untracked audition/ and script __pycache__/ directories. Four of our own old temporary RenderHeritage helper executables were removed to free disk; source utilities and measured/rendered sound assets remain.

---

# Current handover — 0.7.0, 17 November 2026

**Owner direction: focus on SOUND. Do not send download links yet.** The owner explicitly said “dont send me download yet”, then requested stronger distortion and selected sound over usability changes. Keep installer/audio links held until requested. Simple/Advanced views, collapsible FX, favourites and a simpler MAIN page were discussed as suggestions only; no such redesign was started. Latest questions covered FX features, competitive positioning and the analogue engine. Do not claim superiority over Serum/Diva/Nexus without a fair listening comparison, or exact hardware circuit emulation.

## Completed and verified

Added **Heritage 200** as a separate bank: 80 Korg M1, 60 Roland JP-8000/JP-8080 and 60 Nord Lead 2 influences. The owner explicitly confirmed JP-8000/8080 and ADD rather than replace. Existing 160 factory presets remain. Research includes original Korg/Roland/Nord manuals/specifications; original wavetable synthesis and existing documented CC0 multisamples are used, with no manufacturer ROM copies. See HERITAGE_200_UPDATE.md, research/HERITAGE_200_RESEARCH.md, Presets/Heritage200/Recipes.json and the catalogue. The ZIP is included beside Setup.exe; import the intact bank ZIP through BANK > IMPORT and select its Heritage 200 tag. Full prevalidation, late-conflict rollback, stable IDs, edited reimport preservation and all 200 native recalls pass.

Distortion now has **HOT RANGE** (up to 24 dB additional pre-drive, 60 dB total Drive sweep), 20 characters including Tape/Fuzz/Bit Crush, and WARM/ACID/FUZZ buttons. Input, tone, wet mix, pre/post filter, HQ and output gain/meters remain. Fresh Init enables HOT; host default is off for omitted-field legacy recall. Existing 17 mode indices and their standard DSP response are preserved. Frozen 0.6.3 fixture comparisons have zero sample difference. At a quiet -40 dBFS peak sine, Drive55%/Input+12dB, THD is 2.50% standard vs15.90% HOT; HQ cuts the HOT alias-power fraction from0.09910 to0.004853. No adaptive gain follower was introduced. All20 characters are checked at44.1/48/96kHz with sustained-level and dry-mix checks. Native screenshots fit800/1000pixel widths. See DISTORTION_DRIVE_UPDATE.md.

All200 voices were exported/recalled, rendered for single/retrigger and four-note maximum-velocity chord tests, and balanced toward -20LUFS subject to measured headroom. Thirty voices were revised after a spectral/contour review (58 close pairs reduced to12 source/role variants). H128 Resonant Landing, H160 Acid Ember and H163 Drive Tundra use HOT and were remeasured. The other197 retain standard drive. Actual native single-note auditions have highest four-times-oversampled true peak **-2.51dBTP**, median **-20.07LUFS**; some transient/chord-limited voices are quieter. Native chord sample peaks stay below0.89 (largest about0.801); internal pre-safety peak budget0.94. These checks cover the fixed test configurations, not every possible edited macro/effect setting. Do not claim every preset is peak-normalised to0dB. Detailed results: audit/HERITAGE_200_*.json. Musical listening approval remains the owner's test.

## Source, Windows and publication

Main branch: `codex/windows-build`. Windows production source **86122cd3752f41201bc08ba055c9ae86b14355b6**. [Windows workflow44105099806](https://github.com/vegadubstep-star/Space-Infinity/actions/runs/44105099806), job134115181350, passed all20 suites plus installer install/uninstall and byte identity between installed/archived VST3. Version0.7.0;360 presets plus Init after bank import,1107 parameters,39 sources,60 multisample zones,7 filter types. Existing analogue controls and Space/Energy/Width remain.

Independent readback caught a hardcoded0.6.3 label in the parameter reference export. **3fdf00f81f3697ad4ca34f18e9d76470756bbef6** derives it from JucePlugin_VersionString and corrects the reference JSON. Native re-export matches all1107 definitions; only reference version changes. No production DSP/binary change was needed. The verified Setup/VST3 were reused and the outer installer ZIP repackaged with the corrected0.7.0 reference. Documentation links were repaired too.

Direct client uploads to uploads.github.com returned401 while API reads and Git push continued working. No user reauthentication was requested. Utility branch `codex/release070-publication`, commit **b4d4b7f2658ac6ee74e127a9f1d1f095e7dcda0d**, [workflow44111810292](https://github.com/vegadubstep-star/Space-Infinity/actions/runs/44111810292), job134139136119, used a fresh Actions token to publish the corrected ZIP and audio previews without rebuilding Windows. All34 published asset SHA-256/size values match local bytes. The corrected installer ZIP was downloaded again and independently inspected: x64 PE/version, exact original160 bank, approved logo, all60 WAV zones, current docs/1107-parameter reference,200 bank hashes and both preview ZIP CRCs. Evidence: `/workspace/work/release-verification/v0.7.0/` including DOWNLOAD_VERIFIED.json, PUBLISHED_ASSET_DIGESTS.json, WINDOWS_WORKFLOW.log and PUBLICATION_WORKFLOW.log. Summary copies are in docs/audit/RELEASE_070_*.json.

Verified hashes:
- Installer ZIP: `6dfe33919332b3fdac9cc7b8c8efb81e9c1a7e49d9d0302e8a1cf6f2faa53017`
- Setup: `725229abcabc5e1200ffc4168546ebc2b6d033e416c718781e165054aa710980`
- VST3 ZIP: `f4298866d7637a738c74a54d4ccc4c88d95867b53cfb0e6017ce6cd3ad55f171`
- Heritage200 bank: `6a8254f826a10c278dd4de8095cf65b06f0ae0b1d87cc1a8c9300319a4549247`
- Native200-audition ZIP: `f4a1d4f393ebdbdbd952ca9a5f1e7c4f49155b2ac563b512f7d9f301a178ba28`

Release tag is v0.7.0 and contains the installer, bank, individual native auditions and sound comparisons. **Do not send its download links until the owner asks.** Actual FL Studio listening on the other PC is still the owner's test; do not describe native/Windows smoke checks as FL-host certification.

## Local tools / sound focus

`/workspace/work/heritage200/` holds recipes, final native exports, calibration, verified measurements and200 actual native audition WAVs/MP3s. Source authoring utilities are scripts/heritage/. Native previews are direct renders, not gain-scaled raw WAVs: the nonlinear safety stage runs before2x decimation, so scaling an already decimated raw file is not equivalent. Initial600MB unscaled raw WAVs were removed after calibration to conserve space; measurements remain.

`/workspace/work/sound-focus/` holds an additional actual native comparison generator and renders: same140BPM MIDI/cutoff line through DRY/STANDARD/HOT, and Organ Avenue / Seven Engine / FM Sledge / Acid Ember bass examples. Acid Comparison.mp3 and Bass Characters.mp3 use fixed gain matched to -22LUFS for comparison only, with no extra compression/EQ/saturation. ComparisonLevels.json records the gain. They are packaged in SPACE_INFINITY_0.7.0_Sound_Comparisons.zip and published; links/audio delivery are held. Analogue features were explained as software modelling (driven ZDF/acid filters, drift/thermal variation, voice/filter/envelope variation, VCA saturation/asymmetry/memory/coupling, NEUTRAL/WARM/VINTAGE), not circuit-exact hardware emulation. One FX-list correction was made: convolver has TONE shaping, not a dedicated pitch-tune knob. Delay modes are Legacy, Normal, Ping-pong, Tap -> Delay; do not claim RePitch.

No subagents were used/authorised. Existing untracked audition/ and script __pycache__/ folders were preserved and excluded. Native build: native-vst3/build-linux; CMake/ctest at /workspace/work/build-tools/cmake/data/bin; DISPLAY=:99 and XDG_CACHE_HOME=/workspace/work/runtime-cache. All20 full native suites passed in219s; after the reference-only exporter fix, a fresh native parameter export was compared exactly. Avoid unnecessary reruns until code changes or a new concern warrants them.

---

# Verified release — SPACE INFINITY 0.6.3

Ready for the owner's FL Studio listening test. [Installer ZIP](https://github.com/vegadubstep-star/Space-Infinity/releases/download/v0.6.3/SPACE_INFINITY_0.6.3_Windows_x64_Installer.zip). Close FL Studio, Extract All, run Setup, then select revised factory bass presets. [Presets-only ZIP](https://github.com/vegadubstep-star/Space-Infinity/releases/download/v0.6.3/SPACE_INFINITY_0.6.3_Bass_Presets.zip) also works with0.6.2: extract, BANK > IMPORT, choose individual.sipreset files.

Source commit **54ebb5bc570190fd9e433be7a3dca6e937caa611**. [Windows workflow43536195725](https://github.com/vegadubstep-star/Space-Infinity/actions/runs/43536195725), packagejob132265671662 completed successfully:19/19 native suites, Python importer, installer/uninstaller, binary identity, publication, optional artifacts and cache. Local19/19 and Python importer passed.

All35basses rebuilt (15offbeat,5sub,15rolling), with full-wave oscillator foundations, firmer amp body, shorter filter contours, octave/FM/PWM/organ/acid roles, controlled HQ saturation and mono low end. Tailored Tone/Punch/Character mappings. All125other settings/trims/macros exactly match0.6.2, including its acid fix. No synth DSP changes. Counts1106params,39sources,60zones,7filters,160+Init unchanged. Max raw register peak0.615566/max groove0.476999. Bank loudness -29.60..-27.78LUFS/1.82LU, max raw chordpeak0.7800004. Five sub roots measured41.2Hz atMIDI40. See BASS_VOICING_UPDATE.md and audit/BASS_VOICING_063_VALIDATION.md, BASS_HEADROOM_063.json, BASS_SUB_ROOTS_063.json, TRANCE_LOUDNESS_063.json.

Audio/profile evidence: /workspace/work/levels-063-final-{bass,solo,chord}. Same-MIDI old/new groovedemos and range/groove profiles /workspace/work/bass-063-final, including shared SPACE_INFINITY_0.6.3_Bass_Preview.wav (eight clips,constant gain,40ms fades) and PREVIEW_NAMES.txt. Old bank /workspace/work/Bank-062-before-bass.json. New category-selective normalization preserves unselected rows.

Portable35presets were exported through native processor API and individually reloaded, checking all1106parameter values and144modulation target properties. The0.6.2parameter schema and synth source are identical, establishing compatibility. LocalZIP /workspace/work/SPACE_INFINITY_0.6.3_Bass_Presets.zip. Helper sources /workspace/work/bass063-tools. CLI credential expired just afterWindowsCI, so connected GitHub access completed all remaining work. Publishing utility branch codex/bass-pack-publication-063, commit797fae85eebdaaa98ce60ad6b957937c442e1ff1, [workflow43539148896](https://github.com/vegadubstep-star/Space-Infinity/actions/runs/43539148896) validated and uploaded ZIP+checksum successfully without rebuilding the synth.

Downloaded original Windows artifact14090164153 through connected GitHub tool; artifact SHA314274ee108429148fccf46579d915757686bdc43f84832de4e8d8c4caf6e341. All26 packaged files match GitHub's published release-asset SHA/size metadata, proving byte identity with published downloads. Independent readback checked installer ZIP/Setup hash, x64VST3/version, exact bank, approved logo, all60zones, docs/provenance and1106parameter inventory. The two portable-pack assets also match native-validated bytes and published digests (28verifiedassets total). Evidence /workspace/work/release-verification/v0.6.3/{DOWNLOAD_VERIFIED.json,PUBLISHED_ASSET_DIGESTS.json,WINDOWS_VALIDATION_EXCERPT.log,RELEASE.json}.

- installer_zip_sha256: `915be6675298d2cf4fee24c6815a685c2396ac41dd59235553aef3e094b18c17`
- installer_sha256: `d461da58f23a31646ce01c84280aa19cfb4a96895dbebcb72f842f6a85ca88d5`
- vst3_zip_sha256: `449ad68d0a8b8ced704c4576df63ee79cbee3a0bc0560a366f0fad3ef8b7f18a`
- portable_bass_zip_sha256: `894715f28b20d6e9fd15e2e2966dd1c94d0edf6d3c5d96efb8b323bc4cf7266a`

No pending source or publication work. Musical quality remains the owner's listening evaluation; no exact-artist patch claims. Workspace CLI GH_TOKEN expired; use connected GitHub tools if needed. No subagents authorized. Keep production fixed during builds. Do not commit untracked audition/ or Python caches.

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
