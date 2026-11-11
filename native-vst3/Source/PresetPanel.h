#pragma once
#include "PluginProcessor.h"
class PresetPanel final : public juce::Component,private juce::ListBoxModel {
    SpaceInfinityProcessor& processor;
    std::function<void()> changed;
    juce::ListBox list{"Preset bank",this};juce::TextEditor name;
    juce::Label message;
    std::array<juce::TextButton,11> actions;
    std::unique_ptr<juce::FileChooser> chooser;
    int selected=-1;
    int getNumRows() override {return int(processor.presetBank().entries().size());}
    void paintListBoxItem(int row,juce::Graphics& g,int width,int height,bool isSelected) override {
        const auto& bank=processor.presetBank().entries();if(row<0||row>=int(bank.size()))return;
        if(isSelected)g.fillAll(juce::Colour(0xff5f4034));
        g.setColour(juce::Colour(0xffeeeeea));g.drawText(juce::String(row+1)+". "+bank[size_t(row)].name,8,0,width-175,height,juce::Justification::centredLeft);
        g.setColour(juce::Colour(0xffaaa99f));g.drawText(bank[size_t(row)].category,width-160,0,150,height,juce::Justification::centredRight);
    }
    void selectedRowsChanged(int row) override {selected=row;const auto& bank=processor.presetBank().entries();if(row>=0&&row<int(bank.size()))name.setText(bank[size_t(row)].name,false);}
    void listBoxItemDoubleClicked(int row,const juce::MouseEvent&) override {show(processor.loadBankPreset(row),"Loaded preset.");}
    void show(const juce::Result& result,const juce::String& success){
        message.setText(result.wasOk()?success:result.getErrorMessage(),juce::dontSendNotification);
        if(result.wasOk()){list.updateContent();list.repaint();if(changed)changed();}
    }
    void refresh(){const auto key=processor.selectedPresetKey();show(processor.presetBank().reload(),"Preset bank refreshed.");selected=processor.presetBank().indexForKey(key);list.selectRow(selected);}
    void add(){const auto result=processor.saveNewPreset(name.getText());show(result,"Saved new preset, including samples and edited wavetables.");if(result.wasOk())list.selectRow(processor.presetBank().indexForKey(processor.selectedPresetKey()));}
    void remove(){const auto result=processor.presetBank().remove(selected);show(result,"Removed. User files are backed up in the Deleted folder; factory sounds can be restored.");if(result.wasOk())list.selectRow(juce::jmin(selected,getNumRows()-1));}
    void move(int direction){if(selected<0)return;const auto result=processor.presetBank().move(selected,direction);show(result,"Preset order saved.");if(result.wasOk())list.selectRow(selected+direction);}
    void saveAs(){
        auto filename=juce::File::createLegalFileName(name.getText().trim());
        if(filename.isEmpty()||filename=="."||filename=="..")filename="Untitled";
        chooser=std::make_unique<juce::FileChooser>("Save Space Infinity preset as",processor.presetExportFolder().getChildFile(filename.endsWithIgnoreCase(".sipreset")?filename:filename+".sipreset"),"*.sipreset");
        const juce::Component::SafePointer<PresetPanel> safe(this);
        chooser->launchAsync(juce::FileBrowserComponent::saveMode|juce::FileBrowserComponent::canSelectFiles|juce::FileBrowserComponent::warnAboutOverwriting,[safe](const juce::FileChooser& c){
            if(safe==nullptr||c.getResult()==juce::File{})return;
            const auto selected=c.getResult(),file=selected.withFileExtension(".sipreset");
            const auto write=[safe,file]{if(safe!=nullptr)safe->show(safe->processor.exportPreset(file,true),"Saved: "+file.getFullPathName()+". Use Import to add it to the browser.");};
            // If appending the extension changes the target, confirm that exact file.
            if(file!=selected&&file.existsAsFile()){
                juce::AlertWindow::showOkCancelBox(juce::MessageBoxIconType::QuestionIcon,"Replace preset?","Replace "+file.getFullPathName()+"?","Replace","Cancel",safe.getComponent(),juce::ModalCallbackFunction::create([write](int result){if(result!=0)write();}));
            }else write();
        });
    }
    void importPreset(){
        chooser=std::make_unique<juce::FileChooser>("Import Space Infinity preset",juce::File{},"*.sipreset");
        const juce::Component::SafePointer<PresetPanel> safe(this);
        chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,[safe](const juce::FileChooser& c){
            if(safe==nullptr||c.getResult()==juce::File{})return;
            juce::MemoryBlock bytes;const auto file=c.getResult();
            if(file.getSize()>64*1024*1024||!file.loadFileAsData(bytes)){safe->show(juce::Result::fail("Cannot read preset file."),{});return;}
            auto xml=juce::AudioProcessor::getXmlFromBinary(bytes.getData(),int(bytes.getSize()));
            if(!xml||!xml->hasTagName("PARAMETERS")){safe->show(juce::Result::fail("This is not a Space Infinity preset."),{});return;}
            juce::String key;safe->show(safe->processor.presetBank().add(file.getFileNameWithoutExtension(),bytes,key),"Imported preset.");safe->list.selectRow(safe->processor.presetBank().indexForKey(key));
        });
    }
