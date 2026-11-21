"""Author the research pass from a supplied snapshot, without replacing calibrated banks.

Usage: python scripts/refine-research-bank.py BASELINE_JSON OUTPUT_JSON
The output contains only the 90 deliberately revised recipes. Native rendering,
macro compensation and level calibration must precede integration or packaging.
"""
import copy
import json
import sys
from pathlib import Path

baseline = json.loads(Path(sys.argv[1]).read_text())
rows = []

def route(row, source, target, depth):
    row['macros'].append(dict(source=source, target=target, depth=depth))

# Main oscillator, supporting oscillator (actual wave index), filter decay,
# amp decay, sustain, resonance. These are individual articulations, not seeds.
bass = [
 (0,1,.105,.205,.20,.20), (1,2,.16,.28,.30,.43),
 (2,2,.085,.19,.16,.08), (6,0,.125,.26,.22,.23),
 (0,1,.21,.36,.42,.12), (1,0,.23,.30,.18,.48),
 (42,1,.075,.16,.12,.24), (50,2,.095,.21,.20,.19),
 (3,2,.19,.38,.48,.08), (4,0,.14,.24,.26,.31),
 (0,0,.17,.31,.24,.17), (2,2,.065,.14,.10,.14),
 (18,2,.12,.25,.32,.04), (31,2,.135,.24,.24,.65),
 (6,3,.26,.40,.42,.15),
 (0,2,.065,.14,.24,.18), (1,2,.085,.18,.28,.29),
 (2,2,.052,.12,.20,.05), (13,0,.11,.22,.26,.22),
 (6,3,.15,.26,.40,.10), (1,1,.19,.24,.24,.43),
 (4,0,.055,.13,.18,.23), (22,2,.075,.17,.26,.18),
 (3,2,.13,.24,.42,.06), (1,0,.10,.19,.29,.24),
 (0,0,.115,.22,.28,.14), (2,2,.048,.11,.14,.10),
 (18,2,.09,.19,.32,.03), (32,2,.105,.19,.26,.61),
 (6,3,.18,.29,.38,.25),
]

# Each acid is a separate playable instrument: saw/square/pulse plus different
# filter time constants, nonlinear transfer functions and note lengths.
acid = [
 # wave, cutoff, resonance, env octaves, decay, distortion, drive, wet, glide
 (31,170,.82,3.3,.205,15,.42,.73,.065),
 (32,230,.85,2.8,.145,14,.46,.78,.075),
 (33,340,.70,2.3,.092,2,.32,.45,.045),
 (34,205,.89,3.5,.285,3,.44,.75,.09),
 (35,155,.86,3.7,.125,16,.44,.49,.055),
 (36,420,.73,2.6,.075,2,.53,.68,.04),
 (32,115,.88,2.9,.34,0,.38,.70,.11),
 (31,280,.76,3.0,.175,15,.34,.61,.06),
 (34,190,.87,3.8,.15,16,.49,.54,.05),
 (33,135,.80,3.1,.395,3,.30,.57,.12),
]

# Synth plucks: cutoff, filter octaves, filter decay, audible held body.
plucks = {
 'TI066':(900,3.4,.24,.30), 'TI067':(680,3.1,.18,.26),
 'TI072':(1100,2.6,.135,.24), 'TI074':(650,1.5,.32,.32),
 'TI075':(420,3.6,.092,.22), 'TI076':(1250,2.1,.21,.34),
 'TI077':(1350,1.8,.16,.38), 'TI078':(1600,2.3,.27,.26),
 'TI079':(750,3.4,.145,.30), 'TI080':(2300,1.5,.36,.22),
 'TI082':(1700,2.0,.19,.24), 'TI083':(480,3.2,.29,.34),
 'TI085':(1150,2.8,.17,.28),
}

