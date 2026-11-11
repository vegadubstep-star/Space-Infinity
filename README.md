# INFINITY / SPACE INFINITY

Native JUCE Windows x64 synthesizer for FL Studio, with the owner's INFINITY wordmark and purple metallic hardware interface. The VST3 keeps the existing **SPACE INFINITY** host identity.

Current development branch: **codex/windows-build**.

- 119 refreshed factory presets; persistent user preset bank and stable preset identities.
- Live loaded-sample/processed-output waveforms, trim/reverse and sample import.
- Expanded single/multiband dynamics, stereo/Ping-pong/Tap delay, shaped convolution, seven-band EQ with filter/slope choices.
- Frequency-selective sidechain/pumping and trance gate with 15 editable patterns.
- Native oscillator/filter/envelope/modulation controls and the existing granular/wavetable/arp/sequencer engines.

See [studio features](docs/STUDIO_FEATURES.md), [factory patch treatments](docs/STUDIO_FACTORY_BANK.md), [testing instructions](docs/TESTING_STUDIO_BUILD.md), and [native interface screenshots](docs/HARDWARE_INTERFACE.md).

## Windows build

Requires Visual Studio 2022 Desktop C++ workload, CMake and Git. From the project root in PowerShell:

```powershell
.\scripts\build-windows.ps1
ctest --test-dir native-vst3/build -C Release --output-on-failure
```

For an installer/archive, install Inno Setup 6 and run `scripts/package-windows.ps1`. GitHub Actions also builds, tests and packages the Windows x64 outputs on branch pushes. Use the result for the exact commit you checked out; a running workflow is not a successful Windows validation. FL Studio routing, compatibility and listening tests remain separate host checks.

The [test guide](docs/TESTING_STUDIO_BUILD.md) explains where to download the installer and how to exercise the new features. Existing user presets and project parameters are retained; select a factory preset again to hear its refreshed version.
