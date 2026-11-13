# SPACE INFINITY 0.6.0 — Levels and performance controls

Factory presets use measured perceived-loudness trims with a -28 LUFS single-note reference and separate chord headroom limits. Short plucks keep their transients and pads keep their swells. Your saved user presets are not rewritten. Final measured results are recorded in audit/CREATIVE_WORKFLOW_VALIDATION.md.

**LEVEL MATCH** is a separate optional switch beside the main envelopes. Enable it while playing your starting tone. It learns a reference over 250 ms, then smoothly reduces gain when opening the filter makes the sound louder. It never boosts quiet tails or noise, never raises gain above unity and retains the reference across fresh notes. Toggle off/on to learn a new starting level. This is a level helper, not an EQ: the brighter tone remains. Preset loudness trims apply automatically; Level Match is for live sound shaping.

**ANALOGUE / DIGITAL** is the overall engine selector above the filter. Analogue uses your character/component settings. Digital bypasses their drift, tolerance and colour while keeping your selected filter model, voicing and envelope contours. Returning to Analogue restores the stored component settings. The selected mode stays when browsing factory sounds and saves with project state.

Energy, Width and Space remain. The three additional factory macro slots are named **Tone**, **Punch** and **Character**. Tone shapes cutoff and upper EQ; Punch adjusts filter envelope, decay and drive; Character adds voice colour and saturation. They start neutral and remain freely assignable. Init and existing user patches retain their own assignments.

The main pitch wheel covers **-24 to +24 semitones** and springs to zero on mouse release. Host automation remains usable for risers; external automation is not continuously forced back to zero. New factory patches use a 24-semitone incoming MIDI wheel range, adjustable in PITCH.

Drag **LFO +** onto a bass control, or anywhere on the **SUB** panel to assign sub level. Ctrl-dragging the graph also assigns the existing LFO shape; ordinary dragging still edits it. The destination amount popup adjusts depth; right-click the destination to change/remove an assignment.

ENGINES now contains Expression, Granular, Wavetable, Analogue Lab and Pitch. Repeated main-page envelopes, sub level, portamento, analogue character/drive, automation pitch, wavetable scan and convolver controls have been removed from that window. Advanced controls remain available; envelopes are on MAIN, convolver/IR loading in FX, and WT POS on the oscillators.

The 15 gate presets now load their correct timing, depth and smoothing together. Classic 16ths produces 16 pulses per 4/4 bar; Classic 8ths produces eight. The bank also includes offbeat trance/stabs, gallops, anthem hold/chop, syncopated cuts, hard-trance chopping, dotted and true triplet shapes, half-bar holds, accents, breakdown/stutter and a quarter-note pump. These are independently authored common rhythmic patterns, not extracted artist presets. Frequency-split selection remains available.

Close your DAW, Extract All from the installer ZIP, run Setup.exe, then load a fresh instance. To try Level Match: hold a pluck/lead, set a low cutoff, switch it on, wait briefly, then open cutoff.

The BLOCKS view follows the supplied simple-gate reference: linked stereo lanes with click-to-toggle blocks and drag-to-adjust levels. Width controls each block's duty cycle; Length selects 1–16 blocks. Delay initially passes the dry sound, and Fade In gradually introduces the gate depth on the first held note. CURVE keeps the original freely editable LFO-style graph. The sequencer remains separate.
