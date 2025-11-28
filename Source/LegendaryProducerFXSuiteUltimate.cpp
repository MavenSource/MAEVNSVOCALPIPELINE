/**
 * @file LegendaryProducerFXSuiteUltimate.cpp
 * @brief Implementation of All-in-One Vocal Processing Plugin
 */

#include "LegendaryProducerFXSuiteUltimate.h"

namespace MAEVN
{

//==============================================================================
// LegendaryProducerFXSuiteUltimateAudioProcessor Implementation
//==============================================================================

LegendaryProducerFXSuiteUltimateAudioProcessor::LegendaryProducerFXSuiteUltimateAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , multibandCompressorEnabled(true)
    , transientShaperEnabled(false)
    , deEsserEnabled(true)
    , saturationEnabled(false)
    , stereoWidenerEnabled(true)
    , limiterEnabled(true)
    , pthVocalCloneEnabled(false)
    , epicSpaceReverbEnabled(true)
    , abComparisonEnabled(false)
    , abComparisonShowProcessed(true)
    , currentSampleRate(44100.0)
    , currentBlockSize(512)
{
    // Initialize DSP modules with default settings
    juce::dsp::ProcessSpec defaultSpec;
    defaultSpec.sampleRate = 44100.0;
    defaultSpec.maximumBlockSize = 512;
    defaultSpec.numChannels = 2;
    
    multibandCompressor.prepare(defaultSpec);
    transientShaper.prepare(defaultSpec);
    deEsser.prepare(defaultSpec);
    saturation.prepare(defaultSpec);
    stereoWidener.prepare(defaultSpec);
    limiter.prepare(defaultSpec);
    pthVocalClone.prepare(defaultSpec);
    epicSpaceReverb.prepare(defaultSpec);

    Logger::log(Logger::Level::Info, "LegendaryProducerFXSuiteUltimate initialized");
}

LegendaryProducerFXSuiteUltimateAudioProcessor::~LegendaryProducerFXSuiteUltimateAudioProcessor()
{
}

//==============================================================================
const juce::String LegendaryProducerFXSuiteUltimateAudioProcessor::getName() const
{
    return "LegendaryProducerFXSuiteUltimate";
}

bool LegendaryProducerFXSuiteUltimateAudioProcessor::acceptsMidi() const
{
    return false;
}

bool LegendaryProducerFXSuiteUltimateAudioProcessor::producesMidi() const
{
    return false;
}

bool LegendaryProducerFXSuiteUltimateAudioProcessor::isMidiEffect() const
{
    return false;
}

double LegendaryProducerFXSuiteUltimateAudioProcessor::getTailLengthSeconds() const
{
    // Account for reverb tail
    return 5.0;
}

int LegendaryProducerFXSuiteUltimateAudioProcessor::getNumPrograms()
{
    return 1;
}

int LegendaryProducerFXSuiteUltimateAudioProcessor::getCurrentProgram()
{
    return 0;
}

void LegendaryProducerFXSuiteUltimateAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String LegendaryProducerFXSuiteUltimateAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void LegendaryProducerFXSuiteUltimateAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void LegendaryProducerFXSuiteUltimateAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());
    
    // Prepare all DSP modules
    multibandCompressor.prepare(spec);
    transientShaper.prepare(spec);
    deEsser.prepare(spec);
    saturation.prepare(spec);
    stereoWidener.prepare(spec);
    limiter.prepare(spec);
    pthVocalClone.prepare(spec);
    epicSpaceReverb.prepare(spec);
    
    Logger::log(Logger::Level::Info, 
        "LegendaryProducerFXSuiteUltimate prepared: " + juce::String(sampleRate) + " Hz, " 
        + juce::String(samplesPerBlock) + " samples");
}

void LegendaryProducerFXSuiteUltimateAudioProcessor::releaseResources()
{
    multibandCompressor.reset();
    transientShaper.reset();
    deEsser.reset();
    saturation.reset();
    stereoWidener.reset();
    limiter.reset();
    pthVocalClone.reset();
    epicSpaceReverb.reset();
}

bool LegendaryProducerFXSuiteUltimateAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Support stereo only
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    return true;
}

void LegendaryProducerFXSuiteUltimateAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, 
                                                                   juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);
    
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
    
    // Measure input level
    inputLevel.store(calculateRMSLevel(buffer));
    
    // Estimate pitch (for metering)
    currentPitch.store(estimatePitch(buffer));
    
    // A/B Comparison - if enabled and showing original, skip processing
    if (abComparisonEnabled && !abComparisonShowProcessed)
    {
        outputLevel.store(calculateRMSLevel(buffer));
        return;
    }
    
    //==========================================================================
    // Vocal FX Processing Chain
    //==========================================================================
    
    // 1. Multiband Compressor
    if (multibandCompressorEnabled)
    {
        multibandCompressor.process(buffer);
    }
    
    // 2. Transient Shaper
    if (transientShaperEnabled)
    {
        transientShaper.process(buffer);
    }
    
    // 3. De-Esser
    if (deEsserEnabled)
    {
        deEsser.process(buffer);
    }
    
    // 4. Saturation
    if (saturationEnabled)
    {
        saturation.process(buffer);
    }
    
    // 5. Stereo Widener
    if (stereoWidenerEnabled)
    {
        stereoWidener.process(buffer);
    }
    
    //==========================================================================
    // PTH Vocal Clone Processing
    //==========================================================================
    
    if (pthVocalCloneEnabled)
    {
        pthVocalClone.process(buffer);
    }
    
    //==========================================================================
    // Epic Space Reverb Processing
    //==========================================================================
    
    if (epicSpaceReverbEnabled)
    {
        epicSpaceReverb.process(buffer);
    }
    
    //==========================================================================
    // Final Stage: Limiter
    //==========================================================================
    
    if (limiterEnabled)
    {
        limiter.process(buffer);
    }
    
    // Measure output level
    outputLevel.store(calculateRMSLevel(buffer));
}

//==============================================================================
bool LegendaryProducerFXSuiteUltimateAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* LegendaryProducerFXSuiteUltimateAudioProcessor::createEditor()
{
    return new LegendaryProducerFXSuiteUltimateAudioProcessorEditor(*this);
}

//==============================================================================
void LegendaryProducerFXSuiteUltimateAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto* obj = new juce::DynamicObject();
    
    // Save enable states
    obj->setProperty("multibandCompressorEnabled", multibandCompressorEnabled);
    obj->setProperty("transientShaperEnabled", transientShaperEnabled);
    obj->setProperty("deEsserEnabled", deEsserEnabled);
    obj->setProperty("saturationEnabled", saturationEnabled);
    obj->setProperty("stereoWidenerEnabled", stereoWidenerEnabled);
    obj->setProperty("limiterEnabled", limiterEnabled);
    obj->setProperty("pthVocalCloneEnabled", pthVocalCloneEnabled);
    obj->setProperty("epicSpaceReverbEnabled", epicSpaceReverbEnabled);
    
    juce::var state(obj);
    juce::String stateString = juce::JSON::toString(state);
    destData.append(stateString.toRawUTF8(), stateString.getNumBytesAsUTF8());
}

void LegendaryProducerFXSuiteUltimateAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::String stateString = juce::String::createStringFromData(data, sizeInBytes);
    juce::var state = juce::JSON::parse(stateString);
    
    if (state.isObject())
    {
        auto* obj = state.getDynamicObject();
        
        if (obj->hasProperty("multibandCompressorEnabled"))
            multibandCompressorEnabled = obj->getProperty("multibandCompressorEnabled");
        if (obj->hasProperty("transientShaperEnabled"))
            transientShaperEnabled = obj->getProperty("transientShaperEnabled");
        if (obj->hasProperty("deEsserEnabled"))
            deEsserEnabled = obj->getProperty("deEsserEnabled");
        if (obj->hasProperty("saturationEnabled"))
            saturationEnabled = obj->getProperty("saturationEnabled");
        if (obj->hasProperty("stereoWidenerEnabled"))
            stereoWidenerEnabled = obj->getProperty("stereoWidenerEnabled");
        if (obj->hasProperty("limiterEnabled"))
            limiterEnabled = obj->getProperty("limiterEnabled");
        if (obj->hasProperty("pthVocalCloneEnabled"))
            pthVocalCloneEnabled = obj->getProperty("pthVocalCloneEnabled");
        if (obj->hasProperty("epicSpaceReverbEnabled"))
            epicSpaceReverbEnabled = obj->getProperty("epicSpaceReverbEnabled");
    }
}

