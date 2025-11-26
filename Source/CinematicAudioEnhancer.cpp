/**
 * @file CinematicAudioEnhancer.cpp
 * @brief Implementation of Cinematic Audio Enhancement Module
 */

#include "CinematicAudioEnhancer.h"

namespace MAEVN
{

//==============================================================================
// HighPassFilter Implementation
//==============================================================================

HighPassFilter::HighPassFilter()
    : currentSampleRate(44100.0)
    , cutoffFreq(80.0f)
{
}

void HighPassFilter::prepare(double sampleRate, int maxBlockSize)
{
    currentSampleRate = sampleRate;
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(maxBlockSize);
    spec.numChannels = 2;
    
    highPassFilter.prepare(spec);
    setCutoffFrequency(cutoffFreq);
}

void HighPassFilter::process(juce::AudioBuffer<float>& buffer, int numSamples)
{
    juce::dsp::AudioBlock<float> block(buffer.getArrayOfWritePointers(), 
                                        buffer.getNumChannels(), 
                                        static_cast<size_t>(numSamples));
    juce::dsp::ProcessContextReplacing<float> context(block);
    highPassFilter.process(context);
}

void HighPassFilter::reset()
{
    highPassFilter.reset();
}

void HighPassFilter::setCutoffFrequency(float frequency)
{
    cutoffFreq = juce::jlimit(20.0f, 500.0f, frequency);
    *highPassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(
        currentSampleRate, cutoffFreq);
}

//==============================================================================
// PresenceEQ Implementation
//==============================================================================

PresenceEQ::PresenceEQ()
    : currentSampleRate(44100.0)
    , frequency(4000.0f)
    , gainDB(3.0f)
    , qFactor(1.0f)
{
}

void PresenceEQ::prepare(double sampleRate, int maxBlockSize)
{
    currentSampleRate = sampleRate;
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(maxBlockSize);
    spec.numChannels = 2;
    
    presenceFilter.prepare(spec);
    updateCoefficients();
}

void PresenceEQ::process(juce::AudioBuffer<float>& buffer, int numSamples)
{
    juce::dsp::AudioBlock<float> block(buffer.getArrayOfWritePointers(), 
                                        buffer.getNumChannels(), 
                                        static_cast<size_t>(numSamples));
    juce::dsp::ProcessContextReplacing<float> context(block);
    presenceFilter.process(context);
}

void PresenceEQ::reset()
{
    presenceFilter.reset();
}

void PresenceEQ::setFrequency(float freq)
{
    frequency = juce::jlimit(1000.0f, 8000.0f, freq);
    updateCoefficients();
}

void PresenceEQ::setGain(float dB)
{
    gainDB = juce::jlimit(-12.0f, 12.0f, dB);
    updateCoefficients();
}

void PresenceEQ::setQ(float q)
{
    qFactor = juce::jlimit(0.1f, 10.0f, q);
    updateCoefficients();
}

void PresenceEQ::updateCoefficients()
{
    float gainLinear = dBToGain(gainDB);
    *presenceFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        currentSampleRate, frequency, qFactor, gainLinear);
}

//==============================================================================
// GentleCompressor Implementation
//==============================================================================

GentleCompressor::GentleCompressor()
    : currentSampleRate(44100.0)
{
    // Default gentle settings for natural vocals
    compressor.setThreshold(-18.0f);
    compressor.setRatio(2.0f);
    compressor.setAttack(10.0f);
    compressor.setRelease(100.0f);
}

void GentleCompressor::prepare(double sampleRate, int maxBlockSize)
{
    currentSampleRate = sampleRate;
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(maxBlockSize);
    spec.numChannels = 2;
    
    compressor.prepare(spec);
}

void GentleCompressor::process(juce::AudioBuffer<float>& buffer, int numSamples)
{
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    compressor.process(context);
}

void GentleCompressor::reset()
{
    compressor.reset();
}

void GentleCompressor::setThreshold(float dB)
{
    compressor.setThreshold(juce::jlimit(-60.0f, 0.0f, dB));
}

void GentleCompressor::setRatio(float ratio)
{
    compressor.setRatio(juce::jlimit(1.0f, 20.0f, ratio));
}

void GentleCompressor::setAttack(float ms)
{
    compressor.setAttack(juce::jlimit(0.1f, 500.0f, ms));
}

