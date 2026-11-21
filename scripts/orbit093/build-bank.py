"""Build the owner-uploaded INFINITY sample bank, retaining source provenance."""
from pathlib import Path
import hashlib,json,struct,zipfile,xml.etree.ElementTree as E,shutil
import numpy as np
def juce64(data):
 # MemoryBlock uses length-prefixed little-endian six-bit groups, not RFC4648.
 raw=np.frombuffer(data,dtype=np.uint8); padded=np.pad(raw,(0,(-len(raw))%3)).astype(np.uint32).reshape(-1,3)
 bits=padded[:,0]|(padded[:,1]<<8)|(padded[:,2]<<16); codes=np.empty((len(bits),4),dtype=np.uint8)
 table=np.frombuffer(b'.ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+',dtype=np.uint8)
 for n in range(4):codes[:,n]=table[(bits>>(6*n))&63]
 return str(len(data))+'.'+codes.tobytes()[:(len(data)*8+5)//6].decode()
root=Path(__file__).resolve().parents[2]; assets=root/'native-vst3/Samples/User093';assets.mkdir(exist_ok=True);out=root/'native-vst3/Presets/UserVoices093';out.mkdir(exist_ok=True)
rows=[
('ORBITAL CHOIR 11.wav','Astral Chorus',60,'Choir'),('ORBITAL CHOIR 2.wav','Stellar Sanctum',72,'Choir'),('ORBITAL CHOIR 5.wav','Celestial Oath',48,'Choir'),('ORBITAL CHOIR 9.wav','Nebula Voices',60,'Choir'),
('OTHTS1_Lead_09.wav','Ion Lance',48,'Lead'),('OTHTS1_Lead_22.wav','Photon Crest',60,'Lead'),('OTHTS1_Lead_26.wav','Reactor Edge',48,'Lead'),('OTHTS1_Lead_27.wav','Quantum Blade',48,'Lead'),('OTHTS1_Lead_15.wav','Solar Needle',72,'Lead'),
('JP8000 LEAD 2 C4.wav','Zenith Current',60,'Lead'),('JP8000 LEAD 10 C2.wav','Prism Engine',60,'Lead'),('JP8000 LEAD 13 C4.wav','Velocity Arc',48,'Lead'),('JP8000 LEAD 23 C3.wav','Gravity Pulse',36,'Lead'),('JP8000 LEAD 36 C2.wav','Event Horizon',48,'Lead'),('JP8000 LEAD 40 C5.wav','Aurora Beam',60,'Lead'),('JPGT2 - Lead 13.wav','Starlight Alloy',60,'Lead'),('Kore Lead_24.wav','Violet Core',60,'Lead'),('Lead 01.wav','Infinity Signal',60,'Lead'),('lead 5.wav','Spectral Flare',76,'Lead'),('lead 13.wav','Orbital Vector',45,'Lead'),
('Stickz - Chroma Synth One Shot 184 - C.wav','Chromatic Orbit',60,'Lead'),('JP 8000 SYNTH 00 C5 (Drake Liddell Fav Preset).wav','Eclipse Crown',60,'Lead'),('VFX2 Oneshots 140.wav','Deep Space Relay',40,'Lead'),('VTDS1 Synth Oneshots 102 C.wav','Pulse Meridian',29,'Lead'),('Stickz - Smile Synth Shot 106 - C.wav','Radiant Flight',24,'Lead')]
rows += [('bass 18.wav','Dark Matter',36,'Bass'),('bass 28.wav','Sub Reactor',33,'Bass'),('bass 30.wav','Gravity Well',33,'Bass'),('OTHTS1_Stab_10.wav','Ion Impact',36,'Bass'),('OTHTS1_Bass_03.wav','Void Anchor',24,'Bass'),('OTHTS1_Bass_28.wav','Plasma Roller',36,'Bass'),('OTHTS1_Bass_30.wav','Quantum Engine',36,'Bass'),('Kore Bass_10.wav','Magnetic Knock',36,'Bass'),('Kore Bass_25.wav','Low Orbit',29,'Bass'),('Kore Bass_27.wav','Reactor Coil',38,'Bass'),('VEC3 Bass C 026.wav','Proton Punch',36,'Bass'),('VEC3 Bass C 039.wav','Metallic Undertow',36,'Bass'),('VEC3 Bass C 065.wav','Violet Pressure',36,'Bass'),('VEC3 Bass C 177.wav','Blackstar Sub',24,'Bass'),('VEC3 Bass C 199.wav','Deep Field',36,'Bass')]
inv=json.loads((root/'docs/audit/NATIVE_PARAMETER_INVENTORY.json').read_text());base={p['id']:p['init_value'] for p in inv['parameters']};base.update(sampleOscEnabled=0,sampleStereo=1,sampleFidelity=1,sampleGain=0)
archive=out/'SPACE_INFINITY_Orbit_Voices.zip';previous={}
if archive.exists():
 with zipfile.ZipFile(archive) as z:previous={r['id']:r for r in json.loads(z.read('Bank.json'))['presets']}
calibration=out/'CALIBRATION.json';gains=json.loads(calibration.read_text()) if calibration.exists() else {}
manifest=[];designs=[];provenance=[];payload=[]
for index,(original,name,key,category) in enumerate(rows):
 filename='Infinity_'+name.replace(' ','_')+'.wav';dest=assets/filename
 if not dest.exists():
  found=list(Path('/workspace/attachments').glob('*/'+original))
  if not found:raise SystemExit('Missing uploaded source: '+original)
  shutil.copyfile(found[-1],dest)
 data=dest.read_bytes();encoded=juce64(data);provenance.append(dict(original=original,file=filename,sha256=hashlib.sha256(data).hexdigest(),root_midi=key,source='Owner upload'))
 for variation in range(3 if category=='Choir' else 1):
  patchid=f'O{len(manifest)+1:03d}';title=name+([' Stab',' Cathedral',' Hybrid'][variation] if category=='Choir' else '')
  settings=dict(base);settings.update(source=3,sampleRoot=key,sampleLoop=1,sampleLoopFade=35,sampleStart=.08 if category=='Choir' else .04,sampleEnd=.86,attack=.0015,ampHold=0,ampSustain=.72,ampDecay=.3,release=.12,gain=.75,presetTrim=gains.get(patchid,-8),ampVelocity=.25,routeA=1,cutoff=9000,filterEnvAmount=.7,filterDecay=.26,filterSustain=.4,filterMode=1,res=.13,engineMode=1,analogueCharacter=0,drive=0,subLevel=0,noiseLevel=0,levelMatch=0,osc2Enabled=1,osc1Pan=0,osc2Pan=0,sampleFidelity=1,sampleStereo=1,sampleGain=0,delayEnabled=1,echoMode=2,echoMix=.12,delayFeedback=.22,echoTimeL=.28,echoTimeR=.42,reverbEnabled=1,reverb=.16,chorusEnabled=0,motionDepth=0,lfodepth=0)
  for flag in ['compEnabled','flangerEnabled','phaserEnabled','gateEnabled','pumpEnabled','eqEnabled','convolverEnabled','distEnabled','analogueLabEnabled']:settings[flag]=0
  if category=='Choir':
   settings.update(ampSustain=.12,ampDecay=.2,release=.09,sampleStart=.13,cutoff=6000)
   if variation==1:settings.update(attack=.035,ampSustain=.8,ampDecay=.8,release=1.1,cutoff=3800,reverb=.3,echoMix=.08)
   if variation==2:settings.update(sampleOscEnabled=1,osc2Wave=1+([2,3,14,17][index%4]),osc2Gain=-4,osc2Unison=3,osc2Detune=.08,modernUnison=1,manualUnison=1,osc2Octave=-1,osc2Width=.5,osc2WTPos=.25,independentWTPos=1,ampSustain=.5,ampDecay=.35,release=.2,routeB=1)
  else:
   settings.update(ampSustain=[.9,.65,.8,.5][index%4],ampDecay=[.24,.4,.65,.18][index%4],release=[.1,.22,.35,.07][index%4],cutoff=[8000,12000,5400,16000][index%4],filterEnvAmount=[.2,1.1,.6,0][index%4],echoMix=[.1,.18,.07,.14][index%4],reverb=[.12,.22,.09,.18][index%4])
  if category=='Bass':
   settings.update(sampleLoop=index%3==0,sampleStart=0,sampleEnd=1,sampleLoopFade=10,voiceMode=1,glideTime=0,ampSustain=.62 if index%3==0 else .08,ampDecay=[.12,.2,.28][index%3],release=.025,cutoff=[900,1800,3200][index%3],filterEnvAmount=1.2,filterDecay=.1,filterSustain=0,reverbEnabled=0,delayEnabled=0,reverb=0,echoMix=0,ampVelocity=.3)
  matched=root/'native-vst3/Presets/Installed094.json'
  if matched.exists():settings.update(json.loads(matched.read_text())['settings'].get('Uuser_orbitvoices093_'+patchid,{}))
  x=E.Element('PARAMETERS',presetDisplayName=title,creator='Orbital Audio',tags=f'{category}, Orbit Voices, User Samples, INFINITY',presetKey='Uuser_orbitvoices093_'+patchid,sampleName=filename,sampleWav=encoded,macroLabel0='OPEN',macroLabel1='SPACE',macroLabel2='BODY')
  if matched.exists():x.set('tags',x.get('tags')+', Level Matched 094')
  for m,(target,depth) in enumerate([('cutoff',.2),('reverb',.25),('sampleGain',.03)]):x.set(f'freeMacroTarget{m}_0',target);settings[f'freeMacroDepth{m}_0']=depth
  assert set(settings)<=set(base), 'Unknown native settings: '+str(set(settings)-set(base))
  for k,v in settings.items():E.SubElement(x,'PARAM',id=k,value=str(int(v) if isinstance(v,bool) else v))
  xml=E.tostring(x,encoding='utf-8');binary=struct.pack('<II',0x21324356,len(xml))+xml+b'\0';file=f'Presets/{category}/{patchid} {title}.sipreset';row=dict(id=patchid,name=title,category=category,creator='Orbital Audio',tags=f'{category}, Orbit Voices, User Samples, INFINITY',file=file,sha256=hashlib.sha256(binary).hexdigest())
  if patchid in previous:
   old=previous[patchid];row['previous_sha256']=old['sha256'];row['previous_sha256s']=list(dict.fromkeys([old['sha256'],old.get('previous_sha256','')]+old.get('previous_sha256s',[])))[:32]
  manifest.append(row);payload.append((file,binary));designs.append(dict(id=patchid,name=title,category=category,settings={k:v for k,v in settings.items() if base.get(k)!=v},sample_file=str(dest.relative_to(root)),root_midi=key))
with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED,compresslevel=7) as z:
 for file,binary in payload:z.writestr(file,binary)
 z.writestr('Bank.json',json.dumps(dict(format='SPACE_INFINITY_BANK_V2',minimum_version='0.9.4' if (root/'native-vst3/Presets/Installed094.json').exists() else '0.9.3',id='orbitvoices093',name='INFINITY Orbit Voices',presets=manifest),indent=2))
(out/'DESIGNS.json').write_text(json.dumps(dict(presets=designs),indent=2)+'\n');(root/'docs/ORBIT_SAMPLES_093_PROVENANCE.json').write_text(json.dumps(dict(samples=provenance),indent=2)+'\n');(out/(archive.name+'.sha256')).write_text(hashlib.sha256(archive.read_bytes()).hexdigest()+'  '+archive.name+'\n')
print(f'{len(provenance)} renamed sources; {len(manifest)} authored presets; calibration {bool(gains)}')
