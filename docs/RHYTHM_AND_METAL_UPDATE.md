# SPACE INFINITY 0.3.0 — rhythm, modulation and purple metal

The next requested ten-item batch:

1. **Dedicated GATE page:** a large curve display with volume depth and separate sidechain controls.
2. **Dedicated SEQ page:** note pitches, velocities, rests, rate, note length, arp order and octaves, with a live step indicator. Editing notes does not edit the audio gate.
3. **Free gate points and curves:** double-click to add/remove interior points, drag nodes to move them, drag a segment or scroll over it to bend its curve. Up to 33 points.
4. **Drawing and snap:** selectable 4/8/16/32/64 grid, snap switch, draw switch, Shift to paint and Alt to snap temporarily.
5. **Gate timing:** distinct legacy step-rate and full-curve cycle controls, plus phase, swing, PWM and smoothing. Curve cycles range from 1/64 to 16 bars.
6. **MIDI behavior:** Note Retrigger restarts the curve; Note Gate closes it when no notes are held. Existing external-audio sidechain and internal pump remain available.
7. **Fifteen editable gate presets:** presets can be converted into curves while retaining their levels and full pattern duration. Frequency splitting still offers full range, bass, mids or highs.
8. **Modulation usability:** LFO drag handles are available on FX, GATE and SEQ, with destination highlighting, signed amount editing, zero/remove actions and Alt-wheel amount adjustment. Legacy performance/envelope/LFO routes are editable from target menus too. Sequence accents, EQ frequency/gain/Q and warp controls are supported. Hover EQ during a drag to open its controls. Sample trim/root and convolution wet/output controls are live destinations. Offline IR transformations are excluded from live routing.
9. **Further click reduction:** smooth oscillator/sub/noise gains and filter routing changes, fade out ended samples, soften loop boundaries and retire held voices on preset changes. Measured abrupt-end/gain/sub-switch fixtures show over 98% reduction in the largest sample-to-sample jump. This does not establish that every audible artifact is eliminated in every FL Studio project.
10. **More vivid hardware finish:** saturated anodised-purple surfaces, brighter fixed metal reflections, violet-lit knob surrounds and a left-aligned cool-metal INFINITY wordmark.

This is an independent LFOTool-inspired workflow, not Xfer's interface or proprietary DSP. Old parameter IDs and legacy step processing are retained. Choose Step pattern for an unchanged legacy pattern; editing the display or using USE STEP PATTERN converts its levels and duration into a curve.

## Two visible LFO editors

MAIN shows LFO 1 and LFO 2 together, with independent power, shape, rate, BPM/Hz and Free/Retrigger/Envelope controls. Each source handle can be dragged to a supported synth or FX control. Click the source title to choose LFO 3 in either pane. ROUTES and the destination's right-click menu edit assignment amounts. EDIT opens a larger editor with Phase, Delay, Rise, Smooth, Forward/Reverse/Ping Pong, Mono, Triplet, Dotted and Host controls. Right-click the graph to set/clear an envelope loopback time.

Double-click adds/removes interior points; drag points horizontally or vertically; drag a segment or scroll over it to bend it. Snap and Alt-drag align points to the grid. Shift or Draw paints the shape. Curves support up to 33 points and are saved with the preset. Previous fixed-point curves remain unchanged until edited.

The reference is Xfer's [official Serum 2 manual](https://xferrecords.com/manual/serum-2/docs), pages 189–194. This implements an independent version of the documented Normal LFO workflow. It does **not** establish identical Xfer DSP or full feature parity: Path/Chaos/S&H types, multi-point selection, source-copy gestures and separate LFO-shape file management are not included. The editor's draw tool paints interpolated points rather than Serum's separate flat/ramp drawing tools.

## Oscillator and macro additions

Each oscillator has its own WT POS knob, displayed from frame 1, and a perspective stack of waveform frames. Built-in families have 17 interpolated frames; imported Custom Wavetable uses its actual frame count. The six appended choices are **303 Saw, 303 Square, Acid Pulse, Acid Resonant Saw, Acid Squelch and Acid Bite**. These are original generated waveforms, rather than samples from a Roland instrument or Xfer's tables. Previous waveform indices are unchanged.

All six visible macros now support 16 live destinations each, alongside the three LFOs' 16 destinations. Drag a source to a supported knob, then drag its coloured outer ring to adjust the signed amount. Dragging the knob face changes the base value. Right-click gives amount/zero/remove controls; Alt-wheel adjusts the amount, with Shift for finer movement. Existing performance routes remain editable and retain their saved targets and depths. A macro's ROUTES popup lists its new assignments; pre-existing legacy assignments also appear in destination menus.

**UNI** opens each oscillator's Linear/Super/Exp/Inv/Random modes, octave/fifth stacks, central sub-octave stacks, detune Range, Phase, Rand, wavetable-position spread and warp spread. Detune is displayed as a decimal such as 0.12. Updated Blend uses **75% for equal voice levels**, balancing one central voice for odd counts or two for even counts; Blend is ignored with one/two voices. Detune is ignored with one voice. Updated mono unison does not create artificial stereo spread. Phase/Rand are used when Reset phase to zero is off. Old saved states retain the previous unison/position behavior unless switched to the updated controls; fresh factory patches use the updated model.

The source for these behaviors is the [official Serum 2 manual](https://xferrecords.com/manual/serum-2/docs), pages 41–48. It documents the modes, stack choices, width, range and 75% Blend behavior, but does **not** publish the numerical detune transfer curve, exact Super/Random distributions or compensation formula. SPACE INFINITY uses independent distributions and conservative level normalization; its Range is explicitly measured in cents. This is not a measured or bit-identical Serum 2 detune implementation. Phase-memory/phase-legato modes and a second simultaneous warp stage are not included.

## Testing and download

Use the installer ZIP from the **v0.3.0 release**. Extract All, close FL Studio, run Setup.exe, reopen FL Studio and test a fresh instance. The ZIP contains the tested installer, notes and checksum. Older v0.2.0 files remain available separately.

Fourteen native suites include a dedicated dual-LFO check for two visible graphs at all four sizes, independent free-point editing, audio modulation, timing controls and preset recall. Native tests also cover curve point editing/deletion/bending/snap, phase/PWM/swing, MIDI retrigger/note gate, bass-only curve gating, all fifteen patterns at all seven legacy rates, sequencer separation, signed target-side amounts, state recall, click measurements and the existing preset/long-term audio suites. MAIN, FX, GATE and SEQ screenshots/bounds are checked at 800, 1000, 1180 and 1500 pixels. Actual listening and host routing in FL Studio remain user tests.

## Screenshots

[Main](images/Rhythm-Main.png) / [Gate](images/Rhythm-Gate.png) / [Sequencer](images/Rhythm-Seq.png) / [FX](images/Rhythm-FX.png)