void GentleCompressor::setRelease(float ms)
{
    compressor.setRelease(juce::jlimit(1.0f, 2000.0f, ms));
}

//==============================================================================
// CinematicReverb Implementation
//==============================================================================

CinematicReverb::CinematicReverb()
    : preDelayMs(30.0f)
    , currentSampleRate(44100.0)
{
    // Large hall reverb defaults for cinematic feel
    reverbParams.roomSize = 0.8f;
    reverbParams.damping = 0.4f;
    reverbParams.wetLevel = 0.25f;
    reverbParams.dryLevel = 0.75f;
    reverbParams.width = 1.0f;
    reverb.setParameters(reverbParams);
}

void CinematicReverb::prepare(double sampleRate, int maxBlockSize)
{
    currentSampleRate = sampleRate;
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(maxBlockSize);
    spec.numChannels = 2;
    
    reverb.prepare(spec);
    preDelayLine.prepare(spec);
    
    // Set pre-delay
    float delaySamples = static_cast<float>(preDelayMs * sampleRate / 1000.0);
    preDelayLine.setDelay(delaySamples);
}

void CinematicReverb::process(juce::AudioBuffer<float>& buffer, int numSamples)
{
    // Apply pre-delay first
    juce::dsp::AudioBlock<float> block(buffer.getArrayOfWritePointers(), 
                                        buffer.getNumChannels(), 
                                        static_cast<size_t>(numSamples));
    
    for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
    {
        auto* channelData = block.getChannelPointer(channel);
        for (size_t i = 0; i < static_cast<size_t>(numSamples); ++i)
        {
            float delayed = preDelayLine.popSample(static_cast<int>(channel));
            preDelayLine.pushSample(static_cast<int>(channel), channelData[i]);
            channelData[i] = delayed;
        }
    }
    
    // Then apply reverb
    juce::dsp::ProcessContextReplacing<float> context(block);
    reverb.process(context);
}

void CinematicReverb::reset()
{
    reverb.reset();
    preDelayLine.reset();
}

void CinematicReverb::setRoomSize(float size)
{
    reverbParams.roomSize = clamp(size, 0.0f, 1.0f);
    reverb.setParameters(reverbParams);
}

void CinematicReverb::setDamping(float damping)
{
    reverbParams.damping = clamp(damping, 0.0f, 1.0f);
    reverb.setParameters(reverbParams);
}

void CinematicReverb::setWetLevel(float level)
{
    reverbParams.wetLevel = clamp(level, 0.0f, 1.0f);
    reverb.setParameters(reverbParams);
}

void CinematicReverb::setDryLevel(float level)
{
    reverbParams.dryLevel = clamp(level, 0.0f, 1.0f);
    reverb.setParameters(reverbParams);
}

void CinematicReverb::setPreDelay(float ms)
{
    preDelayMs = juce::jlimit(0.0f, 200.0f, ms);
    float delaySamples = static_cast<float>(preDelayMs * currentSampleRate / 1000.0);
    preDelayLine.setDelay(delaySamples);
}

void CinematicReverb::setWidth(float width)
{
    reverbParams.width = clamp(width, 0.0f, 1.0f);
    reverb.setParameters(reverbParams);
}

//==============================================================================
// SubtleDelay Implementation
//==============================================================================

SubtleDelay::SubtleDelay()
    : currentSampleRate(44100.0)
    , delayTimeMs(300.0f)
    , feedbackAmount(0.3f)
    , mixLevel(0.2f)
{
}

void SubtleDelay::prepare(double sampleRate, int maxBlockSize)
{
    currentSampleRate = sampleRate;
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(maxBlockSize);
    spec.numChannels = 2;
    
    delayLine.prepare(spec);
    setDelayTime(delayTimeMs);
    
    feedbackBuffer.setSize(2, maxBlockSize);
}

void SubtleDelay::process(juce::AudioBuffer<float>& buffer, int numSamples)
{
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        
        for (int i = 0; i < numSamples; ++i)
        {
            float input = channelData[i];
            float delayed = delayLine.popSample(channel);
            
            // Apply feedback
            float feedbackSample = delayed * feedbackAmount;
            delayLine.pushSample(channel, input + feedbackSample);
            
            // Mix dry and wet signals
            channelData[i] = input * (1.0f - mixLevel) + delayed * mixLevel;
        }
    }
}

