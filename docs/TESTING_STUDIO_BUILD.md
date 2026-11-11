# Testing the INFINITY studio build

Use the latest `codex/windows-build` revision. The Windows GitHub Actions run builds the VST3/Standalone, runs native tests, verifies installation/uninstallation, and publishes the Windows x64 installer/archive plus native screenshots/audio auditions. A running workflow is not a completed Windows validation.

After the workflow succeeds, download **Space-Infinity-Windows-x64** from that run's Artifacts section. Extract it, close FL Studio, run the setup, then reopen FL Studio and scan/load **SPACE INFINITY**. The plugin keeps its existing host identity; its new header wordmark reads INFINITY.

1. Browse the 119 factory presets with the menu and arrows. Try bass, lead, pad, choir and sampled-instrument categories; the arrows should continue from the selected sound.
2. Import a WAV and play MIDI notes. Check the source waveform, trim handles, reverse, root note and the live output waveform while adjusting the filter/envelope/effects.
3. Open FX and try Single/Multiband compression. The three lanes show live band levels/reduction; drag a lane to change its threshold.
4. Compare Normal, Ping-pong and Tap → Delay. Try separate L/R BPM divisions, linking, offsets, HQ and dragging the filter graph. Double-click the graph for the output spectrum.
5. Scroll to Convolver. Try the four built-in impulses and your own WAV IR, then size/tone/minimum-phase/pre-delay/attack/decay/damping/reverse. Save and reload a user preset to check embedded sample and IR recall.
6. Open the seven-band EQ. Compare bell/shelf/pass/notch shapes and the 12/24/36/48 dB slopes. Drag nodes and scroll over a node for Q.
7. Try all 15 trance-gate presets. Select Bass only to leave the higher bands moving freely. Internal pump works without another audio track. For External audio, enable the optional Sidechain input in the host and route your kick/trigger to it; FL Studio routing still needs this manual host check.

Please report the preset name, feature/control, whether it happens in a fresh instance or recalled project, and a short audio/screenshot example for anything that sounds or behaves wrong.

Native Linux validation for this update covers all 119 preset auditions, UI bounds/screenshots at four sizes, preset/state/migration tests, extended engines and the studio tests. The studio tests exercise dynamics, actual delay impulse timing/routing, frequency-selective gating, 15 distinct patterns, external sidechain detection and the optional input bus, EQ slopes, minimum-phase magnitude preservation, sampler/IR displays and embedded state recall. Windows workflow results and FL Studio listening are separate checks.

The smoke tests allocate full processor instances on the heap, matching host allocation and avoiding stack exhaustion with Windows' standard 1 MB stack. The complete seven-test suite also passes locally with an explicit 1 MB stack limit.
