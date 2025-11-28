/**
 * @file ModelMarketplace.cpp
 * @brief Implementation of Model Marketplace functionality
 */

#include "ModelMarketplace.h"

namespace MAEVN
{

//==============================================================================
// ModelMarketplace Implementation
//==============================================================================

ModelMarketplace::ModelMarketplace()
    : connected(false)
    , downloadPool(2) // 2 concurrent downloads max
{
    modelsDirectory = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                          .getChildFile("MAEVN")
                          .getChildFile("MarketplaceModels");
    modelsDirectory.createDirectory();
    
    loadInstalledModels();
    
    Logger::log(Logger::Level::Info, "ModelMarketplace initialized");
}

ModelMarketplace::~ModelMarketplace()
{
    downloadPool.removeAllJobs(true, 5000);
    saveInstalledModels();
}

bool ModelMarketplace::initialize(const juce::String& endpoint)
{
    const juce::ScopedLock sl(marketplaceLock);
    
    apiEndpoint = endpoint.isEmpty() ? "https://api.maevn.io/marketplace" : endpoint;
    
    // In a real implementation, this would connect to the API
    // For now, generate sample catalog
    catalog = generateSampleCatalog();
    connected = true;
    
    notifyCatalogRefreshed(static_cast<int>(catalog.size()));
    
    Logger::log(Logger::Level::Info, "Marketplace initialized with " + 
                juce::String(static_cast<int>(catalog.size())) + " models");
    
    return true;
}

void ModelMarketplace::refreshCatalog()
{
    const juce::ScopedLock sl(marketplaceLock);
    
    // In real implementation, fetch from API
    catalog = generateSampleCatalog();
    
    notifyCatalogRefreshed(static_cast<int>(catalog.size()));
}

void ModelMarketplace::searchModels(const MarketplaceSearchCriteria& criteria)
{
    const juce::ScopedLock sl(marketplaceLock);
    
    std::vector<MarketplaceModelInfo> results;
    
    for (const auto& model : catalog)
    {
        // Apply filters
        if (!criteria.searchText.isEmpty())
        {
            bool matchesSearch = model.name.containsIgnoreCase(criteria.searchText) ||
                                model.description.containsIgnoreCase(criteria.searchText) ||
                                model.author.containsIgnoreCase(criteria.searchText);
            
            for (const auto& tag : model.tags)
            {
                if (tag.containsIgnoreCase(criteria.searchText))
                {
                    matchesSearch = true;
                    break;
                }
            }
            
            if (!matchesSearch)
                continue;
        }
        
        if (criteria.verifiedOnly && !model.isVerified)
            continue;
        
        if (model.averageRating < criteria.minRating)
            continue;
        
        results.push_back(model);
    }
    
    // Sort results
    if (criteria.sortBy == "downloads")
    {
        std::sort(results.begin(), results.end(), 
                  [&criteria](const MarketplaceModelInfo& a, const MarketplaceModelInfo& b) {
                      return criteria.sortDescending ? 
                             (a.downloadCount > b.downloadCount) : 
                             (a.downloadCount < b.downloadCount);
                  });
    }
    else if (criteria.sortBy == "rating")
    {
        std::sort(results.begin(), results.end(),
                  [&criteria](const MarketplaceModelInfo& a, const MarketplaceModelInfo& b) {
                      return criteria.sortDescending ?
                             (a.averageRating > b.averageRating) :
                             (a.averageRating < b.averageRating);
                  });
    }
    else if (criteria.sortBy == "date")
    {
        std::sort(results.begin(), results.end(),
                  [&criteria](const MarketplaceModelInfo& a, const MarketplaceModelInfo& b) {
                      return criteria.sortDescending ?
                             (a.uploadDate > b.uploadDate) :
                             (a.uploadDate < b.uploadDate);
                  });
    }
    
    // Limit results
    if (criteria.maxResults > 0 && static_cast<int>(results.size()) > criteria.maxResults)
    {
        results.resize(criteria.maxResults);
    }
    
    notifySearchResults(results);
}

const MarketplaceModelInfo* ModelMarketplace::getModelInfo(const juce::String& modelId) const
{
    for (const auto& model : catalog)
    {
        if (model.id == modelId)
            return &model;
    }
    return nullptr;
}

bool ModelMarketplace::downloadModel(const juce::String& modelId, const juce::File& destinationDir)
{
    const MarketplaceModelInfo* model = getModelInfo(modelId);
    if (model == nullptr)
    {
        notifyDownloadFailed(modelId, "Model not found in catalog");
        return false;
    }
    
    // Check if already downloading
    {
        const juce::ScopedLock sl(marketplaceLock);
        if (activeDownloads.find(modelId) != activeDownloads.end())
        {
            return false; // Already downloading
        }
        
        DownloadProgress progress;
        progress.modelId = modelId;
        progress.totalBytes = model->fileSize;
        progress.status = "Starting download...";
        activeDownloads[modelId] = progress;
    }
    
    notifyDownloadStarted(modelId);
    
    // Start download in thread pool
    auto* task = new DownloadTask(*this, *model, destinationDir);
    downloadPool.addJob(task, true);
    
    return true;
}

void ModelMarketplace::cancelDownload(const juce::String& modelId)
{
    const juce::ScopedLock sl(marketplaceLock);
    
    // Remove from active downloads
    auto it = activeDownloads.find(modelId);
    if (it != activeDownloads.end())
    {
        it->second.hasFailed = true;
        it->second.errorMessage = "Cancelled by user";
        activeDownloads.erase(it);
    }
    
    // Note: In real implementation, would also signal the download thread to stop
}

DownloadProgress ModelMarketplace::getDownloadProgress(const juce::String& modelId) const
{
    const juce::ScopedLock sl(marketplaceLock);
    
    auto it = activeDownloads.find(modelId);
    if (it != activeDownloads.end())
    {
        return it->second;
    }
    
    return DownloadProgress();
}

bool ModelMarketplace::isDownloading(const juce::String& modelId) const
{
    const juce::ScopedLock sl(marketplaceLock);
    return activeDownloads.find(modelId) != activeDownloads.end();
}

bool ModelMarketplace::isModelInstalled(const juce::String& modelId) const
{
    const juce::ScopedLock sl(marketplaceLock);
    return installedModels.find(modelId) != installedModels.end();
}

juce::File ModelMarketplace::getInstalledModelPath(const juce::String& modelId) const
{
    const juce::ScopedLock sl(marketplaceLock);
    
    auto it = installedModels.find(modelId);
    if (it != installedModels.end())
    {
        return it->second;
    }
    
    return juce::File();
}

bool ModelMarketplace::deleteInstalledModel(const juce::String& modelId)
{
    const juce::ScopedLock sl(marketplaceLock);
    
    auto it = installedModels.find(modelId);
    if (it != installedModels.end())
    {
        juce::File modelFile = it->second;
        if (modelFile.deleteFile())
        {
            installedModels.erase(it);
            saveInstalledModels();
            Logger::log(Logger::Level::Info, "Deleted model: " + modelId);
            return true;
        }
    }
    
    return false;
}

juce::StringArray ModelMarketplace::getInstalledModelIds() const
{
    const juce::ScopedLock sl(marketplaceLock);
    
    juce::StringArray ids;
    for (const auto& pair : installedModels)
    {
        ids.add(pair.first);
    }
    return ids;
}

void ModelMarketplace::setModelsDirectory(const juce::File& directory)
{
    const juce::ScopedLock sl(marketplaceLock);
    modelsDirectory = directory;
    modelsDirectory.createDirectory();
}

void ModelMarketplace::submitRating(const juce::String& modelId, int rating, 
                                    const juce::String& review)
{
    // In real implementation, submit to API
    Logger::log(Logger::Level::Info, "Submitted rating " + juce::String(rating) + 
                " for model: " + modelId);
}

void ModelMarketplace::reportModel(const juce::String& modelId, const juce::String& reason)
{
    // In real implementation, submit to API
    Logger::log(Logger::Level::Info, "Reported model: " + modelId + " - " + reason);
}

void ModelMarketplace::addListener(ModelMarketplaceListener* listener)
{
    listeners.push_back(listener);
}

void ModelMarketplace::removeListener(ModelMarketplaceListener* listener)
{
    listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
}

juce::String ModelMarketplace::getCategoryName(ModelCategory category)
{
    switch (category)
    {
        case ModelCategory::Vocal:        return "Vocal";
        case ModelCategory::Drums:        return "Drums";
        case ModelCategory::Bass:         return "Bass";
        case ModelCategory::Instruments:  return "Instruments";
        case ModelCategory::Effects:      return "Effects";
        case ModelCategory::Mastering:    return "Mastering";
        case ModelCategory::Experimental: return "Experimental";
        default:                          return "Other";
    }
}

juce::String ModelMarketplace::formatFileSize(size_t bytes)
{
    if (bytes < 1024)
        return juce::String(bytes) + " B";
    else if (bytes < 1024 * 1024)
        return juce::String(bytes / 1024.0, 1) + " KB";
    else if (bytes < 1024 * 1024 * 1024)
        return juce::String(bytes / (1024.0 * 1024.0), 1) + " MB";
    else
        return juce::String(bytes / (1024.0 * 1024.0 * 1024.0), 2) + " GB";
}

void ModelMarketplace::notifyCatalogRefreshed(int numModels)
{
    for (auto* listener : listeners)
    {
        listener->onCatalogRefreshed(numModels);
    }
}

void ModelMarketplace::notifyDownloadStarted(const juce::String& modelId)
{
    for (auto* listener : listeners)
    {
        listener->onDownloadStarted(modelId);
    }
}

void ModelMarketplace::notifyDownloadProgress(const DownloadProgress& progress)
{
    for (auto* listener : listeners)
    {
        listener->onDownloadProgress(progress);
    }
}

void ModelMarketplace::notifyDownloadComplete(const juce::String& modelId, const juce::File& file)
{
    for (auto* listener : listeners)
    {
        listener->onDownloadComplete(modelId, file);
    }
}

void ModelMarketplace::notifyDownloadFailed(const juce::String& modelId, const juce::String& error)
{
    for (auto* listener : listeners)
    {
        listener->onDownloadFailed(modelId, error);
    }
}

void ModelMarketplace::notifySearchResults(const std::vector<MarketplaceModelInfo>& results)
{
    for (auto* listener : listeners)
    {
        listener->onSearchResults(results);
    }
}

void ModelMarketplace::loadInstalledModels()
{
    juce::File indexFile = modelsDirectory.getChildFile("installed.json");
    
    if (indexFile.existsAsFile())
    {
        juce::String jsonString = indexFile.loadFileAsString();
        juce::var json = juce::JSON::parse(jsonString);
        
        if (auto* array = json.getArray())
        {
            for (const auto& item : *array)
            {
                if (auto* obj = item.getDynamicObject())
                {
                    juce::String modelId = obj->getProperty("id").toString();
                    juce::String path = obj->getProperty("path").toString();
                    
                    juce::File file(path);
                    if (file.existsAsFile())
                    {
                        installedModels[modelId] = file;
                    }
                }
            }
        }
    }
}

void ModelMarketplace::saveInstalledModels()
{
    juce::Array<juce::var> array;
    
    for (const auto& pair : installedModels)
    {
        auto* obj = new juce::DynamicObject();
        obj->setProperty("id", pair.first);
        obj->setProperty("path", pair.second.getFullPathName());
        array.add(juce::var(obj));
    }
    
    juce::File indexFile = modelsDirectory.getChildFile("installed.json");
    juce::String jsonString = juce::JSON::toString(juce::var(array), true);
    indexFile.replaceWithText(jsonString);
}

std::vector<MarketplaceModelInfo> ModelMarketplace::generateSampleCatalog()
{
    std::vector<MarketplaceModelInfo> models;
    
    // Sample vocal model
    {
        MarketplaceModelInfo model;
        model.id = "vocal-tts-v1";
        model.name = "MAEVN TTS Vocal";
        model.description = "High-quality text-to-speech model for vocal synthesis. "
                           "Supports multiple emotions and speaking styles.";
        model.author = "MAEVN Team";
        model.version = "1.0.0";
        model.category = ModelCategory::Vocal;
        model.tags = { "TTS", "Vocal", "AI", "Speech" };
        model.fileSize = 150 * 1024 * 1024; // 150MB
        model.downloadCount = 5000;
        model.averageRating = 4.5f;
        model.ratingCount = 120;
        model.isVerified = true;
        model.requiresGPU = false;
        model.license = "MIT";
        models.push_back(model);
    }
    
    // Sample 808 model
    {
        MarketplaceModelInfo model;
        model.id = "808-ddsp-v2";
        model.name = "Neural 808 Bass";
        model.description = "DDSP-based 808 sub-bass generator with realistic harmonics "
                           "and glide support. Perfect for trap and hip-hop production.";
        model.author = "Community";
        model.version = "2.1.0";
        model.category = ModelCategory::Bass;
        model.tags = { "808", "Bass", "Trap", "DDSP" };
        model.fileSize = 45 * 1024 * 1024; // 45MB
        model.downloadCount = 12000;
        model.averageRating = 4.8f;
        model.ratingCount = 300;
        model.isVerified = true;
        model.requiresGPU = false;
        model.license = "Apache 2.0";
        models.push_back(model);
    }
    
    // Sample hi-hat model
    {
        MarketplaceModelInfo model;
        model.id = "hihat-synth-v1";
        model.name = "AI Hi-Hat Synth";
        model.description = "Neural network-based hi-hat synthesizer with velocity-sensitive "
                           "response and realistic transients.";
        model.author = "DrumSynth Labs";
        model.version = "1.2.0";
        model.category = ModelCategory::Drums;
        model.tags = { "Hi-Hat", "Drums", "Synth", "Neural" };
        model.fileSize = 25 * 1024 * 1024; // 25MB
        model.downloadCount = 8500;
        model.averageRating = 4.3f;
        model.ratingCount = 180;
        model.isVerified = true;
        model.requiresGPU = false;
        model.license = "MIT";
        models.push_back(model);
    }
    
    // Sample vocoder model
    {
        MarketplaceModelInfo model;
        model.id = "hifigan-v3";
        model.name = "HiFi-GAN Vocoder";
        model.description = "High-fidelity neural vocoder for converting mel-spectrograms "
                           "to waveforms. GPU recommended for real-time performance.";
        model.author = "MAEVN Team";
        model.version = "3.0.0";
        model.category = ModelCategory::Vocal;
        model.tags = { "Vocoder", "HiFi-GAN", "TTS", "Neural" };
        model.fileSize = 200 * 1024 * 1024; // 200MB
        model.downloadCount = 3500;
        model.averageRating = 4.7f;
        model.ratingCount = 90;
        model.isVerified = true;
        model.requiresGPU = true;
        model.license = "MIT";
        models.push_back(model);
    }
    
    // Sample mastering model
    {
        MarketplaceModelInfo model;
        model.id = "ai-master-v1";
        model.name = "AI Mastering Engine";
        model.description = "Neural network-based mastering chain with automatic EQ, "
                           "compression, and loudness optimization.";
        model.author = "MasterAI";
        model.version = "1.0.0";
        model.category = ModelCategory::Mastering;
        model.tags = { "Mastering", "EQ", "Compression", "AI" };
        model.fileSize = 300 * 1024 * 1024; // 300MB
        model.downloadCount = 2000;
        model.averageRating = 4.2f;
        model.ratingCount = 50;
        model.isVerified = false;
        model.requiresGPU = true;
        model.license = "Commercial";
        models.push_back(model);
    }
    
    return models;
}

//==============================================================================
// DownloadTask Implementation
//==============================================================================

ModelMarketplace::DownloadTask::DownloadTask(ModelMarketplace& owner, 
                                              const MarketplaceModelInfo& model,
                                              const juce::File& destDir)
    : ThreadPoolJob("Download: " + model.name)
    , marketplace(owner)
    , modelInfo(model)
    , destinationDir(destDir)
{
}

juce::ThreadPoolJob::JobStatus ModelMarketplace::DownloadTask::runJob()
{
    // Simulate download with progress updates
    const int numSteps = 20;
    size_t bytesPerStep = modelInfo.fileSize / numSteps;
    
    for (int i = 0; i <= numSteps; ++i)
    {
        if (shouldExit())
        {
            return jobHasFinished;
        }
        
        // Update progress
        {
            const juce::ScopedLock sl(marketplace.marketplaceLock);
            auto& progress = marketplace.activeDownloads[modelInfo.id];
            progress.bytesDownloaded = i * bytesPerStep;
            progress.progress = static_cast<float>(i) / static_cast<float>(numSteps);
            progress.status = "Downloading... " + 
                             ModelMarketplace::formatFileSize(progress.bytesDownloaded) + " / " +
                             ModelMarketplace::formatFileSize(modelInfo.fileSize);
            
            marketplace.notifyDownloadProgress(progress);
        }
        
        // Simulate download time
        juce::Thread::sleep(100);
    }
    
    // Create placeholder file (in real implementation, would save actual downloaded data)
    juce::File destFile = destinationDir.getChildFile(modelInfo.id + ".onnx");
    destFile.create();
    destFile.replaceWithText("# Placeholder for " + modelInfo.name + "\n# Size: " + 
                            ModelMarketplace::formatFileSize(modelInfo.fileSize));
    
    // Mark as complete
    {
        const juce::ScopedLock sl(marketplace.marketplaceLock);
        auto& progress = marketplace.activeDownloads[modelInfo.id];
        progress.isComplete = true;
        progress.progress = 1.0f;
        progress.status = "Complete";
        
        marketplace.installedModels[modelInfo.id] = destFile;
        marketplace.saveInstalledModels();
        marketplace.activeDownloads.erase(modelInfo.id);
    }
    
    marketplace.notifyDownloadComplete(modelInfo.id, destFile);
    
    Logger::log(Logger::Level::Info, "Downloaded model: " + modelInfo.name);
    
    return jobHasFinished;
}

//==============================================================================
// ModelMarketplaceBrowser Implementation
//==============================================================================

ModelMarketplaceBrowser::ModelMarketplaceBrowser(ModelMarketplace* mp)
    : marketplace(mp)
    , modelListBox("Model List", this)
    , selectedModelIndex(-1)
    , downloadProgressBar(nullptr)
{
    setupUI();
    
    if (marketplace != nullptr)
    {
        marketplace->addListener(this);
    }
}

ModelMarketplaceBrowser::~ModelMarketplaceBrowser()
{
    if (marketplace != nullptr)
    {
        marketplace->removeListener(this);
    }
}

void ModelMarketplaceBrowser::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(30, 30, 35));
    
    g.setColour(juce::Colour(50, 50, 55));
    g.drawRect(getLocalBounds(), 1);
}