void SubtleDelay::reset()
{
    delayLine.reset();
}

void SubtleDelay::setDelayTime(float ms)
{
    delayTimeMs = juce::jlimit(1.0f, 2000.0f, ms);
    float delaySamples = static_cast<float>(delayTimeMs * currentSampleRate / 1000.0);
    delayLine.setDelay(delaySamples);
}

void SubtleDelay::setDelayTimeFromBPM(double bpm)
{
    // Quarter note delay time in ms
    float quarterNoteMs = static_cast<float>(60000.0 / bpm);
    setDelayTime(quarterNoteMs);
}

void SubtleDelay::setFeedback(float feedback)
{
    feedbackAmount = juce::jlimit(0.0f, 0.9f, feedback);
}

void SubtleDelay::setMix(float mix)
{
    mixLevel = juce::jlimit(0.0f, 1.0f, mix);
}

//==============================================================================
// ModulationEffect Implementation
//==============================================================================

ModulationEffect::ModulationEffect()
    : currentSampleRate(44100.0)
{
}

void ModulationEffect::prepare(double sampleRate, int maxBlockSize)
{
    currentSampleRate = sampleRate;
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(maxBlockSize);
    spec.numChannels = 2;
    
    chorus.prepare(spec);
    
    // Default lush chorus settings
    chorus.setRate(0.5f);
    chorus.setDepth(0.3f);
    chorus.setMix(0.3f);
    chorus.setCentreDelay(7.0f);
    chorus.setFeedback(-0.2f);
}

void ModulationEffect::process(juce::AudioBuffer<float>& buffer, int numSamples)
{
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    chorus.process(context);
}

void ModulationEffect::reset()
{
    chorus.reset();
}

void ModulationEffect::setRate(float hz)
{
    chorus.setRate(juce::jlimit(0.1f, 10.0f, hz));
}

void ModulationEffect::setDepth(float depth)
{
    chorus.setDepth(juce::jlimit(0.0f, 1.0f, depth));
}

void ModulationEffect::setMix(float mix)
{
    chorus.setMix(juce::jlimit(0.0f, 1.0f, mix));
}

//==============================================================================
// WarmSaturation Implementation
//==============================================================================

WarmSaturation::WarmSaturation()
    : driveAmount(0.2f)
    , outputGainLinear(1.0f)
{
}

void WarmSaturation::prepare(double /*sampleRate*/, int /*maxBlockSize*/)
{
    // No special preparation needed
}

void WarmSaturation::process(juce::AudioBuffer<float>& buffer, int numSamples)
{
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        
        for (int i = 0; i < numSamples; ++i)
        {
            // Apply drive
            float input = channelData[i] * (1.0f + driveAmount * 10.0f);
            
            // Apply soft saturation
            float saturated = saturate(input);
            
            // Apply output gain compensation
            channelData[i] = saturated * outputGainLinear;
        }
    }
}

void WarmSaturation::reset()
{
    // No state to reset
}

void WarmSaturation::setDrive(float drive)
{
    driveAmount = juce::jlimit(0.0f, 1.0f, drive);
}

void WarmSaturation::setOutputGain(float dB)
{
    outputGainLinear = dBToGain(juce::jlimit(-24.0f, 12.0f, dB));
}

float WarmSaturation::saturate(float sample)
{
    // Soft clipping with tanh for warm saturation
    return std::tanh(sample);
}

//==============================================================================
// MultibandCompressor Implementation
//==============================================================================

MultibandCompressor::MultibandCompressor()
    : currentSampleRate(44100.0)
{
    // Default compressor settings
    lowBandCompressor.setThreshold(-20.0f);
    lowBandCompressor.setRatio(4.0f);
    lowBandCompressor.setAttack(10.0f);
    lowBandCompressor.setRelease(100.0f);
    
    midBandCompressor.setThreshold(-15.0f);
    midBandCompressor.setRatio(3.0f);
    midBandCompressor.setAttack(5.0f);
    midBandCompressor.setRelease(80.0f);
    
    highBandCompressor.setThreshold(-12.0f);
    highBandCompressor.setRatio(2.0f);
    highBandCompressor.setAttack(2.0f);
    highBandCompressor.setRelease(60.0f);
}

