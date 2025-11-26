/**
 * @file TimelineLane.h
 * @brief Timeline lane component for track visualization
 */

#pragma once

#include <JuceHeader.h>
#include "Utilities.h"
#include "PatternEngine.h"

namespace MAEVN
{

class TimelineLane : public juce::Component
{
public:
    TimelineLane(int trackIndex, PatternEngine* engine);
    ~TimelineLane() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void setTrackName(const juce::String& name);
    int getTrackIndex() const { return trackIndex; }
    
private:
    int trackIndex;
    PatternEngine* patternEngine;
    juce::String trackName;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimelineLane)
};

} // namespace MAEVN
