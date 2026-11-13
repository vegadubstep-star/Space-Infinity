#!/usr/bin/env python3
"""Audit freshly rendered native auditions without changing any sound settings."""
import argparse,concurrent.futures,json,math,subprocess
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__);p.add_argument('solo',type=Path);p.add_argument('chord',type=Path);p.add_argument('output',type=Path);a=p.parse_args();root=Path(__file__).resolve().parents[1];bank=json.loads((root/'native-vst3/Presets/TranceFactoryBank.json').read_text());chord={r['name']:r for r in json.loads(a.chord.read_text())};solo={r['name']:r for r in json.loads(a.solo.read_text())}
def measure(patch):
 file=a.solo.parent/'Previews'/(patch['name']+'.wav');r=subprocess.run(['ffmpeg','-hide_banner','-nostats','-i',str(file),'-af','loudnorm=I=-28:TP=-2:LRA=11:print_format=json','-f','null','-'],capture_output=True,text=True,check=True);value=json.JSONDecoder().raw_decode(r.stderr[r.stderr.rfind('{'):])[0];lufs=float(value['input_i']);assert math.isfinite(lufs),patch['name'];return dict(name=patch['name'],category=patch['category'],lufs=lufs,solo_peak=solo[patch['name']]['peak'],chord_raw_peak=chord[patch['name']]['raw_peak'],trim=patch['settings']['presetTrim'])
with concurrent.futures.ThreadPoolExecutor(max_workers=6) as pool:rows=list(pool.map(measure,bank['presets']))
low=min(x['lufs'] for x in rows);high=max(x['lufs'] for x in rows);peak=max(x['chord_raw_peak'] for x in rows);result=dict(presets=len(rows),minimum_lufs=low,maximum_lufs=high,spread_lu=high-low,max_chord_raw_peak=peak,rows=rows);a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({k:v for k,v in result.items() if k!='rows'},indent=2));assert len(rows)==160 and high-low<2.5 and peak<.83,'Loudness/headroom audit needs attention'
