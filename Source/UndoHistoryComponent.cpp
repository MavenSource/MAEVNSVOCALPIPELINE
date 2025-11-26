/**
 * @file UndoHistoryComponent.cpp
 * @brief Implementation of Undo History component
 */

#include "UndoHistoryComponent.h"

namespace MAEVN
{

UndoHistoryComponent::UndoHistoryComponent(GlobalUndoManager* manager)
    : undoManager(manager)
{
    setSize(300, 400);
}

UndoHistoryComponent::~UndoHistoryComponent()
{
}

void UndoHistoryComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
    g.setColour(juce::Colours::white);
    g.drawText("Undo History", 10, 10, getWidth() - 20, 30, juce::Justification::centred);
    
    if (undoManager)
    {
        int y = 50;
        const auto& history = undoManager->getHistory();
        int currentIndex = undoManager->getCurrentHistoryIndex();
        
        for (int i = 0; i < (int)history.size() && y < getHeight(); ++i)
        {
            if (i == currentIndex)
            {
                g.setColour(juce::Colours::lightblue);
            }
            else
            {
                g.setColour(juce::Colours::white);
            }
            g.drawText(history[i].description, 10, y, getWidth() - 20, 20, juce::Justification::centredLeft);
            y += 25;
        }
    }
}

void UndoHistoryComponent::resized()
{
}

void UndoHistoryComponent::refreshHistory()
{
    repaint();
}

} // namespace MAEVN
