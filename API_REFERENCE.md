# MAEVN API Reference

## Table of Contents
- [OnnxEngine](#onnxengine)
- [PatternEngine](#patternengine)
- [AIFXEngine](#aifxengine)
- [FXPreset](#fxpreset)
- [FXPresetManager](#fxpresetmanager)
- [GlobalUndoManager](#globalundomanager)
- [Utilities](#utilities)

---

## OnnxEngine

**Header:** `OnnxEngine.h`

**Description:** Manages ONNX Runtime for AI model inference with support for multiple concurrent models and hot-reloading.

### Classes

#### `OnnxEngine`

Main engine for managing ONNX models.

##### Public Methods

```cpp
bool initialize()
```
Initialize ONNX Runtime environment.

**Returns:** `true` if successful

---

```cpp
bool loadModel(const juce::String& role, const juce::String& modelPath)
```
Load an ONNX model with a specific role identifier.

**Parameters:**
- `role` - Model identifier (e.g., "808", "vocal_tts")
- `modelPath` - Full path to .onnx file

**Returns:** `true` if loaded successfully

---

```cpp
bool runInference(const juce::String& role,
                 const std::vector<float>& inputData,
                 const std::vector<int64_t>& inputShape,
                 std::vector<float>& outputData)
```
Run inference using a specific model.

**Parameters:**
- `role` - Model identifier
- `inputData` - Input tensor data (flattened)
- `inputShape` - Shape of input tensor (e.g., {1, 128})
- `outputData` - Output tensor data (populated by function)

**Returns:** `true` if inference succeeded

**Thread Safety:** Thread-safe with internal locking

---

```cpp
int loadModelsFromConfig(const juce::String& configPath)
```
Load all models from config.json file.

**Parameters:**
- `configPath` - Path to config.json

**Returns:** Number of models loaded

**Example config.json:**
```json
{
  "808": "drums/808_ddsp.onnx",
  "hihat": "drums/hihat_ddsp.onnx"
}
```

---

```cpp
bool reloadModel(const juce::String& role)
```
Hot reload a specific model from disk.

**Parameters:**
- `role` - Model identifier to reload

**Returns:** `true` if reloaded successfully

---

```cpp
bool isModelReady(const juce::String& role) const
```
Check if a model is loaded and ready for inference.

---

```cpp
void unloadModel(const juce::String& role)
```
Unload a specific model and free resources.

---

```cpp
juce::StringArray getLoadedModels() const
```
Get list of all loaded model identifiers.

---

```cpp
void setUseGPU(bool useGPU)
```
Enable/disable GPU acceleration if available.

---

### Usage Example

```cpp
// Initialize
OnnxEngine engine;
engine.initialize();

// Load model
engine.loadModel("808", "Models/drums/808_ddsp.onnx");

// Run inference
std::vector<float> input(128, 0.5f);
std::vector<int64_t> shape = {1, 128};
std::vector<float> output;

if (engine.runInference("808", input, shape, output))
{
    // Process output
}
```

---

## PatternEngine

**Header:** `PatternEngine.h`

**Description:** Manages timeline arrangement and stage script parsing with BPM-aware quantization.

### Classes

#### `PatternEngine`

Timeline and arrangement engine.

##### Public Methods

```cpp
int parseStageScript(const juce::String& scriptInput)
```
Parse stage script input into timeline blocks.

**Parameters:**
- `scriptInput` - Text with [HOOK], [VERSE], [808] etc.

**Returns:** Number of blocks parsed

**Example:**
```cpp
juce::String script = 
    "[HOOK] Hook lyrics duration:4.0\n"
    "[VERSE] Verse lyrics duration:8.0\n"
    "[808] Bass pattern";

int numBlocks = engine.parseStageScript(script);
```

---

```cpp
const std::vector<TimelineBlock>& getBlocks() const
```
Get all timeline blocks.

---

```cpp
std::vector<TimelineBlock> getActiveBlocks(double time) const
```
Get blocks active at a specific time.

**Parameters:**
- `time` - Time in seconds

**Returns:** Vector of active blocks

---

```cpp
std::vector<TimelineBlock> getBlocksForTrack(int trackIndex) const
```
Get all blocks for a specific track.

**Parameters:**
- `trackIndex` - Track/lane index (0-5)

---

```cpp
void setBPM(double bpm)
```
Set BPM for quantization.

---

```cpp
double getBPM() const
```
Get current BPM.

---

```cpp
double quantizeTime(double time) const
```
Quantize time to nearest beat.

---

```cpp
double beatsToSeconds(double beats) const
double secondsToBeats(double seconds) const
```
Convert between beats and seconds.

---

```cpp
void updateTransport(bool isPlaying, double position)
```
Update DAW transport information.

---

```cpp
void setQuantizationEnabled(bool enabled)
```
Enable/disable auto-quantization.

---

### TimelineBlock Structure

```cpp
struct TimelineBlock
{
    BlockType type;           // Block type (HOOK, VERSE, 808, etc.)
    double startTime;         // Start time in seconds
    double duration;          // Duration in seconds
    juce::String content;     // Text content or parameters
    int trackIndex;           // Track/lane index (0-5)
};
```

---

## AIFXEngine

**Header:** `AIFXEngine.h`

**Description:** Hybrid DSP and AI effects processing engine with per-track effect chains.

### Classes

#### `AIFXEngine`

Main FX engine.

##### Public Methods

```cpp
AIFXEngine(OnnxEngine* engine)
```
Constructor.

**Parameters:**
- `engine` - Pointer to OnnxEngine for AI effects

---

```cpp
void prepare(double sampleRate, int maxBlockSize)
```
Prepare for playback. Call before processing.

---

```cpp
void process(juce::AudioBuffer<float>& buffer, int numSamples, int trackIndex)
```
Process audio with current FX chain.

**Parameters:**
- `buffer` - Audio buffer to process (in-place)
- `numSamples` - Number of samples to process
- `trackIndex` - Track index (0-5)

---

```cpp
void setFXMode(int trackIndex, FXMode mode)
```
Set FX mode for a track.

**Parameters:**
- `trackIndex` - Track index (0-5)
- `mode` - FXMode::Off, DSP, AI, or Hybrid

---

```cpp
FXMode getFXMode(int trackIndex) const
```
Get current FX mode for a track.

---

```cpp
void addDSPEffect(int trackIndex, std::unique_ptr<Effect> effect)
```
Add DSP effect to track chain.

**Example:**
```cpp
auto compressor = std::make_unique<CompressorEffect>();
compressor->setThreshold(-12.0f);
compressor->setRatio(4.0f);
fxEngine.addDSPEffect(trackIndex, std::move(compressor));
```

---

```cpp
void addAIEffect(int trackIndex, const juce::String& modelRole)
```
Add AI effect to track chain.

---

```cpp
void clearEffects(int trackIndex)
```
Clear all effects from track.

---

### Effect Classes

#### `CompressorEffect`

**Methods:**
```cpp
void setThreshold(float dB)      // Default: -10.0
void setRatio(float ratio)        // Default: 4.0
void setAttack(float ms)          // Default: 5.0
void setRelease(float ms)         // Default: 100.0
```

#### `EQEffect`

**Methods:**
```cpp
void setLowGain(float dB)         // 200 Hz shelf
void setMidGain(float dB)         // 1000 Hz peak
void setHighGain(float dB)        // 8000 Hz shelf
```

#### `ReverbEffect`

**Methods:**
```cpp
void setRoomSize(float size)      // 0.0-1.0
void setDamping(float damping)    // 0.0-1.0
void setWetLevel(float level)     // 0.0-1.0
void setDryLevel(float level)     // 0.0-1.0
```

#### `LimiterEffect`

**Methods:**
```cpp
void setThreshold(float dB)       // Default: -1.0
void setRelease(float ms)         // Default: 50.0
```

---

## FXPreset

**Header:** `FXPreset.h`

**Description:** FX preset data structure with JSON serialization.

### Classes

#### `FXPreset`

##### Public Methods

```cpp
void setName(const juce::String& name)
juce::String getName() const
```

---

```cpp
void setMode(FXMode mode)
FXMode getMode() const
```

---

```cpp
void setCategory(const juce::String& category)
juce::String getCategory() const
```

---

```cpp
void addTag(const juce::String& tag)
void removeTag(const juce::String& tag)
bool hasTag(const juce::String& tag) const
juce::StringArray getTags() const
```

---

```cpp
void setParameter(const juce::String& paramName, float value)
float getParameter(const juce::String& paramName, float defaultValue = 0.0f) const
const std::map<juce::String, float>& getParameters() const
```

---

```cpp
juce::var toJSON() const
bool fromJSON(const juce::var& json)
```

---

```cpp
bool saveToFile(const juce::File& file) const
bool loadFromFile(const juce::File& file)
```

---

### Example

```cpp
FXPreset preset;
preset.setName("MyPreset");
preset.setCategory("Vocal");
preset.setMode(FXMode::Hybrid);
preset.addTag("Trap");
preset.addTag("Clean");
preset.setParameter("compressorThreshold", -12.0f);
preset.setParameter("eqHighGain", 3.0f);

juce::File file("MyPreset.json");
preset.saveToFile(file);
```

---

## FXPresetManager

**Header:** `FXPresetManager.h`

**Description:** Manages preset collection with search and filtering.

### Classes

#### `FXPresetManager`

##### Public Methods

```cpp
int loadPresetsFromDirectory(const juce::File& directory)
```
Load all .json presets from directory.

**Returns:** Number of presets loaded

---

```cpp
bool savePreset(const FXPreset& preset, const juce::File& directory)
```
Save preset to directory.

---

```cpp
void addPreset(const FXPreset& preset)
```
Add preset to collection.

---

```cpp
const FXPreset* getPreset(int index) const
```
Get preset by index.

---

```cpp
int getNumPresets() const
```
Get number of presets.

---

```cpp
std::vector<int> searchPresets(const juce::String& searchTerm) const
```
Search presets by name or tag.

**Returns:** Indices of matching presets

---

```cpp
std::vector<int> filterByCategory(const juce::String& category) const
std::vector<int> filterByTag(const juce::String& tag) const
```
Filter presets by category or tag.

---

```cpp
juce::StringArray getAllCategories() const
juce::StringArray getAllTags() const
```
Get all unique categories and tags.

---

```cpp
const FXPreset* getPresetByName(const juce::String& name) const
bool hasPreset(const juce::String& name) const
```

---

### Example

```cpp
FXPresetManager manager;
manager.loadPresetsFromDirectory(juce::File("Presets"));

// Search
auto results = manager.searchPresets("trap");
for (int idx : results)
{
    auto* preset = manager.getPreset(idx);
    DBG(preset->getName());
}

// Filter
auto vocalPresets = manager.filterByCategory("Vocal");
```

---

## GlobalUndoManager

**Header:** `GlobalUndoManager.h`

**Description:** Global undo/redo system with state snapshots.

### Classes

#### `GlobalUndoManager`

##### Public Methods

```cpp
void addAction(const ActionState& action)
```
Add action to history.

---

```cpp
bool undo()
bool redo()
```
Undo/redo last action.

**Returns:** `true` if successful

---

```cpp
bool canUndo() const
bool canRedo() const
```
Check if undo/redo is available.

---

```cpp
juce::String getUndoDescription() const
juce::String getRedoDescription() const
```
Get description of next undo/redo action.

---

```cpp
bool jumpToHistoryIndex(int index)
```
Jump to specific position in history.

---

```cpp
const std::vector<ActionState>& getHistory() const
int getCurrentHistoryIndex() const
```

---

```cpp
void beginTransaction(const juce::String& description)
void endTransaction()
bool isInTransaction() const
```
Group multiple actions into single undo operation.

---

```cpp
void setMaxHistorySize(int maxSize)
```
Set maximum history size (default: 100).

---

### ActionState Structure

```cpp
struct ActionState
{
    enum class Type {
        FXChange,
        TimelineChange,
        ModelChange,
        ArrangementChange,
        PresetChange
    };
    
    Type type;
    juce::String description;
    juce::var stateData;         // JSON state
    juce::Time timestamp;
};
```

---

### Example

```cpp
GlobalUndoManager undoManager;

// Add action
ActionState action(
    ActionState::Type::FXChange,
    "Apply compressor",
    juce::var(/* state data */)
);
undoManager.addAction(action);

// Transaction
undoManager.beginTransaction("Complex operation");
undoManager.addAction(action1);
undoManager.addAction(action2);
undoManager.endTransaction();  // Creates single compound action

// Undo/Redo
if (undoManager.canUndo())
    undoManager.undo();
```

---

## Utilities

**Header:** `Utilities.h`

**Description:** Shared utilities, constants, and helper functions.

### Enums

```cpp
enum class BlockType
{
    Unknown, Intro, Hook, Verse, Bridge, Outro,
    Drum_808, Drum_HiHat, Drum_Snare,
    Instrument_Piano, Instrument_Synth, Vocal
};

enum class FXMode
{
    Off = 0, DSP = 1, AI = 2, Hybrid = 3
};

enum class InstrumentType
{
    Bass808, HiHat, Snare, Piano, Synth, Vocal, Unknown
};
```

### Functions

```cpp
BlockType stringToBlockType(const juce::String& str)
juce::String blockTypeToString(BlockType type)
```
Convert between string and BlockType.

---

```cpp
template<typename T>
T clamp(T value, T minValue, T maxValue)
```
Clamp value between min and max.

---

```cpp
float dBToGain(float dB)
float gainTodB(float gain)
```
Convert between dB and linear gain.

---

### Logger

```cpp
class Logger
{
    enum class Level { Debug, Info, Warning, Error };
    static void log(Level level, const juce::String& message);
};
```

**Example:**
```cpp
Logger::log(Logger::Level::Info, "Model loaded successfully");
Logger::log(Logger::Level::Error, "Failed to load model");
```

---

## Constants

```cpp
namespace MAEVN
{
    constexpr const char* VERSION = "1.0.0";
    constexpr const char* PLUGIN_NAME = "MAEVN";
    
    constexpr int MAX_BUFFER_SIZE = 4096;
    constexpr int DEFAULT_SAMPLE_RATE = 44100;
    constexpr int MAX_CHANNELS = 2;
    constexpr double PI = 3.14159265358979323846;
    constexpr double TWO_PI = 2.0 * PI;
    
    constexpr const char* MODELS_DIR = "Models/";
    constexpr const char* PRESETS_DIR = "Presets/";
    constexpr const char* CONFIG_FILE = "config.json";
}
```

---

## Thread Safety

### Audio Thread Safe
- `OnnxEngine::runInference()` - with internal locking
- `PatternEngine::getActiveBlocks()` - with internal locking
- `AIFXEngine::process()` - with internal locking

### GUI Thread Only
- `FXPresetManager` - all methods
- `GlobalUndoManager` - all methods (except query methods)
- UI components

### Best Practices
```cpp
// From audio thread
{
    const juce::ScopedLock sl(lock);
    // Quick read of shared data
}

// From GUI thread
juce::MessageManager::callAsync([this]()
{
    // Update UI
});
```

---

## Error Handling

All functions return `bool` for success/failure or throw exceptions that are caught internally.

**Pattern:**
```cpp
if (!engine.loadModel("808", path))
{
    Logger::log(Logger::Level::Error, "Model load failed");
    // Handle error
}
```

---

## Memory Management

- Use RAII (Resource Acquisition Is Initialization)
- Prefer `std::unique_ptr` and `std::shared_ptr`
- No raw `new`/`delete` in public APIs
- Pre-allocate buffers in `prepareToPlay()`

---

## Version Compatibility

**Current Version:** 1.0.0

**ONNX Opset:** 11+  
**JUCE Version:** 7.0+  
**C++ Standard:** C++17

---

## Support

For API questions:
- GitHub Issues
- Developer Guide
- Architecture Documentation
- Source code comments

---

*Last updated: 2024*
