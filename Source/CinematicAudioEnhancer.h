/**
 * @file CinematicAudioEnhancer.h
 * @brief Cinematic Audio Enhancement Module for Grammy-Quality Production
 * 
 * This module provides comprehensive audio enhancement capabilities focusing on:
 * - Cinematic tone and emotional depth
 * - Viral mass appeal processing
 * - Grammy-nominated quality audio production
 * 
 * Features include vocal processing, multi-FX automation, instrument enhancement,
 * and mastering chain components.
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
 * @brief High-pass filter for vocal clarity
 * 
 * Removes unnecessary low frequencies (below 80 Hz) for vocal tracks
 */
class HighPassFilter
{
public:
    HighPassFilter();
    
    void prepare(double sampleRate, int maxBlockSize);
    void process(juce::AudioBuffer<float>& buffer, int numSamples);
    void reset();
    
    /**
     * @brief Set the cutoff frequency (default 80 Hz)
     */
    void setCutoffFrequency(float frequency);

private:
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, 
                                    juce::dsp::IIR::Coefficients<float>> highPassFilter;
    double currentSampleRate;
    float cutoffFreq;
};

//==============================================================================
/**
 * @brief Presence EQ boost for vocal clarity
 * 
 * Boosts around 3-5 kHz for clarity and presence in vocals
 */
class PresenceEQ
{
public:
    PresenceEQ();
    
    void prepare(double sampleRate, int maxBlockSize);
    void process(juce::AudioBuffer<float>& buffer, int numSamples);
    void reset();
    
    /**
     * @brief Set the presence frequency (default 3-5 kHz range)
     */
    void setFrequency(float frequency);
    
    /**
     * @brief Set the boost amount in dB
     */
    void setGain(float dB);
    
    /**
     * @brief Set the Q factor (bandwidth)
     */
    void setQ(float q);

private:
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, 
                                    juce::dsp::IIR::Coefficients<float>> presenceFilter;
    double currentSampleRate;
    float frequency;
    float gainDB;
    float qFactor;
    
    void updateCoefficients();
};

//==============================================================================
/**
 * @brief Gentle compressor for natural vocal dynamics
 * 
 * Applies gentle compression (2:1 ratio) to even out dynamics 
 * while maintaining natural expressiveness
 */
class GentleCompressor
{
public:
    GentleCompressor();
    
    void prepare(double sampleRate, int maxBlockSize);
    void process(juce::AudioBuffer<float>& buffer, int numSamples);
    void reset();
    
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
 * @brief Large hall reverb for cinematic space
 * 
 * Creates a sense of space with large hall reverb, 
 * with adjustable pre-delay for clarity
 */
class CinematicReverb
{
public:
    CinematicReverb();
    
    void prepare(double sampleRate, int maxBlockSize);
    void process(juce::AudioBuffer<float>& buffer, int numSamples);
    void reset();
    
    void setRoomSize(float size);
    void setDamping(float damping);
    void setWetLevel(float level);
    void setDryLevel(float level);
    void setPreDelay(float ms);
    void setWidth(float width);

private:
    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters reverbParams;
    
    // Pre-delay line
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> preDelayLine{192000};
    float preDelayMs;
    double currentSampleRate;
};

//==============================================================================
/**
 * @brief Subtle delay for depth enhancement
 * 
 * Adds subtle delay (quarter-note by default) to enhance depth 
 * without cluttering the mix
 */
class SubtleDelay
{
public:
    SubtleDelay();
    
    void prepare(double sampleRate, int maxBlockSize);
    void process(juce::AudioBuffer<float>& buffer, int numSamples);
    void reset();
    
    /**
     * @brief Set delay time in milliseconds
     */
    void setDelayTime(float ms);
    
    /**
     * @brief Set delay time based on BPM (quarter note)
     */
    void setDelayTimeFromBPM(double bpm);
    
    void setFeedback(float feedback);
    void setMix(float mix);

private:
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine{192000};
    double currentSampleRate;
    float delayTimeMs;
    float feedbackAmount;
    float mixLevel;
    
    juce::AudioBuffer<float> feedbackBuffer;
};

//==============================================================================
/**
 * @brief Chorus/Flanger modulation effect
 * 
 * Creates lush, expansive sound with modulation effects
 */
class ModulationEffect
{
public:
    ModulationEffect();
    
    void prepare(double sampleRate, int maxBlockSize);
    void process(juce::AudioBuffer<float>& buffer, int numSamples);
    void reset();
    
