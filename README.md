# INFINITY / SPACE INFINITY

Native JUCE Windows x64 synthesizer for FL Studio, with the owner's INFINITY wordmark and purple metallic hardware interface. The VST3 keeps the existing **SPACE INFINITY** host identity.

Current development branch: **codex/windows-build**.

[Download the verified 0.9.0 Windows installer ZIP](https://github.com/vegadubstep-star/Space-Infinity/releases/download/v0.9.0/SPACE_INFINITY_0.9.0_Windows_x64_Installer.zip).

Import the included **SPACE_INFINITY_Lush_Trance_20.zip** through BANK → IMPORT to add the 20 new leads.

- [0.9.0 filter, sampler and lead update](docs/CHARACTER_FILTER_SAMPLER_090_UPDATE.md): 15 filters with a live response graph, stereo granular/loop blending, sampler octave fix, Voice Life, A → B transitions, larger controls, 20 new leads and seven stronger existing leads. Windows/native checks and the published installer ZIP are verified.
- [0.8.0 sound-bank and waveform update](docs/TRACK_READY_080_UPDATE.md): 52 oscillator families, six steady choir/hybrid sources, revised factory160 and separate Heritage200, and peak calibration.
- 160 authored trance / 90s German hard-trance sounds plus Init, with revised acid, airy pads, choir ensembles and varied leads. Previous factory presets removed; user-saved presets remain available.
- Live loaded-sample/processed-output waveforms, trim/reverse and sample import.
- Dedicated 20-mode distortion, editable preset name/creator/tags, local factory replacements and hybrid creation.
- Expanded single/multiband dynamics, stereo/Ping-pong/Tap delay, shaped convolution, seven-band EQ with filter/slope choices.
- Frequency-selective sidechain/pumping and trance gate with 15 editable patterns.
- Native oscillator/filter/envelope/modulation controls and the existing granular/wavetable/arp/sequencer engines.

See [stronger 0.7.0 distortion](docs/DISTORTION_DRIVE_UPDATE.md), [Heritage 200 additional bank](docs/HERITAGE_200_UPDATE.md), [0.6.3 bass update](docs/BASS_VOICING_UPDATE.md), [0.6.2 acid cutoff fix](docs/ACID_CUTOFF_UPDATE.md), [0.6.1 acid, bass and choir changes](docs/ACID_BASS_AND_CHOIR_UPDATE.md), [0.6.0 sound and workflow changes](docs/CREATIVE_WORKFLOW_UPDATE.md), [level and control changes](docs/CONTROL_AND_LEVEL_UPDATE.md), [studio features](docs/STUDIO_FEATURES.md), [factory bank status](docs/STUDIO_FACTORY_BANK.md), [testing instructions](docs/TESTING_STUDIO_BUILD.md), and [native interface screenshots](docs/HARDWARE_INTERFACE.md).

## Windows build

Requires Visual Studio 2022 Desktop C++ workload, CMake and Git. From the project root in PowerShell:

```powershell
.\scripts\build-windows.ps1
ctest --test-dir native-vst3/build -C Release --output-on-failure
```

For an installer/archive, install Inno Setup 6 and run `scripts/package-windows.ps1`. GitHub Actions also builds, tests and packages the Windows x64 outputs on branch pushes. Use the result for the exact commit you checked out; a running workflow is not a successful Windows validation. FL Studio routing, compatibility and listening tests remain separate host checks.

The [test guide](docs/TESTING_STUDIO_BUILD.md) explains where to download the installer and how to exercise the new features. Existing user presets and project parameters are retained. The previous factory bank is retired; Restore Factory restores the 160 current factory entries. See [Skywave Pad](docs/AIRWAVE_PAD.md) for voicing, LFO bypass and long-envelope controls, and [new trance leads](docs/TRANCE_LEADS.md) for the eight leads.

See [groove bank additions](docs/GROOVE_BANK.md) for the new rhythmic, bass and house patches.

See [further melodic interpretations](docs/ANTHEM_BANK.md) for the newest leads, arps and performance controls.

[Save As and material finish](docs/SAVE_AS_AND_MATERIALS.md): BANK → SAVE AS... writes a portable `.sipreset` to your chosen folder. The updated interface uses deep violet anodised metal, brushed knob caps and smoked displays.
