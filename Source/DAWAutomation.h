/**
 * @file DAWAutomation.h
 * @brief DAW Automation Hooks for exposing FX parameters as DAW-automatable
 * 
 * This module provides full DAW automation support, exposing all FX parameters
 * (Ghost Choir, Tone Shaper, etc.) as automatable parameters that can be
 * controlled by the host DAW.
 */

#pragma once

#include <JuceHeader.h>
#include <memory>
#include <vector>
#include <functional>
#include "Utilities.h"

namespace MAEVN
{

//==============================================================================
/**
 * @brief Parameter IDs for DAW automation
 */
namespace AutomationIDs
{
    // Master Parameters
    static const juce::String MasterVolume = "masterVolume";
    static const juce::String MasterPan = "masterPan";
    static const juce::String BPM = "bpm";
    
    // Ghost Choir Parameters (Vocal Effect)
    static const juce::String GhostChoirEnabled = "ghostChoirEnabled";
    static const juce::String GhostChoirVoices = "ghostChoirVoices";
    static const juce::String GhostChoirSpread = "ghostChoirSpread";
    static const juce::String GhostChoirDepth = "ghostChoirDepth";
    static const juce::String GhostChoirMix = "ghostChoirMix";
    static const juce::String GhostChoirPitch = "ghostChoirPitch";
    static const juce::String GhostChoirDetune = "ghostChoirDetune";
    
    // Tone Shaper Parameters
    static const juce::String ToneShaperEnabled = "toneShaperEnabled";
    static const juce::String ToneShaperLow = "toneShaperLow";
    static const juce::String ToneShaperMid = "toneShaperMid";
    static const juce::String ToneShaperHigh = "toneShaperHigh";
    static const juce::String ToneShaperPresence = "toneShaperPresence";
    static const juce::String ToneShaperWarmth = "toneShaperWarmth";
    static const juce::String ToneShaperAir = "toneShaperAir";
    
    // Cinematic FX Parameters
    static const juce::String CinematicEnabled = "cinematicEnabled";
    static const juce::String CinematicReverbSize = "cinematicReverbSize";
    static const juce::String CinematicReverbMix = "cinematicReverbMix";
    static const juce::String CinematicDelayTime = "cinematicDelayTime";
    static const juce::String CinematicDelayMix = "cinematicDelayMix";
    static const juce::String CinematicModulation = "cinematicModulation";
    static const juce::String CinematicSaturation = "cinematicSaturation";
    
    // Compressor Parameters
    static const juce::String CompressorThreshold = "compressorThreshold";
    static const juce::String CompressorRatio = "compressorRatio";
    static const juce::String CompressorAttack = "compressorAttack";
    static const juce::String CompressorRelease = "compressorRelease";
    
    // EQ Parameters
    static const juce::String EQLowGain = "eqLowGain";
    static const juce::String EQMidGain = "eqMidGain";
    static const juce::String EQHighGain = "eqHighGain";
    
    // Limiter Parameters
    static const juce::String LimiterCeiling = "limiterCeiling";
    static const juce::String LimiterRelease = "limiterRelease";
    
    // Track-specific FX Mode
    static const juce::String Track0FXMode = "track0FXMode";
    static const juce::String Track1FXMode = "track1FXMode";
    static const juce::String Track2FXMode = "track2FXMode";
    static const juce::String Track3FXMode = "track3FXMode";
    static const juce::String Track4FXMode = "track4FXMode";
    static const juce::String Track5FXMode = "track5FXMode";
    
    // AI Processing Parameters
    static const juce::String AIProcessingEnabled = "aiProcessingEnabled";
    static const juce::String AIAutotuneStrength = "aiAutotuneStrength";
    static const juce::String AIVocalClarity = "aiVocalClarity";
    static const juce::String AIHarmonyDepth = "aiHarmonyDepth";
}

//==============================================================================
/**
 * @brief Parameter listener interface for automation changes
 */
class AutomationListener
{
public:
    virtual ~AutomationListener() = default;
    
    /**
     * @brief Called when a parameter value changes
     * @param parameterID The ID of the parameter that changed
     * @param newValue The new value of the parameter
     */
    virtual void onParameterChanged(const juce::String& parameterID, float newValue) = 0;
};

//==============================================================================
/**
 * @brief DAW Automation Host - manages all automatable parameters
 * 
 * This class creates and manages an AudioProcessorValueTreeState that
 * exposes all FX parameters to the DAW for automation.
 */
class DAWAutomation
{
public:
    DAWAutomation(juce::AudioProcessor& processor);
    ~DAWAutomation();
    
    /**
     * @brief Get the AudioProcessorValueTreeState for DAW integration
     */
    juce::AudioProcessorValueTreeState& getValueTreeState() { return *apvts; }
    
    /**
     * @brief Get parameter value by ID
     * @param parameterID The parameter identifier
     * @return Current parameter value
     */
    float getParameterValue(const juce::String& parameterID) const;
    
    /**
     * @brief Set parameter value by ID (for internal use)
     * @param parameterID The parameter identifier
     * @param value New parameter value
     */
    void setParameterValue(const juce::String& parameterID, float value);
    
    /**
     * @brief Add a listener for parameter changes
     * @param listener The listener to add
     */
    void addListener(AutomationListener* listener);
    
