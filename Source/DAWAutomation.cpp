/**
 * @file DAWAutomation.cpp
 * @brief Implementation of DAW Automation Hooks
 */

#include "DAWAutomation.h"

namespace MAEVN
{

//==============================================================================
// DAWAutomation Implementation
//==============================================================================

DAWAutomation::DAWAutomation(juce::AudioProcessor& processor)
    : audioProcessor(processor)
{
    apvts = std::make_unique<juce::AudioProcessorValueTreeState>(
        processor, nullptr, "MAEVN_Parameters", createParameterLayout());
    
    parameterCallback = std::make_unique<ParameterCallback>(*this);
    
    // Add listener for all parameters
    for (const auto& paramID : getAllParameterIDs())
    {
        apvts->addParameterListener(paramID, parameterCallback.get());
    }
    
    Logger::log(Logger::Level::Info, "DAW Automation initialized with " + 
                juce::String(getAllParameterIDs().size()) + " parameters");
}

DAWAutomation::~DAWAutomation()
{
    for (const auto& paramID : getAllParameterIDs())
    {
        apvts->removeParameterListener(paramID, parameterCallback.get());
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout DAWAutomation::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    // Master Parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::MasterVolume, 1),
        "Master Volume",
        juce::NormalisableRange<float>(-60.0f, 12.0f, 0.1f),
        0.0f, "dB"));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::MasterPan, 1),
        "Master Pan",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f),
        0.0f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::BPM, 1),
        "BPM",
        juce::NormalisableRange<float>(60.0f, 200.0f, 1.0f),
        120.0f, "BPM"));
    
    // Ghost Choir Parameters
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(AutomationIDs::GhostChoirEnabled, 1),
        "Ghost Choir Enabled",
        false));
    
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID(AutomationIDs::GhostChoirVoices, 1),
        "Ghost Choir Voices",
        1, 8, 4));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::GhostChoirSpread, 1),
        "Ghost Choir Spread",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        50.0f, "%"));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::GhostChoirDepth, 1),
        "Ghost Choir Depth",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        50.0f, "%"));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::GhostChoirMix, 1),
        "Ghost Choir Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        50.0f, "%"));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::GhostChoirPitch, 1),
        "Ghost Choir Pitch",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.01f),
        0.0f, "st"));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::GhostChoirDetune, 1),
        "Ghost Choir Detune",
        juce::NormalisableRange<float>(-50.0f, 50.0f, 0.1f),
        0.0f, "cents"));
    
    // Tone Shaper Parameters
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(AutomationIDs::ToneShaperEnabled, 1),
        "Tone Shaper Enabled",
        true));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::ToneShaperLow, 1),
        "Tone Shaper Low",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
        0.0f, "dB"));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::ToneShaperMid, 1),
        "Tone Shaper Mid",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
        0.0f, "dB"));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::ToneShaperHigh, 1),
        "Tone Shaper High",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
        0.0f, "dB"));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::ToneShaperPresence, 1),
        "Tone Shaper Presence",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        50.0f, "%"));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::ToneShaperWarmth, 1),
        "Tone Shaper Warmth",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        50.0f, "%"));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::ToneShaperAir, 1),
        "Tone Shaper Air",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        50.0f, "%"));
    
    // Cinematic FX Parameters
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(AutomationIDs::CinematicEnabled, 1),
        "Cinematic FX Enabled",
        true));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::CinematicReverbSize, 1),
        "Cinematic Reverb Size",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::CinematicReverbMix, 1),
        "Cinematic Reverb Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        30.0f, "%"));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::CinematicDelayTime, 1),
        "Cinematic Delay Time",
        juce::NormalisableRange<float>(1.0f, 2000.0f, 1.0f),
        300.0f, "ms"));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::CinematicDelayMix, 1),
        "Cinematic Delay Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        20.0f, "%"));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::CinematicModulation, 1),
        "Cinematic Modulation",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        30.0f, "%"));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::CinematicSaturation, 1),
        "Cinematic Saturation",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        20.0f, "%"));
    
    // Compressor Parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::CompressorThreshold, 1),
        "Compressor Threshold",
        juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f),
        -18.0f, "dB"));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::CompressorRatio, 1),
        "Compressor Ratio",
        juce::NormalisableRange<float>(1.0f, 20.0f, 0.1f),
        4.0f, ":1"));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::CompressorAttack, 1),
        "Compressor Attack",
        juce::NormalisableRange<float>(0.1f, 200.0f, 0.1f, 0.5f),
        10.0f, "ms"));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::CompressorRelease, 1),
        "Compressor Release",
        juce::NormalisableRange<float>(10.0f, 1000.0f, 1.0f, 0.5f),
        100.0f, "ms"));
    
    // EQ Parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::EQLowGain, 1),
        "EQ Low Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
        0.0f, "dB"));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::EQMidGain, 1),
        "EQ Mid Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
        0.0f, "dB"));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::EQHighGain, 1),
        "EQ High Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
        0.0f, "dB"));
    
    // Limiter Parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::LimiterCeiling, 1),
        "Limiter Ceiling",
        juce::NormalisableRange<float>(-12.0f, 0.0f, 0.1f),
        -0.1f, "dB"));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::LimiterRelease, 1),
        "Limiter Release",
        juce::NormalisableRange<float>(10.0f, 500.0f, 1.0f),
        50.0f, "ms"));
    
    // Track FX Mode (as choice parameters)
    juce::StringArray fxModeChoices = { "Off", "DSP", "AI", "Hybrid" };
    
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID(AutomationIDs::Track0FXMode, 1),
        "Vocals FX Mode", fxModeChoices, 3));
    
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID(AutomationIDs::Track1FXMode, 1),
        "808 FX Mode", fxModeChoices, 1));
    
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID(AutomationIDs::Track2FXMode, 1),
        "Hi-Hat FX Mode", fxModeChoices, 1));
    
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID(AutomationIDs::Track3FXMode, 1),
        "Snare FX Mode", fxModeChoices, 1));
    
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID(AutomationIDs::Track4FXMode, 1),
        "Piano FX Mode", fxModeChoices, 1));
    
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID(AutomationIDs::Track5FXMode, 1),
        "Synth FX Mode", fxModeChoices, 1));
    
    // AI Processing Parameters
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(AutomationIDs::AIProcessingEnabled, 1),
        "AI Processing Enabled",
        true));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::AIAutotuneStrength, 1),
        "AI Autotune Strength",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        50.0f, "%"));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::AIVocalClarity, 1),
        "AI Vocal Clarity",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        50.0f, "%"));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AutomationIDs::AIHarmonyDepth, 1),
        "AI Harmony Depth",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        50.0f, "%"));
    
    return { params.begin(), params.end() };
}

