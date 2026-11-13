#!/usr/bin/env python3
"""Import a user-exported factory-edit ZIP into the next compiled bank, by stable key."""
import argparse, json, os, re, struct, zipfile
from pathlib import Path
import xml.etree.ElementTree as ET

def import_edits(archive, destination):
    destination=Path(destination); pending={}; rows=[]; limit=64*1024*1024
    with zipfile.ZipFile(archive) as z:
        infos=z.infolist()
        if len({i.filename for i in infos})!=len(infos) or any('/' in i.filename or '\\' in i.filename or i.file_size>limit for i in infos):raise ValueError('Invalid ZIP entries')
        if sum(i.file_size for i in infos)>512*1024*1024:raise ValueError('ZIP exceeds bank import limit')
        manifest=json.loads(z.read('FactoryEdits.json'))
        if manifest.get('format')!='SPACE_INFINITY_FACTORY_EDITS_V1':raise ValueError('Unsupported edit archive')
        seen=set()
        for source in manifest['presets']:
            key=source.get('factory_key','')
            if not re.fullmatch(r'F\d{4}',key) or not 1001<=int(key[1:])<=1160 or key in seen:raise ValueError('Invalid/duplicate factory key')
            seen.add(key);row={'factory_key':key}
            for field,maxlen in [('name',64),('creator',64),('tags',512)]:
                value=source.get(field,'')
                if not isinstance(value,str) or len(value)>maxlen or (field=='name' and not value.strip()):raise ValueError('Invalid '+field)
                row[field]=value.strip()
            filename=source.get('file','')
            if filename:
                if filename!=key+'.sipreset':raise ValueError('Unexpected preset filename')
                data=z.read(filename)
                if len(data)<10 or len(data)>limit:raise ValueError('Invalid preset size')
                magic,length=struct.unpack('<II',data[:8])
                if magic!=0x21324356 or length<=0 or length>len(data)-8:raise ValueError('Invalid JUCE preset header')
                xml=ET.fromstring(data[8:8+length].rstrip(b'\0'))
                if xml.tag!='PARAMETERS':raise ValueError('Invalid preset root')
                pending[filename]=data
            elif not source.get('metadata_only'):raise ValueError('Missing sound file')
            row['metadata_only']=not bool(filename);rows.append(row)
    destination.mkdir(parents=True,exist_ok=True)
    metadata=destination/'metadata.json';existing=json.loads(metadata.read_text()) if metadata.exists() else []
    merged={r['factory_key']:r for r in existing};merged.update({r['factory_key']:r for r in rows})
    # Validate the complete archive before touching the repository. Keep replaced overrides.
    backups=destination/'Backups'
    for name,data in pending.items():
        target=destination/name
        if target.exists():
            backups.mkdir(exist_ok=True);(backups/(name+'.previous')).write_bytes(target.read_bytes())
        tmp=target.with_suffix('.tmp');tmp.write_bytes(data);os.replace(tmp,target)
    tmp=metadata.with_suffix('.tmp');tmp.write_text(json.dumps(list(merged.values()),indent=2)+'\n');os.replace(tmp,metadata)
    return len(rows)
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('zip',type=Path);p.add_argument('--destination',type=Path,default=Path(__file__).resolve().parents[1]/'native-vst3/Presets/Overrides');a=p.parse_args();print('Imported',import_edits(a.zip,a.destination),'factory edits. Regenerate the bank and build before publishing.')
