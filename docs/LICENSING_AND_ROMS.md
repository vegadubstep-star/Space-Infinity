# ROM, emulator and framework licence notes

**Important:** The user states their JV-880 ROM files are legally obtained. The project archive intentionally excludes ROMs, the third-party binary VST and decrypted PCM content.

Reference ROM file names in the user's **separate private archive** (file existence confirmed previously; not included here):
- `jv880_waverom1.bin`, `jv880_waverom2.bin`
- `jv880_rom1.bin`
- `jv880_rom2_1.0.0.bin`, `jv880_rom2_1.0.1.bin` — alternate firmware versions; choose after engine verification, not automatically combined
- four descrambled waveform WAV files

ROM possession by the user does not resolve redistribution or derivative-use rights. Do not embed ROM bytes or decrypted samples in binaries/archives/repositories. Consider user-selectable private ROM location at runtime, if permitted.

Potential external source: https://github.com/giulioz/jv880_juce (VirtualJV, based on Nuked SC55). Its LICENSE applies a noncommercial MAME-derived licence to the original emulator code: **redistributions may not be sold or used in commercial products or activity**. Do not simply copy this engine into a proprietary/commercial product. Review license fully and obtain any needed permission or design an independent compatible engine. The project does not include VirtualJV's source code.

JUCE modules are dual-licensed under AGPLv3 and a commercial JUCE licence: https://github.com/juce-framework/JUCE/blob/master/LICENSE.md . Review your product distribution model and obtain appropriate permissions/terms. The included native starter uses CMake FetchContent to pull JUCE 8.0.7 during build; no JUCE sources are packaged.

These are engineering cautions, not legal advice.