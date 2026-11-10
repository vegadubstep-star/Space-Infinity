# Windows native build and test checklist

## Prerequisites
Visual Studio 2022 with C++ Desktop workload, CMake 3.22+, Git, Internet access for `JUCE 8.0.7` FetchContent.

PowerShell from project root:

```powershell
.\scripts\build-windows.ps1
```

Equivalent manual commands:

```powershell
cmake -S native-vst3 -B native-vst3/build -G "Visual Studio 17 2022" -A x64
cmake --build native-vst3/build --config Release
```

Expected VST3 location IF build succeeds: `native-vst3/build/SpaceInfinity_artefacts/Release/VST3/`. The standalone application location is in the matching `Standalone` subfolder, if supported.

## Functional validation — not yet completed
- [ ] Native JUCE project configures successfully on Windows 64-bit
- [ ] C++ project compiles and links with JUCE
- [ ] Plugin can be scanned by DAW (FL Studio), loaded and unloaded repeatedly
- [ ] MIDI notes audibly play; note-offs work; all-notes-off/panic is handled
- [ ] Sustained chords/polyphony, voice stealing and velocity response
- [ ] Silence on stopped transport if no active voices, no hanging notes
- [ ] Audio CPU/peak levels are stable on 44.1/48/96 kHz and 64/128/512 sample buffers
- [ ] Effects bypass, parameter smoothing, no crashes/noise on knob changes
- [ ] Host automation and saving/loading projects/presets work
- [ ] Interface scaling from minimum to maximum; correct pointer and keyboard focus behaviour
- [ ] Screenshots captured from running *native* VST3 or standalone, clearly labeled
- [ ] Plugin validation using pluginval if available, and report logs

## Known code issues to investigate first
- `processBlock()` currently creates a temporary audio buffer with `makeCopyOf`, which may cause heap allocations in the audio thread; preallocate and reuse scratch buffers instead.
- Delay feedback and chorus/reverb need CPU and audible quality validation.
- Current native JUCE editor is not the photorealistic design.
- The browser prototype's matrix and some modulation shapes are not fully wired to DSP.
- No ROM parser, sample mapping, firmware emulation, or original JV preset reader exists in this package.