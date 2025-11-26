# MAEVN Architecture Documentation

## System Overview

MAEVN is a JUCE-based VST3 plugin that combines traditional DSP effects with AI-powered audio processing using ONNX Runtime. The architecture is designed for real-time audio processing with minimal latency.

## High-Level Architecture

```
┌───────────────────────────────────────────────────────────────────┐
│                         VST3 Host (DAW)                            │
└───────────────┬──────────────────────────────────────┬────────────┘
                │                                      │
        ┌───────▼────────┐                    ┌───────▼────────┐
        │  Audio Thread  │                    │   GUI Thread   │
        └───────┬────────┘                    └───────┬────────┘
                │                                      │
    ┌───────────▼────────────────┐          ┌────────▼─────────┐
    │   PluginProcessor          │◄─────────┤  PluginEditor    │
    │   - processBlock()         │          │  - UI Components │
    │   - getStateInformation()  │          │  - User Input    │
    └───┬────────┬────────┬──────┘          └──────────────────┘
        │        │        │
    ┌───▼────┐ ┌▼───────┐ ┌▼──────────┐
    │ ONNX   │ │Pattern │ │  AIFXEngine│
    │Engine  │ │Engine  │ │           │
    └────────┘ └────────┘ └───────────┘
```

## Component Architecture

### 1. PluginProcessor (Audio Processing Core)

**Responsibilities:**
- VST3 interface implementation
- Audio buffer management
- DAW transport synchronization
- State persistence

**Threading Model:**
- Runs on DAW's audio thread
- Must be real-time safe (no allocations, no blocking)
- Uses lock-free communication with UI thread

**Key Methods:**
```cpp
void prepareToPlay(double sampleRate, int samplesPerBlock)
void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
void getStateInformation(juce::MemoryBlock& destData)
void setStateInformation(const void* data, int sizeInBytes)
```

### 2. OnnxEngine (AI/ML Inference)

**Architecture:**
```
┌─────────────────────────────────────────────┐
│           OnnxEngine                        │
├─────────────────────────────────────────────┤
│  Models Map: String → OnnxModel*           │
│  ├─ "808" → OnnxModel                      │
│  ├─ "hihat" → OnnxModel                    │
│  ├─ "vocal_tts" → OnnxModel                │
│  └─ ...                                     │
├─────────────────────────────────────────────┤
│  Each OnnxModel contains:                  │
│  ├─ Ort::Session (ONNX Runtime)            │
│  ├─ Input/Output tensor info                │
│  └─ Thread safety (CriticalSection)        │
└─────────────────────────────────────────────┘
```

**Inference Pipeline:**
1. Load model from .onnx file
2. Create ONNX Runtime session
3. Query input/output tensor shapes
4. On inference:
   - Create input tensor from audio data
   - Run session inference
   - Extract output tensor
   - Convert back to audio

**Optimization Strategies:**
- Graph optimization enabled
- Single thread per inference (real-time constraint)
- Optional GPU acceleration via DirectML/CUDA
- Model quantization support

### 3. PatternEngine (Timeline Management)

**Data Structures:**
```cpp
struct TimelineBlock {
    BlockType type;           // HOOK, VERSE, 808, etc.
    double startTime;         // seconds
    double duration;          // seconds
    juce::String content;     // lyrics or parameters
    int trackIndex;           // which track (0-5)
};

std::vector<TimelineBlock> blocks;  // All timeline blocks
double currentBPM;                   // Current BPM
double playheadPosition;             // Current position in seconds
```

**Parsing Algorithm:**
1. Split input into lines
2. Find `[TAG]` markers
3. Extract content after tag
4. Quantize timing to BPM grid
5. Assign to track based on type
6. Store in blocks vector

**Query Operations:**
- `getActiveBlocks(time)` - O(n) linear scan
- `getBlocksForTrack(index)` - O(n) linear scan
- Future: Use spatial data structure for O(log n)

