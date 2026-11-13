"""Deliberate 0.6 voicing revisions, applied before design hashing/calibration."""
def refine(bank, env, space):
    acid=[(31,180,.78,5.1,.18,0,.22,.45),(32,280,.72,4.3,.25,3,.28,.60),(33,700,.40,2.6,.13,2,.40,.30),(34,340,.85,4.2,.27,6,.17,.18),(31,220,.70,5.2,.12,4,.30,.55),(35,1200,.60,2.8,.10,5,.23,.30),(32,100,.87,5.5,.35,1,.25,.55),(3,520,.60,3.2,.21,9,.20,.45),(31,330,.76,4.7,.14,2,.55,.60),(32,160,.68,4.4,.45,1,.24,.42)]
    airy=choir=acid_index=lead=0
    for patch in bank:
        p=patch['settings'];category=patch['category'];name=patch['name']
        p.update(distEnabled=0,distMix=0,presetTrim=0,midiBendRange=24)
        patch['creator']='Orbital Audio'
        tags=[category,'Trance']
        if category=='Acid':
            wave,cut,res,amount,decay,mode,drive,mix=acid[acid_index]
            p.update(wave=wave,source=0,osc2Enabled=0,osc1Unison=1,osc1Width=0,osc1Rand=0,osc1PhaseReset=1,filterMode=5,cutoff=cut,res=res,filterEnvAmount=amount,filterDecay=decay,filterSustain=0,filterVelocity=2.1,ampVelocity=.48,filterInputDrive=.04,drive=.025,glideTime=.055+acid_index*.006,voiceMode=2,distEnabled=1,distType=mode,distDrive=drive,distMix=mix,distFilterPosition=2,distFrequency=4700 if acid_index==3 else 8500,distQ=.707,distFilterMorph=0,fxOutput8=-4,eq6Frequency=12000)
            p.update(env(.0003,.20,.72,.025));p.update(space(.05 if acid_index%3 else .09,.015,0))
            if acid_index==2:p.update(filterMode=3,subLevel=.12,subEnabled=1,distFilterPosition=1,distFrequency=80,distFilterMorph=1)
            if acid_index in (4,7):p.update(osc1Warp=4,osc1WarpAmount=.065 if acid_index==4 else .16)
            if acid_index==8:p.update(fxOutput8=-6)
            patch['play']='Play your own mono notes; overlap for slides, use velocity for strong accents. Dedicated distortion adds switchable bite.'
            tags+=['303','Squelch','Mono','Unsequenced', ['Tube','Diode','Clipped','Folded'][acid_index%4]];acid_index+=1
        elif category=='Airy Pads':
            p.update(source=0,sampleLoop=0,osc1Unison=7,osc2Unison=5,osc1Detune=.10+.009*airy,osc2Detune=.065+.005*airy,osc1Blend=.72,osc2Blend=.68,osc1Width=.94,osc2Width=1,osc2Octave=1,mix=.20,wave=[0,6,3,0,2,0,3,6,1,3][airy],osc2Wave=[6,3,0,1,3,0,2,0,6,2][airy],noiseLevel=.003 if airy%2==0 else .001,cutoff=5700+airy*340,res=.07,filterMode=4,filterEnvAmount=.45,filterAttack=.85,filterDecay=2.4,filterSustain=.8,filterRelease=.9,eq0Frequency=150,eq6Frequency=11500,drive=.01,filterInputDrive=.015,osc1WarpAmount=.015,osc2Enabled=1)
            p.update(env(.65+airy*.055,1.8,1,2.4+airy*.045));p.update(space(.12,.32,.18 if airy%2==0 else .10));p.update(reverbDamping=.27,reverbWetHighGain=-1.5,reverbWetHighFreq=10000,chorusRate=.08+airy*.015)
            patch.pop('grain_resource',None)
            if name in ('Polar Choir','Ooh Dawn'):p.update(source=27 if name=='Polar Choir' else 28,sampleLoop=1,osc2Enabled=0,mix=0,cutoff=9500)
            if name=='Spiral Strings':p.update(source=23,sampleLoop=1,osc2Enabled=0,mix=0)
            if name=='Granular Stardust':p.update(source=4,grainSize=180,grainDensity=32,grainSpray=.22,grainSpread=.95);patch['grain_resource']='Factory_Choir_oh_60_wav'
            if name=='Crystalline Heaven':p.update(wave=3,osc1Unison=3,osc1Warp=4,osc1WarpAmount=.035,mix=.12)
            if name=='Fifth Heaven':p.update(osc2Semitone=7,mix=.12)
            patch['lfo']={'target':'cutoff','depth':.018,'bars':4+airy%3};tags+=['Airy','Emotional','Wide','Slow'];airy+=1
        elif category=='Choir Stabs':
            source=[27,27,27,29,28,28,29,28,27,29][choir]
            p.update(source=source,masterOctave=[0,-1,-1,1,0,-1,0,-1,0,0][choir],sampleLoop=0,filterMode=4,cutoff=[10000,7600,6800,12000,9000,7200,11000,6200,9800,8200][choir],res=.06,filterEnvAmount=.2,filterVelocity=.4,osc2Enabled=0,mix=0,drive=.015,filterInputDrive=.02,eq0Frequency=105 if choir%3==1 else 160,eq3Gain=-1,eq4Gain=.7,convolverEnabled=0,ampVelocity=.5)
            p.update(env(.002,.26+.025*choir,.30 if choir%2==0 else .12,.15+.018*choir));p.update(space(.035 if choir%2 else .07,.13+.012*choir,.035))
            if name=='Rave Congregation':p.update(distEnabled=1,distType=0,distDrive=.18,distMix=.25,fxOutput8=-2)
            if name=='Vocal Circuit':p.update(source=0,wave=10,osc2Wave=11,osc2Enabled=1,mix=.22,osc1Unison=3,filterMode=3,cutoff=1100,res=.27,filterEnvAmount=.7)
            patch['play']='Register-matched CC0 Ah/Ooh/Ee ensembles or explicitly synthetic vowel. Play short minor chords; velocity changes expression.'
            tags+=['Choir','Rave','Stab',['Ah','Ooh','Ee'][choir%3]];choir+=1
        elif category in ('Uplifting Leads','Hard Trance Leads','90s German Leads'):
            # Preserve the fast attacks while separating dry mono, FM, sync, fifth and supersaw roles.
            tags+=['Lead','Fast Attack']
            if category!='Uplifting Leads':tags+=['Hard Trance']
            p.update(chorusEnabled=int(lead%4==0),chorus=.065,echoMix=[.09,.14,.05,.17,.11][lead%5],reverb=[.10,.16,.07,.13,.18][lead%5],filterDecay=[.13,.28,.19,.37,.22][lead%5],osc1Detune=p['osc1Detune']*(.8+lead%4*.14))
            if name=='Desert Blade':p.update(wave=0,osc2Wave=1,osc2Octave=2,osc2Semitone=7,osc1Warp=0,osc1WarpAmount=0,ampDecay=6,ampSustain=0,distEnabled=1,distType=2,distDrive=.24,distMix=.25,fxOutput8=-3,chorusEnabled=0)
            if name=='Silver Alone':p.update(osc1Unison=1,osc2Enabled=0,mix=0,voiceMode=1,ampDecay=.08,ampSustain=1,release=.02,cutoff=7000,echoMix=.07,reverb=.055,chorusEnabled=0)
            if name in ('Orbital Steel','Saucer Alloy'):p.update(osc1Unison=2,osc1Warp=4,osc1WarpAmount=.12,osc2Octave=1,distEnabled=1,distType=3,distDrive=.18,distMix=.22,fxOutput8=-2)
            if name=='Ion Wire':p.update(gateRate=3,gateDepth=.85,gateAttack=2,gateRelease=5,osc1Unison=8,osc2Unison=5,osc2Octave=1)
            if name=='Komodo Breath':p.update(source=0,wave=13,osc2Wave=3,osc1Unison=2,osc2Unison=1,osc2Octave=0,mix=.16,cutoff=4200,res=.16)
            if name in ('Power District','Reptile Signal'):p.update(osc1Unison=1,osc2Unison=1,osc1Warp=3,osc1WarpAmount=.25,distEnabled=1,distType=0,distDrive=.19,distMix=.25,fxOutput8=-2)
            lead+=1
        elif category=='Arps and Sequences':
            if name=='Choir Spiral':p.update(source=29,sampleLoop=0,filterMode=4,cutoff=8500,filterEnvAmount=.3,res=.07,attack=.002,ampDecay=.25,ampSustain=.2)
            if name=='Yellow Sequence':p.update(filterMode=4,cutoff=2600,res=.24,filterEnvAmount=2.1,osc1Unison=2,osc2Unison=1,wave=10,osc2Wave=3,mix=.15)
        elif category=='Dark Pads' and name=='Obsidian Choir':p.update(source=28,masterOctave=-1,sampleLoop=1)
        if category in ('Offbeat Bass Stabs','Sub Bass Stabs','Rolling Bass'):tags+=['Bass','Unsequenced']
        if category=='Trance Plucks':tags+=['Pluck','Melodic']
        patch['tags']=', '.join(dict.fromkeys(tags))
