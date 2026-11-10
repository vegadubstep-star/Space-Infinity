#include "../Source/AnalogueCircuit.h"
#include <iostream>
#include <random>
int main(){
    std::mt19937 rng(4371);
    std::uniform_real_distribution<float> noise(-1.0f,1.0f);
    float maximum=0.0f;
    for(const double rate:{44100.0,48000.0,96000.0}){
        for(const float resonance:{0.0f,0.4f,0.8f,0.95f}){
            for(const float drive:{1.0f,2.5f,4.0f}){
                AnalogueLadder ladder;
                for(int i=0;i<12000;++i){
                    const float sweep=float(i%4000)/3999.0f;
                    const double cutoff=30.0*std::pow((rate*.42)/30.0,sweep);
                    const float g=float(std::tan(3.141592653589793*cutoff/rate));
                    const float output=ladder.process(i<8000?noise(rng):0.0f,g/(1.0f+g),resonance,drive);
                    if(!std::isfinite(output)||std::abs(output)>8.0f)return 1;
                    maximum=std::max(maximum,std::abs(output));
                }
                ladder.reset();
                if(ladder.process(0.0f,.12f,resonance,drive)!=0.0f)return 2;
            }
        }
    }
    AnalogueDrift a,b;
    a.reset(72);b.reset(72);
    for(int i=0;i<20000;++i){
        const auto x=a.next(.0002f,2048),y=b.next(.0002f,2048);
        if(x!=y||std::abs(x[0])>1.0f||std::abs(x[1])>1.0f)return 3;
    }
    std::cout<<"Analogue filter sweep passed; maximum sample "<<maximum<<"\n";
}