### 4. AIFXEngine (Effects Processing)

**Architecture:**
```
┌─────────────────────────────────────────────────┐
│              AIFXEngine                         │
├─────────────────────────────────────────────────┤
│  Track Array [6]:                               │
│  ├─ Track 0 (Vocals)                            │
│  │   ├─ FXMode: Hybrid                          │
│  │   ├─ DSP Effects: [Compressor, EQ, Reverb]  │
│  │   └─ AI Effects: [AI_Autotune]              │
│  ├─ Track 1 (808)                               │
│  │   ├─ FXMode: DSP                             │
│  │   └─ DSP Effects: [EQ, Limiter]             │
│  └─ ...                                          │
└─────────────────────────────────────────────────┘
```

**FX Processing Flow:**

**Mode: Off**
```
Input → [Pass through] → Output
```

**Mode: DSP**
```
Input → DSP Effect 1 → DSP Effect 2 → ... → Output
```

**Mode: AI**
```
Input → AI Effect 1 → AI Effect 2 → ... → Output
```

**Mode: Hybrid**
```
Input → DSP Effects Chain → AI Effects Chain → Output
```

**Built-in DSP Effects:**

1. **Compressor** - `juce::dsp::Compressor`
   - Threshold, ratio, attack, release
   - Lookahead: 5ms
   - Knee: soft

2. **EQ** - 3-band parametric
   - Low shelf: 200 Hz
   - Mid peak: 1000 Hz
   - High shelf: 8000 Hz

3. **Reverb** - `juce::dsp::Reverb`
   - Room size, damping, wet/dry
   - Stereo width

4. **Limiter** - `juce::dsp::Limiter`
   - Threshold, release
   - Brick-wall limiting

### 4.1. CinematicAudioEnhancer (Grammy-Quality Processing)

**Purpose:**
The CinematicAudioEnhancer module provides comprehensive audio enhancement focusing on:
- Cinematic tone and emotional depth
- Viral mass appeal processing
- Grammy-nominated quality audio production

**Architecture:**
```
┌───────────────────────────────────────────────────────────────────┐
│                   CinematicAudioEnhancer                          │
├───────────────────────────────────────────────────────────────────┤
│  Vocal Processing Chain:                                          │
│  ├─ HighPassFilter (80 Hz cutoff for vocal clarity)              │
│  ├─ PresenceEQ (3-5 kHz boost for presence)                      │
│  ├─ GentleCompressor (2:1 ratio for natural dynamics)            │
│  ├─ CinematicReverb (large hall with pre-delay)                  │
│  └─ SubtleDelay (quarter-note for depth)                         │
├───────────────────────────────────────────────────────────────────┤
│  Multi-FX Automation:                                              │
│  ├─ ModulationEffect (chorus/flanger for lush sound)             │
│  └─ WarmSaturation (soft saturation for richness)                │
├───────────────────────────────────────────────────────────────────┤
│  Mastering Chain:                                                  │
│  ├─ MultibandCompressor (3-band: low/mid/high)                   │
│  ├─ StereoImager (width control, bass mono)                      │
│  ├─ LoudnessNormalizer (target -14 LUFS)                         │
│  └─ FinalLimiter (-0.1 dB ceiling)                               │
└───────────────────────────────────────────────────────────────────┘
```

**Processing Signal Flow:**
```
Input Audio
    │
    ▼
┌──────────────────────┐
│  Vocal Processing    │
│  (HP → EQ → Comp →   │
│   Reverb → Delay)    │
└──────────┬───────────┘
           │
           ▼
┌──────────────────────┐
│  Multi-FX            │
│  (Modulation →       │
│   Saturation)        │
└──────────┬───────────┘
           │
           ▼
┌──────────────────────┐
│  Mastering Chain     │
│  (Multiband Comp →   │
│   Stereo → Loudness  │
│   → Limiter)         │
└──────────┬───────────┘
           │
           ▼
    Output Audio
```

