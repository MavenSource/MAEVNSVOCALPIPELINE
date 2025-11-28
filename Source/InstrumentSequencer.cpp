/**
 * @file InstrumentSequencer.cpp
 * @brief Implementation of Pattern Editor / Instrument Sequencer
 */

#include "InstrumentSequencer.h"

namespace MAEVN
{

//==============================================================================
// HiHatRollGenerator Implementation
//==============================================================================

HiHatRollGenerator::HiHatRollGenerator()
{
}

HiHatRollGenerator::~HiHatRollGenerator()
{
}

SequencerPattern HiHatRollGenerator::generateTrapPattern(int numSteps, float density, 
                                                         float rollProbability)
{
    SequencerPattern pattern;
    pattern.name = "Trap Hi-Hat";
    pattern.numSteps = numSteps;
    pattern.stepsPerBeat = 4;
    pattern.instrumentType = InstrumentType::HiHat;
    pattern.steps.resize(numSteps);
    
    for (int i = 0; i < numSteps; ++i)
    {
        auto& step = pattern.steps[i];
        
        // Basic pattern - every step has a chance based on density
        bool isOnBeat = (i % 4 == 0);
        bool isOnHalfBeat = (i % 2 == 0);
        
        float probability = density;
        if (isOnBeat) probability = 0.95f;
        else if (isOnHalfBeat) probability = 0.8f;
        
        step.active = (random.nextFloat() < probability);
        step.noteNumber = 42; // Closed hi-hat
        
        // Velocity based on position
        if (isOnBeat)
            step.velocity = 0.9f + random.nextFloat() * 0.1f;
        else if (isOnHalfBeat)
            step.velocity = 0.7f + random.nextFloat() * 0.1f;
        else
            step.velocity = 0.5f + random.nextFloat() * 0.2f;
        
        // Add rolls randomly
        if (random.nextFloat() < rollProbability && !isOnBeat)
        {
            step.retrigger = 2 + random.nextInt(3); // 2-4 retriggers
        }
        else
        {
            step.retrigger = 1;
        }
    }
    
    applyVelocityVariation(pattern, 0.1f);
    
    return pattern;
}

SequencerPattern HiHatRollGenerator::generateRoll(int numSteps, float startVelocity,
                                                  float endVelocity, 
                                                  const juce::String& accentPattern)
{
    SequencerPattern pattern;
    pattern.name = "Hi-Hat Roll";
    pattern.numSteps = numSteps;
    pattern.stepsPerBeat = 8; // Fast roll
    pattern.instrumentType = InstrumentType::HiHat;
    pattern.steps.resize(numSteps);
    
    for (int i = 0; i < numSteps; ++i)
    {
        auto& step = pattern.steps[i];
        step.active = true;
        step.noteNumber = 42;
        
        // Interpolate velocity
        float t = static_cast<float>(i) / static_cast<float>(numSteps - 1);
        step.velocity = startVelocity + t * (endVelocity - startVelocity);
        
        // Apply accent pattern
        if (accentPattern.isNotEmpty())
        {
            int patternIndex = i % accentPattern.length();
            if (accentPattern[patternIndex] == '1')
            {
                step.velocity = juce::jmin(1.0f, step.velocity + 0.2f);
            }
        }
    }
    
    return pattern;
}

SequencerPattern HiHatRollGenerator::generateTriplets(int numBeats)
{
    SequencerPattern pattern;
    pattern.name = "Triplet Hi-Hat";
    pattern.numSteps = numBeats * 3;
    pattern.stepsPerBeat = 3;
    pattern.instrumentType = InstrumentType::HiHat;
    pattern.steps.resize(pattern.numSteps);
    
    for (int i = 0; i < pattern.numSteps; ++i)
    {
        auto& step = pattern.steps[i];
        step.active = true;
        step.noteNumber = 42;
        
        // Accent first of each triplet
        if (i % 3 == 0)
            step.velocity = 0.9f;
        else
            step.velocity = 0.6f;
    }
    
    applyVelocityVariation(pattern, 0.05f);
    
    return pattern;
}

SequencerPattern HiHatRollGenerator::generateOpenClosed(int numSteps, 
                                                        const juce::String& openPattern)
{
    SequencerPattern pattern;
    pattern.name = "Open/Closed Hi-Hat";
    pattern.numSteps = numSteps;
    pattern.stepsPerBeat = 4;
    pattern.instrumentType = InstrumentType::HiHat;
    pattern.steps.resize(numSteps);
    
    for (int i = 0; i < numSteps; ++i)
    {
        auto& step = pattern.steps[i];
        step.active = true;
        
        // Check if this step should be open
        int patternIndex = i % openPattern.length();
        bool isOpen = openPattern.isNotEmpty() && openPattern[patternIndex] == '1';
        
        step.noteNumber = isOpen ? 46 : 42; // Open or closed hi-hat
        step.velocity = isOpen ? 0.85f : 0.7f;
    }
    
    applyVelocityVariation(pattern, 0.08f);
    
    return pattern;
}

void HiHatRollGenerator::applyVelocityVariation(SequencerPattern& pattern, float variation)
{
    for (auto& step : pattern.steps)
    {
        if (step.active)
        {
            float delta = (random.nextFloat() - 0.5f) * 2.0f * variation;
            step.velocity = clamp(step.velocity + delta, MIN_STEP_VELOCITY, MAX_STEP_VELOCITY);
        }
    }
}

//==============================================================================
// Bass808GlideGenerator Implementation
//==============================================================================

Bass808GlideGenerator::Bass808GlideGenerator()
{
}

Bass808GlideGenerator::~Bass808GlideGenerator()
{
}

SequencerPattern Bass808GlideGenerator::generateSimpleBass(int numSteps, int rootNote, 
                                                            float glideTime)
{
    SequencerPattern pattern;
    pattern.name = "Simple 808";
    pattern.numSteps = numSteps;
    pattern.stepsPerBeat = 4;
    pattern.instrumentType = InstrumentType::Bass808;
    pattern.steps.resize(numSteps);
    
    // Simple pattern: hit on beat 1 and 3
    for (int i = 0; i < numSteps; ++i)
    {
        auto& step = pattern.steps[i];
        step.noteNumber = rootNote;
        
        if (i % 8 == 0 || i % 8 == 4)
        {
            step.active = true;
            step.velocity = 0.9f;
            step.slideAmount = glideTime / 1000.0f; // Convert ms to 0-1 range
        }
    }
    
    return pattern;
}

SequencerPattern Bass808GlideGenerator::generateGlidePattern(int numSteps,
                                                              const std::vector<int>& noteSequence,
                                                              const juce::String& glidePattern)
{
    SequencerPattern pattern;
    pattern.name = "808 Glide";
    pattern.numSteps = numSteps;
    pattern.stepsPerBeat = 4;
    pattern.instrumentType = InstrumentType::Bass808;
    pattern.steps.resize(numSteps);
    
    int baseNote = 36; // C1
    
    for (int i = 0; i < numSteps; ++i)
    {
        auto& step = pattern.steps[i];
        
        // Get note from sequence
        int noteIdx = i % noteSequence.size();
        step.noteNumber = baseNote + noteSequence[noteIdx];
        step.active = true;
        step.velocity = 0.85f;
        
        // Check if should glide
        int glideIdx = i % glidePattern.length();
        if (glidePattern.isNotEmpty() && glidePattern[glideIdx] == '1')
        {
            step.slideAmount = 0.5f;
        }
    }
    
    return pattern;
}

SequencerPattern Bass808GlideGenerator::generateTrap808(int numSteps, int rootNote)
{
    SequencerPattern pattern;
    pattern.name = "Trap 808";
    pattern.numSteps = numSteps;
    pattern.stepsPerBeat = 4;
    pattern.instrumentType = InstrumentType::Bass808;
    pattern.steps.resize(numSteps);
    
    // Common trap 808 rhythm
    std::vector<int> hitPositions = { 0, 3, 6, 8, 11, 14 };
    std::vector<int> noteOffsets = { 0, 0, 5, 0, 0, 7 };
    std::vector<bool> glides = { false, false, true, false, false, true };
    
    for (int i = 0; i < numSteps; ++i)
    {
        auto& step = pattern.steps[i];
        step.active = false;
        
        for (size_t j = 0; j < hitPositions.size(); ++j)
        {
            if (i % 16 == hitPositions[j])
            {
                step.active = true;
                step.noteNumber = rootNote + noteOffsets[j % noteOffsets.size()];
                step.velocity = 0.88f + random.nextFloat() * 0.12f;
                step.slideAmount = glides[j % glides.size()] ? 0.6f : 0.0f;
                break;
            }
        }
    }
    
    return pattern;
}

SequencerPattern Bass808GlideGenerator::generateSubBass(int numSteps, int rootNote,
                                                         const juce::String& octaveDropPattern)
{
    SequencerPattern pattern;
    pattern.name = "Sub Bass";
    pattern.numSteps = numSteps;
    pattern.stepsPerBeat = 4;
    pattern.instrumentType = InstrumentType::Bass808;
    pattern.steps.resize(numSteps);
    
    for (int i = 0; i < numSteps; ++i)
    {
        auto& step = pattern.steps[i];
        
        if (i % 4 == 0)
        {
            step.active = true;
            step.velocity = 0.9f;
            
            // Check for octave drop
            int patternIdx = (i / 4) % octaveDropPattern.length();
            if (octaveDropPattern.isNotEmpty() && octaveDropPattern[patternIdx] == '1')
            {
                step.noteNumber = rootNote - 12; // Drop an octave
                step.slideAmount = 0.7f;
            }
            else
            {
                step.noteNumber = rootNote;
                step.slideAmount = 0.0f;
            }
        }
    }
    
    return pattern;
}

//==============================================================================
// InstrumentSequencer Implementation
//==============================================================================

InstrumentSequencer::InstrumentSequencer()
    : currentBPM(120.0)
    , playing(false)
    , currentStep(0)
    , samplePosition(0.0)
    , samplesPerStep(0.0)
    , instrumentType(InstrumentType::Unknown)
    , currentPitchBend(0.0f)
    , targetPitchBend(0.0f)
    , pitchBendSmoothingFactor(0.995f)
{
    currentPattern.numSteps = 16;
    currentPattern.steps.resize(64);
    
    Logger::log(Logger::Level::Info, "InstrumentSequencer initialized");
}

InstrumentSequencer::~InstrumentSequencer()
{
}

void InstrumentSequencer::setPattern(const SequencerPattern& pattern)
{
    const juce::ScopedLock sl(sequencerLock);
    currentPattern = pattern;
    instrumentType = pattern.instrumentType;
}

void InstrumentSequencer::setStep(int stepIndex, const SequencerStep& step)
{
    const juce::ScopedLock sl(sequencerLock);
    
    if (stepIndex >= 0 && stepIndex < static_cast<int>(currentPattern.steps.size()))
    {
        currentPattern.steps[stepIndex] = step;
    }
}

SequencerStep InstrumentSequencer::getStep(int stepIndex) const
{
    if (stepIndex >= 0 && stepIndex < static_cast<int>(currentPattern.steps.size()))
    {
        return currentPattern.steps[stepIndex];
    }
    return SequencerStep();
}

void InstrumentSequencer::toggleStep(int stepIndex)
{
    const juce::ScopedLock sl(sequencerLock);
    
    if (stepIndex >= 0 && stepIndex < static_cast<int>(currentPattern.steps.size()))
    {
        currentPattern.steps[stepIndex].active = !currentPattern.steps[stepIndex].active;
    }
}

void InstrumentSequencer::clearPattern()
{
    const juce::ScopedLock sl(sequencerLock);
    
    for (auto& step : currentPattern.steps)
    {
        step.active = false;
    }
}

void InstrumentSequencer::setBPM(double bpm)
{
    const juce::ScopedLock sl(sequencerLock);
    currentBPM = clamp(bpm, 20.0, 300.0);
}

void InstrumentSequencer::start()
{
    const juce::ScopedLock sl(sequencerLock);
    playing = true;
    currentStep = 0;
    samplePosition = 0.0;
}

void InstrumentSequencer::stop()
{
    const juce::ScopedLock sl(sequencerLock);
    playing = false;
    currentStep = 0;
    samplePosition = 0.0;
}

void InstrumentSequencer::processBlock(juce::MidiBuffer& midiBuffer, int numSamples, 
                                        double sampleRate)
{
    if (!playing)
        return;
    
    const juce::ScopedLock sl(sequencerLock);
    
    // Calculate samples per step
    double stepDuration = currentPattern.getStepDuration(currentBPM);
    samplesPerStep = stepDuration * sampleRate;
    
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Check if we've reached the next step
        if (samplePosition >= samplesPerStep)
        {
            samplePosition -= samplesPerStep;
            currentStep = (currentStep + 1) % currentPattern.numSteps;
        }
        
        // Trigger step at the beginning
        if (samplePosition < 1.0)
        {
            const auto& step = currentPattern.steps[currentStep];
            if (step.active)
            {
                // Check probability
                if (random.nextFloat() <= step.probability)
                {
                    triggerStep(currentStep, midiBuffer, sample);
                }
            }
        }
        
        // Update pitch bend smoothing
        updatePitchBend(midiBuffer, sample);
        
        samplePosition += 1.0;
    }
}

