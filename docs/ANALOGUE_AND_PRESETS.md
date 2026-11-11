# Analogue engine and preset bank update

## Engine
- Fixed 2x internal sample rate with JUCE polyphase IIR downsampling. Host latency is reported. This also increases CPU demand.
- Continuous oscillator phases are retained through note retriggers and advance while voices are idle.
- Stable per-voice oscillator tuning, filter calibration, envelope timing and gain differences; the existing Character control sets variation depth.
- Finite-duration capacitor-style amp and filter envelopes. Attack/release remain capped at one second; envelope displays show the active curves.
- Level-dependent drive into the ZDF ladder; consistent resonance modulation in both stereo channels.
- The Analogue panel enables the new voice/envelope behaviour. Missing parameters in older project states default to the earlier behaviour, although the global 2x processing still applies.

## Factory bank
All original 104 patches have an explicit refinement entry in Source/FactoryRefinements.h. Bass patches use centred mono voices and short filter envelopes. Lead voicing and drive are controlled by role; selected brass, vowel, glass and organ patches use the expanded waveforms. Pads receive slow LFO movement and moderate width; choir shots keep restrained character. Two rhythmic patches use the note sequencer. Transitions use one-shot modulation. These are programmed refinements, not a claim of completed human listening/mastering.

## Additional samples and presets
Eight CC0 vocal recording sets from SpectMorph are processed into choir textures; eight additional sampled instruments are selected from VCSL. The factory bank now has 120 presets (104 refined originals plus 16 sampled patches), and 18 built-in sample sources in total. Raw source paths, licence evidence, processing notes and original/output SHA-256 hashes are in Samples/THIRD_PARTY_SAMPLES.md and Samples/SAMPLE_PROVENANCE.json. Instrument roots are set from measured pitch, since source octave naming is not uniform.

## Preset management
BANK opens a persistent, ordered list with Save New, Replace, Rename, Delete, Move Up/Down, A–Z, Restore Factory, Refresh and Import.

Windows default location: %APPDATA%/Orbital Audio/Space Infinity/Presets. User presets are .sipreset files containing parameters and embedded sample, impulse and wavetable data. bank.xml stores stable identities, names, categories and manual order. Browser arrows follow the displayed order. Projects retain a stable preset key, independent of list position.

Factory deletion removes an entry from the user's list; Restore Factory reinstates the factory list and preserves user entries. User deletion first backs up the file in Deleted. Factory sounds cannot be overwritten; Save New makes a user version. File writes are atomic, concurrent writes are serialized with an interprocess lock, and stale instances must Refresh before editing the index.

## Validation
ExtendedCore checks curved-envelope endpoints and calibration stability in addition to the earlier core engines. Native Smoke auditions all 120 presets, checks state/audio/layout, and invokes the extended and preset tests. Preset tests cover saving, cross-instance reopening, replacement, rename, order persistence, stale-write protection, deletion backups, factory restoration and selected-key recall. Windows CI builds VST3/Standalone, validates the installer and publishes actual native screenshots/audio auditions.

Manual listening and FL Studio host testing remain separate checks.
