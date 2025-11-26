/**
 * @file PresetBrowserComponent.h
 * @brief Preset browser UI with search and filtering
 */

#pragma once

#include <JuceHeader.h>
#include "FXPresetManager.h"

namespace MAEVN
{

class PresetBrowserComponent : public juce::Component
{
public:
    PresetBrowserComponent(FXPresetManager* manager);
    ~PresetBrowserComponent() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void refreshPresetList();
    
private:
    FXPresetManager* presetManager;
    juce::TextEditor searchBox;
    juce::ListBox presetList;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBrowserComponent)
};

} // namespace MAEVN
