"""Shared fingerprints for measured register, cutoff and IR-startup headroom."""
import hashlib,json
from pathlib import Path

def recipe_hash(row):
 settings={k:v for k,v in row['settings'].items() if k not in ('gain','presetTrim')}
 return hashlib.sha256(json.dumps(dict(settings=settings,modulation=row.get('macros',row.get('modulation',[])),lfo=row.get('lfo'),wavetable=row.get('wavetable')),sort_keys=True).encode()).hexdigest()

def references():
 path=Path(__file__).with_name('track-ready-headroom-references.json')
 return json.loads(path.read_text()) if path.exists() else {}

def reference_peak(row,reference):
 if reference['recipe_sha256']!=recipe_hash(row):raise ValueError('Stale headroom reference '+row['name'])
 return reference['pre_master_peak']*row['settings']['gain']*10**(row['settings'].get('presetTrim',0)/20)
