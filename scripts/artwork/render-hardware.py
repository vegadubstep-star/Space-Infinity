"""Original deterministic material renders. No third-party artwork or photos."""
from pathlib import Path
import hashlib,json
import numpy as np
from PIL import Image
OUT=Path(__file__).resolve().parents[2]/'native-vst3/Assets/Hardware';OUT.mkdir(parents=True,exist_ok=True)
N=1024;extent=1.30
y,x=np.mgrid[-extent:extent:complex(N),-extent:extent:complex(N)];r=np.hypot(x,y);theta=np.arctan2(y,x);rng=np.random.default_rng(741032)
# A shallow turned crown, chamfer, rubber seal, fluted barrel and bottom lip.
z=np.zeros_like(r)
z=np.where(r<1.0,.08,z);z=np.where(r<.965,.20+.04*(.965-r)/.14,z)
z=np.where(r<.825,.25,z);z=np.where(r<.785,.27,z)
z=np.where(r<.765,.32+(.765-r)*1.1,z)
z=np.where(r<.715,.38+.009*(r/.715)**2,z)
knurls=(.5+.5*np.cos(theta*56))**2
z+=np.where((r>.81)&(r<.97),.022*knurls,0)
zy,zx=np.gradient(z,2*extent/N);normal=np.stack([-zx,-zy,np.ones_like(r)],axis=-1);normal/=np.maximum(1e-8,np.linalg.norm(normal,axis=-1))[...,None]
light=np.array([-.48,-.64,.77]);light/=np.linalg.norm(light);half=light+np.array([0,0,1]);half/=np.linalg.norm(half)
nl=np.maximum(0,np.sum(normal*light,axis=-1));nh=np.maximum(0,np.sum(normal*half,axis=-1))
# Fine lathe marks and a fixed studio softbox reflection in circular brushing.
lathe=.018*np.sin(r*1700)+.009*np.sin(r*3150+.3*np.sin(theta*3));grain=rng.normal(0,.008,r.shape)
aniso=.09*np.cos(2*(theta+.73))+.025*np.cos(4*(theta+.6))
for name,tint in [('KnobSilver',(0.52,.57,.65)),('KnobViolet',(.16,.075,.25))]:
 metal=np.array(tint)[None,None,:]*(.40+.52*nl[...,None]+aniso[...,None]+lathe[...,None]+grain[...,None])
 # Broad reflection, sharply polished bevel highlights and purple room bounce.
 metal+=np.exp(-((x+y*.45+.25)/.55)**2)[...,None]*np.array([.12,.14,.17])
 rough_spec=np.power(nh,24)*.35;metal+=rough_spec[...,None]
 bevel=(r>.715)&(r<.765);metal+=bevel[...,None]*np.power(nh,65)[...,None]*.8
 metal+=np.maximum(0,x+y)[...,None]*np.array([.024,.006,.047])
 rubber=(.012+.035*nl+.009*knurls+grain*.15)[...,None]*np.array([.88,.85,1.12])
 body=np.where((r>.785)[...,None],rubber,metal)
 # Bright turned edge and a deep gasket under the crown.
 ring=(r>.757)&(r<.776);body=np.where(ring[...,None],(.026+.10*nl)[...,None]*np.array([1.0,.9,1.3]),body)
 edge=(r>.70)&(r<.715);body+=edge[...,None]*(.12+.28*nl)[...,None]
 # Fixed top-left illumination casts its shadow down and to the right.
 shadow_r=np.sqrt(((x-.095)/1.02)**2+((y-.15)/.99)**2)
 shadow=.28*np.exp(-np.maximum(0,shadow_r-.87)**2/.027)
 shadow+=.44*np.exp(-np.maximum(0,np.hypot(x-.025,y-.055)-.91)**2/.0035)
 shadow=np.clip(shadow,0,.85)
 alpha=np.clip((1-.009*(.5+.5*np.cos(theta*56))-r)*N/extent,0,1)
 rgb=np.clip(body,0,1)**(1/2.2);a=alpha+shadow*(1-alpha)
 rgb=np.where((a>0)[...,None],rgb*alpha[...,None]/np.maximum(a,1e-12)[...,None],0)
 rgba=np.dstack([rgb,a]);im=Image.fromarray(np.uint8(np.clip(rgba,0,1)*255),'RGBA').resize((512,512),Image.Resampling.LANCZOS);im.save(OUT/(name+'.png'))
# Transparent, zero-mean brushing: highlights and etched grooves rather than a
# grey overlay that washes out the coloured anodisation underneath it.
h,w=256,1024;fine=rng.normal(0,1,(h,w));rows=rng.normal(0,1,(h,1));brush=.65*rows+.35*fine
streak=np.cumsum(rng.normal(0,.012,(h,w)),axis=1);brush+=streak
rgba=np.zeros((h,w,4),dtype=np.uint8);positive=brush>0
rgba[:,:,:3]=np.where(positive[...,None],np.array([245,229,255]),np.array([4,2,9]));rgba[:,:,3]=np.uint8(np.clip(abs(brush)*42,0,95));Image.fromarray(rgba,'RGBA').save(OUT/'BrushedGrain.png')
manifest={p.name:dict(sha256=hashlib.sha256(p.read_bytes()).hexdigest(),source='Original procedural material artwork; fixed upper-left lighting') for p in sorted(OUT.glob('*.png'))};(OUT/'PROVENANCE.json').write_text(json.dumps(manifest,indent=2)+'\n');print('Rendered',len(manifest),'original hardware materials')