float DAWAutomation::getParameterValue(const juce::String& parameterID) const
{
    if (auto* param = apvts->getRawParameterValue(parameterID))
        return param->load();
    return 0.0f;
}

void DAWAutomation::setParameterValue(const juce::String& parameterID, float value)
{
    if (auto* param = apvts->getParameter(parameterID))
        param->setValueNotifyingHost(param->convertTo0to1(value));
}

void DAWAutomation::addListener(AutomationListener* listener)
{
    listeners.push_back(listener);
}

void DAWAutomation::removeListener(AutomationListener* listener)
{
    listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
}

juce::RangedAudioParameter* DAWAutomation::getParameter(const juce::String& parameterID)
{
    return apvts->getParameter(parameterID);
}

std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
DAWAutomation::createSliderAttachment(const juce::String& parameterID, juce::Slider& slider)
{
    return std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        *apvts, parameterID, slider);
}

std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
DAWAutomation::createButtonAttachment(const juce::String& parameterID, juce::Button& button)
{
    return std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        *apvts, parameterID, button);
}

std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
DAWAutomation::createComboBoxAttachment(const juce::String& parameterID, juce::ComboBox& comboBox)
{
    return std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        *apvts, parameterID, comboBox);
}

std::unique_ptr<juce::XmlElement> DAWAutomation::saveState() const
{
    return apvts->copyState().createXml();
}

void DAWAutomation::loadState(const juce::XmlElement* xml)
{
    if (xml != nullptr)
    {
        apvts->replaceState(juce::ValueTree::fromXml(*xml));
    }
}

juce::StringArray DAWAutomation::getAllParameterIDs() const
{
    return {
        AutomationIDs::MasterVolume,
        AutomationIDs::MasterPan,
        AutomationIDs::BPM,
        AutomationIDs::GhostChoirEnabled,
        AutomationIDs::GhostChoirVoices,
        AutomationIDs::GhostChoirSpread,
        AutomationIDs::GhostChoirDepth,
        AutomationIDs::GhostChoirMix,
        AutomationIDs::GhostChoirPitch,
        AutomationIDs::GhostChoirDetune,
        AutomationIDs::ToneShaperEnabled,
        AutomationIDs::ToneShaperLow,
        AutomationIDs::ToneShaperMid,
        AutomationIDs::ToneShaperHigh,
        AutomationIDs::ToneShaperPresence,
        AutomationIDs::ToneShaperWarmth,
        AutomationIDs::ToneShaperAir,
        AutomationIDs::CinematicEnabled,
        AutomationIDs::CinematicReverbSize,
        AutomationIDs::CinematicReverbMix,
        AutomationIDs::CinematicDelayTime,
        AutomationIDs::CinematicDelayMix,
        AutomationIDs::CinematicModulation,
        AutomationIDs::CinematicSaturation,
        AutomationIDs::CompressorThreshold,
        AutomationIDs::CompressorRatio,
        AutomationIDs::CompressorAttack,
        AutomationIDs::CompressorRelease,
        AutomationIDs::EQLowGain,
        AutomationIDs::EQMidGain,
        AutomationIDs::EQHighGain,
        AutomationIDs::LimiterCeiling,
        AutomationIDs::LimiterRelease,
        AutomationIDs::Track0FXMode,
        AutomationIDs::Track1FXMode,
        AutomationIDs::Track2FXMode,
        AutomationIDs::Track3FXMode,
        AutomationIDs::Track4FXMode,
        AutomationIDs::Track5FXMode,
        AutomationIDs::AIProcessingEnabled,
        AutomationIDs::AIAutotuneStrength,
        AutomationIDs::AIVocalClarity,
        AutomationIDs::AIHarmonyDepth
    };
}

