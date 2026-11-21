"""Check exports of the embedded factory slots against the calibrated recipes.

Use RenderBank --factory-only after rebuilding. That mode applies no authoring
settings or routes: these checks therefore detect a forgotten/stale bank embed.
"""
import argparse
from collections import defaultdict
import hashlib
import json
from pathlib import Path
import struct
import xml.etree.ElementTree as ET

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('exports',type=Path)
p.add_argument('recipes',type=Path)
p.add_argument('--output',type=Path,required=True)
a=p.parse_args()
rows=json.loads(a.recipes.read_text())['presets']
exports={r['id']:r for r in json.loads((a.exports/'Exports.json').read_text())['presets']}
assert len(rows)==len(exports)==90
errors=[]

def route_ids(source,slot):
    if source==9:return f'ampFlexTarget_{slot}',f'ampFlexDepth_{slot}'
    prefix='freeMacro' if source<3 else 'lfoFlex' if source<6 else 'performanceFlex'
    index=source if source<3 else source-3 if source<6 else source-6
    return f'{prefix}Target{index}_{slot}',f'{prefix}Depth{index}_{slot}'

for row in rows:
    key=row['id'];path=a.exports/exports[key]['file'];data=path.read_bytes()
    assert struct.unpack('<I',data[:4])[0]==0x21324356
    xml=ET.fromstring(data[8:8+struct.unpack('<I',data[4:8])[0]])
    values={c.attrib['id']:float(c.attrib['value']) for c in xml if 'value' in c.attrib}
    expected=dict(row['settings']);targets={};slots=defaultdict(int)
    mods=list(row['macros'])
    if row.get('lfo'):
        lfo=row['lfo'];expected.update(designerLfo1Enabled=1,designerLfo1Sync=0 if 'hz' in lfo else 1)
        expected['designerLfo1Speed' if 'hz' in lfo else 'motionBars']=lfo.get('hz',lfo.get('bars'))
        mods.append(dict(source=3,target=lfo['target'],depth=lfo['depth']))
    for m in mods:
        source=m['source'];target,depth=route_ids(source,slots[source]);slots[source]+=1
        targets[target]=m['target'];expected[depth]=m['depth']
    actual={k:v for k,v in xml.attrib.items() if k.startswith(('freeMacroTarget','lfoFlexTarget','performanceFlexTarget','ampFlexTarget')) and v not in ('','-1')}
    if actual!=targets:errors.append(key+': embedded modulation routes differ')
    for k,v in expected.items():
        if k not in values or abs(values[k]-v)>max(1e-4,abs(v)*1e-5):errors.append(f'{key}: {k} expected {v}, got {values.get(k)}')
    if xml.attrib.get('presetKey')!='F'+str(1000+row['factory_index']):errors.append(key+': wrong stable factory key')
    for k,v in [('presetDisplayName',row['name']),('creator',row['creator']),('tags',row['tags'])]:
        if xml.attrib.get(k)!=v:errors.append(key+': '+k+' differs')
    for i,label in enumerate(row.get('macro_labels',['TONE','PUNCH','CHARACTER'])):
        if xml.attrib.get('macroLabel'+str(i))!=label:errors.append(key+': macro label differs')

report=dict(presets_verified=len(rows),recipe_sha256=hashlib.sha256(a.recipes.read_bytes().replace(b'\r\n',b'\n')).hexdigest(),
            check='Direct exports of embedded factory slots; parameter values, active modulation targets/depths, stable keys, labels and metadata.',errors=errors)
a.output.write_text(json.dumps(report,indent=2)+'\n')
print('Embedded factory parity:',len(rows),'presets;',len(errors),'errors')
print('\n'.join(errors[:30]))
raise SystemExit(bool(errors))
