#!/usr/bin/env python3
"""Rebuild CC0 choir registers from original singers, avoiding extreme single-sample transposition."""
from pathlib import Path
import json,urllib.request,hashlib,re,subprocess,concurrent.futures
import numpy as np
from scipy.io import wavfile
from scipy.signal import butter,sosfilt,periodogram
root=Path(__file__).resolve().parents[1];cache=Path('/workspace/work/choir-rebuild');cache.mkdir(parents=True,exist_ok=True)
revision='08eba76eadfdc16ded3209f65d3890bf1a2b8c0c'
with urllib.request.urlopen('https://api.github.com/repos/swesterfeld/spectmorph-instruments/git/trees/'+revision+'?recursive=1') as response:tree=json.load(response)['tree']
with urllib.request.urlopen(f'https://raw.githubusercontent.com/swesterfeld/spectmorph-instruments/{revision}/README.md') as response:license_text=response.read().decode()
assert 'CC0' in license_text
files=[r for r in tree if r['path'].endswith('.flac')]
def choose(prefix,note):
 rows=[r for r in files if r['path'].startswith(prefix+'/')];return min(rows,key=lambda r:abs(int(re.search(r'-(\d+)\.flac$',r['path'])[1])-note))
def raw(record):
 target=cache/Path(record['path']).name;url=f'https://raw.githubusercontent.com/swesterfeld/spectmorph-instruments/{revision}/'+record['path']
 if not target.exists():
  with urllib.request.urlopen(url,timeout=40) as response:target.write_bytes(response.read())
 b=target.read_bytes();assert hashlib.sha1(b'blob '+str(len(b)).encode()+b'\0'+b).hexdigest()==record['sha'];pcm=np.frombuffer(subprocess.check_output(['ffmpeg','-v','error','-i',str(target),'-f','f32le','-ac','1','-ar','44100','-']),dtype='<f4').copy();peak=np.max(np.abs(pcm));positions=np.flatnonzero(np.abs(pcm)>peak*.05);begin=int(positions[0])+int(.10*44100);pcm=pcm[begin:];return pcm,dict(path=record['path'],url=url,git_blob=record['sha'],source_sha256=hashlib.sha256(b).hexdigest())
def measured_pitch(pcm,played):
 expected=440*2**((played-69)/12);segment=pcm[11025:min(len(pcm),88200)];f,power=periodogram(segment,44100,nfft=524288);ids=np.where((f>expected*2**(-1.5/12))&(f<expected*2**(1.5/12)))[0];i=ids[np.argmax(power[ids])];logs=np.log(np.maximum(power[i-1:i+2],1e-30));delta=.5*(logs[0]-logs[2])/(logs[0]-2*logs[1]+logs[2]);return float((i+np.clip(delta,-.5,.5))*44100/524288)
out=root/'native-vst3/Samples/Trance';records=[]
for family,(vowel,male) in enumerate([('ah','mirko-ah'),('oh','mirko-oh'),('ih','sven-ih')]):
 for note in [48,60,72]:
  sources=[choose('claudia-'+vowel,note),choose(male,note) if note<65 else choose('claudia-'+vowel,note-3)];audio=np.zeros((int(3.6*44100),2));origins=[]
  for singer,source in enumerate(sources):
   pcm,origin=raw(source);origins.append(origin);played=int(re.search(r'-(\d+)\.flac$',source['path'])[1]);pcm=sosfilt(butter(2,70,fs=44100,btype='highpass',output='sos'),pcm);pcm/=max(.001,float(np.max(np.abs(pcm))));measured=measured_pitch(pcm,played);origin['measured_fundamental_hz']=round(measured,6);origin['tuning_correction_cents']=round(1200*np.log2((440*2**((played-69)/12))/measured),4)
   for copy,cents in enumerate([0,-2.5 if singer==0 else 3.5]):
    positions=np.arange(len(audio))*(440*2**((note-69)/12)/measured)*2**(cents/1200)+((singer*2+copy)*44100/(440*2**((note-69)/12)));end=len(pcm)-4410;begin=min(22050,end//3);fade=1102;positions=np.where(positions>=end,begin+fade+np.mod(positions-end,max(1,end-begin-fade)),positions);layer=np.interp(positions,np.arange(len(pcm)),pcm);blend=np.clip((positions-end+fade)/fade,0,1);head=np.interp(begin+positions-end+fade,np.arange(len(pcm)),pcm);layer=layer*(1-blend)+head*blend;pan=.2 if (singer+copy)%2==0 else .8;weight=([.25,.75] if note<55 else [.55,.45] if note<66 else [.8,.2])[singer]*(1 if copy==0 else .2);audio[:,0]+=layer*np.sqrt(1-pan)*weight;audio[:,1]+=layer*np.sqrt(pan)*weight
  audio-=np.mean(audio,axis=0);audio*=.78/max(.001,np.max(np.abs(audio)));audio[:132]*=np.linspace(0,1,132)[:,None];audio[-4410:]*=np.linspace(1,0,4410)[:,None];name=f'Factory_Choir_{vowel}_{note}';file=out/(name+'.wav');wavfile.write(file,44100,(np.clip(audio,-1,1)*32767).astype('<i2'));records.append(dict(instrument=6+family,resource=name+'_wav',root=note,layer=0,layers=1,loop_start=22050,frames=len(audio),file=file.relative_to(root).as_posix(),output_sha256=hashlib.sha256(file.read_bytes()).hexdigest(),origins=origins,revision=revision,processing='Original CC0 singers; measured fundamental tuning before layering; register-matched clear-core stereo ensemble with quiet -2.5/+3.5-cent support, 70Hz low cut, steady-vowel onset, edge fades; no octave-wide single-singer playback'))
(out/'CHOIR_PROVENANCE.json').write_text(json.dumps(dict(license='CC0-1.0',license_url='https://github.com/swesterfeld/spectmorph-instruments/blob/'+revision+'/README.md',samples=records),indent=2)+'\n')
# Keep original piano/string provenance and append the separately credited choir registers.
base=json.loads((out/'PROVENANCE.json').read_text())['samples'];names=['Studio Grand Multisample','Upright Multisample','Violin Section Multisample','Cello Section Multisample','Spiccato Strings Multisample','Pizzicato Strings Multisample','Ensemble Ah Multisample','Ensemble Ooh Multisample','Ensemble Ee Multisample'];groups=['Grand','Upright','Violins','Celli','Spiccato','Pizzicato']
header='#pragma once\n#include <array>\nstruct FactorySampleZone {int instrument;const char* resource;int root,layer,layers,loopStart;};\ninline constexpr std::array<const char*,9> factoryInstrumentNames{{'+','.join('"'+n+'"' for n in names)+'}};\ninline constexpr std::array<FactorySampleZone,51> factorySampleZones{{\n'
for row in base+records:
 instrument=row['instrument'] if isinstance(row['instrument'],int) else groups.index(row['instrument']);header+='    {'+str(instrument)+',"'+row['resource']+'",'+','.join(str(row[k]) for k in ['root','layer','layers','loop_start'])+'},\n'
header+='}};\n';header_file=root/'native-vst3/Source/FactorySampleZones.h'
if not header_file.exists() or header_file.read_text()!=header:header_file.write_text(header)
print('Prepared 9 CC0 choir zones across three vowels/registers; total factory zones51')
