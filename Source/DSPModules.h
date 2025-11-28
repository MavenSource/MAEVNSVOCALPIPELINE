/**
 * @file DSPModules.h
 * @brief DSP Modules for LegendaryProducerFXSuiteUltimate
 * 
 * This file contains all the DSP processing modules for the combined
 * LegendaryProducerFXSuite, PTH Vocal Clone, and EpicSpaceReverb plugin.
 */

#pragma once

#include <JuceHeader.h>
#include <memory>
#include <vector>
#include <cmath>
#include <array>
#include "Utilities.h"

namespace dspmodules
{

//==============================================================================
/**
 * @brief Multiband Compressor with Low, Mid, High frequency bands
 * 
 * Features separate threshold, ratio, attack, and release controls for each band.
 */
class MultibandCompressor
{
public:
    struct BandSettings
    {
        float threshold = -20.0f;
        float ratio = 4.0f;
        float attack = 10.0f;
        float release = 100.0f;
    };

    MultibandCompressor()
        : currentSampleRate(44100.0)
    {
        // Default settings for each band
        lowBandSettings = { -18.0f, 3.0f, 20.0f, 150.0f };
        midBandSettings = { -15.0f, 2.5f, 10.0f, 100.0f };
        highBandSettings = { -12.0f, 2.0f, 5.0f, 80.0f };
    }

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        currentSampleRate = spec.sampleRate;
        
        lowCrossover.prepare(spec);
        highCrossover.prepare(spec);
        
        lowCrossover.setCutoffFrequency(lowCrossoverFreq);
        highCrossover.setCutoffFrequency(highCrossoverFreq);
        
        lowCrossover.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
        highCrossover.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
        
        lowBandCompressor.prepare(spec);
        midBandCompressor.prepare(spec);
        highBandCompressor.prepare(spec);
        
        applyBandSettings();
        
        lowBandBuffer.setSize(static_cast<int>(spec.numChannels), static_cast<int>(spec.maximumBlockSize));
        midBandBuffer.setSize(static_cast<int>(spec.numChannels), static_cast<int>(spec.maximumBlockSize));
        highBandBuffer.setSize(static_cast<int>(spec.numChannels), static_cast<int>(spec.maximumBlockSize));
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        int numSamples = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();
        
