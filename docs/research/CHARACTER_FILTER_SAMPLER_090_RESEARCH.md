# Filter / granular implementation references — 18 November 2026

- W3C Audio EQ Cookbook: https://www.w3.org/TR/audio-eq-cookbook/
  Retrieved successfully. Independent reference for low-pass/high-pass/band-pass,
  notch and all-pass transfer behaviour and bilinear frequency warping. New SVF
  implementation uses a topology-preserving transform; it is original C++ code.
- Csound Appendix D, Formant Values:
  https://csound.com/docs/manual/MiscFormants.html
  Retrieved successfully. First three alto resonances inform A/E/I/O/U centres:
  A 800/1150/2800; E 400/1600/2700; I 350/1700/2700;
  O 450/800/2830; U 325/700/2530 Hz. Broader configurable Q and musical band weights
  intentionally create a playable vocal filter rather than an exact singer model.

Feedback combs use fractional delay interpolation with |feedback| <= .93, fixed
preallocated memory, and matching complex-valued display transfer. Filter display
accuracy is checked against independent impulse-response DFTs at 44.1/96/192 kHz.
Nonlinear legacy circuit plots are explicitly approximate.

Sampler overlap-add uses Hann windows and the instantaneous window sum as its
normalisation, not an AGC or slowly moving gain accumulator. Minimum three-grain
coverage avoids empty intervals at sparse density settings; upper overlap is
bounded to 24 grains inside a 32-slot pool. Forward/reverse loop crossfades use
raised-cosine complementary weights; they do not force the signal to silence at
wrap. No third-party samples, DSP source or proprietary filter algorithms added.
