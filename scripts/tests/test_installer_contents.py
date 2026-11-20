"""Validate the user-visible 0.9 installer wrapper, including its new sound bank."""
import hashlib
import io
import json
from pathlib import Path
import sys
import zipfile

archive = Path(sys.argv[1])
prefix = archive.name.removesuffix('_Installer.zip')
folder = prefix + '/'
required = {'READ_ME_FIRST.md', 'CHARACTER_FILTER_SAMPLER_090_UPDATE.md',
            'SPACE_INFINITY_Lush_Trance_20.zip',
            'SPACE_INFINITY_Lush_Trance_20.zip.sha256',
            'audit/CHARACTER_090_NATIVE_TESTS.txt', 'audit/LEADS_090_AUDIO_LEVELS.json',
            'audit/NATIVE_PARAMETER_INVENTORY.json', 'INSTALLER_SHA256.txt',
            prefix + '_Setup.exe'}
with zipfile.ZipFile(archive) as outer:
    assert outer.testzip() is None
    assert {folder + name for name in required} <= set(outer.namelist()), 'Missing new bank, instructions or audit'
    instructions = outer.read(folder + 'READ_ME_FIRST.md').decode('utf-8-sig')
    assert '0.9.0' in instructions and 'SPACE_INFINITY_Lush_Trance_20.zip' in instructions
    assert 'Delivery remains held' not in instructions, 'Obsolete installation instructions'
    setup = outer.read(folder + prefix + '_Setup.exe')
    assert hashlib.sha256(setup).hexdigest() == outer.read(folder + 'INSTALLER_SHA256.txt').decode('utf-8-sig').split()[0]
    bank = outer.read(folder + 'SPACE_INFINITY_Lush_Trance_20.zip')
    assert hashlib.sha256(bank).hexdigest() == outer.read(folder + 'SPACE_INFINITY_Lush_Trance_20.zip.sha256').decode('utf-8-sig').split()[0]
    with zipfile.ZipFile(io.BytesIO(bank)) as inner:
        assert inner.testzip() is None
        manifest = json.loads(inner.read('Bank.json'))
        assert manifest['minimum_version'] == '0.9.0' and len(manifest['presets']) == 20
        for preset in manifest['presets']:
            assert hashlib.sha256(inner.read(preset['file'])).hexdigest() == preset['sha256']
    audit = json.loads(outer.read(folder + 'audit/LEADS_090_AUDIO_LEVELS.json'))
    assert audit['presets_measured'] == 27 and not audit['errors']
    assert audit['maximum_true_peak_dbfs'] <= -.29
    inventory = json.loads(outer.read(folder + 'audit/NATIVE_PARAMETER_INVENTORY.json'))
    assert inventory['native_version'] == '0.9.0' and inventory['parameter_count'] == 1223
    parameters = {row['id']: row for row in inventory['parameters']}
    assert parameters['filterMode']['maximum'] == 6
    assert len(parameters['characterFilterMode']['choices']) == 9
print('Installer wrapper: current instructions, 20 native presets, 27 audio audits, 1223 parameters and checksums passed')
