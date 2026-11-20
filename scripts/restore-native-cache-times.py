"""Keep unchanged native inputs older than restored compiler outputs on CI.

Git checkout timestamps every file as new; that defeats the MSBuild object cache.
Use each input's actual last-commit time. Changed sources still rebuild normally.
"""
import os
from pathlib import Path
import subprocess
root = Path(__file__).resolve().parents[1]
log = subprocess.check_output(['git', '-c', 'core.quotepath=false', 'log',
    '--format=__COMMIT__%ct', '--name-only', '--', 'native-vst3', 'scripts'],
    cwd=root, text=True, encoding='utf-8')
seen = set()
timestamp = None
count = 0
for line in log.splitlines():
    if line.startswith('__COMMIT__'):
        timestamp = int(line[len('__COMMIT__'):])
    elif line and timestamp is not None and line not in seen:
        seen.add(line)
        path = root / line
        if path.is_file() and not path.is_symlink():
            os.utime(path, (timestamp, timestamp))
            count += 1
print(f'Restored last-commit times for {count} native/cache inputs')
