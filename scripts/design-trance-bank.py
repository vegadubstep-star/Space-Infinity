#!/usr/bin/env python3
"""Authored trance-bank blueprints; no random preset generation or song MIDI."""
import hashlib,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
INV=json.loads((ROOT/'docs/audit/NATIVE_PARAMETER_INVENTORY.json').read_text())
PARAMS={r['id']:r for r in INV['parameters']}
INIT={r['id']:r['init_value'] for r in INV['parameters'] if r['init_value']!=r['host_default']}
BANK=[]
BASE=dict(INIT,source=0,osc1Enabled=1,osc2Enabled=1,osc1Gain=0,osc2Gain=0,osc1Octave=0,osc2Octave=0,osc2Semitone=0,osc2Tune=0,mix=.25,osc1Unison=1,osc2Unison=1,osc1Detune=.08,osc2Detune=.06,osc1Blend=.75,osc2Blend=.75,osc1Width=.65,osc2Width=.7,osc1UnisonMode=1,osc2UnisonMode=1,osc1Rand=1,osc2Rand=1,osc1PhaseReset=0,osc2PhaseReset=0,manualUnison=1,modernUnison=1,independentWTPos=1,wavetableScan=.18,osc2WTPos=.27,wave=0,osc2Wave=2,osc1Warp=0,osc2Warp=0,osc1WarpAmount=0,osc2WarpAmount=0,subEnabled=1,subLevel=0,noiseLevel=0,analogueUpgrade=1,analogueCharacter=.055,analogueLabEnabled=1,analogueOscDrift=.035,analogueThermal=.02,analogueFilterSpread=.035,analogueEnvSpread=.02,analogueVoiceSpread=.025,analogueVCADrive=.025,analogueAsymmetry=.015,analogueMemory=.015,analogueCoupling=.01,cleanVCO=1,stereoFilterUpgrade=1,declickEnabled=1,vibratoEnabled=0,lfodepth=0,motionDepth=0,patternMode=0,gateEnabled=0,pumpEnabled=0,chorusEnabled=0,phaserEnabled=0,flangerEnabled=0,delayEnabled=0,reverbEnabled=0,compEnabled=0,convolverEnabled=0,eqEnabled=1,eqMode0=4,eq0Frequency=25,eq0Q=.7071,eqMode6=3,eq6Frequency=14500,eq6Q=.7071,filterMode=4,cutoff=6000,res=.12,filterInputDrive=.03,drive=.025,filterAttack=.001,filterDecay=.2,filterSustain=0,filterRelease=.1,filterEnvAmount=0,filterVelocity=.35,filterEnvTiming=0,ampVelocity=.5,ampAttackCurve=-.35,ampDecayCurve=.1,ampReleaseCurve=.05,filterAttackCurve=-.35,filterDecayCurve=.15,filterReleaseCurve=0,routeA=1,routeB=1,routeSub=0,routeNoise=1,gain=.38,voiceMode=0,glideTime=0,pitchBend=0,midiBendRange=2,sampleStart=0,sampleEnd=1,sampleLoop=0,sampleReverse=0)
# Nonexistent spelling is removed before validation; native timing is already in seconds.
BASE.pop('filterEnvTiming',None)
for i in range(3): BASE['macro'+str(i)]=0;BASE['freeMacro'+str(i)]=0;BASE['designerLfo'+str(i+1)+'Enabled' if i else 'designerLfo1Enabled']=0

def env(a=.0007,d=.18,s=.65,r=.08):
    long=max(a,r)>1
    return dict(attack=a/(3 if long else 1),ampDecay=d,ampSustain=s,release=r/(3 if long else 1),ampLong=int(long),ampTimed=1)
def space(delay=0,reverb=0,chorus=0,room=1):
    return dict(delayEnabled=int(delay>0),echoMode=2,echoSyncL=4,echoSyncR=5,echoLink=0,echoMix=delay,delayFeedback=.28,echoFrequency=4200,echoBandwidth=.64,reverbEnabled=int(reverb>0),reverb=reverb,reverbSize=.78,reverbDamping=.42,reverbWidth=1,reverbSpace=room,chorusEnabled=int(chorus>0),chorus=chorus,chorusRate=.16,chorusDepth=.3,chorusDelay=11,chorusFeedback=.04,delayWetLowFreq=220,delayWetLowGain=-9,delayWetHighFreq=6500,delayWetHighGain=-3,reverbWetLowFreq=240,reverbWetLowGain=-12,reverbWetHighFreq=7200,reverbWetHighGain=-4)
