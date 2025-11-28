/**
 * @file LegendaryProducerFXSuiteUltimate.h
 * @brief All-in-One Vocal Processing, Pitch Correction, Harmonization, and Reverb Plugin
 * 
 * This combined plug-in integrates the features of LegendaryProducerFXSuite and EpicSpaceReverb:
 * - Vocal FX Tab: Multiband Compressor, Transient Shaper, De-Esser, Saturation, Stereo Widener, Limiter
 * - PTH Vocal Clone Tab: Pitch Correction, Timbre Shaping, Harmony Generation
 * - EpicSpaceReverb Tab: Advanced Reverb with Room Size, Decay Time, Damping, Pre-Delay, etc.
 */

#pragma once

#include <JuceHeader.h>
#include "DSPModules.h"
#include "Utilities.h"

namespace MAEVN
{

//==============================================================================
/**
 * @brief LegendaryProducerFXSuiteUltimate Audio Processor
 * 
 * Main audio processor for the combined FX suite, PTH vocal clone, and reverb.
 */
class LegendaryProducerFXSuiteUltimateAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    LegendaryProducerFXSuiteUltimateAudioProcessor();
    ~LegendaryProducerFXSuiteUltimateAudioProcessor() override;

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
    // Module enable/disable
    //==============================================================================
    
    // Vocal FX Tab modules
    void setMultibandCompressorEnabled(bool enabled) { multibandCompressorEnabled = enabled; }
    bool isMultibandCompressorEnabled() const { return multibandCompressorEnabled; }
    
    void setTransientShaperEnabled(bool enabled) { transientShaperEnabled = enabled; }
    bool isTransientShaperEnabled() const { return transientShaperEnabled; }
    
    void setDeEsserEnabled(bool enabled) { deEsserEnabled = enabled; }
    bool isDeEsserEnabled() const { return deEsserEnabled; }
    
    void setSaturationEnabled(bool enabled) { saturationEnabled = enabled; }
    bool isSaturationEnabled() const { return saturationEnabled; }
    
    void setStereoWidenerEnabled(bool enabled) { stereoWidenerEnabled = enabled; }
    bool isStereoWidenerEnabled() const { return stereoWidenerEnabled; }
    
    void setLimiterEnabled(bool enabled) { limiterEnabled = enabled; }
    bool isLimiterEnabled() const { return limiterEnabled; }
    
    // PTH Vocal Clone Tab
    void setPTHVocalCloneEnabled(bool enabled) { pthVocalCloneEnabled = enabled; }
    bool isPTHVocalCloneEnabled() const { return pthVocalCloneEnabled; }
    
    // Epic Space Reverb Tab
    void setEpicSpaceReverbEnabled(bool enabled) { epicSpaceReverbEnabled = enabled; }
    bool isEpicSpaceReverbEnabled() const { return epicSpaceReverbEnabled; }

    //==============================================================================
    // Direct access to DSP modules for parameter control
    //==============================================================================
    
    dspmodules::MultibandCompressor& getMultibandCompressor() { return multibandCompressor; }
    dspmodules::TransientShaper& getTransientShaper() { return transientShaper; }
    dspmodules::DeEsser& getDeEsser() { return deEsser; }
    dspmodules::Saturation& getSaturation() { return saturation; }
    dspmodules::StereoWidener& getStereoWidener() { return stereoWidener; }
    dspmodules::Limiter& getLimiter() { return limiter; }
    dspmodules::PTHVocalClone& getPTHVocalClone() { return pthVocalClone; }
    dspmodules::EpicSpaceReverb& getEpicSpaceReverb() { return epicSpaceReverb; }

    //==============================================================================
    // A/B Comparison
    //==============================================================================
    
    void setABComparisonEnabled(bool enabled) { abComparisonEnabled = enabled; }
    bool isABComparisonEnabled() const { return abComparisonEnabled; }
    void toggleABComparison() { abComparisonShowProcessed = !abComparisonShowProcessed; }
    bool isShowingProcessed() const { return abComparisonShowProcessed; }

    //==============================================================================
    // Preset Management
    //==============================================================================
    