void ModelMarketplaceBrowser::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    
    titleLabel.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(5);
    
    auto searchRow = bounds.removeFromTop(30);
    searchBox.setBounds(searchRow.removeFromLeft(200));
    searchRow.removeFromLeft(10);
    categoryFilter.setBounds(searchRow.removeFromLeft(120));
    searchRow.removeFromLeft(10);
    verifiedOnlyToggle.setBounds(searchRow.removeFromLeft(100));
    searchRow.removeFromLeft(10);
    refreshButton.setBounds(searchRow.removeFromLeft(80));
    
    bounds.removeFromTop(10);
    
    auto bottomRow = bounds.removeFromBottom(35);
    downloadButton.setBounds(bottomRow.removeFromLeft(120).reduced(2));
    statusLabel.setBounds(bottomRow);
    
    bounds.removeFromBottom(5);
    modelListBox.setBounds(bounds);
}

void ModelMarketplaceBrowser::onCatalogRefreshed(int numModels)
{
    displayedModels = marketplace->getCatalog();
    modelListBox.updateContent();
    statusLabel.setText("Catalog: " + juce::String(numModels) + " models", juce::dontSendNotification);
}

void ModelMarketplaceBrowser::onDownloadStarted(const juce::String& modelId)
{
    statusLabel.setText("Downloading: " + modelId, juce::dontSendNotification);
}

