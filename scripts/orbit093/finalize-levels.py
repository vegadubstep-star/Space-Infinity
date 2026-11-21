"""Apply or verify native 48-preset, 3-register/9-corner level measurements."""
import argparse
import hashlib
import json
import math
import zipfile
from pathlib import Path

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('report', type=Path)
parser.add_argument('--verify', action='store_true')
args = parser.parse_args()
root = Path(__file__).resolve().parents[2]
bank = root / 'native-vst3/Presets/UserVoices093'
rows = json.loads(args.report.read_text())['presets']
assert len(rows) == 48 and len({r['key'] for r in rows}) == 48
assert all(r['registers_checked'] == 3 and r['macro_corners_checked'] == 9 for r in rows)
assert all(r['maximum_pre_safety_peak'] > 1e-5 and r['maximum_true_peak_4x'] > 1e-5 for r in rows)
if not args.verify:
    trims = {}
    for row in rows:
        change = 20 * math.log10(min(.90 / row['maximum_pre_safety_peak'], .93 / row['maximum_true_peak_4x']))
        trims[row['key'].rsplit('_', 1)[1]] = round(max(-24, min(24, row['presetTrim'] + change)), 5)
    (bank / 'CALIBRATION.json').write_text(json.dumps(trims, indent=2) + '\n')
    print('Calculated 48 trims; rebuild the bank, then rerender before --verify.')
else:
    with zipfile.ZipFile(bank / 'SPACE_INFINITY_Orbit_Voices.zip') as archive:
        manifest = json.loads(archive.read('Bank.json'))
        shipped = {r['id']: r for r in manifest['presets']}
    trims = json.loads((bank / 'CALIBRATION.json').read_text())
    for row in rows:
        patch_id = row['key'].rsplit('_', 1)[1]
        assert shipped[patch_id]['sha256'] == row['source_sha256'], 'Measurements do not match shipped preset bytes'
        assert abs(trims[patch_id] - row['presetTrim']) < .00002, 'Measured trim differs from shipped calibration'
    errors = [r['name'] for r in rows if r['maximum_pre_safety_peak'] > .951 or r['maximum_true_peak_4x'] >= .999]
    document = dict(version='0.9.3', presets_measured=48, sample_sources=40,
                    sample_rate=48000, registers=[-12, 0, 12], full_velocity=True,
                    non_bass_chord=[48, 55, 60, 64], bass_note=36,
                    macro_corners=9, description='Eight independent OPEN/SPACE/BODY corners plus all six performance macros at maximum; 2 seconds per case.',
                    true_peak_oversampling=4, target_pre_safety_peak=.90,
                    target_true_peak=.93,
                    maximum_pre_safety_peak=max(r['maximum_pre_safety_peak'] for r in rows),
                    maximum_true_peak_dbfs=20 * math.log10(max(r['maximum_true_peak_4x'] for r in rows)),
                    bank_sha256=hashlib.sha256((bank / 'SPACE_INFINITY_Orbit_Voices.zip').read_bytes()).hexdigest(),
                    errors=errors, presets=rows,
                    limits='Measured notes/chords and macro corners only. Output safety remains enabled; denser playing, additional processing and edits can change levels. Peak calibration does not imply identical perceived loudness.')
    (root / 'docs/audit/SAMPLER_FULL_RANGE_093_AUDIO_LEVELS.json').write_text(json.dumps(document, indent=2) + '\n')
    assert not errors, errors
    print(f"Verified 48 presets: pre-safety {document['maximum_pre_safety_peak']:.6f}, 4x true peak {document['maximum_true_peak_dbfs']:.3f} dBFS")