void MultibandCompressor::prepare(double sampleRate, int maxBlockSize)
{
    currentSampleRate = sampleRate;
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(maxBlockSize);
    spec.numChannels = 2;
    
    // Prepare crossover filters
    lowCrossover.prepare(spec);
    highCrossover.prepare(spec);
    
    lowCrossover.setCutoffFrequency(LOW_CROSSOVER_FREQ);
    highCrossover.setCutoffFrequency(HIGH_CROSSOVER_FREQ);
    
    lowCrossover.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    highCrossover.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    
    // Prepare band compressors
    lowBandCompressor.prepare(spec);
    midBandCompressor.prepare(spec);
    highBandCompressor.prepare(spec);
    
    // Allocate band buffers
    lowBandBuffer.setSize(2, maxBlockSize);
    midBandBuffer.setSize(2, maxBlockSize);
    highBandBuffer.setSize(2, maxBlockSize);
}

void MultibandCompressor::process(juce::AudioBuffer<float>& buffer, int numSamples)
{
    // Copy input to band buffers
    lowBandBuffer.copyFrom(0, 0, buffer, 0, 0, numSamples);
    lowBandBuffer.copyFrom(1, 0, buffer, 1, 0, numSamples);
    midBandBuffer.copyFrom(0, 0, buffer, 0, 0, numSamples);
    midBandBuffer.copyFrom(1, 0, buffer, 1, 0, numSamples);
    highBandBuffer.copyFrom(0, 0, buffer, 0, 0, numSamples);
    highBandBuffer.copyFrom(1, 0, buffer, 1, 0, numSamples);
    
    // Apply crossover filters
    juce::dsp::AudioBlock<float> lowBlock(lowBandBuffer.getArrayOfWritePointers(), 2, static_cast<size_t>(numSamples));
    juce::dsp::AudioBlock<float> midBlock(midBandBuffer.getArrayOfWritePointers(), 2, static_cast<size_t>(numSamples));
    juce::dsp::AudioBlock<float> highBlock(highBandBuffer.getArrayOfWritePointers(), 2, static_cast<size_t>(numSamples));
    
    juce::dsp::ProcessContextReplacing<float> lowContext(lowBlock);
    juce::dsp::ProcessContextReplacing<float> midContext(midBlock);
    juce::dsp::ProcessContextReplacing<float> highContext(highBlock);
    
    lowCrossover.process(lowContext);
    highCrossover.process(highContext);
    
    // Mid band is what's left (simplified - in production would use proper bandpass)
    for (int channel = 0; channel < 2; ++channel)
    {
        auto* midData = midBandBuffer.getWritePointer(channel);
        const auto* lowData = lowBandBuffer.getReadPointer(channel);
        const auto* highData = highBandBuffer.getReadPointer(channel);
        
        for (int i = 0; i < numSamples; ++i)
        {
            midData[i] = midData[i] - lowData[i] - highData[i];
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
    for (int channel = 0; channel < 2; ++channel)
    {
        auto* outputData = buffer.getWritePointer(channel);
        const auto* lowData = lowBandBuffer.getReadPointer(channel);
        const auto* midData = midBandBuffer.getReadPointer(channel);
        const auto* highData = highBandBuffer.getReadPointer(channel);
        
        for (int i = 0; i < numSamples; ++i)
        {
            outputData[i] = lowData[i] + midData[i] + highData[i];
        }
    }
}

void MultibandCompressor::reset()
{
    lowCrossover.reset();
    highCrossover.reset();
    lowBandCompressor.reset();
    midBandCompressor.reset();
    highBandCompressor.reset();
}

void MultibandCompressor::setLowBandThreshold(float dB)
{
    lowBandCompressor.setThreshold(juce::jlimit(-60.0f, 0.0f, dB));
}

void MultibandCompressor::setMidBandThreshold(float dB)
{
    midBandCompressor.setThreshold(juce::jlimit(-60.0f, 0.0f, dB));
}

void MultibandCompressor::setHighBandThreshold(float dB)
{
    highBandCompressor.setThreshold(juce::jlimit(-60.0f, 0.0f, dB));
}

void MultibandCompressor::setLowBandRatio(float ratio)
{
    lowBandCompressor.setRatio(juce::jlimit(1.0f, 20.0f, ratio));
}

void MultibandCompressor::setMidBandRatio(float ratio)
{
    midBandCompressor.setRatio(juce::jlimit(1.0f, 20.0f, ratio));
}

void MultibandCompressor::setHighBandRatio(float ratio)
{
    highBandCompressor.setRatio(juce::jlimit(1.0f, 20.0f, ratio));
}

//==============================================================================
// StereoImager Implementation
//==============================================================================

StereoImager::StereoImager()
    : stereoWidth(1.2f)
    , monoFrequency(200.0f)
    , currentSampleRate(44100.0)
{
}

void StereoImager::prepare(double sampleRate, int maxBlockSize)
{
    currentSampleRate = sampleRate;
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(maxBlockSize);
    spec.numChannels = 2;
    
    lowPassFilter.prepare(spec);
    *lowPassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(
        sampleRate, monoFrequency);
}

void StereoImager::process(juce::AudioBuffer<float>& buffer, int numSamples)
{
    if (buffer.getNumChannels() < 2)
        return;
    
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
        side *= stereoWidth;
        
        // Reconstruct left and right
        leftChannel[i] = mid + side;
        rightChannel[i] = mid - side;
    }
    
    // Apply low-pass filter to keep bass mono (simplified approach)
    // In a full implementation, would process low frequencies separately
}

void StereoImager::reset()
{
    lowPassFilter.reset();
}

void StereoImager::setWidth(float width)
{
    stereoWidth = juce::jlimit(0.0f, 2.0f, width);
}

void StereoImager::setMonoFrequency(float frequency)
{
    monoFrequency = juce::jlimit(50.0f, 500.0f, frequency);
    *lowPassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(
        currentSampleRate, monoFrequency);
}

//==============================================================================
// LoudnessNormalizer Implementation
//==============================================================================

LoudnessNormalizer::LoudnessNormalizer()
    : targetLUFS(-14.0f)
    , currentLUFS(-24.0f)
    , currentGain(1.0f)
    , rmsSum(0.0f)
    , rmsSampleCount(0)
    , currentSampleRate(44100.0)
{
}

void LoudnessNormalizer::prepare(double sampleRate, int /*maxBlockSize*/)
{
    currentSampleRate = sampleRate;
    reset();
}

void LoudnessNormalizer::process(juce::AudioBuffer<float>& buffer, int numSamples)
{
    // Measure RMS
    float sumSquares = 0.0f;
    
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        const auto* channelData = buffer.getReadPointer(channel);
        for (int i = 0; i < numSamples; ++i)
        {
            sumSquares += channelData[i] * channelData[i];
        }
    }
    
    rmsSum += sumSquares;
    rmsSampleCount += numSamples * buffer.getNumChannels();
    
    // Update loudness measurement every second
    if (rmsSampleCount >= RMS_WINDOW_SIZE)
    {
        float rms = std::sqrt(rmsSum / static_cast<float>(rmsSampleCount));
        // Approximate LUFS from RMS (simplified - real LUFS uses K-weighting)
        currentLUFS = 20.0f * std::log10(rms + 0.0000001f);
        updateGain();
        
        rmsSum = 0.0f;
        rmsSampleCount = 0;
    }
    
    // Apply gain
    buffer.applyGain(currentGain);
}

