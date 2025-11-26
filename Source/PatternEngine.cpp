/**
 * @file PatternEngine.cpp
 * @brief Implementation of timeline and pattern engine
 */

#include "PatternEngine.h"
#include <regex>

namespace MAEVN
{

//==============================================================================
PatternEngine::PatternEngine()
    : currentBPM(120.0)
    , playheadPosition(0.0)
    , playing(false)
    , defaultBlockDuration(4.0)
    , quantizationEnabled(true)
{
}

PatternEngine::~PatternEngine()
{
}

int PatternEngine::parseStageScript(const juce::String& scriptInput)
{
    const juce::ScopedLock sl(blockLock);
    
    clearBlocks();
    
    if (scriptInput.isEmpty())
        return 0;
    
    // Split input into lines
    juce::StringArray lines;
    lines.addLines(scriptInput);
    
    double currentTime = 0.0;
    int blockCount = 0;
    
    for (auto& line : lines)
    {
        line = line.trim();
        if (line.isEmpty())
            continue;
        
        // Check if line contains a block marker [SOMETHING]
        if (line.contains("[") && line.contains("]"))
        {
            TimelineBlock block = parseBlock(line, currentTime);
            
            if (block.type != BlockType::Unknown)
            {
                blocks.push_back(block);
                currentTime += block.duration;
                blockCount++;
            }
        }
    }
    
    // Assign track indices based on block types
    assignTrackIndices();
    
    Logger::log(Logger::Level::Info, "Parsed " + juce::String(blockCount) + " blocks from stage script");
    return blockCount;
}

TimelineBlock PatternEngine::parseBlock(const juce::String& blockText, double startTime)
{
    TimelineBlock block;
    
    // Extract block type from [TAG]
    int startBracket = blockText.indexOf("[");
    int endBracket = blockText.indexOf("]");
    
    if (startBracket < 0 || endBracket < 0 || endBracket <= startBracket)
        return block;
    
    juce::String tag = blockText.substring(startBracket + 1, endBracket).trim();
    block.type = stringToBlockType(tag);
    
    // Extract content after the tag
    block.content = blockText.substring(endBracket + 1).trim();
    
    // Set timing
    block.startTime = quantizationEnabled ? quantizeTime(startTime) : startTime;
    block.duration = defaultBlockDuration;
    
    // Parse duration from content if specified (e.g., "duration:2.0")
    if (block.content.contains("duration:"))
    {
        int durationStart = block.content.indexOf("duration:") + 9;
        int durationEnd = block.content.indexOfChar(durationStart, ' ');
        if (durationEnd < 0) durationEnd = block.content.length();
        
        juce::String durationStr = block.content.substring(durationStart, durationEnd);
        double parsedDuration = durationStr.getDoubleValue();
        if (parsedDuration > 0.0)
            block.duration = parsedDuration;
    }
    
    return block;
}

void PatternEngine::assignTrackIndices()
{
    // Assign track indices based on block type
    for (auto& block : blocks)
    {
        block.trackIndex = getTrackIndexForBlockType(block.type);
    }
}

int PatternEngine::getTrackIndexForBlockType(BlockType type) const
{
    // Track layout:
    // 0 = Vocals
    // 1 = 808 Bass
    // 2 = Hi-Hats
    // 3 = Snare
    // 4 = Piano
    // 5 = Synth/Pad
    
    switch (type)
    {
        case BlockType::Vocal:
        case BlockType::Hook:
        case BlockType::Verse:
        case BlockType::Intro:
        case BlockType::Outro:
            return 0; // Vocal track
            
        case BlockType::Drum_808:
            return 1;
            
        case BlockType::Drum_HiHat:
            return 2;
            
        case BlockType::Drum_Snare:
            return 3;
            
        case BlockType::Instrument_Piano:
            return 4;
            
        case BlockType::Instrument_Synth:
            return 5;
            
        default:
            return 0;
    }
}

std::vector<TimelineBlock> PatternEngine::getActiveBlocks(double time) const
{
    const juce::ScopedLock sl(blockLock);
    
    std::vector<TimelineBlock> activeBlocks;
    
    for (const auto& block : blocks)
    {
        double endTime = block.startTime + block.duration;
        if (time >= block.startTime && time < endTime)
        {
            activeBlocks.push_back(block);
        }
    }
    
    return activeBlocks;
}

std::vector<TimelineBlock> PatternEngine::getBlocksForTrack(int trackIndex) const
{
    const juce::ScopedLock sl(blockLock);
    
    std::vector<TimelineBlock> trackBlocks;
    
    for (const auto& block : blocks)
    {
        if (block.trackIndex == trackIndex)
        {
            trackBlocks.push_back(block);
        }
    }
    
    return trackBlocks;
}

void PatternEngine::setBPM(double bpm)
{
    if (bpm > 0.0)
    {
        currentBPM = bpm;
        Logger::log(Logger::Level::Info, "BPM set to: " + juce::String(bpm));
    }
}

double PatternEngine::quantizeTime(double time) const
{
    if (!quantizationEnabled)
        return time;
    
    // Quantize to nearest beat
    double beatDuration = 60.0 / currentBPM;
    double beats = time / beatDuration;
    double quantizedBeats = std::round(beats);
    return quantizedBeats * beatDuration;
}

double PatternEngine::beatsToSeconds(double beats) const
{
    return beats * (60.0 / currentBPM);
}

double PatternEngine::secondsToBeats(double seconds) const
{
    return seconds / (60.0 / currentBPM);
}

void PatternEngine::clearBlocks()
{
    const juce::ScopedLock sl(blockLock);
    blocks.clear();
}

void PatternEngine::addBlock(const TimelineBlock& block)
{
    const juce::ScopedLock sl(blockLock);
    blocks.push_back(block);
}

void PatternEngine::removeBlock(int index)
{
    const juce::ScopedLock sl(blockLock);
    
    if (index >= 0 && index < (int)blocks.size())
    {
        blocks.erase(blocks.begin() + index);
    }
}

void PatternEngine::updateTransport(bool isPlaying, double position)
{
    playing = isPlaying;
    playheadPosition = position;
}

void PatternEngine::setDefaultBlockDuration(double seconds)
{
    if (seconds > 0.0)
    {
        defaultBlockDuration = seconds;
    }
}

void PatternEngine::setQuantizationEnabled(bool enabled)
{
    quantizationEnabled = enabled;
    Logger::log(Logger::Level::Info, "Quantization: " + juce::String(enabled ? "enabled" : "disabled"));
}

} // namespace MAEVN
