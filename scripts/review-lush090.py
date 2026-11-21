"""Render and calibrate only explicitly supplied lead recipes; checkpoint each row."""
import argparse,copy,hashlib,itertools,json,math,os,re,shutil,subprocess
from concurrent.futures import ThreadPoolExecutor,as_completed
from pathlib import Path
import numpy as np
from scipy.io import wavfile
from scipy.signal import resample_poly
p=argparse.ArgumentParser();p.add_argument('design',type=Path);p.add_argument('output',type=Path);p.add_argument('--renderer',type=Path,default=Path('/tmp/RenderLush'));p.add_argument('--verify',action='store_true');p.add_argument('--pluck-open',action='store_true',help='Measure a full OPEN-only phrase for pluck loudness compensation');p.add_argument('--verify-all-corners',action='store_true',help='Re-render every user-macro corner during final verification');a=p.parse_args();root=Path(__file__).resolve().parents[1];rows=json.loads(a.design.read_text())['presets'];a.output.mkdir(parents=True,exist_ok=True);reportpath=a.output/('Verified.json' if a.verify else 'Calibration.json');report=json.loads(reportpath.read_text()) if reportpath.exists() else {};env=dict(os.environ,DISPLAY=':99',XDG_CACHE_HOME='/workspace/work/runtime-cache');sha=hashlib.sha256(a.renderer.read_bytes()).hexdigest()
def analyze(path):
 sr,x=wavfile.read(path);x=x.astype(np.float64)/32768
 run=subprocess.run(['ffmpeg','-nostdin','-hide_banner','-nostats','-i',str(path),'-af','loudnorm=I=-24:TP=-1:LRA=11:print_format=json','-f','null','-'],capture_output=True,text=True,check=True);loud=json.loads(re.findall(r'\{[^{}]+"input_i"[^{}]+\}',run.stderr,re.S)[-1]);mono=x[:sr*3].mean(axis=1);ps=np.abs(np.fft.rfft(mono*np.hanning(len(mono))))**2;hz=np.fft.rfftfreq(len(mono),1/sr);edges=np.geomspace(30,18000,81);spectrum=np.sqrt(np.array([ps[(hz>=lo)&(hz<hi)].sum() for lo,hi in zip(edges[:-1],edges[1:])])/(ps.sum()+1e-20));envelope=np.sqrt(np.mean(x[:sr*3].reshape(96,-1,2)**2,axis=(1,2)));return dict(lufs=float(loud['input_i']),true_peak_dbfs=20*math.log10(max(1e-12,float(np.abs(resample_poly(x,4,1,axis=0)).max()))),spectrum=spectrum.tolist(),envelope=envelope.tolist())
def inspect(row):
 key=row['id'];fingerprint=hashlib.sha256((json.dumps(row,sort_keys=True)+sha).encode()).hexdigest()
 cached=report.get(key,{}) if report.get(key,{}).get('fingerprint')==fingerprint else {}
 open_phrase=a.pluck_open and 'pluck' in row['category'].lower()
 if 'all6_chord' in cached.get('variants',{}) and (not open_phrase or '100_single' in cached.get('variants',{})) and (not a.verify_all_corners or all(''.join(map(str,b))+'_chord' in cached.get('variants',{}) for b in itertools.product([0,1],repeat=3))):return key,cached
 job=a.output/key;job.mkdir(exist_ok=True);variants=dict(cached.get('variants',{}))
 variants_to_check=list(itertools.product([0,1],repeat=3))+['all6']
 if a.verify and not a.verify_all_corners:
  calibration=json.loads((a.output/'Calibration.json').read_text())[key]['variants'];worst=max(calibration,key=lambda k:calibration[k]['pre_safety_peak']).split('_')[0]
  names=list(dict.fromkeys(['000','111','all6',worst]+(['100'] if open_phrase else [])));variants_to_check=[name if name=='all6' else tuple(map(int,name)) for name in names]
 for bits in variants_to_check:
  name=bits if isinstance(bits,str) else ''.join(map(str,bits))
  full=name in ('000','111','all6') or (open_phrase and name=='100')
  if name+'_chord' in variants and (not full or name+'_single' in variants):continue
  r=copy.deepcopy(row);r['settings'].update({f'freeMacro{i}':1 if name=='all6' else bits[i] for i in range(3)})
  if name=='all6':r['settings'].update({f'macro{i}':1 for i in range(3)})
  design=job/'One.json';design.write_text(json.dumps({'presets':[r]}));out=job/name;cmd=[str(a.renderer.resolve()),str(root),str(design.resolve()),str(out.resolve())]+([] if full else ['--verify']);result=subprocess.run(cmd,env=env,capture_output=True,text=True)
  if result.returncode:raise RuntimeError(key+': '+result.stdout+result.stderr)
  measured=json.loads((out/('Measurements.json' if full else 'Measurements-verified.json')).read_text())['presets'][0]
  for role in (['single','chord'] if full else ['chord']):
   values=dict(measured[role]);
   if full:values.update(analyze(out/(key+'-'+role+'.wav')))
   variants[name+'_'+role]=values
  shutil.rmtree(out)
 raw=max(x['pre_safety_peak'] for x in variants.values());tp=max(x.get('true_peak_dbfs',-100) for x in variants.values());correction=min(20*math.log10(.95/max(raw,1e-12)),-.35-tp)
 result=dict(id=key,name=row['name'],fingerprint=fingerprint,renderer_sha256=sha,variants=variants,maximum_raw_peak=raw,maximum_true_peak_dbfs=tp,correction_db=correction,base_lufs=variants['000_single']['lufs'])
 result['headroom_issue']=bool(a.verify and (raw>.951 or tp>-.29))
 shutil.rmtree(job);return key,result
with ThreadPoolExecutor(max_workers=2) as pool:
 for future in as_completed([pool.submit(inspect,row) for row in rows]):
  key,result=future.result();report[key]=result;tmp=reportpath.with_suffix('.tmp');tmp.write_text(json.dumps(report,indent=2)+'\n');tmp.replace(reportpath);print(key,result['name'],'LUFS',result['base_lufs'],'correction',round(result['correction_db'],2),flush=True)
print('COMPLETE',len(rows),flush=True)

if a.verify and any(r.get('headroom_issue') for r in report.values()):
 print('HEADROOM ISSUES:',[k for k,r in report.items() if r.get('headroom_issue')],flush=True);raise SystemExit(1)
