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
    other.reset();synth.reset();folder.deleteRecursively();
    std::cout<<"Preset save/load/replace/rename/delete/order/restore, cross-instance conflict and analogue recall passed\n";
    return 0;
}
