"""Validate the user-visible 0.9 installer wrapper, including its new sound bank."""
import hashlib
import io
import json
from pathlib import Path
import sys
import zipfile

archive = Path(sys.argv[1])
prefix = archive.name.removesuffix('_Installer.zip')
version = prefix.removeprefix('SPACE_INFINITY_').removesuffix('_Windows_x64')
folder = prefix + '/'
required = {'READ_ME_FIRST.md', 'CHARACTER_FILTER_SAMPLER_090_UPDATE.md',
            'SPACE_INFINITY_Lush_Trance_20.zip',
            'SPACE_INFINITY_Lush_Trance_20.zip.sha256',
            'audit/CHARACTER_090_NATIVE_TESTS.txt', 'audit/LEADS_090_AUDIO_LEVELS.json',
            'audit/NATIVE_PARAMETER_INVENTORY.json', 'INSTALLER_SHA256.txt',
            prefix + '_Setup.exe'}
if tuple(map(int, version.split('.'))) >= (0, 9, 1):
    required.update({'STUDIO_RESEARCH_091_UPDATE.md', 'research/STUDIO_RESEARCH_091.md',
                     'audit/STUDIO_RESEARCH_091_AUDIO_LEVELS.json',
                     'audit/STUDIO_RESEARCH_091_RECIPES.json',
                     'audit/STUDIO_RESEARCH_091_FACTORY_PARITY.json',
                     'audit/STUDIO_RESEARCH_091_NATIVE_TESTS.txt'})
with zipfile.ZipFile(archive) as outer:
    assert outer.testzip() is None
    assert {folder + name for name in required} <= set(outer.namelist()), 'Missing new bank, instructions or audit'
    instructions = outer.read(folder + 'READ_ME_FIRST.md').decode('utf-8-sig')
    assert version in instructions and 'SPACE_INFINITY_Lush_Trance_20.zip' in instructions
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
    assert inventory['native_version'] == version and inventory['parameter_count'] == 1223
    parameters = {row['id']: row for row in inventory['parameters']}
    assert parameters['filterMode']['maximum'] == 6
    assert len(parameters['characterFilterMode']['choices']) == 9
    if tuple(map(int, version.split('.'))) >= (0, 9, 1):
        revised = json.loads(outer.read(folder + 'audit/STUDIO_RESEARCH_091_AUDIO_LEVELS.json'))
        assert revised['presets_measured'] == 90 and not revised['errors']
        assert revised['maximum_true_peak_dbfs'] <= -.29
        assert revised['maximum_pre_safety_peak'] <= .951
        assert revised['maximum_open_difference_lu'] <= 1
        recipes = outer.read(folder + 'audit/STUDIO_RESEARCH_091_RECIPES.json')
        assert revised['text_sha256_line_endings'] == 'LF'
        assert hashlib.sha256(recipes.replace(b'\r\n', b'\n')).hexdigest() == revised['design_sha256']
        assert len(json.loads(recipes)['presets']) == 90
        assert revised['sweep_engine_modes_checked'] == [0, 1]
        bass = revised['additional_bass_registers']
        assert bass['notes'] == [36, 43, 48, 55] and bass['engine_modes'] == [0, 1]
        assert len(bass['presets']) == 35 and all(r['final_predicted_peak'] < .95 for r in bass['presets'])
        parity = json.loads(outer.read(folder + 'audit/STUDIO_RESEARCH_091_FACTORY_PARITY.json'))
        assert parity['presets_verified'] == 90 and not parity['errors']
        assert parity['recipe_sha256'] == revised['design_sha256']
        native_tests = outer.read(folder + 'audit/STUDIO_RESEARCH_091_NATIVE_TESTS.txt').decode('utf-8-sig')
        assert 'SPACE INFINITY ' + version in native_tests and '100% tests passed' in native_tests
        assert 'CTest exit code: 0' in native_tests
        assert 'Research 091' in instructions
print('Installer wrapper: current instructions, native banks/recipes, audio evidence, 1223 parameters and checksums passed')
