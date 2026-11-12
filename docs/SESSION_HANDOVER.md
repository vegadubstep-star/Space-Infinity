# 0.4.2 logo clipping hotfix — Windows validation pending

User reports only the top of the logo is visible in the installed synth. JUCE 8 Direct2D alpha-mask drawing obtains the parent bitmap of a subsection without applying its source area offset. Prior off-screen captures did not exercise that native window path. Removed all on-screen cropped-mask rendering for this badge. LogoArtwork.h now physically copies coverage into a zero-origin software mask and bakes complete 4x RGBA metal/shadow/light tiles; the window draws ordinary fitted images. Approved master artwork is unchanged.

Seven local relevant suites passed, including a large source-offset fixture with lower-stroke checks at 100/125/150/200/300/400% scale. Added actual Windows peer captures for each available rendering engine at 100/125/150/200% app scale, comparing lower logo strokes against the complete render. This Windows-only check must pass before release. Version bumped to 0.4.2; inventory regenerated. Do not give the 0.4.1 installer as the clipping fix.

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
