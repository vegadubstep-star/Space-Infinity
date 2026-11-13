# Trance bank validation — 0.5.0

160 independently authored designs, checked against the exact requested 14 category counts; Init remains separate. The committed JSON and level-calibration map are reproducible from `scripts/design-trance-bank.py` using the committed native parameter inventory. Blueprint hashes prevent an obsolete calibration from being applied after a design change.

Local native verification: all **15 CTest suites passed**, including:

- Every new patch produces finite, audible audio; factory level/headroom checks use actual rendered output, with a three-note chord for polyphonic patches.
- All 35 basses have pattern, gate and pump disabled; no generated bass note-ons. Low-end energy survives mono summing.
- All 20 arp/sequence patches generate notes and stop generating after MIDI release.
- Six representative patches undergo 24 fresh-note retriggers, comparing average energy after settling with the last eight phrases to detect progressive fading.
- All six new sampled instruments play across three key zones. Violin/cello sustains remain audible across repeated loop boundaries, with a continuity check.
- Granular source audio survives preset state recall and is cleared when a different factory patch is loaded.
- Old factory bank entries retire; personal preset files, chosen-folder export, project state, stable-key navigation and cross-instance conflict handling remain tested.
- Existing analogue, modulation, effects, envelope, pitch/glide, UI and Windows logo checks remain in the suite.

A separate six-second-per-patch native profiling command records peak, RMS, pre-final-saturator peak, mono ratio, maximum sample step, late/early energy and generated-note activity. It can export original held-note/chord audition WAVs without song MIDI:

```
SpaceInfinitySmoke --trance-bank-profile /absolute/path/bank-profile.json
```

A single late/early ratio is affected by unison phase and envelope shape; it is not an acoustic similarity score. The 24-note test uses multi-phrase averages. Automated verification establishes signal behaviour and catches regressions; it does not establish subjective professional quality or an exact artist-patch match. Final DAW listening is still the user's testing step.

42 imported WAV output hashes match their CC0 provenance manifest. The installer packaging fixture includes the catalogue, research notes and detailed new sample provenance. Windows installation/uninstallation and actual released binary/hash verification are required separately before declaring a downloadable release ready.