void ModelMarketplaceBrowser::onDownloadProgress(const DownloadProgress& progress)
{
    statusLabel.setText(progress.status, juce::dontSendNotification);
}

void ModelMarketplaceBrowser::onDownloadComplete(const juce::String& modelId, const juce::File& localFile)
{
    statusLabel.setText("Downloaded: " + modelId, juce::dontSendNotification);
    modelListBox.repaint();
}

void ModelMarketplaceBrowser::onDownloadFailed(const juce::String& modelId, const juce::String& error)
{
    statusLabel.setText("Download failed: " + error, juce::dontSendNotification);
}

void ModelMarketplaceBrowser::onSearchResults(const std::vector<MarketplaceModelInfo>& results)
{
    displayedModels = results;
    modelListBox.updateContent();
    statusLabel.setText("Found " + juce::String(static_cast<int>(results.size())) + " models", 
                       juce::dontSendNotification);
}

int ModelMarketplaceBrowser::getNumRows()
{
    return static_cast<int>(displayedModels.size());
}

void ModelMarketplaceBrowser::paintListBoxItem(int rowNumber, juce::Graphics& g,
                                                int width, int height, bool rowIsSelected)
{
    if (rowNumber < 0 || rowNumber >= static_cast<int>(displayedModels.size()))
        return;
    
    const auto& model = displayedModels[rowNumber];
    
    if (rowIsSelected)
        g.fillAll(juce::Colour(60, 100, 180));
    else if (rowNumber % 2 == 0)
        g.fillAll(juce::Colour(40, 40, 45));
    else
        g.fillAll(juce::Colour(35, 35, 40));
    
    // Installed indicator
    if (marketplace != nullptr && marketplace->isModelInstalled(model.id))
    {
        g.setColour(juce::Colours::limegreen);
        g.fillEllipse(width - 20.0f, height / 2.0f - 5.0f, 10.0f, 10.0f);
    }
    
    // Verified badge
    if (model.isVerified)
    {
        g.setColour(juce::Colours::dodgerblue);
        g.fillEllipse(width - 35.0f, height / 2.0f - 5.0f, 10.0f, 10.0f);
    }
    
    // Model name
    g.setColour(juce::Colours::white);
    g.setFont(14.0f);
    g.drawText(model.name, 10, 5, width - 50, 20, juce::Justification::centredLeft);
    
    // Model info
    g.setColour(juce::Colours::grey);
    g.setFont(11.0f);
    juce::String info = model.author + " | " + 
                       ModelMarketplace::formatFileSize(model.fileSize) + " | " +
                       juce::String(model.averageRating, 1) + "★";
    g.drawText(info, 10, 25, width - 50, 15, juce::Justification::centredLeft);
}