    void setRate(float hz);
    void setDepth(float depth);
    void setMix(float mix);

private:
    juce::dsp::Chorus<float> chorus;
    double currentSampleRate;
};

//==============================================================================
/**
 * @brief Warm saturation/distortion for richness
 * 
 * Adds warmth and richness with light saturation, 
 * especially for climactic moments
 */
class WarmSaturation
{
public:
    WarmSaturation();
    
    void prepare(double sampleRate, int maxBlockSize);
    void process(juce::AudioBuffer<float>& buffer, int numSamples);
    void reset();
    
    /**
     * @brief Set saturation drive amount (0.0 - 1.0)
     */
    void setDrive(float drive);
    
    /**
     * @brief Set output gain compensation
     */
    void setOutputGain(float dB);

private:
    float driveAmount;
    float outputGainLinear;
    
    // Soft saturation function
    float saturate(float sample);
};

//==============================================================================
/**
 * @brief Multiband compressor for mastering
 * 
 * Controls dynamics across different frequency ranges
 */
class MultibandCompressor
{
public:
    MultibandCompressor();
    
    void prepare(double sampleRate, int maxBlockSize);
    void process(juce::AudioBuffer<float>& buffer, int numSamples);
    void reset();
    
    void setLowBandThreshold(float dB);
    void setMidBandThreshold(float dB);
    void setHighBandThreshold(float dB);
    
    void setLowBandRatio(float ratio);
    void setMidBandRatio(float ratio);
    void setHighBandRatio(float ratio);

private:
    // Crossover filters
    juce::dsp::LinkwitzRileyFilter<float> lowCrossover;
    juce::dsp::LinkwitzRileyFilter<float> highCrossover;
    
    // Band compressors
    juce::dsp::Compressor<float> lowBandCompressor;
    juce::dsp::Compressor<float> midBandCompressor;
    juce::dsp::Compressor<float> highBandCompressor;
    
    // Band buffers
    juce::AudioBuffer<float> lowBandBuffer;
    juce::AudioBuffer<float> midBandBuffer;
    juce::AudioBuffer<float> highBandBuffer;
    
    double currentSampleRate;
    static constexpr float LOW_CROSSOVER_FREQ = 200.0f;
    static constexpr float HIGH_CROSSOVER_FREQ = 4000.0f;
};

//==============================================================================
/**
 * @brief Stereo imaging for width control
 * 
 * Widens the stereo field while keeping low frequencies centered
 */
class StereoImager
{
public:
    StereoImager();
    
    void prepare(double sampleRate, int maxBlockSize);
    void process(juce::AudioBuffer<float>& buffer, int numSamples);
    void reset();
    
    /**
     * @brief Set stereo width (0.0 = mono, 1.0 = normal, 2.0 = wide)
     */
    void setWidth(float width);
    
    /**
     * @brief Set frequency below which to keep mono
     */
    void setMonoFrequency(float frequency);

private:
    float stereoWidth;
    float monoFrequency;
    
    // Low-pass filter for bass mono processing
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, 
                                    juce::dsp::IIR::Coefficients<float>> lowPassFilter;
    double currentSampleRate;
};

//==============================================================================
/**
 * @brief Loudness normalizer targeting streaming platforms
 * 
 * Targets -14 LUFS for streaming platform compatibility
 */
class LoudnessNormalizer
{
public:
    LoudnessNormalizer();
    
    void prepare(double sampleRate, int maxBlockSize);
    void process(juce::AudioBuffer<float>& buffer, int numSamples);
    void reset();
    
    /**
     * @brief Set target loudness in LUFS (default -14)
     */
    void setTargetLUFS(float lufs);
    
    /**
     * @brief Get current measured loudness
     */
    float getCurrentLUFS() const;

private:
    float targetLUFS;
    float currentLUFS;
    float currentGain;
    
    // RMS measurement
    float rmsSum;
    int rmsSampleCount;
    static constexpr int RMS_WINDOW_SIZE = 44100; // 1 second window
    
    double currentSampleRate;
    
    void updateGain();
};

//==============================================================================
/**
 * @brief Final limiter for mastering (-0.1 dB ceiling)
 * 
 * Ensures the track peaks at -0.1 dB without clipping
 */
class FinalLimiter
{
public:
    FinalLimiter();
    
    void prepare(double sampleRate, int maxBlockSize);
    void process(juce::AudioBuffer<float>& buffer, int numSamples);
    void reset();
    
