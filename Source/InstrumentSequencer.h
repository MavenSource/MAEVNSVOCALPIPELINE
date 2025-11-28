/**
 * @file InstrumentSequencer.h
 * @brief Pattern editor for hi-hat rolls, 808 glides, and other instrument patterns
 * 
 * This module provides a comprehensive pattern sequencer for creating
 * rhythmic patterns, 808 bass glides, hi-hat rolls, and other instrument
 * sequences with step-based editing.
 */

#pragma once

#include <JuceHeader.h>
#include <memory>
#include <vector>
#include <array>
#include "Utilities.h"

namespace MAEVN
{

//==============================================================================
// Sequencer Constants
//==============================================================================
constexpr float MIN_STEP_VELOCITY = 0.1f;   ///< Minimum velocity to ensure audibility
constexpr float MAX_STEP_VELOCITY = 1.0f;   ///< Maximum velocity (full volume)

//==============================================================================
/**
 * @brief Step note data for sequencer
 */
struct SequencerStep
{
    bool active;            ///< Whether this step plays a note
    int noteNumber;         ///< MIDI note number (0-127)
    float velocity;         ///< Note velocity (0.0 - 1.0)
    float pitchBend;        ///< Pitch bend amount (-1.0 to 1.0)
    float slideAmount;      ///< Slide/glide amount (0.0 - 1.0)
    float probability;      ///< Probability of playing (0.0 - 1.0)
    float pan;              ///< Pan position (-1.0 L to 1.0 R)
    int retrigger;          ///< Number of retrigger notes (1-8)
    
    SequencerStep()
        : active(false)
        , noteNumber(60)
        , velocity(0.8f)
        , pitchBend(0.0f)
        , slideAmount(0.0f)
        , probability(1.0f)
        , pan(0.0f)
        , retrigger(1)
    {}
};

//==============================================================================
/**
 * @brief Pattern data containing multiple steps
 */
struct SequencerPattern
{
    juce::String name;
    int numSteps;                              ///< Number of steps (4-64)
    int stepsPerBeat;                          ///< Steps per beat (1-8)
    std::vector<SequencerStep> steps;
    InstrumentType instrumentType;
    bool swing;                                ///< Enable swing
    float swingAmount;                         ///< Swing amount (0.0 - 1.0)
    
    SequencerPattern()
        : name("New Pattern")
        , numSteps(16)
        , stepsPerBeat(4)
        , instrumentType(InstrumentType::Unknown)
        , swing(false)
        , swingAmount(0.5f)
    {
        steps.resize(64);
    }
    
    double getStepDuration(double bpm) const
    {
        return 60.0 / (bpm * stepsPerBeat);
    }
};

//==============================================================================
/**
 * @brief Hi-hat roll pattern generator
 */
class HiHatRollGenerator
{
public:
    HiHatRollGenerator();
    ~HiHatRollGenerator();
    
    /**
     * @brief Generate a trap-style hi-hat pattern
     * @param numSteps Number of steps in the pattern
     * @param density Density of hits (0.0 - 1.0)
     * @param rollProbability Probability of rolls (0.0 - 1.0)
     * @return Generated pattern
     */
    SequencerPattern generateTrapPattern(int numSteps, float density, float rollProbability);
    
    /**
     * @brief Generate a hi-hat roll (rapid repeats)
     * @param numSteps Length of roll
     * @param startVelocity Starting velocity
     * @param endVelocity Ending velocity
     * @param accentPattern Accent pattern (e.g., "1010" for alternating)
     * @return Generated roll pattern
     */
    SequencerPattern generateRoll(int numSteps, float startVelocity, 
                                  float endVelocity, const juce::String& accentPattern);
    
    /**
     * @brief Generate triplet hi-hat pattern
     * @param numBeats Number of beats
     * @return Generated triplet pattern
     */
    SequencerPattern generateTriplets(int numBeats);
    
    /**
     * @brief Generate open/closed hi-hat pattern
     * @param numSteps Number of steps
     * @param openPattern Binary pattern for open hits (e.g., "0010" = open on beat 4)
     * @return Generated pattern
     */
    SequencerPattern generateOpenClosed(int numSteps, const juce::String& openPattern);
    
private:
    juce::Random random;
    
    void applyVelocityVariation(SequencerPattern& pattern, float variation);
};

//==============================================================================
/**
 * @brief 808 bass glide pattern generator
 */
class Bass808GlideGenerator
{
public:
    Bass808GlideGenerator();
    ~Bass808GlideGenerator();
    
    /**
     * @brief Generate a simple bass pattern with glides
     * @param numSteps Number of steps
     * @param rootNote Root MIDI note
     * @param glideTime Glide time in milliseconds
     * @return Generated pattern
     */
    SequencerPattern generateSimpleBass(int numSteps, int rootNote, float glideTime);
    
    /**
     * @brief Generate 808 pattern with pitch slides
     * @param numSteps Number of steps
     * @param noteSequence Sequence of note offsets from root
     * @param glidePattern Which steps should glide (binary pattern)
     * @return Generated pattern
     */
    SequencerPattern generateGlidePattern(int numSteps, 
                                          const std::vector<int>& noteSequence,
                                          const juce::String& glidePattern);
    
    /**
     * @brief Generate trap-style 808 pattern
     * @param numSteps Number of steps
     * @param rootNote Root MIDI note
     * @return Generated pattern
     */
    SequencerPattern generateTrap808(int numSteps, int rootNote);
    