void InstrumentSequencer::setInstrumentType(InstrumentType type)
{
    const juce::ScopedLock sl(sequencerLock);
    instrumentType = type;
    currentPattern.instrumentType = type;
}

juce::var InstrumentSequencer::savePatternToJSON() const
{
    auto* obj = new juce::DynamicObject();
    
    obj->setProperty("name", currentPattern.name);
    obj->setProperty("numSteps", currentPattern.numSteps);
    obj->setProperty("stepsPerBeat", currentPattern.stepsPerBeat);
    obj->setProperty("swing", currentPattern.swing);
    obj->setProperty("swingAmount", currentPattern.swingAmount);
    
    juce::Array<juce::var> stepsArray;
    for (int i = 0; i < currentPattern.numSteps; ++i)
    {
        const auto& step = currentPattern.steps[i];
        
        auto* stepObj = new juce::DynamicObject();
        stepObj->setProperty("active", step.active);
        stepObj->setProperty("noteNumber", step.noteNumber);
        stepObj->setProperty("velocity", step.velocity);
        stepObj->setProperty("pitchBend", step.pitchBend);
        stepObj->setProperty("slideAmount", step.slideAmount);
        stepObj->setProperty("probability", step.probability);
        stepObj->setProperty("pan", step.pan);
        stepObj->setProperty("retrigger", step.retrigger);
        
        stepsArray.add(juce::var(stepObj));
    }
    
    obj->setProperty("steps", stepsArray);
    
    return juce::var(obj);
}

