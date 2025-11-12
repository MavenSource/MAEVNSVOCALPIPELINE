/**
 * @file TimelineLane.cpp
 * @brief Implementation of Timeline Lane component
 */

#include "TimelineLane.h"

namespace MAEVN
{

TimelineLane::TimelineLane(int track, PatternEngine* engine)
    : trackIndex(track)
    , patternEngine(engine)
    , trackName("Track " + juce::String(track))
{
    setSize(800, 60);
}

TimelineLane::~TimelineLane()
{
}

void TimelineLane::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
    g.setColour(juce::Colours::white);
    g.drawText(trackName, getLocalBounds(), juce::Justification::centredLeft);
    
    // Draw blocks if pattern engine is available
    if (patternEngine)
    {
        auto blocks = patternEngine->getBlocksForTrack(trackIndex);
        for (const auto& block : blocks)
        {
            int x = (int)(block.startTime * 50); // 50 pixels per second
            int width = (int)(block.duration * 50);
            juce::Rectangle<int> blockRect(x, 20, width, 30);
            
            g.setColour(juce::Colours::lightblue);
            g.fillRect(blockRect);
            g.setColour(juce::Colours::black);
            g.drawRect(blockRect, 1);
        }
    }
}

void TimelineLane::resized()
{
}

void TimelineLane::setTrackName(const juce::String& name)
{
    trackName = name;
    repaint();
}

} // namespace MAEVN
