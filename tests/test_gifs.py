from PIL import Image, ImageDraw,ImageSequence
import pathlib,subprocess,numpy as np
root=pathlib.Path(__file__).resolve().parent; dest=root/'fixtures';dest.mkdir(exist_ok=True)
cases=[]
for w,h in [(96,64),(64,96),(101,101),(700,520)]:
 for disp in [1,2,3]:
  frames=[]
  for i in range(4):
   im=Image.new('RGBA',(w,h),(0,0,0,0));d=ImageDraw.Draw(im)
   d.rectangle((i*w//6,h//4,i*w//6+w//4,h*3//4),fill=['red','lime','blue','yellow'][i])
   d.rectangle((0,0,5,5),fill='white');frames.append(im)
  name=f'{w}x{h}-disp{disp}';p=dest/(name+'.gif');frames[0].save(p,save_all=True,append_images=frames[1:],duration=[50,100,150,200],loop=0,disposal=disp,optimize=True);cases.append(p)
# Nontransparent static frame, including a source larger than output.
for w,h in [(240,160),(1,1),(1600,900)]:
 a=np.zeros((h,w,3),dtype=np.uint8);a[:,:,0]=np.arange(w,dtype=np.uint16)[None,:]%256;a[:,:,1]=120;a[:,:,2]=240
 p=dest/f'static-{w}x{h}.gif';Image.fromarray(a).save(p);cases.append(p)
for p in cases:
 prefix=dest/p.stem
 r=subprocess.run([str(root/'decoder_test'),str(p),str(prefix)],capture_output=True,text=True,timeout=15)
 if r.returncode:raise RuntimeError(f'{p.name}: {r.stdout} {r.stderr}')
 gif=Image.open(p);w,h=gif.size;side=min(w,h)
 xs=((w-side)*480+(2*np.arange(480)+1)*side)//960
 ys=((h-side)*480+(2*np.arange(480)+1)*side)//960
 for i,frame in enumerate(ImageSequence.Iterator(gif)):
  rgba=frame.convert('RGBA');black=Image.new('RGBA',rgba.size,(0,0,0,255));black.alpha_composite(rgba)
  rgb=np.array(black.convert('RGB')).astype(np.uint16);expected=((rgb[:,:,0]&248)<<8)|((rgb[:,:,1]&252)<<3)|(rgb[:,:,2]>>3)
  expected=expected[ys[:,None],xs[None,:]];actual=np.fromfile(str(prefix)+f'-{i}.raw',dtype=np.uint16).reshape(480,480)
  bad=np.count_nonzero(actual!=expected)
  if bad:raise AssertionError(f'{p.name} frame {i}: {bad} mismatched pixels')
 print('PASS',p.name,flush=True)
print('All rendered frames match Pillow reference after centre-crop and RGB565 conversion.')