def tone(kind):
    profiles={
      'saw':dict(wave=0,osc2Wave=6,osc1Unison=3,osc2Unison=1,mix=.22),
      'pulse':dict(wave=1,osc2Wave=2,osc1Warp=3,osc1WarpAmount=.2,mix=.16),
      'fm':dict(wave=2,osc2Wave=3,osc1Warp=4,osc1WarpAmount=.12,osc2Octave=1,mix=.08),
      'reed':dict(wave=13,osc2Wave=3,mix=.23),
      'round':dict(wave=6,osc2Wave=2,mix=.22,osc1Unison=2),
      'vowel':dict(wave=10,osc2Wave=11,mix=.35,filterMode=3),
      'narrow':dict(wave=4,osc2Wave=2,mix=.27,osc1Warp=3,osc1WarpAmount=.35),
      'sync':dict(wave=22,osc2Wave=0,osc1Warp=2,osc1WarpAmount=.16,mix=.24),
      'tri':dict(wave=3,osc2Wave=2,mix=.18,osc1Warp=1,osc1WarpAmount=.035),
      'octave':dict(wave=0,osc2Wave=1,osc2Octave=1,mix=.3,osc1Unison=3),
      'fifth':dict(wave=6,osc2Wave=0,osc2Semitone=7,mix=.18,osc1Unison=3),
      'metal':dict(wave=3,osc2Wave=2,osc1Warp=1,osc1WarpAmount=.13,osc2Octave=1,mix=.12),
      'organ':dict(wave=18,osc2Wave=2,osc2Octave=-1,mix=.2),
      'acid':dict(wave=31,osc2Enabled=0,mix=0,filterMode=5),
      'sub':dict(wave=2,osc2Enabled=0,mix=0,filterMode=4)}
    return profiles[kind].copy()
def add(name,category,reference,settings=None,kind=None,**extras):
    p=BASE.copy();p.update(env());
    if kind:p.update(tone(kind))
    p.update(settings or {})
    BANK.append(dict(id='TI'+str(len(BANK)+1).zfill(3),name=name,category=category,reference=reference,settings=p,**extras))

def basses(category,rows,rolling=False):
    for name,reference,kind,cutoff,decay,res,body in rows:
        p=dict(voiceMode=1,osc1PhaseReset=1,osc2PhaseReset=1,osc1Rand=0,osc2Rand=0,osc1Width=.08,osc2Width=.07,osc1Detune=.035,osc2Detune=.025,cutoff=cutoff,res=res,filterEnvAmount=body,filterDecay=decay*.7,filterSustain=0,filterVelocity=.6,subLevel=.10 if kind not in ('sub','fm','organ') else 0,subOctave=0,filterInputDrive=.08,drive=.06,eq0Frequency=27,eq6Frequency=7500,gain=.42)
        p.update(env(.0005,decay,.12 if rolling else .27,.028 if rolling else .045))
        if kind=='vowel':p.update(filterMode=4,cutoff=cutoff,eq3Frequency=850,eq3Gain=2.5)
        if kind=='metal':p.update(osc1WarpAmount=.065,filterInputDrive=.12)
        if kind=='fifth':p.update(mix=.1)
        if kind=='sync':p.update(osc1WarpAmount=.10)
        add(name,category,reference,p,kind,play='Play your own short MIDI notes, typically MIDI 36-55. No note generator, gate or pump.',sources=['R_JP','R_BASS','R_TB'])

basses('Offbeat Bass Stabs',[
 ('Tunnel Knock','Cocooma - District of Power','saw',950,.17,.18,2.2),('Reptile Rubber','Mauro Picotto - Lizard','pulse',640,.22,.28,2.8),('Gate Impact','Cosmic Gate - Exploration of Space','fm',1350,.15,.17,1.7),('Yellow Reactor','Cocooma - The Yellow Base','reed',780,.19,.24,2.5),('Ocean Hammer','Cosmic Gate - Melt to the Ocean','round',1100,.23,.12,2.0),('District Vowel','Cocooma - District of Power','vowel',1600,.16,.23,1.5),('Razor Offbeat','Hennes & Cold - The Second Trip','narrow',730,.14,.22,3.0),('Steel Bounce','Scot Project - U','sync',1200,.18,.16,2.1),('Velvet Punch','Kai Tracid - Liquid Skies','tri',860,.24,.20,2.4),('Octane Kickback','DJ Scot Project - O','octave',920,.18,.13,2.6),('Power Fifth','Cocooma - Rage','fifth',1700,.15,.14,1.3),('Foundry Bite','Gary D. - Kinetic Pressure','metal',1400,.13,.25,2.2),('Cathedral Thud','Jones & Stephenson - The First Rebirth','organ',690,.21,.19,2.7),('Acid Anchor','Hardfloor - Acperience 1','acid',430,.19,.56,3.8),('Warm Concrete','Legend B - Lost in Love','round',520,.28,.30,2.9)])
