"""Validate final native measurements and integrate only the verified 90 rows."""
import argparse
import copy
from collections import Counter
import hashlib
import json
import math
from pathlib import Path
import numpy as np

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('design',type=Path)
p.add_argument('review',type=Path)
p.add_argument('stress',type=Path)
p.add_argument('baseline',type=Path)
p.add_argument('--write',action='store_true')
p.add_argument('--digital',type=Path,help='Additional Digital-mode register/sweep measurements')
p.add_argument('--bass-registers',type=Path,help='Bound four additional bass-register probes to their input recipes')
a=p.parse_args()
root=Path(__file__).resolve().parents[1]
rows=json.loads(a.design.read_text())['presets']
review=json.loads(a.review.read_text())
stress=json.loads(a.stress.read_text())
digital=json.loads(a.digital.read_text()) if a.digital else {}
bass_registers=json.loads(a.bass_registers.read_text()) if a.bass_registers else {}
bass_refs={r['id']:r for r in bass_registers.get('presets',[])}
baseline=json.loads(a.baseline.read_text())
errors=[];measurements=[];pairs=[]

def signature(row):
    return hashlib.sha256(json.dumps({k:row.get(k) for k in
        ('settings','macros','lfo','wavetable')},sort_keys=True).encode()).hexdigest()

def text_hash(path):
    return hashlib.sha256(path.read_bytes().replace(b'\r\n',b'\n')).hexdigest()

for row in rows:
    key=row['id'];r=review[key];v=r['variants'];s=stress[key]
    expected=hashlib.sha256((json.dumps(row,sort_keys=True)+r['renderer_sha256']).encode()).hexdigest()
    if expected!=r['fingerprint']:errors.append(key+': stale phrase measurements')
    if s['recipe_sha256']!=hashlib.sha256(json.dumps(row,sort_keys=True).encode()).hexdigest():errors.append(key+': stale sweep/register measurements')
    required={f'{i:03b}_chord' for i in range(8)}|{'000_single','111_single','all6_single','all6_chord'}
    if not required<=set(v):errors.append(key+': incomplete controller coverage')
    if not math.isfinite(v['000_single']['lufs']) or v['000_single']['rms']<=1e-5:
        errors.append(key+': inaudible or invalid starting sound')
    raw=max([s['stress']['pre_safety_peak']]+[m['pre_safety_peak'] for m in v.values()])
    tp=max([s['true_peak_dbfs']]+[m['true_peak_dbfs'] for m in v.values() if 'true_peak_dbfs' in m])
    if digital:
        d=digital[key];rendered=copy.deepcopy(row);rendered['settings']['engineMode']=1
        if d.get('engine_mode')!=1 or d['recipe_sha256']!=s['recipe_sha256'] or d['rendered_recipe_sha256']!=hashlib.sha256(json.dumps(rendered,sort_keys=True).encode()).hexdigest():
            errors.append(key+': stale Digital-mode measurements')
        raw=max(raw,d['stress']['pre_safety_peak']);tp=max(tp,d['true_peak_dbfs'])
    if key in bass_refs:
        ref=bass_refs[key];before=copy.deepcopy(row);before['settings']['gain']=ref['source_gain']
        if ref['source_recipe_sha256']!=hashlib.sha256(json.dumps(before,sort_keys=True).encode()).hexdigest():
            errors.append(key+': stale four-register bass probe')
        scaled=max(ref['analogue_peak'],ref['digital_peak'])*row['settings']['gain']/ref['source_gain']
        raw=max(raw,scaled)
    if not math.isfinite(raw) or raw>.951 or not math.isfinite(tp) or tp>-.29:errors.append(key+': insufficient headroom')
    opened=None
    if 'pluck' in row['category'].lower():
        opened=v['100_single']['lufs']-v['000_single']['lufs']
        if abs(opened)>1:errors.append(key+': OPEN differs by more than 1 LU')
    available=min(20*math.log10(.95/max(1e-12,raw)),-.35-tp)
    positive=sum(max(0,m['depth']*48) for m in row['macros'] if m['target']=='presetTrim' and m['source']<3)
    limited=row['settings']['gain']>=1.19999 and row['settings']['presetTrim']>=24-positive-.0001
    if available>.4 and not limited:errors.append(key+': more than 0.4 dB unused clean gain')
    measurements.append(dict(id=key,name=row['name'],category=row['category'],
        source_signature=signature(row),base_lufs=v['000_single']['lufs'],
        open_difference_lu=opened,max_pre_safety_peak=raw,
        max_true_peak_dbfs=tp,remaining_clean_gain_db=available,
        gain_control_limited=limited,phrase_fingerprint=r['fingerprint'],
        stress_recipe_sha256=s['recipe_sha256']))