bool InstrumentSequencer::loadPatternFromJSON(const juce::var& json)
{
    if (!json.isObject())
        return false;
    
    const juce::ScopedLock sl(sequencerLock);
    
    auto* obj = json.getDynamicObject();
    if (obj == nullptr)
        return false;
    
    currentPattern.name = obj->getProperty("name").toString();
    currentPattern.numSteps = obj->getProperty("numSteps");
    currentPattern.stepsPerBeat = obj->getProperty("stepsPerBeat");
    currentPattern.swing = obj->getProperty("swing");
    currentPattern.swingAmount = obj->getProperty("swingAmount");
    
    auto stepsArray = obj->getProperty("steps").getArray();
    if (stepsArray != nullptr)
    {
        currentPattern.steps.resize(stepsArray->size());
        
        for (int i = 0; i < stepsArray->size(); ++i)
        {
            auto stepVar = stepsArray->getUnchecked(i);
            if (auto* stepObj = stepVar.getDynamicObject())
            {
                auto& step = currentPattern.steps[i];
                step.active = stepObj->getProperty("active");
                step.noteNumber = stepObj->getProperty("noteNumber");
                step.velocity = stepObj->getProperty("velocity");
                step.pitchBend = stepObj->getProperty("pitchBend");
                step.slideAmount = stepObj->getProperty("slideAmount");
                step.probability = stepObj->getProperty("probability");
                step.pan = stepObj->getProperty("pan");
                step.retrigger = stepObj->getProperty("retrigger");
            }
        }
    }
    
    return true;
}

