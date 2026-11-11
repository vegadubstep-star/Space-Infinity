# Preset arrow navigation

Previous/Next now resolve the currently loaded preset's stable key in the bank on every click, load its neighbour, and immediately refresh the menu. Previously the arrows used the ComboBox row, which could still refer to an earlier sound before the editor timer caught up after a bank load or host state restore. A sorted bank could also leave that row pointing at another sound.

Regression coverage invokes the real editor arrow callbacks before timer refresh after a bank load, state restore and bank sort, then checks parameter editing, user presets, editor reopening and wraparound. The earlier wraparound test now loads the sound into the processor rather than changing only the menu label.

Windows compilation and native tests run through the existing Windows CI workflow. FL Studio verification remains a manual check; source changes alone do not establish host compatibility.
