#!/usr/bin/env python3
"""Measure native audition WAVs with FFmpeg BS.1770 loudness; write bounded preset trims."""
import argparse,concurrent.futures,json,math,subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('profile',type=Path);parser.add_argument('auditions',type=Path);parser.add_argument('--target-lufs',type=float,default=-28);parser.add_argument('--headroom-profile',type=Path);args=parser.parse_args()
bank=json.loads((root/'native-vst3/Presets/TranceFactoryBank.json').read_text());profile={r['name']:r for r in json.loads(args.profile.read_text())}
headroom={r['name']:r for r in json.loads(args.headroom_profile.read_text())} if args.headroom_profile else profile
def measure(p):
 result=subprocess.run(['ffmpeg','-hide_banner','-nostats','-i',str(args.auditions/(p['name']+'.wav')),'-af',f'loudnorm=I={args.target_lufs}:TP=-2:LRA=11:print_format=json','-f','null','-'],capture_output=True,text=True,check=True)
 data=json.JSONDecoder().raw_decode(result.stderr[result.stderr.rfind('{'):])[0];loudness=float(data['input_i']);peak=headroom[p['name']]['raw_peak']
 if not math.isfinite(loudness):raise ValueError('Silent or invalid audition: '+p['name'])
 trim=profile[p["name"]].get("preset_trim",0)+min(args.target_lufs-loudness,20*math.log10(.78/max(peak,1e-8)))
 return dict(id=p['id'],name=p['name'],design_hash=p['design_hash'],input_lufs=loudness,presetTrim=round(max(-24,min(24,trim)),5),raw_peak=peak)
with concurrent.futures.ThreadPoolExecutor(max_workers=6) as pool:rows=list(pool.map(measure,bank['presets']))
(root/'scripts/trance-bank-loudness.json').write_text(json.dumps(rows,indent=2)+'\n');print('Measured',len(rows),'presets. Regenerate bank with design-trance-bank.py, rebuild and verify rendered loudness/headroom.')
