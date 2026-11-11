#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace infinity {
constexpr double pi=3.14159265358979323846;
struct Envelope {
    float level=0,releaseStart=-1; bool decay=false;
    float next(bool released,double sr,float a,float d,float s,float r) noexcept {
        if(released){if(releaseStart<0)releaseStart=level;level=std::max(0.0f,level-releaseStart/float(std::max(1.0,double(r)*sr)));}
        else if(!decay){level=std::min(1.0f,level+float(1.0/std::max(1.0,double(a)*sr)));decay=level>=1;}
        else level+=std::clamp(s-level,-float(1.0/std::max(1.0,double(d)*sr)),float(1.0/std::max(1.0,double(d)*sr)));
        return level;
    }
};
struct Granulator {
    struct Grain { double position=0,age=0,length=1,step=1;float pan=0;bool active=false; };
    std::array<Grain,32> grains{}; double until=0;uint32_t rng=23719;
    float random() noexcept {rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return float(rng>>8)/16777215.0f;}
    std::array<float,2> next(const float* data,int start,int end,double ratio,double sr,
        float position,float sizeMs,float density,float spray,float spread,bool reverse) noexcept {
        std::array<float,2> out{}; if(end-start<2)return out;
        if(until--<=0){
            for(auto& grain:grains)if(!grain.active){
                const double length=std::max(8.0,double(sizeMs)*sr*.001);
                const float offset=std::clamp(position+(random()*2-1)*spray,0.0f,1.0f);
                grain={start+double(end-start)*offset,0,length,reverse?-std::abs(ratio):std::abs(ratio),
                    (random()*2-1)*spread,true};break;
            }
            until=sr/std::max(1.0f,density);
        }
        const double range=end-start;
        for(auto& grain:grains)if(grain.active){
            double offset=std::fmod(grain.position-start,range);if(offset<0)offset+=range;
            const double index=start+offset;const int i=int(index);
            const float value=data[i]+float(index-i)*(data[std::min(i+1,end)]-data[i]);
            const float window=float(.5-.5*std::cos(2*pi*grain.age/grain.length));
            out[0]+=value*window*std::sqrt((1-grain.pan)*.5f);
            out[1]+=value*window*std::sqrt((1+grain.pan)*.5f);
            grain.position+=grain.step;grain.age+=1;if(grain.age>=grain.length)grain.active=false;
        }
        const float norm=1.0f/std::sqrt(std::max(1.0f,density*sizeMs*.001f));
        out[0]*=norm;out[1]*=norm;return out;
    }
};
// Fixed-capacity bank: table edits/imports occur under the processor's sample lock.
struct Wavetable {
    static constexpr int points=512,maxFrames=64;
    std::array<std::array<float,points>,maxFrames> frames{};int count=1;
    Wavetable(){for(int i=0;i<points;++i)frames[0][size_t(i)]=float(std::sin(2*pi*i/points));}
    float sample(double phase,float scan) const noexcept {
        phase-=std::floor(phase);const double x=phase*points;const int i=int(x)%points;
        const float f=float(x-i),frame=std::clamp(scan,0.0f,1.0f)*float(count-1);
        const int a=int(frame),b=std::min(a+1,count-1);
        const auto read=[&](int n){const auto& t=frames[size_t(n)];return t[size_t(i)]+f*(t[size_t((i+1)%points)]-t[size_t(i)]);};
        return read(a)+(read(b)-read(a))*(frame-a);
    }
};
struct Pattern {
    struct Held {bool down=false;float velocity=0;};
    std::array<Held,2048> held{};
    std::array<int,16> pitch{};std::array<float,16> velocity{};std::array<bool,16> enabled{};
    int mode=0,order=0,octaves=1,length=16,activeNote=-1,activeChannel=1,index=0;
    double remaining=0,offRemaining=0;float gate=.65f;uint32_t random=1763;
    Pattern(){velocity.fill(.8f);enabled.fill(true);}
    template<class Emit> void stop(Emit emit){if(activeNote>=0)emit(false,activeChannel,activeNote,0.0f);activeNote=-1;remaining=0;offRemaining=0;}
    template<class Emit> void input(bool on,int channel,int note,float vel,Emit emit){
        if(channel<1||channel>16||note<0||note>127)return;
        const bool any=std::any_of(held.begin(),held.end(),[](const Held& h){return h.down;});
        held[size_t((channel-1)*128+note)]={on,vel};
        if(on&&!any){index=0;remaining=0;}
        if(!on&&!std::any_of(held.begin(),held.end(),[](const Held& h){return h.down;}))stop(emit);
    }
    template<class Emit> void next(double interval,Emit emit){
        if(activeNote>=0&&--offRemaining<=0){emit(false,activeChannel,activeNote,0.0f);activeNote=-1;}
        if(remaining>0){remaining-=1;return;}
        std::array<int,2048> keys{};int count=0;
        for(int i=0;i<2048;++i)if(held[size_t(i)].down)keys[size_t(count++)]=i;
        if(count==0)return;
        if(activeNote>=0){emit(false,activeChannel,activeNote,0.0f);activeNote=-1;}
        const int step=index%std::clamp(length,1,16);int key=keys[0],octave=0;
        if(mode==1||mode==3){
            // Sort chord notes by pitch; preserve each note's MIDI channel.
            std::sort(keys.begin(),keys.begin()+count,[](int a,int b){return a%128==b%128?a<b:a%128<b%128;});
            const int total=count*std::clamp(octaves,1,4);int n=index%total;
            if(order==1)n=total-1-n;
            if(order==2){const int cycle=std::max(1,total*2-2);n=index%cycle;if(n>=total)n=cycle-n;}
            if(order==3){random^=random<<13;random^=random>>17;random^=random<<5;n=int(random%uint32_t(total));}
            key=keys[size_t(n%count)];octave=n/count;
        }
        const bool sequence=mode==2||mode==3;
        if(!sequence||enabled[size_t(step)]){
            activeNote=std::clamp(key%128+12*octave+(sequence?pitch[size_t(step)]:0),0,127);
            activeChannel=key/128+1;
            emit(true,activeChannel,activeNote,sequence?velocity[size_t(step)]:held[size_t(key)].velocity);
            offRemaining=std::max(1.0,interval*gate);
        }
        ++index;if(index>1000000)index=0;remaining=std::max(1.0,interval)-1;
    }
};
}