    void savePreset(const juce::String& name);
    void loadPreset(const juce::String& name);
    juce::StringArray getPresetNames() const;

    //==============================================================================
    // Metering
    //==============================================================================
    
    float getInputLevel() const { return inputLevel; }
    float getOutputLevel() const { return outputLevel; }
    float getCurrentPitch() const { return currentPitch; }

private:
    //==============================================================================
    dspmodules::MultibandCompressor multibandCompressor;
    dspmodules::TransientShaper transientShaper;
    dspmodules::DeEsser deEsser;
    dspmodules::Saturation saturation;
    dspmodules::StereoWidener stereoWidener;
    dspmodules::Limiter limiter;

    dspmodules::PTHVocalClone pthVocalClone;
    dspmodules::EpicSpaceReverb epicSpaceReverb;

    // Enable flags for each module
    bool multibandCompressorEnabled;
    bool transientShaperEnabled;
    bool deEsserEnabled;
    bool saturationEnabled;
    bool stereoWidenerEnabled;
    bool limiterEnabled;
    bool pthVocalCloneEnabled;
    bool epicSpaceReverbEnabled;

    // A/B Comparison
    bool abComparisonEnabled;
    bool abComparisonShowProcessed;

    // Metering
    std::atomic<float> inputLevel{0.0f};
    std::atomic<float> outputLevel{0.0f};
    std::atomic<float> currentPitch{0.0f};

    // Processing state
    double currentSampleRate;
    int currentBlockSize;

    // Preset storage
    std::map<juce::String, juce::var> presets;

    //==============================================================================
    /**
     * @brief Calculate RMS level of buffer
     */
    float calculateRMSLevel(const juce::AudioBuffer<float>& buffer);

    /**
     * @brief Estimate pitch from buffer (simplified)
     */
    float estimatePitch(const juce::AudioBuffer<float>& buffer);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LegendaryProducerFXSuiteUltimateAudioProcessor)
};

//==============================================================================
/**
 * @brief LegendaryProducerFXSuiteUltimate Editor
 * 
 * GUI editor with tabbed interface for the combined plugin.
 */
class LegendaryProducerFXSuiteUltimateAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    LegendaryProducerFXSuiteUltimateAudioProcessorEditor(LegendaryProducerFXSuiteUltimateAudioProcessor&);
    ~LegendaryProducerFXSuiteUltimateAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    LegendaryProducerFXSuiteUltimateAudioProcessor& audioProcessor;

    // Tabbed component for the three main tabs
    juce::TabbedComponent tabbedComponent{juce::TabbedButtonBar::TabsAtTop};

    // Vocal FX Tab components
    juce::ToggleButton multibandCompressorToggle{"Multiband Compressor"};
    juce::ToggleButton transientShaperToggle{"Transient Shaper"};
    juce::ToggleButton deEsserToggle{"De-Esser"};
    juce::ToggleButton saturationToggle{"Saturation"};
    juce::ToggleButton stereoWidenerToggle{"Stereo Widener"};
    juce::ToggleButton limiterToggle{"Limiter"};

    // PTH Tab components
    juce::ToggleButton pthVocalCloneToggle{"PTH Vocal Clone"};
    juce::Slider pitchCorrectionSlider;
    juce::Slider brightnessSlider;
    juce::Slider formantShiftSlider;

    // Reverb Tab components
    juce::ToggleButton epicSpaceReverbToggle{"Epic Space Reverb"};
    juce::Slider roomSizeSlider;
    juce::Slider decayTimeSlider;
    juce::Slider dampingSlider;
    juce::Slider preDelaySlider;
    juce::Slider wetDryMixSlider;

    // A/B Comparison button
    juce::TextButton abCompareButton{"A/B"};

    // Metering
    juce::Label inputLevelLabel;
    juce::Label outputLevelLabel;

    void setupVocalFXTab();
    void setupPTHTab();
    void setupReverbTab();
    void updateMeterDisplay();

    juce::Timer meterTimer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LegendaryProducerFXSuiteUltimateAudioProcessorEditor)
};

} // namespace MAEVN
