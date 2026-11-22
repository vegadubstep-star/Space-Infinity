"""Bake a reviewed settings plan, retain stable IDs and known-shipped hashes."""
import argparse,hashlib,json,shutil,struct,zipfile
from pathlib import Path
import xml.etree.ElementTree as ET
p=argparse.ArgumentParser();p.add_argument('work',type=Path);p.add_argument('settings',type=Path);a=p.parse_args();root=Path(__file__).resolve().parents[2];base=root/'native-vst3/Presets'
selection=json.loads((a.work/'Selection.json').read_text());settings=json.loads(a.settings.read_text());keep=set(selection['kept_keys']);assert set(settings)==keep
original=a.work/'Originals';original.mkdir(exist_ok=True)
def snapshot(file,name=None):
    copy=original/(name or file.name)
    if not copy.exists():shutil.copy2(file,copy)
    return copy
factory=base/'TranceFactoryBank.json';doc=json.loads(snapshot(factory).read_text())
for i,r in enumerate(doc['presets'],1001):
    key='F'+str(i)
    if key not in keep:continue
    r['settings'].update(settings[key]);r['tags']=r.get('tags','')+', Curated 300';r['design_hash']=hashlib.sha256(json.dumps({k:r.get(k) for k in ('settings','macros','lfo')},sort_keys=True).encode()).hexdigest()
doc['generation']='curated-300-094';factory.write_text(json.dumps(doc,indent=2)+'\n')
oldcatalog=base/'Installed094.json';catalog=json.loads(snapshot(oldcatalog).read_text());banks={}
for folder,name in [('Heritage200','SPACE_INFINITY_Heritage_200.zip'),('Lush20','SPACE_INFINITY_Lush_Trance_20.zip'),('UserVoices093','SPACE_INFINITY_Orbit_Voices.zip')]:
    file=base/folder/name;source=snapshot(file)
    with zipfile.ZipFile(file) as z:current={r['id']:r for r in json.loads(z.read('Bank.json'))['presets']}
    with zipfile.ZipFile(source) as z:
        m=json.loads(z.read('Bank.json'));payload={n:z.read(n) for n in z.namelist()}
    changed=0
    for r in m['presets']:
        key='Uuser_'+m['id']+'_'+r['id']
        if key not in keep:continue
        b=payload[r['file']];x=ET.fromstring(b[8:8+struct.unpack('<I',b[4:8])[0]])
        for param,value in settings[key].items():
            node=x.find(f"PARAM[@id='{param}']")
            if node is None:node=ET.SubElement(x,'PARAM',id=param)
            node.set('value',str(value))
        x.set('tags',x.get('tags','')+', Curated 300');text=ET.tostring(x,encoding='utf-8');payload[r['file']]=struct.pack('<II',0x21324356,len(text))+text+b'\0'
        old=r['sha256'];cur=current[r['id']];r['previous_sha256s']=list(dict.fromkeys([old,cur['sha256'],r.get('previous_sha256','')]+cur.get('previous_sha256s',[])+r.get('previous_sha256s',[])));r['previous_sha256s']=[h for h in r['previous_sha256s'] if h][:32];r['previous_sha256']=old;r['sha256']=hashlib.sha256(payload[r['file']]).hexdigest();changed+=1
    m['curation_revision']='Curated 300';payload['Bank.json']=(json.dumps(m,indent=2)+'\n').encode()
    temp=file.with_suffix('.tmp')
    with zipfile.ZipFile(temp,'w',zipfile.ZIP_DEFLATED,compresslevel=7) as z:
        for name_,data in payload.items():z.writestr(name_,data)
    temp.replace(file);digest=hashlib.sha256(file.read_bytes()).hexdigest();file.with_suffix('.zip.sha256').write_text(digest+'  '+name+'\n');banks[name]=dict(count=changed,sha256=digest)
    design=base/folder/('DESIGNS.json' if folder=='UserVoices093' else 'Designs.json')
    if design.exists():
        d=json.loads(snapshot(design,folder+'_Designs.json').read_text())
        for r in d['presets']:
            key='Uuser_'+m['id']+'_'+r['id']
            if key in keep:r['settings'].update(settings[key])
        design.write_text(json.dumps(d,indent=2)+'\n')
catalog.update(active_presets=300,revision='Curated 300',factory_presets=sum(k.startswith('F') for k in keep),sampled_presets=sum(k.startswith('Uuser_orbitvoices093_') for k in keep),banks=banks,settings={k:{**catalog['settings'][k],**settings[k]} for k in sorted(keep)},role_counts=selection['quotas'],polished_presets=json.loads((a.work/'FXReview.json').read_text())['changed'])
oldcatalog.write_text(json.dumps(catalog,indent=2)+'\n');print('Baked',len(settings),'curated sounds;',catalog['sampled_presets'],'sampled sounds')
