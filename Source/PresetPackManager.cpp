/**
 * @file PresetPackManager.cpp
 * @brief Implementation of Community Preset Packs functionality
 */

#include "PresetPackManager.h"

namespace MAEVN
{

//==============================================================================
// PresetPack Implementation
//==============================================================================

PresetPack::PresetPack()
{
}

PresetPack::~PresetPack()
{
}

void PresetPack::setInfo(const PresetPackInfo& info)
{
    packInfo = info;
    packInfo.numPresets = static_cast<int>(fxPresets.size());
    packInfo.numPatterns = static_cast<int>(sequencerPatterns.size());
}

void PresetPack::addFXPreset(const FXPreset& preset)
{
    fxPresets.push_back(preset);
    packInfo.numPresets = static_cast<int>(fxPresets.size());
}

void PresetPack::addSequencerPattern(const SequencerPattern& pattern)
{
    sequencerPatterns.push_back(pattern);
    packInfo.numPatterns = static_cast<int>(sequencerPatterns.size());
}

void PresetPack::removeFXPreset(int index)
{
    if (index >= 0 && index < static_cast<int>(fxPresets.size()))
    {
        fxPresets.erase(fxPresets.begin() + index);
        packInfo.numPresets = static_cast<int>(fxPresets.size());
    }
}

void PresetPack::removeSequencerPattern(int index)
{
    if (index >= 0 && index < static_cast<int>(sequencerPatterns.size()))
    {
        sequencerPatterns.erase(sequencerPatterns.begin() + index);
        packInfo.numPatterns = static_cast<int>(sequencerPatterns.size());
    }
}

void PresetPack::clear()
{
    fxPresets.clear();
    sequencerPatterns.clear();
    packInfo = PresetPackInfo();
}

bool PresetPack::saveToFile(const juce::File& file) const
{
    // Create JSON representation
    juce::var jsonData = toJSON();
    juce::String jsonString = juce::JSON::toString(jsonData, true);
    
    // Compress the data
    juce::MemoryBlock uncompressedData;
    uncompressedData.append(jsonString.toRawUTF8(), jsonString.getNumBytesAsUTF8());
    
    juce::MemoryBlock compressedData;
    juce::MemoryOutputStream compressedStream(compressedData, false);
    
    {
        juce::GZIPCompressorOutputStream gzip(compressedStream, 9);
        gzip.write(uncompressedData.getData(), uncompressedData.getSize());
    }
    
    // Write file with header
    juce::FileOutputStream fileStream(file);
    if (!fileStream.openedOk())
        return false;
    
    // Write magic number
    fileStream.writeInt(PACK_MAGIC);
    fileStream.writeInt(PACK_VERSION);
    fileStream.writeInt64(static_cast<int64_t>(compressedData.getSize()));
    
    // Write compressed data
    fileStream.write(compressedData.getData(), compressedData.getSize());
    
    return true;
}

bool PresetPack::loadFromFile(const juce::File& file)
{
    juce::FileInputStream fileStream(file);
    if (!fileStream.openedOk())
        return false;
    
    // Read and verify header
    juce::uint32 magic = fileStream.readInt();
    if (magic != PACK_MAGIC)
    {
        // Try loading as plain JSON (for backward compatibility)
        juce::String jsonString = file.loadFileAsString();
        juce::var json = juce::JSON::parse(jsonString);
        return fromJSON(json);
    }
    
    juce::uint32 version = fileStream.readInt();
    if (version > PACK_VERSION)
    {
        Logger::log(Logger::Level::Warning, "Pack version newer than supported");
    }
    
    int64_t compressedSize = fileStream.readInt64();
    
    // Read compressed data
    juce::MemoryBlock compressedData(static_cast<size_t>(compressedSize));
    fileStream.read(compressedData.getData(), static_cast<int>(compressedSize));
    
    // Decompress
    juce::MemoryInputStream compressedStream(compressedData, false);
    juce::GZIPDecompressorInputStream gzip(compressedStream);
    
    juce::String jsonString = gzip.readEntireStreamAsString();
    juce::var json = juce::JSON::parse(jsonString);
    
    return fromJSON(json);
}

juce::var PresetPack::toJSON() const
{
    auto* obj = new juce::DynamicObject();
    
    // Pack info
    auto* infoObj = new juce::DynamicObject();
    infoObj->setProperty("name", packInfo.name);
    infoObj->setProperty("author", packInfo.author);
    infoObj->setProperty("description", packInfo.description);
    infoObj->setProperty("version", packInfo.version);
    infoObj->setProperty("createdDate", packInfo.createdDate.toMilliseconds());
    infoObj->setProperty("modifiedDate", packInfo.modifiedDate.toMilliseconds());
    infoObj->setProperty("license", packInfo.license);
    infoObj->setProperty("website", packInfo.website);
    
    juce::Array<juce::var> tagsArray;
    for (const auto& tag : packInfo.tags)
        tagsArray.add(tag);
    infoObj->setProperty("tags", tagsArray);
    
    obj->setProperty("info", juce::var(infoObj));
    
    // FX Presets
    juce::Array<juce::var> presetsArray;
    for (const auto& preset : fxPresets)
    {
        presetsArray.add(preset.toJSON());
    }
    obj->setProperty("fxPresets", presetsArray);
    
    // Sequencer Patterns
    juce::Array<juce::var> patternsArray;
    for (const auto& pattern : sequencerPatterns)
    {
        auto* patternObj = new juce::DynamicObject();
        patternObj->setProperty("name", pattern.name);
        patternObj->setProperty("numSteps", pattern.numSteps);
        patternObj->setProperty("stepsPerBeat", pattern.stepsPerBeat);
        patternObj->setProperty("swing", pattern.swing);
        patternObj->setProperty("swingAmount", pattern.swingAmount);
        
        juce::Array<juce::var> stepsArray;
        for (int i = 0; i < pattern.numSteps; ++i)
        {
            const auto& step = pattern.steps[i];
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
        patternObj->setProperty("steps", stepsArray);
        
        patternsArray.add(juce::var(patternObj));
    }
    obj->setProperty("sequencerPatterns", patternsArray);
    
    return juce::var(obj);
}

bool PresetPack::fromJSON(const juce::var& json)
{
    if (!json.isObject())
        return false;
    
    auto* obj = json.getDynamicObject();
    if (obj == nullptr)
        return false;
    
    clear();
    
    // Load info
    if (auto infoVar = obj->getProperty("info"); infoVar.isObject())
    {
        auto* infoObj = infoVar.getDynamicObject();
        packInfo.name = infoObj->getProperty("name").toString();
        packInfo.author = infoObj->getProperty("author").toString();
        packInfo.description = infoObj->getProperty("description").toString();
        packInfo.version = infoObj->getProperty("version").toString();
        packInfo.createdDate = juce::Time(infoObj->getProperty("createdDate").operator int64());
        packInfo.modifiedDate = juce::Time(infoObj->getProperty("modifiedDate").operator int64());
        packInfo.license = infoObj->getProperty("license").toString();
        packInfo.website = infoObj->getProperty("website").toString();
        
        if (auto* tagsArray = infoObj->getProperty("tags").getArray())
        {
            for (const auto& tag : *tagsArray)
                packInfo.tags.add(tag.toString());
        }
    }
    
    // Load FX presets
    if (auto* presetsArray = obj->getProperty("fxPresets").getArray())
    {
        for (const auto& presetVar : *presetsArray)
        {
            FXPreset preset;
            if (preset.fromJSON(presetVar))
            {
                fxPresets.push_back(preset);
            }
        }
    }
    
    // Load sequencer patterns
    if (auto* patternsArray = obj->getProperty("sequencerPatterns").getArray())
    {
        for (const auto& patternVar : *patternsArray)
        {
            if (auto* patternObj = patternVar.getDynamicObject())
            {
                SequencerPattern pattern;
                pattern.name = patternObj->getProperty("name").toString();
                pattern.numSteps = patternObj->getProperty("numSteps");
                pattern.stepsPerBeat = patternObj->getProperty("stepsPerBeat");
                pattern.swing = patternObj->getProperty("swing");
                pattern.swingAmount = patternObj->getProperty("swingAmount");
                
                if (auto* stepsArray = patternObj->getProperty("steps").getArray())
                {
                    pattern.steps.resize(stepsArray->size());
                    for (int i = 0; i < stepsArray->size(); ++i)
                    {
                        if (auto* stepObj = stepsArray->getUnchecked(i).getDynamicObject())
                        {
                            auto& step = pattern.steps[i];
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
                
                sequencerPatterns.push_back(pattern);
            }
        }
    }
    
    packInfo.numPresets = static_cast<int>(fxPresets.size());
    packInfo.numPatterns = static_cast<int>(sequencerPatterns.size());
    
    return true;
}

bool PresetPack::validate() const
{
    if (packInfo.name.isEmpty())
        return false;
    
    for (const auto& preset : fxPresets)
    {
        if (preset.getName().isEmpty())
            return false;
    }
    
    return true;
}

//==============================================================================
// PresetPackManager Implementation
//==============================================================================

PresetPackManager::PresetPackManager(FXPresetManager* manager)
    : presetManager(manager)
{
    packsDirectory = getDefaultPacksDirectory();
    packsDirectory.createDirectory();
    
    refreshInstalledPacks();
    
    Logger::log(Logger::Level::Info, "PresetPackManager initialized");
}

PresetPackManager::~PresetPackManager()
{
}

bool PresetPackManager::importPack(const juce::File& file, bool mergeWithExisting)
{
    const juce::ScopedLock sl(packLock);
    
    notifyProgress(0.0f, "Loading pack file...");
    
    PresetPack pack;
    if (!pack.loadFromFile(file))
    {
        notifyPackImportFailed("Failed to load pack file: " + file.getFileName());
        return false;
    }
    
    notifyProgress(0.3f, "Validating pack...");
    
    if (!pack.validate())
    {
        notifyPackImportFailed("Pack validation failed");
        return false;
    }
    
    notifyProgress(0.5f, "Installing presets...");
    
    if (!extractPackToPresets(pack, mergeWithExisting))
    {
        notifyPackImportFailed("Failed to install presets");
        return false;
    }
    
    // Copy pack file to packs directory
    juce::File destFile = packsDirectory.getChildFile(file.getFileName());
    if (destFile != file)
    {
        file.copyFileTo(destFile);
    }
    
    notifyProgress(1.0f, "Import complete");
    
    refreshInstalledPacks();
    notifyPackImported(pack.getInfo());
    
    Logger::log(Logger::Level::Info, "Imported pack: " + pack.getInfo().name);
    
    return true;
}

bool PresetPackManager::exportPack(const std::vector<int>& presetIndices,
                                   const std::vector<SequencerPattern>& patterns,
                                   const PresetPackInfo& info,
                                   const juce::File& file)
{
    const juce::ScopedLock sl(packLock);
    
    notifyProgress(0.0f, "Creating pack...");
    
    PresetPack pack;
    
    PresetPackInfo packInfo = info;
    packInfo.createdDate = juce::Time::getCurrentTime();
    packInfo.modifiedDate = juce::Time::getCurrentTime();
    pack.setInfo(packInfo);
    
    notifyProgress(0.2f, "Adding presets...");
    
    // Add selected presets
    for (int idx : presetIndices)
    {
        if (const FXPreset* preset = presetManager->getPreset(idx))
        {
            pack.addFXPreset(*preset);
        }
    }
    
    notifyProgress(0.4f, "Adding patterns...");
    
    // Add patterns
    for (const auto& pattern : patterns)
    {
        pack.addSequencerPattern(pattern);
    }
    
    notifyProgress(0.6f, "Validating pack...");
    
    if (!pack.validate())
    {
        notifyPackImportFailed("Pack validation failed before export");
        return false;
    }
    
    notifyProgress(0.8f, "Writing file...");
    
    if (!pack.saveToFile(file))
    {
        notifyPackImportFailed("Failed to write pack file");
        return false;
    }
    
    notifyProgress(1.0f, "Export complete");
    notifyPackExported(pack.getInfo());
    
    Logger::log(Logger::Level::Info, "Exported pack: " + info.name + " to " + file.getFullPathName());
    
    return true;
}

bool PresetPackManager::exportAllPresets(const PresetPackInfo& info, const juce::File& file)
{
    std::vector<int> allIndices;
    for (int i = 0; i < presetManager->getNumPresets(); ++i)
    {
        allIndices.push_back(i);
    }
    
    return exportPack(allIndices, {}, info, file);
}

juce::StringArray PresetPackManager::scanForPacks(const juce::File& directory)
{
    juce::StringArray packFiles;
    
    if (directory.isDirectory())
    {
        juce::Array<juce::File> files;
        directory.findChildFiles(files, juce::File::findFiles, false, "*.maevnpack");
        
        for (const auto& file : files)
        {
            packFiles.add(file.getFullPathName());
        }
        
        // Also look for JSON packs
        directory.findChildFiles(files, juce::File::findFiles, false, "*.json");
        for (const auto& file : files)
        {
            packFiles.add(file.getFullPathName());
        }
    }
    
    return packFiles;
}

bool PresetPackManager::getPackInfo(const juce::File& file, PresetPackInfo& outInfo)
{
    PresetPack pack;
    if (pack.loadFromFile(file))
    {
        outInfo = pack.getInfo();
        return true;
    }
    return false;
}

bool PresetPackManager::uninstallPack(const juce::String& packName)
{
    const juce::ScopedLock sl(packLock);
    
    // Find and remove pack file
    juce::Array<juce::File> packFiles;
    packsDirectory.findChildFiles(packFiles, juce::File::findFiles, false);
    
    for (const auto& file : packFiles)
    {
        PresetPackInfo info;
        if (getPackInfo(file, info) && info.name == packName)
        {
            if (file.deleteFile())
            {
                refreshInstalledPacks();
                Logger::log(Logger::Level::Info, "Uninstalled pack: " + packName);
                return true;
            }
        }
    }
    
    return false;
}

void PresetPackManager::setPacksDirectory(const juce::File& directory)
{
    const juce::ScopedLock sl(packLock);
    
    packsDirectory = directory;
    packsDirectory.createDirectory();
    refreshInstalledPacks();
}

void PresetPackManager::addListener(PresetPackListener* listener)
{
    listeners.push_back(listener);
}

void PresetPackManager::removeListener(PresetPackListener* listener)
{
    listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
}

void PresetPackManager::refreshInstalledPacks()
{
    const juce::ScopedLock sl(packLock);
    
    installedPacks.clear();
    
    auto packFiles = scanForPacks(packsDirectory);
    
    for (const auto& path : packFiles)
    {
        PresetPackInfo info;
        if (getPackInfo(juce::File(path), info))
        {
            installedPacks.push_back(info);
        }
    }
    
    Logger::log(Logger::Level::Info, "Found " + juce::String(static_cast<int>(installedPacks.size())) + 
                " installed packs");
}

std::unique_ptr<PresetPack> PresetPackManager::createPackFromSession(const PresetPackInfo& info)
{
    auto pack = std::make_unique<PresetPack>();
    
    PresetPackInfo packInfo = info;
    packInfo.createdDate = juce::Time::getCurrentTime();
    packInfo.modifiedDate = juce::Time::getCurrentTime();
    pack->setInfo(packInfo);
    
    // Add all current presets
    for (int i = 0; i < presetManager->getNumPresets(); ++i)
    {
        if (const FXPreset* preset = presetManager->getPreset(i))
        {
            pack->addFXPreset(*preset);
        }
    }
    
    return pack;
}

bool PresetPackManager::validatePackFile(const juce::File& file)
{
    PresetPack pack;
    if (!pack.loadFromFile(file))
        return false;
    
    return pack.validate();
}

juce::File PresetPackManager::getDefaultPacksDirectory()
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("MAEVN")
        .getChildFile("Packs");
}

void PresetPackManager::notifyPackImported(const PresetPackInfo& info)
{
    for (auto* listener : listeners)
    {
        listener->onPackImported(info);
    }
}

void PresetPackManager::notifyPackExported(const PresetPackInfo& info)
{
    for (auto* listener : listeners)
    {
        listener->onPackExported(info);
    }
}

void PresetPackManager::notifyPackImportFailed(const juce::String& error)
{
    Logger::log(Logger::Level::Error, error);
    
    for (auto* listener : listeners)
    {
        listener->onPackImportFailed(error);
    }
}

void PresetPackManager::notifyProgress(float progress, const juce::String& status)
{
    for (auto* listener : listeners)
    {
        listener->onPackProgress(progress, status);
    }
}

bool PresetPackManager::extractPackToPresets(const PresetPack& pack, bool merge)
{
    if (!merge)
    {
        presetManager->clearPresets();
    }
    
    for (const auto& preset : pack.getFXPresets())
    {
        // Check for duplicates if merging
        if (merge && presetManager->hasPreset(preset.getName()))
        {
            // Add with modified name
            FXPreset modifiedPreset = preset;
            modifiedPreset.setName(preset.getName() + " (" + pack.getInfo().name + ")");
            presetManager->addPreset(modifiedPreset);
        }
        else
        {
            presetManager->addPreset(preset);
        }
    }
    
    return true;
}

//==============================================================================
// PresetPackBrowserComponent Implementation
//==============================================================================

PresetPackBrowserComponent::PresetPackBrowserComponent(PresetPackManager* manager)
    : packManager(manager)
    , packListBox("Pack List", this)
    , selectedPackIndex(-1)
{
    setupUI();
}

PresetPackBrowserComponent::~PresetPackBrowserComponent()
{
}

void PresetPackBrowserComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(30, 30, 35));
    
    g.setColour(juce::Colour(50, 50, 55));
    g.drawRect(getLocalBounds(), 1);
}

void PresetPackBrowserComponent::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    
    titleLabel.setBounds(bounds.removeFromTop(30));
    
    auto buttonArea = bounds.removeFromTop(35);
    int buttonWidth = (buttonArea.getWidth() - 20) / 3;
    importButton.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(2));
    exportButton.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(2));
    refreshButton.setBounds(buttonArea.reduced(2));
    
    bounds.removeFromTop(5);
    packListBox.setBounds(bounds);
}

