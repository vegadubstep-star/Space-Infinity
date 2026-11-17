"""Link the offline bank authoring utility against the already built native synth."""
from pathlib import Path
import shlex, subprocess, argparse
p=argparse.ArgumentParser();p.add_argument('--build',type=Path,default=Path('native-vst3/build-linux'));p.add_argument('--output',type=Path,default=Path('/tmp/RenderHeritage'));a=p.parse_args();build=a.build.resolve();root=Path(__file__).resolve().parents[2]
flags={}
for line in (build/'CMakeFiles/SpaceInfinitySmoke.dir/flags.make').read_text().splitlines():
 if ' = ' in line:
  k,v=line.split(' = ',1);flags[k]=shlex.split(v)
obj=a.output.with_suffix('.o');log=a.output.with_suffix('.build.log')
with log.open('w') as output:
 subprocess.run(['/usr/bin/c++']+flags['CXX_DEFINES']+flags['CXX_INCLUDES']+flags['CXX_FLAGS']+['-std=c++17','-c',str(root/'scripts/heritage/RenderBank.cpp'),'-o',str(obj)],check=True,stdout=output,stderr=subprocess.STDOUT)
 args=shlex.split((build/'CMakeFiles/SpaceInfinitySmoke.dir/link.txt').read_text());args=[x for x in args if not ('/Tests/' in x and x.endswith('.cpp.o')) and not x.startswith('-Wl,--dependency-file=')];args[args.index('-o')+1]=str(a.output);args.insert(1,str(obj));subprocess.run(args,cwd=build,check=True,stdout=output,stderr=subprocess.STDOUT)
print(a.output)