std::vector<SequencerPattern> InstrumentSequencer::getPresetPatterns(InstrumentType type) const
{
    std::vector<SequencerPattern> presets;
    
    switch (type)
    {
        case InstrumentType::HiHat:
        {
            HiHatRollGenerator gen;
            presets.push_back(gen.generateTrapPattern(16, 0.7f, 0.3f));
            presets.push_back(gen.generateRoll(16, 0.5f, 1.0f, "1010"));
            presets.push_back(gen.generateTriplets(4));
            presets.push_back(gen.generateOpenClosed(16, "0010"));
            break;
        }
        
        case InstrumentType::Bass808:
        {
            Bass808GlideGenerator gen;
            presets.push_back(gen.generateSimpleBass(16, 36, 100.0f));
            presets.push_back(gen.generateTrap808(16, 36));
            presets.push_back(gen.generateSubBass(16, 36, "0100"));
            break;
        }
        
        default:
            break;
    }
    
    return presets;
}

void InstrumentSequencer::humanize(float velocityVariation, float timingVariation)
{
    const juce::ScopedLock sl(sequencerLock);
    
    for (auto& step : currentPattern.steps)
    {
        if (step.active)
        {
            // Velocity variation
            float velDelta = (random.nextFloat() - 0.5f) * 2.0f * velocityVariation;
            step.velocity = clamp(step.velocity + velDelta, 0.1f, 1.0f);
            
            // Probability variation for timing feel
            float probDelta = (random.nextFloat() - 0.5f) * 0.1f;
            step.probability = clamp(step.probability + probDelta, 0.5f, 1.0f);
        }
    }
}