for name,kind,cut,d,s,drive in [('Sine Foundation','sub',500,.22,.15,.01),('Triangle Pressure','tri',310,.18,.18,.025),('Filtered Pulse Sub','pulse',170,.16,.22,.04),('Velvet FM Sub','fm',260,.20,.10,.015),('Octave Undertow','organ',220,.24,.16,.035)]:
 p=dict(voiceMode=1,osc1Unison=1,osc2Unison=1,osc1Width=0,osc2Width=0,osc1PhaseReset=1,osc2PhaseReset=1,osc1WarpAmount=.018,cutoff=cut,res=.06,filterEnvAmount=.3,filterVelocity=.15,filterInputDrive=drive,drive=drive,eq0Frequency=22,eq6Frequency=1300,ampVelocity=.35,gain=.42);p.update(env(.0015,d,s,.032));add(name,'Sub Bass Stabs','Classic trance sine/triangle sub foundations',p,kind,play='Play MIDI 24-48; dry mono low end, no sequencing.',sources=['R_JP'])
basses('Rolling Bass',[
 ('Sixteenth Engine','Astrix - Deep Jungle Walk','saw',1250,.11,.12,2.3),('Binary Runner','Paul van Dyk - For an Angel','pulse',780,.105,.16,2.8),('FM Rail','Cosmic Gate - The Drums','fm',1600,.09,.15,1.9),('Reed Motor','Mauro Picotto - Iguana','reed',1050,.13,.20,2.1),('Rounded Momentum','System F - Out of the Blue','round',1450,.12,.11,1.7),('Vocal Runner','Mauro Picotto - Komodo','vowel',1800,.10,.21,1.4),('Needle Motion','Scot Project - F','narrow',820,.085,.19,3.2),('Sync Conveyor','Cocooma - Energetic','sync',1300,.12,.17,2.6),('Triangle Circuit','Kai Tracid - Your Own Reality','tri',950,.145,.16,2.4),('Octave Rotor','Cosmic Gate - Fire Wire','octave',1700,.10,.12,2.0),('Fifth Locomotion','Cocooma - Virtual Experience','fifth',1950,.095,.12,1.4),('Metal Pursuit','Cocooma - Flying Saucer','metal',1400,.09,.22,2.3),('Drawbar Roller','BBE - Seven Days and One Week','organ',800,.12,.10,2.1),('Acid Tread','Emmanuel Top - Acid Phase','acid',560,.115,.51,3.3),('Dark Flywheel','Push - Universal Nation','round',650,.14,.26,3.0)],True)

uplifting=[
 ('Azure Ascension','System F - Out of the Blue','saw',7,5,0,5600,.14,.72,.22,.16),('Gilded Flight','Gouryella - Gouryella','octave',7,3,-1,6800,.18,.70,.24,.22),('Northern Horizon','Rank 1 - Airwave','round',8,6,1,4700,.12,.82,.35,.14),('Universal Flame','Push - Universal Nation','pulse',5,3,0,4300,.23,.62,.18,.10),('Blanche Skyline','Veracocha - Carte Blanche','saw',7,7,0,7900,.11,.76,.21,.19),('Solar Promise','Above & Beyond - Sun & Moon','narrow',7,3,-1,6200,.17,.67,.19,.13),('Frost Anthem','Armin van Buuren - Shivers','tri',8,5,0,8500,.10,.74,.23,.15),('Angel Alloy','Paul van Dyk - For an Angel','metal',3,3,0,6900,.16,.64,.15,.08),('Silver Alone','Alice Deejay - Better Off Alone','saw',1,1,0,11200,.08,1,.025,0),('Desert Blade','Darude - Sandstorm','pulse',1,1,2,9800,.36,.68,.025,.015)]
for name,ref,kind,ua,ub,octave,cut,res,sus,rel,det in uplifting:
 p=dict(osc1Unison=ua,osc2Unison=ub,osc2Octave=octave,cutoff=cut,res=res,osc1Detune=det,osc2Detune=det*.7,osc1Width=.84,osc2Width=.88,filterEnvAmount=.7,filterDecay=.2,filterSustain=.35,drive=.065,eq0Frequency=130,gain=.34);p.update(env(.0005,.32,sus,rel));p.update(space(.14,.18,.12))
 if name=='Silver Alone':p.update(osc2Enabled=0,mix=0,voiceMode=1,filterEnvAmount=0,chorusEnabled=0,echoMix=.08,reverb=.10)
 if name=='Desert Blade':p.update(wave=0,osc2Wave=1,osc2Semitone=7,mix=.40,osc1Warp=2,osc1WarpAmount=.23,drive=.24,filterInputDrive=.10,voiceMode=1,echoMix=.055,reverb=.08)
 if name=='Frost Anthem':p.update(wave=0,osc2Wave=3,mix=.32)
 add(name,'Uplifting Leads',ref,p,kind,play='Play melodies or open chords; near-zero attack. Pitch wheel adds expression.',sources=['R_JP','R_FERRY','R_SAND','R_ALONE'])