void DAWAutomation::notifyListeners(const juce::String& parameterID, float newValue)
{
    for (auto* listener : listeners)
    {
        listener->onParameterChanged(parameterID, newValue);
    }
}

//==============================================================================
// GhostChoirEffect Implementation
//==============================================================================

GhostChoirEffect::GhostChoirEffect()
    : enabled(false)
    , numVoices(4)
    , spreadAmount(0.5f)
    , depthAmount(0.5f)
    , mixLevel(0.5f)
    , pitchShiftSemitones(0.0f)
    , detuneCents(0.0f)
    , currentSampleRate(44100.0)
    , lfoPhase(0.0f)
    , lfoRate(0.5f)
{
}

GhostChoirEffect::~GhostChoirEffect()
{
}

void GhostChoirEffect::prepare(double sampleRate, int maxBlockSize)
{
    currentSampleRate = sampleRate;
    
    // Initialize delay lines for each voice
    delayLines.clear();
    delayLines.resize(8); // Max 8 voices
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = maxBlockSize;
    spec.numChannels = 2;
    
    for (auto& delay : delayLines)
    {
        delay.setMaximumDelayInSamples(static_cast<int>(sampleRate * 0.1)); // 100ms max delay
        delay.prepare(spec);
    }
    
    processingBuffer.setSize(2, maxBlockSize);
}

void GhostChoirEffect::process(juce::AudioBuffer<float>& buffer, int numSamples)
{
    if (!enabled || numVoices < 1)
        return;
    
    processingBuffer.setSize(buffer.getNumChannels(), numSamples, false, false, true);
    processingBuffer.clear();
    
    // Copy dry signal
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        processingBuffer.copyFrom(channel, 0, buffer, channel, 0, numSamples);
    }
    
    // Process each ghost voice
    for (int voice = 0; voice < numVoices && voice < (int)delayLines.size(); ++voice)
    {
        // Calculate delay time for this voice based on spread
        float voiceSpread = (static_cast<float>(voice) - static_cast<float>(numVoices - 1) / 2.0f) / 
                           (static_cast<float>(numVoices) / 2.0f);
        float delayMs = 5.0f + spreadAmount * 45.0f * std::abs(voiceSpread); // 5-50ms range
        float delaySamples = delayMs * 0.001f * static_cast<float>(currentSampleRate);
        
        // Add LFO modulation
        float lfoMod = generateLFO() * depthAmount * 2.0f; // ±2 samples of modulation
        delaySamples += lfoMod;
        
        delayLines[voice].setDelay(delaySamples);
        
        // Process through delay line
        float voiceGain = mixLevel / static_cast<float>(numVoices);
        
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            float* bufferData = buffer.getWritePointer(channel);
            const float* dryData = processingBuffer.getReadPointer(channel);
            
            for (int i = 0; i < numSamples; ++i)
            {
                float wetSample = delayLines[voice].popSample(channel);
                delayLines[voice].pushSample(channel, dryData[i]);
                
                // Pan voices across stereo field
                float pan = voiceSpread * spreadAmount;
                float leftGain = std::sqrt(0.5f * (1.0f - pan));
                float rightGain = std::sqrt(0.5f * (1.0f + pan));
                
                if (channel == 0)
                    bufferData[i] += wetSample * voiceGain * leftGain;
                else
                    bufferData[i] += wetSample * voiceGain * rightGain;
            }
        }
    }
}

void GhostChoirEffect::reset()
{
    for (auto& delay : delayLines)
    {
        delay.reset();
    }
    lfoPhase = 0.0f;
}

void GhostChoirEffect::setNumVoices(int voices)
{
    numVoices = clamp(voices, 1, 8);
}

