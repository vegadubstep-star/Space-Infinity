# SPACE INFINITY — Codex project handoff

**Owner's intent:** a professional, realistically hardware-styled **Windows x64 VST3** synthesizer for FL Studio, combining a working original synth and—only after a licensing review—optional authentic Roland JV-880 sound emulation using the owner's legally obtained ROMs. The user calls it **SPACE INFINITY**.

## Start here

1. On Windows, extract this ZIP and open the extracted `SPACE_INFINITY_Codex_Project` **folder** in Codex desktop. Choose **GPT-6.1 Sol** if available.
2. Ask Codex: **Read `AGENTS.md` and `docs/START_CODEX_TASK.md`; audit and build the native VST3 first.**
3. Do not upload your personal ROM files to a public GitHub repository. They are **not included** in this project.

## Actual contents

- `native-vst3/` — C++ JUCE synth prototype source, **renamed SPACE INFINITY**, only basic controls and effects; **not built or tested as VST3** in this environment.
- `web-prototype/` — actual interactive browser-based Web Audio synth v0.2 with hardware-style UI, waveform/envelope graph displays and simple presets. **Not a VST3.**
- `design/actual-browser-interface.webp` — capture from the browser prototype. This is a screenshot of real running code, **not a VST3 screenshot**.
- `design/user-hardware-colour-reference.webp` — user's supplied reference photo for materials and colours, **not camera angle**.
- `design/concept-top-down-inspiration.webp` — generated style inspiration only; **not real UI code**.
- `docs/` — tasks, build verification, licensing, privacy and ROM integration notes.
- `scripts/build-windows.ps1` — Windows build helper using Visual Studio/CMake; requires internet for JUCE Git dependency.

## Honest current status

- `web-prototype/index.html` is playable in a browser, but its UI and audio modulation are incomplete.
- The native JUCE project is an **independent synth starter**, not a JV-880 ROM player or VirtualJV fork.
- The JUCE native UI is still basic and **does not match the reference design**. It must be implemented, not just overlaid with a picture.
- No actual Windows `.vst3` is bundled. There is no proof yet it loads in FL Studio.
- JUCE and possible JV emulation dependencies have licensing obligations; see `docs/LICENSING_AND_ROMS.md`.

## Build prerequisites

Windows 10/11 x64, Visual Studio 2022 Desktop C++ workload, Git, CMake, Internet to fetch JUCE 8.0.7. The build helper provides the basic CMake steps; the code may need fixes before it compiles. See `docs/WINDOWS_BUILD_AND_VALIDATION.md`.