//==============================================================================
void LegendaryProducerFXSuiteUltimateAudioProcessor::savePreset(const juce::String& name)
{
    auto* obj = new juce::DynamicObject();
    
    obj->setProperty("multibandCompressorEnabled", multibandCompressorEnabled);
    obj->setProperty("transientShaperEnabled", transientShaperEnabled);
    obj->setProperty("deEsserEnabled", deEsserEnabled);
    obj->setProperty("saturationEnabled", saturationEnabled);
    obj->setProperty("stereoWidenerEnabled", stereoWidenerEnabled);
    obj->setProperty("limiterEnabled", limiterEnabled);
    obj->setProperty("pthVocalCloneEnabled", pthVocalCloneEnabled);
    obj->setProperty("epicSpaceReverbEnabled", epicSpaceReverbEnabled);
    
    presets[name] = juce::var(obj);
    
    Logger::log(Logger::Level::Info, "Preset saved: " + name);
}

void LegendaryProducerFXSuiteUltimateAudioProcessor::loadPreset(const juce::String& name)
{
    auto it = presets.find(name);
    if (it != presets.end())
    {
        auto* obj = it->second.getDynamicObject();
        if (obj != nullptr)
        {
            if (obj->hasProperty("multibandCompressorEnabled"))
                multibandCompressorEnabled = obj->getProperty("multibandCompressorEnabled");
            if (obj->hasProperty("transientShaperEnabled"))
                transientShaperEnabled = obj->getProperty("transientShaperEnabled");
            if (obj->hasProperty("deEsserEnabled"))
                deEsserEnabled = obj->getProperty("deEsserEnabled");
            if (obj->hasProperty("saturationEnabled"))
                saturationEnabled = obj->getProperty("saturationEnabled");
            if (obj->hasProperty("stereoWidenerEnabled"))
                stereoWidenerEnabled = obj->getProperty("stereoWidenerEnabled");
            if (obj->hasProperty("limiterEnabled"))
                limiterEnabled = obj->getProperty("limiterEnabled");
            if (obj->hasProperty("pthVocalCloneEnabled"))
                pthVocalCloneEnabled = obj->getProperty("pthVocalCloneEnabled");
            if (obj->hasProperty("epicSpaceReverbEnabled"))
                epicSpaceReverbEnabled = obj->getProperty("epicSpaceReverbEnabled");
            
            Logger::log(Logger::Level::Info, "Preset loaded: " + name);
        }
    }
}

juce::StringArray LegendaryProducerFXSuiteUltimateAudioProcessor::getPresetNames() const
{
    juce::StringArray names;
    for (const auto& preset : presets)
    {
        names.add(preset.first);
    }
    return names;
}

//==============================================================================
float LegendaryProducerFXSuiteUltimateAudioProcessor::calculateRMSLevel(const juce::AudioBuffer<float>& buffer)
{
    float sumSquares = 0.0f;
    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();
    
    for (int ch = 0; ch < numChannels; ++ch)
    {
        const float* data = buffer.getReadPointer(ch);
        for (int i = 0; i < numSamples; ++i)
        {
            sumSquares += data[i] * data[i];
        }
    }
    
    return std::sqrt(sumSquares / static_cast<float>(numSamples * numChannels));
}

float LegendaryProducerFXSuiteUltimateAudioProcessor::estimatePitch(const juce::AudioBuffer<float>& buffer)
{
    // Simplified pitch estimation using zero-crossing rate
    // A full implementation would use autocorrelation or FFT
    
    int numSamples = buffer.getNumSamples();
    if (numSamples < 2)
        return 0.0f;
    
    const float* data = buffer.getReadPointer(0);
    int zeroCrossings = 0;
    
    for (int i = 1; i < numSamples; ++i)
    {
        if ((data[i - 1] >= 0.0f && data[i] < 0.0f) ||
            (data[i - 1] < 0.0f && data[i] >= 0.0f))
        {
            zeroCrossings++;
        }
    }
    
    // Estimate frequency from zero-crossing rate
    float estimatedFreq = static_cast<float>(zeroCrossings) * static_cast<float>(currentSampleRate) 
                          / (2.0f * static_cast<float>(numSamples));
    
    return estimatedFreq;
}

//==============================================================================
// LegendaryProducerFXSuiteUltimateAudioProcessorEditor Implementation
//==============================================================================

