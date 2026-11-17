#include "HardwareTheme.h"
#include <Artwork.h>

const juce::Image& HardwareTheme::knob(bool violet) {
    static const auto silver=juce::ImageCache::getFromMemory(Artwork::KnobSilver_png,Artwork::KnobSilver_pngSize);
    static const auto purple=juce::ImageCache::getFromMemory(Artwork::KnobViolet_png,Artwork::KnobViolet_pngSize);
    return violet?purple:silver;
}
const juce::Image& HardwareTheme::grain() {
    static const auto image=juce::ImageCache::getFromMemory(Artwork::BrushedGrain_png,Artwork::BrushedGrain_pngSize);
    return image;
}