    /**
     * @brief Remove a listener
     * @param listener The listener to remove
     */
    void removeListener(AutomationListener* listener);
    
    /**
     * @brief Get parameter as RangedAudioParameter for attachment
     * @param parameterID The parameter identifier
     * @return Pointer to the parameter or nullptr if not found
     */
    juce::RangedAudioParameter* getParameter(const juce::String& parameterID);
    
    /**
     * @brief Create a slider attachment for UI
     * @param parameterID The parameter identifier
     * @param slider The slider to attach
     * @return Unique pointer to the attachment
     */
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
    createSliderAttachment(const juce::String& parameterID, juce::Slider& slider);
    
    /**
     * @brief Create a button attachment for UI
     * @param parameterID The parameter identifier
     * @param button The button to attach
     * @return Unique pointer to the attachment
     */
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
    createButtonAttachment(const juce::String& parameterID, juce::Button& button);
    
    /**
     * @brief Create a combobox attachment for UI
     * @param parameterID The parameter identifier
     * @param comboBox The combo box to attach
     * @return Unique pointer to the attachment
     */
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
    createComboBoxAttachment(const juce::String& parameterID, juce::ComboBox& comboBox);
    
    /**
     * @brief Save state to XML
     * @return XML element containing all parameter states
     */
    std::unique_ptr<juce::XmlElement> saveState() const;
    
    /**
     * @brief Load state from XML
     * @param xml XML element containing parameter states
     */
    void loadState(const juce::XmlElement* xml);
    
    /**
     * @brief Get all parameter IDs
     * @return Array of all parameter identifiers
     */
    juce::StringArray getAllParameterIDs() const;
    
private:
    juce::AudioProcessor& audioProcessor;
    std::unique_ptr<juce::AudioProcessorValueTreeState> apvts;
    std::vector<AutomationListener*> listeners;
    
    /**
     * @brief Create parameter layout for APVTS
     */
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    /**
     * @brief Notify all listeners of parameter change
     */
    void notifyListeners(const juce::String& parameterID, float newValue);
    
    /**
     * @brief Parameter listener callback
     */
    class ParameterCallback : public juce::AudioProcessorValueTreeState::Listener
    {
    public:
        ParameterCallback(DAWAutomation& owner) : dawAutomation(owner) {}
        
        void parameterChanged(const juce::String& parameterID, float newValue) override
        {
            dawAutomation.notifyListeners(parameterID, newValue);
        }
        
    private:
        DAWAutomation& dawAutomation;
    };
    
    std::unique_ptr<ParameterCallback> parameterCallback;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DAWAutomation)
};

//==============================================================================
/**
 * @brief Ghost Choir effect - multi-voice harmony generator
 */
class GhostChoirEffect
{
public:
    GhostChoirEffect();
    ~GhostChoirEffect();
    
    void prepare(double sampleRate, int maxBlockSize);
    void process(juce::AudioBuffer<float>& buffer, int numSamples);
    void reset();
    
    void setEnabled(bool enabled) { this->enabled = enabled; }
    void setNumVoices(int voices);
    void setSpread(float spread);
    void setDepth(float depth);
    void setMix(float mix);
    void setPitchShift(float semitones);
    void setDetune(float cents);
    
    bool isEnabled() const { return enabled; }
    
private:
    bool enabled;
    int numVoices;
    float spreadAmount;
    float depthAmount;
    float mixLevel;
    float pitchShiftSemitones;
    float detuneCents;
    
    double currentSampleRate;
    
    // Delay lines for creating voice spread
    std::vector<juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd>> delayLines;
    
    // LFO for subtle modulation
    float lfoPhase;
    float lfoRate;
    
    // Processing buffer
    juce::AudioBuffer<float> processingBuffer;
    
    float generateLFO();
};

//==============================================================================
/**
 * @brief Tone Shaper effect - multi-band tone sculpting
 */
class ToneShaperEffect
{
public:
    ToneShaperEffect();
    ~ToneShaperEffect();
    
    void prepare(double sampleRate, int maxBlockSize);
    void process(juce::AudioBuffer<float>& buffer, int numSamples);
    void reset();
    
    void setEnabled(bool enabled) { this->enabled = enabled; }
    void setLowGain(float dB);
    void setMidGain(float dB);
    void setHighGain(float dB);
    void setPresence(float amount);
    void setWarmth(float amount);
    void setAir(float amount);
    
    bool isEnabled() const { return enabled; }
    
private:
    bool enabled;
    float lowGain;
    float midGain;
    float highGain;
    float presenceAmount;
    float warmthAmount;
    float airAmount;
    
    double currentSampleRate;
    
    // Multi-band filters
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, 
                                    juce::dsp::IIR::Coefficients<float>> lowShelf;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, 
                                    juce::dsp::IIR::Coefficients<float>> midPeak;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, 
                                    juce::dsp::IIR::Coefficients<float>> highShelf;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, 
                                    juce::dsp::IIR::Coefficients<float>> presenceFilter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, 
                                    juce::dsp::IIR::Coefficients<float>> warmthFilter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, 
                                    juce::dsp::IIR::Coefficients<float>> airFilter;
    
    void updateFilters();
};

} // namespace MAEVN
