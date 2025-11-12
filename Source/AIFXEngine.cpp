/**
 * @file AIFXEngine.cpp
 * @brief Implementation of AI FX Engine
 */

#include "AIFXEngine.h"

namespace MAEVN
{

//==============================================================================
// CompressorEffect Implementation
//==============================================================================

CompressorEffect::CompressorEffect()
    : currentSampleRate(44100.0)
{
    compressor.setThreshold(-10.0f);
    compressor.setRatio(4.0f);
    compressor.setAttack(5.0f);
    compressor.setRelease(100.0f);
}

void CompressorEffect::prepare(double sampleRate, int maxBlockSize)
{
    currentSampleRate = sampleRate;
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = maxBlockSize;
    spec.numChannels = 2;
    compressor.prepare(spec);
}

void CompressorEffect::process(juce::AudioBuffer<float>& buffer, int numSamples)
{
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    compressor.process(context);
}

void CompressorEffect::reset()
{
    compressor.reset();
}

void CompressorEffect::setThreshold(float dB)
{
    compressor.setThreshold(dB);
}

void CompressorEffect::setRatio(float ratio)
{
    compressor.setRatio(ratio);
}

void CompressorEffect::setAttack(float ms)
{
    compressor.setAttack(ms);
}

void CompressorEffect::setRelease(float ms)
{
    compressor.setRelease(ms);
}

//==============================================================================
// EQEffect Implementation
//==============================================================================

EQEffect::EQEffect()
    : currentSampleRate(44100.0)
{
}

void EQEffect::prepare(double sampleRate, int maxBlockSize)
{
    currentSampleRate = sampleRate;
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = maxBlockSize;
    spec.numChannels = 2;
    
    eqChain.prepare(spec);
    
    // Low shelf at 200 Hz
    *eqChain.get<0>().coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(
        sampleRate, 200.0f, 0.7f, 1.0f);
    
    // Mid peak at 1000 Hz
    *eqChain.get<1>().coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sampleRate, 1000.0f, 1.0f, 1.0f);
    
    // High shelf at 8000 Hz
    *eqChain.get<2>().coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        sampleRate, 8000.0f, 0.7f, 1.0f);
}

void EQEffect::process(juce::AudioBuffer<float>& buffer, int numSamples)
{
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    eqChain.process(context);
}

void EQEffect::reset()
{
    eqChain.reset();
}

void EQEffect::setLowGain(float dB)
{
    float gain = dBToGain(dB);
    *eqChain.get<0>().coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(
        currentSampleRate, 200.0f, 0.7f, gain);
}

void EQEffect::setMidGain(float dB)
{
    float gain = dBToGain(dB);
    *eqChain.get<1>().coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        currentSampleRate, 1000.0f, 1.0f, gain);
}

void EQEffect::setHighGain(float dB)
{
    float gain = dBToGain(dB);
    *eqChain.get<2>().coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        currentSampleRate, 8000.0f, 0.7f, gain);
}

//==============================================================================
// ReverbEffect Implementation
//==============================================================================

ReverbEffect::ReverbEffect()
{
    reverbParams.roomSize = 0.5f;
    reverbParams.damping = 0.5f;
    reverbParams.wetLevel = 0.3f;
    reverbParams.dryLevel = 0.7f;
    reverbParams.width = 1.0f;
    reverb.setParameters(reverbParams);
}

void ReverbEffect::prepare(double sampleRate, int maxBlockSize)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = maxBlockSize;
    spec.numChannels = 2;
    reverb.prepare(spec);
}

void ReverbEffect::process(juce::AudioBuffer<float>& buffer, int numSamples)
{
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    reverb.process(context);
}

void ReverbEffect::reset()
{
    reverb.reset();
}

void ReverbEffect::setRoomSize(float size)
{
    reverbParams.roomSize = clamp(size, 0.0f, 1.0f);
    reverb.setParameters(reverbParams);
}