        // Copy input to band buffers
        for (int ch = 0; ch < numChannels; ++ch)
        {
            lowBandBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);
            midBandBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);
            highBandBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);
        }
        
        // Apply crossover filters
        juce::dsp::AudioBlock<float> lowBlock(lowBandBuffer);
        juce::dsp::AudioBlock<float> highBlock(highBandBuffer);
        
        juce::dsp::ProcessContextReplacing<float> lowContext(lowBlock);
        juce::dsp::ProcessContextReplacing<float> highContext(highBlock);
        
        lowCrossover.process(lowContext);
        highCrossover.process(highContext);
        
        // Mid band = original - low - high
        // midBandBuffer still contains the original input at this point
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* midData = midBandBuffer.getWritePointer(ch);
            const auto* originalData = buffer.getReadPointer(ch);
            const auto* lowData = lowBandBuffer.getReadPointer(ch);
            const auto* highData = highBandBuffer.getReadPointer(ch);
            
            for (int i = 0; i < numSamples; ++i)
            {
                midData[i] = originalData[i] - lowData[i] - highData[i];
            }
        }
        
        // Apply compression to each band
        juce::dsp::AudioBlock<float> lowCompBlock(lowBandBuffer);
        juce::dsp::AudioBlock<float> midCompBlock(midBandBuffer);
        juce::dsp::AudioBlock<float> highCompBlock(highBandBuffer);
        
        juce::dsp::ProcessContextReplacing<float> lowCompContext(lowCompBlock);
        juce::dsp::ProcessContextReplacing<float> midCompContext(midCompBlock);
        juce::dsp::ProcessContextReplacing<float> highCompContext(highCompBlock);
        
        lowBandCompressor.process(lowCompContext);
        midBandCompressor.process(midCompContext);
        highBandCompressor.process(highCompContext);
        
        // Sum bands back together
        buffer.clear();
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* outputData = buffer.getWritePointer(ch);
            const auto* lowData = lowBandBuffer.getReadPointer(ch);
            const auto* midData = midBandBuffer.getReadPointer(ch);
            const auto* highData = highBandBuffer.getReadPointer(ch);
            
            for (int i = 0; i < numSamples; ++i)
            {
                outputData[i] = lowData[i] + midData[i] + highData[i];
            }
        }
    }

    void reset()
    {
        lowCrossover.reset();
        highCrossover.reset();
        lowBandCompressor.reset();
        midBandCompressor.reset();
        highBandCompressor.reset();
    }

    // Low band controls
    void setLowThreshold(float dB) { lowBandSettings.threshold = dB; applyBandSettings(); }
    void setLowRatio(float ratio) { lowBandSettings.ratio = ratio; applyBandSettings(); }
    void setLowAttack(float ms) { lowBandSettings.attack = ms; applyBandSettings(); }
    void setLowRelease(float ms) { lowBandSettings.release = ms; applyBandSettings(); }

    // Mid band controls
    void setMidThreshold(float dB) { midBandSettings.threshold = dB; applyBandSettings(); }
    void setMidRatio(float ratio) { midBandSettings.ratio = ratio; applyBandSettings(); }
    void setMidAttack(float ms) { midBandSettings.attack = ms; applyBandSettings(); }
    void setMidRelease(float ms) { midBandSettings.release = ms; applyBandSettings(); }

    // High band controls
    void setHighThreshold(float dB) { highBandSettings.threshold = dB; applyBandSettings(); }
    void setHighRatio(float ratio) { highBandSettings.ratio = ratio; applyBandSettings(); }
    void setHighAttack(float ms) { highBandSettings.attack = ms; applyBandSettings(); }
    void setHighRelease(float ms) { highBandSettings.release = ms; applyBandSettings(); }

    // Crossover frequency controls
    void setLowCrossoverFreq(float freq) { lowCrossoverFreq = freq; lowCrossover.setCutoffFrequency(freq); }
    void setHighCrossoverFreq(float freq) { highCrossoverFreq = freq; highCrossover.setCutoffFrequency(freq); }

private:
    void applyBandSettings()
    {
        lowBandCompressor.setThreshold(lowBandSettings.threshold);
        lowBandCompressor.setRatio(lowBandSettings.ratio);
        lowBandCompressor.setAttack(lowBandSettings.attack);
        lowBandCompressor.setRelease(lowBandSettings.release);
        
        midBandCompressor.setThreshold(midBandSettings.threshold);
        midBandCompressor.setRatio(midBandSettings.ratio);
        midBandCompressor.setAttack(midBandSettings.attack);
        midBandCompressor.setRelease(midBandSettings.release);
        
        highBandCompressor.setThreshold(highBandSettings.threshold);
        highBandCompressor.setRatio(highBandSettings.ratio);
        highBandCompressor.setAttack(highBandSettings.attack);
        highBandCompressor.setRelease(highBandSettings.release);
    }

    double currentSampleRate;
    float lowCrossoverFreq = 200.0f;
    float highCrossoverFreq = 4000.0f;
    
    BandSettings lowBandSettings;
    BandSettings midBandSettings;
    BandSettings highBandSettings;
    
    juce::dsp::LinkwitzRileyFilter<float> lowCrossover;
    juce::dsp::LinkwitzRileyFilter<float> highCrossover;
    
    juce::dsp::Compressor<float> lowBandCompressor;
    juce::dsp::Compressor<float> midBandCompressor;
    juce::dsp::Compressor<float> highBandCompressor;
    
    juce::AudioBuffer<float> lowBandBuffer;
    juce::AudioBuffer<float> midBandBuffer;
    juce::AudioBuffer<float> highBandBuffer;
};

//==============================================================================
/**
 * @brief Transient Shaper for controlling attack and sustain
 */
class TransientShaper
{
public:
    // Constants for transient shaping calculations
    static constexpr float ATTACK_SCALING_FACTOR = 10.0f;
    static constexpr float SUSTAIN_SCALING_FACTOR = 0.5f;
    static constexpr float MIN_GAIN = 0.1f;
    static constexpr float MAX_GAIN = 4.0f;
    
