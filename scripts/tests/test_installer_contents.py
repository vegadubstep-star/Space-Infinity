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
if tuple(map(int, version.split('.'))) >= (0, 9, 2):
    required.update({'BROWSING_SAMPLE_GLIDE_092_UPDATE.md', 'audit/BROWSING_SAMPLE_GLIDE_092_NATIVE_TESTS.txt'})
if tuple(map(int, version.split('.'))) >= (0, 9, 3):
    required.update({'SAMPLER_FULL_RANGE_093_UPDATE.md', 'ORBIT_SAMPLES_093_PROVENANCE.json',
                     'SPACE_INFINITY_Orbit_Voices.zip', 'SPACE_INFINITY_Orbit_Voices.zip.sha256',
                     'SPACE_INFINITY_Retired_093_Backup.zip', 'audit/SAMPLER_FULL_RANGE_093_NATIVE_TESTS.txt',
                     'audit/SAMPLER_FULL_RANGE_093_AUDIO_LEVELS.json', 'audit/SAMPLER_FULL_RANGE_093_CURATION.json'})
if tuple(map(int, version.split('.'))) >= (0, 9, 4):
    required.update({'INSTALLED_BANK_094_UPDATE.md', 'audit/INSTALLED_BANK_094_LEVELS.json',
                     'audit/INSTALLED_BANK_094_NATIVE_TESTS.txt', 'audit/INSTALLED_BANK_094_CATALOGUE.json'})
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
        assert manifest['minimum_version'] == ('0.9.4' if tuple(map(int, version.split('.'))) >= (0,9,4) else '0.9.0') and len(manifest['presets']) == 20
        for preset in manifest['presets']:
            assert hashlib.sha256(inner.read(preset['file'])).hexdigest() == preset['sha256']
    audit = json.loads(outer.read(folder + 'audit/LEADS_090_AUDIO_LEVELS.json'))
    assert audit['presets_measured'] == 27 and not audit['errors']
    assert audit['maximum_true_peak_dbfs'] <= -.29
    inventory = json.loads(outer.read(folder + 'audit/NATIVE_PARAMETER_INVENTORY.json'))
    assert inventory['native_version'] == version and inventory['parameter_count'] == (1228 if tuple(map(int, version.split('.'))) >= (0,9,4) else 1227 if tuple(map(int, version.split('.'))) >= (0, 9, 3) else 1223)
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
        assert 'SPACE INFINITY 0.9.1' in native_tests and '100% tests passed' in native_tests
        assert 'CTest exit code: 0' in native_tests
        assert 'Research 091' in instructions
    if tuple(map(int, version.split('.'))) >= (0, 9, 2):
        current_tests = outer.read(folder + 'audit/BROWSING_SAMPLE_GLIDE_092_NATIVE_TESTS.txt').decode('utf-8-sig')
        assert 'SPACE INFINITY 0.9.2' in current_tests and '100% tests passed' in current_tests
        assert 'CTest exit code: 0' in current_tests
        assert 'Mono glides' in instructions
    if tuple(map(int, version.split('.'))) >= (0, 9, 3):
        assert 'automatically' in instructions and 'saved/manual order' in instructions
        native = outer.read(folder + 'audit/SAMPLER_FULL_RANGE_093_NATIVE_TESTS.txt').decode('utf-8-sig')
        assert 'SPACE INFINITY 0.9.3' in native and '100% tests passed' in native and 'CTest exit code: 0' in native
        data = outer.read(folder + 'SPACE_INFINITY_Orbit_Voices.zip')
        assert hashlib.sha256(data).hexdigest() == outer.read(folder + 'SPACE_INFINITY_Orbit_Voices.zip.sha256').decode().split()[0]
        with zipfile.ZipFile(io.BytesIO(data)) as inner:
            assert inner.testzip() is None
            manifest = json.loads(inner.read('Bank.json'))
            assert len(manifest['presets']) == 48 and manifest['minimum_version'] == ('0.9.4' if tuple(map(int, version.split('.'))) >= (0,9,4) else '0.9.3')
            for preset in manifest['presets']:
                assert hashlib.sha256(inner.read(preset['file'])).hexdigest() == preset['sha256']
        levels = json.loads(outer.read(folder + 'audit/SAMPLER_FULL_RANGE_093_AUDIO_LEVELS.json'))
        assert levels['presets_measured'] == 48 and not levels['errors']
        assert levels['maximum_true_peak_dbfs'] < 0 and levels['maximum_pre_safety_peak'] <= .951
        curation = json.loads(outer.read(folder + 'audit/SAMPLER_FULL_RANGE_093_CURATION.json'))
        assert curation['existing_presets_measured'] == 380 and len(curation['retired']) >= 7
        assert curation['backup_sha256'] == hashlib.sha256(outer.read(folder + 'SPACE_INFINITY_Retired_093_Backup.zip')).hexdigest()
    if tuple(map(int, version.split('.'))) >= (0, 9, 4):
        tests = outer.read(folder+'audit/INSTALLED_BANK_094_NATIVE_TESTS.txt').decode('utf-8-sig')
        assert 'SPACE INFINITY 0.9.4' in tests and '100% tests passed' in tests and 'CTest exit code: 0' in tests
        matched = json.loads(outer.read(folder+'audit/INSTALLED_BANK_094_LEVELS.json'))
        assert matched['presets_measured'] == 421 and matched['sampled_orbit_presets'] == 48 and not matched['errors']
        assert matched['maximum_deviation_lu'] <= 1.001 and matched['maximum_default_true_peak_dbfs'] < 0 and matched['maximum_stress_true_peak_dbfs'] < 0
        catalogue = json.loads(outer.read(folder+'audit/INSTALLED_BANK_094_CATALOGUE.json'))
        assert catalogue['active_presets'] == len(catalogue['settings']) == 421
        for name, digest in matched['bank_hashes'].items():
            assert hashlib.sha256(outer.read(folder+name)).hexdigest() == digest == catalogue['banks'][name]['sha256']
        assert parameters['presetReferenceGain']['host_default'] == 0
print('Installer wrapper: current instructions, banks, audio evidence, parameters and checksums passed')
