"""Link a review main against the already built native engine and JUCE objects."""
import argparse, shlex, subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('source',type=Path);p.add_argument('output',type=Path);a=p.parse_args()
root=Path(__file__).resolve().parents[2];build=root/'native-vst3/build-linux';flags={}
for line in (build/'CMakeFiles/SpaceInfinitySmoke.dir/flags.make').read_text().splitlines():
    if ' = ' in line:
        k,v=line.split(' = ',1);flags[k]=shlex.split(v)
a.output.parent.mkdir(parents=True,exist_ok=True);obj=a.output.with_suffix('.o')
subprocess.run(['/usr/bin/c++',*flags['CXX_DEFINES'],*flags['CXX_INCLUDES'],*flags['CXX_FLAGS'],'-c',str(a.source.resolve()),'-o',str(obj)],check=True)
args=shlex.split((build/'CMakeFiles/SpaceInfinitySmoke.dir/link.txt').read_text());args=[x for x in args if '/Tests/' not in x];args.insert(1,str(obj));args[args.index('-o')+1]=str(a.output);subprocess.run(args,cwd=build,check=True)
