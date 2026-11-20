# Lush Trance 20 design notes

Uses the documented unison/voice architecture research in
[the factory-bank research](../TRANCE_FACTORY_BANK_RESEARCH.md), especially
[Roland's JP-8000 manual](https://cdn.roland.com/assets/media/pdf/JP-8000.pdf),
alongside the filter references in CHARACTER_FILTER_SAMPLER_090_RESEARCH.md.
The new collection is original programming, not extracted commercial patches.

Twenty explicit recipes vary harmonic source, relative register, unison count,
detune, centre/body balance, filter topology, articulation, Dimension voicing and
oscillator interaction. Saws alone do not define every patch: string resonators,
parabolic/pulse pairs, odd/even spectra, reed/organ and low bell layers give distinct
roles. Vowel and LP/BP/HP filters appear selectively; not every lead is a moving
filter sweep. Sub layers and rhythm generators remain disabled.

Near-zero attacks, high amp sustain and modest spatial returns retain melodic
presence. Independent slow Voice Life avoids periodic amplitude dips. OPEN reduces
delay/reverb while opening tone; BODY gain compensation reduces excessive macro
level jumps. Existing quiet uplifting leads are reviewed separately rather than
raising every lead's output blindly.

Native audition phrases use velocity 127, repeated single notes and four-note
chords. All eight corners of the three user macros are measured. Base and all-open
phrases additionally receive BS.1770 loudness and 4x-resampled true-peak checks.
Peak calibration retains clean headroom instead of hard-clipping the sample peaks.
These measurements establish engineering behaviour; the owner's listening test
still determines whether each patch suits a track.