void LoudnessNormalizer::reset()
{
    rmsSum = 0.0f;
    rmsSampleCount = 0;
    currentGain = 1.0f;
    currentLUFS = -24.0f;
}

void LoudnessNormalizer::setTargetLUFS(float lufs)
{
    targetLUFS = juce::jlimit(-24.0f, -6.0f, lufs);
}

float LoudnessNormalizer::getCurrentLUFS() const
{
    return currentLUFS;
}

void LoudnessNormalizer::updateGain()
{
    // Calculate required gain adjustment
    float gainDB = targetLUFS - currentLUFS;
    
    // Limit gain adjustment to prevent extreme changes
    gainDB = juce::jlimit(-12.0f, 12.0f, gainDB);
    
    // Smooth gain change (simple one-pole filter)
    float targetGain = dBToGain(gainDB);
    currentGain = currentGain * 0.9f + targetGain * 0.1f;
}

//==============================================================================
// FinalLimiter Implementation
//==============================================================================

FinalLimiter::FinalLimiter()
{
    limiter.setThreshold(-0.1f);
    limiter.setRelease(50.0f);
}

void FinalLimiter::prepare(double sampleRate, int maxBlockSize)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(maxBlockSize);
    spec.numChannels = 2;
    
    limiter.prepare(spec);
}

