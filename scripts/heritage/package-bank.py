"""Package the native-exported Heritage collection for one-action bank import."""
import argparse,hashlib,json,zipfile,struct
import xml.etree.ElementTree as ET
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('renders',type=Path);p.add_argument('design',type=Path);p.add_argument('output',type=Path);a=p.parse_args();root=Path(__file__).resolve().parents[2];design=json.loads(a.design.read_text());rows=[];previous=json.loads((root/'scripts/heritage/previous-heritage-hashes.json').read_text())
assert len(design['presets'])==200
for row in design['presets']:
 path=Path('Presets')/row['family']/row['category']/(row['id']+' '+row['name']+'.sipreset');data=(a.renders/path).read_bytes()
 assert len(data)>1000 and struct.unpack('<I',data[:4])[0]==0x21324356
 xml=ET.fromstring(data[8:8+struct.unpack('<I',data[4:8])[0]])
 assert xml.attrib['presetDisplayName']==row['name'] and xml.attrib['creator']==row['creator']
 if row['wavetable']:assert int(xml.attrib['tableFrames'])==17 and len(xml.attrib['tableData'])>10000
 values={x.attrib.get('id'):float(x.attrib['value']) for x in xml if 'value' in x.attrib}
 for key,value in row['settings'].items():assert abs(values[key]-value)<max(1e-4,abs(value)*1e-5),(row['name'],key,value,values[key])
 rows.append(dict(id=row['id'],file=path.as_posix(),name=row['name'],category=row['category'],family=row['family'],sha256=hashlib.sha256(data).hexdigest(),previous_sha256=previous.get(row['id'],'')))
manifest=dict(format='SPACE_INFINITY_BANK_V2',id='heritage200',name='Heritage 200',minimum_version='0.8.0',creator='Orbital Audio',presets=rows)
readme='''HERITAGE 200 — SPACE INFINITY

200 original presets: 80 workstation/M1 influences, 60 JP-8000/8080 influences,
60 Nord Lead 2 influences. These are interpretations, not manufacturer patches
or component-exact emulations. Existing 160 factory presets remain available.

INSTALL: update SPACE INFINITY to 0.8.0 or later. In its preset manager choose
IMPORT, select this ZIP without extracting it, then choose the Heritage 200 tag.
You can also browse M1 Influence, JP Influence, Nord Influence or role tags.
A second ZIP import updates byte-identical older shipped sounds while preserving
your locally edited sounds, and does not duplicate the collection. To perfect a sound, use REPLACE or SAVE AS. Creator/tags are editable.

Basses follow your MIDI; only the Arps/Motion roles generate notes. TONE opens
the filter; CONTOUR changes filter articulation; SPECTRUM moves the custom table
where used. Every recipe retains the synth's analogue/digital switch and filters.

LEVELS: peak-normalized using maximum-velocity native single-note/retrigger and
four-note chord renders, with pluck OPEN endpoints included. Gain uses measured
clean headroom up to a -0.3 dB true-peak ceiling; perceived LUFS is also measured.
Matching peaks does not make all articulations equally loud to the ear.
The synth retains its final safety stage. Changing macros/effects, playing denser
chords or layering instances can change level; watch the meter while editing.

SOURCES: existing documented CC0 multisamples plus newly synthesized original
waveforms. No Korg, Roland or Nord ROM audio was copied. Built-in multisamples
ship inside the plugin; custom wavetables are embedded in each preset.
These references do not imply endorsement by the named manufacturers.

Technical checks cannot substitute for your musical listening test. Individual
auditions and a category catalogue accompany the release.
'''
catalogue=['# Heritage 200 catalogue','', '| ID | Name | Influence | Role |','|---|---|---|---|']
for r in rows:catalogue.append(f"| {r['id']} | {r['name']} | {r['family']} | {r['category']} |")
a.output.parent.mkdir(parents=True,exist_ok=True)
with zipfile.ZipFile(a.output,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
 z.writestr('Bank.json',json.dumps(manifest,indent=2)+'\n');z.writestr('README.txt',readme);z.writestr('CATALOGUE.md','\n'.join(catalogue)+'\n')
 for r in rows:z.write(a.renders/r['file'],r['file'])
 z.write(root/'docs/research/HERITAGE_200_RESEARCH.md','RESEARCH.md')
 z.write(root/'native-vst3/Presets/Heritage200/Wavetables/PROVENANCE.json','WAVETABLE_PROVENANCE.json')
 z.write(root/'docs/research/TRACK_READY_080_RESEARCH.md','TRACK_READY_RESEARCH.md')
 z.write(root/'native-vst3/Samples/Trance/FLAT_CHOIR_PROVENANCE.json','FLAT_CHOIR_PROVENANCE.json')
 z.write(root/'native-vst3/Samples/Trance/PROVENANCE.json','CC0_MULTISAMPLE_PROVENANCE.json')
 z.write(root/'native-vst3/Samples/SAMPLE_PROVENANCE.json','SAMPLE_PROVENANCE.json')
with zipfile.ZipFile(a.output) as z:assert z.testzip() is None
digest=hashlib.sha256(a.output.read_bytes()).hexdigest();a.output.with_suffix('.zip.sha256').write_text(f'{digest}  {a.output.name}\n');a.output.with_name('CATALOGUE.md').write_text('\n'.join(catalogue)+'\n');print(a.output,digest)
