/**
 * @file ModelMarketplace.h
 * @brief Model Marketplace - Download community-shared ONNX models
 * 
 * This module provides infrastructure for discovering, downloading, and
 * managing community-shared ONNX models from an online marketplace.
 */

#pragma once

#include <JuceHeader.h>
#include <memory>
#include <vector>
#include <functional>
#include "Utilities.h"

namespace MAEVN
{

//==============================================================================
/**
 * @brief Model category types
 */
enum class ModelCategory
{
    Vocal,          ///< Vocal processing models
    Drums,          ///< Drum synthesis models
    Bass,           ///< Bass synthesis models
    Instruments,    ///< Instrument models
    Effects,        ///< Audio effect models
    Mastering,      ///< Mastering/mixing models
    Experimental,   ///< Experimental/research models
    Other           ///< Other category
};

//==============================================================================
/**
 * @brief Model information from marketplace
 */
struct MarketplaceModelInfo
{
    juce::String id;                ///< Unique model ID
    juce::String name;              ///< Model display name
    juce::String description;       ///< Full description
    juce::String author;            ///< Model author
    juce::String version;           ///< Version string
    ModelCategory category;         ///< Model category
    juce::StringArray tags;         ///< Tags for filtering
    
    juce::String downloadUrl;       ///< URL to download model
    juce::String thumbnailUrl;      ///< URL to thumbnail image
    juce::String documentationUrl;  ///< URL to documentation
    
    size_t fileSize;                ///< Model file size in bytes
    juce::Time uploadDate;          ///< Upload date
    juce::Time lastUpdated;         ///< Last update date
    
    int downloadCount;              ///< Number of downloads
    float averageRating;            ///< Average user rating (0-5)
    int ratingCount;                ///< Number of ratings
    
    juce::String license;           ///< License type
    bool isVerified;                ///< Verified by moderators
    bool requiresGPU;               ///< Requires GPU acceleration
    
    // Input/output specifications
    juce::String inputFormat;       ///< Expected input format
    juce::String outputFormat;      ///< Output format
    int sampleRate;                 ///< Expected sample rate (0 = any)
    
    MarketplaceModelInfo()
        : category(ModelCategory::Other)
        , fileSize(0)
        , downloadCount(0)
        , averageRating(0.0f)
        , ratingCount(0)
        , isVerified(false)
        , requiresGPU(false)
        , sampleRate(0)
    {}
};

//==============================================================================
/**
 * @brief Download progress information
 */
struct DownloadProgress
{
    juce::String modelId;
    size_t bytesDownloaded;
    size_t totalBytes;
    float progress;             ///< 0.0 - 1.0
    juce::String status;
    bool isComplete;
    bool hasFailed;
    juce::String errorMessage;
    
    DownloadProgress()
        : bytesDownloaded(0)
        , totalBytes(0)
        , progress(0.0f)
        , isComplete(false)
        , hasFailed(false)
    {}
};

//==============================================================================
/**
 * @brief Listener interface for marketplace events
 */
class ModelMarketplaceListener
{
public:
    virtual ~ModelMarketplaceListener() = default;
    
    /**
     * @brief Called when model catalog is refreshed
     */
    virtual void onCatalogRefreshed(int numModels) = 0;
    
    /**
     * @brief Called when download starts
     */
    virtual void onDownloadStarted(const juce::String& modelId) = 0;
    
    /**
     * @brief Called with download progress updates
     */
    virtual void onDownloadProgress(const DownloadProgress& progress) = 0;
    
    /**
     * @brief Called when download completes
     */
    virtual void onDownloadComplete(const juce::String& modelId, const juce::File& localFile) = 0;
    
    /**
     * @brief Called when download fails
     */
    virtual void onDownloadFailed(const juce::String& modelId, const juce::String& error) = 0;
    
