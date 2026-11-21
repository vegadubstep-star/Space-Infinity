#include "../../native-vst3/Source/PluginProcessor.h"
#include <iostream>
#include <cmath>

static void set(SpaceInfinityProcessor& p,const juce::String& id,float value){
    auto* a=p.state.getParameter(id);if(!a)throw std::runtime_error("Unknown parameter: "+id.toStdString());
    a->setValueNotifyingHost(a->convertTo0to1(value));
}
static bool verifyOnly=false,openMacro=false;
// Authoring stress probe: sweep controls between their endpoints, exercise lower
// bass registers and velocity/legato articulations. No file-sized audio buffer.
static juce::var researchStress(const juce::MemoryBlock& data,const juce::File& directory,const juce::String& category){
    const bool bass=category.containsIgnoreCase("Bass"),acid=category=="Acid",pluck=category.containsIgnoreCase("Pluck"),pad=category.containsIgnoreCase("Pads");
    const auto file=directory.getChildFile("Stress.wav");file.deleteFile();auto stream=file.createOutputStream();if(!stream)throw std::runtime_error("Cannot write stress audio");
    std::unique_ptr<juce::AudioFormatWriter> writer(juce::WavAudioFormat{}.createWriterFor(stream.release(),48000,2,24,{},0));if(!writer)throw std::runtime_error("Cannot create stress WAV writer");
    float raw=0,peak=0;double energy=0;int samples=0;
    for(bool extremes:{false,true})for(int root:bass?std::vector<int>{28,40,52}:acid?std::vector<int>{33,45,57}:std::vector<int>{48,60,72}){
        auto p=std::make_unique<SpaceInfinityProcessor>(directory.getChildFile("StressLibrary"));p->setStateInformation(data.getData(),int(data.getSize()));
        if(extremes){for(int m=0;m<3;++m){set(*p,"freeMacro"+juce::String(m),1);set(*p,"macro"+juce::String(m),1);}}
        p->prepareToPlay(48000,256);juce::AudioBuffer<float> audio(2,256);
        constexpr int blocks=563; // Three seconds per register/controller state.
        for(int block=0;block<blocks;++block){
            const float phase=float(block)/float(blocks-1),triangle=1-std::abs(2*phase-1);
            if(acid)set(*p,"cutoff",80*std::pow(200.0f,triangle));
            if(pluck&&!extremes)set(*p,"freeMacro0",triangle);
            juce::MidiBuffer midi;
            // Detached short notes alternate with held/sliding intervals.
            const bool noteOn=pad?(block==0||block==280):block%60==0;
            const bool noteOff=pad?block==262:block%60==38;
            if(noteOn)for(int delta:!bass&&!acid?std::vector<int>{0,7,12,16}:std::vector<int>{0})
                midi.addEvent(juce::MidiMessage::noteOn(1,root+delta,juce::uint8(block%120==0?127:83)),0);
            if(noteOff)for(int delta:!bass&&!acid?std::vector<int>{0,7,12,16}:std::vector<int>{0})
                midi.addEvent(juce::MidiMessage::noteOff(1,root+delta),0);
            if(acid&&block%120==20)midi.addEvent(juce::MidiMessage::noteOn(1,root+7,juce::uint8(127)),0);
            if(acid&&block%120==44)midi.addEvent(juce::MidiMessage::noteOff(1,root+7),0);
            p->processBlock(audio,midi);raw=std::max(raw,p->takeOutputPeak());
            if(!writer->writeFromAudioSampleBuffer(audio,0,256))throw std::runtime_error("Stress WAV write failed");
            for(int c=0;c<2;++c)for(int n=0;n<256;++n){const float v=audio.getSample(c,n);if(!std::isfinite(v))throw std::runtime_error("Non-finite stress audio");peak=std::max(peak,std::abs(v));energy+=v*v;++samples;}
        }
    }
    auto* result=new juce::DynamicObject;result->setProperty("pre_safety_peak",raw);result->setProperty("peak",peak);result->setProperty("rms",std::sqrt(energy/juce::jmax(1,samples)));result->setProperty("registers",3);return juce::var(result);
}
static juce::var render(const juce::MemoryBlock& data,const juce::File& directory,const juce::String& id,const juce::String& category,bool chord){
    auto p=std::make_unique<SpaceInfinityProcessor>(directory.getChildFile("IsolatedLibrary"));p->setStateInformation(data.getData(),int(data.getSize()));if(openMacro)set(*p,"freeMacro0",1);p->prepareToPlay(48000,256);
    const bool bass=category.containsIgnoreCase("Bass"),pad=category.containsIgnoreCase("Pads")||category=="Strings"||category=="Voices"||category=="Textures";
    const int root=bass?40:category=="Percussion"?60:60;const int frames=48000*(verifyOnly?(pad?6:4):(pad?10:7)),off=48000*(verifyOnly?(pad?4:2):(pad?5:3));
    const auto file=directory.getChildFile(id+(chord?"-chord.wav":"-single.wav"));file.deleteFile();
    std::unique_ptr<juce::AudioFormatWriter> writer;
    if(!verifyOnly){auto stream=file.createOutputStream();if(!stream)throw std::runtime_error("Cannot write audition");writer.reset(juce::WavAudioFormat{}.createWriterFor(stream.release(),48000,2,16,{},0));}
    float peak=0,raw=0,jump=0,last[2]{};double energy=0;juce::AudioBuffer<float> audio(2,256);
    for(int pos=0;pos<frames;pos+=256){const int count=juce::jmin(256,frames-pos);audio.setSize(2,count,false,false,true);audio.clear();juce::MidiBuffer midi;
        const auto event=[&](int when,bool on){if(when>=pos&&when<pos+count){for(int delta:chord&&!bass?std::vector<int>{0,7,12,16}:std::vector<int>{0})midi.addEvent(on?juce::MidiMessage::noteOn(1,root+delta,juce::uint8(127)):juce::MidiMessage::noteOff(1,root+delta),when-pos);}};
        event(0,true);event(off,false);
        // A second articulation checks retriggering and pattern cleanup.
        if(!pad){event(48000,true);event(60000,false);event(72000,true);event(84000,false);}
        p->processBlock(audio,midi);raw=std::max(raw,p->takeOutputPeak());
        for(int c=0;c<2;++c)for(int n=0;n<count;++n){const float v=audio.getSample(c,n);if(!std::isfinite(v))throw std::runtime_error("Non-finite audio");peak=std::max(peak,std::abs(v));jump=std::max(jump,std::abs(v-last[c]));last[c]=v;energy+=v*v;}
        if(writer&&!writer->writeFromAudioSampleBuffer(audio,0,count))throw std::runtime_error("Audition write failed");
    }
    p->releaseResources();auto* row=new juce::DynamicObject;row->setProperty("peak",peak);row->setProperty("pre_safety_peak",raw);row->setProperty("rms",std::sqrt(energy/(frames*2)));row->setProperty("max_sample_step",jump);row->setProperty("frames",frames);return juce::var(row);
}
int main(int argc,char** argv){
 try{
    juce::ScopedJuceInitialiser_GUI init;if(argc<4)return 1;const juce::File root(argv[1]),design(argv[2]),out(argv[3]);out.createDirectory();
    openMacro=argc>4&&juce::String(argv[4])=="--open";verifyOnly=argc>4&&juce::String(argv[4])=="--verify";const bool factoryOnly=argc>4&&juce::String(argv[4])=="--factory-only";const bool exportOnly=factoryOnly||(argc>4&&juce::String(argv[4])=="--export-only"),auditionOnly=argc>4&&juce::String(argv[4])=="--audition",stressOnly=argc>4&&juce::String(argv[4])=="--research-stress";
    const auto bank=juce::JSON::parse(design);juce::Array<juce::var> report;int count=0;
    auto p=std::make_unique<SpaceInfinityProcessor>(out.getChildFile("IsolatedLibrary"));
    for(const auto& row:*bank["presets"].getArray()){
        p=std::make_unique<SpaceInfinityProcessor>(out.getChildFile("IsolatedLibrary"));p->loadPreset(row.hasProperty("factory_index")?int(row["factory_index"]):0);const auto id=row["id"].toString(),category=row["category"].toString(),name=row["name"].toString(),family=row["family"].toString();
        auto& state=p->state.state;
        if(!factoryOnly){
        const auto wt=row["wavetable"].toString();if(wt.isNotEmpty()&&wt!="null"&&p->importWavetable(root.getChildFile("native-vst3/Presets/Heritage200/Wavetables/"+wt+".wav"),512).failed())throw std::runtime_error("Wavetable import failed");
        for(const auto& setting:row["settings"].getDynamicObject()->getProperties())set(*p,setting.name.toString(),float(setting.value));
        state.setProperty("presetDisplayName",name,nullptr);state.setProperty("presetKey","Uuser_heritage200_"+id,nullptr);state.setProperty("creator",row["creator"],nullptr);state.setProperty("tags",row["tags"],nullptr);state.setProperty("factoryReference",family+" architecture influence; original Infinity programming",nullptr);state.setProperty("factoryDescription",row["description"],nullptr);
        for(int m=0;m<3;++m)state.setProperty("macroLabel"+juce::String(m),juce::StringArray{"TONE","CONTOUR",wt.isNotEmpty()?"SPECTRUM":"BODY"}[m],nullptr);
        if(const auto* labels=row["macro_labels"].getArray())for(int m=0;m<juce::jmin(3,labels->size());++m)state.setProperty("macroLabel"+juce::String(m),(*labels)[m],nullptr);
        for(int source=0;source<10;++source)for(int slot=0;slot<16;++slot)p->assignFlexibleSource(source,slot,-1);std::array<int,9> slots{};const auto assign=[&](int source,const juce::String& target,float depth){const int index=p->flexibleTargetIndex(target);if(index<0)throw std::runtime_error("Invalid modulation target "+target.toStdString());const int slot=slots[size_t(source)]++;p->assignFlexibleSource(source,slot,index);set(*p,p->flexibleDepthId(source,slot),depth);};
        if(!row.hasProperty("factory_index")){
        if(!row.hasProperty("macro_labels"))assign(0,"cutoff",.12f);assign(1,"filterEnvAmount",.12f);assign(1,"filterDecay",.07f);
        if(wt.isNotEmpty()&&wt!="null")assign(2,"wavetableScan",.2f);else assign(2,"filterInputDrive",.08f);
        assign(6,"reverb",.12f);assign(7,"filterEnvAmount",.10f);assign(8,"osc1Width",.4f);
        }
        if(const auto* mods=row[row.hasProperty("macros")?"macros":"modulation"].getArray())for(const auto& mod:*mods)assign(int(mod["source"]),mod["target"].toString(),float(mod["depth"]));
        if(row["lfo"].isObject()){const auto lfo=row["lfo"];set(*p,p->lfoParameterId(0,"Enabled"),1);set(*p,p->lfoParameterId(0,"Sync"),lfo.hasProperty("hz")?0:1);set(*p,p->lfoParameterId(0,lfo.hasProperty("hz")?"Speed":"Bars"),float(lfo[lfo.hasProperty("hz")?"hz":"bars"]));assign(3,lfo["target"].toString(),float(lfo["depth"]));}
        }
        const auto folder=out.getChildFile("Presets/"+family+"/"+category);folder.createDirectory();const auto file=folder.getChildFile(id+" "+name+".sipreset");if(p->exportPreset(file,true).failed())throw std::runtime_error("Export failed");
        juce::MemoryBlock data;file.loadFileAsData(data);auto recalled=std::make_unique<SpaceInfinityProcessor>(out.getChildFile("IsolatedLibrary"));recalled->setStateInformation(data.getData(),int(data.getSize()));
        for(auto* parameter:p->getParameters())if(auto* a=dynamic_cast<juce::RangedAudioParameter*>(parameter))if(std::abs(p->state.getRawParameterValue(a->paramID)->load()-recalled->state.getRawParameterValue(a->paramID)->load())>1e-5f)throw std::runtime_error("Parameter recall failed "+a->paramID.toStdString());
        for(int s=0;s<9;++s)for(int slot=0;slot<16;++slot)if(state.getProperty(p->flexibleTargetId(s,slot))!=recalled->state.state.getProperty(p->flexibleTargetId(s,slot)))throw std::runtime_error("Modulation recall failed");
        recalled.reset();auto* r=new juce::DynamicObject;r->setProperty("id",id);r->setProperty("name",name);r->setProperty("category",category);r->setProperty("file",file.getRelativePathFrom(out).replaceCharacter('\\','/'));if(stressOnly)r->setProperty("stress",researchStress(data,out,category));else if(!exportOnly){if(!verifyOnly)r->setProperty("single",render(data,out,id,category,false));if(!auditionOnly)r->setProperty("chord",render(data,out,id,category,true));}report.add(juce::var(r));std::cout<<++count<<" "<<id<<" "<<name<<std::endl;
    }
    auto* result=new juce::DynamicObject;result->setProperty("presets",report);out.getChildFile(stressOnly?"Measurements-stress.json":verifyOnly?"Measurements-verified.json":exportOnly?"Exports.json":auditionOnly?"Measurements-auditions.json":"Measurements.json").replaceWithText(juce::JSON::toString(juce::var(result)));return count==bank["presets"].size()?0:2;
 }catch(const std::exception& e){std::cerr<<e.what()<<std::endl;return 3;}
}
