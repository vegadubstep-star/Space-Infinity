# 0.6.1 validation

The native bank remains 160 sounds plus Init. Exactly 56 designs changed: 35 basses, ten acids, ten owner-recording choir stabs and the Choir Spiral sequence. The other 104 design hashes remain unchanged. Bass presets generate no MIDI patterns. All nine supplied recordings are used; originals, root decisions and source/output hashes are retained in USER_CHOIR_PROVENANCE.json.

Distortion tests cover all 17 modes and three pre/post-filter positions with HQ on, finite output, useful driven response, dry bypass and state recall. A quiet-input sine produced 20.8% THD in the new driven example versus 0.17% in the old setting. The high-frequency hard-clip fixture measured alias fractions 0.0959088 without HQ and 0.00484487 with HQ at matched power. These are specific engineering fixtures, not blanket subjective quality claims.

The Analogue Lab test compares level-matched waveforms, so its change cannot pass through loudness alone. It also checks neutral/tiny-drive continuity and master bypass. The WARM UI button activates the actual analogue routing; MAIN portamento drag selects Legato from Poly, while host automation preserves the chosen voice mode.

All eighteen native suites include repeated-note level stability, old-state defaults, source/parameter recall, bass mono compatibility and UI bounds at four sizes. The portable choir pack validator requires ten embedded-sample .sipreset files and ten stereo 48 kHz WAV hits, verifies its ZIP and writes content hashes. Windows publication additionally runs installer/uninstaller and installed/archive binary identity checks.

Final native auditions measured -29.60 to -27.86 LUFS (1.74 LU spread), with maximum raw chord peak 0.7800004. Measurements are recorded in TRANCE_LOUDNESS_061.json. Rendering uses fixed audition notes and a second chord pass for headroom. Actual music and FL Studio listening remain the owner's evaluation; these authored sounds are not represented as exact artist patches.
