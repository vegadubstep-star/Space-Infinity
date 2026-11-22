"""Check the final palette across category boundaries after the reviewed edits."""
import argparse,hashlib,importlib.util,json,zipfile
from pathlib import Path
import numpy as np
p=argparse.ArgumentParser();p.add_argument('work',type=Path);a=p.parse_args();root=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('palette',Path(__file__).with_name('select-palette.py'));m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
selection=json.loads((a.work/'Selection.json').read_text());keep=set(selection['kept_keys']);changes=json.loads((a.work/'FXPlan.json').read_text());rows={}
for name in ('profiles','profileA','profileB'):
    for r in json.loads((a.work/name/'Profiles.json').read_text())['presets']:
        if r['key'] in keep:rows[r['key']]=r
fresh=[]
for name in ('afterProfileA','afterProfileB'):
    for r in json.loads((a.work/name/'Profiles.json').read_text())['presets']:rows[r['key']]=r;fresh.append(r['key'])
assert set(rows)==keep and len(fresh)==len(changes) and set(fresh)==set(changes)
parity=json.loads((root/'docs/audit/CURATED_300_SOURCE_PARITY.json').read_text());proof={r['key']:r for r in parity['presets']}
for k,r in rows.items():
    assert len(r['registers'])==3 and all(x['rms']>1e-6 for x in r['registers'])
    assert (k in changes) or proof[k]['audio_unchanged']
    if k in changes:assert r['source_sha256']==proof[k]['current_source_sha256'],(k,'fresh profile source differs')
keys=sorted(rows);vectors=[]
for k in keys:
    vector=[]
    for r in rows[k]['registers']:
        for field,weight in [('spectrum',.45),('attack_spectrum',.2),('envelope',.35)]:vector.extend(m.unit(r[field])*np.sqrt(weight/3))
    vectors.append(vector)
scores=np.asarray(vectors)@np.asarray(vectors).T;nearest=[];candidates=[]
for i,k in enumerate(keys):
    bass=m.category(rows[k]['category']) in ('Bass','Acid');choices=[j for j,x in enumerate(keys) if i!=j and (m.category(rows[x]['category']) in ('Bass','Acid'))==bass];j=max(choices,key=lambda j:scores[i,j]);other=keys[j];score=float(scores[i,j]);nearest.append(dict(key=k,name=rows[k]['name'],category=rows[k]['category'],nearest_key=other,nearest_name=rows[other]['name'],nearest_category=rows[other]['category'],score=score))
    for j in choices:
        if j<i and scores[i,j]>=.99:candidates.append(dict(key_a=k,key_b=keys[j],name_a=rows[k]['name'],name_b=rows[keys[j]]['name'],**m.comparison(rows[k],rows[keys[j]])))
doc=dict(version='0.9.4',revision='Curated 300',presets_checked=300,registers=[-12,0,12],fresh_final_profiles=len(fresh),unchanged_audio_profiles_reused=300-len(fresh),maximum_same_pitch_group_similarity=max(r['score'] for r in nearest),review_threshold=.99,close_candidates=candidates,nearest=nearest,profile_sha256s={name:hashlib.sha256((a.work/name/'Profiles.json').read_bytes()).hexdigest() for name in ('profiles','profileA','profileB','afterProfileA','afterProfileB')},limitations='Three-register default-note feature screening covers attack, sustained spectrum and amplitude contour, including pairs across category labels. It cannot prove perceptual uniqueness for every melody, macro setting or listener.')
(root/'docs/audit/CURATED_300_SIMILARITY.json').write_text(json.dumps(doc,indent=2)+'\n');print(json.dumps({k:doc[k] for k in ('presets_checked','fresh_final_profiles','maximum_same_pitch_group_similarity','close_candidates')},indent=2));raise SystemExit(bool(candidates))