**Key Components:**

1. **HighPassFilter** - Removes low frequencies below 80 Hz
   - Cleans up vocal tracks
   - Reduces mud in the mix

2. **PresenceEQ** - Boosts 3-5 kHz range
   - Improves clarity and intelligibility
   - Helps vocals cut through the mix

3. **GentleCompressor** - 2:1 ratio compression
   - Evens out dynamics naturally
   - Maintains expressive performance

4. **CinematicReverb** - Large hall reverb
   - Adjustable pre-delay for clarity
   - Creates sense of space
   - Default: 0.8 room size, 30ms pre-delay

5. **SubtleDelay** - Quarter-note delay
   - Adds depth without cluttering
   - BPM-synced option available

6. **ModulationEffect** - Chorus/Flanger
   - Creates lush, expansive sound
   - Rate, depth, and mix controls

7. **WarmSaturation** - Soft clipping
   - Adds warmth and harmonic richness
   - Uses tanh saturation curve

8. **MultibandCompressor** - 3-band dynamics
   - Independent control per band
   - Crossovers at 200 Hz and 4000 Hz

9. **StereoImager** - Width control
   - Widens stereo field
   - Keeps low frequencies centered

10. **LoudnessNormalizer** - LUFS targeting
    - Default target: -14 LUFS
    - Streaming platform compatible

11. **FinalLimiter** - Brick-wall limiting
    - -0.1 dB ceiling (prevents clipping)
    - Fast release for transparent limiting

**Presets:**

1. **Cinematic Vocal Preset** - Grammy-quality vocal processing
   - All vocal processing enabled
   - Subtle mastering chain
   - Target: Emotional depth

2. **Cinematic Mastering Preset** - Professional mastering
   - Vocal processing disabled
   - Full mastering chain
   - Light saturation

3. **Viral Appeal Preset** - Optimized for mass appeal
   - More aggressive compression
   - Brighter presence boost
   - Louder target (-12 LUFS)

### 5. Preset System

**Class Hierarchy:**
```
FXPreset (Data)
    ├─ name, category, description
    ├─ FXMode (Off/DSP/AI/Hybrid)
    ├─ tags (array)
    └─ parameters (map)

FXPresetManager (Management)
    ├─ presets (vector)
    ├─ loadPresetsFromDirectory()
    ├─ searchPresets()
    ├─ filterByCategory()
    └─ filterByTag()
```

**JSON Schema:**
```json
{
  "name": "string",
  "mode": 0-3,
  "category": "string",
  "description": "string",
  "author": "string",
  "tags": ["string", ...],
  "params": {
    "paramName": float,
    ...
  }
}
```

**Search Algorithm:**
- Full-text search across name, tags, description
- O(n) complexity - acceptable for <1000 presets
- Future: Index with trie or inverted index

### 6. Undo/Redo System

**State Management:**
```
History: [A0] → [A1] → [A2] → [A3]
                         ↑
                    currentIndex

Undo: Move currentIndex left, apply inverse action
Redo: Move currentIndex right, apply action
```

**Action Types:**
- FXChange: FX parameter or mode change
- TimelineChange: Block add/remove/modify
- ModelChange: Model load/unload
- ArrangementChange: BPM or quantization change
- PresetChange: Preset load/save

**Transaction Support:**
```cpp
undoManager.beginTransaction("Complex operation");
undoManager.addAction(action1);
undoManager.addAction(action2);
undoManager.addAction(action3);
undoManager.endTransaction();
// Results in single compound action in history
```

## Threading Model

### Audio Thread (Real-Time Critical)

**Allowed:**
- Process audio buffers
- Run ONNX inference (optimized models)
- Apply DSP effects
- Read atomic variables
- Use lock-free queues

**Forbidden:**
- Memory allocation (new/delete/malloc)
- File I/O
- Network I/O
- Blocking locks
- Unbounded loops

