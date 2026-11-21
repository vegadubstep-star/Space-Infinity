#pragma once
#include <JuceHeader.h>
struct PopupHost {
    virtual ~PopupHost()=default;
    virtual void dismissPopupsFor(juce::Component*)=0;
};
inline PopupHost* popupHostFor(juce::Component* c){for(;c;c=c->getParentComponent())if(auto* host=dynamic_cast<PopupHost*>(c))return host;return nullptr;}