    /**
     * @brief Called when search results are available
     */
    virtual void onSearchResults(const std::vector<MarketplaceModelInfo>& results) = 0;
};

//==============================================================================
/**
 * @brief Search/filter criteria for marketplace
 */
struct MarketplaceSearchCriteria
{
    juce::String searchText;        ///< Text search
    ModelCategory category;         ///< Category filter (optional)
    juce::StringArray tags;         ///< Tag filters
    bool verifiedOnly;              ///< Show only verified models
    bool freeOnly;                  ///< Show only free models
    float minRating;                ///< Minimum rating filter
    juce::String sortBy;            ///< Sort field (downloads, rating, date)
    bool sortDescending;            ///< Sort direction
    int maxResults;                 ///< Maximum results to return
    
    MarketplaceSearchCriteria()
        : category(ModelCategory::Other)
        , verifiedOnly(false)
        , freeOnly(true)
        , minRating(0.0f)
        , sortBy("downloads")
        , sortDescending(true)
        , maxResults(50)
    {}
};

//==============================================================================
/**
 * @brief Model Marketplace Manager
 * 
 * Handles connection to model marketplace, browsing, and downloading
 * community-shared ONNX models.
 */
class ModelMarketplace
{
public:
    ModelMarketplace();
    ~ModelMarketplace();
    
    /**
     * @brief Initialize the marketplace connection
     * @param apiEndpoint API endpoint URL
     * @return true if initialization successful
     */
    bool initialize(const juce::String& apiEndpoint = "");
    
    /**
     * @brief Refresh the model catalog from server
     */
    void refreshCatalog();
    
    /**
     * @brief Search for models
     * @param criteria Search criteria
     */
    void searchModels(const MarketplaceSearchCriteria& criteria);
    
    /**
     * @brief Get all models in catalog
     */
    const std::vector<MarketplaceModelInfo>& getCatalog() const { return catalog; }
    
    /**
     * @brief Get model info by ID
     * @param modelId Model ID
     * @return Pointer to model info or nullptr if not found
     */
    const MarketplaceModelInfo* getModelInfo(const juce::String& modelId) const;
    
    /**
     * @brief Download a model
     * @param modelId Model ID to download
     * @param destinationDir Directory to save model
     * @return true if download started successfully
     */
    bool downloadModel(const juce::String& modelId, const juce::File& destinationDir);
    
    /**
     * @brief Cancel an active download
     * @param modelId Model ID to cancel
     */
    void cancelDownload(const juce::String& modelId);
    
    /**
     * @brief Get download progress
     * @param modelId Model ID
     * @return Download progress info
     */
    DownloadProgress getDownloadProgress(const juce::String& modelId) const;
    
    /**
     * @brief Check if a download is active
     * @param modelId Model ID
     */
    bool isDownloading(const juce::String& modelId) const;
    
    /**
     * @brief Check if a model is installed locally
     * @param modelId Model ID
     */
    bool isModelInstalled(const juce::String& modelId) const;
    
    /**
     * @brief Get local file path for installed model
     * @param modelId Model ID
     * @return File path or empty if not installed
     */
    juce::File getInstalledModelPath(const juce::String& modelId) const;
    
    /**
     * @brief Delete an installed model
     * @param modelId Model ID
     * @return true if deleted successfully
     */
    bool deleteInstalledModel(const juce::String& modelId);
    
    /**
     * @brief Get list of installed model IDs
     */
    juce::StringArray getInstalledModelIds() const;
    
    /**
     * @brief Set the local models directory
     * @param directory Directory for storing models
     */
    void setModelsDirectory(const juce::File& directory);
    
    /**
     * @brief Get the models directory
     */
    juce::File getModelsDirectory() const { return modelsDirectory; }
    
    /**
     * @brief Submit a rating for a model
     * @param modelId Model ID
     * @param rating Rating value (1-5)
     * @param review Optional review text
     */
    void submitRating(const juce::String& modelId, int rating, const juce::String& review = "");
    
