# Additional sampled instruments

Eight choir textures derive from CC0 solo vocal recordings made for SpectMorph (Claudia, Mirko and Sven). These are six-layer choir designs from the original singer recordings, not recordings of eight separate choirs. The other eight sounds derive from Versilian Community Sample Library recordings. All added source recordings are CC0-1.0.

SpectMorph sample origins/licence: https://github.com/swesterfeld/spectmorph-instruments/blob/master/README.md
VCSL permission, including commercial sampler software: https://versilian-studios.com/vcsl/
CC0 dedication: https://creativecommons.org/publicdomain/zero/1.0/

| Included sample | MIDI root | Source |
|---|---:|---|
| Soprano Ah Choir | 60 | https://github.com/swesterfeld/spectmorph-instruments/blob/master/claudia-ah/claudia-ah-1-60.flac |
| Soprano Ih Choir | 60 | https://github.com/swesterfeld/spectmorph-instruments/blob/master/claudia-ih/claudia-ih-2-60.flac |
| Soprano Oh Choir | 60 | https://github.com/swesterfeld/spectmorph-instruments/blob/master/claudia-oh/claudia-oh-1-60.flac |
| Tenor Ah Choir | 48 | https://github.com/swesterfeld/spectmorph-instruments/blob/master/mirko-ah/mirko-ah-48.flac |
| Tenor Oh Choir | 48 | https://github.com/swesterfeld/spectmorph-instruments/blob/master/mirko-oh/mirko-oh-48.flac |
| Bass Ah Choir | 49 | https://github.com/swesterfeld/spectmorph-instruments/blob/master/sven-ah/Sven-a-49.flac |
| Bass Ih Choir | 48 | https://github.com/swesterfeld/spectmorph-instruments/blob/master/sven-ih/Sven-i-48.flac |
| Bass Oh Choir | 48 | https://github.com/swesterfeld/spectmorph-instruments/blob/master/sven-oh/Sven-o-48.flac |
| Vibraphone | 84 | https://github.com/sgossner/VCSL/blob/master/Idiophones/Struck Idiophones/Vibraphone/Soft Mallets/Vibes_soft_C5_v1_rr1_Main.wav |
| Glockenspiel | 84 | https://github.com/sgossner/VCSL/blob/master/Idiophones/Struck Idiophones/Glockenspiel/glock_soft_C5_02.wav |
| Marimba | 72 | https://github.com/sgossner/VCSL/blob/master/Idiophones/Struck Idiophones/Marimba/Marimba_hit_Outrigger_C4_med_01.wav |
| Lute Pluck | 72 | https://github.com/sgossner/VCSL/blob/master/Chordophones/Zithers/Harpsichord, English/Sustains/Lute/ZuckermannKitHarpsi_Lute_Sus_C4_rr1.wav |
| FM Clavi | 72 | https://github.com/sgossner/VCSL/blob/master/Electrophones/TX81Z/Clavisynth/Clavisynth_C3_vl2.wav |
| FM Piano | 60 | https://github.com/sgossner/VCSL/blob/master/Electrophones/TX81Z/FM Piano/FMPiano_C4_vl1.wav |
| Bowed Psaltery | 84 | https://github.com/sgossner/VCSL/blob/master/Chordophones/Zithers/Psaltery, Bowed and Plucked/Spiccato/BowedPsaltery_C5_Main_Spic_rr1.wav |
| Recorder Stab | 72 | https://github.com/sgossner/VCSL/blob/master/Aerophones/Edge-blown Aerophones/Baroque Alto Recorder/Staccato/AltRecorder_Stac_C4_rr1_Main.wav |

Processing and original/output SHA-256 hashes are recorded in SAMPLE_PROVENANCE.json. All new WAVs are 44.1 kHz, 24-bit mono. Choirs are 2.6 seconds; other sounds retain up to four seconds. Existing two choir samples retain their original credits.

## 0.5.0 recorded piano and string multisamples

42 additional stereo pitch/velocity zones derive from Sam Gossner / Versilian Studios' **VCSL** and **VSCO 2 Community Edition**, both released under CC0-1.0. They provide grand and upright piano, violin and cello sections, spiccato and pizzicato strings. The original recordings may be redistributed commercially, including within a sampler plugin.

- VCSL: https://github.com/sgossner/VCSL — https://github.com/sgossner/VCSL/blob/master/LICENSE
- VSCO 2 CE: https://versilian-studios.com/vsco-community/ — https://github.com/sgossner/VSCO-2-CE/blob/master/LICENSE
- Pinned source commits, paths, Git blob hashes, source/output SHA-256 and effective MIDI roots are in **TRANCE_SAMPLE_PROVENANCE.json** in the installer ZIP (repository: `Samples/Trance/PROVENANCE.json`).

Processing: onset trimming, DC removal, brief edge fades, stereo 44.1 kHz PCM16, up to seven seconds for pianos and 4.5 seconds for sustained strings. Velocity-layer gains retain dynamic contrast. Sustain loops and per-note nearest-key/layer selection run inside the synth. Libraries use different octave conventions; the effective MIDI roots in the manifest take precedence over filenames.
