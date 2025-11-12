/**
 * @file FXPreset.cpp
 * @brief Implementation of FX Preset
 */

#include "FXPreset.h"

namespace MAEVN
{

//==============================================================================
FXPreset::FXPreset()
    : name("Untitled")
    , category(PresetCategories::Vocal)
    , mode(FXMode::DSP)
{
}

void FXPreset::addTag(const juce::String& tag)
{
    if (!hasTag(tag))
    {
        tags.add(tag);
    }
}

void FXPreset::removeTag(const juce::String& tag)
{
    tags.removeString(tag);
}

bool FXPreset::hasTag(const juce::String& tag) const
{
    return tags.contains(tag);
}

void FXPreset::setParameter(const juce::String& paramName, float value)
{
    parameters[paramName] = value;
}

float FXPreset::getParameter(const juce::String& paramName, float defaultValue) const
{
    auto it = parameters.find(paramName);
    if (it != parameters.end())
    {
        return it->second;
    }
    return defaultValue;
}

juce::var FXPreset::toJSON() const
{
    auto* obj = new juce::DynamicObject();
    
    obj->setProperty("name", name);
    obj->setProperty("category", category);
    obj->setProperty("description", description);
    obj->setProperty("author", author);
    obj->setProperty("mode", static_cast<int>(mode));
    
    // Tags array
    juce::var tagsArray;
    juce::Array<juce::var> tagsVector;
    for (const auto& tag : tags)
    {
        tagsVector.add(tag);
    }
    tagsArray = tagsVector;
    obj->setProperty("tags", tagsArray);
    
    // Parameters object
    auto* paramsObj = new juce::DynamicObject();
    for (const auto& param : parameters)
    {
        paramsObj->setProperty(param.first, param.second);
    }
    obj->setProperty("params", juce::var(paramsObj));
    
    return juce::var(obj);
}

bool FXPreset::fromJSON(const juce::var& json)
{
    if (!json.isObject())
        return false;
    
    auto* obj = json.getDynamicObject();
    if (!obj)
        return false;
    
    try
    {
        name = obj->getProperty("name").toString();
        category = obj->getProperty("category").toString();
        description = obj->getProperty("description").toString();
        author = obj->getProperty("author").toString();
        
        int modeInt = obj->getProperty("mode");
        mode = static_cast<FXMode>(modeInt);
        
        // Load tags
        tags.clear();
        if (obj->hasProperty("tags"))
        {
            auto tagsVar = obj->getProperty("tags");
            if (tagsVar.isArray())
            {
                auto* tagsArray = tagsVar.getArray();
                for (int i = 0; i < tagsArray->size(); ++i)
                {
                    tags.add(tagsArray->getUnchecked(i).toString());
                }
            }
        }
        
        // Load parameters
        parameters.clear();
        if (obj->hasProperty("params"))
        {
            auto paramsVar = obj->getProperty("params");
            if (paramsVar.isObject())
            {
                auto* paramsObj = paramsVar.getDynamicObject();
                for (auto& prop : paramsObj->getProperties())
                {
                    juce::String paramName = prop.name.toString();
                    float paramValue = static_cast<float>(prop.value);
                    parameters[paramName] = paramValue;
                }
            }
        }
        
        return true;
    }
    catch (...)
    {
        Logger::log(Logger::Level::Error, "Failed to parse preset JSON");
        return false;
    }
}

bool FXPreset::saveToFile(const juce::File& file) const
{
    auto json = toJSON();
    juce::String jsonString = juce::JSON::toString(json, true);
    
    if (file.replaceWithText(jsonString))
    {
        Logger::log(Logger::Level::Info, "Preset saved: " + file.getFullPathName());
        return true;
    }
    
    Logger::log(Logger::Level::Error, "Failed to save preset: " + file.getFullPathName());
    return false;
}

bool FXPreset::loadFromFile(const juce::File& file)
{
    if (!file.existsAsFile())
    {
        Logger::log(Logger::Level::Error, "Preset file not found: " + file.getFullPathName());
        return false;
    }
    
    juce::String content = file.loadFileAsString();
    juce::var json = juce::JSON::parse(content);
    
    if (fromJSON(json))
    {
        Logger::log(Logger::Level::Info, "Preset loaded: " + file.getFullPathName());
        return true;
    }
    
    return false;
}

void FXPreset::clear()
{
    name = "Untitled";
    category = PresetCategories::Vocal;
    description = "";
    author = "";
    mode = FXMode::DSP;
    tags.clear();
    parameters.clear();
}

} // namespace MAEVN