hard=[
 ('Reptile Signal','Mauro Picotto - Lizard','pulse',950,.42,3,.14),('Iguana Chrome','Mauro Picotto - Iguana','fm',2200,.31,3,.18),('Komodo Breath','Mauro Picotto - Komodo','reed',3900,.18,3,.08),('Proxima Horn','Mauro Picotto - Proximus','narrow',4200,.22,5,.15),('Baguette Voltage','Mauro Picotto - Baguette','sync',6100,.27,3,.22),('Orbital Steel','Cosmic Gate - Exploration of Space','metal',5900,.25,3,.24),('Ion Wire','Cosmic Gate - Fire Wire','octave',7800,.15,8,.34),('Ocean Descent','Cosmic Gate - Melt to the Ocean','round',3500,.30,5,.19),('Rainbow Pressure','Cosmic Gate - Somewhere Over the Rainbow','fifth',8500,.13,7,.20),('Earthbound Razor','Cosmic Gate - Back to Earth','acid',4300,.51,3,.18)]
for name,ref,kind,cut,res,unison,warp in hard:
 p=dict(osc1Unison=unison,osc2Unison=3,osc1Detune=.10,osc2Detune=.065,osc1Width=.65,osc2Width=.72,cutoff=cut,res=res,filterEnvAmount=1.4,filterDecay=.21,filterSustain=.28,osc1WarpAmount=warp,drive=.16,filterInputDrive=.12,eq0Frequency=110,voiceMode=2,glideTime=.035,gain=.32);p.update(env(.0004,.26,.62,.14));p.update(space(.12,.12,.06))
 if name=='Reptile Signal':p.update(env(.012,.19,.35,.10));p.update(pitchEnvAmount=2,pitchEnvAttack=.02,pitchEnvDecay=.13)
 if name=='Komodo Breath':p.update(source=20,filterEnvAmount=.45,cutoff=6500,sampleLoop=0,ampVelocity=.6)
 if name=='Ion Wire':p.update(gateEnabled=1,gateDepth=.78,gateRate=3,gateAttack=1,gateRelease=4,voiceMode=0)
 add(name,'Hard Trance Leads',ref,p,kind,play='Direct lead performance; Ion Wire includes a bypassable rhythmic gate.',sources=['R_COSMIC','R_PICOTTO','R_JP'])

german=[
 ('Virtual Voltage','Virtual Experience','saw',6,5,7200,.18,.18),('Power District','District of Power','pulse',3,3,3200,.38,.12),('Rage Vector','Rage','sync',5,1,9800,.21,.28),('Saucer Alloy','Flying Saucer','fm',3,3,5700,.25,.19),('Escape Velocity','The Escape','narrow',5,3,4700,.31,.14),('Yellow Cathode','The Yellow Base','vowel',3,3,3600,.34,.17),('Energetic Fifth','Energetic','fifth',7,3,8900,.16,.22),('Metronomic Edge','Metronoom','metal',3,1,6100,.27,.16),('Blue Wonder','Blaues Wunder','round',8,5,6800,.19,.21),('Never Over','Nothing Is Over','octave',7,5,10400,.12,.24)]
for name,track,kind,ua,ub,cut,res,drive in german:
 p=dict(osc1Unison=ua,osc2Unison=ub,osc1Detune=.085,osc2Detune=.06,osc1Width=.74,osc2Width=.80,cutoff=cut,res=res,drive=drive,filterInputDrive=.10,filterEnvAmount=1.1,filterDecay=.19,filterSustain=.35,eq0Frequency=135,voiceMode=1,gain=.32);p.update(env(.0003,.25,.60,.11));p.update(space(.11,.10,.06))
 if kind=='vowel':p.update(filterMode=3,cutoff=980,filterEnvAmount=.55,eq4Frequency=2300,eq4Gain=1.5)
 if kind=='metal':p.update(pitchEnvAmount=.8,pitchEnvAttack=0,pitchEnvDecay=.025)
 add(name,'90s German Leads','Cocooma - '+track,p,kind,play='Fast-attack 90s hard-trance lead; play your own melody.',sources=['R_COCOOMA','R_JP'])

plucks=[
 ('Glass Meridian','Chicane - Saltwater','tri',0,6200,.34,.18),('Pearl Harbour','Energy 52 - Cafe del Mar','round',0,3500,.28,.24),('Emerald Chime','PPK - Resurrection','sub',14,11500,.42,.35),('Silk Pizzicato','Faithless - Insomnia','sub',26,7200,.19,.16),('Mallet Moon','ATB - 9PM atmosphere','sub',15,5400,.24,.20),('Balearic Nylon','ATB - 9PM guitar-like articulation','sub',16,6800,.31,.25),('Neon Compass','Sasha - Xpander','sync',0,3900,.23,.22),('Pearl Clavicle','BBE - Seven Days and One Week','sub',17,5700,.20,.16),('Vowel Snow','Jam & Spoon - Stella','vowel',0,2600,.38,.30),('Needle Arc','Kai Tracid - Liquid Skies','narrow',0,4300,.16,.12),('Twilight Meow','ATB - 9PM','saw',0,4600,.24,.18),('Nineties Drawbar','Robert Miles - Fable atmosphere','organ',0,3300,.27,.20),('Quartz Horizon','Push - Strange World','reed',0,6500,.34,.28),('Ribbon Pulse','York - On the Beach synth colour','pulse',0,3100,.22,.17),('Ice Drops','Paul van Dyk - Tell Me Why','metal',0,8900,.45,.33),('Copper Strings','Cocooma - The Escape articulation','sub',25,5600,.16,.12),('Prism Fold','Sasha - Xpander morphic colour','fm',0,7400,.29,.24),('Deep Thistle','Humate - Love Stimulation','tri',0,1800,.37,.30),('Cathedral Tear','Delerium - Silence atmosphere','sub',13,8300,.55,.40),('Crystal Relay','BT - Flaming June texture','fifth',0,5200,.26,.22)]