void FinalLimiter::process(juce::AudioBuffer<float>& buffer, int /*numSamples*/)
{
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    limiter.process(context);
}

void FinalLimiter::reset()
{
    limiter.reset();
}

void FinalLimiter::setCeiling(float dB)
{
    limiter.setThreshold(juce::jlimit(-12.0f, 0.0f, dB));
}

void FinalLimiter::setRelease(float ms)
{
    limiter.setRelease(juce::jlimit(1.0f, 500.0f, ms));
}

//==============================================================================
// CinematicAudioEnhancer Implementation
//==============================================================================

CinematicAudioEnhancer::CinematicAudioEnhancer()
    : currentSampleRate(44100.0)
    , currentMaxBlockSize(512)
    , highPassEnabled(true)
    , presenceEQEnabled(true)
    , vocalCompressorEnabled(true)
    , cinematicReverbEnabled(true)
    , subtleDelayEnabled(false)
    , modulationEnabled(false)
    , saturationEnabled(false)
    , multibandCompressorEnabled(true)
    , stereoImagerEnabled(true)
    , loudnessNormalizerEnabled(true)
    , finalLimiterEnabled(true)
{
    // Apply default cinematic preset
    applyCinematicVocalPreset();
}

CinematicAudioEnhancer::~CinematicAudioEnhancer()
{
}

void CinematicAudioEnhancer::prepare(double sampleRate, int maxBlockSize)
{
    const juce::ScopedLock sl(processLock);
    
    currentSampleRate = sampleRate;
    currentMaxBlockSize = maxBlockSize;
    
    // Prepare all components
    highPassFilter.prepare(sampleRate, maxBlockSize);
    presenceEQ.prepare(sampleRate, maxBlockSize);
    vocalCompressor.prepare(sampleRate, maxBlockSize);
    cinematicReverb.prepare(sampleRate, maxBlockSize);
    subtleDelay.prepare(sampleRate, maxBlockSize);
    modulationEffect.prepare(sampleRate, maxBlockSize);
    warmSaturation.prepare(sampleRate, maxBlockSize);
    multibandCompressor.prepare(sampleRate, maxBlockSize);
    stereoImager.prepare(sampleRate, maxBlockSize);
    loudnessNormalizer.prepare(sampleRate, maxBlockSize);
    finalLimiter.prepare(sampleRate, maxBlockSize);
    
    Logger::log(Logger::Level::Info, "CinematicAudioEnhancer prepared");
}

void CinematicAudioEnhancer::process(juce::AudioBuffer<float>& buffer, int numSamples)
{
    const juce::ScopedLock sl(processLock);
    
    //==========================================================================
    // Vocal Processing Chain
    //==========================================================================
    
    // High-pass filter (remove low frequencies below 80 Hz)
    if (highPassEnabled)
        highPassFilter.process(buffer, numSamples);
    
    // Presence EQ (boost 3-5 kHz for clarity)
    if (presenceEQEnabled)
        presenceEQ.process(buffer, numSamples);
    
    // Gentle compression (2:1 ratio for natural dynamics)
    if (vocalCompressorEnabled)
        vocalCompressor.process(buffer, numSamples);
    
    //==========================================================================
    // Multi-FX Processing
    //==========================================================================
    
    // Modulation (chorus/flanger for lush sound)
    if (modulationEnabled)
        modulationEffect.process(buffer, numSamples);
    
    // Warm saturation (for richness during climactic moments)
    if (saturationEnabled)
        warmSaturation.process(buffer, numSamples);
    
    // Subtle delay (quarter-note for depth)
    if (subtleDelayEnabled)
        subtleDelay.process(buffer, numSamples);
    
    // Cinematic reverb (large hall with pre-delay)
    if (cinematicReverbEnabled)
        cinematicReverb.process(buffer, numSamples);
    
    //==========================================================================
    // Mastering Chain
    //==========================================================================
    
    // Multiband compression (control dynamics across frequency ranges)
    if (multibandCompressorEnabled)
        multibandCompressor.process(buffer, numSamples);
    
    // Stereo imaging (widen stereo, keep bass centered)
    if (stereoImagerEnabled)
        stereoImager.process(buffer, numSamples);
    
    // Loudness normalization (target -14 LUFS for streaming)
    if (loudnessNormalizerEnabled)
        loudnessNormalizer.process(buffer, numSamples);
    
    // Final limiter (-0.1 dB ceiling, no clipping)
    if (finalLimiterEnabled)
        finalLimiter.process(buffer, numSamples);
}