    TransientShaper()
        : attackAmount(0.0f)
        , sustainAmount(0.0f)
        , currentSampleRate(44100.0)
        , envelopeFollower(0.0f)
        , previousEnvelope(0.0f)
    {
    }

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        currentSampleRate = spec.sampleRate;
        
        // Calculate envelope follower coefficients
        attackCoeff = std::exp(-1.0f / static_cast<float>(spec.sampleRate * 0.001f)); // 1ms attack
        releaseCoeff = std::exp(-1.0f / static_cast<float>(spec.sampleRate * 0.050f)); // 50ms release
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        int numSamples = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();
        
        for (int i = 0; i < numSamples; ++i)
        {
            // Calculate envelope from all channels
            float inputLevel = 0.0f;
            for (int ch = 0; ch < numChannels; ++ch)
            {
                inputLevel += std::abs(buffer.getSample(ch, i));
            }
            inputLevel /= static_cast<float>(numChannels);
            
            // Envelope follower
            float coeff = (inputLevel > envelopeFollower) ? attackCoeff : releaseCoeff;
            envelopeFollower = envelopeFollower * coeff + inputLevel * (1.0f - coeff);
            
            // Detect transients (rate of change of envelope)
            float envelopeDelta = envelopeFollower - previousEnvelope;
            previousEnvelope = envelopeFollower;
            
            // Calculate gain modification
            float transientGain = 1.0f;
            
            if (envelopeDelta > 0.0f)
            {
                // Attack phase - apply attack shaping
                transientGain += attackAmount * envelopeDelta * ATTACK_SCALING_FACTOR;
            }
            else
            {
                // Sustain phase - apply sustain shaping
                transientGain += sustainAmount * SUSTAIN_SCALING_FACTOR;
            }
            
            transientGain = juce::jlimit(MIN_GAIN, MAX_GAIN, transientGain);
            
            // Apply gain to all channels
            for (int ch = 0; ch < numChannels; ++ch)
            {
                float* sample = buffer.getWritePointer(ch);
                sample[i] *= transientGain;
            }
        }
    }

    void reset()
    {
        envelopeFollower = 0.0f;
        previousEnvelope = 0.0f;
    }

    void setAttack(float amount) { attackAmount = juce::jlimit(-1.0f, 1.0f, amount); }
    void setSustain(float amount) { sustainAmount = juce::jlimit(-1.0f, 1.0f, amount); }

private:
    float attackAmount;
    float sustainAmount;
    double currentSampleRate;
    float envelopeFollower;
    float previousEnvelope;
    float attackCoeff;
    float releaseCoeff;
};

//==============================================================================
/**
 * @brief De-Esser for reducing sibilance
 */
class DeEsser
{
public:
    DeEsser()
        : frequency(6000.0f)
        , threshold(-20.0f)
        , ratio(4.0f)
        , currentSampleRate(44100.0)
    {
    }

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        currentSampleRate = spec.sampleRate;
        
        // Prepare high-pass filter for sibilance detection
        sibilanceFilter.prepare(spec);
        updateFilterCoefficients();
        
        // Prepare compressor for gain reduction
        compressor.prepare(spec);
        compressor.setThreshold(threshold);
        compressor.setRatio(ratio);
        compressor.setAttack(0.5f);
        compressor.setRelease(20.0f);
        
        sideChainBuffer.setSize(static_cast<int>(spec.numChannels), static_cast<int>(spec.maximumBlockSize));
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        int numSamples = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();
        
