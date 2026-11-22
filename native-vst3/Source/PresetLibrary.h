#pragma once
#include <JuceHeader.h>
#include <juce_cryptography/juce_cryptography.h>
#include <functional>
#include <algorithm>
#include <vector>
// Message-thread file operations. Stable keys decouple saved projects from bank order.
class PresetLibrary {
public:
    static juce::String browseCategory(const juce::String& category){
        const auto c=category.toLowerCase();
        if(c.contains("bass"))return "Bass";
        if(c.contains("lead"))return "Lead";
        if(c.contains("pluck"))return "Pluck";
        if(c.contains("pad"))return "Pads";
        if(c.contains("string"))return "Strings";
        if(c.contains("piano")||c.contains("keys"))return "Piano & Keys";
        if(c.contains("arp")||c.contains("sequence")||c=="motion")return "Arps & Sequences";
        if(c.contains("choir")||c=="voices")return "Choir & Voices";
        return category;
    }
    static juce::String processLockName(const juce::File& folder,const juce::String& kind){
       #if JUCE_WINDOWS
        return "SpaceInfinity"+kind+juce::String(folder.getFullPathName().hashCode64());
       #else
        // JUCE otherwise tries /var/tmp, which may be read-only in a sandbox.
        return folder.getChildFile(".infinity-"+kind+".lock").getFullPathName();
       #endif
    }
    static juce::Result prepareProcessLock(const juce::File& folder,const juce::String& kind){
        auto r=folder.createDirectory();if(r.failed())return r;
       #if !JUCE_WINDOWS
        return juce::File(processLockName(folder,kind)).create();
       #else
        return juce::Result::ok();
       #endif
    }
    struct Entry {juce::String key,name,category,file;int factory=-1;juce::String creator,tags;bool detailsEdited=false;};
    PresetLibrary(juce::StringArray names,juce::StringArray categories,const juce::File& requested={},int retiredFactoryId=-1,int firstFactoryId=0,juce::StringArray creators={},juce::StringArray tags={})
        :factoryNames(std::move(names)),factoryCategories(std::move(categories)),retiredFactory(retiredFactoryId),firstFactory(firstFactoryId),factoryCreators(std::move(creators)),factoryTags(std::move(tags)),
         directory(requested==juce::File{}?juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("Orbital Audio/Space Infinity/Presets"):requested),
         lock(processLockName(directory,"Bank")) {reload();}
    const std::vector<Entry>& entries() const noexcept {return bank;}
    juce::File folder() const {return directory;}
    void setCuration(const juce::var& plan){retirements=plan["retired"];applyCuration();}
    juce::Result ensureBundled(const juce::String& id,const void* data,size_t bytes){
        auto prepared=prepareProcessLock(directory,"Bundle");if(prepared.failed())return prepared;
        juce::InterProcessLock bootstrap(processLockName(directory,"Bundle"));
        if(!bootstrap.enter(5000))return juce::Result::fail("Another instance is installing the bundled banks. Reopen BANK to retry.");
        const auto result=[&]()->juce::Result{
            const auto hash=juce::SHA256(data,bytes).toHexString();const auto marker=directory.getChildFile("bundled_"+id+".sha256");
            if(manifest().existsAsFile()&&marker.loadFileAsString().trim()==hash)return reload();
            auto r=directory.createDirectory();if(r.failed())return r;r=reload();if(r.failed())return r;
            juce::TemporaryFile archive(directory.getNonexistentChildFile("bundled_"+id,".zip"));
            if(!archive.getFile().replaceWithData(data,bytes))return juce::Result::fail("Cannot stage bundled presets.");
            int count=0;r=importBank(archive.getFile(),count);if(r.failed())return r;
            return marker.replaceWithText(hash)?juce::Result::ok():juce::Result::fail("Cannot record bundled-bank installation.");
        }();bootstrap.exit();return result;
    }
    int indexForKey(const juce::String& key) const {for(size_t i=0;i<bank.size();++i)if(bank[i].key==key)return int(i);return -1;}
    juce::String keyForFactory(int index) const {return "F"+juce::String(factoryId(index));}
    juce::Result reload(){
        auto file=manifest();snapshot=file.existsAsFile()?file.loadFileAsString():juce::String{};
        bank.clear();
        if(snapshot.isNotEmpty()){
            auto xml=juce::parseXML(snapshot);
            if(!xml||!xml->hasTagName("SPACE_INFINITY_BANK"))return juce::Result::fail("Preset bank index is invalid. Restore Factory or choose a valid bank.");
            for(auto* node:xml->getChildIterator()){
                if(!node->hasTagName("PRESET"))continue;
                Entry e{node->getStringAttribute("key"),node->getStringAttribute("name").substring(0,64),node->getStringAttribute("category").substring(0,40),node->getStringAttribute("file"),node->getIntAttribute("factory",-1)};
                e.creator=node->getStringAttribute("creator");e.tags=node->getStringAttribute("tags");e.detailsEdited=node->getBoolAttribute("detailsEdited");
                if(e.name.isEmpty()||indexForKey(e.key)>=0)continue;
                if(e.factory>=0){
                    if(e.factory<firstFactory)continue;
                    e.factory-=firstFactory;
                    if(e.factory==retiredFactory)continue;
                    if(retiredFactory>=0&&e.factory>retiredFactory)--e.factory;
                    if(e.factory>=factoryNames.size()||e.key!=keyForFactory(e.factory))continue;if(e.file!="factory_"+juce::String(factoryId(e.factory))+".sipreset"||!directory.getChildFile(e.file).existsAsFile())e.file={};if(e.creator.isEmpty())e.creator=creatorFor(e.factory);if(e.tags.isEmpty())e.tags=tagsFor(e.factory);
                    if(firstFactory==120&&e.factory==1&&e.name=="Airwave Pad")e.name=factoryNames[e.factory];
                    // A shipped preset can gain new role/revision tags between
                    // releases. Refresh untouched factory metadata so users can
                    // discover those revisions in an existing library. Local
                    // sound replacements and explicit metadata edits win.
                    if(e.file.isEmpty()&&!e.detailsEdited&&e.name==factoryNames[e.factory]){
                        e.category=factoryCategories[e.factory];
                        e.creator=creatorFor(e.factory);e.tags=tagsFor(e.factory);
                    }
                }
                else if(!safeFilename(e.file)||e.key!="U"+e.file.upToLastOccurrenceOf(".",false,false))continue;
                bank.push_back(std::move(e));
            }
            // A new bank generation retires old factory entries but preserves user files.
            if(xml->getIntAttribute("firstFactory",0)!=firstFactory||xml->getIntAttribute("factoryCount",0)!=factoryNames.size())
                for(int i=factoryNames.size()-1;i>=0;--i)
                    if(indexForKey(keyForFactory(i))<0)bank.insert(bank.begin(),{keyForFactory(i),factoryNames[i],factoryCategories[i],{},i,creatorFor(i),tagsFor(i)});
        }else defaults();
        applyCuration();
        return juce::Result::ok();
    }
    juce::Result read(int index,juce::MemoryBlock& bytes) const {
        if(!valid(index)||bank[size_t(index)].file.isEmpty())return juce::Result::fail("Select a saved preset or edited factory slot.");
        const auto file=directory.getChildFile(bank[size_t(index)].file);
        if(file.getSize()>64*1024*1024||!file.loadFileAsData(bytes))return juce::Result::fail("Cannot read preset file.");
        auto xml=juce::AudioProcessor::getXmlFromBinary(bytes.getData(),int(bytes.getSize()));
        if(!xml||!xml->hasTagName("PARAMETERS"))return juce::Result::fail("This is not a Space Infinity preset.");
        const auto& e=bank[size_t(index)];xml->setAttribute("presetDisplayName",e.name);xml->setAttribute("creator",e.creator);xml->setAttribute("tags",e.tags);juce::AudioProcessor::copyXmlToBinary(*xml,bytes);return juce::Result::ok();
    }
    juce::Result add(const juce::String& name,const juce::MemoryBlock& bytes,juce::String& newKey){
        const auto cleaned=cleanName(name);if(cleaned.isEmpty())return juce::Result::fail("Enter a preset name.");
        const auto file="user_"+juce::Uuid().toString().removeCharacters("-")+".sipreset";
        auto result=mutate([&]{const auto saved=writeBytes(directory.getChildFile(file),bytes);if(saved.failed())return saved;
            newKey="U"+file.upToLastOccurrenceOf(".",false,false);auto xml=juce::AudioProcessor::getXmlFromBinary(bytes.getData(),int(bytes.getSize()));bank.push_back({newKey,cleaned,"User",file,-1,xml?xml->getStringAttribute("creator"):juce::String{},xml?xml->getStringAttribute("tags"):juce::String{}});return juce::Result::ok();});
        if(result.failed())directory.getChildFile(file).deleteFile();return result;
    }
    // Validate the complete archive before writing. Stable collection IDs make a
    // second import harmless and retain local edits rather than replacing them.
    juce::Result importBank(const juce::File& file,int& imported){
        imported=0;
        if(!file.existsAsFile()||file.getSize()>256*1024*1024)return juce::Result::fail("Cannot read bank ZIP (maximum 256 MB).");
        juce::ZipFile zip(file);
        const int manifestIndex=zip.getIndexOfFileName("Bank.json");
        if(manifestIndex<0||zip.getNumEntries()>1024)return juce::Result::fail("This ZIP has no Space Infinity Bank.json.");
        const auto* manifestEntry=zip.getEntry(manifestIndex);
        if(!manifestEntry||manifestEntry->uncompressedSize>1024*1024)return juce::Result::fail("Invalid bank manifest.");
        std::unique_ptr<juce::InputStream> input(zip.createStreamForEntry(manifestIndex));
        if(!input)return juce::Result::fail("Cannot read bank manifest.");
        const auto metadata=juce::JSON::parse(input->readEntireStreamAsString());
        const auto id=metadata["id"].toString();const auto rows=metadata["presets"];
        if((metadata["format"].toString()!="SPACE_INFINITY_BANK_V1"&&metadata["format"].toString()!="SPACE_INFINITY_BANK_V2")||id.isEmpty()||id.length()>64||!id.containsOnly("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-")||!rows.isArray()||rows.size()<1||rows.size()>512)return juce::Result::fail("Invalid Space Infinity bank format.");
        if(metadata["format"].toString()=="SPACE_INFINITY_BANK_V2"){
            const auto minimum=metadata["minimum_version"].toString();const auto wanted=juce::StringArray::fromTokens(minimum,".","");const auto current=juce::StringArray::fromTokens(JucePlugin_VersionString,".","");
            if(wanted.size()!=3||current.size()!=3)return juce::Result::fail("Invalid bank minimum version.");
            for(int i=0;i<3;++i)if(!wanted[i].containsOnly("0123456789")||wanted[i].isEmpty()||wanted[i].length()>6)return juce::Result::fail("Invalid bank minimum version.");
            for(int i=0;i<3;++i){if(wanted[i].getIntValue()>current[i].getIntValue())return juce::Result::fail("This bank needs SPACE INFINITY "+minimum+" or later. Update the plug-in first.");if(wanted[i].getIntValue()<current[i].getIntValue())break;}
        }
        struct Pending {Entry entry;juce::MemoryBlock bytes;juce::String previousHash;juce::var previousHashes;};std::vector<Pending> pending;juce::StringArray seen;juce::int64 total=0;
        for(const auto& row:*rows.getArray()){
            const auto path=row["file"].toString(),patchId=row["id"].toString();
            if(patchId.isEmpty()||patchId.length()>32||!patchId.containsOnly("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-")||seen.contains(patchId)||!path.endsWith(".sipreset")||path.startsWith("/")||path.contains("..")||path.contains("\\")||path.contains(":"))return juce::Result::fail("Invalid or duplicate preset path/ID.");
            seen.add(patchId);const int index=zip.getIndexOfFileName(path);const auto* z=index>=0?zip.getEntry(index):nullptr;
            if(index<0||!z||z->uncompressedSize<1||z->uncompressedSize>64*1024*1024||(total+=z->uncompressedSize)>512*1024*1024)return juce::Result::fail("Missing preset or bank exceeds the size limit.");
            std::unique_ptr<juce::InputStream> stream(zip.createStreamForEntry(index));Pending p;
            if(!stream||stream->readIntoMemoryBlock(p.bytes,z->uncompressedSize)!=z->uncompressedSize)return juce::Result::fail("Incomplete preset in ZIP.");
            auto xml=juce::AudioProcessor::getXmlFromBinary(p.bytes.getData(),int(p.bytes.getSize()));
            if(!xml||!xml->hasTagName("PARAMETERS")||xml->getChildByAttribute("id","cutoff")==nullptr)return juce::Result::fail("Invalid preset in bank ZIP.");
            const auto filename="user_"+id+"_"+patchId+".sipreset";
            if(!safeFilename(filename))return juce::Result::fail("Bank ID and preset ID are too long.");
            p.entry={"U"+filename.upToLastOccurrenceOf(".",false,false),cleanName(xml->getStringAttribute("presetDisplayName")),cleanName(row["category"].toString()).substring(0,40),filename,-1,cleanName(xml->getStringAttribute("creator")),xml->getStringAttribute("tags").substring(0,512)};
            if(p.entry.name.isEmpty())return juce::Result::fail("Bank contains an unnamed preset.");
            if(p.entry.category.isEmpty())p.entry.category="User";
            p.previousHash=row["previous_sha256"].toString();p.previousHashes=row["previous_sha256s"];pending.push_back(std::move(p));
        }
        juce::Array<juce::File> created;
        struct Replaced {juce::File file;juce::MemoryBlock bytes;};std::vector<Replaced> replaced;
        const auto result=mutate([&]{for(auto& p:pending){
            if(isRetired(p.entry))continue;
            const int existing=indexForKey(p.entry.key);
            if(existing>=0){
                const auto& old=bank[size_t(existing)];const auto target=directory.getChildFile(old.file);
                if(old.detailsEdited||!target.existsAsFile())continue;const auto currentHash=juce::SHA256(target).toHexString();if(currentHash==juce::SHA256(p.bytes).toHexString())continue;bool shipped=currentHash==p.previousHash&&p.previousHash.length()==64;if(const auto* history=p.previousHashes.getArray())for(const auto& hash:*history)if(hash.toString().length()==64&&hash.toString()==currentHash)shipped=true;
                if(old.detailsEdited||!target.existsAsFile()||!shipped)continue;
                Replaced backup{target,{}};if(!target.loadFileAsData(backup.bytes))return juce::Result::fail("Cannot back up the previous bank sound.");
                const auto backups=directory.getChildFile("Backups");if(backups.createDirectory().failed()||!target.copyFileTo(backups.getNonexistentChildFile(target.getFileNameWithoutExtension(),".sipreset")))return juce::Result::fail("Cannot retain the previous bank version.");
                replaced.push_back(std::move(backup));const auto written=writeBytes(target,p.bytes);if(written.failed())return written;
                bank[size_t(existing)]=p.entry;++imported;continue;
            }
            const auto target=directory.getChildFile(p.entry.file);if(target.existsAsFile()){if(juce::SHA256(target).toHexString()==juce::SHA256(p.bytes).toHexString()){bank.push_back(p.entry);++imported;continue;}return juce::Result::fail("A bank file already exists outside the index. Refresh or restore it before importing.");}const auto written=writeBytes(target,p.bytes);if(written.failed())return written;created.add(target);bank.push_back(p.entry);++imported;
        }return juce::Result::ok();});
        if(result.failed()){for(const auto& target:created)target.deleteFile();bool restored=true;for(const auto& old:replaced)restored=writeBytes(old.file,old.bytes).wasOk()&&restored;imported=0;if(!restored)return juce::Result::fail("Bank update failed; restore the previous sounds from Backups.");}return result;
    }
    juce::Result replace(int index,const juce::MemoryBlock& bytes){
        if(!valid(index))return juce::Result::fail("Select a preset to replace.");if(bank[size_t(index)].factory==0)return juce::Result::fail("Init remains the reset sound. Use Save New.");
        auto xml=juce::AudioProcessor::getXmlFromBinary(bytes.getData(),int(bytes.getSize()));if(!xml||!xml->hasTagName("PARAMETERS"))return juce::Result::fail("Invalid preset data.");
        const auto entryBefore=bank[size_t(index)];const auto target=directory.getChildFile(entryBefore.factory>=0?"factory_"+juce::String(factoryId(entryBefore.factory))+".sipreset":entryBefore.file);const bool existed=target.existsAsFile();juce::MemoryBlock previousBytes;if(existed&&!target.loadFileAsData(previousBytes))return juce::Result::fail("Cannot retain previous preset data.");
        const auto result=mutate([&]{auto& entry=bank[size_t(index)];const auto filename=entry.factory>=0?"factory_"+juce::String(factoryId(entry.factory))+".sipreset":entry.file;const auto file=directory.getChildFile(filename);if(file.existsAsFile()){const auto backups=directory.getChildFile("Backups");if(backups.createDirectory().failed()||!file.copyFileTo(backups.getNonexistentChildFile(file.getFileNameWithoutExtension(),".sipreset")))return juce::Result::fail("Cannot retain the previous version.");}const auto saved=writeBytes(file,bytes);if(saved.wasOk()){entry.file=filename;const auto renamed=cleanName(xml->getStringAttribute("presetDisplayName"));if(renamed.isNotEmpty())entry.name=renamed;entry.creator=xml->getStringAttribute("creator").substring(0,64);entry.tags=xml->getStringAttribute("tags").substring(0,512);}return saved;});
        if(result.failed()&&lastOperationRan){if(existed){const auto rollback=writeBytes(target,previousBytes);if(rollback.failed())return juce::Result::fail("Save failed; restore the previous version from Backups.");}else target.deleteFile();}return result;
    }
    juce::Result exportEdits(const juce::File& zip){
        juce::ZipFile::Builder builder;juce::Array<juce::var> entries;for(const auto& e:bank)if(e.factory>0&&(e.file.isNotEmpty()||e.detailsEdited)){const auto file=directory.getChildFile(e.file);if(e.file.isNotEmpty()&&!file.existsAsFile())return juce::Result::fail("An edited factory file is missing.");auto* row=new juce::DynamicObject;row->setProperty("factory_key",e.key);row->setProperty("name",e.name);row->setProperty("category",e.category);row->setProperty("creator",e.creator);row->setProperty("tags",e.tags);row->setProperty("file",e.file.isEmpty()?juce::String{}:e.key+".sipreset");row->setProperty("metadata_only",e.file.isEmpty());entries.add(juce::var(row));if(e.file.isNotEmpty())builder.addFile(file,9,e.key+".sipreset");}
        if(entries.isEmpty())return juce::Result::fail("No factory replacements yet. Load a factory sound, edit it, then press Replace.");auto* manifest=new juce::DynamicObject;manifest->setProperty("format","SPACE_INFINITY_FACTORY_EDITS_V1");manifest->setProperty("presets",entries);const auto text=juce::JSON::toString(juce::var(manifest));builder.addEntry(new juce::MemoryInputStream(text.toRawUTF8(),size_t(text.getNumBytesAsUTF8()),true),9,"FactoryEdits.json",juce::Time::getCurrentTime());juce::TemporaryFile temp(zip);auto out=temp.getFile().createOutputStream();if(!out||!builder.writeToStream(*out,nullptr))return juce::Result::fail("Cannot write the edits ZIP.");out.reset();return temp.overwriteTargetFileWithTemporary()?juce::Result::ok():juce::Result::fail("Cannot replace the export ZIP.");
    }
    juce::Result updateDetails(int index,const juce::String& name,const juce::String& creator,const juce::String& tags){
        const auto cleaned=cleanName(name);if(!valid(index)||cleaned.isEmpty())return juce::Result::fail("Select a preset and enter a name.");
        auto tokens=juce::StringArray::fromTokens(tags,",","");tokens.trim();tokens.removeEmptyStrings();tokens.removeDuplicates(true);
        return mutate([&]{auto& e=bank[size_t(index)];e.name=cleaned;e.creator=cleanName(creator);e.tags=tokens.joinIntoString(", ").substring(0,512);e.detailsEdited=true;return juce::Result::ok();});
    }
    juce::Result rename(int index,const juce::String& name){if(!valid(index))return juce::Result::fail("Select a preset.");const auto e=bank[size_t(index)];return updateDetails(index,name,e.creator,e.tags);}
    juce::Result remove(int index){
        if(!valid(index))return juce::Result::fail("Select a preset.");const auto entry=bank[size_t(index)];
        const auto result=mutate([&]{
            if(entry.factory<0){const auto trash=directory.getChildFile("Deleted");if(trash.createDirectory().failed())return juce::Result::fail("Cannot create Deleted preset folder.");
                const auto original=directory.getChildFile(entry.file);if(original.existsAsFile()&&!original.copyFileTo(trash.getNonexistentChildFile(entry.file.upToLastOccurrenceOf(".",false,false),".sipreset")))return juce::Result::fail("Cannot retain deleted preset backup.");}
            bank.erase(bank.begin()+index);return juce::Result::ok();
        });
        if(result.wasOk()&&entry.factory<0)directory.getChildFile(entry.file).deleteFile();return result;
    }
    juce::Result move(int index,int delta){if(!valid(index)||!valid(index+delta))return juce::Result::fail("Already at the end of the list.");return mutate([&]{std::swap(bank[size_t(index)],bank[size_t(index+delta)]);return juce::Result::ok();});}
    juce::Result sortByName(){return mutate([&]{std::stable_sort(bank.begin(),bank.end(),[](const Entry& a,const Entry& b){return a.name.compareIgnoreCase(b.name)<0;});return juce::Result::ok();});}
    juce::Result restoreFactory(){return mutate([&]{auto users=bank;defaults();for(const auto& e:users)if(e.factory<0)bank.push_back(e);return juce::Result::ok();});}
private:
    juce::StringArray factoryNames,factoryCategories;int retiredFactory=-1,firstFactory=0;juce::StringArray factoryCreators,factoryTags;juce::File directory;juce::InterProcessLock lock;
    std::vector<Entry> bank;juce::String snapshot;bool lastOperationRan=false;juce::var retirements;
    bool isRetired(const Entry& e) const {
        if(e.factory==0||e.detailsEdited)return false;
        if(const auto* rows=retirements.getArray())for(const auto& row:*rows)if(row["key"].toString()==e.key&&row["name"].toString()==e.name){
            if(e.factory>0)return e.file.isEmpty();
            const auto hash=row["sha256"].toString();return hash.length()==64&&directory.getChildFile(e.file).existsAsFile()&&juce::SHA256(directory.getChildFile(e.file)).toHexString()==hash;
        }return false;
    }
    void applyCuration(){bank.erase(std::remove_if(bank.begin(),bank.end(),[this](const Entry& e){return isRetired(e);}),bank.end());}
    bool valid(int index) const {return index>=0&&index<int(bank.size());}
    juce::File manifest() const {return directory.getChildFile("bank.xml");}
    static juce::String cleanName(juce::String name){return name.trim().replaceCharacters("\r\n\t","   ").substring(0,64);}
    static bool safeFilename(const juce::String& name){return name.startsWith("user_")&&name.endsWith(".sipreset")&&name.length()<64&&name.containsOnly("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.");}
    int factoryId(int index) const {return firstFactory+(retiredFactory>=0&&index>=retiredFactory?index+1:index);}
    juce::String creatorFor(int i)const{return i<factoryCreators.size()?factoryCreators[i]:juce::String("Orbital Audio");}
    juce::String tagsFor(int i)const{return i<factoryTags.size()?factoryTags[i]:factoryCategories[i];}
    void defaults(){bank.clear();for(int i=0;i<factoryNames.size();++i)bank.push_back({keyForFactory(i),factoryNames[i],factoryCategories[i],{},i,creatorFor(i),tagsFor(i)});}
    static juce::Result writeBytes(const juce::File& target,const juce::MemoryBlock& bytes){juce::TemporaryFile temp(target);
        if(!temp.getFile().replaceWithData(bytes.getData(),bytes.getSize())||!temp.overwriteTargetFileWithTemporary())return juce::Result::fail("Cannot write preset file.");return juce::Result::ok();}
    juce::Result mutate(const std::function<juce::Result()>& operation){
        lastOperationRan=false;
        if(directory.createDirectory().failed())return juce::Result::fail("Preset folder is not writable.");
        if(!lock.enter(1500))return juce::Result::fail("Preset bank is busy. Try again.");
        struct Unlock {juce::InterProcessLock& l;~Unlock(){l.exit();}} unlock{lock};
        const auto current=manifest().existsAsFile()?manifest().loadFileAsString():juce::String{};
        if(current!=snapshot)return juce::Result::fail("Preset bank changed in another instance. Press Refresh and try again.");
        const auto previous=bank;lastOperationRan=true;auto result=operation();
        if(result.failed()){bank=previous;return result;}
        applyCuration();
        juce::XmlElement xml("SPACE_INFINITY_BANK");xml.setAttribute("revision",juce::Uuid().toString());xml.setAttribute("firstFactory",firstFactory);xml.setAttribute("factoryCount",factoryNames.size());
        for(const auto& e:bank){auto* node=xml.createNewChildElement("PRESET");node->setAttribute("key",e.key);node->setAttribute("name",e.name);node->setAttribute("category",e.category);node->setAttribute("file",e.file);node->setAttribute("creator",e.creator);node->setAttribute("tags",e.tags);node->setAttribute("detailsEdited",e.detailsEdited);node->setAttribute("factory",e.factory>=0?factoryId(e.factory):-1);}
        const auto text=xml.toString();juce::TemporaryFile temp(manifest());
        if(!temp.getFile().replaceWithText(text)||!temp.overwriteTargetFileWithTemporary()){bank=previous;return juce::Result::fail("Cannot save preset bank order.");}
        snapshot=text;return juce::Result::ok();
    }
};