void CinematicAudioEnhancer::reset()
{
    const juce::ScopedLock sl(processLock);
    
    highPassFilter.reset();
    presenceEQ.reset();
    vocalCompressor.reset();
    cinematicReverb.reset();
    subtleDelay.reset();
    modulationEffect.reset();
    warmSaturation.reset();
    multibandCompressor.reset();
    stereoImager.reset();
    loudnessNormalizer.reset();
    finalLimiter.reset();
}

//==============================================================================
// Vocal Processing Setters
//==============================================================================

void CinematicAudioEnhancer::setHighPassEnabled(bool enabled)
{
    highPassEnabled = enabled;
}

void CinematicAudioEnhancer::setHighPassCutoff(float frequency)
{
    highPassFilter.setCutoffFrequency(frequency);
}

void CinematicAudioEnhancer::setPresenceEQEnabled(bool enabled)
{
    presenceEQEnabled = enabled;
}

void CinematicAudioEnhancer::setPresenceFrequency(float frequency)
{
    presenceEQ.setFrequency(frequency);
}

void CinematicAudioEnhancer::setPresenceGain(float dB)
{
    presenceEQ.setGain(dB);
}

void CinematicAudioEnhancer::setVocalCompressorEnabled(bool enabled)
{
    vocalCompressorEnabled = enabled;
}

void CinematicAudioEnhancer::setVocalCompressorThreshold(float dB)
{
    vocalCompressor.setThreshold(dB);
}

void CinematicAudioEnhancer::setVocalCompressorRatio(float ratio)
{
    vocalCompressor.setRatio(ratio);
}

void CinematicAudioEnhancer::setCinematicReverbEnabled(bool enabled)
{
    cinematicReverbEnabled = enabled;
}

void CinematicAudioEnhancer::setCinematicReverbSize(float size)
{
    cinematicReverb.setRoomSize(size);
}

void CinematicAudioEnhancer::setCinematicReverbMix(float mix)
{
    cinematicReverb.setWetLevel(mix);
    cinematicReverb.setDryLevel(1.0f - mix);
}

void CinematicAudioEnhancer::setCinematicReverbPreDelay(float ms)
{
    cinematicReverb.setPreDelay(ms);
}

void CinematicAudioEnhancer::setSubtleDelayEnabled(bool enabled)
{
    subtleDelayEnabled = enabled;
}

void CinematicAudioEnhancer::setSubtleDelayTime(float ms)
{
    subtleDelay.setDelayTime(ms);
}

void CinematicAudioEnhancer::setSubtleDelayMix(float mix)
{
    subtleDelay.setMix(mix);
}

//==============================================================================
// Multi-FX Setters
//==============================================================================

void CinematicAudioEnhancer::setModulationEnabled(bool enabled)
{
    modulationEnabled = enabled;
}

void CinematicAudioEnhancer::setModulationRate(float hz)
{
    modulationEffect.setRate(hz);
}

void CinematicAudioEnhancer::setModulationDepth(float depth)
{
    modulationEffect.setDepth(depth);
}

void CinematicAudioEnhancer::setModulationMix(float mix)
{
    modulationEffect.setMix(mix);
}

void CinematicAudioEnhancer::setSaturationEnabled(bool enabled)
{
    saturationEnabled = enabled;
}

void CinematicAudioEnhancer::setSaturationDrive(float drive)
{
    warmSaturation.setDrive(drive);
}

//==============================================================================
// Mastering Chain Setters
//==============================================================================

void CinematicAudioEnhancer::setMultibandCompressorEnabled(bool enabled)
{
    multibandCompressorEnabled = enabled;
}

void CinematicAudioEnhancer::setStereoImagerEnabled(bool enabled)
{
    stereoImagerEnabled = enabled;
}

void CinematicAudioEnhancer::setStereoWidth(float width)
{
    stereoImager.setWidth(width);
}

void CinematicAudioEnhancer::setLoudnessNormalizerEnabled(bool enabled)
{
    loudnessNormalizerEnabled = enabled;
}

void CinematicAudioEnhancer::setTargetLUFS(float lufs)
{
    loudnessNormalizer.setTargetLUFS(lufs);
}

