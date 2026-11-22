"""Review retained effects and create conservative, role-specific clarity edits."""
import argparse,json
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('work',type=Path);a=p.parse_args()
selection=json.loads((a.work/'Selection.json').read_text());by={r['key']:r for r in json.loads((a.work/'SettingsBefore.json').read_text())};changes={};review=[]
for key in selection['kept_keys']:
    r=by[key];s=r['settings'];role=next(x['role'] for x in selection['nearest_retained'] if x['key']==key);edit={};reasons=[]
    verb=s.get('reverbEnabled',0)>.5 and s.get('reverb',0)>0
    delay=s.get('delayEnabled',0)>.5
    def setv(k,v):
        if abs(s.get(k,0)-v)>1e-5:edit[k]=v
    if verb:
        if role=='Lead' and s.get('reverb',0)>.18:
            setv('reverb',.16);reasons.append('Reduce the long return behind the lead transient.')
        if role=='Pluck' and s.get('reverb',0)>.20:
            setv('reverb',.19);reasons.append('Clear space between articulated pluck hits while retaining the programmed open-lead macro.')
        if role=='Pads' and s.get('reverb',0)>.32:
            setv('reverb',.32);reasons.append('Keep the airy tail without letting it dominate the sustained pad.')
        if role in ('Lead','Pluck','Pads','Strings','Choir & Voices','Arps & Sequences') and s.get('reverb',0)>.18 and s.get('reverbWetLowGain',0)>-3:
            setv('reverbWetLowFreq',280 if role=='Pads' else 240);setv('reverbWetLowGain',-3);reasons.append('Low shelf on the wet reverb only; preserve the dry fundamental.')
    if delay and s.get('echoMode',0)>0 and s.get('echoMix',0)>.12 and role not in ('Bass','Acid') and s.get('delayWetLowGain',0)>-3:
        setv('delayWetLowFreq',250);setv('delayWetLowGain',-3);reasons.append('Remove low-frequency buildup in the wet delay without thinning the dry note.')
    # Keep a classic unwarped acid voice, while giving selected alternatives
    # deliberately different oscillator behaviour rather than just FX changes.
    acid_warps={'F1153':(3,.42),'F1155':(4,.035),'F1156':(2,.22)}
    if key in acid_warps:
        mode,amount=acid_warps[key];setv('osc1Warp',mode);setv('osc1WarpAmount',amount);setv('osc1WarpEnvelope',2)
        reasons.append('Filter-envelope-linked warp adds a distinct narrow-pulse / sub-FM / mirrored edge; retain a single main voice and cutoff-driven acid articulation.')
    if edit:changes[key]=edit
    review.append(dict(key=key,name=r['name'],role=role,changes=edit,reasons=reasons or ['Keep the existing effect chain and envelope; no extra processing needed for this role.'],active_effects=[x for x in ('chorus','delay','reverb','dist','comp','phaser','flanger','convolver','eq','gate','pump') if s.get(x+'Enabled',0)>.5],amp_attack=s.get('attack'),amp_decay=s.get('ampDecay'),amp_sustain=s.get('ampSustain'),amp_release=s.get('release'),preserved=['Oscillator/sample sources and root pitch','Filter and amplitude articulation','Performance macro routes','Bass dry fundamental','Static loudness compensation remeasured after effects edits']))
(a.work/'FXPlan.json').write_text(json.dumps(changes,indent=2)+'\n');(a.work/'FXReview.json').write_text(json.dumps(dict(revision='Curated 300',reviewed=300,changed=len(changes),presets=review),indent=2)+'\n');print('Reviewed 300 chains;',len(changes),'clarity edits planned')
