"""Stable choir loops and original synth/choir composites; no added LFO/vibrato."""
from pathlib import Path
import hashlib,json
import numpy as np
from scipy.io import wavfile
from scipy.signal import butter,sosfiltfilt
root=Path(__file__).resolve().parents[1];folder=root/'native-vst3/Samples/Trance';sr=44100;n=sr*3;t=np.arange(n)/sr;f0=261.625565
sources=['Factory_Choir_ah_60.wav','Factory_Choir_oh_60.wav','Factory_Choir_ih_60.wav'];names=['Flat_Ah','Flat_Ooh','Flat_Ee','Hybrid_Rave','Hybrid_Dark','Hybrid_Glass'];records=[]
for i,name in enumerate(names):
 source=folder/sources[i%3];rate,data=wavfile.read(source);scale=float(np.iinfo(data.dtype).max)+1 if np.issubdtype(data.dtype,np.integer) else 1;data=data.astype(np.float64)/scale
 if rate!=sr:raise ValueError('Expected 44.1 kHz prepared choir input')
 if data.ndim==1:data=np.column_stack([data,data])
 # Find the steadiest half-second in the sustained body, excluding onset/tail.
 length=int(rate*.65);best=None
 for start in range(int(rate*.25),max(int(rate*.25)+1,len(data)-length-int(rate*.15)),int(rate*.04)):
  segment=data[start:start+length,:2];e=np.array([np.sqrt(np.mean(x*x)+1e-12) for x in np.array_split(segment,12)]);score=float(np.std(e)/(np.mean(e)+1e-9))
  if best is None or score<best[0]:best=(score,start,segment)
 segment=best[2];cross=min(int(rate*.04),len(segment)//8);body=segment.copy();ramp=np.linspace(0,1,cross)[:,None];body[:cross]=body[:cross]*ramp+body[-cross:]*(1-ramp);body=body[:-cross]
 x=np.tile(body,(int(np.ceil(n/len(body)))+1,1))[:n]
 # Slow envelope levelling removes body swell while leaving fine vocal texture.
 power=np.mean(x*x,axis=1);smooth=np.convolve(power,np.ones(2205)/2205,mode='same');gain=np.sqrt(np.median(smooth)/(smooth+1e-8));x*=np.clip(gain,.7,1.4)[:,None]
 if i>=3:
  synth=np.zeros((n,2));ratios=[-.0009,0,.0009]
  for channel in range(2):
   for voice,detune in enumerate(ratios):
    phase=2*np.pi*f0*(1+detune*(1 if channel==0 else -.8))*t+voice*.71
    for h in range(1,41):
     a=(1/h if i==3 else (1/h if h%2 else .12/h) if i==4 else np.exp(-((h-5)/3)**2)/h+.3/h)*np.exp(-h*.035)
     synth[:,channel]+=a*np.sin(phase*h)
  synth/=max(1e-9,np.max(np.abs(synth)));x=x/(np.max(np.abs(x))+1e-9)*.60+synth*.40
 x=sosfiltfilt(butter(2,75,fs=sr,btype='highpass',output='sos'),x,axis=0);x=sosfiltfilt(butter(6,5500,fs=sr,output='sos'),x,axis=0);x/=max(1e-9,float(np.max(np.abs(x))));x*=.72
 fade=int(sr*.004);x[:fade]*=np.linspace(0,1,fade)[:,None];x[-int(sr*.1):]*=np.linspace(1,0,int(sr*.1))[:,None]
 output=folder/('Factory_'+name+'.wav');wavfile.write(output,sr,np.asarray(x*32767,dtype=np.int16));records.append(dict(name=name,file=output.name,source=source.name,source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),sha256=hashlib.sha256(output.read_bytes()).hexdigest(),source_id=39+i,root=60,loop_start=22050,processing='Steady body selection, crossfaded loop, restrained slow envelope levelling, 75 Hz low-cut and 5.5 kHz antialias roll-off for upper-register playback; original additive synth layer for hybrid variants. No LFO or added vibrato.',license='Derived from the documented CC0 factory ensemble recordings; original additive synth programming.'))
(folder/'FLAT_CHOIR_PROVENANCE.json').write_text(json.dumps(records,indent=2)+'\n');print('Prepared',len(records),'flat choir/hybrid sources')
