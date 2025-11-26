/**
 * @file PluginEditor.h
 * @brief Main VST3 plugin editor UI
 */

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "TimelineLane.h"
#include "PresetBrowserComponent.h"
#include "UndoHistoryComponent.h"

namespace MAEVN
{

class MAEVNAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    MAEVNAudioProcessorEditor(MAEVNAudioProcessor&);
    ~MAEVNAudioProcessorEditor() override;
    
    void paint(juce::Graphics&) override;
    void resized() override;
    
private:
    MAEVNAudioProcessor& audioProcessor;
    
    // UI Components
    juce::TextEditor stageScriptInput;
    juce::TextButton parseButton;
    juce::Label bpmLabel;
    juce::Slider bpmSlider;
    
    // Timeline lanes
    std::vector<std::unique_ptr<TimelineLane>> timelineLanes;
    
    // Preset browser
    std::unique_ptr<PresetBrowserComponent> presetBrowser;
    
    // Undo history
    std::unique_ptr<UndoHistoryComponent> undoHistory;
    
    void setupUI();
    void onParseButtonClicked();
    void onBPMChanged();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MAEVNAudioProcessorEditor)
};

} // namespace MAEVN
