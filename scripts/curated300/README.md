# Curated 300 review

The input was the 421-sound 0.9.4 development palette at source commit
adf5f605082afbd671cfcdcddf00b47657351532. No 0.9.4 installer had shipped.
The running Windows build was cancelled when the owner requested this curation.

ReviewPalette.cpp links against the real native engine and renders all input
sounds at three registers, retaining attack spectrum, full-note spectrum,
amplitude contour, 4x reconstructed peaks and RMS. select-palette.py removes
closest neighbours within explicit musical-role budgets to select 300 sounds.
A final cross-role repair prevents category labels hiding a close string/pluck pair.
It records every removed sound and its closest surviving alternative. These
features support curation; they do not prove subjective perceptual identity.

polish-palette.py records every retained effects chain and conservatively edits
wet returns. Selected acid voices receive filter-envelope-linked warp variants.
review-sample-starts.py decodes the actual WAVs, inspects onsets, restores cropped
lead attacks, retains steady choir segments, and records warp/unison settings.
Source WAV bytes and root pitches remain unchanged.

ExportLibrary.cpp exports native snapshots before curation. install-curation.py
retains the original seven retirements and adds 121 new ones to the normal-view
exclusions. A portable alternatives ZIP uses separate IDs and a recovery tag.
No retired source definition is deleted; personal edits are exempt from hiding.

Render the edited settings plan with scripts/installed094/ReviewInstalled.cpp.
merge-levels.py before merges fresh baseline measurements with previous verified
measurements of unchanged sounds, then computes static reference gains for the
92 edited states only. bake-palette.py writes the plan into actual factory data,
keeps previous shipped hashes and adds the Curated 300 tag. Rebuild the engine
resources and final renderer before rendering the edited bank states again.

verify-source-parity.py proves the 208 other states retain identical audio
parameters, macros, LFO routes and embedded assets. Only metadata serialization
changes, so their existing native level measurements remain valid. Final merging
and scripts/installed094/verify-matching.py require this parity proof whenever an
old measured file hash differs from a newly tagged preset. The release report
explicitly distinguishes fresh final renders from unchanged-audio evidence.

The native installed-bank test covers 300 sounds + Init, all 38 retained sampled
sounds, bank hashes, current settings, repeated startup, optional recovery,
existing-library upgrades and preservation of personal replacements. Browser
checks cover category/search filtering, CLEAR, no-match editing protection,
compact/default and expanded edit views, loading and saved-order navigation.

Raw profiles, snapshots and measurements for this pass are retained under
/workspace/work/curated300. Current release evidence is under docs/audit/CURATED_300_*
and docs/audit/INSTALLED_BANK_094_*. Do not describe the older 421-sound measurements
as fresh renders of this revised palette.
