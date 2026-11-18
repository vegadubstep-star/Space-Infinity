# SPACE INFINITY 0.8.0 — sound bank and waveform update

This update expands the oscillator palette and revises the existing 160 factory
sounds plus the separate Heritage 200 collection for trance, hard trance and
Makina. Preset names are original; artist references are research context.

- Fifteen original oscillator families, bringing the list to 52, with editable
  wavetable position and frequency-aware harmonic filtering for the additions.
- Six steady choir and choir/synth hybrid sources, derived from the documented
  CC0 ensembles and original synthesis. No added vibrato or rhythmic modulation.
- Simpler role browsing: Bass, Lead, Pluck, Pads, Strings, Piano & Keys,
  Arps & Sequences, and Choir & Voices. Style tags remain searchable.
- OPEN / LEAD macros on the 32 plucks: filter opening, lower wet delay/reverb,
  more width and restrained drive; synth voices also gain sustain. Individual
  output compensation controls the increase as a pluck becomes a sustained lead.
- Peak calibration uses measured clean headroom with a −0.3 dB true-peak ceiling.
  BS.1770 perceived loudness is measured alongside peaks. Equal peak levels
  cannot make a short pluck and a sustained sound equally loud to the ear.
- Clean headroom output mode passes low-level audio unchanged, with a final
  safety knee after downsampling. Existing saved projects retain their legacy
  output transfer unless changed explicitly. This is not a true-peak limiter.
- LFO shape selectors identify the actual shape or Custom after editing.
  The legacy filter assignment badge stays behind other pages and popup menus.

The Heritage ZIP requires version 0.8.0 or later. Reimporting it refreshes
byte-identical older supplied patches and preserves locally edited patches and
metadata. Backups and rollback protect an interrupted bank update. Install the
plug-in first, then use the preset manager IMPORT button for the bank ZIP.

Level checks use full-velocity repeated notes and chords, the pluck opening
endpoint and all three assignable macros raised together. Different registers,
denser chords, edits and layered instances can change peaks. Keep an eye on
the output meter while editing.

Research and original asset provenance are documented in
[the sound-design notes](research/TRACK_READY_080_RESEARCH.md) and the included
sample provenance files. These sounds interpret musical styles; they are not
manufacturer patches or component-exact replicas.

Install by extracting the installer ZIP, closing your DAW and running the Setup
application. Import the included Heritage 200 ZIP through the preset manager to
refresh that separate bank; the 160 factory presets update with the plug-in.

Validation: all 24 Linux native checks pass, including V2 bank refresh/rollback,
legacy recall, clean output safety, oscillator quality, routing and editor checks.
The 360-preset audio audit has zero failures: maximum measured true peak
−0.295 dBFS (−0.3 dBTP target, 0.01 dB measurement tolerance), and all 32
OPEN/base transitions are within 0.06 LU. Forty-six additional native headroom
references cover bass registers, acid cutoff automation and observed IR startup.
The final default-sound similarity scan has no candidates above both thresholds;
this is an objective screening method, not a guarantee of subjective preference.

[Measured levels](https://github.com/vegadubstep-star/Space-Infinity/blob/v0.8.0/docs/audit/TRACK_READY_080_AUDIO_LEVELS.json) ·
[Native checks](https://github.com/vegadubstep-star/Space-Infinity/blob/v0.8.0/docs/audit/TRACK_READY_080_NATIVE_TESTS.txt)
