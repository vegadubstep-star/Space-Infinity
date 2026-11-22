"""Install the curated view and a portable recovery bank of removed alternatives."""
import argparse,hashlib,json,zipfile,struct
import xml.etree.ElementTree as ET
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('work',type=Path);a=p.parse_args();root=Path(__file__).resolve().parents[2];base=root/'native-vst3/Presets';selection=json.loads((a.work/'Selection.json').read_text());target=base/'Curation093.json';prior=a.work/'OriginalCuration.json'
if not prior.exists():prior.write_bytes(target.read_bytes())
old=json.loads(prior.read_text());retired=old['retired']+selection['retired'];assert len(retired)==128
archive=base/'SPACE_INFINITY_Curated_300_Alternatives.zip';manifest=dict(format='SPACE_INFINITY_BANK_V2',id='curated300alternatives',name='Curated 300 Alternatives',creator='Orbital Audio',minimum_version='0.9.4',presets=[])
with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED,compresslevel=7) as z:
    for i,r in enumerate(selection['retired'],1):
        data=(a.work/'Recovery'/f"{r['key']}.sipreset").read_bytes();x=ET.fromstring(data[8:8+struct.unpack('<I',data[4:8])[0]]);x.set('tags',x.get('tags','')+', Recovered Alternative');text=ET.tostring(x,encoding='utf-8');data=struct.pack('<II',0x21324356,len(text))+text+b'\0';file=f'Presets/A{i:03}.sipreset';z.writestr(file,data);manifest['presets'].append(dict(id=f'A{i:03}',file=file,name=r['name'],category=r['category'],creator='Orbital Audio',tags='Recovered Alternative',sha256=hashlib.sha256(data).hexdigest()))
    z.writestr('Bank.json',json.dumps(manifest,indent=2)+'\n');z.writestr('README.txt','Optional recovery bank of 121 alternatives removed from the main Curated 300 list. Do not import for the default curated palette. Saved projects and personal replacements remain available.\n')
digest=hashlib.sha256(archive.read_bytes()).hexdigest();archive.with_suffix('.zip.sha256').write_text(digest+'  '+archive.name+'\n')
target.write_text(json.dumps(dict(version='0.9.4',revision='Curated 300',active_presets=300,retired=[{k:r[k] for k in ('key','name','sha256','kept_key','reason')} for r in retired]),indent=2)+'\n')
selection.update(recovery_bank=archive.name,recovery_bank_sha256=digest,previously_archived=7,total_hidden=128)
(root/'docs/audit/CURATED_300_CURATION.json').write_text(json.dumps(selection,indent=2)+'\n')
print('300 active sounds;128 untouched alternatives hidden;121 new alternatives recoverable from',archive.name)
