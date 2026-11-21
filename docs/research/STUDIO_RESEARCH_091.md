# Studio Research preset revision

Research and authoring date: 21 November 2026. This is an original programming
pass for SPACE INFINITY 0.9.0 or later, not a bank of copied artist patches.

## Sources consulted and practical decisions

| Source | Finding used in this pass | Application in Infinity |
|---|---|---|
| [Roland's V-Synth programming seminar, Pablo LaRosa](https://www.rolandus.com/go/v-synth/Seminar3/index.html) | Resonant filtering, overdrive and performance assignments work together; a sub layer can reinforce an aggressive sound. | Acid SWEEP/LIQUID/CORROSION controls separate cutoff motion, envelope articulation and nonlinear colour. |
| [Roland's TB-303 programming tutorial](https://articles.roland.com/mastering-the-tb-303-sequencer-in-roland-cloud/) | Accents change both filter brightness and amplitude. Selective slides contrast with detached notes; cutoff, resonance and decay are useful automation targets. | Velocity-sensitive acid contours, legato glide, ten separately voiced filter/drive combinations. Customer MIDI remains in control of the phrase. |
| [Nord Lead 2X official specifications](https://www.nordkeyboards.com/legacy-products/nord-lead-2x/) | Its Lead 2 architecture includes pulse, FM, sync, filter-envelope velocity and modulation-envelope destinations. Morph controls change several settings together. | Preserve different pulse, reed, FM and interval lead structures; add envelope-dependent warp to selected metallic leads instead of giving every lead another saw stack. |
| [Diva user guide, programming and effects sections](https://u-he.com/downloads/manuals/plugins/diva/Diva-user-guide.pdf) | PWM provides harmonic movement; slow ensemble/chorus adds stereo motion, with the wet level controlling wash. Envelope curves and oscillator levels affect character. | Pads get modest timbral motion and individual swells/tails. Bass contours differ independently of their amp envelope. Analogue character stays restrained on the bass fundamental. |
| [Attack's original Sylenth supersaw walkthrough](https://www.attackmagazine.com/technique/synth-secrets/sylenth-supersaw/) | Stacked voices, octave support and phase behaviour shape width/body; drive and suitable polyphony matter too. | Retain existing lead voice structures and supporting layers, strengthen sustained body, and avoid using maximum detune as the only distinction. |
| [Attack's original synth-layering walkthrough](https://www.attackmagazine.com/technique/synth-secrets/layering-synths/) | Low, middle and upper layers need complementary roles. A separate filter contour with sufficient amp body can open a short tone into a longer one. | Centre the clean bass weight, shorten rolling-bass tails, and give synthesised plucks an audible held body underneath their filter decay. |
| [Attack's original layered pluck walkthrough, Aykan Esen](https://www.attackmagazine.com/technique/synth-secrets/mall-grab-switchblade-lead/) | A sampled transient and a synthetic harmonic layer can be complementary. Effects must preserve the attack. | Keep the sampled pluck identities and natural decay; voice their individual delay/reverb spaces instead of turning all twenty into the same synthesis recipe. |
| [Korg's M1 architecture description](https://www.korg.com/us/products/software/kc_m1/) | PCM and digital effects are integral to the instrument's character. | Preserve the existing sampled mallet, nylon, clavinet and string plucks rather than replacing these with vaguely similar saw patches. No Korg PCM is imported. |
| [Roland's JP-8000 product document](https://cdn.roland.com/assets/media/pdf/JP-8000.pdf) | Detune and mix serve different purposes in the Super Saw sound. | Retain individual oscillator balance and detune settings rather than widening everything equally. This source is a product brochure, not the owner's manual. |

These are design principles translated to Infinity's controls. Numerical settings
in the authoring script are our own choices; they are not documented settings from
the original recordings or claims of exact hardware emulation.

The JP-8000 owner's manual was also located through [Roland support](https://www.roland.com/us/support/by_product/jp-8000/)
but its scanned PDF did not expose readable text to the web tool. The original
TB-303 PDF exceeded that tool's download limit. Sound On Sound's string/formant
articles returned access errors. These are not represented as fully read sources.
Ableton Learning Synths required JavaScript, so its interactive recipe was not
used as evidence. No downloaded artist presets or unlicensed samples are added.

## Scope and musical intent

- Fifteen offbeat basses and fifteen rolling basses: individually voiced filter
  decay, amp decay and sustained level; stable sine support and repeatable phase.
  The rolling sounds remain unsequenced.
- Twenty hard-trance leads: near-instant attacks, firmer sustained bodies and
  less bass energy in their delay/reverb returns. Selected FM/warp transients
  follow the filter envelope. Existing waveform identities remain distinct.
- Twenty plucks: thirteen synthesised tones receive new filter articulations;
  seven sampled tones retain their source. Every pluck has individually voiced
  space, an OPEN / LEAD gesture and measured level compensation.
- Ten airy pads: slower swells, longer releases, gentle timbral variation and
  filtered spatial tails. No amp LFO is introduced to create artificial life.
- Ten acids: saw/square/pulse/resonant/bite sources, contrasting decay lengths,
  nonlinear distortion types and velocity/legato articulation.

Names and original IDs remain stable. The 200 Heritage sounds and the separate
20 Lush leads are outside this focused revision. The other seventy factory
recipes remain unchanged. User-saved patches must not be overwritten.

## Validation requirements

The authored output is provisional until native exports validate all parameter
ranges and recall, macro combinations are rendered, pluck OPEN compensation is
measured, and gain is calibrated against pre-protection peaks and four-times
oversampled true peaks. Similarity measurements flag sounds for review; they
cannot establish musical quality or prove that no two sounds are perceptually
similar. Musical character remains a listening judgement.

Single notes, repeated articulation, chords and macro extremes test headroom;
they do not guarantee that every possible MIDI arrangement or subsequent edit
stays below clipping. Final protection remains enabled. No subjective listening
approval is claimed from automated metrics alone.

## Regression findings during voicing

The full bass regression exercises MIDI 36, 43, 48 and 55 with short, offset
notes, complementing the authoring sweeps at 28, 40 and 52. Four revised basses
needed additional output headroom at those pitches. Their gains were reduced;
the underlying mono/body/retrigger checks were retained.

Four acids initially drove the input too hard for the main cutoff to retain
useful brightness contrast. Lower input gain with HOT still enabled restored
that contrast; the pulse variant also uses a less flattened wet blend. Native
spectral probes confirmed stronger opening above 2 kHz. The audible objective
is expressive filtering with distortion, rather than a fixed saturated tone.

Sampled airy pads intentionally use Dimension alongside the synth pads. The
older factory-choice regression assumed that every sampled source used Classic
chorus; its expectation was updated for this deliberate voicing choice. The
chorus stability, bass-centre, dry mix, stereo-width and recall checks remain.

The recipe audit contains exact renderer input snapshots, including their
pre-verification status labels. The completed audio audit and factory source
record the final verification state. Text-file hashes use normalized LF line
endings for Windows-checkout compatibility.