for name,ref,kind,source,cut,d,r in plucks:
 p=dict(source=source,cutoff=cut,res=.17,filterEnvAmount=1.6 if not source else .5,filterDecay=d*.65,filterSustain=0,osc1Unison=3 if kind in ('saw','round','pulse','fifth') else 1,osc2Unison=1,osc1Detune=.07,osc1Width=.6,osc2Width=.55,drive=.025,eq0Frequency=150,gain=.36);p.update(env(.0007,d,0,r));p.update(space(.20,.20,.06))
 if name=='Glass Meridian':p.update(osc1Warp=4,osc1WarpAmount=.075)
 if name=='Twilight Meow':p.update(voiceMode=2,glideTime=.075,pitchEnvAmount=2,pitchEnvAttack=.018,pitchEnvDecay=.09,filterEnvAmount=1.2)
 if name=='Cathedral Tear':p.update(convolverEnabled=1,irPreset=2,convolverMix=.10,irDecay=2.5,reverb=.12)
 if name=='Needle Arc':p.update(filterMode=3,cutoff=900,filterEnvAmount=.45,res=.40,osc1WarpAmount=.12)
 if name=='Vowel Snow':p.update(cutoff=800,filterEnvAmount=.35,res=.32)
 if name=='Copper Strings':p.update(ampDecay=.42,release=.18)
 add(name,'Trance Plucks',ref,p,kind,play='Short MIDI notes or held chords; each patch has its own decay and echo character.',sources=['R_ATB','R_JP','R_STRINGS'])

for category,rows in [('Airy Pads',[
 ('Skywave Vista','Rank 1 - Airwave','saw',0,4200,.65,2.6),('Polar Choir','Chicane - Saltwater atmosphere','sub',7,5200,.9,2.2),('Granular Stardust','BT - Flaming June atmosphere','tri',4,4800,1.0,2.8),('Aether Wide','Gouryella - Walhalla','octave',0,5700,.8,2.4),('Ooh Dawn','Delerium - Silence atmosphere','sub',9,4300,.7,2.3),('Spiral Strings','William Orbit - Adagio for Strings atmosphere','sub',23,6300,.55,2.1),('Crystalline Heaven','Above & Beyond - Good for Me atmosphere','metal',0,6800,1.2,2.7),('Fifth Heaven','System F - Out of the Blue pad role','fifth',0,3900,.6,2.0),('Aurora Silk','Paul van Dyk - For an Angel pad role','pulse',0,3500,.9,2.4),('Dream Vapor','Robert Miles - Children atmosphere','tri',4,4600,1.1,2.9)]),('Dark Pads',[
 ('Obsidian Choir','Cocooma - Rage atmosphere','sub',10,1500,.6,2.1),('Midnight Formant','Kai Tracid - Your Own Reality atmosphere','vowel',0,1100,.8,2.4),('Ghost Chamber','Cosmic Gate - Melt to the Ocean atmosphere','sub',24,2200,.5,2.0),('Reverse Nebula','Push - Universal Nation atmosphere','fm',4,1800,1.2,2.8),('Shadow Bow','Legend B - Lost in Love atmosphere','sub',23,2600,.7,2.2),('Iron Cathedral','Cocooma - District of Power atmosphere','metal',0,1900,.4,1.7),('Submerged Halo','Mauro Picotto - Komodo atmosphere','reed',0,1400,1.0,2.6),('Dusk Circuit','Humate - Love Stimulation atmosphere','pulse',0,1250,.6,1.9),('Eclipse Air','Jam & Spoon - Stella atmosphere','round',4,2300,1.3,2.8),('Nocturne Machine','Sasha - Xpander atmosphere','sync',0,1700,.8,2.3)])]:
 for name,ref,kind,source,cut,a,r in rows:
  dark=category=='Dark Pads';p=dict(source=source,osc1Unison=5,osc2Unison=3,osc1Detune=.15,osc2Detune=.09,osc1Width=.92,osc2Width=.96,osc2Octave=1 if not dark else -1,mix=.22,cutoff=cut,res=.12 if not dark else .24,filterAttack=.8,filterDecay=2.2,filterSustain=.65,filterRelease=.8,filterEnvAmount=.5,eq0Frequency=160 if not dark else 110,eq6Frequency=7900 if not dark else 4300,sampleLoop=int(source in (4,7,9,10,23,24)),gain=.27);p.update(env(a,1.2,1,r));p.update(space(.13,.34,.22))
  if source==4:p.update(grainPosition=.45,grainSize=120,grainDensity=28,grainSpray=.35,grainSpread=.9)
  if name in ('Ghost Chamber','Iron Cathedral'):p.update(convolverEnabled=1,irPreset=4,convolverMix=.13,irDecay=3.0,reverb=.18)
  if name=='Reverse Nebula':p.update(sampleReverse=1)
  if kind in ('metal','fm'):p.update(osc1WarpAmount=.025,osc2Enabled=1)
  add(name,category,ref,p,kind,lfo={'target':'cutoff','depth':.028 if dark else .018,'bars':3},grain_resource='Choir_claudia_oh_wav' if source==4 else '',play='Hold open chords. Slow timbre motion never modulates master volume.',sources=['R_JP','R_STRINGS','R_VOICE'])

