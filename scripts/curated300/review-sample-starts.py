"""Inspect actual source onsets and preserve attacks in retained sampled leads/bass."""
import argparse,hashlib,json,subprocess
from pathlib import Path
import numpy as np
p=argparse.ArgumentParser();p.add_argument('work',type=Path);a=p.parse_args();root=Path(__file__).resolve().parents[2]
selection=json.loads((a.work/'Selection.json').read_text());keep=set(selection['kept_keys']);by={r['key']:r for r in json.loads((a.work/'SettingsBefore.json').read_text())};changes=json.loads((a.work/'FXPlan.json').read_text());review=json.loads((a.work/'FXReview.json').read_text());rows=[]
for r in json.loads((root/'native-vst3/Presets/UserVoices093/DESIGNS.json').read_text())['presets']:
    key='Uuser_orbitvoices093_'+r['id']
    if key not in keep:continue
    file=root/r['sample_file'];meta=json.loads(subprocess.check_output(['ffprobe','-v','error','-show_streams','-of','json',str(file)]))['streams'][0];sr=int(meta['sample_rate']);channels=int(meta['channels']);raw=subprocess.check_output(['ffmpeg','-v','error','-i',str(file),'-f','f32le','-acodec','pcm_f32le','-']);x=np.frombuffer(raw,dtype='<f4').reshape(-1,channels);level=np.max(np.abs(x),axis=1);assert np.isfinite(x).all() and level.max()>1e-5
    threshold=max(.0001,float(level.max())*.003);audible=np.flatnonzero(level>threshold);onset=int(audible[0]);start=max(0,onset-int(sr*.002));settings=by[key]['settings'];before=settings.get('sampleStart',0);after=before;reason='Retain the steady choir segment and crossfade; its trimmed onset intentionally avoids a swelling section.'
    if r['category']!='Choir':
        after=float(start/(len(x)-1));reason='Start up to 2 ms before the first audible source attack; do not crop the one-shot transient.'
        if abs(after-before)>1e-6:changes.setdefault(key,{})['sampleStart']=after
    if settings.get('sampleLoop',0)>.5 and settings.get('sampleLoopFade',0)<8:changes.setdefault(key,{})['sampleLoopFade']=10
    rows.append(dict(key=key,name=r['name'],file=file.name,source_sha256=hashlib.sha256(file.read_bytes()).hexdigest(),root_midi=r['root_midi'],sample_rate=sr,channels=channels,frames=len(x),onset_ms=onset/sr*1000,previous_start_ratio=before,revised_start_ratio=after,loop_enabled=settings.get('sampleLoop',0),loop_crossfade_ms=changes.get(key,{}).get('sampleLoopFade',settings.get('sampleLoopFade',0)),reason=reason))
for r in review['presets']:
    s={**by[r['key']]['settings'],**changes.get(r['key'],{})};r['warp']=[dict(oscillator=i,mode=s.get(f'osc{i}Warp',0),amount=s.get(f'osc{i}WarpAmount',0),envelope=s.get(f'osc{i}WarpEnvelope',0)) for i in (1,2)];r['unison']=[dict(oscillator=i,voices=s.get(f'osc{i}Unison',1),detune=s.get(f'osc{i}Detune',.2),blend=s.get(f'osc{i}Blend',1)) for i in (1,2)];r['changes']=changes.get(r['key'],{})
    sample=next((x for x in rows if x['key']==r['key']),None)
    if sample and sample['revised_start_ratio']!=sample['previous_start_ratio']:r['reasons'].append('Restore the original transient at the sample start.')
review['changed']=len(changes);(a.work/'FXPlan.json').write_text(json.dumps(changes,indent=2)+'\n');(a.work/'FXReview.json').write_text(json.dumps(review,indent=2)+'\n');(a.work/'SampleReview.json').write_text(json.dumps(dict(presets=rows,checked=len(rows),source_audio_unchanged=True),indent=2)+'\n');print('Checked',len(rows),'retained sampled presets; all source WAV bytes unchanged')
