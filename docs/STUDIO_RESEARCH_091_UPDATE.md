# SPACE INFINITY 0.9.1 — Studio Research presets

This revision refines ninety existing factory sounds using original programming
informed by manufacturer documentation and practical sound-design walkthroughs.
The factory collection still contains 160 sounds. Its IDs and names stay stable.

- **30 basses:** individual filter and amp contours, stable centred sine weight,
  more distinct pulse/reed/FM/drawbar colours and short rolling-bass tails.
  Play your own notes; these sounds do not generate a sequence.
- **20 hard-trance leads:** firmer sustained body, fast attacks, selected
  envelope-shaped metallic transients and cleaner spatial returns.
- **20 plucks:** more meaningful filter articulation, individual delay spacing,
  and an OPEN / LEAD macro that opens the tone while reducing wet effects.
  Synth plucks acquire sustain; sampled instruments retain their natural decay.
- **10 airy pads:** longer swells and releases, gentle harmonic motion and
  spacious effects with a clearer low end. No amplitude LFO was added.
- **10 acids:** distinct waveform/filter/drive combinations, velocity accents,
  and legato glide. SWEEP opens cutoff, LIQUID shapes the filter gesture,
  CORROSION adds drive. Overlap notes to slide; separate them to retrigger.

The other seventy factory recipes, Heritage 200 and Lush Trance 20 are retained.
Your saved preset edits remain in your user library. A locally replaced factory
sound may continue to override its updated factory version; save a copy of your
edit before restoring factory sounds if you want to compare.

## Try the revised sounds

Close FL Studio, extract the installer ZIP and run Setup. Reopen the synth and
filter the preset list using **Research 091** or **Studio Research**, then try
your own MIDI. No additional bank import is needed for these ninety revisions.
The existing optional Heritage 200 and Lush Trance 20 ZIPs still import normally.

[Research notes](research/STUDIO_RESEARCH_091.md) map the sources to the design
decisions. Numerical settings are Infinity programming choices, not original
artist patch sheets or exact emulations of the named instruments.

## Level checks

Native rendering checks articulated single notes, four-note chords, all eight
user-macro corners and all six macros raised. Additional probes cover three
registers, acid cutoff sweeps, pluck OPEN transitions and overlapping pad chords
in both Analogue and Digital modes. Bass checks also cover MIDI 36, 43, 48 and 55.
Pluck OPEN compensation is measured per sound. Final gain is constrained by both
pre-protection peaks and four-times oversampled true peaks, with a -0.3 dBTP
ceiling for the measured phrases.

See the accompanying `audit/STUDIO_RESEARCH_091_AUDIO_LEVELS.json` and native
test report for the actual completed measurements. Automated checks establish
technical behaviour; musical quality remains a listening judgement. Denser
arrangements or subsequent edits can change level, so the output meter and final
protection remain useful.
