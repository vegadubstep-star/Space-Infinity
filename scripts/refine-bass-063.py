"""Thirty-five deliberately voiced, MIDI-played trance basses; no random patches."""
CATEGORIES=('Offbeat Bass Stabs','Sub Bass Stabs','Rolling Bass')
# Each named design has its own oscillator/filter/articulation choices. Wave B
# uses the family index here; design-trance-bank adds its Follow-choice offset.
OFF=[
 dict(wave=0,osc2Wave=1,mix=.20,osc2Octave=1,cutoff=240,filterEnvAmount=3.1,filterDecay=.115,res=.20,filterMode=1,subLevel=.30,pitchEnvAmount=1.4,pitchEnvDecay=.012),
 dict(wave=1,osc2Wave=2,mix=.12,osc1Warp=3,osc1WarpAmount=.30,cutoff=190,filterEnvAmount=3.8,filterDecay=.17,res=.43,filterMode=5,subLevel=.28,pitchEnvAmount=2.0,pitchEnvDecay=.018),
 dict(wave=2,osc2Wave=2,osc2Octave=1,mix=.035,osc1Warp=1,osc1WarpAmount=.14,osc1WarpEnvelope=2,cutoff=1800,filterEnvAmount=.6,filterDecay=.10,res=.08,filterMode=1,subLevel=.26),
 dict(wave=0,osc2Wave=0,osc2Tune=5.0,mix=.32,cutoff=310,filterEnvAmount=2.7,filterDecay=.15,res=.18,filterMode=5,subLevel=.25,distType=14,distDrive=.29,distInput=12,distMix=.28),
 dict(wave=0,osc2Wave=1,osc2Octave=-1,mix=.26,cutoff=540,filterEnvAmount=2.0,filterDecay=.20,res=.12,filterMode=1,subLevel=.15,ampSustain=.36,ampDecay=.30),
 dict(wave=1,osc2Wave=0,osc2Octave=1,mix=.16,osc1Warp=3,osc1WarpAmount=.52,cutoff=390,filterEnvAmount=2.6,filterDecay=.12,res=.54,filterMode=1,subLevel=.30,eq3Frequency=950,eq3Gain=1.5,eq3Q=1.1),
 dict(wave=0,osc2Wave=1,mix=.18,osc1Warp=3,osc1WarpAmount=.20,cutoff=280,filterEnvAmount=3.8,filterDecay=.085,res=.22,filterMode=4,subLevel=.30,distType=16,distDrive=.32,distInput=15,distMix=.34,pitchEnvAmount=1.0,pitchEnvDecay=.010),
 dict(wave=0,osc2Wave=2,mix=.08,osc2Octave=2,osc1Warp=1,osc1WarpAmount=.065,osc1WarpEnvelope=2,cutoff=850,filterEnvAmount=1.8,filterDecay=.10,res=.24,filterMode=1,subLevel=.26,distType=3,distDrive=.26,distInput=12,distMix=.24),
 dict(wave=3,osc2Wave=2,osc2Octave=-1,mix=.18,cutoff=1100,filterEnvAmount=.9,filterDecay=.22,res=.08,filterMode=1,subLevel=.12,ampDecay=.30,ampSustain=.45,filterInputDrive=.09),
 dict(wave=1,osc2Wave=0,osc2Octave=1,mix=.24,cutoff=320,filterEnvAmount=3.2,filterDecay=.13,res=.28,filterMode=4,subLevel=.26,pitchEnvAmount=2.8,pitchEnvDecay=.014),
 dict(wave=0,osc2Wave=0,osc2Semitone=7,mix=.16,cutoff=580,filterEnvAmount=2.2,filterDecay=.14,res=.16,filterMode=1,subLevel=.28,distType=0,distDrive=.20,distInput=10,distMix=.22),
 dict(wave=2,osc2Wave=2,osc2Octave=2,mix=.025,osc1Warp=1,osc1WarpAmount=.24,osc1WarpEnvelope=2,cutoff=2200,filterEnvAmount=.8,filterDecay=.075,res=.16,filterMode=4,subLevel=.30,distType=14,distDrive=.32,distInput=15,distMix=.26,ampDecay=.16,ampSustain=.16),
 dict(wave=2,osc2Wave=2,osc2Octave=1,mix=.24,cutoff=3200,filterEnvAmount=0,filterDecay=.13,res=.03,filterMode=1,subLevel=.40,subOctave=-1,ampDecay=.19,ampSustain=.26),
 dict(wave=31,osc2Enabled=0,mix=0,cutoff=150,filterEnvAmount=2.8,filterDecay=.18,res=.64,filterMode=6,subLevel=.25,distType=15,distDrive=.28,distInput=12,distMix=.32),
 dict(wave=0,osc2Wave=3,mix=.34,cutoff=170,filterEnvAmount=2.7,filterDecay=.25,res=.14,filterMode=5,subLevel=.28,ampDecay=.32,ampSustain=.35,filterInputDrive=.10),
]
ROLL=[
 dict(wave=0,osc2Wave=2,mix=.12,cutoff=230,filterEnvAmount=3.1,filterDecay=.072,res=.18,filterMode=1,subLevel=.28),
 dict(wave=1,osc2Wave=2,mix=.18,osc1Warp=3,osc1WarpAmount=.22,cutoff=200,filterEnvAmount=3.0,filterDecay=.065,res=.30,filterMode=5,subLevel=.20),
 dict(wave=2,osc2Wave=2,osc2Octave=1,mix=.025,osc1Warp=1,osc1WarpAmount=.09,osc1WarpEnvelope=0,cutoff=1600,filterEnvAmount=.7,filterDecay=.06,res=.05,filterMode=1,subLevel=.30,ampSustain=.42),
 dict(wave=0,osc2Wave=0,mix=.30,osc2Tune=-4.5,cutoff=330,filterEnvAmount=2.4,filterDecay=.10,res=.19,filterMode=5,subLevel=.28),
 dict(wave=0,osc2Wave=3,osc2Octave=-1,mix=.24,cutoff=650,filterEnvAmount=1.4,filterDecay=.085,res=.10,filterMode=1,subLevel=.14,ampDecay=.23,ampSustain=.46),
 dict(wave=1,osc2Wave=1,mix=.20,osc2Octave=1,osc1Warp=3,osc1WarpAmount=.56,cutoff=370,filterEnvAmount=2.2,filterDecay=.075,res=.48,filterMode=1,subLevel=.28,eq3Frequency=1050,eq3Gain=1.6,eq3Q=1.2),
 dict(wave=1,osc2Wave=0,mix=.16,osc1Warp=3,osc1WarpAmount=.36,cutoff=270,filterEnvAmount=3.1,filterDecay=.055,res=.21,filterMode=4,subLevel=.26,distType=16,distDrive=.28,distInput=12,distMix=.28),
 dict(wave=0,osc2Wave=2,osc2Octave=2,mix=.035,osc1Warp=1,osc1WarpAmount=.052,osc1WarpEnvelope=2,cutoff=950,filterEnvAmount=1.5,filterDecay=.075,res=.19,filterMode=1,subLevel=.26),
 dict(wave=3,osc2Wave=2,mix=.22,cutoff=1500,filterEnvAmount=.65,filterDecay=.12,res=.06,filterMode=1,subLevel=.16,ampSustain=.48,ampDecay=.23),
 dict(wave=1,osc2Wave=0,mix=.22,osc2Octave=1,cutoff=440,filterEnvAmount=2.0,filterDecay=.068,res=.22,filterMode=4,subLevel=.24,pitchEnvAmount=.65,pitchEnvDecay=.008),
 dict(wave=0,osc2Wave=0,mix=.13,osc2Semitone=7,cutoff=850,filterEnvAmount=1.4,filterDecay=.085,res=.13,filterMode=1,subLevel=.30,distType=0,distDrive=.22,distInput=10,distMix=.24),
 dict(wave=2,osc2Wave=2,mix=.02,osc2Octave=2,osc1Warp=1,osc1WarpAmount=.19,osc1WarpEnvelope=2,cutoff=2300,filterEnvAmount=.6,filterDecay=.065,res=.10,filterMode=1,subLevel=.30,distType=3,distDrive=.28,distInput=12,distMix=.22,ampSustain=.24),
 dict(wave=2,osc2Wave=2,mix=.25,osc2Octave=1,cutoff=3600,filterEnvAmount=0,filterDecay=.10,res=.02,filterMode=1,subLevel=.32,subOctave=-1,ampDecay=.15,ampSustain=.36),
 dict(wave=32,osc2Enabled=0,mix=0,cutoff=180,filterEnvAmount=2.3,filterDecay=.085,res=.56,filterMode=6,subLevel=.25,distType=14,distDrive=.26,distInput=12,distMix=.25),
 dict(wave=0,osc2Wave=3,mix=.30,cutoff=150,filterEnvAmount=2.6,filterDecay=.14,res=.24,filterMode=5,subLevel=.30,ampSustain=.45,ampDecay=.24,filterInputDrive=.10),
]
SUB=[
 dict(wave=2,osc2Enabled=0,mix=0,cutoff=5000,filterMode=1,subEnabled=0,ampDecay=.25,ampSustain=.70),
 dict(wave=3,osc2Wave=2,mix=.32,cutoff=800,filterMode=1,filterEnvAmount=.7,filterDecay=.16,ampDecay=.24,ampSustain=.58),
 dict(wave=1,osc2Wave=2,mix=.15,cutoff=210,filterMode=4,res=.10,filterEnvAmount=.8,filterDecay=.11,subLevel=.25,ampDecay=.19,ampSustain=.48),
 dict(wave=2,osc2Wave=2,mix=.015,osc2Octave=0,osc1Warp=1,osc1WarpAmount=.10,osc1WarpEnvelope=2,cutoff=700,filterMode=1,filterEnvAmount=.6,filterDecay=.12,subLevel=.12,ampDecay=.22,ampSustain=.62),
 dict(wave=2,osc2Wave=2,mix=.18,osc2Octave=0,cutoff=1400,filterMode=1,filterEnvAmount=.2,filterDecay=.18,ampDecay=.28,ampSustain=.66),
]
ROLES=['Saw / octave punch','Rubber pulse','FM knock','Detuned saw body','Octave weight','Vocal pulse','Driven razor','Metallic bounce','Rounded triangle','Octave snap','Fifth body','FM steel','Drawbar weight','Resonant acid body','Warm low-pass']
def refine(bank,env,space):
    counts={}
    for patch in bank:
        category=patch['category']
        if category not in CATEGORIES:continue
        i=counts.get(category,0);counts[category]=i+1;rolling=category==CATEGORIES[2];sub=category==CATEGORIES[1]
        p=patch['settings'];p.update(source=0,wave=0,osc2Wave=2,osc1Octave=-1 if sub else 0,osc2Octave=-1 if sub else 0,masterOctave=0,osc1Tune=0,osc2Tune=0,osc1Semitone=0,osc2Semitone=0,osc1Enabled=1,osc2Enabled=1,osc1Unison=1,osc2Unison=1,osc1Gain=0,osc2Gain=0,osc1Width=0,osc2Width=0,osc1Pan=0,osc2Pan=0,subPan=0,osc1PhaseReset=1,osc2PhaseReset=1,osc1Rand=0,osc2Rand=0,osc1Phase=0,osc2Phase=0,osc1Warp=0,osc2Warp=0,osc1WarpAmount=0,osc2WarpAmount=0,osc1WarpEnvelope=0,osc2WarpEnvelope=0,wavetableScan=0,osc2WTPos=0,subEnabled=1,subLevel=0,subOctave=-1 if sub else 0,subWave=0,routeA=1,routeB=1,routeSub=0,noiseLevel=0,voiceMode=1,glideTime=0,filterMode=1,cutoff=400,res=.12,filterEnvAmount=2.5,filterAttack=.001,filterDecay=.12,filterSustain=0,filterRelease=.025,filterInputDrive=.045,filterVelocity=.7,ampVelocity=.38,filterHold=0,ampHold=0,filterDecayCurve=-.30,ampDecayCurve=-.15,ampAttackCurve=-.35,ampReleaseCurve=.05,filterAttackCurve=-.35,filterReleaseCurve=.05,pitchEnvAmount=0,pitchEnvAttack=0,pitchEnvDecay=.012,pitchEnvVelocity=.25,patternMode=0,gateEnabled=0,pumpEnabled=0,vibratoEnabled=0,lfodepth=0,motionDepth=0,drive=.008,analogueCharacter=.025,analogueLabEnabled=0,analogueLabRange=0,analogueVCADrive=0,analogueAsymmetry=0,analogueMemory=0,analogueCoupling=0,analogueOscDrift=0,analogueThermal=0,analogueFilterSpread=0,analogueEnvSpread=0,analogueVoiceSpread=0,analoguePulseDrift=0,distEnabled=0,distInput=0,distDrive=.2,distMix=0,distHQ=1,distTone=10000,distFilterPosition=1,distFilterMorph=1,distFrequency=140,distQ=.707,fxOutput8=-3,eqEnabled=1,eqMode0=4,eq0Frequency=20 if sub else 25,eq0Q=.707,eqMode6=3,eq6Frequency=1800 if sub else 11000,eq6Q=.707,eq3Frequency=360,eq3Gain=0 if sub else -.8,eq3Q=.8,eq1Gain=0,eq2Gain=0,eq4Gain=0,eq5Gain=0,gain=.40)
        p.update(env(.0025 if sub else .0012,.19 if rolling else .23,.34 if rolling else .24,.018 if rolling else .038));p.update(space(0,0,0))
        recipe=(SUB if sub else ROLL if rolling else OFF)[i];p.update(recipe)
        if sub:p.update(filterEnvAmount=recipe.get('filterEnvAmount',0),res=recipe.get('res',.02),filterInputDrive=0,drive=0,analogueCharacter=0,filterVelocity=.20,ampVelocity=.24,release=.032)
        p['distEnabled']=int(p['distMix']>0)
        for key in ('lfo','grain_resource'):patch.pop(key,None)
        role=['Pure foundation','Triangle warmth','Filtered pulse weight','FM harmonic sub','Octave sine body'][i] if sub else ROLES[i]
        patch['play']=role+'. Play your own '+('short low notes (MIDI36–55); stable mono sub, no pitch kick.' if sub else '16th-note bass patterns; sustained body follows your note lengths, with a shorter filter contour and clean mono weight.' if rolling else 'offbeat notes; a fast attack and independent filter contour leave weight behind the transient.')+' TONE opens the body, PUNCH shapes articulation, CHARACTER adds harmonics.'
        patch['tags']=category+', Bass, Unsequenced, Mono, '+role
