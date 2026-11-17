"""Balance the rendered bank, respecting pre-safety and chord headroom."""
import argparse,json,math,re,subprocess
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import numpy as np
from scipy.io import wavfile
from scipy.signal import resample_poly
p=argparse.ArgumentParser();p.add_argument('design',type=Path);p.add_argument('renders',type=Path);p.add_argument('output',type=Path);p.add_argument('--audit-only',action='store_true');a=p.parse_args()
bank=json.loads(a.design.read_text());measure=json.loads((a.renders/'Measurements.json').read_text())['presets'];byid={r['id']:r for r in measure}
def inspect(row):
 path=a.renders/(row['id']+'-single.wav');run=subprocess.run(['ffmpeg','-nostdin','-hide_banner','-i',str(path),'-af','loudnorm=I=-20:TP=-1:LRA=11:print_format=json','-f','null','-'],capture_output=True,text=True,check=True)
 obj=json.loads(re.findall(r'\{[^{}]+"input_i"[^{}]+\}',run.stderr,re.S)[-1]);rate,data=wavfile.read(path);data=data.astype(np.float64)/32768
 truepeak=float(np.max(np.abs(resample_poly(data,4,1,axis=0))));mono=data.mean(axis=1);n=min(len(mono),rate*3);spectrum=np.abs(np.fft.rfft(mono[:n]*np.hanning(n)))**2;f=np.fft.rfftfreq(n,1/rate);edges=np.geomspace(35,16000,49);bands=np.array([spectrum[(f>=lo)&(f<hi)].sum() for lo,hi in zip(edges[:-1],edges[1:])]);bands=np.sqrt(bands/(bands.sum()+1e-20));env=np.array([np.sqrt(np.mean(chunk**2)) for chunk in np.array_split(mono[:n],24)]);env/=np.linalg.norm(env)+1e-15
 return row['id'],dict(integrated_lufs=float(obj['input_i']),true_peak_dbfs=20*math.log10(max(1e-10,truepeak)),spectrum=bands.tolist(),envelope=env.tolist())
with ThreadPoolExecutor(max_workers=2) as pool:analysis=dict(pool.map(inspect,bank['presets']))
audit=[]
for row in bank['presets']:
 m=byid[row['id']];raw=max(m[k]['pre_safety_peak'] for k in ('single','chord'));info=analysis[row['id']]
 if not math.isfinite(info['integrated_lufs']) or raw<1e-5:raise ValueError('Silent/nonfinite '+row['name'])
 # Aim for -20 LUFS on the fixed maximum-velocity phrase, but never trade
 # chord peaks for that target. No limiter/drive is added to inflate loudness.
 correction=min(-20-info['integrated_lufs'],20*math.log10(.94/raw));old=row['settings']['presetTrim'];trim=max(-24,min(24,old+correction))
 gain=row['settings']['gain'];remaining=old+correction-trim;new_gain=min(1.2,max(.001,gain*10**(remaining/20)))
 if not a.audit_only:row['settings'].update(presetTrim=round(trim,4),gain=round(new_gain,6))
 audit.append(dict(id=row['id'],name=row['name'],measured_lufs=info['integrated_lufs'],true_peak_dbfs=info['true_peak_dbfs'],pre_safety_peak=raw,previous_trim=old,proposed_trim=round(trim,4),previous_gain=gain,proposed_gain=round(new_gain,6),limited_by_chord_headroom=correction<(-20-info['integrated_lufs'])-.01))
pairs=[]
rows=bank['presets']
for i,left in enumerate(rows):
 for right in rows[:i]:
  if left['category']!=right['category']:continue
  l,r=analysis[left['id']],analysis[right['id']];s=float(np.dot(l['spectrum'],r['spectrum']));e=float(np.dot(l['envelope'],r['envelope']))
  if s>.995 and e>.995:pairs.append(dict(a=left['name'],b=right['name'],spectral_similarity=s,envelope_similarity=e))
report=dict(method='48 kHz native render, full-velocity single/repeated notes and four-note chords; BS.1770 via ffmpeg loudnorm; four-times oversampled single-note peak',target_lufs=-20,pre_safety_peak_budget=.94,measurements=audit,similarity_review=sorted(pairs,key=lambda x:-x['spectral_similarity']))
a.output.parent.mkdir(parents=True,exist_ok=True)
if not a.audit_only:a.output.write_text(json.dumps(bank,indent=2)+'\n')
a.output.with_suffix('.audit.json').write_text(json.dumps(report,indent=2)+'\n')
print('Checked',len(audit),'voices;',len(pairs),'similarity review pairs; pre-safety peak range',min(x['pre_safety_peak'] for x in audit),max(x['pre_safety_peak'] for x in audit))
