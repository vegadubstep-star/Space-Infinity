#!/usr/bin/env python3
"""Calibrate authored bank from native measured output, preserving each design hash."""
import argparse,json,math
from pathlib import Path
root=Path(__file__).resolve().parents[1]
a=argparse.ArgumentParser();a.add_argument('profile',type=Path);opt=a.parse_args()
bank=json.loads((root/'native-vst3/Presets/TranceFactoryBank.json').read_text());measured={r['name']:r for r in json.loads(opt.profile.read_text())};rows=[]
for patch in bank['presets']:
 p=patch['settings'];r=measured[patch['name']];bass='Bass' in patch['category'];pad='Pads' in patch['category'];target=.46 if bass else .40 if pad else .48 if patch['category']=='Strings' else .53
 peak=max(r['raw_peak'],math.atanh(min(.999999,r['peak'])))
 gain=.85 if bass else .75;delta=20*math.log10(target/(max(peak,1e-7)*gain/r['gain']))
 settings={'gain':gain}
 if bass:
  # Oscillator gain is before character/filter saturation; a subsequent native pass verifies it.
  db=max(-12,min(6,p['osc1Gain']+delta));settings.update(osc1Gain=db,osc2Gain=max(-12,min(6,p['osc2Gain']+delta)))
  if p['osc1Gain']+delta>6:settings['gain']=min(1.2,gain*10**((p['osc1Gain']+delta-6)/20))
 else:
  # Full-stage outputs are actual instrument controls; all these patches use delay and reverb.
  total=p.get('fxOutput7',0)+p.get('fxOutput1',0)+delta
  settings['fxOutput7']=max(-12,min(12,total))
  settings['fxOutput1']=max(-12,min(12,total-settings['fxOutput7']))
  if total>24:settings['gain']=min(1.2,gain*10**((total-24)/20))
 rows.append(dict(id=patch['id'],design_hash=patch['design_hash'],settings={k:round(v,6) for k,v in settings.items()},measured_peak=round(r['peak'],6),target_raw_peak=target))
(root/'scripts/trance-bank-levels.json').write_text(json.dumps(rows,indent=2)+'\n');print('Calibrated',len(rows),'patches from native output')
