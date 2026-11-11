#pragma once
#include <JuceHeader.h>
#include <functional>
#include <algorithm>
#include <vector>
// Message-thread file operations. Stable keys decouple saved projects from bank order.
class PresetLibrary {
public:
    struct Entry {juce::String key,name,category,file;int factory=-1;};
    PresetLibrary(juce::StringArray names,juce::StringArray categories,const juce::File& requested={})
        :factoryNames(std::move(names)),factoryCategories(std::move(categories)),
         directory(requested==juce::File{}?juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("Orbital Audio/Space Infinity/Presets"):requested),
         lock("SpaceInfinityBank"+juce::String(directory.getFullPathName().hashCode64())) {reload();}
    const std::vector<Entry>& entries() const noexcept {return bank;}
    juce::File folder() const {return directory;}
    int indexForKey(const juce::String& key) const {for(size_t i=0;i<bank.size();++i)if(bank[i].key==key)return int(i);return -1;}
    juce::Result reload(){
        auto file=manifest();snapshot=file.existsAsFile()?file.loadFileAsString():juce::String{};
        bank.clear();
        if(snapshot.isNotEmpty()){
            auto xml=juce::parseXML(snapshot);
            if(!xml||!xml->hasTagName("SPACE_INFINITY_BANK"))return juce::Result::fail("Preset bank index is invalid. Restore Factory or choose a valid bank.");
            for(auto* node:xml->getChildIterator()){
                if(!node->hasTagName("PRESET"))continue;
                Entry e{node->getStringAttribute("key"),node->getStringAttribute("name").substring(0,64),node->getStringAttribute("category").substring(0,40),node->getStringAttribute("file"),node->getIntAttribute("factory",-1)};
                if(e.name.isEmpty()||indexForKey(e.key)>=0)continue;
                if(e.factory>=0){if(e.factory>=factoryNames.size()||e.key!="F"+juce::String(e.factory))continue;e.file={};}
                else if(!safeFilename(e.file)||e.key!="U"+e.file.upToLastOccurrenceOf(".",false,false))continue;
                bank.push_back(std::move(e));
            }
        }else defaults();
        return juce::Result::ok();
    }
    juce::Result read(int index,juce::MemoryBlock& bytes) const {
        if(!valid(index)||bank[size_t(index)].factory>=0)return juce::Result::fail("Select a user preset.");
        const auto file=directory.getChildFile(bank[size_t(index)].file);
        if(file.getSize()>64*1024*1024||!file.loadFileAsData(bytes))return juce::Result::fail("Cannot read preset file.");
        auto xml=juce::AudioProcessor::getXmlFromBinary(bytes.getData(),int(bytes.getSize()));
        return xml&&xml->hasTagName("PARAMETERS")?juce::Result::ok():juce::Result::fail("This is not a Space Infinity preset.");
    }
    juce::Result add(const juce::String& name,const juce::MemoryBlock& bytes,juce::String& newKey){
        const auto cleaned=cleanName(name);if(cleaned.isEmpty())return juce::Result::fail("Enter a preset name.");
        const auto file="user_"+juce::Uuid().toString().removeCharacters("-")+".sipreset";
        auto result=mutate([&]{const auto saved=writeBytes(directory.getChildFile(file),bytes);if(saved.failed())return saved;
            newKey="U"+file.upToLastOccurrenceOf(".",false,false);bank.push_back({newKey,cleaned,"User",file,-1});return juce::Result::ok();});
        if(result.failed())directory.getChildFile(file).deleteFile();return result;
    }
    juce::Result replace(int index,const juce::MemoryBlock& bytes){
        if(!valid(index)||bank[size_t(index)].factory>=0)return juce::Result::fail("Factory presets cannot be overwritten. Use Save New.");
        const auto file=directory.getChildFile(bank[size_t(index)].file);juce::MemoryBlock previous;
        if(!file.loadFileAsData(previous))return juce::Result::fail("Cannot back up existing preset.");
        const auto result=mutate([&]{return writeBytes(file,bytes);});
        if(result.failed()&&lastOperationRan)writeBytes(file,previous);return result;
    }
    juce::Result rename(int index,const juce::String& name){const auto cleaned=cleanName(name);if(!valid(index)||cleaned.isEmpty())return juce::Result::fail("Select a preset and enter a name.");
        return mutate([&]{bank[size_t(index)].name=cleaned;return juce::Result::ok();});}
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
    juce::StringArray factoryNames,factoryCategories;juce::File directory;juce::InterProcessLock lock;
    std::vector<Entry> bank;juce::String snapshot;bool lastOperationRan=false;
    bool valid(int index) const {return index>=0&&index<int(bank.size());}
    juce::File manifest() const {return directory.getChildFile("bank.xml");}
    static juce::String cleanName(juce::String name){return name.trim().replaceCharacters("\r\n\t","   ").substring(0,64);}
    static bool safeFilename(const juce::String& name){return name.startsWith("user_")&&name.endsWith(".sipreset")&&name.length()<64&&name.containsOnly("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.");}
    void defaults(){bank.clear();for(int i=0;i<factoryNames.size();++i)bank.push_back({"F"+juce::String(i),factoryNames[i],factoryCategories[i],{},i});}
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
        juce::XmlElement xml("SPACE_INFINITY_BANK");xml.setAttribute("revision",juce::Uuid().toString());
        for(const auto& e:bank){auto* node=xml.createNewChildElement("PRESET");node->setAttribute("key",e.key);node->setAttribute("name",e.name);node->setAttribute("category",e.category);node->setAttribute("file",e.file);node->setAttribute("factory",e.factory);}
        const auto text=xml.toString();juce::TemporaryFile temp(manifest());
        if(!temp.getFile().replaceWithText(text)||!temp.overwriteTargetFileWithTemporary()){bank=previous;return juce::Result::fail("Cannot save preset bank order.");}
        snapshot=text;return juce::Result::ok();
    }
};
