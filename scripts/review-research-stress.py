"""Checkpoint native register/cutoff/OPEN sweeps and 4x true peaks per recipe."""
import argparse
import copy
from concurrent.futures import ThreadPoolExecutor,as_completed
import hashlib
import json
import math
import os
from pathlib import Path
import shutil
import subprocess

import numpy as np
from scipy.io import wavfile
from scipy.signal import resample_poly

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('design',type=Path)
p.add_argument('output',type=Path)
p.add_argument('--renderer',type=Path,default=Path('/tmp/RenderResearch'))
p.add_argument('--verify',action='store_true')
p.add_argument('--engine-mode',type=int,choices=[0,1],help='Exercise a retained Analogue/Digital mode explicitly')
a=p.parse_args()
root=Path(__file__).resolve().parents[1]
rows=json.loads(a.design.read_text())['presets']
a.output.parent.mkdir(parents=True,exist_ok=True)
report=json.loads(a.output.read_text()) if a.output.exists() else {}
renderer_sha=hashlib.sha256(a.renderer.read_bytes()).hexdigest()
env=dict(os.environ,DISPLAY=':99',XDG_CACHE_HOME='/workspace/work/runtime-cache')

def inspect(row):
    key=row['id'];fingerprint=hashlib.sha256(json.dumps(row,sort_keys=True).encode()).hexdigest()
    cached=report.get(key,{})
    if cached.get('recipe_sha256')==fingerprint and cached.get('renderer_sha256')==renderer_sha and cached.get('engine_mode')==a.engine_mode:
        return key,cached
    suffix='' if a.engine_mode is None else '-mode'+str(a.engine_mode)
    job=a.output.parent/('stress-'+key+suffix);job.mkdir(exist_ok=True)
    rendered=copy.deepcopy(row)
    if a.engine_mode is not None:rendered['settings']['engineMode']=a.engine_mode
    design=job/'One.json';design.write_text(json.dumps(dict(presets=[rendered])))
    result=subprocess.run([str(a.renderer.resolve()),str(root),str(design),str(job),'--research-stress'],env=env,capture_output=True,text=True)
    if result.returncode:raise RuntimeError(key+': '+result.stdout+result.stderr)
    measured=json.loads((job/'Measurements-stress.json').read_text())['presets'][0]['stress']
    sr,x=wavfile.read(job/'Stress.wav')
    assert sr==48000 and x.dtype==np.int32
    x=x.astype(np.float64)/2147483648
    # Each segment is a separate prepare/play experiment. Exclude the artificial
    # joins from interpolation; never treat two unrelated registers as one
    # continuous signal. Raw peaks still cover every rendered sample.
    assert len(x)==6*563*256
    interpolated=max(float(np.abs(resample_poly(part,4,1,axis=0)[128:-128]).max())
                     for part in x.reshape(6,563*256,2))
    true_peak=20*math.log10(max(1e-12,interpolated))
    result=dict(id=key,recipe_sha256=fingerprint,renderer_sha256=renderer_sha,
                stress=measured,true_peak_dbfs=true_peak,probe_join_exclusion_samples=32,
                engine_mode=a.engine_mode,
                rendered_recipe_sha256=hashlib.sha256(json.dumps(rendered,sort_keys=True).encode()).hexdigest(),
                headroom_issue=bool(a.verify and (measured['pre_safety_peak']>.951 or true_peak>-.29)))
    shutil.rmtree(job)
    return key,result

with ThreadPoolExecutor(max_workers=2) as pool:
    for future in as_completed([pool.submit(inspect,row) for row in rows]):
        key,result=future.result();report[key]=result
        tmp=a.output.with_suffix('.tmp');tmp.write_text(json.dumps(report,indent=2)+'\n');tmp.replace(a.output)
        print(key,'raw',round(result['stress']['pre_safety_peak'],6),'dBTP',round(result['true_peak_dbfs'],3),flush=True)
print('COMPLETE',len(rows),flush=True)

if a.verify and any(r.get('headroom_issue') for r in report.values()):
    print('HEADROOM ISSUES:',[k for k,r in report.items() if r.get('headroom_issue')],flush=True)
    raise SystemExit(1)
