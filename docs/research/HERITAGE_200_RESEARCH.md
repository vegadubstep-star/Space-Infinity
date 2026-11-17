# Heritage 200 — research and design record

Owner confirmed JP-8000/JP-8080 and an additional separate 200-preset bank. Existing 160 factory sounds must remain unchanged. Focus: character, not additional loudness. Working allocation: 80 workstation/M1-influenced,60 JP-influenced,60 Nord Lead 2-influenced.

## Primary references checked

- [Korg original M1 manual](https://www.korg.com/us/support/download/manual/1/139/1685/) and [original M1 description](https://www.korg.com/de/products/synthesizers/m1/). PCM-based AI synthesis, digital non-resonant 12 dB filtering, separate pitch/filter/amp contours and effects distinguish this from an analogue supersaw. Piano16', Organ2, Universe and Lore are research reference roles, not names for copied samples. Original M1 and modern Collection features must not be conflated: do not add analogue resonance everywhere and call it an original M1 response.
- [Korg Collection legacy architecture](https://www.korg.com/us/products/software/korg_collection/legacy.php) and [M1V2 overview](https://www.korg.com/us/products/software/kc_m1/). Program/combination layers and source attacks matter. New samples will be owner-provided, already documented CC0 recordings, or newly synthesized sources, not ripped manufacturer ROMs.
- [Roland JP-8000 brochure](https://cdn.roland.com/assets/media/pdf/JP-8000.pdf), [official JP-8000 overview](https://www.roland.com/uk/products/jp-8000/), [manual index](https://www.roland.com/global/support/by_product/jp-8000/owners_manuals/). Seven-saw Supersaw, Triangle Mod and Feedback OSC distinguish it from simply making every patch a detuned saw. Include restrained mono/pulse/bass voices, not only wide anthems. JP-8080 influence must not imply an implemented vocoder unless one actually exists.
- [Nord Lead 2 original specifications](https://www.nordkeyboards.com/legacy-products/nord-lead-2/) and [manual/factory-bank index](https://www.nordkeyboards.com/legacy-products/nord-lead-2/downloads/). This is Lead 2, not original Nord Lead running OS2. Sine/saw/pulse/triangle, deep oscillator FM, hard sync, syncable noise, filter drive, velocity interaction, four performance slots and analogue percussion are the important palette distinctions. Its dry articulation should contrast with JP ensembles. Manual PDF redirect was expired in web fetch; official specifications were successfully read.

## Translation into Infinity

Retain all analogue filters. Use non-resonant two-pole filtering and sampled/multisampled attacks for workstation voices; carefully mixed digital harmonic sources for organ, glass, bell and atmospheric roles. JP ensembles use seven-voice unison with centre/side balance and separate body layers; feedback/triangle-mod interpretations need original waveform families. Nord FM is supported directly. Existing Mirror warp is NOT true hard sync; original sync/ring spectra can be authored as evolving custom wavetables, or the engine can be extended and tested. Never label a mirror-wave preset an exact sync reproduction.

M1 combinations and Nord four-slot performances exceed the current independent-source architecture. Where layering is essential, use documented original PCM composites or explicitly describe a reduced-layer interpretation. Do not claim exact hardware emulation. Make differences audible in source, articulation, register and performance response. Basses remain MIDI-played; only specifically tagged arps/sequences generate notes.

## Completion gates

200 unique named recipes, creator/family/role tags, source provenance, category counts; native export/reload with embedded imported assets; loudness/headroom including chords, note endings and repeated notes; waveform/timbre duplicate review; individual audition renders and representative musical previews. Separate bank installation must be practical (one bank ZIP import rather than200 manual file selections). Keep the existing160 factory bank and saved-state compatibility intact. Listening approval remains the owner’s audition step; measured validation is recorded alongside the bank.

## Manual follow-up

The original M1 PDF was inspected with local OCR because it is a scan. Printed
pages 7, 24–25 and 41 confirm Multisound → VDF → VDA, single/double program
structures, independent pitch/filter contours with velocity sensitivity and wet-only
EQ in the early-reflection group. Infinity uses its own filter ADSR rather than
claiming the M1's additional break/slope stages. Existing CC0 piano, mallet,
string and voice attacks supply the workstation roles; original harmonic tables
supply digital organ, glass and atmospheric roles.

The [JP-8000 owner's manual, oscillator section](https://manualzz.com/doc/12034119/roland-jp-8000-synthesizer-owner-s-manual)
was read as a text mirror: printed pages 55–57 distinguish centre/detuned voice
mix, triangle overtone shaping, monophonic feedback, pulse width and forced
oscillator reset. Infinity's seven-voice ensemble and blend implement the first
idea; original feedback/triangle spectra interpret the next two. Feedback-derived
poly pads deliberately expand that palette. They do not claim identical hardware
feedback behaviour.

The [Nord Lead 2 manual mirror](https://manualzz.com/doc/1796401/nord-lead-2-user-manual)
was read after the official PDF redirect failed. Printed pages 26–29 and 40–41
describe velocity/morph ranges, eight percussion zones, oscillator FM, multiplication
for ring modulation and oscillator reset for sync. The new bank includes dry
velocity-sensitive FM keys and bass, original ring/sync spectral tables and eight
separate percussion presets. It does not implement a Nord eight-zone kit or its
four-slot performance architecture. Macro assignments offer Infinity's own
performance controls rather than pretending to reproduce the Nord morph system.
