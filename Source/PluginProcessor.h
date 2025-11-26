/**
 * @file PluginProcessor.h
 * @brief Main VST3 audio processor
 * 
 * This is the core audio processing component that integrates all
 * the MAEVN modules and handles DAW communication.
 */

#pragma once

#include <JuceHeader.h>
#include "OnnxEngine.h"
#include "PatternEngine.h"
#include "AIFXEngine.h"
#include "CinematicAudioEnhancer.h"
#include "FXPresetManager.h"
#include "GlobalUndoManager.h"
#include "Utilities.h"

namespace MAEVN
{

class MAEVNAudioProcessor : public juce::AudioProcessor
{
public:
    MAEVNAudioProcessor();
    ~MAEVNAudioProcessor() override;
    
    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    
    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    
    //==============================================================================
    const juce::String getName() const override;
    
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    
    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;
    
    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    
    //==============================================================================
    // Public access to engines for editor
    OnnxEngine& getOnnxEngine() { return onnxEngine; }
    PatternEngine& getPatternEngine() { return patternEngine; }
    AIFXEngine& getAIFXEngine() { return aiFXEngine; }
    CinematicAudioEnhancer& getCinematicEnhancer() { return cinematicEnhancer; }
    FXPresetManager& getPresetManager() { return presetManager; }
    GlobalUndoManager& getUndoManager() { return undoManager; }
    
    //==============================================================================
    // Cinematic enhancement control
    void setCinematicEnhancerEnabled(bool enabled) { cinematicEnhancerEnabled = enabled; }
    bool isCinematicEnhancerEnabled() const { return cinematicEnhancerEnabled; }
    
private:
    //==============================================================================
    OnnxEngine onnxEngine;
    PatternEngine patternEngine;
    AIFXEngine aiFXEngine;
    CinematicAudioEnhancer cinematicEnhancer;
    FXPresetManager presetManager;
    GlobalUndoManager undoManager;
    
    double currentSampleRate;
    int currentBlockSize;
    
    // Cinematic enhancer enable flag
    bool cinematicEnhancerEnabled;
    
    // Audio buffers for track processing
    std::array<juce::AudioBuffer<float>, 6> trackBuffers; // 6 tracks
    
    /**
     * @brief Initialize models and presets
     */
    void initializeModelsAndPresets();
    
    /**
     * @brief Process audio for all tracks
     */
    void processAllTracks(juce::AudioBuffer<float>& buffer, int numSamples);
    
    /**
     * @brief Update transport info from DAW
     */
    void updateTransportInfo();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MAEVNAudioProcessor)
};

} // namespace MAEVN
