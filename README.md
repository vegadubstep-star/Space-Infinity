# INFINITY / SPACE INFINITY

Native JUCE Windows x64 synthesizer for FL Studio, with the owner's INFINITY wordmark and purple metallic hardware interface. The VST3 keeps the existing **SPACE INFINITY** host identity.

Current development branch: **codex/windows-build**.

- 160 authored trance / 90s German hard-trance sounds plus Init, with revised acid, airy pads, choir ensembles and varied leads. Previous factory presets removed; user-saved presets remain available.
- Live loaded-sample/processed-output waveforms, trim/reverse and sample import.
- Dedicated 14-mode distortion, editable preset name/creator/tags, local factory replacements and hybrid creation.
- Expanded single/multiband dynamics, stereo/Ping-pong/Tap delay, shaped convolution, seven-band EQ with filter/slope choices.
- Frequency-selective sidechain/pumping and trance gate with 15 editable patterns.
- Native oscillator/filter/envelope/modulation controls and the existing granular/wavetable/arp/sequencer engines.

See [0.6.1 acid, bass and choir changes](docs/ACID_BASS_AND_CHOIR_UPDATE.md), [0.6.0 sound and workflow changes](docs/CREATIVE_WORKFLOW_UPDATE.md), [level and control changes](docs/CONTROL_AND_LEVEL_UPDATE.md), [studio features](docs/STUDIO_FEATURES.md), [factory bank status](docs/STUDIO_FACTORY_BANK.md), [testing instructions](docs/TESTING_STUDIO_BUILD.md), and [native interface screenshots](docs/HARDWARE_INTERFACE.md).

## Windows build

Requires Visual Studio 2022 Desktop C++ workload, CMake and Git. From the project root in PowerShell:

```powershell
.\scripts\build-windows.ps1
ctest --test-dir native-vst3/build -C Release --output-on-failure
```

For an installer/archive, install Inno Setup 6 and run `scripts/package-windows.ps1`. GitHub Actions also builds, tests and packages the Windows x64 outputs on branch pushes. Use the result for the exact commit you checked out; a running workflow is not a successful Windows validation. FL Studio routing, compatibility and listening tests remain separate host checks.

The [test guide](docs/TESTING_STUDIO_BUILD.md) explains where to download the installer and how to exercise the new features. Existing user presets and project parameters are retained. The previous factory bank is retired; Restore Factory now restores the thirty-four new bank entries. See [Skywave Pad](docs/AIRWAVE_PAD.md) for voicing, LFO bypass and long-envelope controls, and [new trance leads](docs/TRANCE_LEADS.md) for the eight leads.

See [groove bank additions](docs/GROOVE_BANK.md) for the new rhythmic, bass and house patches.

See [further melodic interpretations](docs/ANTHEM_BANK.md) for the newest leads, arps and performance controls.

[Save As and material finish](docs/SAVE_AS_AND_MATERIALS.md): BANK → SAVE AS... writes a portable `.sipreset` to your chosen folder. The updated interface uses deep violet anodised metal, brushed knob caps and smoked displays.
