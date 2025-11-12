/**
 * @file PluginEditor.cpp
 * @brief Implementation of plugin editor UI
 */

#include "PluginEditor.h"

namespace MAEVN
{

//==============================================================================
MAEVNAudioProcessorEditor::MAEVNAudioProcessorEditor(MAEVNAudioProcessor& p)
    : AudioProcessorEditor(&p)
    , audioProcessor(p)
{
    setSize(1200, 800);
    setupUI();
    
    Logger::log(Logger::Level::Info, "MAEVN Editor initialized");
}

MAEVNAudioProcessorEditor::~MAEVNAudioProcessorEditor()
{
}

//==============================================================================
void MAEVNAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    
    g.setColour(juce::Colours::white);
    g.setFont(24.0f);
    g.drawText("MAEVN - AI Vocal + Instrument Generator", 
               20, 20, getWidth() - 40, 40, juce::Justification::centred);
}

void MAEVNAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(80); // Header space
    
    // Stage script input
    auto scriptArea = bounds.removeFromTop(100);
    stageScriptInput.setBounds(scriptArea.reduced(10));
    
    // Parse button and BPM controls
    auto controlArea = bounds.removeFromTop(50);
    parseButton.setBounds(controlArea.removeFromLeft(150).reduced(10));
    bpmLabel.setBounds(controlArea.removeFromLeft(80).reduced(10));
    bpmSlider.setBounds(controlArea.removeFromLeft(200).reduced(10));
    
    // Timeline lanes
    int laneHeight = 60;
    auto timelineArea = bounds.removeFromTop(laneHeight * 6 + 20);
    for (int i = 0; i < (int)timelineLanes.size(); ++i)
    {
        if (timelineLanes[i])
        {
            timelineLanes[i]->setBounds(timelineArea.removeFromTop(laneHeight).reduced(5));
        }
    }
    
    // Bottom area for preset browser and undo history
    auto bottomArea = bounds;
    if (presetBrowser)
    {
        presetBrowser->setBounds(bottomArea.removeFromLeft(400).reduced(10));
    }
    if (undoHistory)
    {
        undoHistory->setBounds(bottomArea.removeFromRight(300).reduced(10));
    }
}

void MAEVNAudioProcessorEditor::setupUI()
{
    // Stage script input
    addAndMakeVisible(stageScriptInput);
    stageScriptInput.setMultiLine(true);
    stageScriptInput.setReturnKeyStartsNewLine(true);
    stageScriptInput.setTextToShowWhenEmpty(
        "Enter stage script here, e.g.:\n[HOOK] Catchy hook lyrics\n[VERSE] Verse lyrics\n[808] Bass pattern",
        juce::Colours::grey);
    
    // Parse button
    addAndMakeVisible(parseButton);
    parseButton.setButtonText("Parse Script");
    parseButton.onClick = [this] { onParseButtonClicked(); };
    
    // BPM controls
    addAndMakeVisible(bpmLabel);
    bpmLabel.setText("BPM:", juce::dontSendNotification);
    
    addAndMakeVisible(bpmSlider);
    bpmSlider.setRange(60.0, 200.0, 1.0);
    bpmSlider.setValue(audioProcessor.getPatternEngine().getBPM());
    bpmSlider.onValueChange = [this] { onBPMChanged(); };
    
    // Create timeline lanes
    const juce::StringArray trackNames = {
        "Vocals", "808 Bass", "Hi-Hats", "Snare", "Piano", "Synth"
    };
    
    for (int i = 0; i < 6; ++i)
    {
        auto lane = std::make_unique<TimelineLane>(i, &audioProcessor.getPatternEngine());
        lane->setTrackName(trackNames[i]);
        addAndMakeVisible(*lane);
        timelineLanes.push_back(std::move(lane));
    }
    
    // Preset browser
    presetBrowser = std::make_unique<PresetBrowserComponent>(&audioProcessor.getPresetManager());
    addAndMakeVisible(*presetBrowser);
    
    // Undo history
    undoHistory = std::make_unique<UndoHistoryComponent>(&audioProcessor.getUndoManager());
    addAndMakeVisible(*undoHistory);
}

void MAEVNAudioProcessorEditor::onParseButtonClicked()
{
    juce::String scriptText = stageScriptInput.getText();
    
    if (scriptText.isEmpty())
    {
        // Show alert
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            "Empty Script",
            "Please enter a stage script to parse.",
            "OK");
        return;
    }
    
    // Parse the script
    int numBlocks = audioProcessor.getPatternEngine().parseStageScript(scriptText);
    
    // Add undo action
    ActionState action(
        ActionState::Type::TimelineChange,
        "Parse stage script (" + juce::String(numBlocks) + " blocks)",
        juce::var(scriptText)
    );
    audioProcessor.getUndoManager().addAction(action);
    
    // Refresh timeline lanes
    for (auto& lane : timelineLanes)
    {
        lane->repaint();
    }
    
    // Show result
    juce::String message = "Parsed " + juce::String(numBlocks) + " blocks from stage script.";
    Logger::log(Logger::Level::Info, message);
}

void MAEVNAudioProcessorEditor::onBPMChanged()
{
    double newBPM = bpmSlider.getValue();
    audioProcessor.getPatternEngine().setBPM(newBPM);
    
    // Add undo action
    ActionState action(
        ActionState::Type::ArrangementChange,
        "Change BPM to " + juce::String(newBPM, 1),
        juce::var(newBPM)
    );
    audioProcessor.getUndoManager().addAction(action);
    
    Logger::log(Logger::Level::Info, "BPM changed to: " + juce::String(newBPM, 1));
}

} // namespace MAEVN
