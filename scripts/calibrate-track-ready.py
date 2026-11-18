"""Level both banks from native BS.1770/true-peak/macros review; preserve authored IDs."""
import argparse,json,math,hashlib
from pathlib import Path
import numpy as np
from track_ready_common import references,reference_peak
p=argparse.ArgumentParser();p.add_argument('review',type=Path);p.add_argument('--target',type=float,default=-24);p.add_argument('--mode',choices=['peak','loudness'],default='peak',help='peak uses maximum measured clean gain; loudness caps gain at the requested LUFS');p.add_argument('--write',action='store_true');p.add_argument('--refine',action='store_true',help='Only correct measured ceiling excesses, unused gain or OPEN differences, with 0.05 dB correction margin');p.add_argument('--output',type=Path,required=True);a=p.parse_args();root=Path(__file__).resolve().parents[1];review=json.loads(a.review.read_text());changes=[];sounds=[];pending=[];extra=references()
for family,file in [('Trance','TranceFactoryBank.json'),('Heritage','Heritage200/Designs.json')]:
 path=root/'native-vst3/Presets'/file;bank=json.loads(path.read_text())
 for row in bank['presets']:
  key=family+'-'+row['id']
  if key not in review:raise ValueError('Unmeasured '+key)
  measured=review[key]
  canonical=dict(row);canonical['_bank']=family
  if family=='Trance':
   canonical['factory_index']=bank['presets'].index(row)+1;canonical['family']='Trance';canonical.setdefault('creator','Orbital Audio');canonical.setdefault('description',canonical.get('play',''));canonical.setdefault('tags',canonical['category'])
  if not measured.get('renderer_sha256') or hashlib.sha256((json.dumps(canonical,sort_keys=True)+measured['renderer_sha256']).encode()).hexdigest()!=measured['design_sha256']:raise ValueError('Stale audio review '+key)
  v=measured['variants'];s=row['settings'];open_change=0
  if 'max_chord' not in v:raise ValueError('Missing macro-extreme review '+key)
  if 'open_single' in v:
   open_change=v['open_single']['lufs']-v['base_single']['lufs'];old_comp=row.get('open_compensation_db',2.16);new_comp=max(-6,min(24,old_comp+open_change));open_change=new_comp-old_comp
   if a.refine and abs(open_change)<.2:new_comp=old_comp;open_change=0
   if a.write and (not a.refine or open_change!=0):
    row['open_compensation_db']=round(new_comp,5)
    for mod in row['macros' if family=='Trance' else 'modulation']:
     if mod['source']==0 and mod['target']=='presetTrim':mod['depth']=-new_comp/48
  raw=max(x['pre_safety_peak']*10**(-open_change/20 if (name.startswith('open') or (name.startswith('max') and 'open_single' in v)) else 0) for name,x in v.items());tp=max(x['true_peak_dbfs']-(open_change if (name.startswith('open') or (name.startswith('max') and 'open_single' in v)) else 0) for name,x in v.items())
  safe_correction=min(20*math.log10(.98/max(raw,1e-10)),-.3-tp)
  reference=extra.get(key);extra_peak=reference_peak(row,reference) if reference else None
  if reference:safe_correction=min(safe_correction,20*math.log10(reference['raw_budget']/max(extra_peak,1e-10)))
  correction=safe_correction if a.mode=='peak' else min(a.target-v['base_single']['lufs'],safe_correction)
  if a.refine:correction=correction-.05 if correction<-.01 or correction>.3 else 0
  before=s.get('presetTrim',0);after=max(-24,min(24,before+correction));new_gain=s['gain']*10**((before+correction-after)/20)
  if new_gain>1.2 or new_gain<.001:raise ValueError('Gain budget exceeded '+key)
  if a.write:s.update(presetTrim=round(after,5),gain=round(new_gain,7));row['review_level_mode']=a.mode;row['review_peak_ceiling_dbfs']=-.3;row['status']='measured_peak_calibrated' if a.mode=='peak' else 'measured_loudness_calibrated'
  if a.write and a.mode=='loudness':row['review_level_target_lufs']=a.target
  elif a.write:row.pop('review_level_target_lufs',None)
  changes.append(dict(key=key,name=row['name'],category=row['category'],base_lufs=v['base_single']['lufs'],predicted_lufs=v['base_single']['lufs']+correction,correction_db=correction,old_trim=before,new_trim=after,open_correction_db=open_change,raw_peak=raw,true_peak_dbfs=tp,extra_reference_peak=extra_peak,extra_reference=reference['condition'] if reference else None,predicted_max_true_peak_dbfs=tp+correction,unused_safe_gain_db=safe_correction-correction,headroom_limited=correction<a.target-v['base_single']['lufs']-.1))
  sounds.append((key,row['name'],row['category'],v['base_single']))
 if a.write:pending.append((path,bank))
pairs=[]
for i,(key,name,category,x) in enumerate(sounds):
 for other,oname,ocategory,y in sounds[:i]:
  sx,sy=np.array(x['spectrum']),np.array(y['spectrum']);ex,ey=np.array(x['envelope']),np.array(y['envelope']);spec=float(sx@sy/(np.linalg.norm(sx)*np.linalg.norm(sy)+1e-20));shape=float(ex@ey/(np.linalg.norm(ex)*np.linalg.norm(ey)+1e-20))
  if spec>.995 and shape>.995:pairs.append(dict(a=key,a_name=name,a_category=category,b=other,b_name=oname,b_category=ocategory,spectral_similarity=spec,envelope_similarity=shape))
result=dict(level_mode=a.mode,target_lufs=a.target if a.mode=='loudness' else None,true_peak_ceiling_dbfs=-.3,raw_peak_budget=.98,calibration_applied=a.write,measurements=changes,similarity_review=sorted(pairs,key=lambda x:-x['spectral_similarity']))
for path,bank in pending:path.write_text(json.dumps(bank,indent=2)+'\n')
a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(result,indent=2)+'\n');print('Measured',len(changes),'voices;',len(pairs),'near-match candidates;',sum(x['headroom_limited'] for x in changes),'headroom-constrained voices')
