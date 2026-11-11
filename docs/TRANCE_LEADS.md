# New trance leads

These patches are original INFINITY approximations programmed from the owner's supplied recipes. Serum percentages do not map directly to INFINITY's detune, blend, filter or effects controls. They are not sampled from the referenced recordings or soundbanks.

## Aurelia Lead (Gouryella-inspired)

Seven saw voices with a three-voice pulse/square-style layer one octave below. The lower layer is 37.5% of the mix, equivalent to 60% of A's level. Master detune is 0.0088 with stereo widths of 80%/60%; B's spread is scaled to approximately 55% of A's, with 75% blend. Its Pulse waveform is INFINITY's existing 48% duty pulse.

The amp has a 10 ms attack, 400 ms decay, approximately -3 dB sustain and 350 ms release. A resonant 12 dB low-pass sits at 6.5 kHz with low resonance. A four-bar triangle gently moves cutoff and A's width. Bright noise is very quiet and follows the amp envelope; there is no separate noise attack envelope. Eighth-note ping-pong delay and restrained plate-style reverb provide space. Cross-FM is left off to keep the core clean.

## Azure Horizon Lead (Out of the Blue-inspired)

Seven saw voices plus five additional saw voices, tight detune (0.003 shared spread), B +3 cents, no octave shift. The two engines provide the layered saw core without an additional sine/granular oscillator.

The amp uses an 8 ms attack, 300 ms decay, 78% sustain and 280 ms release. The 12 dB low-pass starts at 1.25 kHz. A note-triggered filter envelope adds up to 2.4 octaves with an 18 ms attack, 320 ms decay and 24% sustain, creating the melodic opening/breath. A much shallower four-bar LFO adds slow movement. Gentle chorus, filtered eighth-note ping-pong delay, restrained reverb, a +1.5 dB bell around 4.8 kHz, and 30% wet multiband compression add clarity. Upward compression is disabled (below ratio 1), with conservative downward ratio 2:1.

Custom wavetable scan modulation and legato glide are now available; these earlier patches retain their programmed width/cutoff movement. There is no extra full oscillator layer beyond A, B and the sine sub. These patches use width/cutoff movement and oscillator voicing instead. Reverb depth remains host-automatable. The LFO can be bypassed without removing the note-triggered filter envelope.

Factory IDs are F122 (Aurelia), F123 (Azure Horizon) and F124 (Prism Drive). Init F120 and Skywave Pad F121 retain their identities; the pad's name changes without replacing its saved identity. Previous F0–F119 sounds remain retired.

The native lead check renders 24 successive notes per patch and checks output bounds, late/early level, state recall and pad-to-lead envelope reset. It writes eight-second WAV previews to the audition artifact. Final timbre and FL Studio behavior still need the owner's listening test.

## Prism Drive Lead (general melodic lead recipe)

A distinct variation of the saw/pulse architecture: seven saw voices over three octave-down pulse voices, shared detune 0.0072, 72%/48% stereo widths and B at -3 cents. The 10/400/350 ms envelope and -3 dB sustain remain. Very light FM from B (2.8% warp amount), a 5.4 kHz low-pass, four-second cutoff/width motion, restrained drive, 125 ms stereo delay and a shorter, quieter reverb make this more direct than Aurelia. It uses INFINITY's own chorus/saturation/reverb; no external effect plugin is required.

## Orbital Expedition Lead (Exploration of Space-inspired)

Three triangle voices FM-modulated by a lightly detuned sine layer. A fast 4 ms attack, 220 ms decay, 58% sustain and 200 ms release articulate the metallic/pluck core. The 1.8 kHz low-pass opens with an independent 1.8-octave note envelope; a 150 Hz EQ high-pass removes sub energy. Moderate drive, 40% wet single-band compression, eighth-note ping-pong delay and restrained bright room reverb add edge and space. A small 5.1 Hz legacy LFO adds pitch/filter movement; VIBRATO bypasses it. Legato mode uses a 65 ms glide time constant. Its new factory identity is F125.

## Synth controls added for these recipes