    /**
     * @brief Generate sub-bass pattern
     * @param numSteps Number of steps
     * @param rootNote Root MIDI note
     * @param octaveDropPattern Pattern for octave drops
     * @return Generated pattern
     */
    SequencerPattern generateSubBass(int numSteps, int rootNote, 
                                     const juce::String& octaveDropPattern);
    
private:
    juce::Random random;
};

//==============================================================================
/**
 * @brief Main Instrument Sequencer class
 */
class InstrumentSequencer
{
public:
    InstrumentSequencer();
    ~InstrumentSequencer();
    
    /**
     * @brief Set the current pattern
     * @param pattern The pattern to use
     */
    void setPattern(const SequencerPattern& pattern);
    
    /**
     * @brief Get the current pattern
     */
    const SequencerPattern& getPattern() const { return currentPattern; }
    
    /**
     * @brief Get modifiable pattern reference
     */
    SequencerPattern& getPatternRef() { return currentPattern; }
    
    /**
     * @brief Set step data
     * @param stepIndex Index of the step (0-based)
     * @param step The step data
     */
    void setStep(int stepIndex, const SequencerStep& step);
    
    /**
     * @brief Get step data
     * @param stepIndex Index of the step
     * @return The step data
     */
    SequencerStep getStep(int stepIndex) const;
    
    /**
     * @brief Toggle step on/off
     * @param stepIndex Index of the step
     */
    void toggleStep(int stepIndex);
    
    /**
     * @brief Clear all steps
     */
    void clearPattern();
    
    /**
     * @brief Set BPM for playback
     * @param bpm Tempo in beats per minute
     */
    void setBPM(double bpm);
    
    /**
     * @brief Get current BPM
     */
    double getBPM() const { return currentBPM; }
    
    /**
     * @brief Start playback
     */
    void start();
    
    /**
     * @brief Stop playback
     */
    void stop();
    
    /**
     * @brief Check if playing
     */
    bool isPlaying() const { return playing; }
    
    /**
     * @brief Process a block of samples
     * @param midiBuffer MIDI buffer to fill with events
     * @param numSamples Number of samples in the block
     * @param sampleRate Current sample rate
     */
    void processBlock(juce::MidiBuffer& midiBuffer, int numSamples, double sampleRate);
    
    /**
     * @brief Get current step position
     */
    int getCurrentStep() const { return currentStep; }
    
    /**
     * @brief Set instrument type
     * @param type The instrument type
     */
    void setInstrumentType(InstrumentType type);
    
    /**
     * @brief Get instrument type
     */
    InstrumentType getInstrumentType() const { return instrumentType; }
    
    /**
     * @brief Get hi-hat roll generator
     */
    HiHatRollGenerator& getHiHatGenerator() { return hiHatGenerator; }
    
    /**
     * @brief Get 808 glide generator
     */
    Bass808GlideGenerator& get808Generator() { return bass808Generator; }
    
    /**
     * @brief Save pattern to JSON
     * @return JSON representation of the pattern
     */
    juce::var savePatternToJSON() const;
    
    /**
     * @brief Load pattern from JSON
     * @param json JSON data
     * @return true if loaded successfully
     */
    bool loadPatternFromJSON(const juce::var& json);
    
    /**
     * @brief Get all preset patterns for an instrument type
     * @param type The instrument type
     * @return Array of preset patterns
     */
    std::vector<SequencerPattern> getPresetPatterns(InstrumentType type) const;
    
    /**
     * @brief Apply humanization to pattern
     * @param velocityVariation Velocity variation amount (0.0 - 1.0)
     * @param timingVariation Timing variation in milliseconds
     */
    void humanize(float velocityVariation, float timingVariation);
    
    /**
     * @brief Shift pattern left/right
     * @param steps Number of steps to shift (negative = left)
     */
    void shiftPattern(int steps);
    
    /**
     * @brief Reverse the pattern
     */
    void reversePattern();
    
    /**
     * @brief Double the pattern length
     */
    void doublePattern();
    
    /**
     * @brief Halve the pattern length
     */
    void halvePattern();
    
private:
    SequencerPattern currentPattern;
    double currentBPM;
    bool playing;
    int currentStep;
    double samplePosition;
    double samplesPerStep;
    InstrumentType instrumentType;
    
    HiHatRollGenerator hiHatGenerator;
    Bass808GlideGenerator bass808Generator;
    
    juce::Random random;
    juce::CriticalSection sequencerLock;
    
    // For pitch glide processing
    float currentPitchBend;
    float targetPitchBend;
    float pitchBendSmoothingFactor;
    
    /**
     * @brief Trigger a step
     */
    void triggerStep(int step, juce::MidiBuffer& midiBuffer, int sampleOffset);
    
    /**
     * @brief Update pitch bend smoothing
     */
    void updatePitchBend(juce::MidiBuffer& midiBuffer, int sampleOffset);
    
    /**
     * @brief Calculate sample position for step
     */
    double getStepSamplePosition(int step, double sampleRate) const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InstrumentSequencer)
};

//==============================================================================
/**
 * @brief Sequencer grid UI component
 */
class SequencerGridComponent : public juce::Component
{
public:
    SequencerGridComponent(InstrumentSequencer* sequencer);
    ~SequencerGridComponent() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    
    /**
     * @brief Set the number of visible steps
     * @param steps Number of steps to display
     */
    void setVisibleSteps(int steps);
    
    /**
     * @brief Set the number of rows (for velocity/pitch display)
     * @param rows Number of rows
     */
    void setNumRows(int rows);
    
    /**
     * @brief Refresh the display
     */
    void refresh();
    
private:
    InstrumentSequencer* sequencer;
    int visibleSteps;
    int numRows;
    int hoveredStep;
    int selectedStep;
    
    juce::Rectangle<float> getStepBounds(int step, int row) const;
    void updateStepFromMouse(const juce::MouseEvent& event);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SequencerGridComponent)
};

} // namespace MAEVN
