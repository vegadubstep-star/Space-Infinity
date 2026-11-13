#!/usr/bin/env python3
"""Prepare owner-supplied choir one-shots; retain originals and root-note provenance."""
from pathlib import Path
import argparse,json,hashlib,re,shutil,warnings
import numpy as np
from scipy.io import wavfile
from scipy.signal import butter,sosfilt
root=Path(__file__).resolve().parents[1];p=argparse.ArgumentParser(description=__doc__);p.add_argument('manifest',type=Path);a=p.parse_args();uploads=json.loads(a.manifest.read_text());originals=root/'native-vst3/Samples/UserChoirOriginals';originals.mkdir(exist_ok=True);out=root/'native-vst3/Samples/Trance';rows=[]
for i,source in enumerate(uploads):
 name=source['name'];slug=('Ambient_C4' if 'SweepsC4' in name else 'Ambient_C3' if 'SweepsC3' in name else 'Orbital_'+re.search(r'(\d+)\.wav$',name)[1]);original=originals/(slug+'.wav')
 if Path(source['path']).resolve()!=original.resolve():shutil.copyfile(source['path'],original)
 with warnings.catch_warnings():warnings.simplefilter('ignore');sr,pcm=wavfile.read(original)
 x=pcm.astype(np.float64);x=x/(2**(8*pcm.dtype.itemsize-1)) if pcm.dtype.kind=='i' else x
 if x.ndim==1:x=np.repeat(x[:,None],2,axis=1)
 x=sosfilt(butter(2,35,fs=sr,btype='highpass',output='sos'),x,axis=0)
 hop=max(1,int(sr*.01));rms=np.array([np.sqrt(np.mean(x[j:j+hop]**2)) for j in range(0,len(x)-hop,hop)])
 ambient='Ambient' in slug;threshold=(.72 if ambient else .06)*np.max(rms);indices=np.where(rms>=threshold)[0];start=max(0,int(indices[0]*hop)-int(sr*.003));end=min(len(x),start+int(sr*(1.2 if ambient else 1.6)));x=x[start:end]
 peak=np.max(np.abs(x));x*=.72/max(peak,1e-8);fade=min(int(sr*.003),len(x)//4);x[:fade]*=np.linspace(0,1,fade)[:,None];tail=min(int(sr*.04),len(x)//4);x[-tail:]*=np.linspace(1,0,tail)[:,None]
 filename='Factory_User_Choir_'+slug+'.wav';target=out/filename;wavfile.write(target,sr,(np.clip(x,-1,1)*32767).astype('<i2'));note=72 if slug=='Ambient_C4' else 60
 rows.append(dict(instrument=9+i,source_id=30+i,resource=target.stem+'_wav',name=('Ambient Choir C4' if slug=='Ambient_C4' else 'Ambient Choir C3' if slug=='Ambient_C3' else 'Orbital Choir '+slug.split('_')[-1]),root=note,layer=0,layers=1,loop_start=0,file=target.relative_to(root).as_posix(),output_sha256=hashlib.sha256(target.read_bytes()).hexdigest(),original_file=original.relative_to(root).as_posix(),original_name=name,source_sha256=hashlib.sha256(original.read_bytes()).hexdigest(),crop_start=start,crop_end=end,sample_rate=sr,root_basis='Measured C5 fundamental (~524 Hz)' if note==72 else 'Measured C4 fundamental (~262 Hz)' if ambient else 'Embedded WAV smpl unity note (60)',processing='35Hz low cut, onset/body selection, peak trim, 3ms onset and 40ms tail fades. No added detuning, vibrato, layers or chorus.'))
manifest=dict(source='Project-owner uploads supplied for choir-stab creation',license='User supplied; not part of the CC0 sample collection',samples=rows);(out/'USER_CHOIR_PROVENANCE.json').write_text(json.dumps(manifest,indent=2)+'\n')
base=json.loads((out/'PROVENANCE.json').read_text())['samples'];old=json.loads((out/'CHOIR_PROVENANCE.json').read_text())['samples'];groups=['Grand','Upright','Violins','Celli','Spiccato','Pizzicato'];names=['Studio Grand Multisample','Upright Multisample','Violin Section Multisample','Cello Section Multisample','Spiccato Strings Multisample','Pizzicato Strings Multisample','Ensemble Ah Multisample','Ensemble Ooh Multisample','Ensemble Ee Multisample']+[r['name'] for r in rows]
header='#pragma once\n#include <array>\nstruct FactorySampleZone {int instrument;const char* resource;int root,layer,layers,loopStart;};\ninline constexpr std::array<const char*,18> factoryInstrumentNames{{'+','.join('"'+n+'"' for n in names)+'}};\ninline constexpr std::array<FactorySampleZone,60> factorySampleZones{{\n'
for row in base+old+rows:
 instrument=row['instrument'] if isinstance(row['instrument'],int) else groups.index(row['instrument']);header+='    {'+str(instrument)+',"'+row['resource']+'",'+','.join(str(row[k]) for k in ['root','layer','layers','loop_start'])+'},\n'
header+='}};\n';file=root/'native-vst3/Source/FactorySampleZones.h'
if file.read_text()!=header:file.write_text(header)
print('Prepared all nine owner-supplied choirs; 60 zones and 39 source choices.')
