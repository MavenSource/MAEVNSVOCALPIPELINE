/**
 * @file PluginProcessor.cpp
 * @brief Implementation of main audio processor
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace MAEVN
{

//==============================================================================
MAEVNAudioProcessor::MAEVNAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , aiFXEngine(&onnxEngine)
    , currentSampleRate(44100.0)
    , currentBlockSize(512)
    , cinematicEnhancerEnabled(true)
{
    // Initialize ONNX engine
    onnxEngine.initialize();
    
    // Load models and presets
    initializeModelsAndPresets();
    
    // Apply default cinematic vocal preset
    cinematicEnhancer.applyCinematicVocalPreset();
    
    Logger::log(Logger::Level::Info, "MAEVN Audio Processor initialized with Cinematic Enhancer");
}

MAEVNAudioProcessor::~MAEVNAudioProcessor()
{
}

//==============================================================================
const juce::String MAEVNAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MAEVNAudioProcessor::acceptsMidi() const
{
    return true;
}

bool MAEVNAudioProcessor::producesMidi() const
{
    return false;
}

bool MAEVNAudioProcessor::isMidiEffect() const
{
    return false;
}

double MAEVNAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MAEVNAudioProcessor::getNumPrograms()
{
    return 1;
}

int MAEVNAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MAEVNAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String MAEVNAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void MAEVNAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void MAEVNAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;
    
    // Prepare AI FX engine
    aiFXEngine.prepare(sampleRate, samplesPerBlock);
    
    // Prepare Cinematic Audio Enhancer
    cinematicEnhancer.prepare(sampleRate, samplesPerBlock);
    
    // Allocate track buffers
    for (auto& buffer : trackBuffers)
    {
        buffer.setSize(2, samplesPerBlock);
    }
    
    Logger::log(Logger::Level::Info, 
        "Prepared to play: " + juce::String(sampleRate) + " Hz, " + juce::String(samplesPerBlock) + " samples");
}

void MAEVNAudioProcessor::releaseResources()
{
    aiFXEngine.reset();
    cinematicEnhancer.reset();
}

bool MAEVNAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Support stereo only
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    return true;
}

void MAEVNAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    int numSamples = buffer.getNumSamples();
    
    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, numSamples);
    
    // Update transport information
    updateTransportInfo();
    
    // Process all tracks with FX
    processAllTracks(buffer, numSamples);
    
    // Apply Cinematic Audio Enhancement (final processing stage)
    if (cinematicEnhancerEnabled)
    {
        cinematicEnhancer.process(buffer, numSamples);
    }
    
    juce::ignoreUnused(midiMessages);
}

//==============================================================================
bool MAEVNAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* MAEVNAudioProcessor::createEditor()
{
    return new MAEVNAudioProcessorEditor(*this);
}

//==============================================================================
void MAEVNAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Save plugin state
    auto* obj = new juce::DynamicObject();
    
    obj->setProperty("bpm", patternEngine.getBPM());
    obj->setProperty("cinematicEnhancerEnabled", cinematicEnhancerEnabled);
    
    // Save FX modes for each track
    juce::Array<juce::var> fxModes;
    for (int i = 0; i < aiFXEngine.getNumTracks(); ++i)
    {
        fxModes.add(static_cast<int>(aiFXEngine.getFXMode(i)));
    }
    obj->setProperty("fxModes", fxModes);
    
    juce::var state(obj);
    juce::String stateString = juce::JSON::toString(state);
    destData.append(stateString.toRawUTF8(), stateString.getNumBytesAsUTF8());
}

void MAEVNAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Restore plugin state
    juce::String stateString = juce::String::createStringFromData(data, sizeInBytes);
    juce::var state = juce::JSON::parse(stateString);
    
    if (state.isObject())
    {
        auto* obj = state.getDynamicObject();
        
        if (obj->hasProperty("bpm"))
        {
            patternEngine.setBPM(obj->getProperty("bpm"));
        }
        
        if (obj->hasProperty("cinematicEnhancerEnabled"))
        {
            cinematicEnhancerEnabled = obj->getProperty("cinematicEnhancerEnabled");
        }
        
        if (obj->hasProperty("fxModes"))
        {
            auto fxModesArray = obj->getProperty("fxModes").getArray();
            for (int i = 0; i < fxModesArray->size() && i < aiFXEngine.getNumTracks(); ++i)
            {
                int modeInt = fxModesArray->getUnchecked(i);
                aiFXEngine.setFXMode(i, static_cast<FXMode>(modeInt));
            }
        }
    }
}

//==============================================================================
void MAEVNAudioProcessor::initializeModelsAndPresets()
{
    // Get plugin directory
    juce::File pluginFile = juce::File::getSpecialLocation(
        juce::File::currentApplicationFile);
    juce::File pluginDir = pluginFile.getParentDirectory();
    
    // Try to load models
    juce::File modelsDir = pluginDir.getChildFile("Models");
    if (modelsDir.isDirectory())
    {
        juce::File configFile = modelsDir.getChildFile("config.json");
        if (configFile.existsAsFile())
        {
            onnxEngine.loadModelsFromConfig(configFile.getFullPathName());
        }
    }
    
    // Try to load presets
    juce::File presetsDir = pluginDir.getChildFile("Presets");
    if (presetsDir.isDirectory())
    {
        presetManager.loadPresetsFromDirectory(presetsDir);
    }
}

void MAEVNAudioProcessor::processAllTracks(juce::AudioBuffer<float>& buffer, int numSamples)
{
    // For now, just apply FX to main buffer
    // In full implementation, would route to different tracks
    
    // Apply FX for each active track
    // This is a simplified version - full version would handle routing
    for (int trackIndex = 0; trackIndex < aiFXEngine.getNumTracks(); ++trackIndex)
    {
        if (aiFXEngine.getFXMode(trackIndex) != FXMode::Off)
        {
            aiFXEngine.process(buffer, numSamples, trackIndex);
        }
    }
}

void MAEVNAudioProcessor::updateTransportInfo()
{
    auto playHead = getPlayHead();
    if (playHead != nullptr)
    {
        juce::AudioPlayHead::CurrentPositionInfo posInfo;
        if (playHead->getCurrentPosition(posInfo))
        {
            // Update pattern engine with transport info
            patternEngine.updateTransport(posInfo.isPlaying, posInfo.timeInSeconds);
            
            // Update BPM if it changed
            if (posInfo.bpm > 0.0)
            {
                if (std::abs(posInfo.bpm - patternEngine.getBPM()) > 0.1)
                {
                    patternEngine.setBPM(posInfo.bpm);
                }
            }
        }
    }
}

} // namespace MAEVN

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MAEVN::MAEVNAudioProcessor();
}