LegendaryProducerFXSuiteUltimateAudioProcessorEditor::LegendaryProducerFXSuiteUltimateAudioProcessorEditor(
    LegendaryProducerFXSuiteUltimateAudioProcessor& p)
    : AudioProcessorEditor(&p)
    , audioProcessor(p)
{
    setSize(800, 600);
    
    // Setup tabbed component
    addAndMakeVisible(tabbedComponent);
    
    // Create tabs
    auto* vocalFXTab = new juce::Component();
    auto* pthTab = new juce::Component();
    auto* reverbTab = new juce::Component();
    
    tabbedComponent.addTab("Vocal FX", juce::Colours::darkgrey, vocalFXTab, true);
    tabbedComponent.addTab("PTH Vocal Clone", juce::Colours::darkgrey, pthTab, true);
    tabbedComponent.addTab("EpicSpaceReverb", juce::Colours::darkgrey, reverbTab, true);
    
    setupVocalFXTab();
    setupPTHTab();
    setupReverbTab();
    
    // A/B Comparison button
    addAndMakeVisible(abCompareButton);
    abCompareButton.onClick = [this]
    {
        audioProcessor.toggleABComparison();
        abCompareButton.setButtonText(audioProcessor.isShowingProcessed() ? "A/B (Processed)" : "A/B (Original)");
    };
    
    // Metering labels
    addAndMakeVisible(inputLevelLabel);
    inputLevelLabel.setText("Input: 0 dB", juce::dontSendNotification);
    
    addAndMakeVisible(outputLevelLabel);
    outputLevelLabel.setText("Output: 0 dB", juce::dontSendNotification);
    
    // Start meter update timer (30 Hz refresh rate)
    startTimerHz(30);
    
    Logger::log(Logger::Level::Info, "LegendaryProducerFXSuiteUltimate Editor initialized");
}

LegendaryProducerFXSuiteUltimateAudioProcessorEditor::~LegendaryProducerFXSuiteUltimateAudioProcessorEditor()
{
    stopTimer();
}

void LegendaryProducerFXSuiteUltimateAudioProcessorEditor::timerCallback()
{
    updateMeterDisplay();
}

void LegendaryProducerFXSuiteUltimateAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    
    g.setColour(juce::Colours::white);
    g.setFont(24.0f);
    g.drawText("LegendaryProducerFXSuiteUltimate", 
               20, 10, getWidth() - 40, 40, juce::Justification::centred);
}

void LegendaryProducerFXSuiteUltimateAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(50); // Header space
    
    // A/B button and metering at top right
    auto topArea = bounds.removeFromTop(30);
    abCompareButton.setBounds(topArea.removeFromRight(100).reduced(5));
    outputLevelLabel.setBounds(topArea.removeFromRight(120).reduced(5));
    inputLevelLabel.setBounds(topArea.removeFromRight(120).reduced(5));
    
    // Tabbed component takes the rest
    tabbedComponent.setBounds(bounds.reduced(10));
}

void LegendaryProducerFXSuiteUltimateAudioProcessorEditor::setupVocalFXTab()
{
    auto* tab = tabbedComponent.getTabContentComponent(0);
    if (tab == nullptr) return;
    
    // Multiband Compressor toggle
    tab->addAndMakeVisible(multibandCompressorToggle);
    multibandCompressorToggle.setToggleState(audioProcessor.isMultibandCompressorEnabled(), juce::dontSendNotification);
    multibandCompressorToggle.onClick = [this]
    {
        audioProcessor.setMultibandCompressorEnabled(multibandCompressorToggle.getToggleState());
    };
    
    // Transient Shaper toggle
    tab->addAndMakeVisible(transientShaperToggle);
    transientShaperToggle.setToggleState(audioProcessor.isTransientShaperEnabled(), juce::dontSendNotification);
    transientShaperToggle.onClick = [this]
    {
        audioProcessor.setTransientShaperEnabled(transientShaperToggle.getToggleState());
    };
    
    // De-Esser toggle
    tab->addAndMakeVisible(deEsserToggle);
    deEsserToggle.setToggleState(audioProcessor.isDeEsserEnabled(), juce::dontSendNotification);
    deEsserToggle.onClick = [this]
    {
        audioProcessor.setDeEsserEnabled(deEsserToggle.getToggleState());
    };
    
    // Saturation toggle
    tab->addAndMakeVisible(saturationToggle);
    saturationToggle.setToggleState(audioProcessor.isSaturationEnabled(), juce::dontSendNotification);
    saturationToggle.onClick = [this]
    {
        audioProcessor.setSaturationEnabled(saturationToggle.getToggleState());
    };
    
    // Stereo Widener toggle
    tab->addAndMakeVisible(stereoWidenerToggle);
    stereoWidenerToggle.setToggleState(audioProcessor.isStereoWidenerEnabled(), juce::dontSendNotification);
    stereoWidenerToggle.onClick = [this]
    {
        audioProcessor.setStereoWidenerEnabled(stereoWidenerToggle.getToggleState());
    };
    
    // Limiter toggle
    tab->addAndMakeVisible(limiterToggle);
    limiterToggle.setToggleState(audioProcessor.isLimiterEnabled(), juce::dontSendNotification);
    limiterToggle.onClick = [this]
    {
        audioProcessor.setLimiterEnabled(limiterToggle.getToggleState());
    };
    
    // Layout
    int yPos = 20;
    int height = 30;
    int spacing = 10;
    
    multibandCompressorToggle.setBounds(20, yPos, 200, height);
    yPos += height + spacing;
    
    transientShaperToggle.setBounds(20, yPos, 200, height);
    yPos += height + spacing;
    
    deEsserToggle.setBounds(20, yPos, 200, height);
    yPos += height + spacing;
    
    saturationToggle.setBounds(20, yPos, 200, height);
    yPos += height + spacing;
    
    stereoWidenerToggle.setBounds(20, yPos, 200, height);
    yPos += height + spacing;
    
    limiterToggle.setBounds(20, yPos, 200, height);
}