    void setCeiling(float dB);
    void setRelease(float ms);

private:
    juce::dsp::Limiter<float> limiter;
};

//==============================================================================
/**
 * @brief Main Cinematic Audio Enhancer Module
 * 
 * Combines all processing components for Grammy-quality audio production
 * with focus on cinematic tone, emotional depth, and viral appeal
 */
class CinematicAudioEnhancer
{
public:
    CinematicAudioEnhancer();
    ~CinematicAudioEnhancer();
    
    /**
     * @brief Prepare for playback
     */
    void prepare(double sampleRate, int maxBlockSize);
    
    /**
     * @brief Process audio buffer
     */
    void process(juce::AudioBuffer<float>& buffer, int numSamples);
    
    /**
     * @brief Reset all internal states
     */
    void reset();
    
    //==========================================================================
    // Vocal Processing
    //==========================================================================
    
    void setHighPassEnabled(bool enabled);
    void setHighPassCutoff(float frequency);
    
    void setPresenceEQEnabled(bool enabled);
    void setPresenceFrequency(float frequency);
    void setPresenceGain(float dB);
    
    void setVocalCompressorEnabled(bool enabled);
    void setVocalCompressorThreshold(float dB);
    void setVocalCompressorRatio(float ratio);
    
    void setCinematicReverbEnabled(bool enabled);
    void setCinematicReverbSize(float size);
    void setCinematicReverbMix(float mix);
    void setCinematicReverbPreDelay(float ms);
    
    void setSubtleDelayEnabled(bool enabled);
    void setSubtleDelayTime(float ms);
    void setSubtleDelayMix(float mix);
    
    //==========================================================================
    // Multi-FX Automation
    //==========================================================================
    
    void setModulationEnabled(bool enabled);
    void setModulationRate(float hz);
    void setModulationDepth(float depth);
    void setModulationMix(float mix);
    
    void setSaturationEnabled(bool enabled);
    void setSaturationDrive(float drive);
    
    //==========================================================================
    // Mastering Chain
    //==========================================================================
    
    void setMultibandCompressorEnabled(bool enabled);
    
    void setStereoImagerEnabled(bool enabled);
    void setStereoWidth(float width);
    
    void setLoudnessNormalizerEnabled(bool enabled);
    void setTargetLUFS(float lufs);
    
    void setFinalLimiterEnabled(bool enabled);
    void setLimiterCeiling(float dB);
    
    //==========================================================================
    // Preset Management
    //==========================================================================
    
    /**
     * @brief Apply cinematic vocal preset
     * Grammy-quality vocal processing for emotional depth
     */
    void applyCinematicVocalPreset();
    
    /**
     * @brief Apply cinematic mastering preset
     * Professional mastering chain for final polish
     */
    void applyCinematicMasteringPreset();
    
    /**
     * @brief Apply viral appeal preset
     * Processing optimized for viral mass appeal
     */
    void applyViralAppealPreset();
    
    /**
     * @brief Get current loudness measurement
     */
    float getCurrentLUFS() const;

private:
    double currentSampleRate;
    int currentMaxBlockSize;
    
    //==========================================================================
    // Vocal Processing Components
    //==========================================================================
    HighPassFilter highPassFilter;
    PresenceEQ presenceEQ;
    GentleCompressor vocalCompressor;
    CinematicReverb cinematicReverb;
    SubtleDelay subtleDelay;
    
    //==========================================================================
    // Multi-FX Components
    //==========================================================================
    ModulationEffect modulationEffect;
    WarmSaturation warmSaturation;
    
    //==========================================================================
    // Mastering Chain Components
    //==========================================================================
    MultibandCompressor multibandCompressor;
    StereoImager stereoImager;
    LoudnessNormalizer loudnessNormalizer;
    FinalLimiter finalLimiter;
    
    //==========================================================================
    // Enable Flags
    //==========================================================================
    bool highPassEnabled;
    bool presenceEQEnabled;
    bool vocalCompressorEnabled;
    bool cinematicReverbEnabled;
    bool subtleDelayEnabled;
    bool modulationEnabled;
    bool saturationEnabled;
    bool multibandCompressorEnabled;
    bool stereoImagerEnabled;
    bool loudnessNormalizerEnabled;
    bool finalLimiterEnabled;
    
    juce::CriticalSection processLock;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CinematicAudioEnhancer)
};

} // namespace MAEVN
