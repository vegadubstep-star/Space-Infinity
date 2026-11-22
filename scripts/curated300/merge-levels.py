"""Reuse proven measurements only for unchanged audio states; merge fresh edits."""
import argparse,copy,json,math
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('work',type=Path);p.add_argument('phase',choices=['before','after']);a=p.parse_args();keep=set(json.loads((a.work/'Selection.json').read_text())['kept_keys']);changes=json.loads((a.work/'FXPlan.json').read_text());target=-22.5
if a.phase=='before':
    old=json.loads(Path('/workspace/work/preset094/Final.json').read_text())['presets'];rows={r['key']:copy.deepcopy(r) for r in old if r['key'] in keep}
    fresh=set()
    for name in ('levelsA','levelsB','levelsC','levelsD'):
        for r in json.loads((a.work/name/'Measurements.json').read_text())['presets']:
            if r['key'] in keep:rows[r['key']]=r;fresh.add(r['key'])
    assert set(changes)<=fresh and set(rows)==keep
    settings=json.loads((a.work/'RenderPlan.json').read_text());outliers=[]
    for k,r in rows.items():
        if k not in changes:continue
        m=r['phrase'];safe=20*math.log10(min(.95/m['pre_safety_peak'],.93/m['true_peak_4x']));correction=min(target-m['lufs'],safe);settings[k]['presetReferenceGain']=round(r['reference_gain']+correction,6)
        if m['lufs']+correction<target-1.001:outliers.append(dict(key=k,name=r['name'],lufs=m['lufs'],safe_correction=safe))
    (a.work/'GainOutliers.json').write_text(json.dumps(outliers,indent=2)+'\n');assert not outliers,outliers
    (a.work/'FinalSettings.json').write_text(json.dumps(settings,indent=2)+'\n');(a.work/'Baseline300.json').write_text(json.dumps(dict(presets=list(rows.values())),indent=2)+'\n');print('300 evidence rows merged;',len(fresh),'fresh baseline renders;',len(changes),'edited sounds ready for final bank render')
else:
    baseline={r['key']:r for r in json.loads((a.work/'Baseline300.json').read_text())['presets']};rows={k:copy.deepcopy(r) for k,r in baseline.items() if k not in changes}
    for r in rows.values():r['reused_unchanged_audio']=True
    fresh=[]
    for name in ('finalA','finalB'):
        for r in json.loads((a.work/name/'Measurements.json').read_text())['presets']:rows[r['key']]=r;fresh.append(r['key'])
    assert set(fresh)==set(changes) and len(fresh)==len(changes) and set(rows)==keep
    (a.work/'Final300.json').write_text(json.dumps(dict(presets=list(rows.values())),indent=2)+'\n');print('300 verified states:',len(fresh),'fresh final renders;',300-len(fresh),'unchanged audio states with byte/parameter parity')