# Pad identities: attack seconds, release seconds, cutoff, chorus wet, air.
pads = [
 (.75,2.6,6100,.19,.16), (1.05,2.9,8900,.09,.08),
 (1.25,2.8,7200,.11,.12), (.62,2.25,5200,.16,.20),
 (.92,2.7,8200,.10,.10), (.82,2.45,6700,.13,.12),
 (.48,2.1,9100,.12,.14), (1.15,2.95,5700,.18,.22),
 (.68,2.35,7400,.20,.24), (1.35,2.85,8400,.10,.18),
]

counts = {}
for index, original in enumerate(baseline['presets']):
    cat = original['category']
    if cat not in ('Offbeat Bass Stabs','Rolling Bass','Hard Trance Leads',
                   '90s German Leads','Trance Plucks','Airy Pads','Acid'):
        continue
    row = copy.deepcopy(original)
    s = row['settings']
    i = counts.get(cat, 0)
    counts[cat] = i + 1
    row['factory_index'] = index + 1
    row['family'] = 'Studio Research'
    row['macros'] = [m for m in row.get('macros', [])
                     if m['target'] != 'presetTrim']
    # Safe initial render level; replaced by measured calibration later.
    s.update(presetTrim=-12, outputSafety=1)
    if cat in ('Offbeat Bass Stabs','Rolling Bass'):
        j = i + (15 if cat == 'Rolling Bass' else 0)
        wave, support, fd, ad, sustain, resonance = bass[j]
        s.update(wave=wave, osc2Wave=support+1, filterDecay=fd,
                 ampDecay=ad, ampSustain=sustain, res=resonance,
                 attack=.0012, release=.028 if cat=='Rolling Bass' else .043,
                 ampDecayCurve=-.24, filterDecayCurve=-.38,
                 subWave=0, subPan=0, subLevel=min(.28,s.get('subLevel',.20)),
                 routeSub=0, osc1Width=0, osc2Width=0,
                 osc1Rand=0, osc2Rand=0, osc1PhaseReset=1, osc2PhaseReset=1,
                 osc1Unison=1, osc2Unison=1, noiseLevel=0,
                 patternMode=0, gateEnabled=0, pumpEnabled=0,
                 analogueLabEnabled=1, analogueVCADrive=.055,
                 analogueAsymmetry=.025, analogueMemory=.035,
                 analogueOscDrift=0, analogueThermal=0, soundLife=0)
        # Harmonic families already encoded in these waves need less extra FM.
        if wave in (18,22,50):
            s['osc1WarpAmount'] *= .55
        if wave==18:
            s.update(osc1Warp=0, osc2Octave=1, mix=.19)
        # Keep the FM basses apart by modulator ratio, not just envelope length.
        if j in (11,17,26):
            octave, semitone, amount={11:(1,0,.17),17:(1,7,.13),26:(2,4,.105)}[j]
            s.update(osc2Octave=octave,osc2Semitone=semitone,
                     osc1Warp=1,osc1WarpAmount=amount,mix=.06)
        if row['id']=='TI033':
            s.update(wave=17,osc2Wave=3,osc2Octave=2,mix=.21,
                     cutoff=4200,res=.06,filterEnvAmount=.7,
                     filterDecay=.055,ampDecay=.12,ampSustain=.18,subLevel=.13)
        if row['id']=='TI035':
            s.update(wave=14,osc2Wave=3,osc2Octave=0,mix=.10,
                     cutoff=820,filterEnvAmount=1.65,filterDecay=.12,
                     ampDecay=.20,ampSustain=.30,subLevel=.17)
        row['macro_labels']=['TONE','PUNCH','CHARACTER']
        row['description']=(row.get('play','')+' Research revision: independently '
            'voiced filter and amp contour, repeatable centred bass attack and '
            'separate clean sine weight. No built-in bass sequence.')
    elif cat=='Acid':
        wave, cutoff, resonance, amount, decay, dtype, drive, wet, glide=acid[i]
        s.update(wave=wave, osc1Unison=1, osc2Unison=1,
                 osc1Width=0, osc2Width=0, cutoff=cutoff, res=resonance,
                 filterEnvAmount=amount, filterDecay=decay, filterSustain=0,
                 filterDecayCurve=-.38, ampDecay=.12+decay*.42,
                 ampSustain=.44 if i==8 else .68 if i not in (2,5) else .48,
                 attack=.0006, release=.022 if i<6 else .036,
                 voiceMode=2, glideTime=glide, filterVelocity=.86,
                 ampVelocity=.48, distEnabled=1, distHQ=1, distHot=1,
                 distType=dtype, distDrive=drive, distMix=wet,
                 distInput=[12,12,6,6,6,12,12,12,0,12][i],
                 distFilterPosition=2, distFilterMorph=0,
                 distFrequency=[11500,8700,7600,10400,9200,6800,8000,12800,7800,9600][i],
                 distTone=14000, filterInputDrive=.12,
                 analogueLabEnabled=1, analogueVCADrive=.045,
                 analogueMemory=.045, soundLife=.025,
                 chorusEnabled=0, reverbEnabled=1, reverb=.012,
                 echoMix=.055 if i in (3,7,9) else .025,
                 delayFeedback=.23, delayWetLowFreq=330, delayWetLowGain=-12,
                 reverbWetLowFreq=420, reverbWetLowGain=-12)
        row['macros']=[m for m in row['macros'] if m['source'] not in (0,1,2)]
        for source,target,depth in [(0,'cutoff',.52),(0,'filterEnvAmount',.05),
             (1,'filterDecay',.052),(1,'filterEnvAmount',.12),
             (2,'distDrive',.17),(2,'distMix',.12)]:route(row,source,target,depth)
        row['macro_labels']=['SWEEP','LIQUID','CORROSION']
        row['description']=('Velocity accents open the resonant contour and amp. '
            'Overlap notes for legato slides; separate notes retrigger the squelch. '
            'SWEEP opens cutoff, LIQUID lengthens the filter gesture, CORROSION '
            'adds a distinct nonlinear drive character. Program your own MIDI.')
    elif cat=='Trance Plucks':
        if row['id'] in plucks:
            cutoff, amount, decay, sustain=plucks[row['id']]
            s.update(cutoff=cutoff, filterEnvAmount=amount, filterDecay=decay,
                     ampSustain=sustain, ampDecay=max(.22,decay*1.45),
                     filterSustain=0, filterDecayCurve=-.30,
                     ampDecayCurve=-.12, soundLife=.10)
        # Individual space/timing, rather than identical returns on every pluck.
        s.update(echoMix=[.17,.14,.13,.10,.16,.12,.19,.09,.16,.12,
                          .10,.08,.18,.13,.17,.10,.15,.14,.12,.16][i],
                 reverb=[.18,.14,.19,.12,.15,.16,.13,.10,.20,.12,
                         .10,.13,.21,.16,.22,.11,.18,.15,.24,.14][i],
                 delayFeedback=[.29,.23,.31,.19,.26][i%5],
                 echoSyncL=[4,5,4,3,5][i%5],
                 echoSyncR=[5,4,6,5,6][i%5],
                 chorus=.045 if s.get('source',0)==0 else .025,
                 reverbWetLowFreq=320, reverbWetLowGain=-12,
                 delayWetLowFreq=270, delayWetLowGain=-10)
        row['macros']=[m for m in row['macros'] if m['source']!=0]
        for target,depth in [('cutoff',.58),('echoMix',-s['echoMix']*.68),
                             ('reverb',-s['reverb']*.63),('filterInputDrive',.025)]:
            route(row,0,target,depth)
        if s.get('source',0)==0:
            route(row,0,'ampSustain',.86-s['ampSustain'])
            route(row,0,'ampDecay',.065)
            route(row,0,'oscMelt',.07)
        row['macro_labels']=['OPEN / LEAD','CONTOUR','CHARACTER']
        row['description']=('A distinct transient and filtered body with voiced '
            'delay spacing. OPEN / LEAD raises cutoff and sustain while reducing '
            'delay/reverb returns; sampled plucks retain their natural decay. '
            'Its output compensation is measured from the native instrument.')
    elif cat=='Airy Pads':
        attack, release, cutoff, chorus, life=pads[i]
        # x3 mode scales attack/decay/release together: store unscaled values.
        s.update(ampLong=1, attack=attack/3, release=release/3,
                 ampDecay=.8, ampSustain=1, cutoff=cutoff,
                 filterAttack=min(1,attack*.8), filterDecay=3.2,
                 filterSustain=.65, filterEnvAmount=.5,
                 chorus=chorus, reverb=.29, reverbSize=.86,
                 reverbWetLowFreq=360, reverbWetLowGain=-12,
                 reverbWetHighFreq=9600, reverbWetHighGain=-2,
                 delayWetLowFreq=420, delayWetLowGain=-12,
                 echoMix=.10, delayFeedback=.32, soundLife=life,
                 chorusEngine=1, dimensionMode=3 if i in (0,3,7,8) else 2,
                 dimensionWidth=1.12, dimensionTone=.70)
        if s.get('source',0)==0:
            s.update(osc1WTSpread=.065, osc2WTSpread=.045)
            row['lfo']=dict(hz=[.073,.051,.061,.043,.083][i%5],
                            target='wavetableScan',depth=.035)
        else:
            s.update(sampleLoopFade=65, sampleCloudMix=.065,
                     sampleSmear=.08, sampleDarken=.035)
            # Older numeric Bars entries selected fractional-bar menu entries.
            # Use explicit Hz here so these sampled pads genuinely move slowly.
            row['lfo']=dict(hz=[.053,.041,.047,.037,.061][i%5],
                            target='cutoff',depth=.012)
        if s.get('source')==4:
            s.update(grainSize=170, grainDensity=34, grainSpray=.025,
                     grainSpread=.62, grainFreeze=1)
        row['macro_labels']=['BLOOM','CONTOUR','CHARACTER']
        row['description']=('Slow, sustained emotional chord layer: a shaped '
            'swell, uncluttered effect low end and independent gentle timbral '
            'motion. No amplitude LFO. Longer release is intentional; leave '
            'space between chord changes. BLOOM opens the harmonic colour.')
    else:
        # Retain the already distinct pulse/FM/reed/fifth architectures, and
        # give the sustained phrase a firmer body with controlled short FX.
        s.update(attack=min(.0015,s['attack']),
                 ampSustain=[.84,.88,.82,.90,.86,.84,.92,.86,.88,.82][i],
                 ampDecay=[.24,.31,.19,.28,.34,.22,.26,.20,.32,.25][i],
                 filterDecay=[.15,.22,.18,.26,.12,.17,.31,.21,.14,.24][i],
                 soundLife=.10 if cat=='Hard Trance Leads' else .14,
                 echoMix=min(.11,s.get('echoMix',.08)),
                 reverb=min(.13,s.get('reverb',.10)),
                 delayWetLowFreq=320, delayWetLowGain=-12,
                 reverbWetLowFreq=380, reverbWetLowGain=-12,
                 osc1WTSpread=.025, osc2WTSpread=.015)
        if row['id']=='TI052':s.update(osc1Warp=3,osc1WarpAmount=.22)
        if row['id'] in ('TI047','TI051','TI059','TI063'):
            s.update(osc1WarpEnvelope=2, filterEnvAmount=.9, filterSustain=.35)
        row['description']=(row.get('play','')+' Research revision: firm '
            'sustained lead body, independently voiced attack/filter decay and '
            'less low-frequency wash in spatial returns. Existing pulse, reed, '
            'FM and stacked-interval identities are retained.')
    row['play']=row['description']
    row['tags']=', '.join(dict.fromkeys(t.strip() for t in
        (row.get('tags','')+', Studio Research, Research 091').split(',') if t.strip()))
    row.update(review_revision='research-091-pending',status='awaiting_native_audio_review')
    row.pop('open_compensation_db',None)
    rows.append(row)

assert len(rows)==90
Path(sys.argv[2]).write_text(json.dumps(dict(title='Studio Research 90',presets=rows),indent=2)+'\n')
print('Authored',len(rows),'revisions:',counts)
