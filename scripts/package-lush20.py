"""Verify native exported presets and package the 20-lead import bank."""
import argparse,hashlib,json,struct,zipfile
import xml.etree.ElementTree as ET
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('export',type=Path);a=p.parse_args();root=Path(__file__).resolve().parents[1];folder=root/'native-vst3/Presets/Lush20';design=json.loads((folder/'Designs.json').read_text());assert len(design['presets'])==20;rows=[]
for r in design['presets']:
 path=Path('Presets')/r['family']/r['category']/(r['id']+' '+r['name']+'.sipreset');data=(a.export/path).read_bytes();assert struct.unpack('<I',data[:4])[0]==0x21324356;xml=ET.fromstring(data[8:8+struct.unpack('<I',data[4:8])[0]]);assert xml.attrib['presetDisplayName']==r['name'] and xml.attrib['creator']==r['creator'];values={c.attrib.get('id'):float(c.attrib['value']) for c in xml if 'value' in c.attrib}
 for key,v in r['settings'].items():assert abs(values[key]-v)<max(1e-4,abs(v)*1e-5),(r['id'],key,v,values[key])
 rows.append(dict(id=r['id'],file=path.as_posix(),name=r['name'],category='Lead',family=r['family'],sha256=hashlib.sha256(data).hexdigest()))
manifest=dict(format='SPACE_INFINITY_BANK_V2',id='lushtrance20',name='Lush Trance 20',minimum_version='0.9.0',creator='Orbital Audio',presets=rows);readme='''LUSH TRANCE 20 — SPACE INFINITY 0.9.0+

In the preset manager select IMPORT and choose this ZIP without extracting it.
Then select the Lush 20 / Lush Trance 20 tag or the Lead category.
The original factory sounds and Heritage 200 remain available.

20 original layered leads with near-zero attack and sustained body.
OPEN brightens and dries the return effects, BODY adds interaction,
HORIZON widens the sound. Several sounds use the A > B oscillator journey.
Creator and tags are editable. SAVE AS keeps the original; REPLACE stores your edit.

Levels use measured native maximum-velocity single notes and chords, all eight
user-macro corners, and 4x true-peak checks for base/all-open phrases. More notes,
extra effects and edits can change level. Watch the output meter while editing.
No artist presets, ROM samples or song melodies have been copied.
'''
out=folder/'SPACE_INFINITY_Lush_Trance_20.zip'
with zipfile.ZipFile(out,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
 z.writestr('Bank.json',json.dumps(manifest,indent=2)+'\n');z.writestr('README.txt',readme)
 for r in rows:z.write(a.export/r['file'],r['file'])
 z.write(root/'docs/research/LUSH_TRANCE_090_RESEARCH.md','RESEARCH.md')
with zipfile.ZipFile(out) as z:assert z.testzip() is None
sha=hashlib.sha256(out.read_bytes()).hexdigest();out.with_suffix('.zip.sha256').write_text(f'{sha}  {out.name}\n');print(out,sha)
