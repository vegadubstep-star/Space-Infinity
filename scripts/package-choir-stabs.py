#!/usr/bin/env python3
"""Validate and package portable owner-choir presets plus rendered one-shots."""
import argparse,io,hashlib,json,struct,wave,zipfile
from pathlib import Path
import xml.etree.ElementTree as ET
p=argparse.ArgumentParser(description=__doc__);p.add_argument('folder',type=Path);p.add_argument('version');p.add_argument('output',type=Path);a=p.parse_args();presets=sorted((a.folder/'Presets').glob('*.sipreset'));clips=sorted((a.folder/'One Shots').glob('*.wav'));assert len(presets)==len(clips)==10
def decode_juce_memory(text):
 length,encoded=text.split('.',1);length=int(length);alphabet='.ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+';output=bytearray();accumulator=bits=0
 for char in encoded:
  accumulator|=alphabet.index(char)<<bits;bits+=6
  if bits>=8:output.append(accumulator&255);accumulator>>=8;bits-=8
 assert len(output)>=length
 return bytes(output[:length])
for file in presets:
 b=file.read_bytes();magic,length=struct.unpack('<II',b[:8]);assert magic==0x21324356;xml=ET.fromstring(b[8:8+length].rstrip(b'\0'));assert xml.tag=='PARAMETERS' and len(decode_juce_memory(xml.attrib['sampleWav']))>1000;assert float(next(c for c in xml if c.attrib.get('id')=='source').attrib['value'])==3
 with wave.open(io.BytesIO(decode_juce_memory(xml.attrib['sampleWav'])),'rb') as embedded:assert embedded.getnframes()>1000 and embedded.getnchannels()==2
for file in clips:
 with wave.open(str(file),'rb') as w:assert w.getnchannels()==2 and w.getframerate()==48000 and w.getnframes()>48000
readme='''SPACE INFINITY — Owner Choir Stabs\n\nTen new .sipreset sounds with their WAV sources embedded, plus ten rendered stereo WAV one-shots.\n\nInstall SPACE INFINITY '''+a.version+''' and use BANK > IMPORT for the presets. You can also drop the WAV one-shots into your DAW sampler. The WAV previews are rendered at MIDI note 60, including each preset's register offset, then peak-normalised for sampler use.\n\nThese use the nine choir recordings supplied by the project owner. No additional pitch LFO, detuned ensemble layers or chorus are applied. Some recordings contain layered tones; single-note hits keep them clearest.\n'''
(a.folder/'READ_ME.txt').write_text(readme);manifest=[dict(file=f.relative_to(a.folder).as_posix(),sha256=hashlib.sha256(f.read_bytes()).hexdigest()) for f in presets+clips];(a.folder/'CONTENTS.json').write_text(json.dumps(manifest,indent=2)+'\n');a.output.parent.mkdir(parents=True,exist_ok=True)
with zipfile.ZipFile(a.output,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 for file in presets+clips+[a.folder/'READ_ME.txt',a.folder/'CONTENTS.json']:z.write(file,'SPACE_INFINITY_'+a.version+'_Choir_Stabs/'+file.relative_to(a.folder).as_posix())
with zipfile.ZipFile(a.output) as z:assert z.testzip() is None
sha=hashlib.sha256(a.output.read_bytes()).hexdigest();a.output.with_name(a.output.name+'.sha256').write_text(sha+'  '+a.output.name+'\n');print('Verified ten embedded-sample presets and ten WAV stabs:',sha)
