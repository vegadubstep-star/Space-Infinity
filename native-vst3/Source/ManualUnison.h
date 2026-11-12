#pragma once
#include <cmath>
#include <array>
namespace infinity {
// Independent distributions implementing the qualitative behavior documented by
// Xfer. The manual does not disclose Serum's numerical transfer curves.
struct UnisonSettings {
    int mode=0,stack=0;float amount=0,range=36,blend=.75f;
    int groups()const{return stack==0?1:stack<=3?stack+1:stack<=6?1+2*(stack-3):1;}
    float semitones(int group,bool centre)const {if(stack>=7)return centre?(stack==7?-12.0f:-24.0f):0.0f;if(stack<=3)return float(group*12);return group==0?0:float((group+1)/2*12-(group%2?5:0));}
    double ratio(int voice,int count,int group)const {
        const bool centre=std::abs(2*voice-(count-1))<=1;
        double x=count==1?0:2.0*voice/(count-1)-1;
        if(mode==1)x=.35*x+.65*x*x*x;
        else if(mode==2)x=std::copysign(x*x,x);
        else if(mode==3)x*=x<0?1.2:.8;
        else if(mode==4){const unsigned hash=(unsigned(voice+1)*747796405u+unsigned(count)*2891336453u);x=count==1?0:double((hash^(hash>>16))&65535)/32767.5-1;}
        const double cents=amount*range;
        const double spread=mode==0?1+x*(std::exp2(cents/1200)-1):std::exp2(x*cents/1200);
        return spread*std::exp2(semitones(group,centre)/12.0);
    }
    float weight(int voice,int count)const {if(count<=2)return 1;const bool centre=std::abs(2*voice-(count-1))<=1;return centre?(blend<=.75f?1:(1-blend)/.25f):(blend<.75f?blend/.75f:1);}
};
}
