#include "../Source/PluginProcessor.h"
#include <iostream>
#include <map>
#include <stdexcept>
namespace {
void check(bool value,const juce::String& message){if(!value)throw std::runtime_error(message.toStdString());}
float value(SpaceInfinityProcessor& p,const char* id){return p.state.getRawParameterValue(id)->load();}
}
int runTranceBankSmoke(int group,const juce::File& profile){
 try {
    auto p=std::make_unique<SpaceInfinityProcessor>();
    const std::map<juce::String,int> expected{{"Offbeat Bass Stabs",15},{"Sub Bass Stabs",5},{"Rolling Bass",15},{"Uplifting Leads",10},{"Hard Trance Leads",10},{"90s German Leads",10},{"Trance Plucks",20},{"Airy Pads",10},{"Dark Pads",10},{"Strings",10},{"Trance Pianos",5},{"Arps and Sequences",20},{"Choir Stabs",10},{"Acid",10}};
    std::map<juce::String,int> counts;juce::Array<juce::var> rows;juce::StringArray fingerprints;
    check(p->presetNames().size()==161,"160 factory sounds plus Init");
    for(int index=1;index<p->presetNames().size();++index){
        const auto category=p->presetCategory(index),name=p->presetNames()[index];++counts[category];
        const bool bass=category.contains("Bass"),pad=category.contains("Pads"),pattern=category=="Arps and Sequences";
        if((group==0&&!bass&&category!="Acid"&&!pattern)||(group==1&&(bass||category=="Acid"||pattern)))continue;
        const auto selectedNames=juce::SystemStats::getEnvironmentVariable("SPACE_INFINITY_PROFILE_NAMES",{});if(profile!=juce::File{}&&selectedNames.isNotEmpty()&&!juce::StringArray::fromTokens(selectedNames,",","").contains(name))continue;
        p->loadPreset(index);
        check(p->state.state.getProperty("factoryReference").toString().isNotEmpty(),name+": documented reference");
        if(bass)check(value(*p,"patternMode")==0&&value(*p,"gateEnabled")==0&&value(*p,"pumpEnabled")==0,name+": bass must follow customer MIDI");
        if(category.contains("Leads")&&name!="Reptile Signal")check(value(*p,"attack")<=.003f&&value(*p,"ampLong")==0,name+": fast lead envelope");
        auto state=p->state.copyState();for(const auto* property:{"presetKey","factoryReference","factoryDescription"})state.removeProperty(property,nullptr);const auto settings=state.createXml()->toString();check(!fingerprints.contains(settings),name+": distinct state");fingerprints.add(settings);
        p->prepareToPlay(48000,256);p->takeOutputPeak();juce::AudioBuffer<float> audio(2,256);double energy=0,mono=0;float peak=0,maxStep=0,previous=0;int notes=0;double first=0,last=0;
        std::unique_ptr<juce::AudioFormatWriter> preview;
        if(profile!=juce::File{}){const auto folder=profile.getParentDirectory().getChildFile("Previews");folder.createDirectory();const auto file=folder.getChildFile(name+".wav");file.deleteFile();auto stream=file.createOutputStream();juce::WavAudioFormat format;preview.reset(format.createWriterFor(stream.get(),48000,2,24,{},0));check(preview!=nullptr,"preview writer");stream.release();}
        const int length=group>=2?1125:pad?1125:375;
        for(int block=0;block<length;++block){juce::MidiBuffer midi;
            const int note=bass||category=="Acid"?40:category=="Strings"?69:60;
            const bool chord=(group!=3||pattern)&&!bass&&category!="Acid"&&value(*p,"voiceMode")==0;
            const int cycle=pad?length:188,position=block%cycle;
            if(position==0){midi.addEvent(juce::MidiMessage::noteOn(1,note,.82f),0);if(chord)for(int offset:{7,12})midi.addEvent(juce::MidiMessage::noteOn(1,note+offset,.75f),0);}
            if(position==(pad?length-96:120)){midi.addEvent(juce::MidiMessage::allNotesOff(1),0);}
            p->processBlock(audio,midi);if(preview)check(preview->writeFromAudioSampleBuffer(audio,0,256),"preview write");for(const auto event:p->generatedNotes())if(event.getMessage().isNoteOn()){++notes;if(pattern&&position>120)check(false,name+": pattern stops after MIDI release");}
            for(int i=0;i<256;++i){const float l=audio.getSample(0,i),r=audio.getSample(1,i);check(std::isfinite(l)&&std::isfinite(r),name+": finite output");peak=std::max(peak,std::max(std::abs(l),std::abs(r)));maxStep=std::max(maxStep,std::abs(l-previous));previous=l;const double e=.5*(l*l+r*r);energy+=e;mono+=.25*(l+r)*(l+r);if(block>=24&&block<100)first+=e;if(block>=length-164&&block<length-88)last+=e;}
        }
        check(peak>.0001f&&peak<=1,name+": audible and bounded");
        if(bass)check(notes==0&&mono>energy*.65,name+": no generated bass notes and mono compatibility");
        if(pattern)check(notes>2,name+": pattern produces notes");
        const float rawPeak=p->takeOutputPeak();if(group<2)check(std::isfinite(rawPeak)&&peak<=.990001f&&peak>.08f,name+": peak-calibrated audible output and clean safety ceiling");
        auto* row=new juce::DynamicObject;row->setProperty("index",index);row->setProperty("name",name);row->setProperty("category",category);row->setProperty("gain",value(*p,"gain"));row->setProperty("preset_trim",value(*p,"presetTrim"));row->setProperty("peak",peak);row->setProperty("raw_peak",rawPeak);row->setProperty("rms",std::sqrt(energy/(length*256)));row->setProperty("mono_ratio",mono/std::max(energy,1e-12));row->setProperty("maximum_step",maxStep);row->setProperty("generated_notes",notes);row->setProperty("late_early_energy",last/std::max(first,1e-12));rows.add(juce::var(row));
        std::cout<<index<<" "<<name<<": peak "<<peak<<", RMS "<<std::sqrt(energy/(length*256))<<", generated "<<notes<<std::endl;
    }
    check(counts==expected,"exact requested category counts");
    if(profile!=juce::File{}){profile.getParentDirectory().createDirectory();check(profile.replaceWithText(juce::JSON::toString(rows)),"profile export");}
    if(group!=0){
        const auto set=[&](const juce::String& id,float v){auto* a=p->state.getParameter(id);a->setValueNotifyingHost(a->convertTo0to1(v));};
        for(int source:{21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44}){
            p->loadPreset(0);set("source",float(source));set("attack",0);set("release",.1f);set("ampSustain",1);set("ampVelocity",0);set("cutoff",16000);set("sampleLoop",source==23||source==24||(source>=27&&source<=29)?1:0);set("gain",.35f);
            for(int key:{48,60,72}){
                p->prepareToPlay(48000,256);juce::AudioBuffer<float> b(2,256);double energy=0;
                for(int block=0;block<188;++block){juce::MidiBuffer midi;if(block==0)midi.addEvent(juce::MidiMessage::noteOn(1,key,.82f),0);p->processBlock(b,midi);for(int ch=0;ch<2;++ch)for(int i=0;i<256;++i){const float sample=b.getSample(ch,i);check(std::isfinite(sample),"multisample pitch-zone finite");energy+=sample*sample;}}
                check(energy>.1,"all factory instruments and uploaded choir one-shots play across key zones");
            }
            if(source==23||source==24||(source>=27&&source<=29)){p->prepareToPlay(48000,256);juce::AudioBuffer<float> b(2,256);double early=0,late=0;float previous=0,jump=0;for(int block=0;block<1875;++block){juce::MidiBuffer midi;if(block==0)midi.addEvent(juce::MidiMessage::noteOn(1,69,.82f),0);p->processBlock(b,midi);for(int i=0;i<256;++i){const float sample=b.getSample(0,i);if(block>900)jump=std::max(jump,std::abs(sample-previous));previous=sample;if(block>200&&block<600)early+=sample*sample;if(block>1400)late+=sample*sample;}}check(late>early*.12&&jump<.2f,"string sustain loop remains audible and continuous");}
        }
    }
    if(group==1){
        for(const auto* name:{"Azure Ascension","Saucer Alloy","Silver Alone","Ivory Dreamline","Cathedral Strike","Skywave Vista"}){
            p->loadPreset(p->presetNames().indexOf(name));p->prepareToPlay(48000,256);juce::AudioBuffer<float> b(2,256);std::array<double,24> energy{};
            const int period=juce::String(name)=="Skywave Vista"?375:188;
            for(int phrase=0;phrase<24;++phrase)for(int block=0;block<period;++block){juce::MidiBuffer midi;if(block==0)midi.addEvent(juce::MidiMessage::noteOn(1,69,.82f),0);if(block==period-60)midi.addEvent(juce::MidiMessage::allNotesOff(1),0);p->processBlock(b,midi);for(int ch=0;ch<2;++ch)for(int i=0;i<256;++i){const float sample=b.getSample(ch,i);check(std::isfinite(sample),"repeated note finite");if(block>16&&block<period-65)energy[size_t(phrase)]+=sample*sample;}}
            double first=0,last=0;for(int n=4;n<12;++n)first+=energy[size_t(n)];for(int n=16;n<24;++n)last+=energy[size_t(n)];check(first>0&&last>first*.55&&last<first*1.85,juce::String(name)+": 24 fresh notes retain their level");std::cout<<name<<" 24-note energy ratio "<<last/first<<std::endl;
        }
    }
    // A factory grain patch embeds its source in exported state; loading a dry patch clears that source.
    p->loadPreset(p->presetNames().indexOf("Granular Stardust"));juce::MemoryBlock grain;p->getStateInformation(grain);auto recalled=std::make_unique<SpaceInfinityProcessor>();recalled->setStateInformation(grain.getData(),int(grain.getSize()));check(value(*recalled,"source")==4,"granular source recall");
    p->loadPreset(1);juce::MemoryBlock dry;p->getStateInformation(dry);check(dry.getSize()<grain.getSize(),"factory change clears previous imported audio");
    std::cout<<"Trance bank: requested counts, playable output, MIDI behaviour and grain recall passed\n";return 0;
 }catch(const std::exception& e){std::cerr<<"Trance bank: "<<e.what()<<std::endl;return 150;}
}
