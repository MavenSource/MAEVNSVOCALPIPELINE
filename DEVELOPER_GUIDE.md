# MAEVN Developer Guide

## Table of Contents
1. [Architecture Overview](#architecture-overview)
2. [Module Documentation](#module-documentation)
3. [Building the Project](#building-the-project)
4. [Testing](#testing)
5. [Contributing](#contributing)
6. [Performance Optimization](#performance-optimization)

## Architecture Overview

MAEVN follows a modular architecture with clear separation between AI/ML inference, DSP processing, and UI components.

### Core Components

```
┌─────────────────────────────────────────────────────────────┐
│                      PluginEditor (UI)                       │
├─────────────────────────────────────────────────────────────┤
│                    PluginProcessor (DSP)                     │
├──────────────┬──────────────┬──────────────┬────────────────┤
│  OnnxEngine  │PatternEngine │ AIFXEngine   │ PresetManager  │
│  (AI/ML)     │ (Timeline)   │ (Effects)    │ (Storage)      │
└──────────────┴──────────────┴──────────────┴────────────────┘
```

### Data Flow

1. **Audio Input** → PluginProcessor::processBlock()
2. **Transport Info** → PatternEngine (BPM, position)
3. **Active Blocks** → PatternEngine → Instrument triggers
4. **AI Inference** → OnnxEngine → Audio generation
5. **FX Processing** → AIFXEngine → Mixed output
6. **Audio Output** → DAW

## Module Documentation

### OnnxEngine

**Purpose:** Manages ONNX Runtime for AI model inference

**Key Features:**
- Thread-safe model loading and inference
- Hot-reloading support
- GPU acceleration support (optional)
- Multiple concurrent models

**Usage Example:**
```cpp
OnnxEngine engine;
engine.initialize();
engine.loadModel("808", "Models/drums/808_ddsp.onnx");

std::vector<float> input = {/* ... */};
std::vector<int64_t> shape = {1, 128};
std::vector<float> output;
engine.runInference("808", input, shape, output);
```

**Performance Considerations:**
- Inference runs on audio thread (real-time safe)
- Models should be optimized for <1ms inference time
- Use quantization for faster inference

### PatternEngine

**Purpose:** Parses stage scripts and manages timeline arrangement

**Key Features:**
- BPM-aware quantization
- Block-based arrangement
- DAW transport sync
- Multiple track support

**Stage Script Format:**
```
[HOOK] Hook lyrics here duration:4.0
[VERSE] Verse lyrics duration:8.0
[808] Bass pattern
[HIHAT] Hi-hat pattern
```

**Usage Example:**
```cpp
PatternEngine engine;
engine.setBPM(140.0);
engine.parseStageScript(scriptText);

auto activeBlocks = engine.getActiveBlocks(currentTime);
```

### AIFXEngine

**Purpose:** Hybrid DSP and AI effects processing

**Key Features:**
- 4 FX modes: Off, DSP, AI, Hybrid
- Per-track effect chains
- Real-time parameter control
- Built-in DSP effects (EQ, compressor, reverb, limiter)

**Usage Example:**
```cpp
AIFXEngine fxEngine(&onnxEngine);
fxEngine.prepare(sampleRate, blockSize);

// Add DSP effects
auto compressor = std::make_unique<CompressorEffect>();
fxEngine.addDSPEffect(trackIndex, std::move(compressor));

// Set mode
fxEngine.setFXMode(trackIndex, FXMode::Hybrid);

// Process
fxEngine.process(audioBuffer, numSamples, trackIndex);
```

### FXPresetManager

**Purpose:** Manages FX preset storage, search, and organization

**Key Features:**
- JSON-based preset format
- Category and tag system
- Full-text search
- Preset import/export

**Preset Structure:**
```json
{
  "name": "RadioVocals",
  "mode": 3,
  "category": "Vocal",
  "tags": ["Radio", "Clean", "Trap"],
  "params": {
    "compressorThreshold": -12.0,
    "eqHighGain": 3.0
  }
}
```

### GlobalUndoManager

**Purpose:** Comprehensive undo/redo system with state snapshots

**Key Features:**
- Transaction support (group multiple actions)
- Jump-to-history
- Configurable history size
- JSON state serialization

**Usage Example:**
```cpp
GlobalUndoManager undoManager;

// Add action
ActionState action(
    ActionState::Type::FXChange,
    "Apply Radio Vocals preset",
    presetData
);
undoManager.addAction(action);

// Undo/Redo
if (undoManager.canUndo())
    undoManager.undo();
```

## Building the Project

### Prerequisites

1. **JUCE Framework 7.0+**
   - Download from: https://juce.com/get-juce
   - Extract to: `C:\JUCE` (Windows) or `/usr/local/JUCE` (Linux/Mac)

2. **ONNX Runtime SDK**
   - Download from: https://github.com/microsoft/onnxruntime/releases
   - Extract to: `C:\onnxruntime` (Windows) or `/usr/local/onnxruntime` (Linux/Mac)

3. **CMake 3.20+**
   - Download from: https://cmake.org/download/

4. **Python 3.10+ (for model export)**
   - Install PyTorch: `pip install torch`
   - Install ONNX: `pip install onnx`

### Build Steps

#### Windows
```batch
# 1. Setup repository
setup_maevn_repo.bat

# 2. Generate ONNX models
build_maevn_onnx.bat

# 3. Configure CMake
cmake -B Build -S . -DJUCE_PATH="C:/JUCE" -DONNXRUNTIME_PATH="C:/onnxruntime"

# 4. Build
cmake --build Build --config Release

# 5. Install (copies to VST3 folder)
cmake --install Build --config Release
```

#### Linux/macOS
```bash
# 1. Setup repository
./setup_maevn_repo.sh

# 2. Generate ONNX models
python3 scripts/export_onnx_models.py

# 3. Configure CMake
cmake -B Build -S . \
    -DJUCE_PATH="/usr/local/JUCE" \
    -DONNXRUNTIME_PATH="/usr/local/onnxruntime"

# 4. Build
cmake --build Build --config Release

# 5. Install
sudo cmake --install Build --config Release
```

### Build Options

**Optimization Flags:**
- `-DCMAKE_BUILD_TYPE=Release` - Release build with optimizations
- `-DCMAKE_BUILD_TYPE=Debug` - Debug build with symbols

**Plugin Formats:**
- Default: VST3 + Standalone
- macOS: Add AU format automatically

## Testing

### Unit Testing

Currently, testing is manual. Future versions will include:
- GoogleTest integration
- Automated preset loading tests
- ONNX inference benchmarks

### Manual Testing Workflow

1. **Load in DAW**
   - Open your DAW (Reaper, Ableton, FL Studio)
   - Load MAEVN.vst3

2. **Test Stage Script Parsing**
   ```
   [HOOK] Test hook duration:2.0
   [VERSE] Test verse duration:4.0
   [808] Bass pattern
   ```

3. **Test FX Presets**
   - Load "RadioVocals" preset
   - Verify FX mode switches
   - Test undo/redo

4. **Performance Check**
   - Monitor CPU usage in DAW
   - Should be <5% at 44.1kHz, 512 samples
   - Check for dropouts

## Contributing

### Code Style

- Follow JUCE coding standards
- Use C++17 features
- Document all public APIs
- Use `namespace MAEVN` for all code

### Pull Request Process

1. Fork repository
2. Create feature branch: `feature/your-feature-name`
3. Implement feature with tests
4. Update documentation
5. Submit PR with description

### Commit Message Format

```
feat: add AI vocal synthesis
fix: resolve buffer underrun in processBlock
docs: update README with build instructions
refactor: simplify OnnxEngine inference
perf: optimize DSP effect processing
```

## Performance Optimization

### Real-Time Audio Processing

**Critical Rules:**
1. No memory allocation on audio thread
2. No file I/O on audio thread
3. No mutex locks that can block
4. Use lock-free structures where possible

**Buffer Sizes:**
- Design for 64-2048 sample buffers
- Test at 44.1kHz and 48kHz
- Target <1ms processing per block

### ONNX Model Optimization

**Model Size Guidelines:**
- Instruments: <5MB per model
- FX models: <10MB per model
- Vocals: <50MB for TTS+vocoder

**Inference Optimization:**
```python
# Quantize model to INT8
from onnxruntime.quantization import quantize_dynamic
quantize_dynamic(
    model_input="model.onnx",
    model_output="model_int8.onnx",
    weight_type=QuantType.QInt8
)
```

**Graph Optimization:**
- Enable constant folding
- Use ONNX Runtime optimizations
- Profile with ONNX Runtime profiler

### Memory Management

**Buffer Pooling:**
```cpp
// Pre-allocate buffers
std::array<juce::AudioBuffer<float>, 6> trackBuffers;

void prepareToPlay(double sampleRate, int samplesPerBlock)
{
    for (auto& buffer : trackBuffers)
        buffer.setSize(2, samplesPerBlock);
}
```

**Smart Pointers:**
- Use `std::unique_ptr` for ownership
- Use `std::shared_ptr` for shared resources
- Avoid raw pointers

### Profiling Tools

**Recommended Tools:**
- Windows: Visual Studio Profiler
- macOS: Instruments (Xcode)
- Linux: Valgrind, perf

**Profile Targets:**
- `processBlock()` time
- ONNX inference time
- FX processing time
- Memory allocations

## Advanced Topics

### Custom ONNX Models

To use custom models:

1. Export model to ONNX (opset 11+)
2. Place in `Models/` directory
3. Update `Models/config.json`
4. Reload in plugin

**Model Requirements:**
- Input: Float tensor (varying shapes supported)
- Output: Float tensor (audio samples)
- No external dependencies

### Extending FX Chain

To add new effects:

1. Implement `Effect` interface:
```cpp
class MyCustomEffect : public Effect
{
public:
    void process(juce::AudioBuffer<float>& buffer, int numSamples) override;
    void prepare(double sampleRate, int maxBlockSize) override;
    void reset() override;
    juce::String getName() const override { return "MyEffect"; }
};
```

2. Register with AIFXEngine:
```cpp
auto effect = std::make_unique<MyCustomEffect>();
aiFXEngine.addDSPEffect(trackIndex, std::move(effect));
```

### Custom UI Themes

UI components use JUCE LookAndFeel:

```cpp
class MAEVNLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawButtonBackground(juce::Graphics& g, 
                            juce::Button& button,
                            const juce::Colour& backgroundColour,
                            bool shouldDrawButtonAsHighlighted,
                            bool shouldDrawButtonAsDown) override
    {
        // Custom drawing code
    }
};
```

## Troubleshooting

### Common Issues

**1. ONNX Runtime Not Found**
```
Solution: Set ONNXRUNTIME_PATH environment variable or pass to CMake
```

**2. Models Not Loading**
```
Check: Models/config.json paths are relative to Models directory
Verify: .onnx files exist and are valid
```

**3. High CPU Usage**
```
Profile: Check which component is bottleneck
Optimize: Reduce model complexity or use quantization
```

**4. Audio Dropouts**
```
Increase: Buffer size in DAW
Check: No file I/O or allocations on audio thread
Disable: AI processing temporarily to isolate issue
```

## Resources

- **JUCE Documentation:** https://docs.juce.com/
- **ONNX Runtime:** https://onnxruntime.ai/docs/
- **DDSP Research:** https://github.com/magenta/ddsp
- **Audio Programming:** https://www.musicdsp.org/

## License

This project is under development. License TBD.

## Contact

For questions and support, please open an issue on GitHub.
