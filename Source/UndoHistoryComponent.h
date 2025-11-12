/**
 * @file UndoHistoryComponent.h
 * @brief UI component for undo history visualization
 */

#pragma once

#include <JuceHeader.h>
#include "GlobalUndoManager.h"

namespace MAEVN
{

class UndoHistoryComponent : public juce::Component
{
public:
    UndoHistoryComponent(GlobalUndoManager* manager);
    ~UndoHistoryComponent() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void refreshHistory();
    
private:
    GlobalUndoManager* undoManager;
    juce::ListBox historyList;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UndoHistoryComponent)
};

} // namespace MAEVN
