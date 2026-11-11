# INFINITY studio update

The native interface now has purple metal panels with a fixed-light gleam and the owner's INFINITY wordmark. All controls remain native JUCE components connected to audio parameters.

## Sampler displays

When a sample source is selected, the left oscillator screen becomes the actual loaded sample's peak waveform, with trim boundaries, reverse direction and a playback marker. Drag the boundaries or use Start/End below. WAV files can also be dropped on that screen. The right screen shows the actual rendered, post-FX output. Play MIDI notes to see changes caused by pitch, filter, envelope, distortion and effects; scroll over this screen to change its time window. Original sample data remains intact.

## FX rack

The FX page now scrolls through dedicated compressor, delay, convolver, trance-gate/sidechain, chorus, flanger, phaser and reverb panels. The EQ opens from the button below the rack. The fixed signal order is gate/sidechain → compressor → chorus → flanger → phaser → delay → reverb → convolver → EQ → output.

- Compressor: Single or Multiband operation; threshold, ratio, attack, release, makeup gain, mix, low/high crossovers, below-threshold dynamics, three band gains and output level. The band display reports actual input levels and gain reduction; drag a lane to offset its threshold. The optional peak ceiling is a sample-peak safeguard, not a true-peak/lookahead limiter. A Legacy mode retains compatibility with earlier project states. Parallel multiband mixing uses a phase-aligned dry reference.
- Delay: Normal stereo, Ping-pong and Tap → Delay routing; separate L/R time and tempo division, time multipliers, linking, HQ cubic interpolation, feedback, filter frequency/width, mix and output level. Drag the filter graph to change frequency and width; double-click for an output spectrum overlay. Delay time is limited to four seconds per channel. Legacy mode preserves the earlier delay.
- Convolver: four original procedural room/plate/hall/metal impulses, external WAV loading/drag-drop, previous/next selection, processed IR envelope, size, tone, minimum phase, pre-delay with BPM sync, attack, decay, damping, IR gain, mix, output level and reverse. Custom IR data is automatically embedded in presets/projects. Decoding, transformation and minimum-phase FFT preparation run outside the audio callback; parameter automation requests background rebuilds. Minimum phase rearranges the impulse's phase to move energy forward while retaining its magnitude response; it is distinct from an ordinary algorithmic reverb. The built-in impulses are original procedural assets, not Serum files.
- EQ: seven bands with bell, low/high shelf, low/high-pass, notch and band-pass shapes. Pass filters offer 12/24/36/48 dB-per-octave slopes. Drag nodes for frequency/gain and use the wheel for Q. The plotted curve uses the same coefficient calculations as the DSP.
- Trance gate and sidechain: 16 editable step levels, 15 named original patterns, attack/release, depth and tempo division. The separate ducking section supports an internal tempo pump or an optional external audio input. Choose Full range, Bass only, Mid band or High band and adjust the split frequencies. This isolates processing to the chosen band. Enable and route the Sidechain input in your host before choosing External audio.

## Reference research

The compressor, delay and convolver control sets were checked against Xfer's [Serum 2 User Guide](https://xferrecords.com/manual/serum-2), particularly the Compressor, Convolve and Delay sections. These are independent implementations with INFINITY's own hardware styling; they do not use Serum code, interface assets, impulse responses or factory presets, and do not claim identical sound.

## Factory bank

The previous factory bank has been removed. Only a dry **Init** sound remains while the owner specifies the replacement bank. See [factory bank status](STUDIO_FACTORY_BANK.md).

Saved projects and user presets retain their embedded parameters/assets. Old factory IDs are retired rather than reassigned.
