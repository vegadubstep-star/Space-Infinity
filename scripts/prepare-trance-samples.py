#!/usr/bin/env python3
"""Fetch pinned CC0 recordings, verify Git blob IDs, and prepare compact stereo zones."""
import concurrent.futures, hashlib, json, math, subprocess, urllib.request, urllib.parse
from pathlib import Path
import numpy as np
from scipy.io import wavfile
from scipy.signal import correlate, find_peaks
ROOT=Path(__file__).resolve().parents[1]
CACHE=ROOT.parent/'trance-research'/'sample-originals'; CACHE.mkdir(parents=True,exist_ok=True)
OUT=ROOT/'native-vst3'/'Samples'/'Trance';OUT.mkdir(parents=True,exist_ok=True)
selection=json.loads((ROOT/'scripts/trance-sample-selection.json').read_text())

def prepare(spec):
    name=f"Factory_{spec['instrument']}_{spec['root']}_{spec['layer']}"
    raw=CACHE/(name+'.wav')
    url='https://raw.githubusercontent.com/'+spec['repo']+'/'+spec['revision']+'/'+urllib.parse.quote(spec['path'],safe='/')
    if not raw.exists():
        with urllib.request.urlopen(url,timeout=45) as response: raw.write_bytes(response.read())
    data=raw.read_bytes(); blob=hashlib.sha1(b'blob '+str(len(data)).encode()+b'\0'+data).hexdigest()
    if blob!=spec['git_blob']: raise ValueError('Upstream content changed: '+name)
    pcm=np.frombuffer(subprocess.check_output(['ffmpeg','-v','error','-i',str(raw),'-f','f32le','-ac','2','-ar','44100','-']),dtype='<f4').reshape(-1,2).copy()
    peak=float(np.max(np.abs(pcm))); envelope=np.max(np.abs(pcm),axis=1)
    audible=np.flatnonzero(envelope>peak*.001);start=max(0,int(audible[0])-88) if len(audible) else 0
    pcm=pcm[start:start+int((4.5 if spec['loop'] else 7.0 if spec['instrument'] in ('Grand','Upright') else 2.0)*44100)]
    # Retain a natural bow/piano attack, remove DC, and fade only the file tail.
    pcm-=np.mean(pcm,axis=0);n=min(len(pcm),4410);pcm[-n:]*=np.linspace(1,0,n)[:,None]
    edge=min(32,len(pcm));pcm[:edge]*=np.linspace(0,1,edge)[:,None]
    # Steinway filenames use scientific octave names (C3 = MIDI 48).
    # VSCO/Yamaha collection names use C3 = MIDI 60. Verified against recorded spectra.
    root=spec['root']-12 if spec['instrument']=='Grand' else spec['root']
    gain=.72/(float(np.max(np.abs(pcm)))+1e-9)
    layer_count=3 if spec['instrument']=='Grand' else 2 if spec['instrument'] in ('Upright','Violins','Celli') else 1
    gain*=([.68,.84,1] if layer_count==3 else [.76,1] if layer_count==2 else [1])[spec['layer']]
    pcm=np.clip(pcm*gain,-.98,.98)
    target=OUT/(name+'.wav');wavfile.write(target,44100,(pcm*32767).astype('<i2'))
    return dict(spec,file=target.relative_to(ROOT).as_posix(),resource=name+'_wav',root=root,layers=layer_count,loop_start=int(.85*44100) if spec['loop'] else 0,frames=len(pcm),source_url=url,source_sha256=hashlib.sha256(data).hexdigest(),output_sha256=hashlib.sha256(target.read_bytes()).hexdigest(),processing='Stereo 44.1kHz PCM16; onset trim, DC removal, short edge fades; 7s piano/4.5s string maximum; velocity-layer gain retained')

with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:
    records=list(pool.map(prepare,selection))
manifest={'license':'CC0-1.0','sources':['https://github.com/sgossner/VCSL/blob/master/LICENSE','https://github.com/sgossner/VSCO-2-CE/blob/master/LICENSE'],'samples':records}
(OUT/'PROVENANCE.json').write_text(json.dumps(manifest,indent=2)+'\n')
groups=['Grand','Upright','Violins','Celli','Spiccato','Pizzicato']
header='#pragma once\n#include <array>\nstruct FactorySampleZone {int instrument;const char* resource;int root,layer,layers,loopStart;};\ninline constexpr std::array<const char*,6> factoryInstrumentNames{{"Studio Grand Multisample","Upright Multisample","Violin Section Multisample","Cello Section Multisample","Spiccato Strings Multisample","Pizzicato Strings Multisample"}};\ninline constexpr std::array<FactorySampleZone,'+str(len(records))+'> factorySampleZones{{\n'
for r in records:header+='    {'+str(groups.index(r['instrument']))+',"'+r['resource']+'",'+','.join(str(r[k]) for k in ['root','layer','layers','loop_start'])+'},\n'
header+='}};\n';(ROOT/'native-vst3/Source/FactorySampleZones.h').write_text(header)
print('Prepared',len(records),'CC0 zones;',round(sum((ROOT/r['file']).stat().st_size for r in records)/1024**2,1),'MiB')
print('Roots:',[(r['instrument'],r['root'],r['layer']) for r in records])
