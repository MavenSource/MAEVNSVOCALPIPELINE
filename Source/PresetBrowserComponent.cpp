/**
 * @file PresetBrowserComponent.cpp
 * @brief Implementation of Preset Browser component
 */

#include "PresetBrowserComponent.h"

namespace MAEVN
{

PresetBrowserComponent::PresetBrowserComponent(FXPresetManager* manager)
    : presetManager(manager)
{
    addAndMakeVisible(searchBox);
    searchBox.setTextToShowWhenEmpty("Search presets...", juce::Colours::grey);
    
    setSize(400, 600);
}

PresetBrowserComponent::~PresetBrowserComponent()
{
}

void PresetBrowserComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
    g.setColour(juce::Colours::white);
    g.drawText("Preset Browser", 10, 10, getWidth() - 20, 30, juce::Justification::centred);
}

void PresetBrowserComponent::resized()
{
    searchBox.setBounds(10, 50, getWidth() - 20, 30);
}

void PresetBrowserComponent::refreshPresetList()
{
    repaint();
}

} // namespace MAEVN
