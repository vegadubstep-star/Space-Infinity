"""Measure and encode actual final native renders; never normalize previews."""
import argparse,json,re,subprocess,zipfile,hashlib
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import numpy as np
from scipy.io import wavfile
from scipy.signal import resample_poly
p=argparse.ArgumentParser();p.add_argument('renders',type=Path);p.add_argument('recipes',type=Path);p.add_argument('output',type=Path);a=p.parse_args();bank=json.loads(a.recipes.read_text());folder=a.output.parent/'HeritagePreviews';folder.mkdir(parents=True,exist_ok=True)
def encode(row):
 source=a.renders/(row['id']+'-single.wav');rate,pcm=wavfile.read(source);data=pcm.astype(np.float64)/32768;peak=float(np.max(np.abs(resample_poly(data,4,1,axis=0))));db=20*np.log10(max(peak,1e-12))
 if db>-.9:raise ValueError(f"Final native true peak exceeds headroom target: {row['name']} {db}")
 dest=folder/row['family']/row['category']/(row['id']+' '+row['name']+'.mp3');dest.parent.mkdir(parents=True,exist_ok=True)
 subprocess.run(['ffmpeg','-nostdin','-hide_banner','-loglevel','error','-y','-i',str(source),'-c:a','libmp3lame','-q:a','3',str(dest)],check=True)
 run=subprocess.run(['ffmpeg','-nostdin','-hide_banner','-i',str(source),'-af','loudnorm=I=-20:TP=-1:LRA=11:print_format=json','-f','null','-'],capture_output=True,text=True,check=True);m=json.loads(re.findall(r'\{[^{}]+"input_i"[^{}]+\}',run.stderr,re.S)[-1])
 return dict(id=row['id'],name=row['name'],family=row['family'],category=row['category'],sample_rate=rate,true_peak_dbfs=float(db),integrated_lufs=float(m['input_i']),preview=dest.relative_to(folder).as_posix(),native_wav_sha256=hashlib.sha256(source.read_bytes()).hexdigest())
with ThreadPoolExecutor(max_workers=2) as pool:report=list(pool.map(encode,bank['presets']))
assert len(report)==200
audit=dict(method='Actual calibrated native 48 kHz renders; 4x polyphase true-peak measurement; BS.1770 loudness via ffmpeg. MP3 previews encoded directly with no additional normalization.',presets=report)
a.output.with_suffix('.levels.json').write_text(json.dumps(audit,indent=2)+'\n')
with zipfile.ZipFile(a.output,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 z.writestr('README.txt','200 actual native preset auditions. Hold/chord/retrigger phrases use the calibrated preset level. These are audio previews, not preset files. The preset bank ZIP is imported separately. No preview normalization or external effects were added.\n')
 for r in report:z.write(folder/r['preview'],r['preview'])
 z.writestr('LEVELS.json',json.dumps(audit,indent=2)+'\n')
digest=hashlib.sha256(a.output.read_bytes()).hexdigest();a.output.with_suffix('.zip.sha256').write_text(f'{digest}  {a.output.name}\n')
print('Encoded 200 native previews; maximum true peak',max(r['true_peak_dbfs'] for r in report),'dBFS;',digest)
