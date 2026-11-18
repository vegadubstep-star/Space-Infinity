# Numbered graph validation — 18 November 2026

This follow-up changes UI presentation, plotting bounds and pointer interaction only. No DSP, parameter definitions, preset bank or sample edits.

- Native AnalogueExpansion, DualLFO and RhythmWorkflow suites:3/3 passed in9.56seconds.
- MAIN/FX/GATE/SEQ editor bounds and rendered views passed at800/1000/1180/1500 pixels.
- Cursor at the LFO plot midpoint reports3.2s /50% with a4-bar pass at150BPM. Both panes are refreshed for the captured close-up.
- Real envelope drag changes Attack to150ms; graph readout matches. LONG x3 reports450ms Attack and300ms Release for the test patch.
- LFO and gate point insertion, segment bending, snapping and deletion still pass against the visible plotting surface.
- A final focused LFO run and four-page UI render passed after the footer separator/capture refresh correction.
- Graph-Numbers-MAIN.png and Graph-Numbers-Detail.png are actual compiled native UI renders. The latter is a2× workbench capture with the live numeric hover badge.

The held Windows installer from sourcef8db6e7 predates this follow-up. It was not replaced or published. Downloads remain held; package current source before supplying the next Windows build.
