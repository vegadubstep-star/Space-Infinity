# Orbit Voices authoring

`build-bank.py` builds the portable 48-preset bank from the 40 canonical
`native-vst3/Samples/User093/Infinity_*.wav` files. The provenance document maps
original upload names to those actual renamed files and their SHA-256 hashes.
The source PCM bytes are retained; measured root notes may differ from names
provided with the uploads.

The builder starts from native Init values in the parameter inventory, writes
JUCE binary XML and JUCE MemoryBlock's length-prefixed six-bit encoding for
embedded samples. RFC 4648 base64 is incompatible with this field.

`CALIBRATION.json` contains native preset trims. `finalize-levels.py REPORT`
calculates trims from a native engine report covering three registers and nine
macro corners. Rebuild the bank, rerender, then run
`finalize-levels.py FINAL_REPORT --verify` to record actual level evidence.
The authoring helper and source report were retained in the working session;
the release contains the verified audit, bank hash and individual source hashes.

Bank manifests retain previous shipped hashes so automatic updates can replace
an untouched shipped version while preserving custom sound or metadata edits.
The library backs up replaced bytes. Avoid deleting this history during updates.

Curation uses stable keys, matching names and unchanged source hashes. Seven
Heritage variants are hidden from the normal bank; their underlying files and
factory IDs remain intact for project recall. The separate Retired backup ZIP
uses a new bank ID so it can be imported intentionally without being hidden.
