# 0.7.1 preview validation

Native binary/source: f8db6e7efd09617118cf270949f7171a10248032.

- All21 native CTest suites passed in229.84seconds, including200 imported-bank recalls, all factory bass/acid/lead/pad checks, analogue, saved-state, effects and UI regressions.
- New noise tests pass for successive identical Retrigger bursts in White/Dark/Bright, Free advancing through idle gaps, saved mode recall and Random fallback for old state.
- Actual attached LFO/FX parsers accept milliseconds/seconds/kHz. Host tempo120→150BPM changes the four-bar label8.00→6.40seconds. Dotted, triplet, free-Hz and ping-pong timing checks pass.
- Main/FX/Gate/Seq bounds and native screenshots pass at800/1000/1180/1500 widths; existing logo DPI snapshots also pass. Actual Main/FX1000 screenshots are in docs/images/Hardware-{MAIN,FX}-071.png.
- All1107 existing parameter definitions/indices remain identical; noiseMode is appended. Factory160 bank and separate Heritage200 ZIP are unchanged.
- Python factory-edit importer test passed.

Windows preview run44163814684/job134324403116 succeeded for the same source. All21 suites passed in274.16seconds; Python import, installer install/uninstall, installed/archive byte identity and ZIP checks passed. Publish was skipped as requested by publish_release=false.

The workflow artifact was downloaded through the connected GitHub tool after the direct signed download was refused. Its full SHA-256 matches GitHub’s digest. Independent readback verifies x64 PE/version0.7.1, unchanged VST3 class IDs, embedded original160 bank, all three new material PNGs plus the approved logo, exact1108-parameter reference and the unchanged Heritage200 bank.

The outer installer ZIP was repackaged once to put the final verification status into README/hardware notes. The Windows-tested Setup is byte-identical. Both original workflow ZIP and final ZIP digests are retained in HARDWARE_TIMING_NOISE_071_WINDOWS.json. No production binary changed after validation.

The final installer ZIP is saved locally and remains unpublished; download delivery is held. Native/Windows smoke checks do not establish listening approval in FL Studio.

Detailed local evidence: /workspace/work/release-verification/v0.7.1/.
