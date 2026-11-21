# SPACE INFINITY 0.9.2 — preset browsing and sample portamento

Clicking a preset row in BANK now loads it immediately. The previous/next arrows
follow the same category-grouped order as the header selector, starting from the
last loaded preset. Restoring a project, reordering a bank or reopening the editor
keeps navigation anchored by preset identity. Search/tag filtering alone does not
change the sound. The arrows traverse the full grouped selector, including user
presets; they wrap at either end.

Sample playback supports portamento. Adjusting PORTAMENTO while a sample is in
Poly selects Mono automatically. Mono now remembers the last played pitch across
note gaps, including when a sample has finished. Legato retains overlap-only
glide. Sample root, octave, reverse playback and pitch-wheel scaling remain in
the playback-rate path. Zero time gives immediate pitch changes. Preset loads,
voice-mode changes, preparation and All Notes Off reset the remembered pitch;
different MIDI channels and source engines do not inherit it.

Granular and sample-loop EDIT panels share the editor's popup owner. Opening
BANK, the preset selector or a different page dismisses the current edit panel.
The filter EDIT panel uses the same lifecycle.

Close FL Studio, extract the installer ZIP and run Setup, then load a fresh
instance. Factory preset recipes and optional banks are unchanged from 0.9.1;
your saved sounds and local replacements are retained. Earlier sound-bank audio
reports are included as historical evidence; the current native report covers
these navigation, popup and portamento changes.