void LegendaryProducerFXSuiteUltimateAudioProcessorEditor::setupPTHTab()
{
    auto* tab = tabbedComponent.getTabContentComponent(1);
    if (tab == nullptr) return;
    
    // PTH Vocal Clone toggle
    tab->addAndMakeVisible(pthVocalCloneToggle);
    pthVocalCloneToggle.setToggleState(audioProcessor.isPTHVocalCloneEnabled(), juce::dontSendNotification);
    pthVocalCloneToggle.onClick = [this]
    {
        audioProcessor.setPTHVocalCloneEnabled(pthVocalCloneToggle.getToggleState());
    };
    
    // Pitch Correction slider
    tab->addAndMakeVisible(pitchCorrectionSlider);
    pitchCorrectionSlider.setRange(-12.0, 12.0, 0.1);
    pitchCorrectionSlider.setValue(0.0);
    pitchCorrectionSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    pitchCorrectionSlider.onValueChange = [this]
    {
        audioProcessor.getPTHVocalClone().setPitchCorrection(static_cast<float>(pitchCorrectionSlider.getValue()));
    };
    
    // Brightness slider
    tab->addAndMakeVisible(brightnessSlider);
    brightnessSlider.setRange(0.0, 1.0, 0.01);
    brightnessSlider.setValue(0.5);
    brightnessSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    brightnessSlider.onValueChange = [this]
    {
        audioProcessor.getPTHVocalClone().setBrightness(static_cast<float>(brightnessSlider.getValue()));
    };
    
    // Formant Shift slider
    tab->addAndMakeVisible(formantShiftSlider);
    formantShiftSlider.setRange(-12.0, 12.0, 0.1);
    formantShiftSlider.setValue(0.0);
    formantShiftSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    formantShiftSlider.onValueChange = [this]
    {
        audioProcessor.getPTHVocalClone().setFormantShift(static_cast<float>(formantShiftSlider.getValue()));
    };
    
    // Layout
    int yPos = 20;
    int height = 30;
    int spacing = 15;
    
    pthVocalCloneToggle.setBounds(20, yPos, 200, height);
    yPos += height + spacing;
    
    pitchLabel.setText("Pitch Correction (semitones):", juce::dontSendNotification);
    tab->addAndMakeVisible(pitchLabel);
    pitchLabel.setBounds(20, yPos, 200, height);
    pitchCorrectionSlider.setBounds(220, yPos, 300, height);
    yPos += height + spacing;
    
    brightnessLabel.setText("Brightness:", juce::dontSendNotification);
    tab->addAndMakeVisible(brightnessLabel);
    brightnessLabel.setBounds(20, yPos, 200, height);
    brightnessSlider.setBounds(220, yPos, 300, height);
    yPos += height + spacing;
    
    formantLabel.setText("Formant Shift (semitones):", juce::dontSendNotification);
    tab->addAndMakeVisible(formantLabel);
    formantLabel.setBounds(20, yPos, 200, height);
    formantShiftSlider.setBounds(220, yPos, 300, height);
}

