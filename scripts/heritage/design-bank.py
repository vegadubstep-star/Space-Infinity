"""Heritage200 authored palette and recipe ledger. Drafts require native audition."""
import json,hashlib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2];out=ROOT/'native-vst3/Presets/Heritage200';inventory=json.loads((ROOT/'docs/audit/NATIVE_PARAMETER_INVENTORY.json').read_text());limits={p['id']:p for p in inventory['parameters']}
BASE=dict(source=0,wave=0,osc2Wave=3,osc1Enabled=1,osc2Enabled=0,osc1Gain=0,osc2Gain=0,osc1Unison=1,osc2Unison=1,osc1Width=0,osc2Width=0,osc1Octave=0,osc2Octave=0,osc2Semitone=0,osc1Tune=0,osc2Tune=0,mix=0,masterOctave=0,manualUnison=1,modernUnison=1,independentWTPos=1,wavetableScan=0,osc2WTPos=0,osc1Rand=0,osc2Rand=0,osc1PhaseReset=1,osc2PhaseReset=1,osc1Warp=0,osc2Warp=0,osc1WarpAmount=0,osc2WarpAmount=0,subEnabled=0,subLevel=0,noiseLevel=0,routeA=1,routeB=1,routeSub=0,routeNoise=1,filterMode=1,cutoff=7000,res=.08,filterAttack=.001,filterDecay=.25,filterSustain=0,filterRelease=.10,filterEnvAmount=0,filterVelocity=.6,filterInputDrive=0,attack=.001,ampDecay=.35,ampSustain=.65,release=.12,ampTimed=1,ampLong=0,ampVelocity=.65,voiceMode=0,glideTime=0,pitchEnvAmount=0,analogueUpgrade=1,analogueCharacter=0,analogueLabEnabled=0,cleanVCO=1,stereoFilterUpgrade=1,declickEnabled=1,patternMode=0,gateEnabled=0,pumpEnabled=0,vibratoEnabled=0,lfodepth=0,motionDepth=0,designerLfo1Enabled=0,designerLfo2Enabled=0,designerLfo3Enabled=0,macro0=0,macro1=0,macro2=0,freeMacro0=0,freeMacro1=0,freeMacro2=0,drive=0,distEnabled=0,distHot=0,chorusEnabled=0,flangerEnabled=0,phaserEnabled=0,delayEnabled=0,reverbEnabled=0,convolverEnabled=0,compEnabled=0,eqEnabled=1,eqMode0=4,eq0Frequency=30,eq0Q=.707,eqMode6=3,eq6Frequency=15000,eq6Q=.707,gain=.4,presetTrim=0)
PROFILES={
 'grand':dict(source=21,ampDecay=2.8,ampSustain=0,release=.22,cutoff=11000,filterVelocity=.9),
 'upright':dict(source=22,ampDecay=2.0,ampSustain=0,release=.16,cutoff=8500),
 'ep':dict(source=18,ampDecay=2.5,ampSustain=.08,release=.28,cutoff=9000),
 'clav':dict(source=17,ampDecay=.6,ampSustain=.15,release=.08,cutoff=5500,filterEnvAmount=1),
 'drawbar':dict(wave=30,ampDecay=.16,ampSustain=.88,release=.04,cutoff=7500),
 'organbass':dict(wave=2,osc2Enabled=1,osc2Wave=3,osc2Octave=1,mix=.3,subEnabled=1,subLevel=.28,subOctave=-1,voiceMode=1,ampDecay=.19,ampSustain=.35,release=.035,cutoff=2200),
 'vibe':dict(source=13,ampDecay=1.8,ampSustain=0,release=.35,cutoff=11000),
 'glock':dict(source=14,ampDecay=1.9,ampSustain=0,release=.4,cutoff=10000),
 'marimba':dict(source=15,ampDecay=.8,ampSustain=0,release=.12,cutoff=8500),
 'lute':dict(source=16,ampDecay=1,ampSustain=0,release=.13,cutoff=6500),
 'recorder':dict(source=20,attack=.015,ampDecay=.9,ampSustain=.6,release=.20,cutoff=7500),
 'bow':dict(source=19,attack=.18,ampDecay=1.4,ampSustain=.8,release=.7,cutoff=6500,sampleLoop=1),
 'strings':dict(source=23,attack=.3,ampDecay=1.2,ampSustain=1,release=.8,cutoff=8500,sampleLoop=1),
 'celli':dict(source=24,attack=.15,ampDecay=1.2,ampSustain=1,release=.65,cutoff=5500,sampleLoop=1),
 'pizz':dict(source=26,ampDecay=.55,ampSustain=0,release=.12,cutoff=10000),
 'choir':dict(source=27,attack=.22,ampDecay=1.1,ampSustain=.8,release=.65,cutoff=8000,sampleLoop=1),
 'ooh':dict(source=28,attack=.30,ampDecay=1.4,ampSustain=.8,release=.9,cutoff=6000,sampleLoop=1),
 'digital':dict(wave=30,attack=.08,ampDecay=1.2,ampSustain=.65,release=.7,cutoff=8500),
 'bell':dict(wave=30,ampDecay=1.6,ampSustain=0,release=.38,cutoff=13000),
 'super':dict(osc1Unison=7,osc2Enabled=1,osc2Wave=1,osc2Unison=3,osc1Detune=.12,osc2Detune=.06,osc1Blend=.65,osc2Blend=.6,osc1Width=.9,osc2Width=.8,osc1Rand=1,osc2Rand=1,osc1PhaseReset=0,osc2PhaseReset=0,mix=.22,cutoff=5800,filterEnvAmount=1,ampDecay=.5,ampSustain=.72,release=.22),
 'sawpad':dict(osc1Unison=7,osc2Enabled=1,osc2Wave=1,osc2Unison=3,osc1Detune=.09,osc2Detune=.055,osc1Blend=.7,osc2Blend=.6,osc1Width=1,osc2Width=1,osc1Rand=1,osc2Rand=1,osc1PhaseReset=0,osc2PhaseReset=0,mix=.18,osc2Octave=1,cutoff=3200,attack=.55,ampDecay=1.5,ampSustain=1,release=.9,filterAttack=.8,filterDecay=1.5,filterSustain=.8,filterEnvAmount=1),
 'pulse':dict(wave=1,osc1Warp=3,osc1WarpAmount=.28,osc2Enabled=1,osc2Wave=3,mix=.16,cutoff=4200,filterEnvAmount=1.4,res=.22),
 'mono':dict(voiceMode=2,glideTime=.04,osc2Enabled=1,osc2Wave=2,mix=.2,cutoff=3600,filterEnvAmount=1.8,res=.22,release=.09),
 'fm':dict(wave=2,osc2Enabled=1,osc2Wave=3,osc2Octave=1,mix=.025,osc1Warp=1,osc1WarpAmount=.13,osc1WarpEnvelope=2,cutoff=7000,filterEnvAmount=.8,ampDecay=.4,ampSustain=.5,release=.12),
 'fmkeys':dict(wave=2,osc2Enabled=1,osc2Wave=3,osc2Octave=2,mix=.02,osc1Warp=1,osc1WarpAmount=.18,osc1WarpEnvelope=2,cutoff=10000,filterDecay=.4,ampDecay=1.3,ampSustain=.08,release=.25),
 'bass':dict(voiceMode=1,osc2Enabled=1,osc2Wave=3,mix=.15,subEnabled=1,subLevel=.25,subOctave=0,cutoff=260,filterEnvAmount=3,filterDecay=.13,res=.2,ampDecay=.22,ampSustain=.35,release=.025,filterInputDrive=.06,eq0Frequency=25),
 'fmbass':dict(wave=2,voiceMode=1,osc2Enabled=1,osc2Wave=3,osc2Octave=1,mix=.03,osc1Warp=1,osc1WarpAmount=.14,osc1WarpEnvelope=2,subEnabled=1,subLevel=.25,subOctave=0,cutoff=1700,filterDecay=.10,filterEnvAmount=.8,ampDecay=.21,ampSustain=.35,release=.022),
 'pluck':dict(osc1Unison=3,osc1Detune=.045,osc1Width=.6,cutoff=450,filterEnvAmount=3.8,filterDecay=.22,res=.15,ampDecay=.4,ampSustain=.10,release=.17),
 'respad':dict(wave=1,osc1Warp=3,osc1WarpAmount=.2,osc2Enabled=1,osc2Wave=1,mix=.3,osc2Tune=4,cutoff=1800,res=.42,attack=.30,ampDecay=1.2,ampSustain=.9,release=.85,filterAttack=.4,filterEnvAmount=1.5),
 'sync':dict(wave=30,voiceMode=2,glideTime=.035,cutoff=5500,filterEnvAmount=1.2,ampDecay=.45,ampSustain=.6,release=.12),
 'ring':dict(wave=30,cutoff=8000,ampDecay=.7,ampSustain=.28,release=.25),
 'drum':dict(wave=2,voiceMode=1,pitchEnvAmount=12,pitchEnvDecay=.035,ampDecay=.20,ampSustain=0,release=.025,cutoff=7000),
}
# Names identify intended musical identities. Table/source, contour and effect
# choices are explicit in each row: name / architecture / cutoff / decay / scan.
GROUPS=[
('M1','Piano', 'Atrium Grand:grand:10500:2.8:0|Ivory House:grand:7800:1.4:0|Velvet Eight:upright:6300:2.4:0|Crystal Hammer:ep:11000:1.9:0|Midnight Studio:grand:4800:3.1:0|Sunrise Keys:grand:13500:2.2:0|Amber Upright:upright:7500:1.6:0|Dream Corridor:grand:8800:3.8:0|Paper Piano:upright:4200:1.1:0|Starlit Ivory:ep:14500:3.2:0'),
('M1','Electric Keys','Tine Nocturne:ep:9000:2.6:0|Reed Lantern:clav:4500:.9:0|Silver Dyno:fmkeys:11500:1.9:.1|Glass Suitcase:ep:14000:1.5:0|Soft Circuit:fmkeys:5800:2.3:.06|Clav Mosaic:clav:7800:.5:0|Electric Velvet:ep:4800:3.4:0|Neon Tines:fmkeys:13000:1.2:.2'),
('M1','Organ','Cathedral Keys:drawbar:6200:.14:.2|Garage Drawbars:organbass:3400:.23:0|Sunday Circuit:drawbar:4200:.2:.05|Bright Pipes:drawbar:13000:.3:.9|Chapel Bass:organbass:1200:.19:0|Gospel Relay:drawbar:8500:.18:.55|Percussive Wheel:drawbar:5000:.12:.75|Velvet Chapel:drawbar:2800:.24:.1|Digital Nave:drawbar:11000:.4:.65|Rave Congregation Keys:organbass:5200:.16:0'),
('M1','Mallets','Bamboo Steps:marimba:7500:.7:0|Pan Lantern:recorder:9200:.65:0|Glass Rain:glock:14000:1.7:0|Silverwood:vibe:10500:2.4:0|Copper Kalimba:lute:6800:.8:0|Wind Chimes:bell:15000:2.2:.65|Ceramic Pluck:marimba:4400:.45:0|Music Box Harbor:glock:8000:1.2:0|Wire Garden:bell:12500:1.1:.3|Violet Vibes:vibe:5600:3:0'),
('M1','Bass','Organ Avenue:organbass:2100:.18:0|Digital Pick:bass:1200:.15:0|Warm Finger:bass:240:.28:0|Fretless Lantern:fmbass:950:.38:.05|House Foundation:organbass:4200:.12:0|Rubber Station:bass:420:.21:0|Night Platform:fmbass:1800:.23:.12|Soft Thumb:bass:180:.32:0'),
('M1','Pads','Cosmic Lobby:digital:8000:1.8:.45|Lore Horizon:digital:5500:2.2:.2|Dream Atlas:ooh:7500:1.5:0|Magic Terrace:digital:10500:1.7:.75|Amber Atmosphere:bow:4600:2.3:0|Polar Atrium:choir:9800:1.8:0|Wind Pavilion:digital:6800:2.5:.65|Glass Canopy:digital:13000:1.3:.3|Night Exhibition:ooh:4200:2.8:0|Crystal Weather:digital:11000:2.1:.85'),
('M1','Strings','Symphonic Avenue:strings:10000:1.4:0|Cello Gallery:celli:5000:1.6:0|Pizzicato Steps:pizz:12500:.65:0|Chamber Cinema:strings:6800:1.8:0|Bowed Lantern:bow:7200:2:0|Velvet Orchestra:celli:3500:2.3:0|Digital Quartet:strings:13500:1.1:0|Staccato Hall:pizz:6800:.36:0'),
('M1','Voices','Ooh Observatory:ooh:6200:1.8:0|Ah Glasshouse:choir:10500:1.2:0|Vocal Crystal:digital:8200:1.4:.4|Choir Lantern:choir:4800:2:0|Air Chapel:ooh:11000:1.6:0|Digital Syllable:digital:6500:.3:.75|Soft Congregation:choir:3600:1.7:0|Voicewave Terrace:digital:12500:1.5:.2'),
('M1','Textures','Orbit Chamber:digital:10500:2.6:.8|Pole Reflection:bell:11800:1.8:.1|Wire Twilight:digital:7200:.8:.65|Bottled Light:recorder:5400:1.4:0|Metal Rainfall:bell:14500:2.4:.9|Spectrum Pavilion:digital:8500:1.9:.5|Bowed Planet:bow:5800:2.5:0|Tropical Circuit:marimba:13000:.9:0'),
('JP','Leads','Skyline Seven:super:6500:.5:0|Anthem Beacon:super:10500:.38:0|Pulse Runway:pulse:7200:.35:0|Feedback Crown:sync:8500:.7:.65|Triangle Voltage:sync:6200:.55:.2|Fifth Horizon:super:7800:.6:0|Velvet Mono:mono:2800:.4:0|Silver Ribbon:pulse:10000:.3:0|Glass Supersaw:super:13000:.25:0|Copper Feedback:sync:4400:.85:.9|Narrow Laser:pulse:5200:.21:0|Octave Summit:super:9000:.48:0|Warm Airfoil:mono:4100:.65:0|Resonant Launch:mono:1700:.3:0|Prism Ensemble:super:11500:.72:0|Dawn Driver:sync:7200:.36:.45'),
('JP','Pads','Cloud Armada:sawpad:4500:1.5:0|Amber Supersky:sawpad:2500:2:0|Triangle Aurora:respad:5200:1.8:0|Feedback Nebula:digital:7800:2.2:.5|Pulse Constellation:respad:3200:1.5:0|Silver Canopy:sawpad:8500:1.8:0|Quiet Horizon:sawpad:1700:2.4:0|Fifth Atmosphere:sawpad:6200:1.3:0|Velvet Polysky:respad:4500:2.2:0|Northern Assembly:sawpad:7200:1.6:0|Dusk Feedback:digital:3800:2.6:.85|Airwave Terrace:sawpad:10500:1.9:0'),
('JP','Plucks','Crystal Flight:pluck:520:.28:0|Pulse Raindrop:pulse:850:.20:0|Anthem Fragment:pluck:780:.38:0|Glass Shortwave:fmkeys:10000:.6:.08|Seven Sparks:pluck:420:.18:0|Copper String:pluck:1200:.45:0|Feedback Droplet:sync:2100:.3:.25|Digital Plectrum:pluck:650:.55:0|Neon Harp:pluck:1800:.24:0|Velvet Sequence Key:pulse:480:.33:0|Fifth Flicker:pluck:950:.42:0|Silver Icicle:fmkeys:13000:.35:.16'),
('JP','Bass','Seven Engine:bass:420:.23:0|Pulse Thrust:bass:280:.18:0|Feedback Torque:sync:1500:.20:.5|Rubber Airfoil:bass:620:.26:0|Triangle Anchor:fmbass:1200:.28:.07|Octave Motor:bass:900:.16:0|Sub Terminal:organbass:1100:.24:0|Resonant Landing:bass:200:.29:0|Warm Throttle:bass:340:.34:0|Metal Wing:fmbass:2200:.18:.18'),
('JP','Arps','Skyline Ladder:pluck:720:.24:0|Pulse Navigation:pulse:1400:.20:0|Feedback Orbit:sync:4500:.28:.6|Seven Satellites:pluck:1100:.35:0|Triangle Compass:fmkeys:8500:.5:.1|Fifth Departure:pluck:480:.4:0|Velvet Motion:pulse:2500:.26:0|Crystal Runway:pluck:1800:.18:0|Neon Traverse:super:5500:.22:0|Dawn Circuit:pluck:360:.48:0'),
('Nord','Leads','Scarlet Saw:mono:5200:.28:0|Pulse Ice:pulse:6800:.24:0|Metal Fjord:fm:9800:.42:.18|Sync Needle:sync:8500:.35:.6|Ring Halo:ring:11500:.5:.45|Velvet Brass:pulse:3400:.45:0|Nordic Fifth:mono:7200:.4:0|Hollow Signal:fm:4800:.6:.08|Bright Compass:mono:13500:.18:0|Vocal Relay:sync:6000:.38:.8|Steel Ribbon:fm:11000:.32:.25|Resonant Ember:mono:1300:.55:0'),
('Nord','Bass','Red Attack:bass:250:.18:0|Rubber Glacier:bass:430:.25:0|FM Sledge:fmbass:2100:.17:.20|Pulse Quarry:bass:340:.20:0|Sync Excavator:sync:1700:.18:.7|Ring Undertow:ring:2900:.23:.25|Sub Fjord:organbass:900:.28:0|Acid Ember:bass:180:.30:0|Fifth Boulder:bass:820:.19:0|Wooden Circuit:fmbass:1150:.35:.07|Drive Tundra:bass:520:.16:0|Deep Aurora:bass:140:.4:0'),
('Nord','Keys','FM Compass:fmkeys:10000:1.2:.15|Ring Clav:ring:7800:.38:.3|Pulse Brass Key:pulse:4200:.45:0|Steel Petal:fmkeys:13000:.85:.28|Hollow Mallet:fmkeys:6200:1.8:.07|Sync Harp:sync:4800:.55:.4|Violet Organ:drawbar:7500:.18:.6|Digital Nord Bell:bell:14500:1.6:.5|Wooden Tine:fmkeys:4800:2.1:.1|Glass Hammer:ring:11500:.7:.75'),
('Nord','Pads','Resonance Fjord:respad:1700:1.8:0|Pulse Snowfield:respad:3800:2.2:0|FM Twilight:digital:8200:2:.35|Sync Aurora:digital:6500:1.7:.65|Ring Mist:digital:10500:2.8:.45|Fifth Glacier:respad:5200:1.4:0|Scarlet Strings:respad:7500:1.9:0|Hollow Horizon:digital:4200:2.6:.1|Velvet Ice:respad:2500:2.4:0|Metal Atmosphere:digital:13000:1.6:.8'),
('Nord','Motion','Scarlet Staircase:pluck:640:.22:0|FM Clockwork:fmkeys:8500:.32:.2|Sync Transit:sync:5200:.28:.55|Pulse Steps:pulse:1800:.20:0|Ring Current:ring:10500:.4:.6|Fifth Engine:mono:3200:.25:0|Resonance Route:pluck:280:.48:0|Hollow Arpeggio:fmkeys:5500:.65:.08'),
('Nord','Percussion','Low Voltage Drum:drum:1800:.22:0|Copper Tom:drum:7500:.32:0|FM Woodblock:fmkeys:8500:.10:.32|Ring Metal Hit:ring:13000:.14:.85|Pulse Rim:pulse:4500:.07:0|Noise Hat:drum:14500:.055:0|Sync Zap:sync:11000:.18:.9|Glass Clonk:fmkeys:15000:.25:.45'),
]
rows=[]
for family,category,data in GROUPS:
 for i,text in enumerate(data.split('|')):
  name,profile,cut,decay,scan=text.split(':');p=dict(BASE,**{});p.update(PROFILES[profile]);p.update(cutoff=float(cut),ampDecay=float(decay),wavetableScan=float(scan));table=None
  if p['wave']==30:
   table='DigitalDrawbar' if profile=='drawbar' else 'DigitalGlass' if profile=='bell' else 'NorthernRing' if profile=='ring' else 'NorthernSync' if profile=='sync' else 'DigitalVoice' if category=='Voices' else 'DigitalAir'
   if family=='JP' and profile in ('sync','digital'):table='EnsembleTriangle' if 'Triangle' in name else 'EnsembleFeedback'
   if family=='Nord' and profile=='digital':table='NorthernFM' if 'FM' in name else 'NorthernRing' if 'Ring' in name else 'NorthernHollow' if 'Hollow' in name else 'NorthernSync'
  if family=='M1':p.update(engineMode=1,res=0,filterMode=1)
  else:p.update(engineMode=0,analogueCharacter=.035,filterMode=4 if profile in ('bass','sawpad') else 1)
  if category in ('Pads','Strings','Voices','Textures'):
   p.update(reverbEnabled=1,reverb=.24,reverbSize=.78,reverbDamping=.38,reverbWidth=1,chorusEnabled=1,chorus=.10,chorusRate=.12,chorusDepth=.25)
   if profile in ('digital','respad'):p.update(attack=.45,ampSustain=.9,release=.85)
  if category in ('Leads','Plucks','Arps','Motion'):
   p.update(delayEnabled=1,echoMode=2,echoMix=.14,echoSyncL=4,echoSyncR=5,echoLink=0,delayFeedback=.28,reverbEnabled=1,reverb=.12,reverbSize=.65)
  if 'Bass' in category:p.update(voiceMode=1,release=.025,osc1Width=0,osc2Width=0,ampSustain=.32,attack=.0015,eq0Frequency=25)
  if category in ('Arps','Motion'):p.update(patternMode=1,patternRate=2,arpOctaves=2)
  if category=='Percussion':p.update(ampSustain=0,release=.025,voiceMode=1)
  if 'Fifth' in name:p.update(osc2Enabled=1,osc2Wave=1,osc2Semitone=7,mix=.16)
  if 'Octave' in name:p.update(osc2Enabled=1,osc2Wave=1,osc2Octave=1,mix=.24)
  if 'Pulse' in name and p['source']==0 and not table:p.update(wave=1,osc1Warp=3,osc1WarpAmount=.38)
  if name=='Noise Hat':p.update(osc1Enabled=0,noiseLevel=.7,noiseType=2,filterMode=2,cutoff=6500,pitchEnvAmount=0)
  if name=='Copper Tom':p.update(masterOctave=-1,pitchEnvAmount=7,pitchEnvDecay=.06)
  if profile in ('grand','upright','ep','clav','vibe','glock','marimba','lute','recorder','pizz'):p.update(reverbEnabled=1,reverb=.11,reverbSize=.5)
  # Deliberately authored performance and tone differences within each family.
  # These rows supplement source/contour choices; no random recipe generation.
  accents={
   'Atrium Grand':dict(eq3Frequency=350,eq3Gain=-2,reverb=.16),
   'Ivory House':dict(ampDecay=.95,ampVelocity=.25,eq4Frequency=2400,eq4Gain=2.8,distEnabled=1,distInput=3,distDrive=.12,distMix=.18),
   'Velvet Eight':dict(eq6Frequency=5400,ampVelocity=.8,reverb=.18),
   'Crystal Hammer':dict(eq4Frequency=4500,eq4Gain=2.5,chorusEnabled=1,chorus=.08),
   'Midnight Studio':dict(ampVelocity=.95,eq3Frequency=220,eq3Gain=1.5,reverb=.08),
   'Sunrise Keys':dict(eq4Frequency=3200,eq4Gain=2,ampVelocity=.45,release=.32),
   'Amber Upright':dict(filterEnvAmount=.35,filterDecay=.25,ampVelocity=.6),
   'Dream Corridor':dict(delayEnabled=1,echoMix=.13,delayFeedback=.28,reverb=.24,release=.55),
   'Paper Piano':dict(eqMode0=4,eq0Frequency=280,ampVelocity=.35,release=.065),
   'Starlit Ivory':dict(reverb=.22,chorusEnabled=1,chorus=.12,eq4Frequency=6500,eq4Gain=3),
   'Reed Lantern':dict(osc1Octave=-1,filterEnvAmount=1.6,res=0,release=.04),
   'Silver Dyno':dict(osc2Semitone=7,osc1WarpAmount=.09,chorusEnabled=1,chorus=.14),
   'Glass Suitcase':dict(eq0Frequency=140,eq3Frequency=850,eq3Gain=-2.5,chorusEnabled=1,chorus=.22),
   'Soft Circuit':dict(osc2Octave=1,osc1WarpAmount=.07,ampVelocity=.85),
   'Clav Mosaic':dict(distEnabled=1,distInput=0,distDrive=.18,distMix=.3,filterEnvAmount=2.2,release=.03),
   'Electric Velvet':dict(eq6Frequency=4800,chorusEnabled=1,chorus=.18,reverb=.18),
   'Neon Tines':dict(osc2Octave=3,osc1WarpAmount=.075,delayEnabled=1,echoMix=.1),
   'Garage Drawbars':dict(wave=30,osc2Enabled=0,mix=0,subLevel=.18,filterEnvAmount=.8),
   'Percussive Wheel':dict(ampSustain=.55,filterEnvAmount=1.5,filterDecay=.065),
   'Bright Pipes':dict(osc2Enabled=1,osc2Wave=3,osc2Octave=2,mix=.18),
   'Rave Congregation Keys':dict(distEnabled=1,distDrive=.15,distInput=3,distMix=.2,ampSustain=.28),
   'Pan Lantern':dict(ampSustain=0,attack=.008,release=.08,eq0Frequency=180),
   'Wind Chimes':dict(osc1Octave=1,eq0Frequency=500,delayEnabled=1,echoMix=.16),
   'Copper Kalimba':dict(filterEnvAmount=1.5,filterDecay=.07,eq3Frequency=750,eq3Gain=2),
   'Wire Garden':dict(osc1Octave=-1,osc2Enabled=1,osc2Wave=3,osc2Octave=1,mix=.16),
   'Digital Pick':dict(wave=1,pitchEnvAmount=1.8,pitchEnvAttack=0,pitchEnvDecay=.018,filterDecay=.09),
   'Warm Finger':dict(wave=3,osc2Wave=3,filterEnvAmount=1.4,res=0),
   'Fretless Lantern':dict(glideTime=.07,voiceMode=2,osc2Octave=0,osc1WarpAmount=.06,ampSustain=.52),
   'Rubber Station':dict(wave=1,osc1Warp=3,osc1WarpAmount=.38,filterEnvAmount=3.4,filterDecay=.11),
   'Night Platform':dict(osc2Octave=2,osc1WarpAmount=.22,ampSustain=.2),
   'Soft Thumb':dict(wave=3,osc2Enabled=0,subLevel=.16,filterEnvAmount=1.1,ampSustain=.5),
   'Cosmic Lobby':dict(osc2Enabled=1,osc2Wave=3,osc2Octave=2,mix=.10),
   'Lore Horizon':dict(filterEnvAmount=1,filterAttack=1.0,filterDecay=2.5,filterSustain=.2),
   'Magic Terrace':dict(osc1Octave=1,eq0Frequency=240,chorus=.18),
   'Wind Pavilion':dict(noiseLevel=.018,filterAttack=1.0,filterEnvAmount=.7),
   'Glass Canopy':dict(ampDecay=3,ampSustain=.4,attack=.06,reverb=.32),
   'Crystal Weather':dict(osc2Enabled=1,osc2Wave=3,osc2Octave=1,mix=.08),
   'Digital Syllable':dict(attack=.004,ampSustain=0,release=.18,reverb=.13),
   'Pole Reflection':dict(osc1Octave=1,delayEnabled=1,echoMix=.18),
   'Wire Twilight':dict(attack=.005,ampSustain=.12,release=.26),
   'Metal Rainfall':dict(osc1Octave=-1,ampVelocity=.9,reverb=.33),
   'Feedback Crown':dict(osc1Unison=3,osc1Detune=.035,osc1Width=.5,distEnabled=1,distInput=0,distDrive=.22,distMix=.2),
   'Triangle Voltage':dict(osc2Enabled=1,osc2Wave=3,osc2Octave=-1,mix=.2),
   'Glass Supersaw':dict(osc2Octave=1,mix=.12,eq0Frequency=220),
   'Copper Feedback':dict(filterMode=4,res=.32,filterInputDrive=.12),
   'Narrow Laser':dict(voiceMode=2,glideTime=.025,osc1WarpAmount=.72,osc2Enabled=0),
   'Resonant Launch':dict(res=.5,filterEnvAmount=2.6,filterDecay=.2),
   'Prism Ensemble':dict(osc1Detune=.18,osc2Octave=1,ampSustain=.85),
   'Dawn Driver':dict(osc2Enabled=1,osc2Wave=2,mix=.2,filterEnvAmount=2),
   'Triangle Aurora':dict(wave=30,osc2Wave=4,mix=.25,res=.12),
   'Pulse Constellation':dict(osc1WarpAmount=.65,osc2Wave=2,mix=.2),
   'Airwave Terrace':dict(attack=.85,release=.95,filterEnvAmount=.5,reverb=.32),
   'Seven Sparks':dict(osc1Unison=7,osc1Detune=.10,osc1Blend=.7,ampSustain=0),
   'Copper String':dict(wave=1,osc1Unison=1,res=.32,filterEnvAmount=2.8,ampSustain=0),
   'Neon Harp':dict(osc2Enabled=1,osc2Wave=3,osc2Octave=1,mix=.12,ampSustain=0),
   'Pulse Thrust':dict(wave=1,osc1Warp=3,osc1WarpAmount=.35,filterDecay=.085),
   'Seven Engine':dict(osc1Unison=7,osc1Detune=.035,osc1Blend=.75,osc1Width=.35),
   'Feedback Torque':dict(subEnabled=1,subLevel=.3,subOctave=0,ampSustain=.35,filterEnvAmount=1.5),
   'Rubber Airfoil':dict(wave=1,osc1Warp=3,osc1WarpAmount=.62,filterEnvAmount=2.8,res=.3),
   'Resonant Landing':dict(filterMode=6,res=.62,filterEnvAmount=3,distEnabled=1,distInput=3,distDrive=.3,distMix=.24),
   'Metal Wing':dict(osc2Octave=2,osc1WarpAmount=.24,distEnabled=1,distInput=0,distDrive=.2,distMix=.18),
   'Metal Fjord':dict(osc2Octave=2,osc1WarpAmount=.20,filterEnvAmount=1.2),
   'Sync Needle':dict(filterEnvAmount=2.6,filterDecay=.15,res=.25),
   'Velvet Brass':dict(attack=.018,filterAttack=.05,filterEnvAmount=2.2,osc1WarpAmount=.15),
   'Hollow Signal':dict(osc2Octave=0,osc1WarpAmount=.065,ampSustain=.78),
   'Bright Compass':dict(osc2Wave=1,osc2Tune=3,mix=.32,filterEnvAmount=.4),
   'Vocal Relay':dict(res=.3,filterEnvAmount=1.8,filterDecay=.35),
   'Steel Ribbon':dict(osc2Semitone=7,osc1WarpAmount=.19),
   'Rubber Glacier':dict(wave=1,osc1Warp=3,osc1WarpAmount=.46,filterDecay=.14),
   'FM Sledge':dict(osc2Octave=2,osc1WarpAmount=.27,distEnabled=1,distInput=0,distDrive=.3,distMix=.2),
   'Pulse Quarry':dict(wave=1,osc1Warp=3,osc1WarpAmount=.16,filterEnvAmount=2.8),
   'Ring Undertow':dict(subEnabled=1,subLevel=.25,subOctave=0,filterEnvAmount=.7,ampSustain=.35),
   'Acid Ember':dict(wave=31,filterMode=6,res=.73,filterEnvAmount=4.2,filterDecay=.16,distEnabled=1,distInput=9,distType=14,distDrive=.4,distMix=.5),
   'Drive Tundra':dict(wave=32,filterInputDrive=.18,distEnabled=1,distInput=6,distDrive=.35,distMix=.4),
   'Deep Aurora':dict(wave=3,osc2Enabled=0,filterEnvAmount=1.2,ampSustain=.6,subLevel=.18),
   'FM Compass':dict(osc2Octave=1,osc1WarpAmount=.11),
   'Steel Petal':dict(osc2Octave=3,osc1WarpAmount=.2),
   'Hollow Mallet':dict(osc2Octave=0,osc1WarpAmount=.095),
   'Wooden Tine':dict(osc2Semitone=7,osc1WarpAmount=.08),
   'Scarlet Strings':dict(osc1Unison=3,osc1Detune=.03,osc1Width=.65,osc2Wave=1),
   'Pulse Snowfield':dict(osc1WarpAmount=.6,osc2Octave=1,mix=.18),
   'Low Voltage Drum':dict(osc1Octave=-2,pitchEnvAttack=0,pitchEnvAmount=19,pitchEnvDecay=.045),
   'Copper Tom':dict(osc1Octave=-1,pitchEnvAttack=0),
   'FM Woodblock':dict(osc2Octave=3,osc1WarpAmount=.3,filterEnvAmount=0),
   'Ring Metal Hit':dict(eq0Frequency=800,osc1Octave=1),
   'Pulse Rim':dict(osc1Octave=1,eq0Frequency=950,filterEnvAmount=0),
   'Sync Zap':dict(pitchEnvAmount=-12,pitchEnvAttack=0,pitchEnvDecay=.14),
   'Glass Clonk':dict(osc2Semitone=7,osc1WarpAmount=.25),
  }
  p.update(accents.get(name,{}))
  # Listening-review candidates found by the first spectral/contour audit.
  # Change architecture/register, not just labels or master gain.
  character_review={
   'Digital Quartet':dict(source=25,sampleLoop=0,attack=.015,ampDecay=.7,ampSustain=.25,release=.16,chorus=.04,reverb=.16),
   'Chamber Cinema':dict(source=24,attack=.08,ampDecay=.9,ampSustain=.72,release=.45,eq3Frequency=800,eq3Gain=2),
   'Air Chapel':dict(source=29,attack=.38,filterEnvAmount=.8,filterAttack=.6),
   'Choir Lantern':dict(source=5,attack=.12,ampSustain=.72,release=.5),
   'Soft Congregation':dict(source=10,attack=.3,filterEnvAmount=.8,cutoff=2100,release=.9),
   'Night Exhibition':dict(source=9,attack=.6,release=.95,chorus=.04),
   'Northern Assembly':dict(wave=6,osc2Wave=5,osc2Octave=0,mix=.35,osc1Detune=.045,attack=.2,release=.6,filterMode=1,res=.1,chorus=.18),
   'Silver Canopy':dict(osc2Octave=1,osc2Semitone=7,mix=.38,osc1Blend=.52,chorus=.06),
   'Airwave Terrace':dict(wave=6,osc2Wave=2,osc2Octave=1,mix=.28,osc1Detune=.17,osc1Blend=.5,chorus=.24,reverb=.38,ampLong=1,attack=.65,release=.7),
   'Fifth Atmosphere':dict(mix=.48,osc2Unison=1,osc1Unison=5,osc1Detune=.065),
   'Quiet Horizon':dict(wave=1,osc1Warp=3,osc1WarpAmount=.72,osc2Wave=4,osc2Octave=0,mix=.38,cutoff=650,res=.38,filterEnvAmount=.8),
   'Sub Terminal':dict(wave=3,osc2Wave=5,osc2Octave=0,mix=.12,masterOctave=-1,filterEnvAmount=.8,filterDecay=.07),
   'Sub Fjord':dict(wave=2,osc2Wave=3,osc2Octave=2,mix=.07,subLevel=.15,ampSustain=.6),
   'Wooden Circuit':dict(osc2Wave=4,osc2Octave=0,osc1WarpAmount=.23,ampSustain=.2),
   'Pulse Ice':dict(osc1WarpAmount=.75,osc2Wave=4,osc2Octave=-1,mix=.3,release=.065),
   'Scarlet Saw':dict(osc1Warp=1,osc1WarpAmount=.085,osc2Wave=3,osc2Octave=-1,mix=.025),
   'Anthem Beacon':dict(osc2Octave=1,mix=.42,osc1Detune=.16,osc2Unison=5,eq0Frequency=160),
   'FM Sledge':dict(wave=3,osc2Wave=4,osc2Semitone=7,osc1WarpAmount=.32),
   'Rave Congregation Keys':dict(wave=17,osc2Wave=5,mix=.12,subLevel=.2,filterEnvAmount=.7),
   'House Foundation':dict(wave=18,osc2Wave=3,mix=.08,ampSustain=.2,filterDecay=.06),
   'Velvet Chapel':dict(wavetableScan=.18,osc2Enabled=1,osc2Wave=3,osc2Octave=1,mix=.12),
   'Digital Nave':dict(wavetableScan=.95,osc2Enabled=1,osc2Wave=3,osc2Octave=2,mix=.38),
   'Violet Vibes':dict(source=0,wave=2,osc2Enabled=1,osc2Wave=3,osc2Octave=2,mix=.02,osc1Warp=1,osc1WarpAmount=.055,osc1WarpEnvelope=2,ampDecay=2.4,ampSustain=0,reverb=.2),
   'Amber Upright':dict(masterOctave=1,eq0Frequency=200,ampVelocity=.5,ampDecay=1.1,release=.08),
   'Seven Sparks':dict(wave=5,osc1Detune=.17,osc1Warp=3,osc1WarpAmount=.4,filterEnvAmount=2.6),
   'Crystal Runway':dict(wave=30,wavetableScan=.45,osc1Unison=2,osc1Detune=.02),
   'Anthem Fragment':dict(wave=27,osc1Unison=3,osc1Detune=.06,filterEnvAmount=2.6),
   'Warm Throttle':dict(wave=6,osc2Wave=4,osc2Octave=-1,mix=.3,ampSustain=.5),
   'Deep Aurora':dict(wave=2,subLevel=.08,filterEnvAmount=.7),
   'Velvet Ice':dict(wave=30,wavetableScan=.08,res=.08,osc2Wave=3,osc2Octave=-1,mix=.2),
  }
  p.update(character_review.get(name,{}))
  if name in ("Acid Ember","Drive Tundra","Resonant Landing"):p["distHot"]=1
  if name in ('Velvet Chapel','Velvet Ice'):table='NorthernHollow'
  if name=='Crystal Runway':table='EnsembleTriangle'

  if category=='Plucks':p.update(ampSustain=0)
  if category in ('Arps','Motion'):
   # Explicit rhythm/ordering palette; every bass elsewhere stays unsequenced.
   order,rate,octaves,gate=[(0,2,2,.55),(1,2,1,.38),(2,4,2,.62),(0,3,1,.45),(2,2,3,.7),(1,4,2,.58),(0,1,2,.42),(2,3,1,.32),(1,2,3,.8),(0,4,1,.65)][i]
   p.update(arpOrder=order,patternRate=rate,arpOctaves=octaves,patternGate=gate,ampSustain=0)
  if table:
   table={'Cosmic Lobby':'DigitalGlass','Lore Horizon':'DigitalVoice','Magic Terrace':'DigitalWire','Glass Canopy':'DigitalGlass','Crystal Weather':'DigitalGlass','Wire Twilight':'DigitalWire','Spectrum Pavilion':'NorthernVowel','Triangle Aurora':'EnsembleTriangle','Vocal Relay':'NorthernVowel'}.get(name,table)
  if name=='Garage Drawbars':table='DigitalDrawbar'
  # Slow spectral movement only; no unintentional amp fades.
  modulation=[]
  if category in ('Pads','Textures') and table:
   p.update(designerLfo1Enabled=1,designerLfo1Sync=0,designerLfo1Speed=.07 if family=='JP' else .11,designerLfo1CurveEnabled=1,designerLfo1CurveCount=3,designerLfo1X0=0,designerLfo1X1=.5,designerLfo1X2=1,designerLfo1Y0=0,designerLfo1Y1=1,designerLfo1Y2=0)
   modulation=[dict(source=3,target='wavetableScan',depth=.1)]
  for k,v in p.items():
   if k not in limits:raise ValueError('Unknown '+k)
   if not limits[k]['minimum']-1e-5<=v<=limits[k]['maximum']+1e-5:raise ValueError((name,k,v))
  rows.append(dict(id=f'H{len(rows)+1:03}',name=name,family=family,category=category,profile=profile,creator='Orbital Audio',tags=f'Heritage 200, {family} Influence, {category}, '+('Sequenced' if category in ('Arps','Motion') else 'MIDI Played'),wavetable=table,settings=p,modulation=modulation,description=f'{family}-influenced {profile} voice. '+('Hold a chord; internal arpeggiator generates notes.' if category in ('Arps','Motion') else 'Play your own notes; no sequencer.'),status='draft_pending_native_audition'))
assert len(rows)==200,len(rows);assert len({r['name'] for r in rows})==200
result=dict(title='Heritage 200',preserve_factory_bank=True,status='draft_pending_native_audition',presets=rows);(out/'Designs.json').write_text(json.dumps(result,indent=2)+'\n');print('Authored',len(rows),'draft recipes:',{f:sum(r['family']==f for r in rows) for f in ('M1','JP','Nord')})