strings=[
 ('Adagio Chamber','William Orbit - Adagio for Strings','sub',23,.12,.85,4400),('Festival Arco','Tiesto - Adagio for Strings','sub',23,.045,.65,7900),('Velvet Celli','Robert Miles - Children string role','sub',24,.10,.8,2900),('Cathedral Arco','Gouryella - Gouryella string role','sub',23,.25,2.2,6500),('Staccato Voltage','90s orchestral trance stabs','sub',25,.0007,.11,8500),('Pizzicato Signal','Faithless - Insomnia articulation family','sub',26,.0005,.15,7200),('Trembling Skyline','Classic tremolo trance strings','sub',23,.08,.7,5300),('JP String Anthem','System F - Out of the Blue string layer','saw',0,.18,1.2,4600),('Solina Silver','Jean-Michel Jarre string-machine tradition','pulse',0,.3,1.5,3900),('Noir Ensemble','90s German hard-trance string beds','round',0,.22,1.0,2700)]
for name,ref,kind,source,a,r,cut in strings:
 p=dict(source=source,sampleLoop=int(source in (23,24)),osc1Unison=7,osc2Unison=3,osc2Octave=1,mix=.18,osc1Detune=.11,osc2Detune=.055,cutoff=cut,res=.06,filterEnvAmount=.25,filterAttack=.3,filterDecay=1,filterSustain=.8,ampVelocity=.6,eq0Frequency=source==24 and 55 or 120,gain=.3);p.update(env(a,.8,1 if source not in (25,26) else .15,r));p.update(space(.04,.22,.07 if source else .30))
 if name=='Cathedral Arco':p.update(convolverEnabled=1,irPreset=3,convolverMix=.12,irDecay=3.5)
 extras={}
 if name=='Trembling Skyline':extras['lfo']={'target':'gain','depth':-.065,'hz':4.8}
 if name=='Solina Silver':extras['lfo']={'target':'osc1WarpAmount','depth':.045,'hz':.17};p.update(osc1Warp=3,osc1WarpAmount=.18)
 add(name,'Strings',ref,p,kind,play='Real recorded sections or named analogue ensemble designs; MIDI 48-84.',sources=['R_STRINGS','R_VSCO'],**extras)

for name,ref,source,cut,delay,rev,release in [
 ('Ivory Dreamline','Robert Miles - Children',21,6100,.17,.28,.48),('Rave Hammer Piano','K-Klass - Let Me Show You piano tradition',22,13500,.08,.12,.18),('Sunlit Grand','Eric Prydz - Pjanoo piano tradition',21,11000,.07,.16,.24),('Crystal Dream Keys','Robert Miles - Fable dream-key colour',18,6900,.19,.25,.45),('Detroit Silver Keys','Derrick May - Strings of Life piano tradition',22,9600,.10,.18,.23)]:
 p=dict(source=source,cutoff=cut,res=.035,filterEnvAmount=0,sampleLoop=0,ampVelocity=.65,eq0Frequency=70 if source==21 else 120,eq3Frequency=310,eq3Gain=-2.2,eq4Frequency=2900,eq4Gain=1.6 if source==22 else .5,drive=.01,gain=.40);p.update(env(.0005,8,1,release));p.update(space(delay,rev,0));add(name,'Trance Pianos',ref,p,'sub',play='Velocity-sensitive sampled piano/explicit FM dream-key; play your own chords and melodies.',sources=['R_CHILDREN','R_VCSL'])

