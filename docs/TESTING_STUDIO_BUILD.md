# Installing and testing SPACE INFINITY 0.9.4

Close FL Studio and other plug-in hosts. Right-click the installer ZIP, choose
Extract All, then run SPACE_INFINITY_0.9.4_Windows_x64_Setup.exe from the extracted
folder. Reopen FL Studio and load a fresh SPACE INFINITY instance. The plug-in
identity and user preset storage are preserved.

## Preset browsing, sample glide and edit panels

Click a sound once in BANK to load it. The header arrows move to the adjacent
sound in the saved/manual order shown in the preset selector, starting from the
last loaded sound (including project recall). Search/tag changes alone do not
change the loaded sound.

On a sample, adjusting PORTAMENTO from Poly selects Mono. Mono glides between
separated notes as well as overlaps; Legato glides only on overlapping notes.
Zero portamento gives an immediate pitch change. Prepare/host reset, preset
loads and All Notes Off clear the remembered pitch. A pause of at least 250 ms
(or twice the chosen glide time) clears it too. The displayed time is the full
glide duration; use Shift for fine adjustment and double-click to type ms. No sample loop is forced:
enable Loop yourself if a short one-shot needs to sustain.

Open granular EDIT, then BANK, the preset dropdown, or another page. The edit
popup should close. Sample-loop and filter EDIT use the same popup lifecycle.

## Try the revised factory sounds

Select the **Research 091** or **Studio Research** tag in BANK. The retained sounds from the earlier
90-preset research revision include basses, hard-trance leads, plucks, airy pads
and acids. No import is needed. Stable IDs remain available for old project
states; the normal palette is now the curated 300. User replacements stay in your library.

On plucks try OPEN / LEAD: the filter and sustain open while delay/reverb returns
reduce, with measured compensation. On acids try SWEEP, LIQUID and CORROSION;
overlapping MIDI notes slide, separated notes retrigger the filter contour.
Rolling basses follow your MIDI and do not generate their own pattern.

## Curated installed bank

All 300 curated sounds plus Init load automatically, including retained sampled
choir, lead and bass presets. No import is required. Choose Bass, Lead, Pluck,
Pads or another category in BANK; tags and search can narrow it further. CLEAR
resets all filters. Enter after a search loads the first result; Escape clears
filters. Click a sound to load, or use list arrows then Enter. EDIT / SAVE opens
the metadata, save/replace and hybrid controls.

The main palette excludes 121 further close alternatives from the previous
421-sound list. Personal replacements are preserved. The optional
SPACE_INFINITY_Curated_300_Alternatives.zip recovers those alternatives into a
separate bank; it is not needed for normal use.

Retained sounds receive static loudness matching separate from their macro
compensation, after their current effects and sample-start settings are rendered.
There is no automatic gain ride. Chords, three registers and all six macros are
also checked for clipping. MIDI, octave, velocity and edits change the result.
See audit/INSTALLED_BANK_094_LEVELS.json and the CURATED_300 audits.

## Included banks and new samples

Heritage, Lush Trance and Orbit Voices sources are included automatically on
first load. The curated view exposes the selected 300 sounds; extra close
alternatives stay hidden. No manual import is needed.
Their ZIPs remain in the installer folder as portable backups:
SPACE_INFINITY_Heritage_200.zip, SPACE_INFINITY_Lush_Trance_20.zip and
SPACE_INFINITY_Orbit_Voices.zip. Edited local versions are preserved.

The Orbit Voices backup contains 12 choir variations, 21 leads and 15 basses.
The normal curated list retains 38 of these sampled sounds, made from the
40 uploaded WAVs, with INFINITY filenames, root pitches and OPEN/SPACE/BODY
macros. Seven measured near-duplicate Heritage variants are hidden from the
standard list; SPACE_INFINITY_Retired_093_Backup.zip restores them as a separate
optional bank. Existing projects still recall their original sound.

In BANK, select or move sounds into the order you want. Header arrows follow
that saved order, starting at the last selected preset. A–Z sorts only when you
choose it; selecting a sound does not sort the bank.

Use IMPORT CLEAN to hear an external sample with its stereo image and bass
preserved, without the previous preset's filter, effects or gain settings.
It resets the current patch to a clean sampled instrument. Save work first.
For original playback use the sample's root note at full velocity. Output safety
can reduce peaks above its ceiling; avoid clipped source audio.

Enable + OSC B beside the sample to add a complete oscillator layer. Its octave,
fine pitch, wavetable, unison, detune, warp, phase, pan, gain and filter routing
are available on the right. GRAINS keeps the granular controls accessible.
The independent SAMPLE GAIN changes the sample without changing oscillator B.

## Try the new controls

- Filter: choose Formant vowels, Notch, Comb or a steep HP/BP model in the MAIN
  filter selector. EDIT opens its larger response graph and vowel/morph knobs.
  Drag the graph horizontally for cutoff and vertically for resonance.
- Sampler: choose a sampled source. Use LOOP, then try 20–80 ms LOOP FADE.
  Raise GRAIN MIX gradually; SIZE, POSITION, SPRAY and WIDTH shape the blend.
  DIFFUSE softens the texture; WARMTH darkens it. FREEZE holds the grain centre;
  keep Loop enabled to sustain a one-shot sample.
- The octave fields above the sampled source and live display now adjust sampler
  pitch and the granular layer respectively (oscillator B when + OSC B is on). The granular octave is relative to
  sampler octave; Master Octave and the pitch wheel still apply globally.
- LIFE adds independent pitch/timbre variation without an amplitude LFO.
- A > B: enable both oscillators, open A > B, choose direction, raise Amount to
  100% and set Time/Curve. Each note starts its own transition. The amp envelope
  still determines note length; releasing early can end it before the transition.
- Both envelope plots and LFOs remain on MAIN. Hold/Decay changes must update
  their plots. Drag LFO/AMP ENV/macros to supported knobs and adjust the coloured
  destination ring; Shift gives fine adjustment. Right-click offers amount,
  invert and remove. Pitch wheel release returns to zero with separate up/down stops.

The new lead levels are checked using full-velocity repeated notes, four-note
chords and macro extremes, with true-peak headroom. Denser chords, added effects
and edits can change the level; use the meters while adjusting your sound.

[0.9.1 preset changes](STUDIO_RESEARCH_091_UPDATE.md) ·
[0.9.0 changes](CHARACTER_FILTER_SAMPLER_090_UPDATE.md) ·
[0.8.0 sound-bank history](TRACK_READY_080_UPDATE.md)