void ReverbEffect::setDamping(float damping)
{
    reverbParams.damping = clamp(damping, 0.0f, 1.0f);
    reverb.setParameters(reverbParams);
}

void ReverbEffect::setWetLevel(float level)
{
    reverbParams.wetLevel = clamp(level, 0.0f, 1.0f);
    reverb.setParameters(reverbParams);
}

void ReverbEffect::setDryLevel(float level)
{
    reverbParams.dryLevel = clamp(level, 0.0f, 1.0f);
    reverb.setParameters(reverbParams);
}

//==============================================================================
// LimiterEffect Implementation
//==============================================================================

LimiterEffect::LimiterEffect()
{
    limiter.setThreshold(-1.0f);
    limiter.setRelease(50.0f);
}

void LimiterEffect::prepare(double sampleRate, int maxBlockSize)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = maxBlockSize;
    spec.numChannels = 2;
    limiter.prepare(spec);
}

void LimiterEffect::process(juce::AudioBuffer<float>& buffer, int numSamples)
{
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    limiter.process(context);
}

void LimiterEffect::reset()
{
    limiter.reset();
}

void LimiterEffect::setThreshold(float dB)
{
    limiter.setThreshold(dB);
}

void LimiterEffect::setRelease(float ms)
{
    limiter.setRelease(ms);
}

//==============================================================================
// AIEffect Implementation
//==============================================================================

AIEffect::AIEffect(OnnxEngine* engine, const juce::String& role)
    : onnxEngine(engine)
    , modelRole(role)
    , currentSampleRate(44100.0)
{
}

void AIEffect::prepare(double sampleRate, int maxBlockSize)
{
    currentSampleRate = sampleRate;
    inputBuffer.resize(maxBlockSize * 2); // stereo
    outputBuffer.resize(maxBlockSize * 2);
}

void AIEffect::process(juce::AudioBuffer<float>& buffer, int numSamples)
{
    if (!onnxEngine || !onnxEngine->isModelReady(modelRole))
        return;
    
    // Convert audio buffer to flat array for ONNX
    int numChannels = buffer.getNumChannels();
    inputBuffer.clear();
    
    for (int channel = 0; channel < numChannels; ++channel)
    {
        const float* channelData = buffer.getReadPointer(channel);
        for (int i = 0; i < numSamples; ++i)
        {
            inputBuffer.push_back(channelData[i]);
        }
    }
    
    // Run AI inference
    std::vector<int64_t> shape = {1, numChannels, numSamples};
    if (onnxEngine->runInference(modelRole, inputBuffer, shape, outputBuffer))
    {
        // Convert output back to audio buffer
        int outputIndex = 0;
        for (int channel = 0; channel < numChannels; ++channel)
        {
            float* channelData = buffer.getWritePointer(channel);
            for (int i = 0; i < numSamples; ++i)
            {
                if (outputIndex < (int)outputBuffer.size())
                {
                    channelData[i] = outputBuffer[outputIndex++];
                }
            }
        }
    }
}

void AIEffect::reset()
{
    inputBuffer.clear();
    outputBuffer.clear();
}

//==============================================================================
// AIFXEngine Implementation
//==============================================================================

AIFXEngine::AIFXEngine(OnnxEngine* engine)
    : onnxEngine(engine)
    , currentSampleRate(44100.0)
    , currentMaxBlockSize(512)
{
}

AIFXEngine::~AIFXEngine()
{
}

void AIFXEngine::prepare(double sampleRate, int maxBlockSize)
{
    const juce::ScopedLock sl(fxLock);
    
    currentSampleRate = sampleRate;
    currentMaxBlockSize = maxBlockSize;
    
    for (auto& track : trackFX)
    {
        for (auto& effect : track.dspEffects)
        {
            effect->prepare(sampleRate, maxBlockSize);
        }
        for (auto& effect : track.aiEffects)
        {
            effect->prepare(sampleRate, maxBlockSize);
        }
    }
}

