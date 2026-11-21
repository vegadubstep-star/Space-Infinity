"""Plan/bake static BS.1770 preset matching, preserving macro trim and local edits."""
import argparse,hashlib,json,math,shutil,struct,zipfile
from pathlib import Path
import xml.etree.ElementTree as ET
p=argparse.ArgumentParser(description=__doc__);p.add_argument('measurements',type=Path);p.add_argument('work',type=Path);p.add_argument('--target',type=float,default=-22.5);p.add_argument('--write',action='store_true');p.add_argument('--settings',type=Path,help='Use a previously refined settings plan');a=p.parse_args()
root=Path(__file__).resolve().parents[2];a.work.mkdir(parents=True,exist_ok=True);rows=json.loads(a.measurements.read_text())['presets'];assert len(rows)==421 and len({r['key'] for r in rows})==421
changes={};limited=[]
for r in rows:
 m=r['phrase'];assert math.isfinite(m['lufs']) and m['rms']>1e-6
 safe=20*math.log10(min(.95/m['pre_safety_peak'],.93/m['true_peak_4x']))
 correction=min(a.target-m['lufs'],safe)
 changes[r['key']]={**r['polish'],'presetReferenceGain':round(max(-48,min(24,correction)),6)}
 if m['lufs']+correction<a.target-1:limited.append(dict(key=r['key'],name=r['name'],lufs=m['lufs'],safe_lufs=m['lufs']+safe,settings=changes[r['key']]))
if a.settings:changes=json.loads(a.settings.read_text())
(a.work/'Settings.json').write_text(json.dumps(changes,indent=2)+'\n');(a.work/'Outliers.json').write_text(json.dumps(limited,indent=2)+'\n')
print('421 settings planned; outliers beyond 1 LU:',len(limited));print(json.dumps(limited,indent=2))
if not a.write:raise SystemExit(0)
assert not limited or a.settings, 'Refine unresolved crest-factor outliers before baking'
originals=a.work/'Originals';originals.mkdir(exist_ok=True)
factory=root/'native-vst3/Presets/TranceFactoryBank.json';copy=originals/'TranceFactoryBank.json'
if not copy.exists():shutil.copy2(factory,copy)
doc=json.loads(copy.read_text());tag='Level Matched 094'
for i,r in enumerate(doc['presets'],1):
 settings=changes['F'+str(1000+i)];r['settings'].update(settings);r['tags']=r.get('tags','')+', '+tag;r['loudness_revision']='installed-094'
 r['design_hash']=hashlib.sha256(json.dumps({k:r.get(k) for k in ('settings','macros','lfo')},sort_keys=True).encode()).hexdigest()
doc['generation']='installed-loudness-094';factory.write_text(json.dumps(doc,indent=2)+'\n')
updated={};source_hashes={}
for folder,name in [('Heritage200','SPACE_INFINITY_Heritage_200.zip'),('Lush20','SPACE_INFINITY_Lush_Trance_20.zip'),('UserVoices093','SPACE_INFINITY_Orbit_Voices.zip')]:
 target=root/'native-vst3/Presets'/folder/name;copy=originals/name
 if not copy.exists():shutil.copy2(target,copy)
 with zipfile.ZipFile(target) as current_zip:current_rows={r['id']:r for r in json.loads(current_zip.read('Bank.json'))['presets']}
 source_hashes[name]=hashlib.sha256(copy.read_bytes()).hexdigest()
 with zipfile.ZipFile(copy) as z:
  manifest=json.loads(z.read('Bank.json'));payload={n:z.read(n) for n in z.namelist()};changed=0
  for r in manifest['presets']:
   key='Uuser_'+manifest['id']+'_'+r['id']
   if key not in changes:continue # Seven archived variants retain exact original hashes.
   data=payload[r['file']];x=ET.fromstring(data[8:8+struct.unpack('<I',data[4:8])[0]])
   for param,value in changes[key].items():
    node=x.find(f"PARAM[@id='{param}']")
    if node is None:node=ET.SubElement(x,'PARAM',id=param)
    node.set('value',str(value))
   x.set('tags',x.get('tags','')+', '+tag);text=ET.tostring(x,encoding='utf-8');revised=struct.pack('<II',0x21324356,len(text))+text+b'\0';payload[r['file']]=revised
   old=r['sha256'];current=current_rows.get(r['id'],{});r['previous_sha256s']=[h for h in dict.fromkeys([old,current.get('sha256',''),r.get('previous_sha256','')]+current.get('previous_sha256s',[])+r.get('previous_sha256s',[])) if h][:32];r['previous_sha256']=old;r['sha256']=hashlib.sha256(revised).hexdigest();changed+=1
  manifest['minimum_version']='0.9.4';manifest['level_revision']='installed-094';payload['Bank.json']=(json.dumps(manifest,indent=2)+'\n').encode()
  if 'README.txt' in payload:payload['README.txt']=b'Automatically included in SPACE INFINITY 0.9.4. No manual import is needed.\n'+payload['README.txt']
  tmp=target.with_suffix('.tmp')
  with zipfile.ZipFile(tmp,'w',zipfile.ZIP_DEFLATED,compresslevel=7) as out:
   for n,data in payload.items():out.writestr(n,data)
  tmp.replace(target)
 target.with_suffix('.zip.sha256').write_text(hashlib.sha256(target.read_bytes()).hexdigest()+'  '+name+'\n');updated[name]=dict(count=changed,sha256=hashlib.sha256(target.read_bytes()).hexdigest())
 # Keep source designs aligned with their actual native parameters.
 designs=root/'native-vst3/Presets'/folder/('DESIGNS.json' if folder=='UserVoices093' else 'Designs.json')
 if designs.exists():
  copy=originals/(folder+'_Designs.json')
  if not copy.exists():shutil.copy2(designs,copy)
  design=json.loads(copy.read_text())
  for r in design['presets']:
   key='Uuser_'+manifest['id']+'_'+r['id']
   if key in changes:r['settings'].update(changes[key])
  designs.write_text(json.dumps(design,indent=2)+'\n')
report=dict(version='0.9.4',target_lufs=a.target,active_presets=421,init=1,factory_presets=160,banks=updated,original_bank_hashes=source_hashes,polished_presets=sum(bool(r['polish']) for r in rows),settings=changes)
(root/'native-vst3/Presets/Installed094.json').write_text(json.dumps(report,indent=2)+'\n');print('Baked all421 sounds:',updated)
