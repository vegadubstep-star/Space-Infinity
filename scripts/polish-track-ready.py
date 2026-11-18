"""Apply authored sound/performer changes to an existing design, retaining IDs and assets."""
import argparse,json
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('input',type=Path);p.add_argument('output',type=Path);a=p.parse_args();bank=json.loads(a.input.read_text());is_factory=any('reference' in r for r in bank['presets'])
hot=json.loads((Path(__file__).resolve().parent/'heritage/hot-character-updates.json').read_text())
character=json.loads((Path(__file__).resolve().parent/'heritage/character-voicings.json').read_text())
# Specific timbral roles, chosen against the reviewed neighbouring patches.
voices={
 'Glass Meridian':(37,.08),'Pearl Harbour':(38,.22),'Neon Compass':(44,.18),
 'Quartz Horizon':(49,.30),'Deep Thistle':(48,.16),'Crystal Relay':(51,.35),
 'Solar Promise':(47,.18),'Orbital Steel':(50,.40),'Blue Wonder':(42,.12),
 'Power District':(39,.25),'Ocean Descent':(43,.18),'Energetic Fifth':(45,.12),
 'Warm Airfoil':(38,.18),'Resonant Launch':(51,.22),'Deep Aurora':(46,.16),
 'Triangle Anchor':(37,.10),'Sunrise Keys':(49,.12),'Amber Supersky':(44,.18),
 'Digital Plectrum':(48,.25),'Neon Harp':(51,.08),'Bright Compass':(47,.12),
 'Hollow Signal':(42,.16),'Crystal Flight':(38,.08),'Pulse Raindrop':(39,.18),
 'Anthem Fragment':(45,.10),'Seven Sparks':(40,.10),'Silver Icicle':(43,.28),
 'Metal Fjord':(50,.35),'Scarlet Saw':(41,.14)}