void LegendaryProducerFXSuiteUltimateAudioProcessorEditor::setupReverbTab()
{
    auto* tab = tabbedComponent.getTabContentComponent(2);
    if (tab == nullptr) return;
    
    // Epic Space Reverb toggle
    tab->addAndMakeVisible(epicSpaceReverbToggle);
    epicSpaceReverbToggle.setToggleState(audioProcessor.isEpicSpaceReverbEnabled(), juce::dontSendNotification);
    epicSpaceReverbToggle.onClick = [this]
    {
        audioProcessor.setEpicSpaceReverbEnabled(epicSpaceReverbToggle.getToggleState());
    };
    
    // Room Size slider
    tab->addAndMakeVisible(roomSizeSlider);
    roomSizeSlider.setRange(0.0, 1.0, 0.01);
    roomSizeSlider.setValue(0.7);
    roomSizeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    roomSizeSlider.onValueChange = [this]
    {
        audioProcessor.getEpicSpaceReverb().setRoomSize(static_cast<float>(roomSizeSlider.getValue()));
    };
    
    // Decay Time slider
    tab->addAndMakeVisible(decayTimeSlider);
    decayTimeSlider.setRange(0.1, 10.0, 0.1);
    decayTimeSlider.setValue(2.5);
    decayTimeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    decayTimeSlider.onValueChange = [this]
    {
        audioProcessor.getEpicSpaceReverb().setDecayTime(static_cast<float>(decayTimeSlider.getValue()));
    };
    
    // Damping slider
    tab->addAndMakeVisible(dampingSlider);
    dampingSlider.setRange(0.0, 1.0, 0.01);
    dampingSlider.setValue(0.5);
    dampingSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    dampingSlider.onValueChange = [this]
    {
        audioProcessor.getEpicSpaceReverb().setDamping(static_cast<float>(dampingSlider.getValue()));
    };
    
    // Pre-Delay slider
    tab->addAndMakeVisible(preDelaySlider);
    preDelaySlider.setRange(0.0, 200.0, 1.0);
    preDelaySlider.setValue(30.0);
    preDelaySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    preDelaySlider.onValueChange = [this]
    {
        audioProcessor.getEpicSpaceReverb().setPreDelay(static_cast<float>(preDelaySlider.getValue()));
    };
    
    // Wet/Dry Mix slider
    tab->addAndMakeVisible(wetDryMixSlider);
    wetDryMixSlider.setRange(0.0, 1.0, 0.01);
    wetDryMixSlider.setValue(0.3);
    wetDryMixSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    wetDryMixSlider.onValueChange = [this]
    {
        audioProcessor.getEpicSpaceReverb().setWetDryMix(static_cast<float>(wetDryMixSlider.getValue()));
    };
    
    // Layout
    int yPos = 20;
    int height = 30;
    int spacing = 15;
    
    epicSpaceReverbToggle.setBounds(20, yPos, 200, height);
    yPos += height + spacing;
    
    roomSizeLabel.setText("Room Size:", juce::dontSendNotification);
    tab->addAndMakeVisible(roomSizeLabel);
    roomSizeLabel.setBounds(20, yPos, 150, height);
    roomSizeSlider.setBounds(170, yPos, 300, height);
    yPos += height + spacing;
    
    decayLabel.setText("Decay Time (s):", juce::dontSendNotification);
    tab->addAndMakeVisible(decayLabel);
    decayLabel.setBounds(20, yPos, 150, height);
    decayTimeSlider.setBounds(170, yPos, 300, height);
    yPos += height + spacing;
    
    dampingLabel.setText("Damping:", juce::dontSendNotification);
    tab->addAndMakeVisible(dampingLabel);
    dampingLabel.setBounds(20, yPos, 150, height);
    dampingSlider.setBounds(170, yPos, 300, height);
    yPos += height + spacing;
    
    preDelayLabel.setText("Pre-Delay (ms):", juce::dontSendNotification);
    tab->addAndMakeVisible(preDelayLabel);
    preDelayLabel.setBounds(20, yPos, 150, height);
    preDelaySlider.setBounds(170, yPos, 300, height);
    yPos += height + spacing;
    
    wetDryLabel.setText("Wet/Dry Mix:", juce::dontSendNotification);
    tab->addAndMakeVisible(wetDryLabel);
    wetDryLabel.setBounds(20, yPos, 150, height);
    wetDryMixSlider.setBounds(170, yPos, 300, height);
}

void LegendaryProducerFXSuiteUltimateAudioProcessorEditor::updateMeterDisplay()
{
    float inputdB = 20.0f * std::log10(audioProcessor.getInputLevel() + 0.0001f);
    float outputdB = 20.0f * std::log10(audioProcessor.getOutputLevel() + 0.0001f);
    
    inputLevelLabel.setText("Input: " + juce::String(inputdB, 1) + " dB", juce::dontSendNotification);
    outputLevelLabel.setText("Output: " + juce::String(outputdB, 1) + " dB", juce::dontSendNotification);
}

} // namespace MAEVN