        // Copy to sidechain buffer and filter
        for (int ch = 0; ch < numChannels; ++ch)
        {
            sideChainBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);
        }
        
        juce::dsp::AudioBlock<float> sideChainBlock(sideChainBuffer);
        juce::dsp::ProcessContextReplacing<float> filterContext(sideChainBlock);
        sibilanceFilter.process(filterContext);
        
        // Use filtered signal to control compression on original
        for (int i = 0; i < numSamples; ++i)
        {
            // Measure sibilance level
            float sibilanceLevel = 0.0f;
            for (int ch = 0; ch < numChannels; ++ch)
            {
                sibilanceLevel += std::abs(sideChainBuffer.getSample(ch, i));
            }
            sibilanceLevel /= static_cast<float>(numChannels);
            
            // Calculate gain reduction based on sibilance
            float thresholdLinear = MAEVN::dBToGain(threshold);
            float gainReduction = 1.0f;
            
            if (sibilanceLevel > thresholdLinear)
            {
                float overThreshold = sibilanceLevel / thresholdLinear;
                float targetGain = 1.0f / std::pow(overThreshold, 1.0f - 1.0f / ratio);
                gainReduction = targetGain;
            }
            
            // Apply gain reduction to original signal
            for (int ch = 0; ch < numChannels; ++ch)
            {
                float* sample = buffer.getWritePointer(ch);
                sample[i] *= gainReduction;
            }
        }
    }

    void reset()
    {
        sibilanceFilter.reset();
        compressor.reset();
    }

    void setFrequency(float freq)
    {
        frequency = juce::jlimit(2000.0f, 10000.0f, freq);
        updateFilterCoefficients();
    }
    
    void setThreshold(float dB) { threshold = juce::jlimit(-60.0f, 0.0f, dB); compressor.setThreshold(threshold); }
    void setRatio(float r) { ratio = juce::jlimit(1.0f, 20.0f, r); compressor.setRatio(ratio); }
    void setAttack(float ms) { compressor.setAttack(ms); }
    void setRelease(float ms) { compressor.setRelease(ms); }

private:
    void updateFilterCoefficients()
    {
        *sibilanceFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(
            currentSampleRate, frequency);
    }

    float frequency;
    float threshold;
    float ratio;
    double currentSampleRate;
    
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, 
                                    juce::dsp::IIR::Coefficients<float>> sibilanceFilter;
    juce::dsp::Compressor<float> compressor;
    juce::AudioBuffer<float> sideChainBuffer;
};

//==============================================================================
/**
 * @brief Saturation effect with Drive and Tone controls
 */
class Saturation
{
public:
    // Drive scaling factor for saturation intensity
    static constexpr float DRIVE_SCALING_FACTOR = 10.0f;
    
    Saturation()
        : drive(0.5f)
        , tone(0.5f)
        , outputGain(1.0f)
        , currentSampleRate(44100.0)
    {
    }

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        currentSampleRate = spec.sampleRate;
        
        // Prepare tone filter
        toneFilter.prepare(spec);
        updateToneFilter();
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        int numSamples = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();
        
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* data = buffer.getWritePointer(ch);
            
            for (int i = 0; i < numSamples; ++i)
            {
                // Apply drive
                float input = data[i] * (1.0f + drive * DRIVE_SCALING_FACTOR);
                
                // Soft clipping saturation (tanh waveshaper)
                float saturated = std::tanh(input);
                
                // Apply output gain compensation
                data[i] = saturated * outputGain;
            }
        }
        
        // Apply tone filter
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        toneFilter.process(context);
    }

    void reset()
    {
        toneFilter.reset();
    }

    void setDrive(float d) { drive = juce::jlimit(0.0f, 1.0f, d); }
    void setTone(float t) { tone = juce::jlimit(0.0f, 1.0f, t); updateToneFilter(); }
    void setOutput(float dB) { outputGain = MAEVN::dBToGain(juce::jlimit(-24.0f, 12.0f, dB)); }

private:
    void updateToneFilter()
    {
        // Tone control: 0 = darker (low-pass), 0.5 = neutral, 1 = brighter (high-shelf boost)
        float filterFreq = 1000.0f + tone * 4000.0f;
        float gainLinear = 0.5f + tone; // 0.5 to 1.5
        
        *toneFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
            currentSampleRate, filterFreq, 0.7f, gainLinear);
    }

    float drive;
    float tone;
    float outputGain;
    double currentSampleRate;
    
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, 
                                    juce::dsp::IIR::Coefficients<float>> toneFilter;
};

//==============================================================================
/**
 * @brief Stereo Widener effect
 */
class StereoWidener
{
public:
    StereoWidener()
        : width(1.0f)
        , frequency(200.0f)
        , outputGain(1.0f)
        , currentSampleRate(44100.0)
    {
    }

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        currentSampleRate = spec.sampleRate;
        
        // Prepare low-pass filter for bass mono processing
        bassFilter.prepare(spec);
        updateFilterCoefficients();
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        if (buffer.getNumChannels() < 2)
            return;
        
        int numSamples = buffer.getNumSamples();
        