**Communication with GUI:**
```
Audio Thread              GUI Thread
     │                        │
     │ ─── AudioParameter ──> │  (atomic read)
     │                        │
     │ <── CommandFifo ────── │  (lock-free queue)
     │                        │
```

### GUI Thread (Non-Real-Time)

**Responsibilities:**
- Handle user input
- Update UI components
- Load/save presets
- Parse stage scripts
- Communicate with audio thread via thread-safe mechanisms

### Background Thread (Optional)

**Future Use Cases:**
- Preset scanning
- Model preloading
- Waveform rendering
- File exports

## Memory Management

### Buffer Allocation Strategy

**Pre-allocation:**
```cpp
// In prepareToPlay()
trackBuffers.resize(numTracks);
for (auto& buffer : trackBuffers)
    buffer.setSize(numChannels, maxBlockSize);
```

**Pool Allocation:**
- Reuse buffers across process calls
- No allocation in processBlock()

### ONNX Model Memory

**Loading:**
- Models loaded once at initialization
- Kept in memory for fast access
- ~100MB total for all models

**Inference:**
- Input/output tensors created per call
- Use pre-allocated buffers when possible
- Future: Tensor pooling

## Performance Characteristics

### Latency Budget (44.1kHz, 512 samples = 11.6ms)

| Component          | Target | Maximum |
|-------------------|--------|---------|
| Pattern lookup    | 0.1ms  | 0.5ms   |
| ONNX inference    | 1.0ms  | 3.0ms   |
| DSP effects       | 1.0ms  | 2.0ms   |
| Buffer management | 0.1ms  | 0.5ms   |
| **Total**         | **2.2ms** | **6.0ms** |

### Memory Usage

| Component       | Memory  |
|----------------|---------|
| ONNX models    | ~100MB  |
| Audio buffers  | ~1MB    |
| UI resources   | ~10MB   |
| Code/libs      | ~50MB   |
| **Total**      | **~161MB** |

### CPU Usage Targets

- Idle: <1%
- Playing: <5%
- With AI FX: <15%
- Maximum: <25%

## Scalability Considerations

### Model Scaling

**Current: 5-7 models**
- 808, hihat, snare, piano, synth
- vocal_tts, vocal_hifigan

**Future: 20+ models**
- Need model streaming
- LRU cache for models
- On-demand loading

### Track Scaling

**Current: 6 fixed tracks**
- Hardcoded array

**Future: Dynamic tracks**
- Vector of tracks
- Add/remove at runtime
- Save track configuration

### Preset Scaling

**Current: <100 presets**
- Linear search acceptable

**Future: 1000+ presets**
- Need indexing
- Database backend
- Cloud sync

## Security Considerations

### ONNX Model Safety

**Threats:**
- Malicious model files
- Buffer overflows in models
- Excessive computation

**Mitigations:**
- Validate model signatures
- Timeout on inference
- Sandbox ONNX Runtime
- User confirmation for external models

### State Persistence

**Threats:**
- Corrupted state data
- Injection attacks via presets

**Mitigations:**
- JSON schema validation
- Sanitize file paths
- Limit file sizes
- Checksum verification

## Future Architecture Improvements

### 1. Plugin State Management
- Implement proper versioning
- Migration paths between versions
- Backward compatibility

### 2. Model Management
- Model marketplace integration
- Automatic updates
- A/B testing of models

### 3. Distributed Processing
- Offload AI to separate process
- GPU compute on another device
- Cloud-based inference option

### 4. Advanced Routing
- Sidechain support
- Send/return busses
- Multi-output stems

### 5. Scripting Engine
- Lua/Python scripting for custom logic
- User-defined generators
- Procedural patterns

## References

- JUCE Framework: https://juce.com/
- ONNX Runtime: https://onnxruntime.ai/
- VST3 SDK: https://steinbergmedia.github.io/vst3_doc/
- Real-Time Audio Programming: Ross Bencina's papers
- Lock-Free Programming: Herb Sutter's articles
