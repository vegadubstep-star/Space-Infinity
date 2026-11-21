"""Apply measured macro compensation, then maximum safe post-FX gain.

All tone/FX changes happen before this script. Compensation only modulates
presetTrim (a post-FX gain), so prior measured corner peaks can be transformed
without pretending to remeasure nonlinear processing. Final native renders
remain mandatory.
"""
import argparse
import hashlib
import json
import math
from pathlib import Path

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('stage',choices=['compensate','levels','refine'])
p.add_argument('design',type=Path)
p.add_argument('review',type=Path)
p.add_argument('output',type=Path)
p.add_argument('--stress',type=Path)
p.add_argument('--digital',type=Path)
a=p.parse_args()
bank=json.loads(a.design.read_text())
review=json.loads(a.review.read_text())
audit=[]

def gain(db):return 10**(db/20)
def compensation(row,name):
    bits=[1]*3 if name=='all6' else list(map(int,name))
    return sum(m['depth']*48*bits[m['source']] for m in row['macros']
               if m['target']=='presetTrim' and m['source']<3)

stress=json.loads(a.stress.read_text()) if a.stress else {}
digital=json.loads(a.digital.read_text()) if a.digital else {}
for row in bank['presets']:
    key=row['id']
    r=review[key]
    if a.stage=='refine':
        expected=hashlib.sha256((json.dumps(row,sort_keys=True)+r['renderer_sha256']).encode()).hexdigest()
        fingerprint=hashlib.sha256(json.dumps(row,sort_keys=True).encode()).hexdigest()
        if r['fingerprint']!=expected or stress[key]['recipe_sha256']!=fingerprint or digital[key]['recipe_sha256']!=fingerprint:
            raise ValueError('Stale final measurements '+key)
        raw=max([m['pre_safety_peak'] for m in r['variants'].values()]+[stress[key]['stress']['pre_safety_peak'],digital[key]['stress']['pre_safety_peak']])
        tp=max([m['true_peak_dbfs'] for m in r['variants'].values() if 'true_peak_dbfs' in m]+[stress[key]['true_peak_dbfs'],digital[key]['true_peak_dbfs']])
        correction=min(20*math.log10(.95/max(1e-12,raw)),-.35-tp)
        if correction<-.02:
            correction-=.02
            old=row['settings']['gain'];row['settings']['gain']=round(old*gain(correction),8)
            audit.append(dict(id=key,correction_db=correction,old_gain=old,new_gain=row['settings']['gain']))
        continue
    if a.stage=='compensate':
        expected=hashlib.sha256((json.dumps(row,sort_keys=True)+r['renderer_sha256']).encode()).hexdigest()
        if expected!=r['fingerprint']:raise ValueError('Stale tone measurements '+key)
        v=r['variants']
        is_pluck='pluck' in row['category'].lower()
        if is_pluck and '100_single' not in v:raise ValueError('Missing OPEN phrase '+key)
        base=v['000_chord']['pre_safety_peak']
        offsets=[]
        for source,name in enumerate(['100','010','001']):
            if source==0 and is_pluck:
                # Integrated loudness of the same articulated phrase at both
                # endpoints. No arbitrary fixed offset from another preset.
                db=max(-6,min(24,v['100_single']['lufs']-v['000_single']['lufs']))
            else:
                db=max(0,min(12,20*math.log10(max(1e-12,v[name+'_chord']['pre_safety_peak'])/max(1e-12,base))-1))
            if abs(db)>.02:row['macros'].append(dict(source=source,target='presetTrim',depth=-db/48))
            offsets.append(round(db,5))
        if is_pluck:row['open_compensation_db']=offsets[0]
        audit.append(dict(id=key,macro_compensation_db=offsets,source_fingerprint=r['fingerprint']))
    else:
        s=row['settings']
        # All controller-corner audio was measured before compensation; trim
        # is after the FX and is therefore a linear amplitude transformation.
        raw=max(v['pre_safety_peak']*gain(compensation(row,name.split('_')[0]))
                for name,v in r['variants'].items())
        tp=max(v['true_peak_dbfs']+compensation(row,name.split('_')[0])
               for name,v in r['variants'].items() if 'true_peak_dbfs' in v)
        entry=stress[key]
        expected=hashlib.sha256(json.dumps(row,sort_keys=True).encode()).hexdigest()
        if entry['recipe_sha256']!=expected:raise ValueError('Stale register/sweep probe '+key)
        raw=max(raw,entry['stress']['pre_safety_peak'])
        tp=max(tp,entry['true_peak_dbfs'])
        if digital:
            d=digital[key]
            if d['recipe_sha256']!=expected or d.get('engine_mode')!=1:raise ValueError('Stale Digital sweep '+key)
            raw=max(raw,d['stress']['pre_safety_peak']);tp=max(tp,d['true_peak_dbfs'])
        correction=min(20*math.log10(.95/max(raw,1e-12)),-.35-tp)
        old=s['presetTrim'];old_gain=s['gain'];wanted=old+correction
        # Leave room for the macro offsets inside presetTrim's -24..+24 dB
        # range. Shift static gain into Output where needed, so OPEN's measured
        # compensation does not disappear against the trim parameter's stops.
        offsets=[m['depth']*48 for m in row['macros']
                 if m['target']=='presetTrim' and m['source']<3]
        minimum_trim=-24-sum(min(0,x) for x in offsets)
        maximum_trim=24-sum(max(0,x) for x in offsets)
        if minimum_trim>maximum_trim+1e-5:raise ValueError('Macro trim range exceeded '+key)
        trim=max(minimum_trim,min(maximum_trim,wanted))
        wanted_gain=s['gain']*gain(wanted-trim)
        if wanted_gain>1.2:
            trim=min(maximum_trim,wanted+20*math.log10(old_gain/1.2))
            wanted_gain=old_gain*gain(wanted-trim)
        applied_gain=max(.001,min(1.2,wanted_gain))
        # Store actual float values used by the engine, with ample precision.
        s.update(presetTrim=round(trim,6),gain=round(applied_gain,8))
        actual=trim-old+20*math.log10(applied_gain/old_gain)
        row.update(review_revision='studio-research-091',status='awaiting_final_native_verification',review_peak_ceiling_dbfs=-.3)
        row['calibration_gain_limited']=wanted_gain>1.2
        audit.append(dict(id=key,correction_db=correction,
            predicted_pre_safety_peak=raw*gain(actual),
            predicted_true_peak_dbfs=tp+actual,
            gain_control_limited=wanted_gain>1.2))

a.output.parent.mkdir(parents=True,exist_ok=True)
a.output.write_text(json.dumps(bank,indent=2)+'\n')
a.output.with_suffix('.audit.json').write_text(json.dumps(dict(stage=a.stage,presets=audit),indent=2)+'\n')
print(a.stage,len(audit),'presets ->',a.output)
