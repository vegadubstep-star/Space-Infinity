"""Wrap a previously verified release installer without rebuilding its binary."""
import argparse
import hashlib
from pathlib import Path
import zipfile

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('directory', type=Path)
parser.add_argument('version')
options = parser.parse_args()
root = options.directory
prefix = f'SPACE_INFINITY_{options.version}_Windows_x64'
setup = f'{prefix}_Setup.exe'
checksums = {}
for line in (root / 'SHA256SUMS.txt').read_text(encoding='utf-8-sig').splitlines():
    digest, name = line.split(None, 1)
    checksums[name.strip()] = digest.lower()
expected = checksums[setup]
actual = hashlib.sha256((root / setup).read_bytes()).hexdigest()
if actual != expected:
    raise SystemExit('Installer checksum does not match the verified release.')
archive = root / f'{prefix}_Installer.zip'
folder = prefix + '/'
files = [setup, 'READ_ME_FIRST.md', 'TEN_CONTROL_UPDATES.md',
         'THIRD_PARTY_SAMPLES.md', 'SAMPLE_PROVENANCE.json', 'CC0-1.0.txt']
for optional in ['audit/TRACK_READY_080_AUDIO_LEVELS.json', 'audit/TRACK_READY_080_NATIVE_TESTS.txt', 'audit/TRACK_READY_080_SIMILARITY.json', 'TRACK_READY_080_UPDATE.md', 'FLAT_CHOIR_PROVENANCE.json', 'MODULATION_CHORUS_MELT_UPDATE.md', 'research/CHORUS_DIMENSION_RESEARCH.md', 'HARDWARE_TIMING_NOISE_UPDATE.md', 'SPACE_INFINITY_Heritage_200.zip', 'SPACE_INFINITY_Heritage_200.zip.sha256', 'HERITAGE_200_UPDATE.md', 'DISTORTION_DRIVE_UPDATE.md', 'RHYTHM_AND_METAL_UPDATE.md', 'ANALOGUE_AND_PARITY_UPDATE.md',
                 'SERUM2_FEATURE_AUDIT.md', 'audit/NATIVE_PARAMETER_INVENTORY.json',
                 'CREATIVE_WORKFLOW_UPDATE.md', 'CHOIR_SAMPLE_PROVENANCE.json', 'ACID_BASS_AND_CHOIR_UPDATE.md', 'ACID_CUTOFF_UPDATE.md', 'BASS_VOICING_UPDATE.md', 'USER_CHOIR_PROVENANCE.json', 'CONTROL_AND_LEVEL_UPDATE.md', 'TRANCE_FACTORY_BANK.md', 'TRANCE_FACTORY_BANK_RESEARCH.md',
                 'TRANCE_PRESET_CATALOGUE.md', 'TRANCE_SAMPLE_PROVENANCE.json']:
    if (root / optional).is_file():
        files.append(optional)
# These are part of the deliverable, not optional historical documentation.
if tuple(map(int, options.version.split('.'))) >= (0, 9, 0):
    files.extend(['CHARACTER_FILTER_SAMPLER_090_UPDATE.md',
                  'audit/CHARACTER_090_NATIVE_TESTS.txt',
                  'audit/LEADS_090_AUDIO_LEVELS.json',
                  'SPACE_INFINITY_Lush_Trance_20.zip',
                  'SPACE_INFINITY_Lush_Trance_20.zip.sha256'])
if tuple(map(int, options.version.split('.'))) >= (0, 9, 1):
    files.extend(['STUDIO_RESEARCH_091_UPDATE.md',
                  'research/STUDIO_RESEARCH_091.md',
                  'audit/STUDIO_RESEARCH_091_AUDIO_LEVELS.json',
                  'audit/STUDIO_RESEARCH_091_RECIPES.json',
                  'audit/STUDIO_RESEARCH_091_FACTORY_PARITY.json',
                  'audit/STUDIO_RESEARCH_091_NATIVE_TESTS.txt'])
if tuple(map(int, options.version.split('.'))) >= (0, 9, 2):
    files.extend(['BROWSING_SAMPLE_GLIDE_092_UPDATE.md',
                  'audit/BROWSING_SAMPLE_GLIDE_092_NATIVE_TESTS.txt'])
if tuple(map(int, options.version.split('.'))) >= (0, 9, 3):
    files.extend(['SAMPLER_FULL_RANGE_093_UPDATE.md', 'ORBIT_SAMPLES_093_PROVENANCE.json',
                  'audit/SAMPLER_FULL_RANGE_093_NATIVE_TESTS.txt', 'audit/SAMPLER_FULL_RANGE_093_AUDIO_LEVELS.json',
                  'audit/SAMPLER_FULL_RANGE_093_CURATION.json', 'SPACE_INFINITY_Orbit_Voices.zip',
                  'SPACE_INFINITY_Orbit_Voices.zip.sha256', 'SPACE_INFINITY_Retired_093_Backup.zip'])
if tuple(map(int, options.version.split('.'))) >= (0, 9, 4):
    files.extend(['INSTALLED_BANK_094_UPDATE.md', 'audit/INSTALLED_BANK_094_LEVELS.json',
                  'audit/INSTALLED_BANK_094_NATIVE_TESTS.txt', 'audit/INSTALLED_BANK_094_CATALOGUE.json'])
for name in files:
    if not (root / name).is_file():
        raise SystemExit(f'Missing release file: {name}')
with zipfile.ZipFile(archive, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as output:
    for name in files:
        output.write(root / name, folder + name)
    output.writestr(folder + 'INSTALLER_SHA256.txt', f'{expected}  {setup}\n')
with zipfile.ZipFile(archive) as uploaded:
    if uploaded.testzip() is not None:
        raise SystemExit('Installer ZIP integrity check failed.')
    if hashlib.sha256(uploaded.read(folder + setup)).hexdigest() != expected:
        raise SystemExit('Packaged installer differs from the verified installer.')
digest = hashlib.sha256(archive.read_bytes()).hexdigest()
(root / (archive.name + '.sha256')).write_text(
    f'{digest}  {archive.name}\n', encoding='utf-8')
print(f'Verified {archive.name}: {digest}')
