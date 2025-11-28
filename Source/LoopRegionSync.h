/**
 * @file LoopRegionSync.h
 * @brief Loop Region Sync - Auto-fit arrangement to DAW selection
 * 
 * This module provides synchronization between the plugin's timeline
 * and the DAW's loop/selection region, automatically fitting arrangements
 * to match the selected area.
 */

#pragma once

#include <JuceHeader.h>
#include <memory>
#include <vector>
#include <functional>
#include "Utilities.h"
#include "PatternEngine.h"

namespace MAEVN
{

//==============================================================================
/**
 * @brief Loop region information from DAW
 */
struct LoopRegion
{
    double startTime;       ///< Start time in seconds
    double endTime;         ///< End time in seconds
    bool isLooping;         ///< Whether DAW is currently looping
    bool hasSelection;      ///< Whether a region is selected
    
    LoopRegion()
        : startTime(0.0)
        , endTime(0.0)
        , isLooping(false)
        , hasSelection(false)
    {}
    
    double getDuration() const { return endTime - startTime; }
    bool isValid() const { return endTime > startTime; }
};

//==============================================================================
/**
 * @brief Fit mode options for arrangement
 */
enum class FitMode
{
    Stretch,        ///< Stretch/compress blocks to fit region
    Trim,           ///< Trim blocks to fit within region
    Loop,           ///< Loop blocks to fill region
    Quantize,       ///< Snap blocks to nearest beat boundaries
    Smart           ///< Automatically choose best mode
};

//==============================================================================
/**
 * @brief Listener interface for loop region changes
 */
class LoopRegionListener
{
public:
    virtual ~LoopRegionListener() = default;
    
    /**
     * @brief Called when the DAW loop region changes
     * @param region The new loop region
     */
    virtual void onLoopRegionChanged(const LoopRegion& region) = 0;
    
    /**
     * @brief Called when arrangement is fitted to region
     * @param region The target region
     * @param numBlocksAffected Number of blocks that were adjusted
     */
    virtual void onArrangementFitted(const LoopRegion& region, int numBlocksAffected) = 0;
};

//==============================================================================
/**
 * @brief Main class for Loop Region Sync functionality
 * 
 * Handles synchronization between the plugin's arrangement and
 * the DAW's loop/selection region.
 */
class LoopRegionSync
{
public:
    LoopRegionSync(PatternEngine* patternEngine);
    ~LoopRegionSync();
    
    /**
     * @brief Update with current DAW transport info
     * @param playHead The DAW's play head
     */
    void updateFromPlayHead(juce::AudioPlayHead* playHead);
    
    /**
     * @brief Get current loop region
     * @return Current loop region info
     */
    LoopRegion getCurrentLoopRegion() const { return currentRegion; }
    
    /**
     * @brief Check if a loop region is currently active
     */
    bool hasActiveLoopRegion() const { return currentRegion.isValid(); }
    
    /**
     * @brief Fit arrangement to current loop region
     * @param mode The fit mode to use
     * @return Number of blocks affected
     */
    int fitArrangementToLoop(FitMode mode = FitMode::Smart);
    
    /**
     * @brief Fit specific blocks to loop region
     * @param blockIndices Indices of blocks to fit
     * @param mode The fit mode to use
     * @return Number of blocks affected
     */
    int fitBlocksToLoop(const std::vector<int>& blockIndices, FitMode mode = FitMode::Smart);
    
    /**
     * @brief Auto-fit arrangement when loop region changes
     * @param enabled Whether to enable auto-fit
     */
    void setAutoFitEnabled(bool enabled) { autoFitEnabled = enabled; }
    
    /**
     * @brief Check if auto-fit is enabled
     */
    bool isAutoFitEnabled() const { return autoFitEnabled; }
    
    /**
     * @brief Set the fit mode for auto-fit
     * @param mode The fit mode to use
     */
    void setDefaultFitMode(FitMode mode) { defaultFitMode = mode; }
    
    /**
     * @brief Get the default fit mode
     */
    FitMode getDefaultFitMode() const { return defaultFitMode; }
    
    /**
     * @brief Add a listener for loop region events
     * @param listener The listener to add
     */
    void addListener(LoopRegionListener* listener);
    
    /**
     * @brief Remove a listener
     * @param listener The listener to remove
     */
    void removeListener(LoopRegionListener* listener);
    
    /**
     * @brief Manually set the loop region (for testing or manual override)
     * @param region The loop region to set
     */
    void setLoopRegion(const LoopRegion& region);
    
    /**
     * @brief Clear the current loop region
     */
    void clearLoopRegion();
    
    /**
     * @brief Get blocks that intersect with the loop region
     * @return Indices of intersecting blocks
     */
    std::vector<int> getBlocksInLoopRegion() const;
    
    /**
     * @brief Calculate optimal fit for blocks in region
     * @param mode The fit mode to use
     * @return Vector of adjusted block durations
     */
    std::vector<double> calculateOptimalFit(FitMode mode) const;
    
    /**
     * @brief Quantize blocks to beat grid within loop region
     * @return Number of blocks quantized
     */
    int quantizeBlocksToGrid();
    
    /**
     * @brief Stretch blocks proportionally to fill loop region
     * @return Stretch factor applied
     */
    double stretchBlocksToFit();
    
    /**
     * @brief Loop blocks to fill entire loop region
     * @return Number of block copies created
     */
    int loopBlocksToFill();
    
private:
    PatternEngine* patternEngine;
    LoopRegion currentRegion;
    LoopRegion previousRegion;
    
    bool autoFitEnabled;
    FitMode defaultFitMode;
    
    std::vector<LoopRegionListener*> listeners;
    
    juce::CriticalSection syncLock;
    
    /**
     * @brief Notify listeners of loop region change
     */
    void notifyLoopRegionChanged();
    
    /**
     * @brief Notify listeners of arrangement fit
     */
    void notifyArrangementFitted(int numBlocksAffected);
    
    /**
     * @brief Apply stretch mode to blocks
     */
    int applyStretchMode();
    
    /**
     * @brief Apply trim mode to blocks
     */
    int applyTrimMode();
    
    /**
     * @brief Apply loop mode to blocks
     */
    int applyLoopMode();
    
    /**
     * @brief Apply quantize mode to blocks
     */
    int applyQuantizeMode();
    
    /**
     * @brief Determine best fit mode automatically
     */
    FitMode determineBestFitMode() const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoopRegionSync)
};

} // namespace MAEVN
