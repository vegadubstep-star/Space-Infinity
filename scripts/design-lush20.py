"""Author the additional bank. Original settings, distinct oscillator/filter architectures."""
import copy,json,sys
from pathlib import Path
root=Path(__file__).resolve().parents[1]
factory=json.loads((root/'native-vst3/Presets/TranceFactoryBank.json').read_text())
base=copy.deepcopy(factory['presets'][35]['settings'])
# name, wave A/B, octave B, mix, unison A/B, detune A/B, cutoff, filter, contour, release, chorus voicing
voices=[
 ('Velvet Zenith',0,6,0,.32,7,5,.13,.09,6100,4,.55,.28,0),
 ('Opal Current',47,37,0,.24,5,3,.095,.055,7200,1,.8,.19,1),
 ('Silken Orbit',48,0,0,.38,6,4,.11,.075,6800,5,.35,.32,2),
 ('Crystal Canopy',0,20,1,.12,7,2,.12,.025,7400,1,.65,.26,3),
 ('Hollow Aurora',42,14,0,.28,5,3,.09,.065,5200,4,.85,.23,0),
 ('Pearlescent Tide',37,47,1,.18,5,3,.07,.09,8300,1,.4,.3,1),
 ('Astral Silk',44,38,0,.31,5,4,.105,.055,5700,5,.6,.34,2),
 ('Emerald Halo',17,0,1,.36,4,7,.045,.13,6500,1,.7,.2,0),
 ('Celestial Ribbon',0,6,1,.24,9,5,.17,.10,7600,4,.45,.28,3),
 ('Iridescent Promise',6,45,0,.19,7,3,.1,.045,6200,1,.5,.33,2),
 ('Luminous Reed',13,0,0,.43,4,7,.055,.115,6900,4,.9,.17,1),
 ('Prismatic Breath',47,38,0,.35,7,5,.12,.075,5900,7,.6,.27,0),
 ('Sapphire Aperture',0,18,-1,.18,7,3,.14,.035,8400,4,.3,.24,3),
 ('Ivory Radiance',38,6,0,.45,5,5,.06,.095,7300,1,.7,.35,2),
 ('Violet Choirline',0,12,0,.21,7,4,.12,.045,5400,11,.35,.3,1),
 ('Golden Airfoil',16,44,0,.35,5,5,.085,.12,7900,1,.5,.22,0),
 ('Serene Hypersky',46,0,1,.24,5,7,.075,.12,6400,4,.7,.28,3),
 ('Dawn Resonance',0,51,0,.18,7,3,.13,.055,4800,14,.55,.25,2),
 ('Diamond Horizon',43,47,0,.38,7,5,.11,.085,8100,1,.4,.2,0),
 ('Infinite Embrace',6,0,0,.49,9,7,.15,.11,6700,5,.65,.38,1),
]
rows=[]
for i,(name,a,b,octave,mix,ua,ub,da,db,cut,mode,contour,release,chorus) in enumerate(voices):
 s=copy.deepcopy(base);s.update(wave=a,osc2Wave=b,osc2Octave=octave,mix=mix,osc1Unison=ua,osc2Unison=ub,osc1Detune=da,osc2Detune=db,cutoff=cut,filterMode=mode,filterEnvAmount=contour,attack=.0005 if i%3 else .001,ampDecay=.28+.025*(i%5),ampSustain=.86+.02*(i%4),release=release,filterDecay=.16+.03*(i%4),filterSustain=.32,echoMix=.07+.01*(i%3),reverb=.08+.01*(i%4),chorus=.12,chorusEngine=1,dimensionMode=chorus,chorusRate=.12+.025*(i%5),chorusDepth=.28,oscMelt=.05+.025*(i%4),soundLife=.18+.025*(i%5),presetTrim=0,osc1Width=.78+.025*(i%4),osc2Width=.84,ampVelocity=.3,osc2Tune=(-2 if i%2 else 2),wavetableScan=.12+.04*(i%5),osc2WTPos=.18+.045*(i%4),filterVowel=1.2,filterMorph=.12,oscTransition=0)
 if i in (3,6,9,13,16,19):s.update(oscTransition=.25,oscTransitionTime=.6+.2*(i%4),oscTransitionCurve=-.2,oscTransitionDirection=i%2)
 if mode==11:s.update(cutoff=1100,res=.36,osc2Wave=0,filterEnvAmount=.05,filterVelocity=.04,routeB=0,mix=.4,osc1Unison=5,osc2Unison=7)
 if mode==7:s.update(cutoff=2300,res=.25)
 macros=[dict(source=0,target='cutoff',depth=.055 if mode not in (7,11) else .012),dict(source=0,target='echoMix',depth=-.025),dict(source=0,target='reverb',depth=-.025),dict(source=0,target='ampSustain',depth=.06),dict(source=1,target='oscMelt',depth=.09),dict(source=1,target='filterInputDrive',depth=.025),dict(source=1,target='presetTrim',depth=-.022),dict(source=2,target='osc1Width',depth=.08),dict(source=2,target='osc2Width',depth=.07),dict(source=2,target='soundLife',depth=.10),dict(source=8,target='reverb',depth=.035),dict(source=8,target='echoMix',depth=.025),dict(source=6,target='filterEnvAmount',depth=.025),dict(source=7,target='osc1Width',depth=.08)]
 s.update(characterFilterMode=mode-6 if mode>=7 else 0,filterMode=mode if mode<7 else 0)
 rows.append(dict(id=f'LT{i+1:03}',name=name,family='Lush Trance',category='Lead',creator='Orbital Audio',tags='Lead, Trance, Lush, Stereo, Lush 20',description=f'Original {a}/{b} oscillator architecture; {ua}+{ub} voices, filter {mode}. Fast melodic attack, sustained body and controlled spatial returns. OPEN brightens and dries the lead; BODY adds oscillator interaction; HORIZON broadens its image.',factory_index=0,settings=s,macro_labels=['OPEN','BODY','HORIZON'],macros=macros))
out=root/'native-vst3/Presets/Lush20/Designs.json'
if out.exists() and '--overwrite' not in sys.argv:raise SystemExit('Refusing to overwrite a measured bank; use --overwrite only when deliberately re-authoring.')
out.write_text(json.dumps(dict(title='Lush Trance 20',presets=rows),indent=2)+'\n');print(out)
