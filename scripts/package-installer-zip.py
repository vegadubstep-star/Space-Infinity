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
for optional in ['RHYTHM_AND_METAL_UPDATE.md', 'ANALOGUE_AND_PARITY_UPDATE.md',
                 'SERUM2_FEATURE_AUDIT.md', 'audit/NATIVE_PARAMETER_INVENTORY.json',
                 'CREATIVE_WORKFLOW_UPDATE.md', 'CHOIR_SAMPLE_PROVENANCE.json', 'ACID_BASS_AND_CHOIR_UPDATE.md', 'ACID_CUTOFF_UPDATE.md', 'BASS_VOICING_UPDATE.md', 'USER_CHOIR_PROVENANCE.json', 'CONTROL_AND_LEVEL_UPDATE.md', 'TRANCE_FACTORY_BANK.md', 'TRANCE_FACTORY_BANK_RESEARCH.md',
                 'TRANCE_PRESET_CATALOGUE.md', 'TRANCE_SAMPLE_PROVENANCE.json']:
    if (root / optional).is_file():
        files.append(optional)
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