void InstrumentSequencer::shiftPattern(int steps)
{
    const juce::ScopedLock sl(sequencerLock);
    
    if (steps == 0 || currentPattern.numSteps == 0)
        return;
    
    std::vector<SequencerStep> temp(currentPattern.numSteps);
    
    for (int i = 0; i < currentPattern.numSteps; ++i)
    {
        int newIndex = (i + steps) % currentPattern.numSteps;
        if (newIndex < 0) newIndex += currentPattern.numSteps;
        temp[newIndex] = currentPattern.steps[i];
    }
    
    for (int i = 0; i < currentPattern.numSteps; ++i)
    {
        currentPattern.steps[i] = temp[i];
    }
}

void InstrumentSequencer::reversePattern()
{
    const juce::ScopedLock sl(sequencerLock);
    
    for (int i = 0; i < currentPattern.numSteps / 2; ++i)
    {
        int j = currentPattern.numSteps - 1 - i;
        std::swap(currentPattern.steps[i], currentPattern.steps[j]);
    }
}

void InstrumentSequencer::doublePattern()
{
    const juce::ScopedLock sl(sequencerLock);
    
    int originalLength = currentPattern.numSteps;
    int newLength = std::min(originalLength * 2, 64);
    
    currentPattern.steps.resize(newLength);
    currentPattern.numSteps = newLength;
    
    // Copy the pattern
    for (int i = originalLength; i < newLength; ++i)
    {
        currentPattern.steps[i] = currentPattern.steps[i - originalLength];
    }
}

void InstrumentSequencer::halvePattern()
{
    const juce::ScopedLock sl(sequencerLock);
    
    if (currentPattern.numSteps > 4)
    {
        currentPattern.numSteps /= 2;
    }
}