arp_rows=[
 ('Azure Ladder','System F - Out of the Blue energy','saw',0,0,2,2),('Polar Cascade','Rank 1 - Airwave shimmer','tri',0,1,2,2),('Silk Pendulum','Chicane - Saltwater shimmer','sub',16,2,1,1),('Glass Triplets','ATB - 9PM crystalline colour','metal',0,0,5,2),('Ivory Orbit','Robert Miles - Children piano atmosphere','sub',21,2,1,2),('Neon Steps','Sasha - Xpander movement','sync',0,0,2,2),('Marimba Constellation','Balearic trance mallet arps','sub',15,0,2,2),('Choir Spiral','Delerium - Silence vocal atmosphere','sub',7,1,2,1),('Prism Random','BT - Flaming June evolving motion','fm',0,3,2,2),('Pizzicato Conductor','Classic pizzicato trance arpeggios','sub',26,0,3,2)]
for name,ref,kind,source,order,rate,octaves in arp_rows:
 p=dict(source=source,patternMode=1,arpOrder=order,patternRate=rate,arpOctaves=octaves,patternGate=.48,cutoff=6300,filterEnvAmount=1.2,filterDecay=.13,filterSustain=0,res=.15,osc1Unison=3,osc2Unison=1,osc1Detune=.08,eq0Frequency=140,gain=.32);p.update(env(.0005,.16,.16,.13));p.update(space(.22,.18,.06));add(name,'Arps and Sequences',ref,p,kind,play='Hold a chord. Host-tempo arpeggiator; no quoted song melody.',sources=['R_JP','R_PATTERN'])
seq_rows=[
 ('Tunnel Transmission','Cocooma energy','pulse',[0,0,12,0,7,0,12,3,0,7,0,10,12,7,3,0]),('Reptile Circuit','Mauro Picotto energy','fm',[0,0,7,12,0,3,7,0,10,7,0,12,3,0,7,10]),('Yellow Sequence','Cocooma - The Yellow Base energy','vowel',[0,3,0,7,0,10,7,3,12,0,3,7,10,7,3,0]),('Lunar Pizzicato','Faithless articulation family','sub',[0,7,12,7,3,10,15,10,0,7,12,15,10,7,3,0]),('Liquid Voltage','Kai Tracid acid tradition','acid',[0,0,12,0,3,0,7,0,0,10,7,3,12,7,0,0]),('Foundry Pulse','Cosmic Gate - Fire Wire energy','octave',[0,12,0,7,0,12,3,7,0,12,10,7,3,0,7,12]),('Emerald Relay','Hard-trance bell sequences','metal',[0,7,3,10,12,7,15,10,0,3,7,12,10,7,3,0]),('Night Train','Scot Project hard-trance tradition','sync',[0,0,7,0,12,7,0,3,0,10,0,7,12,3,7,0]),('Velvet Clockwork','Humate - Love Stimulation atmosphere','organ',[0,7,12,19,3,10,15,22,0,12,7,19,10,3,15,7]),('Starfall Dialogue','Gouryella uplifting tradition','round',[0,12,7,3,10,7,15,12,0,7,12,19,15,10,7,3])]
for number,(name,ref,kind,notes) in enumerate(seq_rows):
 p=dict(patternMode=2,patternRate=2 if number!=6 else 5,sequenceLength=16,patternGate=.48 if number%2==0 else .62,cutoff=3800+number*300,res=.22,filterEnvAmount=2.1,filterDecay=.11,filterSustain=0,filterVelocity=.85,voiceMode=1,osc1Unison=3,osc2Unison=1,osc1Detune=.065,eq0Frequency=95,gain=.32);p.update(env(.0005,.14,.18,.08));p.update(space(.17,.14,.05))
 if kind=='sub':p.update(source=26,voiceMode=0)
 if kind=='acid':p.update(osc1Unison=1,res=.66,filterMode=5,cutoff=390,glideTime=.065)
 for step,note in enumerate(notes):p['stepPitch'+str(step)]=note;p['stepVelocity'+str(step)]=[1,.66,.81,.7,.88,.6,.78,.64][(step+number)%8];p['stepOn'+str(step)]=int(not(step in (5,13) and number in (1,3,7)))
 add(name,'Arps and Sequences',ref,p,kind,play='Hold a root note. Original root-relative 16-step pattern; transpose with MIDI.',sources=['R_PATTERN','R_TB'])

