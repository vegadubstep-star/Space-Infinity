# Codex instructions — SPACE INFINITY

You are working in the user's **SPACE INFINITY** synthesizer project. Read README.md and docs before editing.

## Non-negotiable user goals
- Windows x64 **VST3** instrument that runs in **FL Studio**.
- Highly realistic physical hardware aesthetic: straight-on **bird's-eye/top-down** (no angled perspective), dark textured brushed metal/black-anodised panels, finely detailed screws/seams, red LED lighting, clear typography, professional restrained design. **Not** a static mockup: controls must function.
- Show genuine real-time **oscillator waveforms**, **amplifier ADSR** shape, **filter envelope** shape and modulation waveforms. Display state must match actual DSP when feasible.
- Resizable JUCE interface: clear section hierarchy, large tactile controls, low knob count, precise values/tooltips.
- Strong sounds for 1990s German hard trance (Cocooma/Gary D-style *original* sound design; no copying proprietary recordings or presets).
- Excellent chorus, reverb, stereo tempo-synchronised delay; solid filters, modulation, patch recall.

## First milestone (do not skip)
1. Inspect all files; report what is functional vs stubbed or unverified.
2. Build `native-vst3` with JUCE on **Windows x64**; correct all compiler/linker errors. Report commands, exact output, and actual test results.
3. Render **a real screenshot from a running native plugin/standalone app**, not a generated concept.
4. Smoke-test MIDI note-on/off, note stealing, panic/all notes off, audio channels and levels, host automations, preset/state serialization and reload. Test in FL Studio when available, and report when not available.
5. Address realtime audio quality: no per-block heap allocation; parameter smoothing; avoid clicks/denormals; stable CPU usage.
6. Checkpoint through git commits and a precise changelog.

## Subsequent milestones
- Port browser design into a *functional* high-fidelity JUCE editor (not WebView unless justified).
- Implement DSP-coupled waveform and ADSR displays, proper multi-stage envelopes, rich filters, mod matrix, stereo effects.
- Investigate optional legally licensed JV-880 emulation, **but do not import VirtualJV source or ROMs without reviewing the source licence and the owner's intended distribution model**. If incompatible with commercial use, propose an independent sample-based engine with user-supplied patches/waves instead.
- Handle **ROMs outside the repository**. Never add ROM binaries, extracted WAVs, proprietary firmware or the existing third-party VST3 into Git or a build artifact.
- Provide a tested Windows VST3/standalone release candidate only after tests; do not claim success from source code alone.

## Coding expectations
C++17 or newer; JUCE; CMake; straightforward architecture; testable DSP; thread-safe audio processing; unit or smoke tests; reproducible build. Treat screenshots in design/ as visual references. The browser prototype is a visual/behavioural reference, not production audio code.

## Reporting
Be truthful: separate implemented/compiled/validated; do not claim FL Studio compatibility unless tested. Communicate in plain English and use visuals from the actual application.