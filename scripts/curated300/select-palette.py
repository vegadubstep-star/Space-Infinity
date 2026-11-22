"""Reduce the shipped palette by closest rendered neighbours within musical roles.

Uses every preset at three registers: whole-note spectrum, attack spectrum and
amplitude contour. This is feature-based curation, not a claim of perceptual
identity. Native files/IDs remain recoverable and user replacements are exempt.
"""
import argparse, hashlib, json
from pathlib import Path
import numpy as np

ROOT=Path(__file__).resolve().parents[2]
QUOTAS={'Bass':58,'Lead':80,'Pluck':28,'Pads':40,'Arps & Sequences':25,
        'Choir & Voices':16,'Strings':12,'Piano & Keys':16,'Acid':8,
        'Mallets':5,'Organ':4,'Percussion':3,'Textures':5}

def category(c):
    c=c.lower()
    for word,role in [('bass','Bass'),('lead','Lead'),('pluck','Pluck'),('pad','Pads'),
                      ('string','Strings'),('piano','Piano & Keys'),('keys','Piano & Keys'),
                      ('arp','Arps & Sequences'),('sequence','Arps & Sequences'),('choir','Choir & Voices')]:
        if word in c:return role
    return {'voices':'Choir & Voices','motion':'Arps & Sequences'}.get(c,c.title())

def unit(v):
    v=np.asarray(v,dtype=float)
    return v/max(1e-20,np.linalg.norm(v))

def comparison(a,b):
    result=[]
    for x,y in zip(a['registers'],b['registers']):
        s=float(unit(x['spectrum'])@unit(y['spectrum']))
        t=float(unit(x['attack_spectrum'])@unit(y['attack_spectrum']))
        e=float(unit(x['envelope'])@unit(y['envelope']))
        result.append(dict(spectrum=s,attack=t,envelope=e,score=.45*s+.2*t+.35*e))
    return dict(score=float(np.mean([x['score'] for x in result])),minimum_register_score=min(x['score'] for x in result),registers=result)

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('work',type=Path);a=p.parse_args()
    rows=[]
    for name in ('profiles','profileA','profileB'):
        rows+=json.loads((a.work/name/'Profiles.json').read_text())['presets']
    assert len(rows)==421 and len({r['key'] for r in rows})==421
    rows.sort(key=lambda r:r['key']);by={r['key']:r for r in rows}
    for r in rows:
        assert len(r['registers'])==3
        assert all(x['rms']>1e-6 for x in r['registers']),r['name']
        r['role']=category(r['category'])
    keep=set(by);removals=[];pairs={}
    for role,quota in QUOTAS.items():
        group=[r['key'] for r in rows if r['role']==role]
        assert len(group)>=quota,(role,len(group),quota)
        for i,k in enumerate(group):
            for j in group[:i]:pairs[tuple(sorted((k,j)))]=comparison(by[k],by[j])
        while len(group)>quota:
            pair=max(((pairs[tuple(sorted((k,j)))]['score'],k,j) for i,k in enumerate(group) for j in group[:i]))
            _,x,y=pair
            # A denser neighbourhood loses its less distinctive representative.
            def redundancy(k):
                scores=sorted([pairs[tuple(sorted((k,j)))]['score'] for j in group if j!=k],reverse=True)
                return sum(scores[:min(5,len(scores))])/min(5,len(scores))
            victim=max((x,y),key=lambda k:(redundancy(k),k));group.remove(victim);keep.remove(victim)
            removals.append(victim)
    # Labels must not conceal a duplicate: repair very close cross-role pairs
    # with a more distinctive removed alternative from the same role budget.
    repairs=[]
    def same_pitch(x,y):return (by[x]['role'] in ('Bass','Acid'))==(by[y]['role'] in ('Bass','Acid'))
    def metric(x,y):
        pair=tuple(sorted((x,y)))
        if pair not in pairs:pairs[pair]=comparison(by[x],by[y])
        return pairs[pair]['score']
    for iteration in range(20):
        collisions=[(metric(x,y),x,y) for i,x in enumerate(sorted(keep)) for y in sorted(keep)[:i] if by[x]['role']!=by[y]['role'] and same_pitch(x,y) and metric(x,y)>=.99]
        if not collisions:break
        score,x,y=max(collisions);options=[]
        for victim in (x,y):
            remaining=keep-{victim}
            for candidate in set(by)-keep:
                if by[candidate]['role']!=by[victim]['role']:continue
                peak=max(metric(candidate,k) for k in remaining if same_pitch(candidate,k))
                options.append((peak,victim,candidate))
        peak,victim,candidate=min(options)
        assert peak<score-.001, 'No distinct cross-role replacement available'
        keep.remove(victim);keep.add(candidate);removals.remove(candidate);removals.append(victim)
        repairs.append(dict(retired_key=victim,retired_name=by[victim]['name'],restored_key=candidate,restored_name=by[candidate]['name'],cross_role_key=y if victim==x else x,cross_role_name=by[y if victim==x else x]['name'],previous_similarity=score,replacement_maximum_similarity=peak))
    assert len(keep)==300 and len(removals)==121
    retired=[]
    for k in removals:
        choices=[j for j in keep if by[j]['role']==by[k]['role']]
        repair=next((x for x in repairs if x['retired_key']==k),None)
        nearest=repair['cross_role_key'] if repair else max(choices,key=lambda j:pairs[tuple(sorted((k,j)))]['score']);match=pairs[tuple(sorted((k,nearest)))]
        retired.append(dict(key=k,name=by[k]['name'],category=by[k]['category'],role=by[k]['role'],sha256=by[k]['source_sha256'],kept_key=nearest,kept_name=by[nearest]['name'],reason='Closest redundant variant in a role-balanced 300-sound palette; attack, sustained spectrum and contour compared at three registers.',comparison=match))
    nearest=[]
    for k in sorted(keep):
        choices=[j for j in keep if j!=k and by[j]['role']==by[k]['role']]
        j=max(choices,key=lambda j:pairs[tuple(sorted((k,j)))]['score']);nearest.append(dict(key=k,name=by[k]['name'],role=by[k]['role'],nearest_key=j,nearest_name=by[j]['name'],**pairs[tuple(sorted((k,j)))]))
    result=dict(renderer_sha256=hashlib.sha256((a.work/'ReviewPalette').read_bytes()).hexdigest(),registers=[-12,0,12],reference_notes='Bass/acid C2; other roles C4; each transposed by the register offset',version='0.9.4',revision='Curated 300',input_presets=421,active_presets=300,retired_count=121,cross_role_repairs=repairs,quotas=QUOTAS,kept_keys=sorted(keep),retired=retired,nearest_retained=nearest,method='All 421 current native presets rendered at three registers; cosine features for whole-note spectrum, first 213ms spectrum and 96-point amplitude envelope, weights .45/.20/.35. Closest redundant variants removed within musical-role budgets.',limitations='Feature similarity is not perceptual identity. Different MIDI, macros, sound systems and taste can change perceived similarity. Role budgets deliberately also remove less distinctive alternatives to reach 300.',profile_sha256s={name:hashlib.sha256((a.work/name/'Profiles.json').read_bytes()).hexdigest() for name in ('profiles','profileA','profileB')})
    (a.work/'Selection.json').write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps(dict(active=len(keep),removed=len(retired),roles=QUOTAS,nearest=[{k:r[k] for k in ('name','nearest_name','score')} for r in sorted(nearest,key=lambda r:r['score'],reverse=True)[:20]]),indent=2))