void AIFXEngine::reset()
{
    const juce::ScopedLock sl(fxLock);
    
    for (auto& track : trackFX)
    {
        for (auto& effect : track.dspEffects)
        {
            effect->reset();
        }
        for (auto& effect : track.aiEffects)
        {
            effect->reset();
        }
    }
}

void AIFXEngine::process(juce::AudioBuffer<float>& buffer, int numSamples, int trackIndex)
{
    if (trackIndex < 0 || trackIndex >= NUM_TRACKS)
        return;
    
    const juce::ScopedLock sl(fxLock);
    
    auto& track = trackFX[trackIndex];
    
    switch (track.mode)
    {
        case FXMode::Off:
            // No processing
            break;
            
        case FXMode::DSP:
            processDSP(buffer, numSamples, trackIndex);
            break;
            
        case FXMode::AI:
            processAI(buffer, numSamples, trackIndex);
            break;
            
        case FXMode::Hybrid:
            processHybrid(buffer, numSamples, trackIndex);
            break;
    }
}

void AIFXEngine::processDSP(juce::AudioBuffer<float>& buffer, int numSamples, int trackIndex)
{
    auto& track = trackFX[trackIndex];
    
    for (auto& effect : track.dspEffects)
    {
        effect->process(buffer, numSamples);
    }
}

void AIFXEngine::processAI(juce::AudioBuffer<float>& buffer, int numSamples, int trackIndex)
{
    auto& track = trackFX[trackIndex];
    
    for (auto& effect : track.aiEffects)
    {
        effect->process(buffer, numSamples);
    }
}

void AIFXEngine::processHybrid(juce::AudioBuffer<float>& buffer, int numSamples, int trackIndex)
{
    // First apply DSP effects
    processDSP(buffer, numSamples, trackIndex);
    
    // Then apply AI effects
    processAI(buffer, numSamples, trackIndex);
}

void AIFXEngine::setFXMode(int trackIndex, FXMode mode)
{
    if (trackIndex >= 0 && trackIndex < NUM_TRACKS)
    {
        const juce::ScopedLock sl(fxLock);
        trackFX[trackIndex].mode = mode;
    }
}

FXMode AIFXEngine::getFXMode(int trackIndex) const
{
    if (trackIndex >= 0 && trackIndex < NUM_TRACKS)
    {
        return trackFX[trackIndex].mode;
    }
    return FXMode::Off;
}

void AIFXEngine::addDSPEffect(int trackIndex, std::unique_ptr<Effect> effect)
{
    if (trackIndex >= 0 && trackIndex < NUM_TRACKS)
    {
        const juce::ScopedLock sl(fxLock);
        effect->prepare(currentSampleRate, currentMaxBlockSize);
        trackFX[trackIndex].dspEffects.push_back(std::move(effect));
    }
}

void AIFXEngine::addAIEffect(int trackIndex, const juce::String& modelRole)
{
    if (trackIndex >= 0 && trackIndex < NUM_TRACKS && onnxEngine)
    {
        const juce::ScopedLock sl(fxLock);
        auto effect = std::make_unique<AIEffect>(onnxEngine, modelRole);
        effect->prepare(currentSampleRate, currentMaxBlockSize);
        trackFX[trackIndex].aiEffects.push_back(std::move(effect));
    }
}

void AIFXEngine::clearEffects(int trackIndex)
{
    if (trackIndex >= 0 && trackIndex < NUM_TRACKS)
    {
        const juce::ScopedLock sl(fxLock);
        trackFX[trackIndex].dspEffects.clear();
        trackFX[trackIndex].aiEffects.clear();
    }
}

void AIFXEngine::setEffectParameter(int trackIndex, int effectIndex, 
                                     const juce::String& paramName, float value)
{
    // Parameter setting would be implemented here
    // This is a placeholder for future parameter automation
}

} // namespace MAEVN
