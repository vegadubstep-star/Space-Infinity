"""Prove every retained patch only changes its reviewed settings and metadata."""
import argparse,hashlib,json,struct,zipfile
from pathlib import Path
import xml.etree.ElementTree as ET
p=argparse.ArgumentParser();p.add_argument('work',type=Path);a=p.parse_args();root=Path(__file__).resolve().parents[2];base=root/'native-vst3/Presets';original=a.work/'Originals';settings=json.loads((a.work/'FinalSettings.json').read_text());changes=json.loads((a.work/'FXPlan.json').read_text());keep=set(settings);rows=[]
old=json.loads((original/'TranceFactoryBank.json').read_text());new=json.loads((base/'TranceFactoryBank.json').read_text())
assert old['init_settings']==new['init_settings'] and len(old['presets'])==len(new['presets'])==160
for i,(x,y) in enumerate(zip(old['presets'],new['presets']),1001):
    key='F'+str(i);wanted=settings.get(key,{})
    assert {k:v for k,v in x.items() if k not in ('settings','tags','design_hash')}=={k:v for k,v in y.items() if k not in ('settings','tags','design_hash')},key
    assert {**x['settings'],**wanted}==y['settings'],key
    if key in keep:rows.append(dict(key=key,audio_unchanged=x['settings']==y['settings'],old_source_sha256='',current_source_sha256='',macro_lfo_and_assets_unchanged=True))
metadata={'tags','presetDisplayName','creator','presetKey'}
def unpack(b):
    x=ET.fromstring(b[8:8+struct.unpack('<I',b[4:8])[0]]);params={n.get('id'):float(n.get('value')) for n in x.findall('PARAM')};attrs={k:v for k,v in x.attrib.items() if k not in metadata};other=[ET.tostring(n) for n in x if n.tag!='PARAM'];return params,attrs,other
for folder,name in [('Heritage200','SPACE_INFINITY_Heritage_200.zip'),('Lush20','SPACE_INFINITY_Lush_Trance_20.zip'),('UserVoices093','SPACE_INFINITY_Orbit_Voices.zip')]:
    with zipfile.ZipFile(original/name) as old,zipfile.ZipFile(base/folder/name) as new:
        manifest=json.loads(new.read('Bank.json'))
        for row in manifest['presets']:
            key='Uuser_'+manifest['id']+'_'+row['id'];before=old.read(row['file']);after=new.read(row['file']);x,xa,xc=unpack(before);y,ya,yc=unpack(after)
            assert xa==ya and xc==yc,(key,'assets or routes changed')
            wanted={**x,**settings.get(key,{})};assert set(wanted)==set(y) and all(abs(wanted[k]-y[k])<1e-6 for k in wanted),(key,'unreviewed parameter changed')
            if key not in keep:assert before==after,(key,'retired recovery bytes changed')
            else:rows.append(dict(key=key,audio_unchanged=x==y,old_source_sha256=hashlib.sha256(before).hexdigest(),current_source_sha256=hashlib.sha256(after).hexdigest(),macro_lfo_and_assets_unchanged=True))
assert len(rows)==300 and {r['key'] for r in rows}==keep
for row in rows:assert row['audio_unchanged']==(row['key'] not in changes),row['key']
report=dict(version='0.9.4',revision='Curated 300',presets_verified=300,edited_audio_states=len(changes),unchanged_audio_states=300-len(changes),retired_extra_bytes_preserved=True,presets=rows,errors=[])
(root/'docs/audit/CURATED_300_SOURCE_PARITY.json').write_text(json.dumps(report,indent=2)+'\n');print('300 source states verified;',report['unchanged_audio_states'],'retain exactly the measured original audio parameters, assets and routes')