int PresetPackBrowserComponent::getNumRows()
{
    return static_cast<int>(displayedPacks.size());
}

void PresetPackBrowserComponent::paintListBoxItem(int rowNumber, juce::Graphics& g,
                                                  int width, int height, bool rowIsSelected)
{
    if (rowNumber < 0 || rowNumber >= static_cast<int>(displayedPacks.size()))
        return;
    
    const auto& pack = displayedPacks[rowNumber];
    
    if (rowIsSelected)
        g.fillAll(juce::Colour(60, 100, 180));
    else if (rowNumber % 2 == 0)
        g.fillAll(juce::Colour(40, 40, 45));
    else
        g.fillAll(juce::Colour(35, 35, 40));
    
    g.setColour(juce::Colours::white);
    g.setFont(14.0f);
    g.drawText(pack.name, 10, 5, width - 20, 20, juce::Justification::centredLeft);
    
    g.setColour(juce::Colours::grey);
    g.setFont(11.0f);
    g.drawText(pack.author + " | " + juce::String(pack.numPresets) + " presets",
               10, 25, width - 20, 15, juce::Justification::centredLeft);
}

void PresetPackBrowserComponent::listBoxItemClicked(int row, const juce::MouseEvent& event)
{
    selectedPackIndex = row;
}

void PresetPackBrowserComponent::listBoxItemDoubleClicked(int row, const juce::MouseEvent& event)
{
    // Double-click to import/activate pack
    if (row >= 0 && row < static_cast<int>(displayedPacks.size()))
    {
        // Future: Open pack details or install
    }
}

