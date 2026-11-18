# Hardware interface redesign

The current 0.8.0 editor defaults to 1280 × 760, with dual visible LFOs,
amp/filter graphs, a recessed logo badge and the pitch wheel beside the sub.
The screenshots below are native captures. The dated research and early
validation record farther down are historical; current release details are in
[the 0.8.0 update](TRACK_READY_080_UPDATE.md).

![Native Main page](images/hardware-main.png)

![Native FX page](images/hardware-fx.png)

## Visual research

Reviewed 11 November 2026:

- [u-he Diva official product page and interface screenshots](https://u-he.com/products/plug-ins/diva/index.html): consistent lighting, readable printed scales, sculpted knobs, distinct material zones and wood end cheeks. SPACE INFINITY uses this hardware vocabulary with its own layout and artwork.
- [Davies Molding 1900-H instrument knob](https://daviesmolding.com/catalog/series/knobs/control/1900-h/): fluted thermoset skirts and a clear physical pointer inspired the new rotary controls.
- [NKK illuminated pushbuttons](https://www.nkkswitches.com/products/illuminated-pushbutton/): sculptured caps, recessed mounting and restrained status illumination informed the switch treatment.
- [ambientCG materials and licence](https://docs.ambientcg.com/license/): CC0 texture source considered for metal and wood surfaces. Photographic PBR maps are not bundled in this implementation; original, deterministic procedural grain keeps the fine surface detail consistent at every editor size.
- [Poly Haven asset FAQ](https://docs.polyhaven.com/en/faq): alternative CC0 material source reviewed.

## Implemented in the native editor

- Purple anodised-metal panels with fixed-light glare, charcoal metal modules, subtly brushed surface grain, walnut-coloured end cheeks, machined screws and folded chassis rails.
- Fluted rotary skirts, layered contact shadows, fixed-light cap reflections, inset pointer marks, silk-screened scales and satin-metal secondary knobs.
- Fader slots with printed calibration, shaded caps and amber index lines; illuminated slide switches and sculpted mode buttons.
- Recessed programme and waveform displays; coordinated amber/ivory display traces and typography.
- Smaller oscillator displays leave space for physical fine-tune, width and voice-count knobs. Default size is 1180 x 820; the existing 800 x 760 through 1500 x 950 resize range remains available.
- Static chassis, panel and material rendering is cached per editor size/page. Animated traces and controls can repaint without rebuilding the wood, grain, gradients and seams each frame.

Parameters and attachments remain connected to the existing native controls. DSP algorithms, factory patches, saved parameter IDs and preset navigation are unchanged by this visual work. The oscillator display remains a parameter-based waveform preview, not an oscilloscope capture of the audio output.

## Reproducing visual checks

On Windows, build with the existing scripts. Native tests are also available on Linux by configuring with `-DSPACE_INFINITY_BUILD_TESTS=ON` and the JUCE desktop dependencies installed. Run `SpaceInfinitySmoke --ui-only` on a display to write real native Main/FX screenshots at 800, 1000, 1180 and 1500 pixels and check visible controls remain inside the editor.

No u-he, Davies or NKK graphics, logos, skin files or product photographs are included in the plugin. Hardware materials and controls use original JUCE drawing code in `Source/HardwareTheme.h`. The owner-supplied INFINITY logo is adapted as a transparent PNG for the header.

## Validation on 11 November 2026

- Built with JUCE 8.0.7, GCC 14 and CMake Release on Linux; native Standalone and VST3 targets compile. Launched the actual Standalone application on X11.
- Native Main and FX screenshot/bounds checks pass at 800 x 760, 1000 x 790, 1180 x 820 and 1500 x 950.
- ExtendedCore, AnalogueProbe, isolated preset-management/navigation tests and extended native engine smoke tests pass. The last two can also be invoked as `SpaceInfinitySmoke --preset-tests` and `SpaceInfinitySmoke --extended-tests`; they are separate CTest entries so an earlier audition failure does not prevent them from running.
- The initial redesign audition stopped at **Glass Pizzicato**, peak **0.0000525983**, below the existing **0.0001** minimum. That patch was subsequently removed at the owner's request. The new 119-preset audition reaches **Frozen Interlude**, peak **0.0000587209**, which also fails the minimum. No other patch was removed and the threshold was not changed. This remains a release blocker.
- A new Windows CI run is required for this revision. Windows/FL Studio testing was not performed in the Linux environment. CI now retains the audition/screenshots artifact after test failures while the release package remains gated on successful validation.

`hardware-main.png` and `hardware-fx.png` are native component renders from the built editor. `hardware-standalone.png` captures the running Linux application, including its native window frame; it is not a generated design concept.

## Current studio revision

The owner subsequently requested purple gleaming metal and supplied the INFINITY logo. The updated interface, live sampler displays, expanded compressor/delay/convolver, EQ shapes/slopes, frequency-selective trance gate/sidechain, and all 119 refreshed factory presets are described in [STUDIO_FEATURES.md](STUDIO_FEATURES.md). The quiet sampled-preset issue recorded above is resolved by correcting unsuitable high/band-pass settings; the complete seven-test native suite and 119-preset audition now pass. Windows/FL Studio validation remains a separate check.
