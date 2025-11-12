/**
 * @file FXPresetManager.h
 * @brief Manager for FX presets with search and filtering
 * 
 * This module manages a collection of FX presets, providing search,
 * filtering, and organization capabilities.
 */

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <memory>
#include "FXPreset.h"
#include "Utilities.h"

namespace MAEVN
{

//==============================================================================
/**
 * @brief FX Preset Manager - handles preset collection and search
 */
class FXPresetManager
{
public:
    FXPresetManager();
    ~FXPresetManager();
    
    /**
     * @brief Load all presets from directory
     * @param directory Directory containing .json preset files
     * @return Number of presets loaded
     */
    int loadPresetsFromDirectory(const juce::File& directory);
    
    /**
     * @brief Save preset to directory
     * @param preset Preset to save
     * @param directory Directory to save to
     * @return true if saved successfully
     */
    bool savePreset(const FXPreset& preset, const juce::File& directory);
    
    /**
     * @brief Add preset to collection
     */
    void addPreset(const FXPreset& preset);
    
    /**
     * @brief Remove preset by index
     */
    void removePreset(int index);
    
    /**
     * @brief Get preset by index
     */
    const FXPreset* getPreset(int index) const;
    
    /**
     * @brief Get number of presets
     */
    int getNumPresets() const { return static_cast<int>(presets.size()); }
    
    /**
     * @brief Search presets by name or tag
     * @param searchTerm Search string
     * @return Indices of matching presets
     */
    std::vector<int> searchPresets(const juce::String& searchTerm) const;
    
    /**
     * @brief Filter presets by category
     * @param category Category to filter by
     * @return Indices of matching presets
     */
    std::vector<int> filterByCategory(const juce::String& category) const;
    
    /**
     * @brief Filter presets by tag
     * @param tag Tag to filter by
     * @return Indices of matching presets
     */
    std::vector<int> filterByTag(const juce::String& tag) const;
    
    /**
     * @brief Get all unique categories from loaded presets
     */
    juce::StringArray getAllCategories() const;
    
    /**
     * @brief Get all unique tags from loaded presets
     */
    juce::StringArray getAllTags() const;
    
    /**
     * @brief Clear all presets
     */
    void clearPresets();
    
    /**
     * @brief Get preset by name
     */
    const FXPreset* getPresetByName(const juce::String& name) const;
    
    /**
     * @brief Check if preset with name exists
     */
    bool hasPreset(const juce::String& name) const;
    
    /**
     * @brief Get presets directory path
     */
    juce::File getPresetsDirectory() const { return presetsDirectory; }
    
    /**
     * @brief Set presets directory
     */
    void setPresetsDirectory(const juce::File& directory);
    
    /**
     * @brief Reload all presets from current directory
     */
    void reloadPresets();
    
private:
    std::vector<FXPreset> presets;
    juce::File presetsDirectory;
    
    mutable juce::CriticalSection presetLock;
    
    /**
     * @brief Load single preset file
     */
    bool loadPresetFile(const juce::File& file);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FXPresetManager)
};

} // namespace MAEVN
