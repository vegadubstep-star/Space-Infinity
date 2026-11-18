"""Checkpointed native audio review; keeps only one preset's WAVs on disk."""
import argparse,json,math,os,re,subprocess,hashlib,concurrent.futures,shutil
from pathlib import Path
import numpy as np
from scipy.io import wavfile
from scipy.signal import resample_poly
p=argparse.ArgumentParser();p.add_argument('--renderer',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--workers',type=int,default=2);p.add_argument('--limit',type=int);p.add_argument('--reset',action='store_true');a=p.parse_args();root=Path(__file__).resolve().parents[1];a.output.mkdir(parents=True,exist_ok=True)
paths=[root/'native-vst3/Presets/TranceFactoryBank.json',root/'native-vst3/Presets/Heritage200/Designs.json'];rows=[]
for family,path in zip(['Trance','Heritage'],paths):
 for i,row in enumerate(json.loads(path.read_text())['presets']):
  row=dict(row);row['_bank']=family
  if family=='Trance':row['factory_index']=i+1;row['family']='Trance';row.setdefault('creator','Orbital Audio');row.setdefault('description',row.get('play',''));row.setdefault('tags',row['category'])
  rows.append(row)
renderer_sha=hashlib.sha256(a.renderer.read_bytes()).hexdigest()
report_path=a.output/'Review.json';report={} if a.reset or not report_path.exists() else json.loads(report_path.read_text())
def analyze(path):
 run=subprocess.run(['ffmpeg','-nostdin','-hide_banner','-nostats','-i',str(path),'-af','loudnorm=I=-24:TP=-1:LRA=11:print_format=json','-f','null','-'],capture_output=True,text=True,check=True)
 loud=json.loads(re.findall(r'\{[^{}]+"input_i"[^{}]+\}',run.stderr,re.S)[-1]);sr,x=wavfile.read(path);x=x.astype(np.float64)/32768;mono=x.mean(axis=1);n=min(len(mono),sr*4);ps=np.abs(np.fft.rfft(mono[:n]*np.hanning(n)))**2;f=np.fft.rfftfreq(n,1/sr);edges=np.geomspace(25,18000,81);bins=np.sqrt(np.array([ps[(f>=lo)&(f<hi)].sum() for lo,hi in zip(edges[:-1],edges[1:])])/(ps.sum()+1e-20));env=np.array([np.sqrt(np.mean(c*c)) for c in np.array_split(mono[:n],32)]);env/=np.linalg.norm(env)+1e-20
 return dict(lufs=float(loud['input_i']),true_peak_dbfs=20*math.log10(max(1e-12,float(np.abs(resample_poly(x,4,1,axis=0)).max()))),spectrum=bins.tolist(),envelope=env.tolist())
env=dict(os.environ,DISPLAY=':99',XDG_CACHE_HOME='/workspace/work/runtime-cache')
def inspect(index,row):
 key=row['_bank']+'-'+row['id'];fingerprint=hashlib.sha256((json.dumps(row,sort_keys=True)+renderer_sha).encode()).hexdigest()
 cached=report.get(key,{}) if report.get(key,{}).get('design_sha256')==fingerprint else {}
 ir=bool(row['settings'].get('convolverEnabled',0))
 if ('max_ir2_chord' if ir else 'max_chord') in cached.get('variants',{}):return None
 job=a.output/key;job.mkdir(exist_ok=True);design=job/'One.json';design.write_text(json.dumps({'presets':[row]}));variants=dict(cached.get('variants',{}))
 for variant in ['base']+(['open'] if 'pluck' in row['category'].lower() else [])+['max']+(['max_ir1','max_ir2'] if ir else []):
  if variant+'_chord' in variants:continue
  voice=dict(row);voice['settings']=dict(row['settings'])
  if variant.startswith('max'):voice['settings'].update(freeMacro0=1,freeMacro1=1,freeMacro2=1)
  design.write_text(json.dumps({'presets':[voice]}))
  folder=job/variant
  cmd=[str(a.renderer.resolve()),str(root),str(design.resolve()),str(folder.resolve())]+(['--open'] if variant=='open' else [])
  result=subprocess.run(cmd,env=env,capture_output=True,text=True)
  if result.returncode:raise RuntimeError(key+': '+result.stderr+result.stdout)
  measured=json.loads((folder/'Measurements.json').read_text())['presets'][0]
  for mode in ['single','chord']:
   wav=folder/(row['id']+'-'+mode+'.wav');info=analyze(wav);info.update(measured[mode]);variants[variant+'_'+mode]=info;wav.unlink()
 result=dict(id=row['id'],name=row['name'],bank=row['_bank'],category=row['category'],design_sha256=fingerprint,renderer_sha256=renderer_sha,variants=variants)
 if not math.isfinite(variants['base_single']['lufs']):raise RuntimeError('Silent preset '+key)
 raw=max(v['pre_safety_peak'] for v in variants.values());correction=min(-24-variants['base_single']['lufs'],20*math.log10(.98/max(raw,1e-10)),-.3-max(v['true_peak_dbfs'] for v in variants.values()))
 result['level_correction_db']=correction;result['headroom_limited']=correction<(-24-variants['base_single']['lufs'])-.05
 shutil.rmtree(job)
 return index,key,row['name'],result
failures=[]
with concurrent.futures.ThreadPoolExecutor(max_workers=max(1,min(3,a.workers))) as pool:
 futures=[pool.submit(inspect,index,row) for index,row in enumerate(rows[:a.limit] if a.limit else rows)]
 for future in concurrent.futures.as_completed(futures):
  try:value=future.result()
  except Exception as error:failures.append(str(error));print('FAILED',str(error),flush=True);continue
  if value is None:continue
  index,key,name,result=value;report[key]=result;temporary=report_path.with_suffix('.tmp');temporary.write_text(json.dumps(report,indent=2)+'\n');temporary.replace(report_path)
  complete=sum('max_chord' in r.get('variants',{}) and r.get('renderer_sha256')==renderer_sha for r in report.values());v=result['variants'];print(complete,len(rows),key,name,'LUFS',round(v['base_single']['lufs'],2),'trim',round(result['level_correction_db'],2),flush=True)
print('COMPLETE',len(report),'failures',len(failures),flush=True)
if failures:raise SystemExit(1)
