"""Archive validation and stable-key metadata/audio import regression checks."""
import importlib.util,json,struct,tempfile,unittest,zipfile
from pathlib import Path
spec=importlib.util.spec_from_file_location('factory_edits',Path(__file__).resolve().parents[1]/'import-factory-edits.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
class FactoryEdits(unittest.TestCase):
 def test_import_and_reject(self):
  with tempfile.TemporaryDirectory() as tmp:
   root=Path(tmp);out=root/'Overrides';xml=b'<PARAMETERS creator="Alice"><PARAM id="cutoff" value="1234"/></PARAMETERS>';data=struct.pack('<II',0x21324356,len(xml))+xml+b'\0'
   row=dict(factory_key='F1001',name='My Acid',creator='Alice',tags='Acid, Custom',file='F1001.sipreset',metadata_only=False)
   def make(row,extra=None):
    file=root/'edits.zip'
    with zipfile.ZipFile(file,'w') as z:
     z.writestr('FactoryEdits.json',json.dumps(dict(format='SPACE_INFINITY_FACTORY_EDITS_V1',presets=[row])))
     if row['file']:z.writestr(row['file'],data)
     if extra:z.writestr(extra,b'bad')
    return file
   self.assertEqual(m.import_edits(make(row),out),1);self.assertEqual((out/'F1001.sipreset').read_bytes(),data)
   metadata=dict(row,file='',metadata_only=True,tags='Acid, Updated');m.import_edits(make(metadata),out)
   self.assertEqual((out/'F1001.sipreset').read_bytes(),data);self.assertEqual(json.loads((out/'metadata.json').read_text())[0]['tags'],'Acid, Updated')
   before=(out/'metadata.json').read_bytes()
   with self.assertRaises(ValueError):m.import_edits(make(row,'../escape'),out)
   self.assertEqual((out/'metadata.json').read_bytes(),before)
   with self.assertRaises(ValueError):m.import_edits(make(dict(row,factory_key='F9999')),out)
   with self.assertRaises(ValueError):m.import_edits(make(dict(row,file='other.sipreset')),out)
if __name__=='__main__':unittest.main()
