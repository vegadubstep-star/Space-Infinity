# Native extended engines

Adds native DSP and controls to the codex/windows-build branch:

- Independent per-voice filter ADSR; bipolar cutoff depth in octaves. Attack/release max 1 second.
- Granular WAV source: 32 Hann-windowed grains per voice; position, size, density, spray, width, root pitch, trim and reverse.
- Held-note arpeggiator: up/down/up-down/random, 1–4 octaves, tempo divisions and gate.
- 16-step relative-pitch sequencer with length, velocity and rest controls; may combine with arp.
- Partitioned stereo convolution using JUCE; external WAV IR up to 4 seconds / 4 MB, dry/wet and bypass. Decoding on caller thread; prepared buffer transferred to the audio thread through a try-lock before the wait-free load.
- Editable/importable/exportable wavetable bank: 1–64 frames, 512 internal points/frame, mono WAV input at 512/1024/2048 samples/frame, interpolated frame scanning, drawing, sine/saw generation, duplicate/delete. Both oscillators can select Custom Wavetable. No spectral editor or antialiased wavetable mipmaps yet.
- Previous/next waveform controls on both oscillators, including wraparound.
- New parameters and imported sample/IR/table assets save in project state; older states use defaults. Existing factory selection disables pattern/filter-envelope/convolution additions.

Open ENGINES for five panels. Import a WAV on Main, then choose Granular WAV to use granular playback. Hold MIDI notes for arp/sequencer playback. Patterns are internal instrument note generators; this plugin does not emit MIDI to other plugins.

Validation targets: ExtendedCore (envelope timing, table interpolation, arpeggio timing/order/gates, sequencer rests and granular finite audio), ExtendedSmoke (native audio, file import/export, embedded state recall, MIDI generation, UI waveform navigation), existing Smoke and AnalogueProbe. Windows CI builds VST3 and Standalone, runs CTest and verifies installer/archive. FL Studio listening/host validation remains a separate manual check.