        auto* leftChannel = buffer.getWritePointer(0);
        auto* rightChannel = buffer.getWritePointer(1);
        
        for (int i = 0; i < numSamples; ++i)
        {
            float left = leftChannel[i];
            float right = rightChannel[i];
            
            // Calculate mid and side
            float mid = (left + right) * 0.5f;
            float side = (left - right) * 0.5f;
            
            // Apply width to side signal
            side *= width;
            
            // Reconstruct left and right
            leftChannel[i] = (mid + side) * outputGain;
            rightChannel[i] = (mid - side) * outputGain;
        }
    }

    void reset()
    {
        bassFilter.reset();
    }

    void setWidth(float w) { width = juce::jlimit(0.0f, 2.0f, w); }
    void setFrequency(float freq) { frequency = juce::jlimit(50.0f, 500.0f, freq); updateFilterCoefficients(); }
    void setOutput(float dB) { outputGain = MAEVN::dBToGain(juce::jlimit(-24.0f, 12.0f, dB)); }

private:
    void updateFilterCoefficients()
    {
        *bassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(
            currentSampleRate, frequency);
    }

    float width;
    float frequency;
    float outputGain;
    double currentSampleRate;
    
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, 
                                    juce::dsp::IIR::Coefficients<float>> bassFilter;
};

//==============================================================================
/**
 * @brief Limiter effect with Threshold and Ceiling controls
 */
class Limiter
{
public:
    Limiter()
        : threshold(-1.0f)
        , ceiling(-0.1f)
        , attack(0.5f)
        , release(50.0f)
    {
    }

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        limiter.prepare(spec);
        limiter.setThreshold(threshold);
        limiter.setRelease(release);
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        limiter.process(context);
        
        // Apply ceiling
        float ceilingLinear = MAEVN::dBToGain(ceiling);
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            float* data = buffer.getWritePointer(ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                if (data[i] > ceilingLinear)
                    data[i] = ceilingLinear;
                else if (data[i] < -ceilingLinear)
                    data[i] = -ceilingLinear;
            }
        }
    }

    void reset()
    {
        limiter.reset();
    }

    void setThreshold(float dB) { threshold = juce::jlimit(-24.0f, 0.0f, dB); limiter.setThreshold(threshold); }
    void setCeiling(float dB) { ceiling = juce::jlimit(-12.0f, 0.0f, dB); }
    void setAttack(float ms) { attack = juce::jlimit(0.1f, 50.0f, ms); }
    void setRelease(float ms) { release = juce::jlimit(1.0f, 500.0f, ms); limiter.setRelease(release); }

private:
    float threshold;
    float ceiling;
    float attack;
    float release;
    
    juce::dsp::Limiter<float> limiter;
};

//==============================================================================
/**
 * @brief PTH Vocal Clone module for Pitch, Timbre, and Harmony processing
 */
class PTHVocalClone
{
public:
    // Constants for pitch drift calculation
    static constexpr float DRIFT_FREQUENCY_FACTOR = 0.001f;
    static constexpr float DRIFT_AMPLITUDE_FACTOR = 0.01f;
    
    PTHVocalClone()
        : pitchCorrection(0.0f)  // semitones (-12 to +12)
        , correctionSpeed(50.0f) // ms (10 to 100)
        , pitchDrift(0.1f)       // amount of natural variation
        , spectralShaping(0.5f)  // timbre control
        , formantShift(0.0f)     // semitones
        , brightness(0.5f)       // 0-1
        , harmonyEnabled(false)
        , humanize(0.3f)         // random variation amount
        , currentSampleRate(44100.0)
    {
        for (int i = 0; i < 4; ++i)
        {
            harmonyVoices[i] = 0.0f;
            harmonyLevels[i] = 0.5f;
        }
    }

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        currentSampleRate = spec.sampleRate;
        
        // Prepare brightness filter
        brightnessFilter.prepare(spec);
        updateBrightnessFilter();
        
        // Prepare formant filter
        formantFilter.prepare(spec);
        updateFormantFilter();
        
        // Initialize delay lines for pitch shifting (simple approach)
        for (auto& delay : pitchDelays)
        {
            delay.prepare(spec);
            delay.setMaximumDelayInSamples(static_cast<int>(spec.sampleRate * 0.1)); // 100ms max
        }
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        int numSamples = buffer.getNumSamples();
        