void InstrumentSequencer::triggerStep(int step, juce::MidiBuffer& midiBuffer, int sampleOffset)
{
    const auto& stepData = currentPattern.steps[step];
    
    int channel = 1;
    int noteNumber = stepData.noteNumber;
    float velocity = stepData.velocity;
    
    // Handle retriggers
    int numTriggers = stepData.retrigger;
    if (numTriggers < 1) numTriggers = 1;
    if (numTriggers > 8) numTriggers = 8;
    
    double samplesPerRetrigger = samplesPerStep / numTriggers;
    
    for (int t = 0; t < numTriggers; ++t)
    {
        int triggerOffset = sampleOffset + static_cast<int>(t * samplesPerRetrigger);
        
        // Decrease velocity for retriggers
        float triggerVelocity = velocity * (1.0f - 0.1f * t);
        int midiVelocity = static_cast<int>(triggerVelocity * 127);
        midiVelocity = clamp(midiVelocity, 1, 127);
        
        midiBuffer.addEvent(
            juce::MidiMessage::noteOn(channel, noteNumber, static_cast<juce::uint8>(midiVelocity)),
            triggerOffset
        );
        
        // Note off after short duration
        int noteOffOffset = triggerOffset + static_cast<int>(samplesPerRetrigger * 0.8);
        midiBuffer.addEvent(
            juce::MidiMessage::noteOff(channel, noteNumber),
            noteOffOffset
        );
    }
    
    // Set target pitch bend for glide
    if (stepData.slideAmount > 0.0f)
    {
        targetPitchBend = stepData.pitchBend;
    }
}

void InstrumentSequencer::updatePitchBend(juce::MidiBuffer& midiBuffer, int sampleOffset)
{
    // Smooth pitch bend
    currentPitchBend = currentPitchBend * pitchBendSmoothingFactor + 
                       targetPitchBend * (1.0f - pitchBendSmoothingFactor);
    
    // Convert to MIDI pitch bend
    int bendValue = static_cast<int>((currentPitchBend + 1.0f) * 8192.0f);
    bendValue = clamp(bendValue, 0, 16383);
    
    if (bendValue != 8192) // Only send if not centered
    {
        midiBuffer.addEvent(
            juce::MidiMessage::pitchWheel(1, bendValue),
            sampleOffset
        );
    }
}

double InstrumentSequencer::getStepSamplePosition(int step, double sampleRate) const
{
    double stepDuration = currentPattern.getStepDuration(currentBPM);
    return step * stepDuration * sampleRate;
}

//==============================================================================
// SequencerGridComponent Implementation
//==============================================================================

