#include "../Source/PluginProcessor.h"
#include "../Source/PresetPanel.h"
#include <iostream>
int runPresetSmoke(){
    const auto folder=juce::File::getSpecialLocation(juce::File::tempDirectory).getNonexistentChildFile("SpaceInfinityBankTest",{},false);
    folder.createDirectory();
    // Heap allocation also exercises multiple simultaneous plugin instances without large test stack frames.
    auto synth=std::make_unique<SpaceInfinityProcessor>(folder);
    synth->loadPreset(12);auto* cutoff=synth->state.getParameter("cutoff");cutoff->setValueNotifyingHost(cutoff->convertTo0to1(1234));
    if(synth->saveNewPreset("My Analogue Lead").failed())return 70;
    auto& bank=synth->presetBank();const auto key=synth->selectedPresetKey();
    if(bank.entries().size()!=121||bank.indexForKey(key)!=120)return 71;
    if(bank.move(120,-1).failed()||bank.indexForKey(key)!=119)return 72;
    if(bank.rename(119,"Personal Lead").failed())return 73;
    auto other=std::make_unique<SpaceInfinityProcessor>(folder);
    const int loaded=other->presetBank().indexForKey(key);
    if(loaded!=119||other->loadBankPreset(loaded).failed()||std::abs(other->state.getRawParameterValue("cutoff")->load()-1234)>.01)return 74;
    if(other->presetBank().sortByName().failed())return 75;
    if(bank.sortByName().wasOk())return 76; // A stale instance must not overwrite another instance's edits.
    if(bank.reload().failed()||bank.indexForKey(key)<0)return 77;
    cutoff->setValueNotifyingHost(cutoff->convertTo0to1(2345));
    if(synth->replaceBankPreset(bank.indexForKey(key)).failed())return 78;
    if(other->presetBank().reload().failed()||other->loadBankPreset(other->presetBank().indexForKey(key)).failed()||std::abs(other->state.getRawParameterValue("cutoff")->load()-2345)>.01)return 79;
    if(bank.remove(bank.indexForKey("F0")).failed()||bank.indexForKey("F0")!=-1||bank.restoreFactory().failed()||bank.entries().size()!=121)return 80;
    if(bank.remove(bank.indexForKey(key)).failed()||bank.entries().size()!=120)return 81;
    if(folder.getChildFile("Deleted").findChildFiles(juce::File::findFiles,false,"*.sipreset").isEmpty())return 82;
    if(synth->replaceBankPreset(0).wasOk())return 83;
    PresetPanel panel(*synth,[]{});panel.setSize(710,465);
    const auto audition=juce::File::getCurrentWorkingDirectory().getChildFile("audition");audition.createDirectory();
    auto stream=audition.getChildFile("Preset-Manager.png").createOutputStream();if(stream)juce::PNGImageFormat{}.writeImageToStream(panel.createComponentSnapshot(panel.getLocalBounds()),*stream);
    juce::AudioBuffer<float> audio(2,256);synth->loadPreset(0);synth->prepareToPlay(48000,256);
    if(synth->getLatencySamples()<1)return 84;
    for(int i=0;i<120;++i){synth->loadPreset(i);if(synth->state.getRawParameterValue("analogueUpgrade")->load()!=1)return 85;}
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
        if(synth->loadBankPreset(bank.indexForKey("F42")).failed())return 88;
        next->onClick();if(synth->selectedPresetKey()!="F43")return 89;
        previous->onClick();if(synth->selectedPresetKey()!="F42")return 90;
        synth->getStateInformation(state);
        synth->loadPreset(0);synth->setStateInformation(state.getData(),int(state.getSize()));
        previous->onClick();if(synth->selectedPresetKey()!="F41")return 91;
        cutoff->setValueNotifyingHost(cutoff->convertTo0to1(3210));
        next->onClick();if(synth->selectedPresetKey()!="F42")return 92;
        if(bank.sortByName().failed())return 93;
        const int sorted=bank.indexForKey("F42");
        const auto following=bank.entries()[size_t((sorted+1)%int(bank.entries().size()))].key;
        next->onClick();if(synth->selectedPresetKey()!=following)return 94;
        previous->onClick();if(synth->selectedPresetKey()!="F42")return 95;
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
