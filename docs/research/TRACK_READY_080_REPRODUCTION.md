# Reproducing the 0.8.0 bank review

The shipped JSON recipes are the current authoring source. The older
`design-trance-bank.py` and `heritage/design-bank.py` are historical seed-bank
generators; running them directly replaces later voicing, macro and calibration
work. Keep a copy of the current designs before deliberately regenerating seeds.

1. `prepare-flat-choirs.py` reproduces the six new sources from the documented
   CC0 ensemble assets. Build the native synth after changing audio assets or DSP.
2. `polish-track-ready.py INPUT OUTPUT` applies the checked-in character recipes,
   new waves, choir and performance-macro choices while retaining IDs and trims.
   Its `open_compensation_db` fields preserve individually measured transitions.
3. `heritage/build-renderer.py --output /tmp/RenderTrackReady` links the native
   offline renderer against the current Linux build. Do not replace the binary
   while reviewing: its SHA256 is part of the measurement cache key.
4. `review-track-ready.py --renderer /tmp/RenderTrackReady --output REVIEW_DIR`
   renders full-velocity single/repeated notes and chords, plus OPEN endpoints on
   plucks and all three assignable macros raised. Per-voice temporary WAVs are
   deleted after FFmpeg BS.1770 and four-times oversampled true-peak analysis.
5. Run `calibrate-track-ready.py REVIEW_DIR/Review.json --output AUDIT.json`
   without `--write` first. It rejects stale or incomplete measurements. Review
   near-match candidates and change character/articulation where appropriate,
   then rerun the reviewer to refresh changed sounds.
6. Add `--write` to apply calibration. Default `--mode peak` uses all measured
   safe gain up to −0.3 dBTP and the .98 pre-safety peak budget. Optional
   `--mode loudness --target -24` instead stops at a common perceived target;
   this is not the owner's final requested maximum-peak policy.
7. Rerender the calibrated designs, then run `verify-track-ready.py
   REVIEW_DIR/Review.json AUDIT_FINAL.json`. It checks recipe hashes, finite
   audio, headroom, unused gain and OPEN/base loudness differences. Keep the
   final audit with the release source. Complete native and Windows test suites.
8. Export the final Heritage JSON through the native renderer's `--export-only`,
   then use `heritage/package-bank.py`. V2 requires plug-in 0.8.0 and carries the
   prior shipped SHA256 for safe updates of byte-identical, unedited patches.

Peak normalization and equal perceived loudness are different constraints.
The measured phrase and macro conditions are recorded; arbitrary MIDI density,
registers, edited effects or layered instances can change peaks. The native
Clean headroom safety curve is not a true-peak limiter.

The final refinement also records four-register bass and dynamic/held-cutoff
acid fixtures in `scripts/track-ready-headroom-references.json`. Each contains
a sound-recipe hash, measured raw peak, measurement gain/trim, normalized
pre-master peak and source-report hash. Calibration rejects stale references.
These fixtures retain the existing stricter raw-headroom checks. Repeated
convolver trials and a previously observed IR-startup maximum are also budgeted;
asynchronous IR loading can change an early render's peak. `--refine` corrects
measured excesses with a 0.05 dB margin and leaves already safe gains alone.
