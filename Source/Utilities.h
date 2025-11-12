/**
 * @file Utilities.h
 * @brief Shared utilities, constants, and helper functions for MAEVN plugin
 * 
 * This file contains common utilities used across all modules including
 * constants, type definitions, and helper functions.
 */

#pragma once

#include <JuceHeader.h>
#include <string>
#include <vector>
#include <memory>

namespace MAEVN
{

//==============================================================================
// Version Information
//==============================================================================
constexpr const char* VERSION = "1.0.0";
constexpr const char* PLUGIN_NAME = "MAEVN";

//==============================================================================
// Audio Processing Constants
//==============================================================================
constexpr int MAX_BUFFER_SIZE = 4096;
constexpr int DEFAULT_SAMPLE_RATE = 44100;
constexpr int MAX_CHANNELS = 2;
constexpr double PI = 3.14159265358979323846;
constexpr double TWO_PI = 2.0 * PI;

//==============================================================================
// Model Configuration
//==============================================================================
constexpr const char* MODELS_DIR = "Models/";
constexpr const char* PRESETS_DIR = "Presets/";
constexpr const char* CONFIG_FILE = "config.json";

//==============================================================================
// Block Types for Timeline
//==============================================================================
enum class BlockType
{
    Unknown = 0,
    Intro,
    Hook,
    Verse,
    Bridge,
    Outro,
    Drum_808,
    Drum_HiHat,
    Drum_Snare,
    Instrument_Piano,
    Instrument_Synth,
    Vocal
};

//==============================================================================
// FX Mode Types
//==============================================================================
enum class FXMode
{
    Off = 0,
    DSP = 1,
    AI = 2,
    Hybrid = 3
};

//==============================================================================
// Instrument Types
//==============================================================================
enum class InstrumentType
{
    Bass808,
    HiHat,
    Snare,
    Piano,
    Synth,
    Vocal,
    Unknown
};

//==============================================================================
// Helper Functions
//==============================================================================

/**
 * @brief Convert string to BlockType enum
 */
inline BlockType stringToBlockType(const juce::String& str)
{
    if (str.equalsIgnoreCase("INTRO")) return BlockType::Intro;
    if (str.equalsIgnoreCase("HOOK")) return BlockType::Hook;
    if (str.equalsIgnoreCase("VERSE")) return BlockType::Verse;
    if (str.equalsIgnoreCase("BRIDGE")) return BlockType::Bridge;
    if (str.equalsIgnoreCase("OUTRO")) return BlockType::Outro;
    if (str.equalsIgnoreCase("808")) return BlockType::Drum_808;
    if (str.equalsIgnoreCase("HIHAT")) return BlockType::Drum_HiHat;
    if (str.equalsIgnoreCase("SNARE")) return BlockType::Drum_Snare;
    if (str.equalsIgnoreCase("PIANO")) return BlockType::Instrument_Piano;
    if (str.equalsIgnoreCase("SYNTH")) return BlockType::Instrument_Synth;
    if (str.equalsIgnoreCase("VOCAL")) return BlockType::Vocal;
    return BlockType::Unknown;
}

/**
 * @brief Convert BlockType enum to string
 */
inline juce::String blockTypeToString(BlockType type)
{
    switch (type)
    {
        case BlockType::Intro: return "INTRO";
        case BlockType::Hook: return "HOOK";
        case BlockType::Verse: return "VERSE";
        case BlockType::Bridge: return "BRIDGE";
        case BlockType::Outro: return "OUTRO";
        case BlockType::Drum_808: return "808";
        case BlockType::Drum_HiHat: return "HIHAT";
        case BlockType::Drum_Snare: return "SNARE";
        case BlockType::Instrument_Piano: return "PIANO";
        case BlockType::Instrument_Synth: return "SYNTH";
        case BlockType::Vocal: return "VOCAL";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Clamp value between min and max
 */
template<typename T>
inline T clamp(T value, T minValue, T maxValue)
{
    return juce::jmax(minValue, juce::jmin(value, maxValue));
}

/**
 * @brief Convert dB to linear gain
 */
inline float dBToGain(float dB)
{
    return std::pow(10.0f, dB / 20.0f);
}

/**
 * @brief Convert linear gain to dB
 */
inline float gainTodB(float gain)
{
    return 20.0f * std::log10(gain);
}

/**
 * @brief Simple envelope ADSR structure
 */
struct ADSREnvelope
{
    float attack = 0.01f;   // seconds
    float decay = 0.1f;     // seconds
    float sustain = 0.7f;   // level (0-1)
    float release = 0.2f;   // seconds
};

/**
 * @brief Timeline block structure
 */
struct TimelineBlock
{
    BlockType type;
    double startTime;      // in seconds
    double duration;       // in seconds
    juce::String content;  // text content for vocals, parameters for instruments
    int trackIndex;        // which lane/track this belongs to
    
    TimelineBlock() 
        : type(BlockType::Unknown)
        , startTime(0.0)
        , duration(1.0)
        , trackIndex(0)
    {}
};

/**
 * @brief Logging utility
 */
class Logger
{
public:
    enum class Level
    {
        Debug,
        Info,
        Warning,
        Error
    };
    
    static void log(Level level, const juce::String& message)
    {
        juce::String prefix;
        switch (level)
        {
            case Level::Debug:   prefix = "[DEBUG] "; break;
            case Level::Info:    prefix = "[INFO] "; break;
            case Level::Warning: prefix = "[WARNING] "; break;
            case Level::Error:   prefix = "[ERROR] "; break;
        }
        
        DBG(prefix + message);
    }
};

//==============================================================================
// Thread Safety Helpers
//==============================================================================

/**
 * @brief Thread-safe queue for real-time audio processing
 */
template<typename T>
class LockFreeQueue
{
public:
    LockFreeQueue(size_t capacity) : buffer(capacity) {}
    
    bool push(const T& item)
    {
        if (buffer.push(item))
            return true;
        return false;
    }
    
    bool pop(T& item)
    {
        if (buffer.pop(item))
            return true;
        return false;
    }
    
private:
    juce::AbstractFifo buffer;
};

} // namespace MAEVN