void CinematicAudioEnhancer::setFinalLimiterEnabled(bool enabled)
{
    finalLimiterEnabled = enabled;
}

void CinematicAudioEnhancer::setLimiterCeiling(float dB)
{
    finalLimiter.setCeiling(dB);
}

//==============================================================================
// Preset Methods
//==============================================================================

void CinematicAudioEnhancer::applyCinematicVocalPreset()
{
    // Vocal Processing - Grammy-quality settings
    setHighPassEnabled(true);
    setHighPassCutoff(80.0f);
    
    setPresenceEQEnabled(true);
    setPresenceFrequency(4000.0f);
    setPresenceGain(3.0f);
    
    setVocalCompressorEnabled(true);
    setVocalCompressorThreshold(-18.0f);
    setVocalCompressorRatio(2.0f);
    
    setCinematicReverbEnabled(true);
    setCinematicReverbSize(0.8f);
    setCinematicReverbMix(0.25f);
    setCinematicReverbPreDelay(30.0f);
    
    setSubtleDelayEnabled(true);
    setSubtleDelayTime(300.0f);
    setSubtleDelayMix(0.15f);
    
    // Multi-FX - Subtle enhancement
    setModulationEnabled(false);
    setSaturationEnabled(false);
    
    // Mastering - Professional polish
    setMultibandCompressorEnabled(true);
    setStereoImagerEnabled(true);
    setStereoWidth(1.2f);
    setLoudnessNormalizerEnabled(true);
    setTargetLUFS(-14.0f);
    setFinalLimiterEnabled(true);
    setLimiterCeiling(-0.1f);
    
    Logger::log(Logger::Level::Info, "Applied Cinematic Vocal Preset");
}

void CinematicAudioEnhancer::applyCinematicMasteringPreset()
{
    // Disable vocal-specific processing
    setHighPassEnabled(false);
    setPresenceEQEnabled(false);
    setVocalCompressorEnabled(false);
    setCinematicReverbEnabled(false);
    setSubtleDelayEnabled(false);
    
    // Multi-FX - Light enhancement
    setModulationEnabled(false);
    setSaturationEnabled(true);
    setSaturationDrive(0.1f);
    
    // Mastering - Full chain
    setMultibandCompressorEnabled(true);
    setStereoImagerEnabled(true);
    setStereoWidth(1.3f);
    setLoudnessNormalizerEnabled(true);
    setTargetLUFS(-14.0f);
    setFinalLimiterEnabled(true);
    setLimiterCeiling(-0.1f);
    
    Logger::log(Logger::Level::Info, "Applied Cinematic Mastering Preset");
}

void CinematicAudioEnhancer::applyViralAppealPreset()
{
    // Vocal Processing - Punchy and present
    setHighPassEnabled(true);
    setHighPassCutoff(100.0f);
    
    setPresenceEQEnabled(true);
    setPresenceFrequency(5000.0f);
    setPresenceGain(4.0f);
    
    setVocalCompressorEnabled(true);
    setVocalCompressorThreshold(-15.0f);
    setVocalCompressorRatio(3.0f);
    
    setCinematicReverbEnabled(true);
    setCinematicReverbSize(0.5f);
    setCinematicReverbMix(0.2f);
    setCinematicReverbPreDelay(20.0f);
    
    setSubtleDelayEnabled(true);
    setSubtleDelayTime(250.0f);
    setSubtleDelayMix(0.1f);
    
    // Multi-FX - More aggressive
    setModulationEnabled(true);
    setModulationRate(0.3f);
    setModulationDepth(0.2f);
    setModulationMix(0.15f);
    
    setSaturationEnabled(true);
    setSaturationDrive(0.15f);
    
    // Mastering - Louder for impact
    setMultibandCompressorEnabled(true);
    setStereoImagerEnabled(true);
    setStereoWidth(1.4f);
    setLoudnessNormalizerEnabled(true);
    setTargetLUFS(-12.0f);
    setFinalLimiterEnabled(true);
    setLimiterCeiling(-0.1f);
    
    Logger::log(Logger::Level::Info, "Applied Viral Appeal Preset");
}

float CinematicAudioEnhancer::getCurrentLUFS() const
{
    return loudnessNormalizer.getCurrentLUFS();
}

} // namespace MAEVN
