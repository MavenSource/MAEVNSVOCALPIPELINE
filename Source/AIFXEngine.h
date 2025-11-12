/**
 * @file AIFXEngine.h
 * @brief Hybrid DSP and AI effects processing engine
 * 
 * This module provides a flexible effects chain that can combine traditional
 * DSP effects with AI-powered processing using ONNX models.
 */

#pragma once

#include <JuceHeader.h>
#include <memory>
#include <vector>
#include "Utilities.h"
#include "OnnxEngine.h"

namespace MAEVN
{

//==============================================================================
/**
 * @brief Base class for all effects
 */
class Effect
{
public:
    virtual ~Effect() = default;
    
    /**
     * @brief Process audio buffer
     */
    virtual void process(juce::AudioBuffer<float>& buffer, int numSamples) = 0;
    
    /**
     * @brief Prepare for playback
     */
    virtual void prepare(double sampleRate, int maxBlockSize) = 0;
    
    /**
     * @brief Reset internal state
     */
    virtual void reset() = 0;
    
    /**
     * @brief Get effect name
     */
    virtual juce::String getName() const = 0;
};

//==============================================================================
/**
 * @brief DSP Compressor effect
 */
class CompressorEffect : public Effect
{
public:
    CompressorEffect();
    
    void process(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void prepare(double sampleRate, int maxBlockSize) override;
    void reset() override;
    juce::String getName() const override { return "Compressor"; }
    
    void setThreshold(float dB);
    void setRatio(float ratio);
    void setAttack(float ms);
    void setRelease(float ms);
    
private:
    juce::dsp::Compressor<float> compressor;
    double currentSampleRate;
};

//==============================================================================
/**
 * @brief DSP EQ effect (3-band parametric)
 */
class EQEffect : public Effect
{
public:
    EQEffect();
    
    void process(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void prepare(double sampleRate, int maxBlockSize) override;
    void reset() override;
    juce::String getName() const override { return "EQ"; }
    
    void setLowGain(float dB);
    void setMidGain(float dB);
    void setHighGain(float dB);
    
private:
    juce::dsp::ProcessorChain<
        juce::dsp::IIR::Filter<float>,  // Low shelf
        juce::dsp::IIR::Filter<float>,  // Mid peak
        juce::dsp::IIR::Filter<float>   // High shelf
    > eqChain;
    
    double currentSampleRate;
};

//==============================================================================
/**
 * @brief DSP Reverb effect
 */
class ReverbEffect : public Effect
{
public:
    ReverbEffect();
    
    void process(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void prepare(double sampleRate, int maxBlockSize) override;
    void reset() override;
    juce::String getName() const override { return "Reverb"; }
    
    void setRoomSize(float size);
    void setDamping(float damping);
    void setWetLevel(float level);
    void setDryLevel(float level);
    
private:
    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters reverbParams;
};

//==============================================================================
/**
 * @brief DSP Limiter effect
 */
class LimiterEffect : public Effect
{
public:
    LimiterEffect();
    
    void process(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void prepare(double sampleRate, int maxBlockSize) override;
    void reset() override;
    juce::String getName() const override { return "Limiter"; }
    
    void setThreshold(float dB);
    void setRelease(float ms);
    
private:
    juce::dsp::Limiter<float> limiter;
};

//==============================================================================
/**
 * @brief AI-powered effect using ONNX model
 */
class AIEffect : public Effect
{
public:
    AIEffect(OnnxEngine* engine, const juce::String& modelRole);
    
    void process(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void prepare(double sampleRate, int maxBlockSize) override;
    void reset() override;
    juce::String getName() const override { return "AI: " + modelRole; }
    
private:
    OnnxEngine* onnxEngine;
    juce::String modelRole;
    double currentSampleRate;
    
    // Buffer for AI processing
    std::vector<float> inputBuffer;
    std::vector<float> outputBuffer;
};

//==============================================================================
/**
 * @brief Main AI FX Engine - manages effects chains
 */
class AIFXEngine
{
public:
    AIFXEngine(OnnxEngine* engine);
    ~AIFXEngine();
    
    /**
     * @brief Process audio with current FX chain
     */
    void process(juce::AudioBuffer<float>& buffer, int numSamples, int trackIndex);
    
    /**
     * @brief Prepare for playback
     */
    void prepare(double sampleRate, int maxBlockSize);
    
    /**
     * @brief Reset all effects
     */
    void reset();
    
    /**
     * @brief Set FX mode for a track
     */
    void setFXMode(int trackIndex, FXMode mode);
    
    /**
     * @brief Get current FX mode for a track
     */
    FXMode getFXMode(int trackIndex) const;
    
    /**
     * @brief Add DSP effect to track chain
     */
    void addDSPEffect(int trackIndex, std::unique_ptr<Effect> effect);
    
    /**
     * @brief Add AI effect to track chain
     */
    void addAIEffect(int trackIndex, const juce::String& modelRole);
    
    /**
     * @brief Clear all effects from track
     */
    void clearEffects(int trackIndex);
    
    /**
     * @brief Get number of tracks
     */
    int getNumTracks() const { return NUM_TRACKS; }
    
    /**
     * @brief Set parameter for track effect
     */
    void setEffectParameter(int trackIndex, int effectIndex, const juce::String& paramName, float value);
    
private:
    static constexpr int NUM_TRACKS = 6; // Vocal, 808, HiHat, Snare, Piano, Synth
    
    struct TrackFX
    {
        FXMode mode = FXMode::Off;
        std::vector<std::unique_ptr<Effect>> dspEffects;
        std::vector<std::unique_ptr<Effect>> aiEffects;
    };
    
    std::array<TrackFX, NUM_TRACKS> trackFX;
    OnnxEngine* onnxEngine;
    
    double currentSampleRate;
    int currentMaxBlockSize;
    
    juce::CriticalSection fxLock;
    
    /**
     * @brief Process with DSP effects only
     */
    void processDSP(juce::AudioBuffer<float>& buffer, int numSamples, int trackIndex);
    
    /**
     * @brief Process with AI effects only
     */
    void processAI(juce::AudioBuffer<float>& buffer, int numSamples, int trackIndex);
    
    /**
     * @brief Process with hybrid DSP+AI chain
     */
    void processHybrid(juce::AudioBuffer<float>& buffer, int numSamples, int trackIndex);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AIFXEngine)
};

} // namespace MAEVN
