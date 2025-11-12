/**
 * @file GlobalUndoManager.cpp
 * @brief Implementation of Global Undo Manager
 */

#include "GlobalUndoManager.h"

namespace MAEVN
{

//==============================================================================
GlobalUndoManager::GlobalUndoManager()
    : currentIndex(-1)
    , maxHistorySize(100)
    , inTransaction(false)
{
}

GlobalUndoManager::~GlobalUndoManager()
{
}

void GlobalUndoManager::addAction(const ActionState& action)
{
    const juce::ScopedLock sl(historyLock);
    
    if (inTransaction)
    {
        // Add to transaction buffer
        transactionActions.push_back(action);
        return;
    }
    
    // Remove any actions after current index (they become invalidated)
    if (currentIndex < (int)history.size() - 1)
    {
        history.erase(history.begin() + currentIndex + 1, history.end());
    }
    
    // Add new action
    history.push_back(action);
    currentIndex++;
    
    // Trim if necessary
    trimHistory();
    
    Logger::log(Logger::Level::Info, "Action added: " + action.description);
}

bool GlobalUndoManager::undo()
{
    const juce::ScopedLock sl(historyLock);
    
    if (!canUndo())
        return false;
    
    const auto& action = history[currentIndex];
    
    // Call undo callback if set
    if (undoCallback)
    {
        undoCallback(action);
    }
    
    currentIndex--;
    
    Logger::log(Logger::Level::Info, "Undo: " + action.description);
    return true;
}

bool GlobalUndoManager::redo()
{
    const juce::ScopedLock sl(historyLock);
    
    if (!canRedo())
        return false;
    
    currentIndex++;
    const auto& action = history[currentIndex];
    
    // Call redo callback if set
    if (redoCallback)
    {
        redoCallback(action);
    }
    
    Logger::log(Logger::Level::Info, "Redo: " + action.description);
    return true;
}

bool GlobalUndoManager::canUndo() const
{
    const juce::ScopedLock sl(historyLock);
    return currentIndex >= 0 && !history.empty();
}

bool GlobalUndoManager::canRedo() const
{
    const juce::ScopedLock sl(historyLock);
    return currentIndex < (int)history.size() - 1;
}

juce::String GlobalUndoManager::getUndoDescription() const
{
    const juce::ScopedLock sl(historyLock);
    
    if (canUndo())
    {
        return history[currentIndex].description;
    }
    return "";
}

juce::String GlobalUndoManager::getRedoDescription() const
{
    const juce::ScopedLock sl(historyLock);
    
    if (canRedo())
    {
        return history[currentIndex + 1].description;
    }
    return "";
}

bool GlobalUndoManager::jumpToHistoryIndex(int index)
{
    const juce::ScopedLock sl(historyLock);
    
    if (index < -1 || index >= (int)history.size())
        return false;
    
    int previousIndex = currentIndex;
    currentIndex = index;
    
    // Apply or undo actions as needed
    if (index > previousIndex)
    {
        // Redo actions
        for (int i = previousIndex + 1; i <= index; ++i)
        {
            if (redoCallback)
            {
                redoCallback(history[i]);
            }
        }
    }
    else if (index < previousIndex)
    {
        // Undo actions
        for (int i = previousIndex; i > index; --i)
        {
            if (undoCallback)
            {
                undoCallback(history[i]);
            }
        }
    }
    
    Logger::log(Logger::Level::Info, "Jumped to history index: " + juce::String(index));
    return true;
}

void GlobalUndoManager::clearHistory()
{
    const juce::ScopedLock sl(historyLock);
    
    history.clear();
    currentIndex = -1;
    
    Logger::log(Logger::Level::Info, "History cleared");
}

void GlobalUndoManager::setMaxHistorySize(int maxSize)
{
    const juce::ScopedLock sl(historyLock);
    
    if (maxSize > 0)
    {
        maxHistorySize = maxSize;
        trimHistory();
    }
}

void GlobalUndoManager::beginTransaction(const juce::String& description)
{
    const juce::ScopedLock sl(historyLock);
    
    if (!inTransaction)
    {
        inTransaction = true;
        transactionDescription = description;
        transactionActions.clear();
        Logger::log(Logger::Level::Info, "Transaction began: " + description);
    }
}

void GlobalUndoManager::endTransaction()
{
    const juce::ScopedLock sl(historyLock);
    
    if (inTransaction)
    {
        if (!transactionActions.empty())
        {
            // Create a single compound action from all transaction actions
            auto* obj = new juce::DynamicObject();
            juce::Array<juce::var> actionsArray;
            
            for (const auto& action : transactionActions)
            {
                auto* actionObj = new juce::DynamicObject();
                actionObj->setProperty("type", static_cast<int>(action.type));
                actionObj->setProperty("description", action.description);
                actionObj->setProperty("data", action.stateData);
                actionsArray.add(juce::var(actionObj));
            }
            
            obj->setProperty("actions", actionsArray);
            
            ActionState compoundAction(
                ActionState::Type::FXChange,  // Type doesn't matter for compound
                transactionDescription,
                juce::var(obj)
            );
            
            // Add compound action to history
            if (currentIndex < (int)history.size() - 1)
            {
                history.erase(history.begin() + currentIndex + 1, history.end());
            }
            
            history.push_back(compoundAction);
            currentIndex++;
            trimHistory();
        }
        
        inTransaction = false;
        transactionActions.clear();
        Logger::log(Logger::Level::Info, "Transaction ended: " + transactionDescription);
    }
}

void GlobalUndoManager::setUndoCallback(std::function<void(const ActionState&)> callback)
{
    undoCallback = callback;
}

void GlobalUndoManager::setRedoCallback(std::function<void(const ActionState&)> callback)
{
    redoCallback = callback;
}

void GlobalUndoManager::trimHistory()
{
    // Remove oldest actions if history exceeds max size
    while ((int)history.size() > maxHistorySize)
    {
        history.erase(history.begin());
        if (currentIndex >= 0)
        {
            currentIndex--;
        }
    }
}

} // namespace MAEVN
