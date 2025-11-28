/**
 * @file PresetPackManager.h
 * @brief Community Preset Packs - Import/export preset bundles
 * 
 * This module provides functionality for creating, importing, and exporting
 * preset bundles that can be shared within the community.
 */

#pragma once

#include <JuceHeader.h>
#include <memory>
#include <vector>
#include "Utilities.h"
#include "FXPreset.h"
#include "FXPresetManager.h"
#include "InstrumentSequencer.h"

namespace MAEVN
{

//==============================================================================
/**
 * @brief Preset pack metadata
 */
struct PresetPackInfo
{
    juce::String name;              ///< Pack name
    juce::String author;            ///< Author name
    juce::String description;       ///< Pack description
    juce::String version;           ///< Version string
    juce::Time createdDate;         ///< Creation date
    juce::Time modifiedDate;        ///< Last modified date
    juce::StringArray tags;         ///< Tags for categorization
    juce::String license;           ///< License information
    juce::String website;           ///< Author website/link
    int numPresets;                 ///< Number of presets in pack
    int numPatterns;                ///< Number of sequencer patterns
    juce::String thumbnailPath;     ///< Path to thumbnail image
    
    PresetPackInfo()
        : version("1.0.0")
        , numPresets(0)
        , numPatterns(0)
    {}
};

//==============================================================================
/**
 * @brief Represents a complete preset pack
 */
class PresetPack
{
public:
    PresetPack();
    ~PresetPack();
    
    /**
     * @brief Set pack info
     */
    void setInfo(const PresetPackInfo& info);
    
    /**
     * @brief Get pack info
     */
    const PresetPackInfo& getInfo() const { return packInfo; }
    
    /**
     * @brief Add an FX preset to the pack
     * @param preset The preset to add
     */
    void addFXPreset(const FXPreset& preset);
    
    /**
     * @brief Add a sequencer pattern to the pack
     * @param pattern The pattern to add
     */
    void addSequencerPattern(const SequencerPattern& pattern);
    
    /**
     * @brief Get all FX presets
     */
    const std::vector<FXPreset>& getFXPresets() const { return fxPresets; }
    
    /**
     * @brief Get all sequencer patterns
     */
    const std::vector<SequencerPattern>& getSequencerPatterns() const { return sequencerPatterns; }
    
    /**
     * @brief Remove an FX preset by index
     */
    void removeFXPreset(int index);
    
    /**
     * @brief Remove a sequencer pattern by index
     */
    void removeSequencerPattern(int index);
    
    /**
     * @brief Clear all contents
     */
    void clear();
    
    /**
     * @brief Save pack to file (.maevnpack)
     * @param file The destination file
     * @return true if successful
     */
    bool saveToFile(const juce::File& file) const;
    
    /**
     * @brief Load pack from file
     * @param file The source file
     * @return true if successful
     */
    bool loadFromFile(const juce::File& file);
    
    /**
     * @brief Export pack to JSON
     * @return JSON representation
     */
    juce::var toJSON() const;
    
    /**
     * @brief Import pack from JSON
     * @param json JSON data
     * @return true if successful
     */
    bool fromJSON(const juce::var& json);
    
    /**
     * @brief Validate pack integrity
     * @return true if pack is valid
     */
    bool validate() const;
    
private:
    PresetPackInfo packInfo;
    std::vector<FXPreset> fxPresets;
    std::vector<SequencerPattern> sequencerPatterns;
    
    // Pack file magic number and version
    static constexpr juce::uint32 PACK_MAGIC = 0x4D564E50; // "MVNP"
    static constexpr juce::uint32 PACK_VERSION = 1;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetPack)
};

//==============================================================================
/**
 * @brief Listener interface for preset pack events
 */
class PresetPackListener
{
public:
    virtual ~PresetPackListener() = default;
    
    /**
     * @brief Called when a pack is imported
     */
    virtual void onPackImported(const PresetPackInfo& info) = 0;
    
    /**
     * @brief Called when a pack is exported
     */
    virtual void onPackExported(const PresetPackInfo& info) = 0;
    
    /**
     * @brief Called when pack import fails
     */
    virtual void onPackImportFailed(const juce::String& error) = 0;
    
    /**
     * @brief Called during import/export progress
     */
    virtual void onPackProgress(float progress, const juce::String& status) = 0;
};

//==============================================================================
/**
 * @brief Main Preset Pack Manager
 * 
 * Handles importing, exporting, and managing preset packs
 */
