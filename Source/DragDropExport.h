/**
 * @file DragDropExport.h
 * @brief Drag-to-MIDI/Audio functionality for exporting blocks to DAW timeline
 * 
 * This module enables users to drag [HOOK], [VERSE], and other blocks
 * directly from the plugin timeline into the DAW timeline, either as
 * MIDI clips or rendered audio files.
 */

#pragma once

#include <JuceHeader.h>
#include <memory>
#include <vector>
#include "Utilities.h"
#include "PatternEngine.h"

namespace MAEVN
{

//==============================================================================
/**
 * @brief Export format for drag/drop operations
 */
enum class ExportFormat
{
    MIDI,           ///< Export as MIDI file
    WAV,            ///< Export as WAV audio file
    AIFF,           ///< Export as AIFF audio file
    MP3,            ///< Export as MP3 audio file (if available)
    OGG             ///< Export as OGG audio file
};

//==============================================================================
/**
 * @brief Represents an exportable audio/MIDI region
 */
struct ExportableRegion
{
    juce::String name;              ///< Region name (e.g., "HOOK_1")
    BlockType blockType;            ///< Type of block
    double startTime;               ///< Start time in seconds
    double duration;                ///< Duration in seconds
    juce::String content;           ///< Text content or parameters
    int trackIndex;                 ///< Source track index
    
    // Audio data (if rendered)
    juce::AudioBuffer<float> audioBuffer;
    double sampleRate;
    
    // MIDI data (if applicable)
    juce::MidiMessageSequence midiSequence;
    
    ExportableRegion()
        : blockType(BlockType::Unknown)
        , startTime(0.0)
        , duration(1.0)
        , trackIndex(0)
        , sampleRate(44100.0)
    {}
};

//==============================================================================
/**
 * @brief Listener interface for drag/drop export events
 */
class DragDropExportListener
{
public:
    virtual ~DragDropExportListener() = default;
    
    /**
     * @brief Called when export starts
     * @param region The region being exported
     */
    virtual void onExportStarted(const ExportableRegion& region) = 0;
    
    /**
     * @brief Called when export completes
     * @param region The exported region
     * @param success Whether export was successful
     */
    virtual void onExportCompleted(const ExportableRegion& region, bool success) = 0;
    
    /**
     * @brief Called to report export progress
     * @param progress Progress value (0.0 - 1.0)
     */
    virtual void onExportProgress(float progress) = 0;
};

//==============================================================================
/**
 * @brief Main class for handling drag-to-MIDI/Audio export
 * 
 * Allows users to drag timeline blocks directly into DAW timeline,
 * automatically rendering audio or creating MIDI files as needed.
 */
class DragDropExport : public juce::DragAndDropContainer
{
public:
    DragDropExport();
    ~DragDropExport();
    
    /**
     * @brief Start a drag operation for a timeline block
     * @param block The block to drag
     * @param audioData Optional pre-rendered audio data
     * @param sourceComponent The component initiating the drag
     * @return true if drag started successfully
     */
    bool startBlockDrag(const TimelineBlock& block, 
                       const juce::AudioBuffer<float>* audioData,
                       juce::Component* sourceComponent);
    
    /**
     * @brief Start a drag operation for multiple blocks
     * @param blocks The blocks to drag
     * @param audioData Combined audio data for all blocks
     * @param sourceComponent The component initiating the drag
     * @return true if drag started successfully
     */
    bool startMultiBlockDrag(const std::vector<TimelineBlock>& blocks,
                            const juce::AudioBuffer<float>* audioData,
                            juce::Component* sourceComponent);
    
    /**
     * @brief Export a block to a specific format
     * @param block The block to export
     * @param format The export format
     * @param destinationFile The file to export to
     * @return true if export successful
     */
    bool exportBlock(const TimelineBlock& block,
                    ExportFormat format,
                    const juce::File& destinationFile);
    
    /**
     * @brief Export multiple blocks as a single file
     * @param blocks The blocks to export
     * @param format The export format
     * @param destinationFile The file to export to
     * @return true if export successful
     */
    bool exportBlocks(const std::vector<TimelineBlock>& blocks,
                     ExportFormat format,
                     const juce::File& destinationFile);
    
    /**
     * @brief Set the sample rate for audio export
     * @param sampleRate Sample rate in Hz
     */
    void setSampleRate(double sampleRate) { this->sampleRate = sampleRate; }
    
    /**
     * @brief Set the bit depth for audio export
     * @param bitDepth Bit depth (16, 24, or 32)
     */
    void setBitDepth(int bitDepth) { this->bitDepth = bitDepth; }
    
    /**
     * @brief Set the BPM for MIDI export
     * @param bpm Tempo in beats per minute
     */
    void setBPM(double bpm) { this->bpm = bpm; }
    
    /**
     * @brief Add an export listener
     * @param listener The listener to add
     */
    void addListener(DragDropExportListener* listener);
    
    /**
     * @brief Remove an export listener
     * @param listener The listener to remove
     */
    void removeListener(DragDropExportListener* listener);
    
    /**
     * @brief Get the file extension for a format
     * @param format The export format
     * @return File extension string (e.g., ".wav")
     */
    static juce::String getFileExtension(ExportFormat format);
    
    /**
     * @brief Create a drag image for a block
     * @param block The block to create an image for
     * @return The drag image
     */
    static juce::Image createDragImage(const TimelineBlock& block);
    
    /**
     * @brief Get the temporary export directory
     * @return Path to temp export directory
     */
    juce::File getTempExportDirectory() const;
    
    /**
     * @brief Clean up temporary export files
     */
    void cleanupTempFiles();
    
private:
    double sampleRate;
    int bitDepth;
    double bpm;
    
    std::vector<DragDropExportListener*> listeners;
    juce::File tempExportDir;
    
    /**
     * @brief Create MIDI file from block
     */
    bool createMIDIFile(const TimelineBlock& block, const juce::File& file);
    
    /**
     * @brief Create audio file from block
     */
    bool createAudioFile(const TimelineBlock& block, 
                        ExportFormat format,
                        const juce::File& file);
    
    /**
     * @brief Create MIDI sequence from block content
     */
    juce::MidiMessageSequence createMIDISequence(const TimelineBlock& block);
    
    /**
     * @brief Notify listeners of export start
     */
    void notifyExportStarted(const ExportableRegion& region);
    
    /**
     * @brief Notify listeners of export completion
     */
    void notifyExportCompleted(const ExportableRegion& region, bool success);
    
    /**
     * @brief Notify listeners of progress
     */
    void notifyProgress(float progress);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DragDropExport)
};

//==============================================================================
/**
 * @brief Draggable timeline block component
 * 
 * A UI component that can be dragged from the plugin into the DAW
 */
class DraggableBlockComponent : public juce::Component,
                                 public juce::DragAndDropContainer
{
public:
    DraggableBlockComponent(const TimelineBlock& block, DragDropExport* exporter);
    ~DraggableBlockComponent() override;
    
    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    
    /**
     * @brief Set the audio data for this block
     * @param audioData The rendered audio buffer
     */
    void setAudioData(const juce::AudioBuffer<float>& audioData);
    
    /**
     * @brief Get the timeline block
     */
    const TimelineBlock& getBlock() const { return block; }
    
private:
    TimelineBlock block;
    DragDropExport* dragExporter;
    juce::AudioBuffer<float> renderedAudio;
    bool hasAudioData;
    bool isDragging;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DraggableBlockComponent)
};

} // namespace MAEVN