stable={'Power District':39,'Yellow Cathode':41,'Rage Vector':42,'Saucer Alloy':43,'Never Over':44,'Rave Congregation':42,'Temple Choir':39,'Cathedral Strike':39,'Soprano Blade':41,'Ooh Colossus':40,'Monastery Impact':43,'Vocal Circuit':44}
for row in bank['presets']:
 s=row['settings'];s['outputSafety']=1;category=row['category'].lower();mods_key='macros' if is_factory else 'modulation';mods=row.get(mods_key,[])
 if row['name'] in character:
  s.update(character[row['name']])
  if s.get('wave')!=30:row['wavetable']=None
 if row['name'] in hot:s.update(hot[row['name']]);s['outputSafety']=1
 if row['name']=='Festival Arco':s.update(attack=.035,ampDecay=.55,ampSustain=.68,release=.22,ampLong=0,cutoff=8800,reverb=.14,echoMix=.04,eq3Frequency=400,eq3Gain=-2.5)
 if row['name']=='Trembling Skyline':s.update(source=24,sampleLoop=1,attack=.85,ampDecay=1.1,ampSustain=.88,release=.9,ampLong=0,cutoff=5200,filterAttack=1.2,filterEnvAmount=1,filterSustain=.8)
 if row['name'] in voices and s.get('source',0)==0:
  s['wave'],s['wavetableScan']=voices[row['name']]
  if s.get('wave')!=30:row['wavetable']=None if not is_factory else row.get('wavetable')
 if row['name'] in stable:
  s.update(source=stable[row['name']],sampleLoop=1,oscMelt=0,echoMix=.055,reverb=.12,osc2Enabled=0,mix=0,chorusEnabled=0,phaserEnabled=0,flangerEnabled=0,designerLfo1Enabled=0,designerLfo2Enabled=0,designerLfo3Enabled=0,noiseLevel=0,filterMode=4,cutoff=7600,filterEnvAmount=.5,ampTimed=1)
  row.pop('lfo',None)
 makina={
  'Narrow Laser':(4,3,.06,.24,4300,2,.03),
  'Silver Ribbon':(47,5,.10,.08,6200,0,0),
  'Hollow Signal':(14,1,.02,.13,3600,2,.045),
  'Metal Fjord':(50,1,.01,.11,5400,1,.015),
  'Bright Compass':(0,7,.18,.035,6500,0,0),
  'Pulse Ice':(39,3,.04,.08,4700,2,.04),
  'Nordic Fifth':(45,3,.07,.06,5900,0,0),
  'Steel Ribbon':(41,2,.035,.09,7200,2,.025)}
 if row['name'] in makina:
  wave,unison,detune,warp,cutoff,mode,glide=makina[row['name']]
  s.update(source=0,wave=wave,osc2Enabled=1,osc2Wave=3,osc2Octave=-1,mix=.13,osc1Unison=unison,osc2Unison=1,osc1Detune=detune,osc2Detune=0,osc1Width=.65 if mode==0 else .25,osc2Width=0,osc1Warp=1 if row['name'] in ('Metal Fjord','Hollow Signal') else 3,osc1WarpAmount=warp,filterMode=4,cutoff=cutoff,res=.16,filterEnvAmount=.65,filterDecay=.14,filterSustain=.25,attack=.0008,ampDecay=.24,ampSustain=.72,release=.085,ampLong=0,voiceMode=mode,glideTime=glide,delayEnabled=1,echoMix=.075,reverbEnabled=1,reverb=.095,chorusEnabled=0,flangerEnabled=0,phaserEnabled=0,distEnabled=1,distType=0,distDrive=.18,distInput=3,distMix=.14,distTone=9000,fxOutput8=-1.5,eq0Frequency=160,pitchEnvAmount=.65 if row['name']=='Narrow Laser' else 0,pitchEnvDecay=.025)
  row['wavetable']=None;row['macro_labels']=['TONE','RAVE','EDGE']
  mods=[m for m in mods if m['source'] not in (0,1,2)]
  mods += [dict(source=0,target='cutoff',depth=.17),dict(source=1,target='osc1WarpAmount',depth=.12),dict(source=1,target='oscMelt',depth=.16),dict(source=1,target='echoMix',depth=-.035),dict(source=2,target='distDrive',depth=.15),dict(source=2,target='distMix',depth=.10)]
  row['tags']+=', Makina, Spanish Rave, Fast Attack'
  row['description']='Original Makina-style '+('FM/reed' if s['osc1Warp']==1 else 'pulse/saw')+' lead: fast articulation, restrained supporting layer, short spatial tail. TONE opens the filter; RAVE adds harmonic interaction; EDGE adds controlled bite. Genre interpretation, not an artist patch copy.'
 if 'pluck' in category:
  if s.get('filterMode') in (2,3):s['filterMode']=4
  s.update(delayEnabled=1,reverbEnabled=1)
  s['echoMix']=max(.13,min(.23,s.get('echoMix',.17)));s['reverb']=max(.15,min(.25,s.get('reverb',.18)))
  s.update(delayWetLowFreq=240,delayWetLowGain=-10,reverbWetLowFreq=280,reverbWetLowGain=-12)
  mods=[m for m in mods if m['source']!=0]
  mods += [dict(source=0,target='cutoff',depth=.43),dict(source=0,target='echoMix',depth=-s['echoMix']*.78),dict(source=0,target='reverb',depth=-s['reverb']*.70),dict(source=0,target='reverbSize',depth=.08),dict(source=0,target='filterInputDrive',depth=.025),dict(source=0,target='osc1Width',depth=.10),dict(source=0,target='osc2Width',depth=.10),dict(source=0,target='presetTrim',depth=-row.get('open_compensation_db',2.16)/48)]
  if s.get('source',0)==0:
   mods += [dict(source=0,target='ampSustain',depth=max(0,.78-s.get('ampSustain',0))),dict(source=0,target='ampDecay',depth=.10),dict(source=0,target='oscMelt',depth=.12)]
  row['macro_labels']=['OPEN / LEAD','CONTOUR','CHARACTER']
  row['play' if is_factory else 'description']='OPEN / LEAD opens the filter while reducing echo and reverb wet levels; adds a little drive, width and room size. Synth plucks gain sustain; sampled instruments keep their natural decay. Velocity shapes expression.'
 if 'lead' in category and s.get('source',0)==0:
  s['attack']=min(s.get('attack',.001),.003);s['oscMelt']=.035 if s.get('osc2Enabled',0) else 0
 if row['name']=='Velvet FM Sub':s.update(osc1WarpAmount=.16,filterDecay=.095,filterEnvAmount=1.05,cutoff=1100,ampDecay=.13,ampSustain=.34,release=.035,mix=.025,subLevel=.16)
 if 'bass' in category:
  s['subPan']=0;s['subWave']=0;s['noiseLevel']=min(s.get('noiseLevel',0),.005)
 if 'pad' in category and s.get('source',0)==0:
  s.update(chorusEngine=1,dimensionMode=2,dimensionTone=.62,dimensionWidth=1.12)
  s['chorus']=min(.22,max(.12,s.get('chorus',0)));s['chorusEnabled']=1
 if isinstance(row.get('tags'),str):row['tags']=', '.join(dict.fromkeys(t.strip() for t in row['tags'].split(',') if t.strip() and t.strip() not in ('Skudero Style Reference','Sistema 3 Style Reference')))
 row[mods_key]=mods
 row['review_revision']='track-ready-080'
a.output.write_text(json.dumps(bank,indent=2)+'\n');print('Reviewed',len(bank['presets']),'designs:',a.output)