void PresetPackBrowserComponent::refresh()
{
    if (packManager != nullptr)
    {
        packManager->refreshInstalledPacks();
        displayedPacks.clear();
        
        for (const auto& info : packManager->getInstalledPacks())
        {
            displayedPacks.push_back(info);
        }
        
        packListBox.updateContent();
    }
}

void PresetPackBrowserComponent::showImportDialog()
{
    onImportClicked();
}

void PresetPackBrowserComponent::showExportDialog()
{
    onExportClicked();
}

void PresetPackBrowserComponent::setupUI()
{
    addAndMakeVisible(titleLabel);
    titleLabel.setText("Preset Packs", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    
    addAndMakeVisible(importButton);
    importButton.setButtonText("Import");
    importButton.onClick = [this] { onImportClicked(); };
    
    addAndMakeVisible(exportButton);
    exportButton.setButtonText("Export");
    exportButton.onClick = [this] { onExportClicked(); };
    
    addAndMakeVisible(refreshButton);
    refreshButton.setButtonText("Refresh");
    refreshButton.onClick = [this] { onRefreshClicked(); };
    
    addAndMakeVisible(packListBox);
    packListBox.setRowHeight(45);
    packListBox.setColour(juce::ListBox::backgroundColourId, juce::Colour(25, 25, 30));
    
    refresh();
}

void PresetPackBrowserComponent::onImportClicked()
{
    juce::FileChooser chooser("Import Preset Pack",
                              juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
                              "*.maevnpack;*.json");
    
    if (chooser.browseForFileToOpen())
    {
        juce::File file = chooser.getResult();
        if (packManager != nullptr)
        {
            packManager->importPack(file, true);
            refresh();
        }
    }
}

void PresetPackBrowserComponent::onExportClicked()
{
    juce::FileChooser chooser("Export Preset Pack",
                              juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                                  .getChildFile("MyPack.maevnpack"),
                              "*.maevnpack");
    
    if (chooser.browseForFileToSave(true))
    {
        juce::File file = chooser.getResult();
        
        PresetPackInfo info;
        info.name = file.getFileNameWithoutExtension();
        info.author = "User";
        info.description = "Exported preset pack";
        info.version = "1.0.0";
        
        if (packManager != nullptr)
        {
            packManager->exportAllPresets(info, file);
        }
    }
}

void PresetPackBrowserComponent::onRefreshClicked()
{
    refresh();
}

} // namespace MAEVN