void ModelMarketplaceBrowser::listBoxItemClicked(int row, const juce::MouseEvent& event)
{
    selectedModelIndex = row;
    downloadButton.setEnabled(row >= 0);
}

void ModelMarketplaceBrowser::refresh()
{
    if (marketplace != nullptr)
    {
        marketplace->refreshCatalog();
    }
}

void ModelMarketplaceBrowser::setupUI()
{
    addAndMakeVisible(titleLabel);
    titleLabel.setText("Model Marketplace", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    
    addAndMakeVisible(searchBox);
    searchBox.setTextToShowWhenEmpty("Search models...", juce::Colours::grey);
    searchBox.onTextChange = [this] { onSearchChanged(); };
    
    addAndMakeVisible(categoryFilter);
    categoryFilter.addItem("All Categories", 1);
    categoryFilter.addItem("Vocal", 2);
    categoryFilter.addItem("Drums", 3);
    categoryFilter.addItem("Bass", 4);
    categoryFilter.addItem("Effects", 5);
    categoryFilter.setSelectedId(1);
    
    addAndMakeVisible(verifiedOnlyToggle);
    verifiedOnlyToggle.setButtonText("Verified");
    
    addAndMakeVisible(refreshButton);
    refreshButton.setButtonText("Refresh");
    refreshButton.onClick = [this] { onRefreshClicked(); };
    
    addAndMakeVisible(modelListBox);
    modelListBox.setRowHeight(50);
    modelListBox.setColour(juce::ListBox::backgroundColourId, juce::Colour(25, 25, 30));
    
    addAndMakeVisible(downloadButton);
    downloadButton.setButtonText("Download");
    downloadButton.setEnabled(false);
    downloadButton.onClick = [this] { onDownloadClicked(); };
    
    addAndMakeVisible(statusLabel);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    
    // Initial refresh
    refresh();
}

void ModelMarketplaceBrowser::onSearchChanged()
{
    if (marketplace == nullptr)
        return;
    
    MarketplaceSearchCriteria criteria;
    criteria.searchText = searchBox.getText();
    criteria.verifiedOnly = verifiedOnlyToggle.getToggleState();
    
    marketplace->searchModels(criteria);
}

void ModelMarketplaceBrowser::onRefreshClicked()
{
    refresh();
}

void ModelMarketplaceBrowser::onDownloadClicked()
{
    if (marketplace == nullptr || selectedModelIndex < 0)
        return;
    
    if (selectedModelIndex < static_cast<int>(displayedModels.size()))
    {
        const auto& model = displayedModels[selectedModelIndex];
        marketplace->downloadModel(model.id, marketplace->getModelsDirectory());
    }
}

} // namespace MAEVN
