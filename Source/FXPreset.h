/**
 * @file FXPreset.h
 * @brief FX Preset data structures and serialization
 * 
 * This module defines the structure for FX presets with support for
 * categorization, tagging, and JSON serialization.
 */

#pragma once

#include <JuceHeader.h>
#include <map>
#include <vector>
#include "Utilities.h"

namespace MAEVN
{

//==============================================================================
/**
 * @brief FX Preset structure
 */
class FXPreset
{
public:
    FXPreset();
    ~FXPreset() = default;
    
    /**
     * @brief Get preset name
     */
    juce::String getName() const { return name; }
    
    /**
     * @brief Set preset name
     */
    void setName(const juce::String& newName) { name = newName; }
    
    /**
     * @brief Get FX mode
     */
    FXMode getMode() const { return mode; }
    
    /**
     * @brief Set FX mode
     */
    void setMode(FXMode newMode) { mode = newMode; }
    
    /**
     * @brief Get category
     */
    juce::String getCategory() const { return category; }
    
    /**
     * @brief Set category
     */
    void setCategory(const juce::String& newCategory) { category = newCategory; }
    
    /**
     * @brief Get all tags
     */
    juce::StringArray getTags() const { return tags; }
    
    /**
     * @brief Add a tag
     */
    void addTag(const juce::String& tag);
    
    /**
     * @brief Remove a tag
     */
    void removeTag(const juce::String& tag);
    
    /**
     * @brief Check if preset has a specific tag
     */
    bool hasTag(const juce::String& tag) const;
    
    /**
     * @brief Set parameter value
     */
    void setParameter(const juce::String& paramName, float value);
    
    /**
     * @brief Get parameter value
     */
    float getParameter(const juce::String& paramName, float defaultValue = 0.0f) const;
    
    /**
     * @brief Get all parameters
     */
    const std::map<juce::String, float>& getParameters() const { return parameters; }
    
    /**
     * @brief Serialize to JSON
     */
    juce::var toJSON() const;
    
    /**
     * @brief Deserialize from JSON
     */
    bool fromJSON(const juce::var& json);
    
    /**
     * @brief Save to file
     */
    bool saveToFile(const juce::File& file) const;
    
    /**
     * @brief Load from file
     */
    bool loadFromFile(const juce::File& file);
    
    /**
     * @brief Clear all data
     */
    void clear();
    
    /**
     * @brief Get description
     */
    juce::String getDescription() const { return description; }
    
    /**
     * @brief Set description
     */
    void setDescription(const juce::String& desc) { description = desc; }
    
    /**
     * @brief Get author
     */
    juce::String getAuthor() const { return author; }
    
    /**
     * @brief Set author
     */
    void setAuthor(const juce::String& newAuthor) { author = newAuthor; }
    
private:
    juce::String name;
    juce::String category;
    juce::String description;
    juce::String author;
    FXMode mode;
    juce::StringArray tags;
    std::map<juce::String, float> parameters;
    
    JUCE_LEAK_DETECTOR(FXPreset)
};

//==============================================================================
/**
 * @brief Preset categories
 */
namespace PresetCategories
{
    static const juce::String Vocal = "Vocal";
    static const juce::String Bass808 = "808";
    static const juce::String Drums = "Drums";
    static const juce::String HiHat = "HiHat";
    static const juce::String Snare = "Snare";
    static const juce::String Piano = "Piano";
    static const juce::String Synth = "Synth";
    static const juce::String Master = "Master";
    static const juce::String Experimental = "Experimental";
    
    /**
     * @brief Get all available categories
     */
    inline juce::StringArray getAllCategories()
    {
        return juce::StringArray {
            Vocal, Bass808, Drums, HiHat, Snare, Piano, Synth, Master, Experimental
        };
    }
}

//==============================================================================
/**
 * @brief Common preset tags
 */
namespace PresetTags
{
    static const juce::String Trap = "Trap";
    static const juce::String Clean = "Clean";
    static const juce::String Dirty = "Dirty";
    static const juce::String Radio = "Radio";
    static const juce::String Wide = "Wide";
    static const juce::String Mono = "Mono";
    static const juce::String Warm = "Warm";
    static const juce::String Bright = "Bright";
    static const juce::String Dark = "Dark";
    static const juce::String Compressed = "Compressed";
    static const juce::String Dynamic = "Dynamic";
    static const juce::String Reverb = "Reverb";
    static const juce::String Delay = "Delay";
    static const juce::String Distortion = "Distortion";
    static const juce::String Vintage = "Vintage";
    static const juce::String Modern = "Modern";
    static const juce::String AI = "AI";
    static const juce::String Hybrid = "Hybrid";
    
    // Cinematic Audio Enhancement Tags
    static const juce::String Cinematic = "Cinematic";
    static const juce::String Grammy = "Grammy";
    static const juce::String Emotional = "Emotional";
    static const juce::String Viral = "Viral";
    static const juce::String Mastering = "Mastering";
    static const juce::String Presence = "Presence";
    static const juce::String LargeHall = "LargeHall";
    static const juce::String Saturation = "Saturation";
    static const juce::String Multiband = "Multiband";
    static const juce::String StereoWide = "StereoWide";
    static const juce::String LUFS14 = "LUFS14";
    
    /**
     * @brief Get all common tags
     */
    inline juce::StringArray getAllTags()
    {
        return juce::StringArray {
            Trap, Clean, Dirty, Radio, Wide, Mono, Warm, Bright, Dark,
            Compressed, Dynamic, Reverb, Delay, Distortion, Vintage, Modern, AI, Hybrid,
            Cinematic, Grammy, Emotional, Viral, Mastering, Presence, LargeHall,
            Saturation, Multiband, StereoWide, LUFS14
        };
    }
}

} // namespace MAEVN