    /**
     * @brief Report a model for review
     * @param modelId Model ID
     * @param reason Reason for report
     */
    void reportModel(const juce::String& modelId, const juce::String& reason);
    
    /**
     * @brief Add a listener
     */
    void addListener(ModelMarketplaceListener* listener);
    
    /**
     * @brief Remove a listener
     */
    void removeListener(ModelMarketplaceListener* listener);
    
    /**
     * @brief Check if marketplace is connected
     */
    bool isConnected() const { return connected; }
    
    /**
     * @brief Get category name as string
     */
    static juce::String getCategoryName(ModelCategory category);
    
    /**
     * @brief Format file size for display
     */
    static juce::String formatFileSize(size_t bytes);
    
private:
    juce::String apiEndpoint;
    juce::File modelsDirectory;
    bool connected;
    
    std::vector<MarketplaceModelInfo> catalog;
    std::map<juce::String, DownloadProgress> activeDownloads;
    std::map<juce::String, juce::File> installedModels;
    
    std::vector<ModelMarketplaceListener*> listeners;
    
    juce::CriticalSection marketplaceLock;
    
    /**
     * @brief Internal download task
     */
    class DownloadTask : public juce::ThreadPoolJob
    {
    public:
        DownloadTask(ModelMarketplace& owner, const MarketplaceModelInfo& model, 
                    const juce::File& destDir);
        
        JobStatus runJob() override;
        
    private:
        ModelMarketplace& marketplace;
        MarketplaceModelInfo modelInfo;
        juce::File destinationDir;
    };
    
    juce::ThreadPool downloadPool;
    
    void notifyCatalogRefreshed(int numModels);
    void notifyDownloadStarted(const juce::String& modelId);
    void notifyDownloadProgress(const DownloadProgress& progress);
    void notifyDownloadComplete(const juce::String& modelId, const juce::File& file);
    void notifyDownloadFailed(const juce::String& modelId, const juce::String& error);
    void notifySearchResults(const std::vector<MarketplaceModelInfo>& results);
    
    void loadInstalledModels();
    void saveInstalledModels();
    
    std::vector<MarketplaceModelInfo> generateSampleCatalog();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModelMarketplace)
};

//==============================================================================
/**
 * @brief Model marketplace browser UI component
 */
class ModelMarketplaceBrowser : public juce::Component,
                                 public ModelMarketplaceListener,
                                 public juce::ListBoxModel
{
public:
    ModelMarketplaceBrowser(ModelMarketplace* marketplace);
    ~ModelMarketplaceBrowser() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // ModelMarketplaceListener
    void onCatalogRefreshed(int numModels) override;
    void onDownloadStarted(const juce::String& modelId) override;
    void onDownloadProgress(const DownloadProgress& progress) override;
    void onDownloadComplete(const juce::String& modelId, const juce::File& localFile) override;
    void onDownloadFailed(const juce::String& modelId, const juce::String& error) override;
    void onSearchResults(const std::vector<MarketplaceModelInfo>& results) override;
    
    // ListBoxModel
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, 
                         int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent& event) override;
    
    /**
     * @brief Refresh the display
     */
    void refresh();
    
private:
    ModelMarketplace* marketplace;
    
    juce::Label titleLabel;
    juce::TextEditor searchBox;
    juce::ComboBox categoryFilter;
    juce::ToggleButton verifiedOnlyToggle;
    juce::TextButton refreshButton;
    juce::TextButton downloadButton;
    
    juce::ListBox modelListBox;
    juce::Label statusLabel;
    juce::ProgressBar* downloadProgressBar;
    
    std::vector<MarketplaceModelInfo> displayedModels;
    int selectedModelIndex;
    
    void setupUI();
    void onSearchChanged();
    void onRefreshClicked();
    void onDownloadClicked();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModelMarketplaceBrowser)
};

} // namespace MAEVN
