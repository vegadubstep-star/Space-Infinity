#include "../Source/PluginProcessor.h"
#include "../Source/PresetPanel.h"
#include <iostream>
int runBankZipSmoke(const juce::File& archive){
    const auto folder=juce::File::getSpecialLocation(juce::File::tempDirectory).getNonexistentChildFile("InfinityHeritageTest",{},false);folder.createDirectory();
    auto synth=std::make_unique<SpaceInfinityProcessor>(folder);auto& library=synth->presetBank();const auto count=library.entries().size();
    const auto original=library.entries();int imported=0;
    // A later write conflict must roll back earlier writes and preserve the
    // orphan that caused the conflict, as well as all factory entries.
    const auto orphan=folder.getChildFile("user_heritage200_H100.sipreset");orphan.replaceWithText("retain existing orphan");
    if(library.importBank(archive,imported).wasOk()||imported!=0||library.entries().size()!=count||orphan.loadFileAsString()!="retain existing orphan"||folder.findChildFiles(juce::File::findFiles,false,"*.sipreset").size()!=1)return 210;
    orphan.deleteFile();auto result=library.importBank(archive,imported);
    if(result.failed()){std::cerr<<result.getErrorMessage()<<std::endl;return 201;}
    if(imported!=200||library.entries().size()!=count+200)return 202;
    for(size_t i=0;i<count;++i)if(original[i].key!=library.entries()[i].key||original[i].name!=library.entries()[i].name||original[i].file!=library.entries()[i].file)return 203;
    for(size_t i=count;i<library.entries().size();++i){if(synth->loadBankPreset(int(i)).failed())return 204;const auto& e=library.entries()[i];if(!e.tags.contains("Heritage 200")||e.creator!="Orbital Audio"||synth->state.state.getProperty("presetDisplayName").toString()!=e.name)return 205;}
    const auto key=library.entries()[count].key;if(library.updateDetails(int(count),"My edited Heritage piano","Owner","Personal").failed())return 206;
    if(library.importBank(archive,imported).failed()||imported!=0||library.entries().size()!=count+200||library.entries()[count].name!="My edited Heritage piano")return 207;
    if(library.reload().failed()||library.indexForKey(key)!=int(count)||library.entries()[count].tags!="Personal")return 208;
    // Upgrade only byte-identical shipped sounds; keep metadata/sound edits.
    juce::ZipFile shipping(archive);std::unique_ptr<juce::InputStream> manifestStream(shipping.createStreamForEntry(shipping.getIndexOfFileName("Bank.json")));const auto shippingManifest=juce::JSON::parse(manifestStream->readEntireStreamAsString());
    const auto makeUpdate=[&](const juce::File& destination,const std::vector<int>& indexes,bool conflict,juce::String minimum="0.8.0"){
        juce::ZipFile::Builder builder;juce::Array<juce::var> rows;
        for(int i:indexes){const auto old=shippingManifest["presets"][i];std::unique_ptr<juce::InputStream> stream(shipping.createStreamForEntry(shipping.getIndexOfFileName(old["file"].toString())));juce::MemoryBlock originalBytes;stream->readIntoMemoryBlock(originalBytes);auto xml=juce::AudioProcessor::getXmlFromBinary(originalBytes.getData(),int(originalBytes.getSize()));xml->getChildByAttribute("id","cutoff")->setAttribute("value",3000+i);juce::MemoryBlock revised;juce::AudioProcessor::copyXmlToBinary(*xml,revised);
            auto* row=new juce::DynamicObject;row->setProperty("id",old["id"]);row->setProperty("file",old["file"]);row->setProperty("category",old["category"]);row->setProperty("previous_sha256",juce::SHA256(originalBytes).toHexString());rows.add(juce::var(row));builder.addEntry(new juce::MemoryInputStream(revised,true),9,old["file"].toString(),juce::Time::getCurrentTime());
            if(conflict){auto* extra=new juce::DynamicObject;extra->setProperty("id","NewConflict");extra->setProperty("file","Conflict.sipreset");extra->setProperty("category","Lead");rows.add(juce::var(extra));builder.addEntry(new juce::MemoryInputStream(revised,true),9,"Conflict.sipreset",juce::Time::getCurrentTime());}
        }
        auto* manifest=new juce::DynamicObject;manifest->setProperty("format","SPACE_INFINITY_BANK_V2");manifest->setProperty("minimum_version",minimum);manifest->setProperty("id","heritage200");manifest->setProperty("presets",rows);const auto text=juce::JSON::toString(juce::var(manifest));builder.addEntry(new juce::MemoryInputStream(text.toRawUTF8(),size_t(text.getNumBytesAsUTF8()),true),9,"Bank.json",juce::Time::getCurrentTime());auto output=destination.createOutputStream();return output&&builder.writeToStream(*output,nullptr);
    };
    juce::MemoryBlock editedBytes;library.read(int(count+3),editedBytes);auto editedXml=juce::AudioProcessor::getXmlFromBinary(editedBytes.getData(),int(editedBytes.getSize()));editedXml->getChildByAttribute("id","cutoff")->setAttribute("value",1733);juce::AudioProcessor::copyXmlToBinary(*editedXml,editedBytes);if(library.replace(int(count+3),editedBytes).failed())return 211;
    const auto update=folder.getChildFile("Upgrade.zip");if(!makeUpdate(update,{0,2,3},false)||library.importBank(update,imported).failed()||imported!=1)return 212;
    if(library.entries()[count].name!="My edited Heritage piano"||synth->loadBankPreset(int(count+2)).failed()||synth->state.getRawParameterValue("cutoff")->load()!=3002)return 213;
    if(synth->loadBankPreset(int(count+3)).failed()||synth->state.getRawParameterValue("cutoff")->load()!=1733||library.importBank(update,imported).failed()||imported!=0)return 214;
    const auto replaceFile=folder.getChildFile(library.entries()[count+4].file);const auto beforeHash=juce::SHA256(replaceFile).toHexString();const auto conflictFile=folder.getChildFile("user_heritage200_NewConflict.sipreset");conflictFile.replaceWithText("retain update orphan");const auto conflicting=folder.getChildFile("UpgradeConflict.zip");if(!makeUpdate(conflicting,{4},true)||library.importBank(conflicting,imported).wasOk()||imported!=0||juce::SHA256(replaceFile).toHexString()!=beforeHash||conflictFile.loadFileAsString()!="retain update orphan")return 215;
    const auto future=folder.getChildFile("Unsupported.zip");
    for(const auto* version:{"99.0.0","0.1.bad","0.8.","0.8.00000000000000000009"}){
        future.deleteFile();if(!makeUpdate(future,{5},false,version)||library.importBank(future,imported).wasOk()||imported!=0||library.entries().size()!=count+200)return 216;
    }
    // A truncated/invalid ZIP must leave both index and saved files untouched.
    const auto invalid=folder.getChildFile("Invalid.zip");invalid.replaceWithText("not a ZIP");if(library.importBank(invalid,imported).wasOk()||imported!=0||library.entries().size()!=count+200)return 209;
    synth.reset();folder.deleteRecursively();std::cout<<"200-preset bank import, native recall, existing bank preservation, edited reimport and invalid archive rejection passed\n";return 0;
}
int runPresetSmoke(){
    const auto folder=juce::File::getSpecialLocation(juce::File::tempDirectory).getNonexistentChildFile("SpaceInfinityBankTest",{},false);
    folder.createDirectory();
    // Heap allocation also exercises multiple simultaneous plugin instances without large test stack frames.
    auto synth=std::make_unique<SpaceInfinityProcessor>(folder);
    const int factoryCount=synth->presetNames().size();
    // Upgrade a complete old bank with a user sound: all old factory IDs retire.
    const auto legacyFolder=folder.getChildFile("Legacy");
    juce::StringArray legacyNames,legacyCategories;
    for(int i=0;i<120;++i){legacyNames.add("Old sound "+juce::String(i));legacyCategories.add("Legacy");}
    PresetLibrary legacy(legacyNames,legacyCategories,legacyFolder);
    juce::MemoryBlock saved;synth->getStateInformation(saved);juce::String savedKey;
    if(legacy.add("Keep my sound",saved,savedKey).failed())return 105;
    auto migrated=std::make_unique<SpaceInfinityProcessor>(legacyFolder);
    auto& migratedBank=migrated->presetBank();
    if(migratedBank.entries().size()!=size_t(factoryCount+1)||migratedBank.indexForKey(savedKey)<0)return 106;
    for(int i=0;i<120;++i)if(migratedBank.indexForKey("F"+juce::String(i))!=-1)return 107;
    if(migratedBank.entries().front().key!="F1000"||migratedBank.entries().front().name!="Init")return 108;
    if(migrated->loadBankPreset(migratedBank.indexForKey(savedKey)).failed())return 109;
    if(migratedBank.restoreFactory().failed()||migratedBank.reload().failed()||migratedBank.entries().size()!=size_t(factoryCount+1))return 110;
    if(migratedBank.indexForKey(savedKey)<0||!legacyFolder.getChildFile(legacy.entries().back().file).existsAsFile())return 111;
    migrated.reset();
    const auto initFolder=folder.getChildFile("InitGeneration");
    PresetLibrary initOnly({"Init"},{"Init"},initFolder,-1,1000);
    if(initOnly.rename(0,"Init").failed())return 112;
    auto upgraded=std::make_unique<SpaceInfinityProcessor>(initFolder);
    if(upgraded->presetBank().entries().size()!=size_t(factoryCount)||upgraded->presetBank().indexForKey("F1001")<0)return 113;
    if(upgraded->presetBank().remove(upgraded->presetBank().indexForKey("F1001")).failed()||upgraded->presetBank().reload().failed()||upgraded->presetBank().indexForKey("F1001")!=-1)return 114;
    upgraded.reset();
    const auto padFolder=folder.getChildFile("PadGeneration");
    PresetLibrary previousPad({"Init","Airwave Pad"},{"Init","Pads"},padFolder,-1,120);
    if(previousPad.rename(0,"Init").failed())return 115;
    auto renamedPad=std::make_unique<SpaceInfinityProcessor>(padFolder);
    const auto& renamedBank=renamedPad->presetBank();
    if(renamedBank.entries().size()!=size_t(factoryCount)||renamedBank.entries()[size_t(renamedBank.indexForKey("F1001"))].name!="Tunnel Knock")return 116;
    renamedPad.reset();
    synth->loadPreset(0);auto* cutoff=synth->state.getParameter("cutoff");cutoff->setValueNotifyingHost(cutoff->convertTo0to1(1234));
    const auto exportedFolder=folder.getChildFile("Chosen Folder");exportedFolder.createDirectory();const auto exportedFile=exportedFolder.getChildFile("My Custom Sound.sipreset");
    const auto originalKey=synth->selectedPresetKey();
    if(synth->exportPreset(exportedFile).failed()||synth->selectedPresetKey()!=originalKey||synth->presetBank().entries().size()!=size_t(factoryCount))return 117;
    juce::MemoryBlock exported,unchanged;if(!exportedFile.loadFileAsData(exported))return 118;
    cutoff->setValueNotifyingHost(cutoff->convertTo0to1(2222));
    if(synth->exportPreset(exportedFile).wasOk()||!exportedFile.loadFileAsData(unchanged)||exported!=unchanged)return 119;
    exported.reset();if(synth->exportPreset(exportedFile,true).failed()||!exportedFile.loadFileAsData(exported)||synth->presetExportFolder()!=exportedFolder)return 120;
    auto exportedRecall=std::make_unique<SpaceInfinityProcessor>(folder.getChildFile("Export Recall"));exportedRecall->setStateInformation(exported.getData(),int(exported.getSize()));
    if(std::abs(exportedRecall->state.getRawParameterValue("cutoff")->load()-2222)>.01f){std::cerr<<"Export recall cutoff: "<<exportedRecall->state.getRawParameterValue("cutoff")->load()<<std::endl;return 121;}
    if(synth->exportPreset(exportedFolder.getChildFile("Wrong.txt")).wasOk()||synth->exportPreset(folder.getChildFile("Missing/Sound.sipreset")).wasOk())return 122;
    exportedRecall.reset();cutoff->setValueNotifyingHost(cutoff->convertTo0to1(1234));
    if(synth->saveNewPreset("My Analogue Lead").failed())return 70;
    auto& bank=synth->presetBank();const auto key=synth->selectedPresetKey();
    if(bank.entries().size()!=size_t(factoryCount+1)||bank.indexForKey(key)!=factoryCount)return 71;
    if(bank.move(factoryCount,-1).failed()||bank.indexForKey(key)!=factoryCount-1)return 72;
    if(bank.rename(factoryCount-1,"Personal Lead").failed())return 73;
    if(bank.updateDetails(bank.indexForKey(key),"Personal Lead","Test Creator","Trance, Bright, trance").failed())return 123;
    if(bank.reload().failed())return 124;
    const auto& details=bank.entries()[size_t(bank.indexForKey(key))];
    if(details.creator!="Test Creator"||details.tags!="Trance, Bright")return 125;
    auto other=std::make_unique<SpaceInfinityProcessor>(folder);
    const int loaded=other->presetBank().indexForKey(key);
    if(loaded!=factoryCount-1||other->loadBankPreset(loaded).failed()||std::abs(other->state.getRawParameterValue("cutoff")->load()-1234)>.01)return 74;
    if(other->state.state.getProperty("creator").toString()!="Test Creator"||other->state.state.getProperty("tags").toString()!="Trance, Bright")return 126;
    const auto taggedExport=exportedFolder.getChildFile("Tagged.sipreset");
    if(other->exportPreset(taggedExport).failed())return 127;
    juce::MemoryBlock taggedBytes;if(!taggedExport.loadFileAsData(taggedBytes))return 128;
    auto taggedXml=juce::AudioProcessor::getXmlFromBinary(taggedBytes.getData(),int(taggedBytes.getSize()));
    if(!taggedXml||taggedXml->getStringAttribute("creator")!="Test Creator"||taggedXml->getStringAttribute("tags")!="Trance, Bright")return 129;
    if(other->presetBank().sortByName().failed())return 75;
    if(bank.sortByName().wasOk())return 76; // A stale instance must not overwrite another instance's edits.
    if(bank.reload().failed()||bank.indexForKey(key)<0)return 77;
    cutoff->setValueNotifyingHost(cutoff->convertTo0to1(2345));
    if(synth->replaceBankPreset(bank.indexForKey(key)).failed())return 78;
    if(other->presetBank().reload().failed()||other->loadBankPreset(other->presetBank().indexForKey(key)).failed()||std::abs(other->state.getRawParameterValue("cutoff")->load()-2345)>.01)return 79;
    if(bank.remove(bank.indexForKey("F1000")).failed()||bank.indexForKey("F1000")!=-1||bank.restoreFactory().failed()||bank.entries().size()!=size_t(factoryCount+1))return 80;
    if(bank.remove(bank.indexForKey(key)).failed()||bank.entries().size()!=size_t(factoryCount))return 81;
    if(folder.getChildFile("Deleted").findChildFiles(juce::File::findFiles,false,"*.sipreset").isEmpty())return 82;
    if(synth->replaceBankPreset(0).wasOk())return 83;
    const auto factoryIndex=bank.indexForKey("F1001");
    if(bank.updateDetails(factoryIndex,"Tagged Factory","Factory Author","Acid, Custom").failed()||bank.reload().failed())return 130;
    if(synth->loadBankPreset(bank.indexForKey("F1001")).failed()||synth->state.state.getProperty("tags").toString()!="Acid, Custom")return 131;
    const auto metadataZip=folder.getChildFile("Factory Details.zip");if(bank.exportEdits(metadataZip).failed())return 132;
    juce::ZipFile editArchive(metadataZip);std::unique_ptr<juce::InputStream> editManifest(editArchive.createStreamForEntry(editArchive.getIndexOfFileName("FactoryEdits.json")));
    if(!editManifest)return 133;auto editRows=juce::JSON::parse(editManifest->readEntireStreamAsString())["presets"];if(!editRows.isArray()||editRows.size()!=1||!bool(editRows[0]["metadata_only"])||editRows[0]["tags"].toString()!="Acid, Custom")return 134;
    PresetPanel panel(*synth,[]{});panel.setSize(710,465);
    const auto audition=juce::File::getCurrentWorkingDirectory().getChildFile("audition");audition.createDirectory();
    const auto screenshotFile=audition.getChildFile("Preset-Manager.png");screenshotFile.deleteFile();auto stream=screenshotFile.createOutputStream();if(stream)juce::PNGImageFormat{}.writeImageToStream(panel.createComponentSnapshot(panel.getLocalBounds()),*stream);
    juce::AudioBuffer<float> audio(2,256);synth->loadPreset(0);synth->prepareToPlay(48000,256);
    if(synth->getLatencySamples()<1)return 84;
    for(int i=0;i<factoryCount;++i){synth->loadPreset(i);if(synth->state.getRawParameterValue("analogueUpgrade")->load()!=1)return 85;}
    juce::MemoryBlock state;synth->getStateInformation(state);other->setStateInformation(state.getData(),int(state.getSize()));
    if(other->selectedPresetKey()!=synth->selectedPresetKey())return 86;
    // Exercise the actual arrow callbacks without pumping the editor timer.
    // Manager loads, project recall and reordering must use the loaded key.
    {
        auto editor=std::unique_ptr<juce::AudioProcessorEditor>(synth->createEditor());
        juce::Button* previous=nullptr;juce::Button* next=nullptr;
        for(auto* child:editor->getChildren()){
            if(child->getName()=="Previous preset")previous=dynamic_cast<juce::Button*>(child);
            if(child->getName()=="Next preset")next=dynamic_cast<juce::Button*>(child);
        }
        if(!previous||!next)return 87;
        if(synth->saveNewPreset("Navigation A").failed())return 88;
        const auto navigationKey=synth->selectedPresetKey();
        next->onClick();if(synth->selectedPresetKey()!="F1000")return 89;
        previous->onClick();if(synth->selectedPresetKey()!=navigationKey)return 90;
        synth->getStateInformation(state);
        synth->loadPreset(0);synth->setStateInformation(state.getData(),int(state.getSize()));
        previous->onClick();if(synth->selectedPresetKey()!=bank.keyForFactory(factoryCount-1))return 91;
        cutoff->setValueNotifyingHost(cutoff->convertTo0to1(3210));
        next->onClick();if(synth->selectedPresetKey()!=navigationKey)return 92;
        if(bank.sortByName().failed())return 93;
        const int sorted=bank.indexForKey(navigationKey);
        const auto following=bank.entries()[size_t((sorted+1)%int(bank.entries().size()))].key;
        next->onClick();if(synth->selectedPresetKey()!=following)return 94;
        previous->onClick();if(synth->selectedPresetKey()!=navigationKey)return 95;
        if(synth->saveNewPreset("Navigation User Preset").failed())return 96;
        const auto userKey=synth->selectedPresetKey();
        previous->onClick();
        if(synth->selectedPresetKey()!=bank.entries()[bank.entries().size()-2].key)return 97;
        next->onClick();if(synth->selectedPresetKey()!=userKey)return 98;
        editor.reset();editor.reset(synth->createEditor());
        next=nullptr;
        for(auto* child:editor->getChildren())
            if(child->getName()=="Next preset")next=dynamic_cast<juce::Button*>(child);
        if(!next)return 99;
        next->onClick();if(synth->selectedPresetKey()!=bank.entries().front().key)return 100;
    }
    other.reset();synth.reset();folder.deleteRecursively();
    std::cout<<"Preset save/load/replace/rename/delete/order/restore, cross-instance conflict and analogue recall passed\n";
    return 0;
}