        // Apply brightness filter (timbre)
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        brightnessFilter.process(context);
        
        // Apply formant filter for timbre shaping
        formantFilter.process(context);
        
        // Note: Full pitch correction and harmony generation would require 
        // FFT-based processing and complex algorithms. This is a simplified
        // implementation that demonstrates the structure.
        
        // Apply spectral shaping (simplified as EQ adjustment)
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            float* data = buffer.getWritePointer(ch);
            for (int i = 0; i < numSamples; ++i)
            {
                // Add subtle pitch drift for natural sound
                float driftAmount = (std::sin(static_cast<float>(i) * DRIFT_FREQUENCY_FACTOR) * pitchDrift * DRIFT_AMPLITUDE_FACTOR);
                data[i] *= (1.0f + driftAmount);
            }
        }
    }

    void reset()
    {
        brightnessFilter.reset();
        formantFilter.reset();
        for (auto& delay : pitchDelays)
        {
            delay.reset();
        }
    }

    // Pitch controls
    void setPitchCorrection(float semitones) { pitchCorrection = juce::jlimit(-12.0f, 12.0f, semitones); }
    void setCorrectionSpeed(float ms) { correctionSpeed = juce::jlimit(10.0f, 100.0f, ms); }
    void setPitchDrift(float amount) { pitchDrift = juce::jlimit(0.0f, 1.0f, amount); }
    
    // Timbre controls
    void setSpectralShaping(float amount) { spectralShaping = juce::jlimit(0.0f, 1.0f, amount); }
    void setFormantShift(float semitones) { formantShift = juce::jlimit(-12.0f, 12.0f, semitones); updateFormantFilter(); }
    void setBrightness(float b) { brightness = juce::jlimit(0.0f, 1.0f, b); updateBrightnessFilter(); }
    
    // Harmony controls
    void setHarmonyEnabled(bool enabled) { harmonyEnabled = enabled; }
    void setHarmonyVoice(int voiceIndex, float semitones)
    {
        if (voiceIndex >= 0 && voiceIndex < 4)
            harmonyVoices[voiceIndex] = juce::jlimit(-24.0f, 24.0f, semitones);
    }
    void setHarmonyLevel(int voiceIndex, float level)
    {
        if (voiceIndex >= 0 && voiceIndex < 4)
            harmonyLevels[voiceIndex] = juce::jlimit(0.0f, 1.0f, level);
    }
    void setHumanize(float amount) { humanize = juce::jlimit(0.0f, 1.0f, amount); }

    // Harmony mode presets
    enum class HarmonyMode { Thirds, Fifths, Sixths, Custom };
    void setHarmonyMode(HarmonyMode mode)
    {
        switch (mode)
        {
            case HarmonyMode::Thirds:
                harmonyVoices[0] = 4.0f;   // Major third
                harmonyVoices[1] = -3.0f;  // Minor third below
                break;
            case HarmonyMode::Fifths:
                harmonyVoices[0] = 7.0f;   // Perfect fifth
                harmonyVoices[1] = -5.0f;  // Perfect fourth below
                break;
            case HarmonyMode::Sixths:
                harmonyVoices[0] = 9.0f;   // Major sixth
                harmonyVoices[1] = -4.0f;  // Major third below
                break;
            case HarmonyMode::Custom:
                // Keep current custom settings
                break;
        }
    }

private:
    void updateBrightnessFilter()
    {
        float freq = 2000.0f + brightness * 6000.0f;
        float gain = 0.7f + brightness * 0.6f;
        *brightnessFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
            currentSampleRate, freq, 0.7f, gain);
    }
    
    void updateFormantFilter()
    {
        // Simplified formant shifting using a peak filter
        float freq = 1000.0f * std::pow(2.0f, formantShift / 12.0f);
        freq = juce::jlimit(200.0f, 5000.0f, freq);
        *formantFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            currentSampleRate, freq, 2.0f, 1.2f);
    }

    float pitchCorrection;
    float correctionSpeed;
    float pitchDrift;
    float spectralShaping;
    float formantShift;
    float brightness;
    bool harmonyEnabled;
    std::array<float, 4> harmonyVoices;
    std::array<float, 4> harmonyLevels;
    float humanize;
    double currentSampleRate;
    
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, 
                                    juce::dsp::IIR::Coefficients<float>> brightnessFilter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, 
                                    juce::dsp::IIR::Coefficients<float>> formantFilter;
    std::array<juce::dsp::DelayLine<float>, 4> pitchDelays;
};

