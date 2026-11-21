"""Verify actual native renders of every baked installed preset against its source bytes."""
import argparse,hashlib,json,math,zipfile
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__);p.add_argument('baseline',type=Path);p.add_argument('final',type=Path);p.add_argument('--renderer',type=Path,required=True);a=p.parse_args();root=Path(__file__).resolve().parents[2]
catalog=json.loads((root/'native-vst3/Presets/Installed094.json').read_text());before={r['key']:r for r in json.loads(a.baseline.read_text())['presets']};rows=json.loads(a.final.read_text())['presets'];assert len(rows)==len(before)==len(catalog['settings'])==421
hashes={};banks={};history=[]
for folder,name in [('Heritage200','SPACE_INFINITY_Heritage_200.zip'),('Lush20','SPACE_INFINITY_Lush_Trance_20.zip'),('UserVoices093','SPACE_INFINITY_Orbit_Voices.zip')]:
 data=(root/'native-vst3/Presets'/folder/name).read_bytes();sha=hashlib.sha256(data).hexdigest();assert sha==catalog['banks'][name]['sha256'];banks[name]=sha
 with zipfile.ZipFile(root/'native-vst3/Presets'/folder/name) as z:
  assert z.testzip() is None;manifest=json.loads(z.read('Bank.json'));assert manifest['minimum_version']=='0.9.4'
  for r in manifest['presets']:
   assert hashlib.sha256(z.read(r['file'])).hexdigest()==r['sha256'];key='Uuser_'+manifest['id']+'_'+r['id'];hashes[key]=r['sha256']
   if key in before:assert before[key]['file_sha256']==r['previous_sha256'] or before[key]['file_sha256'] in r['previous_sha256s'];history.append(key)
errors=[];measurements=[];target=catalog['target_lufs']
for row in rows:
 key=row['key'];expected=catalog['settings'][key];base=before[key];main=row['phrase'];stress=row['stress']
 if row['file_sha256'] and row['file_sha256']!=hashes[key]:errors.append(key+': rendered preset bytes differ from shipping bank')
 if abs(row['reference_gain']-expected['presetReferenceGain'])>.0001:errors.append(key+': static reference gain differs')
 for name,m in [('default',main),('stress',stress)]:
  if not all(math.isfinite(v) for v in m.values()) or m['rms']<=1e-6:errors.append(key+': invalid/silent '+name)
 if abs(main['lufs']-target)>1.001:errors.append(key+': loudness differs by more than 1 LU')
 if main['pre_safety_peak']>.952 or main['true_peak_4x']>.941:errors.append(key+': insufficient default clean headroom')
 if max(main['peak'],stress['peak'])>=1 or stress['true_peak_4x']>=1:errors.append(key+': clipping/true-peak overshoot')
 measurements.append(dict(key=key,name=row['name'],category=row['category'],source_sha256=row['file_sha256'],before_lufs=base['phrase']['lufs'],lufs=main['lufs'],difference_from_target_lu=main['lufs']-target,static_reference_gain_db=row['reference_gain'],default=main,stress=stress,baked_settings=expected,effects_polish=base['polish']))
assert len({r['key'] for r in rows})==421
old=[r['phrase']['lufs'] for r in before.values()];levels=[r['phrase']['lufs'] for r in rows]
doc=dict(version='0.9.4',renderer_sha256=hashlib.sha256(a.renderer.read_bytes()).hexdigest(),presets_measured=421,factory_presets=160,extra_presets=261,sampled_orbit_presets=48,installed_entries_including_init=422,init_excluded_from_loudness_matching=True,target_lufs=target,tolerance_lu=1,before_lufs_range=[min(old),max(old)],after_lufs_range=[min(levels),max(levels)],maximum_deviation_lu=max(abs(v-target) for v in levels),maximum_default_true_peak_dbfs=20*math.log10(max(r['phrase']['true_peak_4x'] for r in rows)),maximum_stress_true_peak_dbfs=20*math.log10(max(r['stress']['true_peak_4x'] for r in rows)),maximum_default_pre_safety_peak=max(r['phrase']['pre_safety_peak'] for r in rows),maximum_stress_pre_safety_peak=max(r['stress']['pre_safety_peak'] for r in rows),polished_presets=catalog['polished_presets'],bank_hashes=banks,factory_json_sha256=hashlib.sha256((root/'native-vst3/Presets/TranceFactoryBank.json').read_bytes().replace(b'\r\n',b'\n')).hexdigest(),text_sha256_line_endings='LF',known_shipped_update_paths_verified=len(history),method=dict(loudness='ITU-R BS.1770 K weighting; stereo; 400 ms windows, 100 ms hops; -70 LUFS absolute/-10 LU relative gating',meter_crosscheck='FFmpeg ebur128 sine and gated-burst fixtures agree within 0.04 LU',sample_rate=48000,true_peak_oversampling=4,phrase='6.4 s: three 180 ms notes then a 4 s hold and 1 s tail, velocity110; bass/acid E2, others C4',stress='4 s, three registers -12/0/+12, full-velocity single bass/acid or four-note chords; all six performance macros at maximum after2s',gain='Static post-FX reference gain; existing macro gain compensation, envelopes and tone retained',effects='Small room only on21 previously dry keyboard/organ sounds; basses kept dry'),limits='Matching applies to the documented reference phrase. Register, MIDI density, velocity, macros and user edits change perceived loudness. Clean output safety remains active for stress peaks. Historical 0.9.3 audits describe the earlier bank revision.',errors=errors,presets=measurements)
out=root/'docs/audit/INSTALLED_BANK_094_LEVELS.json';out.write_text(json.dumps(doc,indent=2)+'\n');print(json.dumps({k:doc[k] for k in ['presets_measured','after_lufs_range','maximum_deviation_lu','maximum_default_true_peak_dbfs','maximum_stress_true_peak_dbfs','errors']},indent=2));raise SystemExit(bool(errors))
