"""Verify measured final bank headroom, OPEN transitions and maximum clean gain."""
import argparse,hashlib,json,math
from pathlib import Path
from track_ready_common import references,reference_peak
p=argparse.ArgumentParser();p.add_argument('review',type=Path);p.add_argument('output',type=Path);a=p.parse_args();root=Path(__file__).resolve().parents[1];review=json.loads(a.review.read_text());rows=[];errors=[];bank_hashes={};extra=references()
for family,file in [('Trance','TranceFactoryBank.json'),('Heritage','Heritage200/Designs.json')]:
 path=root/'native-vst3/Presets'/file;bank_hashes[family]=hashlib.sha256(path.read_bytes()).hexdigest();bank=json.loads(path.read_text())
 if len(bank['presets'])!=(160 if family=='Trance' else 200):errors.append(family+': incorrect preset count')
 for index,row in enumerate(bank['presets'],1):
  key=family+'-'+row['id'];r=review[key];canonical=dict(row);canonical['_bank']=family
  if family=='Trance':canonical['factory_index']=index;canonical['family']='Trance';canonical.setdefault('creator','Orbital Audio');canonical.setdefault('description',canonical.get('play',''));canonical.setdefault('tags',canonical['category'])
  if hashlib.sha256((json.dumps(canonical,sort_keys=True)+r['renderer_sha256']).encode()).hexdigest()!=r['design_sha256']:errors.append(key+': stale measurements')
  v=r['variants'];required=['base_single','base_chord','max_single','max_chord']
  if 'pluck' in row['category'].lower():required+=['open_single','open_chord']
  if any(name not in v for name in required):errors.append(key+': incomplete variants');continue
  tp=max(x['true_peak_dbfs'] for x in v.values());raw=max(x['pre_safety_peak'] for x in v.values());peak=max(x['peak'] for x in v.values());available=min(-.3-tp,20*math.log10(.98/max(raw,1e-10)));opened=v['open_single']['lufs']-v['base_single']['lufs'] if 'open_single' in v else None
  reference=extra.get(key);extra_peak=reference_peak(row,reference) if reference else None
  if reference:
   available=min(available,20*math.log10(reference['raw_budget']/max(extra_peak,1e-10)))
   if extra_peak>reference['raw_budget']*1.005:errors.append(key+': register/cutoff/startup headroom exceeded')
  if row['settings'].get('convolverEnabled') and 'max_ir2_chord' not in v:errors.append(key+': missing repeated convolver measurement')
  if not all(math.isfinite(x['lufs']) and math.isfinite(x['pre_safety_peak']) for x in v.values()):errors.append(key+': invalid audio')
  if tp>-.29 or raw>.987 or peak>.99001:errors.append(key+': headroom ceiling exceeded')
  if available>.3:errors.append(key+': more than 0.3 dB unused measured clean gain')
  if opened is not None and abs(opened)>1:errors.append(key+': OPEN loudness differs by more than 1 LU')
  rows.append(dict(key=key,name=row['name'],category=row['category'],lufs=v['base_single']['lufs'],open_difference_lu=opened,max_true_peak_dbfs=tp,max_pre_safety_peak=raw,max_sample_peak=peak,extra_reference=reference,extra_reference_peak=extra_peak,remaining_clean_gain_db=available))
result=dict(presets=len(rows),ceiling_dbTP=-.3,measurement='48 kHz; full velocity; single/retrigger, four-note chords, OPEN and all three performance macros raised; true peak via 4x resampling',bank_sha256=bank_hashes,review_sha256=hashlib.sha256(a.review.read_bytes()).hexdigest(),errors=errors,presets_measured=rows)
a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(result,indent=2)+'\n');print('Verified',len(rows),'presets;',len(errors),'issues');print('\n'.join(errors[:30]));raise SystemExit(bool(errors))
