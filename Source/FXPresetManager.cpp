/**
 * @file FXPresetManager.cpp
 * @brief Implementation of FX Preset Manager
 */

#include "FXPresetManager.h"

namespace MAEVN
{

//==============================================================================
FXPresetManager::FXPresetManager()
{
}

FXPresetManager::~FXPresetManager()
{
}

int FXPresetManager::loadPresetsFromDirectory(const juce::File& directory)
{
    const juce::ScopedLock sl(presetLock);
    
    if (!directory.isDirectory())
    {
        Logger::log(Logger::Level::Warning, "Presets directory not found: " + directory.getFullPathName());
        return 0;
    }
    
    presetsDirectory = directory;
    presets.clear();
    
    // Find all .json files in directory
    juce::Array<juce::File> files = directory.findChildFiles(
        juce::File::findFiles, false, "*.json");
    
    int loadedCount = 0;
    for (const auto& file : files)
    {
        if (loadPresetFile(file))
        {
            loadedCount++;
        }
    }
    
    Logger::log(Logger::Level::Info, 
        "Loaded " + juce::String(loadedCount) + " presets from " + directory.getFullPathName());
    
    return loadedCount;
}

bool FXPresetManager::loadPresetFile(const juce::File& file)
{
    FXPreset preset;
    if (preset.loadFromFile(file))
    {
        presets.push_back(preset);
        return true;
    }
    return false;
}

bool FXPresetManager::savePreset(const FXPreset& preset, const juce::File& directory)
{
    if (!directory.isDirectory())
    {
        if (!directory.createDirectory())
        {
            Logger::log(Logger::Level::Error, "Failed to create presets directory");
            return false;
        }
    }
    
    // Generate filename from preset name
    juce::String filename = preset.getName().replaceCharacter(' ', '_') + ".json";
    juce::File file = directory.getChildFile(filename);
    
    return preset.saveToFile(file);
}

void FXPresetManager::addPreset(const FXPreset& preset)
{
    const juce::ScopedLock sl(presetLock);
    presets.push_back(preset);
}

void FXPresetManager::removePreset(int index)
{
    const juce::ScopedLock sl(presetLock);
    
    if (index >= 0 && index < (int)presets.size())
    {
        presets.erase(presets.begin() + index);
    }
}

const FXPreset* FXPresetManager::getPreset(int index) const
{
    const juce::ScopedLock sl(presetLock);
    
    if (index >= 0 && index < (int)presets.size())
    {
        return &presets[index];
    }
    return nullptr;
}

std::vector<int> FXPresetManager::searchPresets(const juce::String& searchTerm) const
{
    const juce::ScopedLock sl(presetLock);
    
    std::vector<int> results;
    
    if (searchTerm.isEmpty())
    {
        // Return all presets
        for (int i = 0; i < (int)presets.size(); ++i)
        {
            results.push_back(i);
        }
        return results;
    }
    
    juce::String lowerSearch = searchTerm.toLowerCase();
    
    for (int i = 0; i < (int)presets.size(); ++i)
    {
        const auto& preset = presets[i];
        
        // Search in name
        if (preset.getName().toLowerCase().contains(lowerSearch))
        {
            results.push_back(i);
            continue;
        }
        
        // Search in category
        if (preset.getCategory().toLowerCase().contains(lowerSearch))
        {
            results.push_back(i);
            continue;
        }
        
        // Search in tags
        bool foundInTags = false;
        for (const auto& tag : preset.getTags())
        {
            if (tag.toLowerCase().contains(lowerSearch))
            {
                foundInTags = true;
                break;
            }
        }
        if (foundInTags)
        {
            results.push_back(i);
            continue;
        }
        
        // Search in description
        if (preset.getDescription().toLowerCase().contains(lowerSearch))
        {
            results.push_back(i);
            continue;
        }
    }
    
    return results;
}

std::vector<int> FXPresetManager::filterByCategory(const juce::String& category) const
{
    const juce::ScopedLock sl(presetLock);
    
    std::vector<int> results;
    
    for (int i = 0; i < (int)presets.size(); ++i)
    {
        if (presets[i].getCategory().equalsIgnoreCase(category))
        {
            results.push_back(i);
        }
    }
    
    return results;
}

std::vector<int> FXPresetManager::filterByTag(const juce::String& tag) const
{
    const juce::ScopedLock sl(presetLock);
    
    std::vector<int> results;
    
    for (int i = 0; i < (int)presets.size(); ++i)
    {
        if (presets[i].hasTag(tag))
        {
            results.push_back(i);
        }
    }
    
    return results;
}

juce::StringArray FXPresetManager::getAllCategories() const
{
    const juce::ScopedLock sl(presetLock);
    
    juce::StringArray categories;
    
    for (const auto& preset : presets)
    {
        juce::String category = preset.getCategory();
        if (!categories.contains(category))
        {
            categories.add(category);
        }
    }
    
    categories.sort(false);
    return categories;
}

juce::StringArray FXPresetManager::getAllTags() const
{
    const juce::ScopedLock sl(presetLock);
    
    juce::StringArray allTags;
    
    for (const auto& preset : presets)
    {
        for (const auto& tag : preset.getTags())
        {
            if (!allTags.contains(tag))
            {
                allTags.add(tag);
            }
        }
    }
    
    allTags.sort(false);
    return allTags;
}

void FXPresetManager::clearPresets()
{
    const juce::ScopedLock sl(presetLock);
    presets.clear();
}

const FXPreset* FXPresetManager::getPresetByName(const juce::String& name) const
{
    const juce::ScopedLock sl(presetLock);
    
    for (const auto& preset : presets)
    {
        if (preset.getName().equalsIgnoreCase(name))
        {
            return &preset;
        }
    }
    
    return nullptr;
}

bool FXPresetManager::hasPreset(const juce::String& name) const
{
    return getPresetByName(name) != nullptr;
}

void FXPresetManager::setPresetsDirectory(const juce::File& directory)
{
    presetsDirectory = directory;
}

void FXPresetManager::reloadPresets()
{
    if (presetsDirectory.isDirectory())
    {
        loadPresetsFromDirectory(presetsDirectory);
    }
}

} // namespace MAEVN
