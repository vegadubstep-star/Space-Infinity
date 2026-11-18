> The 0.8.0 refresh revises both banks and peak calibration; see [the current update](TRACK_READY_080_UPDATE.md). The original release record follows.

# SPACE INFINITY 0.7.0 — Heritage 200

This release also adds HOT RANGE distortion, Tape/Fuzz/Bit Crush and WARM/ACID/FUZZ starting buttons; see [the distortion update](DISTORTION_DRIVE_UPDATE.md).

Adds a separate collection of 200 original presets influenced by the Korg M1,
Roland JP-8000/JP-8080 and Nord Lead 2. Existing 160 factory presets and saved
projects retain their identities. These sounds interpret the instruments' roles
and synthesis approaches; they are not copied ROM programs or exact emulations.

## Install and browse

Extract the Windows installer ZIP, close your DAW and run Setup.exe. Open the
updated synth, open its preset manager, press **IMPORT**, and select
**SPACE_INFINITY_Heritage_200.zip**, supplied next to the installer. Keep this
inner bank ZIP intact. Select **Heritage 200** in the tag filter to see the bank.
Use **M1 Influence**, **JP Influence**, **Nord Influence**, or musical role tags
to narrow the list. Reimporting this collection preserves your edited versions.

| Influence | Presets | Palette |
|---|---:|---|
| M1 | 80 | Piano, electric keys, organs, mallets, bass, pads, strings, voices, textures |
| JP-8000/8080 | 60 | Seven-saw leads, pads, plucks, bass and arps; original feedback/triangle spectra |
| Nord Lead 2 | 60 | FM, pulse, sync/ring spectra, bass, keys, pads, motion and percussion |

**TONE** opens the filter, **CONTOUR** shapes filter articulation, and
**SPECTRUM** moves the custom wavetable where present. Basses follow your MIDI;
only Arps/Motion roles generate notes. Rename, tag, change creator and replace
presets through the existing manager. All analogue filters remain available.

## Sources and level policy

Uses the synth's documented CC0 instrument recordings and 12 newly synthesized
original wavetable families. Custom tables are embedded in portable presets;
built-in multisamples ship inside the plugin. No manufacturer ROM audio is used.
The research/design record is in `docs/research/HERITAGE_200_RESEARCH.md`.

Levels are measured with maximum-velocity single/repeated notes and four-note
chords, then balanced toward a strong output without exhausting measured
headroom. Gain is limited by chord peaks when necessary. The existing final
safety stage is retained. Musical dynamics remain; added effects, macro changes
and denser playing can alter level. Continue watching the output meter while
editing. Technical measurements are not a substitute for your listening test.

The bank ZIP contains a full 200-sound catalogue. Native export/recall, collection
import, preservation of existing factory sounds and reimport of locally edited
sounds are checked during release validation.

Validation: all 20 native test suites pass. All 200 calibrated presets export and
recall; all 200 single-note audition renders were measured. Their highest
four-times-oversampled true peak is -2.51 dBTP, and median integrated loudness is
-20.07 LUFS on the fixed phrase. Full-velocity four-note chord tests stay below
0.89 sample peak; gain boosts stop when the measured headroom budget is reached.
Detailed per-preset results are retained in docs/audit/HERITAGE_200_*.json.
