/**
 * @file GlobalUndoManager.h
 * @brief Global undo/redo system with state snapshots
 * 
 * This module provides a comprehensive undo/redo system that tracks
 * changes to FX, arrangements, models, and timeline.
 */

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <memory>
#include "Utilities.h"

namespace MAEVN
{

//==============================================================================
/**
 * @brief Action state snapshot
 */
struct ActionState
{
    enum class Type
    {
        FXChange,
        TimelineChange,
        ModelChange,
        ArrangementChange,
        PresetChange
    };
    
    Type type;
    juce::String description;
    juce::var stateData;  // JSON representation of the state
    juce::Time timestamp;
    
    ActionState(Type t, const juce::String& desc, const juce::var& data)
        : type(t)
        , description(desc)
        , stateData(data)
        , timestamp(juce::Time::getCurrentTime())
    {}
};

//==============================================================================
/**
 * @brief Global Undo Manager
 */
class GlobalUndoManager
{
public:
    GlobalUndoManager();
    ~GlobalUndoManager();
    
    /**
     * @brief Add a new action to the history
     * @param action Action to add
     */
    void addAction(const ActionState& action);
    
    /**
     * @brief Undo the last action
     * @return true if undo was successful
     */
    bool undo();
    
    /**
     * @brief Redo the next action
     * @return true if redo was successful
     */
    bool redo();
    
    /**
     * @brief Check if undo is available
     */
    bool canUndo() const;
    
    /**
     * @brief Check if redo is available
     */
    bool canRedo() const;
    
    /**
     * @brief Get the description of the next undo action
     */
    juce::String getUndoDescription() const;
    
    /**
     * @brief Get the description of the next redo action
     */
    juce::String getRedoDescription() const;
    
    /**
     * @brief Jump to a specific position in history
     * @param index Index in history to jump to
     * @return true if successful
     */
    bool jumpToHistoryIndex(int index);
    
    /**
     * @brief Get all history actions
     */
    const std::vector<ActionState>& getHistory() const { return history; }
    
    /**
     * @brief Get current position in history
     */
    int getCurrentHistoryIndex() const { return currentIndex; }
    
    /**
     * @brief Clear all history
     */
    void clearHistory();
    
    /**
     * @brief Set maximum history size
     */
    void setMaxHistorySize(int maxSize);
    
    /**
     * @brief Get maximum history size
     */
    int getMaxHistorySize() const { return maxHistorySize; }
    
    /**
     * @brief Begin a transaction (group multiple actions)
     */
    void beginTransaction(const juce::String& description);
    
    /**
     * @brief End current transaction
     */
    void endTransaction();
    
    /**
     * @brief Check if in transaction
     */
    bool isInTransaction() const { return inTransaction; }
    
    /**
     * @brief Set undo callback
     * @param callback Function to call when undo is performed
     */
    void setUndoCallback(std::function<void(const ActionState&)> callback);
    
    /**
     * @brief Set redo callback
     * @param callback Function to call when redo is performed
     */
    void setRedoCallback(std::function<void(const ActionState&)> callback);
    
private:
    std::vector<ActionState> history;
    int currentIndex;
    int maxHistorySize;
    
    bool inTransaction;
    juce::String transactionDescription;
    std::vector<ActionState> transactionActions;
    
    std::function<void(const ActionState&)> undoCallback;
    std::function<void(const ActionState&)> redoCallback;
    
    mutable juce::CriticalSection historyLock;
    
    /**
     * @brief Trim history if it exceeds max size
     */
    void trimHistory();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GlobalUndoManager)
};

} // namespace MAEVN
