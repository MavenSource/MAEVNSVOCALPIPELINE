/**
 * @file DragDropExport.cpp
 * @brief Implementation of Drag-to-MIDI/Audio functionality
 */

#include "DragDropExport.h"
#include <array>

namespace MAEVN
{

//==============================================================================
// DragDropExport Implementation
//==============================================================================

DragDropExport::DragDropExport()
    : sampleRate(44100.0)
    , bitDepth(24)
    , bpm(120.0)
{
    // Create temp export directory
    tempExportDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                        .getChildFile("MAEVN_Export");
    tempExportDir.createDirectory();
    
    Logger::log(Logger::Level::Info, "DragDropExport initialized with temp dir: " + 
                tempExportDir.getFullPathName());
}

DragDropExport::~DragDropExport()
{
    cleanupTempFiles();
}

bool DragDropExport::startBlockDrag(const TimelineBlock& block,
                                    const juce::AudioBuffer<float>* audioData,
                                    juce::Component* sourceComponent)
{
    if (sourceComponent == nullptr)
        return false;
    
    // Create exportable region
    ExportableRegion region;
    region.name = blockTypeToString(block.type) + "_" + juce::String(block.startTime, 1);
    region.blockType = block.type;
    region.startTime = block.startTime;
    region.duration = block.duration;
    region.content = block.content;
    region.trackIndex = block.trackIndex;
    region.sampleRate = sampleRate;
    
    if (audioData != nullptr)
    {
        region.audioBuffer.makeCopyOf(*audioData);
    }
    
    notifyExportStarted(region);
    
    // Create temporary audio file for drag
    juce::File tempFile = tempExportDir.getChildFile(region.name + ".wav");
    
    bool success = false;
    if (audioData != nullptr && audioData->getNumSamples() > 0)
    {
        // Export audio data to temp file
        success = createAudioFile(block, ExportFormat::WAV, tempFile);
    }
    else
    {
        // Create MIDI file if no audio
        tempFile = tempExportDir.getChildFile(region.name + ".mid");
        success = createMIDIFile(block, tempFile);
    }
    
    if (success)
    {
        // Create drag image
        juce::Image dragImage = createDragImage(block);
        
        // Start the drag with the file
        juce::StringArray files;
        files.add(tempFile.getFullPathName());
        
        performExternalDragDropOfFiles(files, true, sourceComponent, [this, region](void)
        {
            notifyExportCompleted(region, true);
        });
        
        return true;
    }
    
    notifyExportCompleted(region, false);
    return false;
}

bool DragDropExport::startMultiBlockDrag(const std::vector<TimelineBlock>& blocks,
                                         const juce::AudioBuffer<float>* audioData,
                                         juce::Component* sourceComponent)
{
    if (blocks.empty() || sourceComponent == nullptr)
        return false;
    
    // Create combined region name
    juce::String combinedName = "MAEVN_Export_";
    combinedName += juce::String(blocks.size()) + "_blocks";
    
    // Create temp file
    juce::File tempFile = tempExportDir.getChildFile(combinedName + ".wav");
    
    // For multi-block export, we need to combine the audio
    if (audioData != nullptr && audioData->getNumSamples() > 0)
    {
        // Write combined audio to file
        juce::WavAudioFormat wavFormat;
        std::unique_ptr<juce::AudioFormatWriter> writer(
            wavFormat.createWriterFor(
                new juce::FileOutputStream(tempFile),
                sampleRate,
                static_cast<unsigned int>(audioData->getNumChannels()),
                bitDepth,
                {},
                0
            )
        );
        
        if (writer != nullptr)
        {
            writer->writeFromAudioSampleBuffer(*audioData, 0, audioData->getNumSamples());
        }
    }
    
    // Start drag
    juce::StringArray files;
    files.add(tempFile.getFullPathName());
    
    ExportableRegion region;
    region.name = combinedName;
    
    performExternalDragDropOfFiles(files, true, sourceComponent, [this, region](void)
    {
        notifyExportCompleted(region, true);
    });
    
    return true;
}

bool DragDropExport::exportBlock(const TimelineBlock& block,
                                 ExportFormat format,
                                 const juce::File& destinationFile)
{
    ExportableRegion region;
    region.name = destinationFile.getFileNameWithoutExtension();
    region.blockType = block.type;
    region.startTime = block.startTime;
    region.duration = block.duration;
    region.content = block.content;
    region.trackIndex = block.trackIndex;
    
    notifyExportStarted(region);
    
    bool success = false;
    
    if (format == ExportFormat::MIDI)
    {
        success = createMIDIFile(block, destinationFile);
    }
    else
    {
        success = createAudioFile(block, format, destinationFile);
    }
    
    notifyExportCompleted(region, success);
    return success;
}

bool DragDropExport::exportBlocks(const std::vector<TimelineBlock>& blocks,
                                  ExportFormat format,
                                  const juce::File& destinationFile)
{
    if (blocks.empty())
        return false;
    
    // For now, export the first block only
    // In a full implementation, this would combine all blocks
    return exportBlock(blocks[0], format, destinationFile);
}

juce::String DragDropExport::getFileExtension(ExportFormat format)
{
    switch (format)
    {
        case ExportFormat::MIDI: return ".mid";
        case ExportFormat::WAV:  return ".wav";
        case ExportFormat::AIFF: return ".aiff";
        case ExportFormat::MP3:  return ".mp3";
        case ExportFormat::OGG:  return ".ogg";
        default: return ".wav";
    }
}

juce::Image DragDropExport::createDragImage(const TimelineBlock& block)
{
    // Create a visual representation of the block for dragging
    int width = 150;
    int height = 40;
    
    juce::Image image(juce::Image::ARGB, width, height, true);
    juce::Graphics g(image);
    
    // Background color based on block type
    juce::Colour bgColor;
    switch (block.type)
    {
        case BlockType::Hook:
        case BlockType::Intro:
        case BlockType::Verse:
        case BlockType::Bridge:
        case BlockType::Outro:
        case BlockType::Vocal:
            bgColor = juce::Colour(100, 150, 255);
            break;
        case BlockType::Drum_808:
            bgColor = juce::Colour(255, 100, 100);
            break;
        case BlockType::Drum_HiHat:
        case BlockType::Drum_Snare:
            bgColor = juce::Colour(255, 180, 100);
            break;
        case BlockType::Instrument_Piano:
        case BlockType::Instrument_Synth:
            bgColor = juce::Colour(100, 255, 150);
            break;
        default:
            bgColor = juce::Colour(180, 180, 180);
            break;
    }
    
    g.setColour(bgColor.withAlpha(0.8f));
    g.fillRoundedRectangle(0.0f, 0.0f, static_cast<float>(width), 
                          static_cast<float>(height), 5.0f);
    
    g.setColour(juce::Colours::white);
    g.setFont(14.0f);
    g.drawText("[" + blockTypeToString(block.type) + "]", 
               5, 5, width - 10, 15, juce::Justification::centredLeft);
    
    g.setFont(11.0f);
    g.drawText(juce::String(block.duration, 1) + "s", 
               5, 22, width - 10, 13, juce::Justification::centredLeft);
    
    return image;
}

juce::File DragDropExport::getTempExportDirectory() const
{
    return tempExportDir;
}

void DragDropExport::cleanupTempFiles()
{
    if (tempExportDir.isDirectory())
    {
        juce::Array<juce::File> files;
        tempExportDir.findChildFiles(files, juce::File::findFiles, false);
        
        for (auto& file : files)
        {
            file.deleteFile();
        }
    }
}

void DragDropExport::addListener(DragDropExportListener* listener)
{
    listeners.push_back(listener);
}

void DragDropExport::removeListener(DragDropExportListener* listener)
{
    listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
}

bool DragDropExport::createMIDIFile(const TimelineBlock& block, const juce::File& file)
{
    juce::MidiFile midiFile;
    midiFile.setTicksPerQuarterNote(480);
    
    juce::MidiMessageSequence sequence = createMIDISequence(block);
    
    midiFile.addTrack(sequence);
    
    juce::FileOutputStream outputStream(file);
    if (outputStream.openedOk())
    {
        return midiFile.writeTo(outputStream);
    }
    
    return false;
}

bool DragDropExport::createAudioFile(const TimelineBlock& block,
                                     ExportFormat format,
                                     const juce::File& file)
{
    // Create a silent audio buffer as placeholder
    // In a full implementation, this would use the rendered audio from the block
    int numSamples = static_cast<int>(block.duration * sampleRate);
    juce::AudioBuffer<float> buffer(2, numSamples);
    buffer.clear();
    
    // Create appropriate audio format
    std::unique_ptr<juce::AudioFormat> audioFormat;
    
    switch (format)
    {
        case ExportFormat::WAV:
            audioFormat = std::make_unique<juce::WavAudioFormat>();
            break;
        case ExportFormat::AIFF:
            audioFormat = std::make_unique<juce::AiffAudioFormat>();
            break;
        default:
            audioFormat = std::make_unique<juce::WavAudioFormat>();
            break;
    }
    
    std::unique_ptr<juce::AudioFormatWriter> writer(
        audioFormat->createWriterFor(
            new juce::FileOutputStream(file),
            sampleRate,
            static_cast<unsigned int>(buffer.getNumChannels()),
            bitDepth,
            {},
            0
        )
    );
    
    if (writer != nullptr)
    {
        return writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
    }
    
    return false;
}

juce::MidiMessageSequence DragDropExport::createMIDISequence(const TimelineBlock& block)
{
    juce::MidiMessageSequence sequence;
    
    // Calculate tick positions
    double ticksPerBeat = 480.0;
    double beatsPerSecond = bpm / 60.0;
    
    double startTick = block.startTime * beatsPerSecond * ticksPerBeat;
    double endTick = (block.startTime + block.duration) * beatsPerSecond * ticksPerBeat;
    
    // Create appropriate MIDI messages based on block type
    switch (block.type)
    {
        case BlockType::Drum_808:
        {
            // Create 808 bass notes
            int noteNumber = 36; // C1 - typical 808 kick
            for (double tick = startTick; tick < endTick; tick += ticksPerBeat * 2)
            {
                sequence.addEvent(juce::MidiMessage::noteOn(10, noteNumber, 0.8f), tick);
                sequence.addEvent(juce::MidiMessage::noteOff(10, noteNumber, 0.0f), tick + ticksPerBeat);
            }
            break;
        }
        
        case BlockType::Drum_HiHat:
        {
            // Create hi-hat pattern
            int noteNumber = 42; // Closed hi-hat
            for (double tick = startTick; tick < endTick; tick += ticksPerBeat / 2)
            {
                float velocity = (static_cast<int>(tick) % static_cast<int>(ticksPerBeat) == 0) ? 0.9f : 0.6f;
                sequence.addEvent(juce::MidiMessage::noteOn(10, noteNumber, velocity), tick);
                sequence.addEvent(juce::MidiMessage::noteOff(10, noteNumber, 0.0f), tick + ticksPerBeat / 4);
            }
            break;
        }
        
        case BlockType::Drum_Snare:
        {
            // Create snare on 2 and 4
            int noteNumber = 38; // Snare drum
            for (double tick = startTick + ticksPerBeat; tick < endTick; tick += ticksPerBeat * 2)
            {
                sequence.addEvent(juce::MidiMessage::noteOn(10, noteNumber, 0.85f), tick);
                sequence.addEvent(juce::MidiMessage::noteOff(10, noteNumber, 0.0f), tick + ticksPerBeat / 2);
            }
            break;
        }
        
        case BlockType::Instrument_Piano:
        case BlockType::Instrument_Synth:
        {
            // Create a simple chord progression
            int baseNote = 60; // Middle C
            std::array<int, 3> chordNotes = { 0, 4, 7 }; // Major triad
            
            for (double tick = startTick; tick < endTick; tick += ticksPerBeat * 4)
            {
                for (int interval : chordNotes)
                {
                    sequence.addEvent(juce::MidiMessage::noteOn(1, baseNote + interval, 0.7f), tick);
                    sequence.addEvent(juce::MidiMessage::noteOff(1, baseNote + interval, 0.0f), 
                                     tick + ticksPerBeat * 3.5);
                }
            }
            break;
        }
        
        default:
        {
            // For vocal blocks, create a placeholder note
            sequence.addEvent(juce::MidiMessage::noteOn(1, 60, 0.7f), startTick);
            sequence.addEvent(juce::MidiMessage::noteOff(1, 60, 0.0f), endTick);
            break;
        }
    }
    
    sequence.updateMatchedPairs();
    return sequence;
}

void DragDropExport::notifyExportStarted(const ExportableRegion& region)
{
    for (auto* listener : listeners)
    {
        listener->onExportStarted(region);
    }
}

void DragDropExport::notifyExportCompleted(const ExportableRegion& region, bool success)
{
    for (auto* listener : listeners)
    {
        listener->onExportCompleted(region, success);
    }
}

void DragDropExport::notifyProgress(float progress)
{
    for (auto* listener : listeners)
    {
        listener->onExportProgress(progress);
    }
}

//==============================================================================
// DraggableBlockComponent Implementation
//==============================================================================

DraggableBlockComponent::DraggableBlockComponent(const TimelineBlock& blk, DragDropExport* exporter)
    : block(blk)
    , dragExporter(exporter)
    , hasAudioData(false)
    , isDragging(false)
{
    setMouseCursor(juce::MouseCursor::DraggingHandCursor);
}

DraggableBlockComponent::~DraggableBlockComponent()
{
}

void DraggableBlockComponent::paint(juce::Graphics& g)
{
    // Background based on block type
    juce::Colour bgColor;
    switch (block.type)
    {
        case BlockType::Hook:
        case BlockType::Intro:
        case BlockType::Verse:
        case BlockType::Bridge:
        case BlockType::Outro:
        case BlockType::Vocal:
            bgColor = juce::Colour(70, 130, 180);
            break;
        case BlockType::Drum_808:
            bgColor = juce::Colour(178, 34, 34);
            break;
        case BlockType::Drum_HiHat:
        case BlockType::Drum_Snare:
            bgColor = juce::Colour(210, 105, 30);
            break;
        case BlockType::Instrument_Piano:
        case BlockType::Instrument_Synth:
            bgColor = juce::Colour(34, 139, 34);
            break;
        default:
            bgColor = juce::Colour(128, 128, 128);
            break;
    }
    
    if (isDragging)
        bgColor = bgColor.brighter(0.3f);
    
    g.setColour(bgColor);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);
    
