# SPACE INFINITY 0.6.2 — acid cutoff automation

The ten acid presets now use a dedicated Acid Resonant filter response. Its resonant peak follows the main CUTOFF control, with internal headroom so the nonlinear feedback loop keeps its character. The existing six filter responses remain available at their original indices.

The 0.6.1 acid patches combined large filter-envelope excursions with very high post-filter distortion. This flattened the moving resonance and reached the high-cutoff limit early. Positive envelope excursions now approach the available high-frequency headroom smoothly, rather than clipping against a fixed ceiling. The revised patches use saw, square and pulse foundations, more useful envelope excursions and driven tube/diode/clipped colours with enough headroom for the filter movement to remain audible. Wavefolded/FM variants are replaced by cutoff-focused acid voices.

In MAIN, automate CUTOFF (host parameter Filter Cutoff). RESONANCE sets the squelch. FILTER ENV AMOUNT controls the per-note excursion and FILTER DECAY controls its speed. PRE DRIVE colours the filter loop; FX INPUT and DRIVE add additional grit. Overlap mono notes for slides and vary velocity for accents. Basslines remain unsequenced.

Load the new factory acid presets in a fresh instance to compare. Existing saved projects retain their saved filter and distortion settings. The cutoff parameter ID, units and host mapping are retained for existing automation.

Native tests measure the resonant peak at several cutoff settings and sample rates, stress fast cutoff/drive sweeps and feedback decay, and compare low/open cutoff spectra of every actual driven acid preset. Auditions include rising and falling automated sweeps with accented notes. Preset level matching also checks full-sweep headroom.

Control roles were checked against [Roland's TB-303 panel guide](https://static.roland.com/assets/media/pdf/TB-303-Graphic-Tour.pdf). The new filter is an authored nonlinear four-stage response with a voiced output tap, not a measured or component-exact TB-303 reproduction. [Tim Stinchcombe's diode-filter analysis](https://www.timstinchcombe.co.uk/index.php?pge=diode) explains why a real diode ladder differs from a generic transistor ladder; this update makes no circuit-equivalence claim.

Close FL Studio, extract the installer ZIP and run Setup. Then load a fresh instance and select a factory Acid preset.
