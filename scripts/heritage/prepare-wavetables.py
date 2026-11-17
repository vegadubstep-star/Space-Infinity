"""Original harmonic, sync-cycle and ring-product tables; no manufacturer data."""
import json,hashlib
from pathlib import Path
import numpy as np
from scipy.io import wavfile
ROOT=Path(__file__).resolve().parents[2];out=ROOT/'native-vst3/Presets/Heritage200/Wavetables';out.mkdir(parents=True,exist_ok=True)
n=4096;t=np.arange(n)/n;tables={}
for name in ['NorthernSync','NorthernRing','NorthernVowel','NorthernFM','NorthernHollow','EnsembleFeedback','EnsembleTriangle','DigitalDrawbar','DigitalGlass','DigitalWire','DigitalVoice','DigitalAir']:
 frames=[]
 for frame in range(17):
  x=frame/16;theta=2*np.pi*t
  if name=='NorthernSync':y=2*((t*(1+7*x))%1)-1
  elif name=='NorthernRing':y=np.sin(theta)*np.sin(theta*(2+6*x))
  elif name in ('NorthernFM','NorthernHollow'):y=np.sin(theta+(0.1+3*x)*np.sin(theta*(2 if name=='NorthernFM' else 3)))
  elif name=='EnsembleFeedback':
   y=np.sin(theta)
   for _ in range(5):y=np.sin(theta+(.15+1.7*x)*y)
  elif name=='EnsembleTriangle':y=(2/np.pi)*np.arcsin(np.sin(theta+(.1+1.2*x)*np.sin(theta*2)))
  else:
   y=np.zeros(n)
   for h in range(1,33):
    if name=='DigitalDrawbar':a={1:1,2:.25+.4*x,3:.6*x,4:.25,6:.2*x,8:.12*x}.get(h,0)
    elif name in ('DigitalVoice','NorthernVowel'):a=(np.exp(-((h-(3+5*x))/1.5)**2)+.5*np.exp(-((h-(10-3*x))/2)**2))/h+(1 if h==1 else 0)*.35
    elif name=='DigitalGlass':a=(.7 if h in (1,3,7,11) else .08)/h*(1+x*h/12)
    elif name=='DigitalWire':a=np.exp(-h*(.025+.10*x))/np.sqrt(h)
    else:a=np.exp(-h*(.1+.12*x))/h
    y+=a*np.sin(theta*h+(h%3)*x*.5)
  # A bounded32-harmonic representation. Frame bank is authored, not ROM data.
  f=np.fft.rfft(y);f[33:]=0;y=np.fft.irfft(f,n);y=y[::8];y-=y.mean();y/=(max(abs(y))+.000001);frames.append(y*.94)
 pcm=(np.concatenate(frames)*32767).astype('<i2');path=out/(name+'.wav');wavfile.write(path,44100,pcm);tables[name]=dict(file=path.relative_to(ROOT).as_posix(),frames=17,frame_size=512,sha256=hashlib.sha256(path.read_bytes()).hexdigest(),source='Original mathematical synthesis;32-harmonic limit')
(out/'PROVENANCE.json').write_text(json.dumps(tables,indent=2)+'\n');print('Prepared',len(tables),'original evolving tables')
