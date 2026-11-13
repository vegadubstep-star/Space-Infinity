"""Distinct, restrained bass voicings and direct choir stabs from the owner's uploads."""
def refine(bank,env,space):
    # waveform A/B, layer blend, cutoff, envelope octaves, filter decay, resonance, sub level
    off=[(0,0,.18,500,3.4,.13,.12,.22),(1,3,.12,320,4.5,.17,.28,.25),(2,3,.08,900,1.5,.10,.08,.18),(6,3,.20,400,3.5,.16,.15,.22),(0,1,.22,700,2.5,.20,.08,.12),(1,2,.24,350,3.2,.14,.18,0),(0,1,.12,450,3.8,.12,.18,.20),(0,2,.15,1100,2,.16,.13,.15),(3,2,.16,600,1.6,.22,.10,0),(1,0,.18,500,3.3,.14,.24,.18),(0,0,.08,750,2.5,.15,.09,.26),(0,1,.16,1200,1.9,.10,.13,.20),(2,2,.20,1400,.2,.17,.06,.18),(31,2,0,250,4.3,.15,.55,.18),(6,3,.25,280,2.9,.24,.20,.20)]
    roll=[(0,2,.10,900,3,.085,.10,.18),(1,2,.14,620,3.5,.075,.13,.13),(2,3,.08,1600,1.4,.065,.08,.20),(6,1,.14,750,3,.10,.15,.18),(6,2,.22,1100,2,.09,.07,.12),(1,3,.18,450,3.2,.08,.23,.18),(4,2,.12,650,3.1,.07,.14,.17),(0,3,.14,1000,2.4,.09,.13,.18),(3,2,.12,850,1.8,.11,.10,0),(0,1,.10,1250,2.2,.08,.11,.18),(6,0,.07,1300,1.6,.08,.08,.22),(1,0,.12,720,3.1,.07,.20,.18),(2,2,.18,1800,.5,.095,.06,.12),(31,2,0,330,4.2,.08,.48,.18),(0,2,.25,330,3,.12,.20,.12)]
    sub=[(2,2,0,900,0,.18,.01,0),(3,2,.20,480,.2,.16,.03,0),(1,2,.12,120,.5,.14,.05,.25),(2,2,.05,350,.5,.16,.04,.15),(2,2,.15,850,.2,.20,.02,0)]
    counters={};choir_rows=[(32,0,1,.24,.12,.12,12000,.14),(34,-1,1,.20,.16,.10,8500,.08),(33,-1,1,.22,.16,.12,9000,.08),(30,0,0,.28,.20,.16,15000,.12),(35,0,1,.34,.20,.18,11000,.14),(37,-1,1,.28,.12,.13,8500,.10),(31,0,0,.25,.20,.14,12500,.12),(36,-1,1,.22,.12,.10,8000,.08),(38,0,1,.17,.18,.11,12000,.10),(33,0,1,.16,0,.09,14000,.04)]
    for patch in bank:
        p=patch['settings'];category=patch['category'];p['analogueLabRange']=0
        if category in ('Offbeat Bass Stabs','Sub Bass Stabs','Rolling Bass'):
            i=counters.get(category,0);counters[category]=i+1;rolling=category=='Rolling Bass';is_sub=category=='Sub Bass Stabs';rows=sub if is_sub else roll if rolling else off
            wave,second,mix,cut,amount,decay,res,sublevel=rows[i]
            p.update(source=0,wave=wave,osc2Wave=second,osc1Octave=-1 if is_sub else 0,osc2Octave=-1 if is_sub else 0,osc2Semitone=0,osc2Tune=0,osc1Enabled=1,osc2Enabled=int(mix>0),osc1Unison=1,osc2Unison=1,osc1Detune=.02,osc2Detune=.018,osc1Width=0,osc2Width=0,osc1Rand=0,osc2Rand=0,osc1PhaseReset=1,osc2PhaseReset=1,osc1Warp=0,osc1WarpAmount=0,osc2Warp=0,osc2WarpAmount=0,osc1Gain=0,osc2Gain=0,mix=mix,subEnabled=1,subLevel=sublevel,subOctave=-1 if is_sub else 0,subWave=0,noiseLevel=0,voiceMode=1,glideTime=0,filterMode=4,cutoff=cut,res=res,filterEnvAmount=amount,filterAttack=.0005,filterDecay=decay,filterSustain=0,filterRelease=.025,filterVelocity=.35 if is_sub else .65,filterInputDrive=0 if is_sub else .045,drive=0 if is_sub else .015,ampVelocity=.45,eq0Frequency=21 if is_sub else 27,eq6Frequency=1500 if is_sub else 8500,eq3Frequency=380,eq3Gain=0 if is_sub else -1.2,eq3Q=.8,patternMode=0,gateEnabled=0,pumpEnabled=0,vibratoEnabled=0,lfodepth=0,motionDepth=0,pitchEnvAmount=0,pitchEnvAttack=0,pitchEnvDecay=.025,analogueCharacter=0 if is_sub else .025,analogueLabEnabled=0 if is_sub else 1,analogueLabRange=0 if is_sub else 1,analogueVCADrive=0 if is_sub else .035,analogueAsymmetry=0 if is_sub else .02,analogueMemory=0 if is_sub else .02,analogueCoupling=0 if is_sub else .01,analogueOscDrift=0 if is_sub else .015,analogueThermal=0 if is_sub else .01,analogueFilterSpread=0 if is_sub else .015,analogueEnvSpread=0 if is_sub else .01,analogueVoiceSpread=0 if is_sub else .015,analoguePulseDrift=0 if is_sub else .01,distEnabled=0,distInput=0,distHQ=0,distTone=20000,gain=.40)
            p.update(env(.001 if is_sub else .0008,decay*1.30,.52 if is_sub else .08 if rolling else .22,.025 if rolling else .038));p.update(space(0,0,0))
            for key in ('lfo','grain_resource'):patch.pop(key,None)
            if is_sub:
                if i==3:p.update(osc1Warp=4,osc1WarpAmount=.025)
                if i==4:p.update(osc2Octave=0)
            else:
                if i==2:p.update(osc1Warp=4,osc1WarpAmount=.045 if rolling else .035,osc1WarpEnvelope=2)
                if i in (1,5,6):p.update(osc1Warp=3,osc1WarpAmount=.12 if i==1 else .24 if i==5 else .36)
                if i==7:p.update(osc1Warp=2,osc1WarpAmount=.055)
                if i in (4,8):p.update(osc2Octave=-1)
                if i==9:p.update(osc2Octave=1 if rolling else -1)
                if i==10:p.update(osc2Semitone=7)
                if i==12:p.update(osc2Octave=1,subOctave=-1)
                if i==13:p.update(filterMode=5,filterInputDrive=.09,distEnabled=1,distType=0,distInput=6,distDrive=.22,distMix=.25,distHQ=1,distTone=7000,fxOutput8=-1)
                if not rolling and i in (1,9):p.update(pitchEnvAmount=.8,pitchEnvDecay=.028)
                if not rolling and i==4:p.update(osc1Unison=2,osc1Width=.10,osc1Detune=.018)
                if i in ((11,) if rolling else (6,11)):p.update(distEnabled=1,distType=14 if i==6 or rolling else 2,distInput=7 if rolling else 9,distDrive=.24 if rolling else .30,distMix=.18 if rolling else .25,distHQ=1,distTone=7500,fxOutput8=-2)
            patch['play']='Play your own MIDI. '+('Clean mono sub with a phase-stable fundamental.' if is_sub else 'Tight retriggered rolling articulation; no note generator or gate.' if rolling else 'Distinct offbeat body/sub layers; short notes and velocity accents.')
            patch['tags']=category+', Bass, Unsequenced, '+('Clean, Sub' if is_sub else 'Tight, Rolling' if rolling else 'Offbeat, Punchy')
        elif category=='Choir Stabs':
            i=counters.get(category,0);counters[category]=i+1;source,octave,mono,decay,sustain,release,cut,verb=choir_rows[i]
            p.update(source=source,sampleLoop=0,sampleReverse=0,sampleStart=0,sampleEnd=1,sampleRoot=72 if source==30 else 60,masterOctave=octave,voiceMode=mono,glideTime=0,osc2Enabled=0,mix=0,subLevel=0,noiseLevel=0,osc1Unison=1,osc2Unison=1,osc1Warp=0,osc2Warp=0,osc1WarpAmount=0,osc2WarpAmount=0,vibratoEnabled=0,lfodepth=0,motionDepth=0,pitchEnvAmount=0,analogueCharacter=0,analogueLabEnabled=0,filterMode=4,cutoff=cut,res=.04,filterEnvAmount=0,filterVelocity=.15,filterInputDrive=0,drive=0,distEnabled=0,chorusEnabled=0,phaserEnabled=0,flangerEnabled=0,convolverEnabled=0,eq0Frequency=105 if octave<0 else 150,eq3Gain=-.6,eq4Gain=.5,ampVelocity=.45,gain=.4)
            p.update(env(.0015,decay,sustain,release));p.update(space(0,verb,0));p.update(reverbSize=.46,reverbDamping=.48)
            for key in ('lfo','grain_resource'):patch.pop(key,None)
            patch['play']='Owner-supplied choir one-shot. Tight, straight vocal stab; no added detuning, vibrato, chorus or motion. Play single hits or short chords.'
            patch['tags']='Choir Stabs, Choir, User Recordings, Straight, '+('Deep' if octave<0 else 'Bright' if i in (3,6,8) else 'Rave')
        elif category=='Arps and Sequences' and patch['name']=='Choir Spiral':
            p.update(source=31,osc2Enabled=0,mix=0,sampleLoop=0,chorusEnabled=0,vibratoEnabled=0,lfodepth=0,motionDepth=0,analogueLabEnabled=0,analogueCharacter=0,filterMode=4,cutoff=11000,filterEnvAmount=0)