SequencerGridComponent::SequencerGridComponent(InstrumentSequencer* seq)
    : sequencer(seq)
    , visibleSteps(16)
    , numRows(1)
    , hoveredStep(-1)
    , selectedStep(-1)
{
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

SequencerGridComponent::~SequencerGridComponent()
{
}

void SequencerGridComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Background
    g.setColour(juce::Colour(40, 40, 45));
    g.fillRoundedRectangle(bounds, 4.0f);
    
    // Draw grid
    float stepWidth = bounds.getWidth() / static_cast<float>(visibleSteps);
    float rowHeight = bounds.getHeight() / static_cast<float>(numRows);
    
    for (int step = 0; step < visibleSteps; ++step)
    {
        for (int row = 0; row < numRows; ++row)
        {
            auto stepBounds = getStepBounds(step, row);
            
            // Background color based on beat
            juce::Colour bgColor;
            if (step % 4 == 0)
                bgColor = juce::Colour(60, 60, 65);
            else if (step % 2 == 0)
                bgColor = juce::Colour(50, 50, 55);
            else
                bgColor = juce::Colour(45, 45, 50);
            
            g.setColour(bgColor);
            g.fillRoundedRectangle(stepBounds.reduced(1), 2.0f);
            
            // Draw step if active
            if (sequencer != nullptr)
            {
                auto stepData = sequencer->getStep(step);
                if (stepData.active)
                {
                    // Color based on velocity
                    float hue = 0.55f; // Blue-ish
                    float saturation = 0.7f;
                    float brightness = 0.3f + stepData.velocity * 0.7f;
                    
                    g.setColour(juce::Colour::fromHSV(hue, saturation, brightness, 1.0f));
                    g.fillRoundedRectangle(stepBounds.reduced(3), 2.0f);
                    
                    // Retrigger indicator
                    if (stepData.retrigger > 1)
                    {
                        g.setColour(juce::Colours::white.withAlpha(0.7f));
                        g.setFont(8.0f);
                        g.drawText(juce::String(stepData.retrigger), stepBounds.reduced(2),
                                  juce::Justification::bottomRight);
                    }
                }
                
                // Current step indicator
                if (sequencer->isPlaying() && step == sequencer->getCurrentStep())
                {
                    g.setColour(juce::Colours::yellow.withAlpha(0.5f));
                    g.drawRoundedRectangle(stepBounds.reduced(2), 2.0f, 2.0f);
                }
            }
            
            // Hover effect
            if (step == hoveredStep)
            {
                g.setColour(juce::Colours::white.withAlpha(0.2f));
                g.fillRoundedRectangle(stepBounds.reduced(1), 2.0f);
            }
            
            // Selected step
            if (step == selectedStep)
            {
                g.setColour(juce::Colours::white);
                g.drawRoundedRectangle(stepBounds.reduced(2), 2.0f, 1.5f);
            }
        }
    }
    
    // Beat markers
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    for (int i = 0; i < visibleSteps; i += 4)
    {
        float x = i * stepWidth;
        g.drawLine(x, 0, x, bounds.getHeight(), 1.0f);
    }
}

void SequencerGridComponent::resized()
{
    repaint();
}

void SequencerGridComponent::mouseDown(const juce::MouseEvent& event)
{
    updateStepFromMouse(event);
    
    if (selectedStep >= 0 && sequencer != nullptr)
    {
        if (event.mods.isRightButtonDown())
        {
            // Right click - open context menu
            // Future: Show step editor popup
        }
        else
        {
            // Left click - toggle step
            sequencer->toggleStep(selectedStep);
            repaint();
        }
    }
}

void SequencerGridComponent::mouseDrag(const juce::MouseEvent& event)
{
    int previousStep = selectedStep;
    updateStepFromMouse(event);
    
    if (selectedStep != previousStep && selectedStep >= 0 && sequencer != nullptr)
    {
        auto step = sequencer->getStep(selectedStep);
        if (!step.active)
        {
            step.active = true;
            sequencer->setStep(selectedStep, step);
            repaint();
        }
    }
}

void SequencerGridComponent::setVisibleSteps(int steps)
{
    visibleSteps = clamp(steps, 4, 64);
    repaint();
}

void SequencerGridComponent::setNumRows(int rows)
{
    numRows = clamp(rows, 1, 8);
    repaint();
}

void SequencerGridComponent::refresh()
{
    repaint();
}

juce::Rectangle<float> SequencerGridComponent::getStepBounds(int step, int row) const
{
    auto bounds = getLocalBounds().toFloat();
    float stepWidth = bounds.getWidth() / static_cast<float>(visibleSteps);
    float rowHeight = bounds.getHeight() / static_cast<float>(numRows);
    
    return juce::Rectangle<float>(
        step * stepWidth,
        row * rowHeight,
        stepWidth,
        rowHeight
    );
}

void SequencerGridComponent::updateStepFromMouse(const juce::MouseEvent& event)
{
    auto bounds = getLocalBounds().toFloat();
    float stepWidth = bounds.getWidth() / static_cast<float>(visibleSteps);
    
    int step = static_cast<int>(event.position.x / stepWidth);
    step = clamp(step, 0, visibleSteps - 1);
    
    selectedStep = step;
    hoveredStep = step;
}

} // namespace MAEVN
