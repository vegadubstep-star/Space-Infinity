# Factory bank reset

The previous 119 factory sounds and their refinement tables have been removed at the owner’s request. The factory list now contains **Init** and **Airwave Pad**. Init is a dry oscillator sound with full sustain and no LFO depth, gate, pumping or effects. [Airwave Pad](AIRWAVE_PAD.md) is the first replacement, programmed from the owner’s specification.

Existing bank indexes retire all previous factory IDs F0–F119. Init uses F120 and Airwave Pad uses F121, so old identities are never reassigned. User-saved files and their ordering remain intact; Init is inserted when an older bank is opened. Restore Factory restores Init and Airwave Pad and preserves user sounds.

Saved DAW projects and user presets still recall their stored parameters and embedded audio; deleting a factory listing does not rewrite existing music projects.