for i,row in enumerate(rows):
    x=review[row['id']]['variants']['000_single']
    for other in rows[:i]:
        y=review[other['id']]['variants']['000_single']
        sx,sy=np.array(x['spectrum']),np.array(y['spectrum'])
        ex,ey=np.array(x['envelope']),np.array(y['envelope'])
        spec=float(sx@sy/(np.linalg.norm(sx)*np.linalg.norm(sy)+1e-20))
        env=float(ex@ey/(np.linalg.norm(ex)*np.linalg.norm(ey)+1e-20))
        if spec>.995 and env>.995:pairs.append(dict(a=row['id'],b=other['id'],a_name=row['name'],b_name=other['name'],spectral_similarity=spec,envelope_similarity=env))

assert len(rows)==90 and len({r['id'] for r in rows})==90
assert len(baseline['presets'])==160
original={r['id']:r for r in baseline['presets']}
if bass_refs:
    wanted={r['id'] for r in baseline['presets'] if 'Bass' in r['category']}
    if set(bass_refs)!=wanted:errors.append('Incomplete four-register bass collection')
    revised_ids={r['id'] for r in rows}
    for i,r in enumerate(baseline['presets'],1):
        if r['id'] not in bass_refs or r['id'] in revised_ids:continue
        ref=bass_refs[r['id']];before=copy.deepcopy(r);before['factory_index']=i
        if ref['source_recipe_sha256']!=hashlib.sha256(json.dumps(before,sort_keys=True).encode()).hexdigest():errors.append(r['id']+': stale retained bass probe')
        if max(ref['analogue_peak'],ref['digital_peak'])>=.95:errors.append(r['id']+': retained bass exceeds four-register headroom')
assert all(r['id'] in original and r['name']==original[r['id']]['name'] and r['category']==original[r['id']]['category'] for r in rows)
report=dict(version='0.9.1',presets_measured=len(rows),factory_presets=160,
    factory_presets_retained_unchanged=70,roles=dict(Counter(r['category'] for r in rows)),
    maximum_true_peak_dbfs=max(r['max_true_peak_dbfs'] for r in measurements),
    maximum_pre_safety_peak=max(r['max_pre_safety_peak'] for r in measurements),
    maximum_open_difference_lu=max(abs(r['open_difference_lu']) for r in measurements if r['open_difference_lu'] is not None),
    measurement='48 kHz; articulated singles, four-note chords, all eight free-macro corners and all six macros raised. Three-register sweeps, accented/legato acid cutoff sweeps, pluck OPEN ramps and overlapping pad chords. 4x true peak; pre-safety peak constrained to 0.95. No claim of subjective listening approval.',
    phrase_renderer_sha256=next(iter(review.values()))['renderer_sha256'],
    stress_renderer_sha256=next(iter(stress.values()))['renderer_sha256'],
    design_sha256=text_hash(a.design),
    text_sha256_line_endings='LF',
    phrase_review_sha256=text_hash(a.review),
    stress_review_sha256=text_hash(a.stress),
    digital_sweep_review_sha256=text_hash(a.digital) if a.digital else None,
    sweep_engine_modes_checked=[0,1] if digital else [0],
    additional_bass_registers=bass_registers or None,
    baseline_sha256=text_hash(a.baseline),
    errors=errors,similarity_candidates=sorted(pairs,key=lambda r:-r['spectral_similarity']),measurements=measurements)
output=root/'docs/audit/STUDIO_RESEARCH_091_AUDIO_LEVELS.json'
output.write_text(json.dumps(report,indent=2)+'\n')
print('Verified',len(rows),'presets;',len(errors),'errors;',len(pairs),'similarity candidates')
print('\n'.join(errors[:30]))
if errors:raise SystemExit(1)
if a.write:
    revised={r['id']:r for r in rows}
    for i,old in enumerate(baseline['presets']):
        if old['id'] not in revised:continue
        row=dict(revised[old['id']])
        for k in ('factory_index','family','description'):row.pop(k,None)
        row.update(status='measured_peak_calibrated',review_level_mode='peak',
                   design_hash=signature(row)[:16])
        baseline['presets'][i]=row
    baseline['generation']='studio-research-091'
    path=root/'native-vst3/Presets/TranceFactoryBank.json'
    path.write_text(json.dumps(baseline,indent=2)+'\n')
    (root/'docs/audit/STUDIO_RESEARCH_091_RECIPES.json').write_bytes(a.design.read_bytes())
    report['factory_bank_sha256']=text_hash(path)
    output.write_text(json.dumps(report,indent=2)+'\n')
    print('Integrated ninety revisions; seventy factory recipes unchanged.')
