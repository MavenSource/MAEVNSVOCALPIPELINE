/**
 * @file PatternEngine.h
 * @brief Timeline and arrangement engine for parsing stage scripts
 * 
 * This module handles parsing of lyrical stage script input, manages timeline
 * blocks, BPM-aware quantization, and DAW transport synchronization.
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
 * @brief Pattern Engine - manages timeline arrangement and block parsing
 */
class PatternEngine
{
public:
    PatternEngine();
    ~PatternEngine();
    
    /**
     * @brief Parse stage script input into timeline blocks
     * @param scriptInput Text with [HOOK], [VERSE], [808] etc.
     * @return Number of blocks parsed
     */
    int parseStageScript(const juce::String& scriptInput);
    
    /**
     * @brief Get all timeline blocks
     */
    const std::vector<TimelineBlock>& getBlocks() const { return blocks; }
    
    /**
     * @brief Get blocks active at a specific time
     * @param time Time in seconds
     * @return Vector of active blocks
     */
    std::vector<TimelineBlock> getActiveBlocks(double time) const;
    
    /**
     * @brief Get blocks for a specific track
     * @param trackIndex Track/lane index
     * @return Vector of blocks for that track
     */
    std::vector<TimelineBlock> getBlocksForTrack(int trackIndex) const;
    
    /**
     * @brief Set BPM for quantization
     */
    void setBPM(double bpm);
    
    /**
     * @brief Get current BPM
     */
    double getBPM() const { return currentBPM; }
    
    /**
     * @brief Quantize time to nearest beat
     * @param time Time in seconds
     * @return Quantized time
     */
    double quantizeTime(double time) const;
    
    /**
     * @brief Convert beats to seconds
     */
    double beatsToSeconds(double beats) const;
    
    /**
     * @brief Convert seconds to beats
     */
    double secondsToBeats(double seconds) const;
    
    /**
     * @brief Clear all blocks
     */
    void clearBlocks();
    
    /**
     * @brief Add a single block manually
     */
    void addBlock(const TimelineBlock& block);
    
    /**
     * @brief Remove block at index
     */
    void removeBlock(int index);
    
    /**
     * @brief Update DAW transport information
     * @param isPlaying Is DAW playing
     * @param position Current playhead position in seconds
     */
    void updateTransport(bool isPlaying, double position);
    
    /**
     * @brief Get current playhead position
     */
    double getCurrentPosition() const { return playheadPosition; }
    
    /**
     * @brief Check if currently playing
     */
    bool isPlaying() const { return playing; }
    
    /**
     * @brief Set default block duration
     */
    void setDefaultBlockDuration(double seconds);
    
    /**
     * @brief Enable/disable auto-quantization
     */
    void setQuantizationEnabled(bool enabled);
    
private:
    std::vector<TimelineBlock> blocks;
    double currentBPM;
    double playheadPosition;
    bool playing;
    double defaultBlockDuration;
    bool quantizationEnabled;
    
    mutable juce::CriticalSection blockLock;
    
    /**
     * @brief Parse a single block from text
     * Example: "[HOOK] This is the hook lyrics"
     */
    TimelineBlock parseBlock(const juce::String& blockText, double startTime);
    
    /**
     * @brief Assign track indices based on block types
     */
    void assignTrackIndices();
    
    /**
     * @brief Get track index for a block type
     */
    int getTrackIndexForBlockType(BlockType type) const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PatternEngine)
};

} // namespace MAEVN