void GhostChoirEffect::setSpread(float spread)
{
    spreadAmount = clamp(spread / 100.0f, 0.0f, 1.0f);
}

void GhostChoirEffect::setDepth(float depth)
{
    depthAmount = clamp(depth / 100.0f, 0.0f, 1.0f);
}

void GhostChoirEffect::setMix(float mix)
{
    mixLevel = clamp(mix / 100.0f, 0.0f, 1.0f);
}

void GhostChoirEffect::setPitchShift(float semitones)
{
    pitchShiftSemitones = clamp(semitones, -24.0f, 24.0f);
}

void GhostChoirEffect::setDetune(float cents)
{
    detuneCents = clamp(cents, -50.0f, 50.0f);
}

float GhostChoirEffect::generateLFO()
{
    float lfoValue = std::sin(lfoPhase * TWO_PI);
    lfoPhase += lfoRate / static_cast<float>(currentSampleRate);
    if (lfoPhase >= 1.0f)
        lfoPhase -= 1.0f;
    return lfoValue;
}

//==============================================================================
// ToneShaperEffect Implementation
//==============================================================================

ToneShaperEffect::ToneShaperEffect()
    : enabled(true)
    , lowGain(0.0f)
    , midGain(0.0f)
    , highGain(0.0f)
    , presenceAmount(0.5f)
    , warmthAmount(0.5f)
    , airAmount(0.5f)
    , currentSampleRate(44100.0)
{
}

ToneShaperEffect::~ToneShaperEffect()
{
}

void ToneShaperEffect::prepare(double sampleRate, int maxBlockSize)
{
    currentSampleRate = sampleRate;
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = maxBlockSize;
    spec.numChannels = 2;
    
    lowShelf.prepare(spec);
    midPeak.prepare(spec);
    highShelf.prepare(spec);
    presenceFilter.prepare(spec);
    warmthFilter.prepare(spec);
    airFilter.prepare(spec);
    
    updateFilters();
}

void ToneShaperEffect::process(juce::AudioBuffer<float>& buffer, int numSamples)
{
    if (!enabled)
        return;
    
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    // Apply all filters in sequence
    lowShelf.process(context);
    midPeak.process(context);
    highShelf.process(context);
    presenceFilter.process(context);
    warmthFilter.process(context);
    airFilter.process(context);
}

void ToneShaperEffect::reset()
{
    lowShelf.reset();
    midPeak.reset();
    highShelf.reset();
    presenceFilter.reset();
    warmthFilter.reset();
    airFilter.reset();
}

void ToneShaperEffect::setLowGain(float dB)
{
    lowGain = clamp(dB, -12.0f, 12.0f);
    updateFilters();
}

void ToneShaperEffect::setMidGain(float dB)
{
    midGain = clamp(dB, -12.0f, 12.0f);
    updateFilters();
}

void ToneShaperEffect::setHighGain(float dB)
{
    highGain = clamp(dB, -12.0f, 12.0f);
    updateFilters();
}

void ToneShaperEffect::setPresence(float amount)
{
    presenceAmount = clamp(amount / 100.0f, 0.0f, 1.0f);
    updateFilters();
}

void ToneShaperEffect::setWarmth(float amount)
{
    warmthAmount = clamp(amount / 100.0f, 0.0f, 1.0f);
    updateFilters();
}

void ToneShaperEffect::setAir(float amount)
{
    airAmount = clamp(amount / 100.0f, 0.0f, 1.0f);
    updateFilters();
}

void ToneShaperEffect::updateFilters()
{
    // Low shelf at 200 Hz
    float lowGainLinear = dBToGain(lowGain);
    *lowShelf.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(
        currentSampleRate, 200.0f, 0.7f, lowGainLinear);
    
    // Mid peak at 1000 Hz
    float midGainLinear = dBToGain(midGain);
    *midPeak.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        currentSampleRate, 1000.0f, 1.0f, midGainLinear);
    
    // High shelf at 8000 Hz
    float highGainLinear = dBToGain(highGain);
    *highShelf.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        currentSampleRate, 8000.0f, 0.7f, highGainLinear);
    
    // Presence at 4000 Hz
    float presenceGain = dBToGain((presenceAmount - 0.5f) * 12.0f); // ±6dB range
    *presenceFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        currentSampleRate, 4000.0f, 1.0f, presenceGain);
    
    // Warmth at 250 Hz
    float warmthGain = dBToGain((warmthAmount - 0.5f) * 6.0f); // ±3dB range
    *warmthFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        currentSampleRate, 250.0f, 0.8f, warmthGain);
    
    // Air at 12000 Hz
    float airGain = dBToGain((airAmount - 0.5f) * 8.0f); // ±4dB range
    *airFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        currentSampleRate, 12000.0f, 0.7f, airGain);
}

} // namespace MAEVN