    g.setColour(bgColor.brighter(0.2f));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1.0f), 4.0f, 2.0f);
    
    // Block label
    g.setColour(juce::Colours::white);
    g.setFont(12.0f);
    g.drawText("[" + blockTypeToString(block.type) + "]",
               getLocalBounds().reduced(5, 2),
               juce::Justification::centredLeft);
    
    // Duration indicator
    g.setFont(10.0f);
    g.drawText(juce::String(block.duration, 1) + "s",
               getLocalBounds().reduced(5, 2),
               juce::Justification::centredRight);
    
    // Audio indicator
    if (hasAudioData)
    {
        g.setColour(juce::Colours::limegreen);
        g.fillEllipse(getWidth() - 15.0f, 5.0f, 8.0f, 8.0f);
    }
}

void DraggableBlockComponent::mouseDown(const juce::MouseEvent& event)
{
    if (event.mods.isLeftButtonDown())
    {
        isDragging = true;
        repaint();
    }
}

void DraggableBlockComponent::mouseDrag(const juce::MouseEvent& event)
{
    if (isDragging && event.getDistanceFromDragStart() > 5)
    {
        // Start the drag operation
        if (dragExporter != nullptr)
        {
            const juce::AudioBuffer<float>* audioPtr = hasAudioData ? &renderedAudio : nullptr;
            dragExporter->startBlockDrag(block, audioPtr, this);
        }
        isDragging = false;
        repaint();
    }
}

void DraggableBlockComponent::setAudioData(const juce::AudioBuffer<float>& audioData)
{
    renderedAudio.makeCopyOf(audioData);
    hasAudioData = true;
    repaint();
}

} // namespace MAEVN