//==============================================================================
/**
 * @brief Epic Space Reverb with advanced reverb controls
 */
class EpicSpaceReverb
{
public:
    // Constants for reverb calculations
    static constexpr float EARLY_REFLECTIONS_MIX = 0.3f;
    static constexpr float ER_DECAY_FACTOR = 0.1f;
    static constexpr float ROOM_SHAPE_OFFSET = 0.5f;
    
    EpicSpaceReverb()
        : roomSize(0.7f)
        , decayTime(2.5f)
        , damping(0.5f)
        , preDelay(30.0f)
        , wetDryMix(0.3f)
        , earlyReflections(0.5f)
        , lateReverb(0.7f)
        , reverbTail(0.8f)
        , roomShape(0.5f)
        , currentSampleRate(44100.0)
    {
    }

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        currentSampleRate = spec.sampleRate;
        
        reverb.prepare(spec);
        updateReverbParameters();
        
        preDelayLine.prepare(spec);
        preDelayLine.setMaximumDelayInSamples(static_cast<int>(spec.sampleRate * 0.2)); // 200ms max pre-delay
        updatePreDelay();
        
        // Prepare early reflections delay lines
        for (int i = 0; i < 8; ++i)
        {
            earlyReflectionDelays[i].prepare(spec);
            earlyReflectionDelays[i].setMaximumDelayInSamples(static_cast<int>(spec.sampleRate * 0.1));
            
            // Set different delay times for early reflections
            float delayMs = 5.0f + static_cast<float>(i) * 8.0f; // 5ms, 13ms, 21ms, etc.
            earlyReflectionDelays[i].setDelay(static_cast<float>(delayMs * spec.sampleRate / 1000.0));
        }
        
        // Prepare damping filter
        dampingFilter.prepare(spec);
        updateDampingFilter();
        
        wetBuffer.setSize(static_cast<int>(spec.numChannels), static_cast<int>(spec.maximumBlockSize));
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        int numSamples = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();
        
        // Copy to wet buffer
        wetBuffer.copyFrom(0, 0, buffer, 0, 0, numSamples);
        if (numChannels > 1)
            wetBuffer.copyFrom(1, 0, buffer, 1, 0, numSamples);
        