for name,source,cut,decay,release,res in [
 ('Cathedral Strike',5,6300,.32,.22,.12),('Iron Tenor',8,4300,.24,.16,.20),('Basement Ah',10,3200,.29,.19,.22),('Soprano Blade',6,8500,.20,.14,.16),('Ooh Colossus',9,5100,.42,.30,.14),('Monastery Impact',11,3900,.25,.18,.24),('Silver Vowel',7,7200,.30,.24,.13),('Granitic Oh',12,2800,.34,.23,.26),('Rave Congregation',5,9600,.16,.11,.10),('Vocal Circuit',0,4100,.22,.15,.28)]:
 p=dict(source=source,sampleLoop=0,filterMode=4 if source else 3,cutoff=cut if source else 850,res=res,filterEnvAmount=.85 if source else .40,filterDecay=decay*.6,filterSustain=.1,osc1Unison=5,osc2Unison=3,mix=.3,ampVelocity=.45,filterVelocity=.6,eq0Frequency=145,eq3Frequency=730,eq3Gain=1.4,eq4Frequency=2600,eq4Gain=1.8,gain=.34);p.update(env(.0005,decay,.22,release));p.update(space(.07,.16,.18))
 if name in ('Cathedral Strike','Ooh Colossus'):p.update(convolverEnabled=1,irPreset=2,convolverMix=.08,irDecay=1.3,reverb=.08)
 if name=='Rave Congregation':p.update(filterInputDrive=.11,drive=.10,echoMix=.04,reverb=.11)
 add(name,'Choir Stabs','Classic 90s techno / German hard-trance choir-stab role',p,'vowel',play='Play short notes or minor chords. Independent CC0 voices, not lifted record samples.',sources=['R_VOICE','R_COCOOMA'])

for name,wave,cut,res,decay,drive,slide in [
 ('Liquid Chronicle',31,260,.68,.19,.11,.065),('Square Serum',32,350,.73,.23,.08,.075),('Pulse Corrosion',33,470,.64,.14,.16,.055),('Resonant Wire',34,290,.78,.26,.09,.085),('Squelch Reactor',35,390,.70,.18,.19,.07),('Biting Cathode',36,590,.61,.11,.24,.05),('Deep Silver Acid',31,180,.82,.31,.07,.095),('Glass Acid',32,760,.57,.16,.13,.06),('Hardline 303',34,430,.76,.13,.27,.045),('Velvet Slide',33,240,.66,.28,.10,.10)]:
 p=dict(wave=wave,osc2Enabled=0,mix=0,osc1Unison=1,osc1Width=0,osc1PhaseReset=1,osc1Rand=0,voiceMode=2,glideTime=slide,filterMode=5,cutoff=cut,res=res,filterEnvAmount=4.2,filterDecay=decay,filterSustain=0,filterVelocity=1.45,ampVelocity=.42,filterInputDrive=drive,drive=drive,eq0Frequency=26,eq6Frequency=9800,gain=.32);p.update(env(.0003,.19,.70,.028));p.update(space(.07,.035,0));add(name,'Acid', 'Hardfloor - Acperience 1 / Emmanuel Top - Acid Phase acid vocabulary',p,play='Unsequenced mono-legato acid. Overlap notes for slides; velocity accents cutoff and level.',sources=['R_TB'])

assert len(BANK)==160,len(BANK)
assert len(set(p['name'] for p in BANK))==160
# Give every patch useful neutral-at-zero performance assignments.
for patch in BANK:
 p=patch['settings']
 # Oscillator B has a leading Follow choice; blueprints use waveform-family indices.
 p['osc2Wave']+=1
 patch['macros']=[{'source':6,'target':'cutoff','depth':.16},{'source':6,'target':'drive','depth':.10},{'source':7,'target':'osc1Width','depth':.22},{'source':7,'target':'osc2Width','depth':.20},{'source':8,'target':'reverb','depth':.15},{'source':8,'target':'echoMix','depth':.13},{'source':0,'target':'wavetableScan' if p['source']==0 else 'cutoff','depth':.25},{'source':1,'target':'filterEnvAmount','depth':.22},{'source':2,'target':'osc1Detune' if p['source']==0 else 'reverbDamping','depth':.13}]
 if not p['reverbEnabled']:patch['macros']=[m for m in patch['macros'] if m['target']!='reverb']
 if not p['delayEnabled']:patch['macros']=[m for m in patch['macros'] if m['target']!='echoMix']
 patch['design_hash']=hashlib.sha256(json.dumps(p,sort_keys=True).encode()).hexdigest()[:16]
 for key,value in p.items():
  if key not in PARAMS:raise ValueError('Unknown parameter '+key+' in '+patch['name'])
  low,high=PARAMS[key]['minimum'],PARAMS[key]['maximum']
  if key=='source':high=26
  if value<low-1e-5 or value>high+1e-5:raise ValueError(f'{patch["name"]}: {key}={value} outside {low}..{high}')
levels_file=ROOT/'scripts/trance-bank-levels.json'
if levels_file.exists():
 levels={r['id']:r for r in json.loads(levels_file.read_text())}
 for patch in BANK:
  level=levels.get(patch['id'])
  if level and level.get('design_hash')==patch['design_hash']:patch['settings'].update(level['settings'])
result={'generation':1000,'title':'Trance / German Hard Trance 160','init_settings':INIT,'presets':BANK}
path=ROOT/'native-vst3/Presets/TranceFactoryBank.json';path.parent.mkdir(exist_ok=True);path.write_text(json.dumps(result,indent=2)+'\n')
print('Designed',len(BANK),'patches in',len(set(p['category'] for p in BANK)),'categories')