Each oscillator now has **SPREAD x** (0–2 times the master detune) and **BLEND** (side-voice level relative to the centre voice or centre pair). Defaults of 1x/100% preserve the previous engine's equal-weight stacks. Existing projects missing these parameters receive those defaults. Skywave uses a 65% blend; the saw/pulse leads use 75% on A and a tighter B spread, with Prism's B blend at 70%. Azure uses tighter spread on both saw layers.

**ENGINES → FILTER ENV → Legato glide** controls oscillator pitch smoothing for overlapping notes in Legato voice mode. Zero preserves the previous immediate pitch change; samples retain their existing stepping behavior. The value is an exponential time constant, not a fixed-duration ramp. Mono retrigger and poly mode keep their existing articulation.

The lead test measures an octave glide from audio zero crossings, alongside repeated-note output and saved-state checks. No original song MIDI is bundled.

## Ionwire Gate Lead (FireWire-inspired)

Eight saw voices plus eight pulse voices one octave up, detune 0.016 and a 1.375x spread on B, 75%/72% blend, very wide stereo. A full-range square trance gate alternates on/off at 1/32-note steps, creating a 1/16-note cycle with half-cycle open, with 1 ms attack and 3 ms release. Gate depth is 100%. The internal gate handles amplitude directly; the designer LFOs remain bypassed. Disable GATE to hear the continuous oscillator core.

Multiband compression uses 16 dB makeup with -12 dB output compensation (4 dB net wet gain), 65% mix, 4:1 downward ratio and no upward gain. A 160 Hz high-pass and short sixteenth-note ping-pong delay keep it focused. Macro 1 (Energy) is assigned to cutoff, initially neutral. No pitch-dive envelope is imposed. Factory identity: F126.

## Expanse Motion Pluck (Xpander-inspired)

A three-voice saw blended almost equally with sine, lightly detuned, with an independent +2 semitone pitch envelope (20 ms attack, 90 ms decay back to concert pitch). A 3 ms amp attack, 250 ms decay, 34% sustain and 220 ms release create the pluck. An independent filter envelope opens the low-pass, while a four-bar LFO moves both oscillator widths and chorus mix. Slow chorus and 12/19 ms stereo delays provide dimension-style width; restrained reverb and a 170 Hz high-pass keep the hit clear. This uses INFINITY's own effects, not a named third-party Hyper/Dimension implementation. Factory ID: F127.

The new per-voice pitch envelope controls are in **ENGINES → ANALOGUE**: amount in semitones, attack and decay in seconds. Zero amount (the default) bypasses it, preserving older patches. The envelope acts on synth oscillators, independently of the amp and filter envelopes; release returns it to zero over 30 ms. The lead test measures the audible pitch returning to the played note after the transient.

## Solo Afterglow Lead (Better Off Alone-inspired)

One centred saw, no detune, mono retrigger, with Osc B off. Instant attack, 8 ms decay, full sustain and 20 ms release. Full sustain means the amp decay does not lower the held-note level; articulation comes from note length, the short release and a small 45 ms filter transient above the 3.6 kHz low-pass. A 220 Hz high-pass removes lower weight. Designer LFOs, legacy vibrato, chorus, gate and compression are off. A low-mix 125 ms stereo delay and tiny room reverb add restrained space. Factory ID: F128. No song melody is embedded.

## Silken Voltage Lead (square-lead variation)

A separate interpretation of the repeated saw/pulse lead recipe, using its suggested two-square variation: seven pulse voices above three quieter octave-down pulse voices, 75%/70% blends, 72%/55% widths and a tighter B detune spread. Very light audio-rate FM from B naturally follows keyboard pitch; it is not a keytracked low-frequency oscillator. INFINITY's pulse waveform has a 48% duty cycle. The 10 ms attack, 400 ms decay, -3 dB sustain and 350 ms release remain. A 5.5 kHz low-pass, a small 300 Hz cut and +1.8 dB around 4.8 kHz balance body and presence. Short stereo delay, chorus and restrained reverb finish the sound. Factory ID: F135. The native lead test includes its repeated-note level, bounds, preview and state recall.
