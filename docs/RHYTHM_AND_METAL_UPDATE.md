# SPACE INFINITY 0.3.0 — rhythm, modulation and purple metal

The next requested ten-item batch:

1. **Dedicated GATE page:** a large curve display with volume depth and separate sidechain controls.
2. **Dedicated SEQ page:** note pitches, velocities, rests, rate, note length, arp order and octaves, with a live step indicator. Editing notes does not edit the audio gate.
3. **Free gate points and curves:** double-click to add/remove interior points, drag nodes to move them, drag a segment or scroll over it to bend its curve. Up to 33 points.
4. **Drawing and snap:** selectable 4/8/16/32/64 grid, snap switch, draw switch, Shift to paint and Alt to snap temporarily.
5. **Gate timing:** distinct legacy step-rate and full-curve cycle controls, plus phase, swing, PWM and smoothing. Curve cycles range from 1/64 to 16 bars.
6. **MIDI behavior:** Note Retrigger restarts the curve; Note Gate closes it when no notes are held. Existing external-audio sidechain and internal pump remain available.
7. **Fifteen editable gate presets:** presets can be converted into curves while retaining their levels and full pattern duration. Frequency splitting still offers full range, bass, mids or highs.
8. **Modulation usability:** LFO drag handles are available on FX, GATE and SEQ, with destination highlighting, signed amount editing, zero/remove actions and Alt-wheel amount adjustment. Legacy performance/envelope/LFO routes are editable from target menus too. Sequence accents, EQ frequency/gain/Q and warp controls are supported. Hover EQ during a drag to open its controls. Offline sample trim/root and IR transformations are excluded from live routing; convolution wet/output controls remain assignable.
9. **Further click reduction:** smooth oscillator/sub/noise gains and filter routing changes, fade out ended samples, soften loop boundaries and retire held voices on preset changes. Measured abrupt-end/gain/sub-switch fixtures show over 98% reduction in the largest sample-to-sample jump. This does not establish that every audible artifact is eliminated in every FL Studio project.
10. **More vivid hardware finish:** saturated anodised-purple surfaces, brighter fixed metal reflections, violet-lit knob surrounds and a left-aligned cool-metal INFINITY wordmark.

This is an independent LFOTool-inspired workflow, not Xfer's interface or proprietary DSP. Old parameter IDs and legacy step processing are retained. Choose Step pattern for an unchanged legacy pattern; editing the display or using USE STEP PATTERN converts its levels and duration into a curve.

## Testing and download

Use the installer ZIP from the **v0.3.0 release**. Extract All, close FL Studio, run Setup.exe, reopen FL Studio and test a fresh instance. The ZIP contains the tested installer, notes and checksum. Older v0.2.0 files remain available separately.

Native tests cover curve point editing/deletion/bending/snap, phase/PWM/swing, MIDI retrigger/note gate, bass-only curve gating, all fifteen patterns at all seven legacy rates, sequencer separation, signed target-side amounts, state recall, click measurements and the existing preset/long-term audio suites. MAIN, FX, GATE and SEQ screenshots/bounds are checked at 800, 1000, 1180 and 1500 pixels. Actual listening and host routing in FL Studio remain user tests.

## Screenshots

[Main](images/Rhythm-Main.png) / [Gate](images/Rhythm-Gate.png) / [Sequencer](images/Rhythm-Seq.png) / [FX](images/Rhythm-FX.png)