class PresetPackManager
{
public:
    PresetPackManager(FXPresetManager* presetManager);
    ~PresetPackManager();
    
    /**
     * @brief Import a preset pack from file
     * @param file The pack file to import
     * @param mergeWithExisting If true, merge with existing presets
     * @return true if successful
     */
    bool importPack(const juce::File& file, bool mergeWithExisting = true);
    
    /**
     * @brief Export selected presets as a pack
     * @param presetIndices Indices of presets to include
     * @param patterns Sequencer patterns to include
     * @param info Pack metadata
     * @param file Destination file
     * @return true if successful
     */
    bool exportPack(const std::vector<int>& presetIndices,
                   const std::vector<SequencerPattern>& patterns,
                   const PresetPackInfo& info,
                   const juce::File& file);
    
    /**
     * @brief Export all presets as a pack
     * @param info Pack metadata
     * @param file Destination file
     * @return true if successful
     */
    bool exportAllPresets(const PresetPackInfo& info, const juce::File& file);
    
    /**
     * @brief Get list of available pack files in a directory
     * @param directory Directory to scan
     * @return Array of pack file paths
     */
    juce::StringArray scanForPacks(const juce::File& directory);
    
    /**
     * @brief Get pack info without fully loading
     * @param file The pack file
     * @param outInfo Info structure to fill
     * @return true if info was read successfully
     */
    bool getPackInfo(const juce::File& file, PresetPackInfo& outInfo);
    
    /**
     * @brief Get list of installed packs
     */
    const std::vector<PresetPackInfo>& getInstalledPacks() const { return installedPacks; }
    
    /**
     * @brief Uninstall a pack
     * @param packName Name of pack to remove
     * @return true if successful
     */
    bool uninstallPack(const juce::String& packName);
    
    /**
     * @brief Set the packs directory
     * @param directory Directory for storing packs
     */
    void setPacksDirectory(const juce::File& directory);
    
    /**
     * @brief Get the packs directory
     */
    juce::File getPacksDirectory() const { return packsDirectory; }
    
    /**
     * @brief Add a listener
     */
    void addListener(PresetPackListener* listener);
    
    /**
     * @brief Remove a listener
     */
    void removeListener(PresetPackListener* listener);
    
    /**
     * @brief Refresh list of installed packs
     */
    void refreshInstalledPacks();
    
    /**
     * @brief Create a pack from the current session
     * @param info Pack metadata
     * @return The created pack
     */
    std::unique_ptr<PresetPack> createPackFromSession(const PresetPackInfo& info);
    
    /**
     * @brief Validate a pack file
     * @param file The pack file to validate
     * @return true if valid
     */
    bool validatePackFile(const juce::File& file);
    
    /**
     * @brief Get default packs directory
     * @return Default packs directory path
     */
    static juce::File getDefaultPacksDirectory();
    
private:
    FXPresetManager* presetManager;
    juce::File packsDirectory;
    std::vector<PresetPackInfo> installedPacks;
    std::vector<PresetPackListener*> listeners;
    
    juce::CriticalSection packLock;
    
    void notifyPackImported(const PresetPackInfo& info);
    void notifyPackExported(const PresetPackInfo& info);
    void notifyPackImportFailed(const juce::String& error);
    void notifyProgress(float progress, const juce::String& status);
    
    bool extractPackToPresets(const PresetPack& pack, bool merge);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetPackManager)
};

//==============================================================================
/**
 * @brief UI Component for browsing and managing preset packs
 */
class PresetPackBrowserComponent : public juce::Component,
                                    public juce::ListBoxModel
{
public:
    PresetPackBrowserComponent(PresetPackManager* manager);
    ~PresetPackBrowserComponent() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // ListBoxModel interface
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, 
                         int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent& event) override;
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent& event) override;
    
    /**
     * @brief Refresh the pack list
     */
    void refresh();
    
    /**
     * @brief Show import dialog
     */
    void showImportDialog();
    
    /**
     * @brief Show export dialog
     */
    void showExportDialog();
    
private:
    PresetPackManager* packManager;
    
    juce::ListBox packListBox;
    juce::TextButton importButton;
    juce::TextButton exportButton;
    juce::TextButton refreshButton;
    juce::Label titleLabel;
    
    std::vector<PresetPackInfo> displayedPacks;
    int selectedPackIndex;
    
    void setupUI();
    void onImportClicked();
    void onExportClicked();
    void onRefreshClicked();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetPackBrowserComponent)
};

} // namespace MAEVN