        // Apply pre-delay
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* wetData = wetBuffer.getWritePointer(ch);
            for (int i = 0; i < numSamples; ++i)
            {
                float delayed = preDelayLine.popSample(ch);
                preDelayLine.pushSample(ch, wetData[i]);
                wetData[i] = delayed;
            }
        }
        
        // Apply early reflections
        if (earlyReflections > 0.0f)
        {
            for (int i = 0; i < numSamples; ++i)
            {
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    float* wetData = wetBuffer.getWritePointer(ch);
                    float earlySum = 0.0f;
                    
                    for (int er = 0; er < 8; ++er)
                    {
                        float erSample = earlyReflectionDelays[er].popSample(ch);
                        earlyReflectionDelays[er].pushSample(ch, wetData[i]);
                        
                        // Apply room shape (affects reflection pattern)
                        float erGain = (1.0f - static_cast<float>(er) * ER_DECAY_FACTOR) * earlyReflections;
                        erGain *= (ROOM_SHAPE_OFFSET + roomShape * ROOM_SHAPE_OFFSET);
                        earlySum += erSample * erGain;
                    }
                    
                    wetData[i] += earlySum * EARLY_REFLECTIONS_MIX;
                }
            }
        }
        
        // Apply main reverb
        juce::dsp::AudioBlock<float> wetBlock(wetBuffer);
        juce::dsp::ProcessContextReplacing<float> reverbContext(wetBlock);
        reverb.process(reverbContext);
        
        // Apply damping filter
        juce::dsp::ProcessContextReplacing<float> dampContext(wetBlock);
        dampingFilter.process(dampContext);
        
        // Apply reverb tail adjustment
        wetBuffer.applyGain(lateReverb * reverbTail);
        
        // Mix wet and dry
        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float* wetData = wetBuffer.getReadPointer(ch);
            float* outData = buffer.getWritePointer(ch);
            
            for (int i = 0; i < numSamples; ++i)
            {
                outData[i] = outData[i] * (1.0f - wetDryMix) + wetData[i] * wetDryMix;
            }
        }
    }

    void reset()
    {
        reverb.reset();
        preDelayLine.reset();
        dampingFilter.reset();
        for (auto& delay : earlyReflectionDelays)
        {
            delay.reset();
        }
    }

    // Basic controls
    void setRoomSize(float size) { roomSize = juce::jlimit(0.0f, 1.0f, size); updateReverbParameters(); }
    void setDecayTime(float time) { decayTime = juce::jlimit(0.1f, 10.0f, time); updateReverbParameters(); }
    void setDamping(float d) { damping = juce::jlimit(0.0f, 1.0f, d); updateReverbParameters(); updateDampingFilter(); }
    void setPreDelay(float ms) { preDelay = juce::jlimit(0.0f, 200.0f, ms); updatePreDelay(); }
    void setWetDryMix(float mix) { wetDryMix = juce::jlimit(0.0f, 1.0f, mix); }
    
    // Advanced controls
    void setEarlyReflections(float amount) { earlyReflections = juce::jlimit(0.0f, 1.0f, amount); }
    void setLateReverb(float amount) { lateReverb = juce::jlimit(0.0f, 1.0f, amount); }
    void setReverbTail(float amount) { reverbTail = juce::jlimit(0.0f, 1.0f, amount); }
    void setRoomShape(float shape) { roomShape = juce::jlimit(0.0f, 1.0f, shape); }

private:
    void updateReverbParameters()
    {
        juce::dsp::Reverb::Parameters params;
        params.roomSize = roomSize;
        params.damping = damping;
        params.wetLevel = 1.0f; // We handle wet/dry mix ourselves
        params.dryLevel = 0.0f;
        params.width = 1.0f;
        params.freezeMode = 0.0f;
        reverb.setParameters(params);
    }
    
    void updatePreDelay()
    {
        float delaySamples = static_cast<float>(preDelay * currentSampleRate / 1000.0);
        preDelayLine.setDelay(delaySamples);
    }
    
    void updateDampingFilter()
    {
        // Higher damping = more high frequency absorption
        float cutoff = 20000.0f - damping * 15000.0f;
        cutoff = juce::jlimit(1000.0f, 20000.0f, cutoff);
        *dampingFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(
            currentSampleRate, cutoff);
    }

    float roomSize;
    float decayTime;
    float damping;
    float preDelay;
    float wetDryMix;
    float earlyReflections;
    float lateReverb;
    float reverbTail;
    float roomShape;
    double currentSampleRate;
    
    // Maximum delay line sizes: 192000 samples = ~4 seconds at 48kHz (for pre-delay)
    // Early reflection delay: 19200 samples = ~400ms at 48kHz (for early reflections)
    static constexpr int MAX_PRE_DELAY_SAMPLES = 192000;
    static constexpr int MAX_EARLY_REFLECTION_DELAY_SAMPLES = 19200;
    
    juce::dsp::Reverb reverb;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> preDelayLine{MAX_PRE_DELAY_SAMPLES};
    std::array<juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>, 8> earlyReflectionDelays = {{
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>{MAX_EARLY_REFLECTION_DELAY_SAMPLES},
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>{MAX_EARLY_REFLECTION_DELAY_SAMPLES},
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>{MAX_EARLY_REFLECTION_DELAY_SAMPLES},
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>{MAX_EARLY_REFLECTION_DELAY_SAMPLES},
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>{MAX_EARLY_REFLECTION_DELAY_SAMPLES},
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>{MAX_EARLY_REFLECTION_DELAY_SAMPLES},
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>{MAX_EARLY_REFLECTION_DELAY_SAMPLES},
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>{MAX_EARLY_REFLECTION_DELAY_SAMPLES}
    }};
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, 
                                    juce::dsp::IIR::Coefficients<float>> dampingFilter;
    juce::AudioBuffer<float> wetBuffer;
};

} // namespace dspmodules
