/**
 * @file LoopRegionSync.cpp
 * @brief Implementation of Loop Region Sync functionality
 */

#include "LoopRegionSync.h"

namespace MAEVN
{

//==============================================================================
// LoopRegionSync Implementation
//==============================================================================

LoopRegionSync::LoopRegionSync(PatternEngine* engine)
    : patternEngine(engine)
    , autoFitEnabled(false)
    , defaultFitMode(FitMode::Smart)
{
    Logger::log(Logger::Level::Info, "LoopRegionSync initialized");
}

LoopRegionSync::~LoopRegionSync()
{
}

void LoopRegionSync::updateFromPlayHead(juce::AudioPlayHead* playHead)
{
    if (playHead == nullptr)
        return;
    
    const juce::ScopedLock sl(syncLock);
    
    previousRegion = currentRegion;
    
    // Get current position info from DAW
    if (auto posInfo = playHead->getPosition())
    {
        // Check if DAW is looping
        if (auto loopPoints = posInfo->getLoopPoints())
        {
            currentRegion.isLooping = posInfo->getIsLooping();
            
            // Convert PPQ to seconds using BPM
            double bpm = 120.0;
            if (auto tempo = posInfo->getBpm())
            {
                bpm = *tempo;
            }
            
            double beatsPerSecond = bpm / 60.0;
            
            currentRegion.startTime = loopPoints->ppqStart / beatsPerSecond;
            currentRegion.endTime = loopPoints->ppqEnd / beatsPerSecond;
            currentRegion.hasSelection = true;
        }
        else
        {
            currentRegion.hasSelection = false;
            currentRegion.isLooping = false;
        }
    }
    
    // Check if region changed
    bool regionChanged = (currentRegion.startTime != previousRegion.startTime ||
                         currentRegion.endTime != previousRegion.endTime ||
                         currentRegion.isLooping != previousRegion.isLooping);
    
    if (regionChanged && currentRegion.isValid())
    {
        notifyLoopRegionChanged();
        
        if (autoFitEnabled)
        {
            int affected = fitArrangementToLoop(defaultFitMode);
            notifyArrangementFitted(affected);
        }
    }
}

int LoopRegionSync::fitArrangementToLoop(FitMode mode)
{
    if (!currentRegion.isValid() || patternEngine == nullptr)
        return 0;
    
    const juce::ScopedLock sl(syncLock);
    
    FitMode actualMode = (mode == FitMode::Smart) ? determineBestFitMode() : mode;
    
    int numAffected = 0;
    
    switch (actualMode)
    {
        case FitMode::Stretch:
            numAffected = applyStretchMode();
            break;
            
        case FitMode::Trim:
            numAffected = applyTrimMode();
            break;
            
        case FitMode::Loop:
            numAffected = applyLoopMode();
            break;
            
        case FitMode::Quantize:
            numAffected = applyQuantizeMode();
            break;
            
        default:
            break;
    }
    
    Logger::log(Logger::Level::Info, "Fitted " + juce::String(numAffected) + 
                " blocks to loop region");
    
    return numAffected;
}

int LoopRegionSync::fitBlocksToLoop(const std::vector<int>& blockIndices, FitMode mode)
{
    if (!currentRegion.isValid() || patternEngine == nullptr || blockIndices.empty())
        return 0;
    
    const juce::ScopedLock sl(syncLock);
    
    // Get blocks and calculate total duration
    const auto& allBlocks = patternEngine->getBlocks();
    double totalDuration = 0.0;
    
    for (int idx : blockIndices)
    {
        if (idx >= 0 && idx < static_cast<int>(allBlocks.size()))
        {
            totalDuration += allBlocks[idx].duration;
        }
    }
    
    if (totalDuration <= 0.0)
        return 0;
    
    // Calculate stretch factor
    double targetDuration = currentRegion.getDuration();
    double stretchFactor = targetDuration / totalDuration;
    
    // Apply stretch to selected blocks
    // Note: In a full implementation, this would modify the blocks in the pattern engine
    // Here we demonstrate the calculation
    
    return static_cast<int>(blockIndices.size());
}

void LoopRegionSync::addListener(LoopRegionListener* listener)
{
    const juce::ScopedLock sl(syncLock);
    listeners.push_back(listener);
}

void LoopRegionSync::removeListener(LoopRegionListener* listener)
{
    const juce::ScopedLock sl(syncLock);
    listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
}

void LoopRegionSync::setLoopRegion(const LoopRegion& region)
{
    const juce::ScopedLock sl(syncLock);
    
    previousRegion = currentRegion;
    currentRegion = region;
    
    notifyLoopRegionChanged();
    
    if (autoFitEnabled && currentRegion.isValid())
    {
        int affected = fitArrangementToLoop(defaultFitMode);
        notifyArrangementFitted(affected);
    }
}

void LoopRegionSync::clearLoopRegion()
{
    const juce::ScopedLock sl(syncLock);
    
    currentRegion = LoopRegion();
    notifyLoopRegionChanged();
}

std::vector<int> LoopRegionSync::getBlocksInLoopRegion() const
{
    std::vector<int> result;
    
    if (!currentRegion.isValid() || patternEngine == nullptr)
        return result;
    
    const auto& blocks = patternEngine->getBlocks();
    
    for (size_t i = 0; i < blocks.size(); ++i)
    {
        const auto& block = blocks[i];
        double blockEnd = block.startTime + block.duration;
        
        // Check if block overlaps with loop region
        if (block.startTime < currentRegion.endTime && 
            blockEnd > currentRegion.startTime)
        {
            result.push_back(static_cast<int>(i));
        }
    }
    
    return result;
}

std::vector<double> LoopRegionSync::calculateOptimalFit(FitMode mode) const
{
    std::vector<double> adjustedDurations;
    
    if (!currentRegion.isValid() || patternEngine == nullptr)
        return adjustedDurations;
    
    auto blockIndices = getBlocksInLoopRegion();
    const auto& allBlocks = patternEngine->getBlocks();
    
    if (blockIndices.empty())
        return adjustedDurations;
    
    // Calculate current total duration
    double totalDuration = 0.0;
    for (int idx : blockIndices)
    {
        totalDuration += allBlocks[idx].duration;
    }
    
    double targetDuration = currentRegion.getDuration();
    
    switch (mode)
    {
        case FitMode::Stretch:
        {
            double stretchFactor = targetDuration / totalDuration;
            for (int idx : blockIndices)
            {
                adjustedDurations.push_back(allBlocks[idx].duration * stretchFactor);
            }
            break;
        }
        
        case FitMode::Trim:
        {
            double remaining = targetDuration;
            for (int idx : blockIndices)
            {
                double blockDur = allBlocks[idx].duration;
                if (remaining > 0)
                {
                    adjustedDurations.push_back(std::min(blockDur, remaining));
                    remaining -= blockDur;
                }
                else
                {
                    adjustedDurations.push_back(0.0);
                }
            }
            break;
        }
        
        case FitMode::Quantize:
        {
            double beatDuration = 60.0 / patternEngine->getBPM();
            for (int idx : blockIndices)
            {
                double blockDur = allBlocks[idx].duration;
                double numBeats = std::round(blockDur / beatDuration);
                adjustedDurations.push_back(numBeats * beatDuration);
            }
            break;
        }
        
        default:
            for (int idx : blockIndices)
            {
                adjustedDurations.push_back(allBlocks[idx].duration);
            }
            break;
    }
    
    return adjustedDurations;
}

int LoopRegionSync::quantizeBlocksToGrid()
{
    return applyQuantizeMode();
}

double LoopRegionSync::stretchBlocksToFit()
{
    if (!currentRegion.isValid() || patternEngine == nullptr)
        return 1.0;
    
    auto blockIndices = getBlocksInLoopRegion();
    const auto& allBlocks = patternEngine->getBlocks();
    
    if (blockIndices.empty())
        return 1.0;
    
    // Calculate current total duration
    double totalDuration = 0.0;
    for (int idx : blockIndices)
    {
        totalDuration += allBlocks[idx].duration;
    }
    
    if (totalDuration <= 0.0)
        return 1.0;
    
    double stretchFactor = currentRegion.getDuration() / totalDuration;
    
    // Apply stretch factor
    applyStretchMode();
    
    return stretchFactor;
}

int LoopRegionSync::loopBlocksToFill()
{
    return applyLoopMode();
}

void LoopRegionSync::notifyLoopRegionChanged()
{
    for (auto* listener : listeners)
    {
        listener->onLoopRegionChanged(currentRegion);
    }
}

void LoopRegionSync::notifyArrangementFitted(int numBlocksAffected)
{
    for (auto* listener : listeners)
    {
        listener->onArrangementFitted(currentRegion, numBlocksAffected);
    }
}

int LoopRegionSync::applyStretchMode()
{
    auto blockIndices = getBlocksInLoopRegion();
    
    if (blockIndices.empty())
        return 0;
    
    // Calculate stretch factor and apply
    // In a full implementation, this would modify the pattern engine blocks
    auto adjustedDurations = calculateOptimalFit(FitMode::Stretch);
    
    Logger::log(Logger::Level::Info, "Applied stretch mode to " + 
                juce::String(static_cast<int>(blockIndices.size())) + " blocks");
    
    return static_cast<int>(blockIndices.size());
}

int LoopRegionSync::applyTrimMode()
{
    auto blockIndices = getBlocksInLoopRegion();
    
    if (blockIndices.empty())
        return 0;
    
    auto adjustedDurations = calculateOptimalFit(FitMode::Trim);
    
    Logger::log(Logger::Level::Info, "Applied trim mode to " + 
                juce::String(static_cast<int>(blockIndices.size())) + " blocks");
    
    return static_cast<int>(blockIndices.size());
}

int LoopRegionSync::applyLoopMode()
{
    auto blockIndices = getBlocksInLoopRegion();
    
    if (blockIndices.empty() || patternEngine == nullptr)
        return 0;
    
    const auto& allBlocks = patternEngine->getBlocks();
    
    // Calculate how many loop iterations fit
    double totalDuration = 0.0;
    for (int idx : blockIndices)
    {
        totalDuration += allBlocks[idx].duration;
    }
    
    if (totalDuration <= 0.0)
        return 0;
    
    double targetDuration = currentRegion.getDuration();
    int numLoops = static_cast<int>(std::ceil(targetDuration / totalDuration));
    
    Logger::log(Logger::Level::Info, "Applied loop mode: " + 
                juce::String(numLoops) + " iterations");
    
    return numLoops;
}

int LoopRegionSync::applyQuantizeMode()
{
    auto blockIndices = getBlocksInLoopRegion();
    
    if (blockIndices.empty())
        return 0;
    
    auto adjustedDurations = calculateOptimalFit(FitMode::Quantize);
    
    Logger::log(Logger::Level::Info, "Applied quantize mode to " + 
                juce::String(static_cast<int>(blockIndices.size())) + " blocks");
    
    return static_cast<int>(blockIndices.size());
}

FitMode LoopRegionSync::determineBestFitMode() const
{
    if (!currentRegion.isValid() || patternEngine == nullptr)
        return FitMode::Stretch;
    
    auto blockIndices = getBlocksInLoopRegion();
    const auto& allBlocks = patternEngine->getBlocks();
    
    if (blockIndices.empty())
        return FitMode::Stretch;
    
    // Calculate current total duration
    double totalDuration = 0.0;
    for (int idx : blockIndices)
    {
        totalDuration += allBlocks[idx].duration;
    }
    
    double targetDuration = currentRegion.getDuration();
    double ratio = targetDuration / totalDuration;
    
    // Determine best mode based on ratio
    if (ratio > 0.9 && ratio < 1.1)
    {
        // Very close to target - use quantize for minor adjustments
        return FitMode::Quantize;
    }
    else if (ratio > 1.5)
    {
        // Target is much longer - loop to fill
        return FitMode::Loop;
    }
    else if (ratio < 0.5)
    {
        // Target is much shorter - trim excess
        return FitMode::Trim;
    }
    else
    {
        // Moderate difference - stretch to fit
        return FitMode::Stretch;
    }
}

} // namespace MAEVN