public:
    PresetPanel(SpaceInfinityProcessor& p,std::function<void()> callback):processor(p),changed(std::move(callback)){
        setSize(710,465);addAndMakeVisible(list);list.setRowHeight(25);list.setColour(juce::ListBox::backgroundColourId,juce::Colour(0xff161a1d));
        name.setTextToShowWhenEmpty("Enter a preset name",juce::Colours::grey);name.setInputRestrictions(64);addAndMakeVisible(name);addAndMakeVisible(message);
        const char* labels[]{"SAVE NEW","SAVE AS...","REPLACE","RENAME","DELETE","MOVE UP","MOVE DOWN","A–Z","RESTORE FACTORY","REFRESH","IMPORT"};
        for(size_t i=0;i<actions.size();++i){actions[i].setButtonText(labels[i]);addAndMakeVisible(actions[i]);}
        actions[0].onClick=[this]{add();};actions[1].onClick=[this]{saveAs();};
        actions[1].setTooltip("Save the current sound, including samples and wavetables, to a folder and filename you choose.");
        actions[2].onClick=[this]{show(processor.replaceBankPreset(selected),"Replaced selected user preset with the current sound.");};
        actions[3].onClick=[this]{show(processor.presetBank().rename(selected,name.getText()),"Preset renamed.");};
        actions[4].onClick=[this]{remove();};actions[5].onClick=[this]{move(-1);};actions[6].onClick=[this]{move(1);};
        actions[7].onClick=[this]{show(processor.presetBank().sortByName(),"Presets sorted A-Z.");list.selectRow(processor.presetBank().indexForKey(processor.selectedPresetKey()));};
        actions[8].onClick=[this]{show(processor.presetBank().restoreFactory(),"Factory bank restored. User presets retained.");};
        actions[9].onClick=[this]{refresh();};actions[10].onClick=[this]{importPreset();};
        message.setColour(juce::Label::textColourId,juce::Colour(0xffd2c7b2));
        message.setText("Save New adds to your bank. Save As saves to any folder. Import adds a saved file to the browser.",juce::dontSendNotification);
        list.selectRow(processor.presetBank().indexForKey(processor.selectedPresetKey()));
    }
    void paint(juce::Graphics& g) override {g.fillAll(juce::Colour(0xff252b31));}
    void resized() override {
        list.setBounds(12,12,686,270);name.setBounds(12,294,686,27);
        for(int i=0;i<11;++i){const bool primary=i<5;const int column=primary?i:i-5,cell=primary?138:115;actions[size_t(i)].setBounds(12+column*cell,primary?332:368,cell-8,29);}
        message.setBounds(12,408,686,48);
    }
};
