<div align="center">

# 🎚️ MAEVN

### **Dynamic AI-Powered Vocal + Instrument Generator (VST3)**

[![JUCE](https://img.shields.io/badge/JUCE-7.0%2B-orange?style=flat-square&logo=juce)](https://juce.com/)
[![ONNX Runtime](https://img.shields.io/badge/ONNX%20Runtime-1.16%2B-blue?style=flat-square&logo=onnx)](https://onnxruntime.ai/)
[![CMake](https://img.shields.io/badge/CMake-3.20%2B-red?style=flat-square&logo=cmake)](https://cmake.org/)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=cplusplus)](https://isocpp.org/)
[![License](https://img.shields.io/badge/License-MIT-green?style=flat-square)](LICENSE)

---

**MAEVN** is an all-in-one AI-powered VST3 plugin built with **JUCE + ONNX Runtime**, designed for producers, sound designers, and experimental artists.

*🔥 AI Vocals · 🥁 Trap Instruments · 🎛️ Hybrid FX · 🎼 Timeline Arrangement · ↩️ Full Undo/Redo*

</div>

---

## 📖 Table of Contents

- [Introduction](#-introduction)
- [Features](#-features)
  - [Vocals](#-vocals)
  - [Instruments](#-instruments)
  - [FX Chains](#-fx-chains)
  - [Timeline & Arrangement](#-timeline--arrangement)
  - [Undo/Redo System](#️-undoredo-system)
- [Repo Structure](#-repo-structure)
- [Installation & Build](#️-installation--build)
  - [Requirements](#requirements)
  - [Build Steps](#build-steps)
- [Workflow in DAW](#-workflow-in-daw)
- [Roadmap](#-roadmap)
- [Contributor's Guide](#-contributors-guide)
  - [Coding Standards](#coding-standards)
  - [Module Ownership](#module-ownership)
  - [Workflow](#workflow)
  - [Testing](#testing)
  - [Contribution Principles](#contribution-principles)
  - [Getting Started](#getting-started-for-contributors)

---

## 🎯 Introduction

**MAEVN** (pronounced *"Maven"*) is a revolutionary **AI-powered VST3 plugin** that brings cutting-edge machine learning directly into your Digital Audio Workstation (DAW). Built on the robust **JUCE framework** with **ONNX Runtime** integration, MAEVN provides:

| Capability | Description |
|------------|-------------|
| 🎤 **AI Vocal Synthesis** | Text-to-Speech engine with emotion parsing and vocoder integration |
| 🥁 **Trap-Inspired Instruments** | AI-generated 808s, hi-hats, snares, piano, and synth pads |
| 🎛️ **Hybrid FX Processing** | Combine traditional DSP with AI-powered effects |
| 🎼 **Stage-Script Arrangement** | Parse lyrical scripts into timeline blocks with BPM sync |
| ↩️ **Complete Undo/Redo** | Full action history with jump-to-any-state capability |

MAEVN is designed for **real-time performance** with latency budgets under 6ms at 44.1kHz/512 samples, making it suitable for live production workflows.

---

## ⚡ Features

### 🎤 Vocals

MAEVN's vocal engine is powered by AI Text-to-Speech with full vocoder integration:

| Feature | Description |
|---------|-------------|
| **AI TTS Engine** | Bring your own ONNX models or use default TTS models for vocal synthesis |
| **Vocoder Integration** | Converts mel-spectrograms to high-fidelity waveforms using HiFi-GAN architecture |
| **Emotion Parsing** | Automatically detects and applies emotion/cadence from stage-script inputs |
| **Block Types** | Supports `[HOOK]`, `[VERSE]`, `[INTRO]`, `[BRIDGE]`, `[OUTRO]` vocal blocks |

**How it works:**
1. Input your lyrics with stage-script tags (e.g., `[HOOK] Catchy hook lyrics`)
2. MAEVN parses the text and applies TTS inference
3. The vocoder converts spectrograms to audio waveforms
4. Emotion and cadence are automatically applied based on context

```
[HOOK] This is the catchy hook duration:4.0
[VERSE] Deep verse with emotion duration:8.0
```

### 🥁 Instruments

MAEVN includes five AI-powered instrument generators optimized for trap and modern production:

| Instrument | Technology | Description |
|------------|------------|-------------|
| **808s** | DDSP-inspired AI | Deep sub-bass generator with glide support |
| **Hi-Hats** | Noise + Envelope Synthesis | Trap-style rolls with velocity modulation |
| **Snare/Clap** | Hybrid Noise/Body | Layered transient + body synthesis |
| **Piano** | Additive/DDSP | Lightweight piano model for melodic content |
| **Synth/Pad** | FM Synthesis | Ambient pad generator with rich harmonics |

**Model Hot-Reload:** Drop new `.onnx` models into `/Models/` and MAEVN automatically detects and loads them at runtime.

### 🎛️ FX Chains

MAEVN provides a powerful hybrid FX system combining traditional DSP with AI-powered processing:

| Mode | Processing Chain | Use Case |
|------|-----------------|----------|
| **Off** | No processing (bypass) | Dry signal passthrough |
| **DSP** | Compressor → EQ → Reverb → Limiter | Classic mixing approach |
| **AI** | AI Autotune → AI Mastering | ML-powered enhancement |
| **Hybrid** | DSP Chain → AI Chain | Best of both worlds |

**Preset System:**
- 📁 **Save/Load Presets** — JSON-based preset storage
- 🏷️ **Categories & Tags** — Organize presets by type (Vocal, 808, Synth, etc.)
- 🔍 **Search & Filter** — Full-text search with tag cloud navigation
- ▶️ **One-Click Preview** — Audition FX chains before applying

**Example Preset (JSON):**
```json
{
  "name": "RadioVocals",
  "mode": 3,
  "category": "Vocal",
  "tags": ["Trap", "Radio", "Clean"],
  "params": {
    "compressorThreshold": -12.0,
    "eqHighGain": 3.0,
    "reverbMix": 0.4
  }
}
```

### 🎼 Timeline & Arrangement

MAEVN's **PatternEngine** parses stage-script input into structured timeline blocks:

| Feature | Description |
|---------|-------------|
| **Stage-Script Parsing** | Tags like `[HOOK]`, `[VERSE]`, `[808]` auto-create blocks |
| **BPM Synchronization** | Quantizes blocks to DAW tempo grid |
| **Multi-Track Lanes** | Separate lanes for Vocals, 808, HiHat, Piano, Synth |
| **Duration Control** | Specify block length with `duration:X.X` syntax |

**Supported Block Types:**

| Block Type | Track Lane | Description |
|------------|------------|-------------|
| `[INTRO]`, `[HOOK]`, `[VERSE]`, `[BRIDGE]`, `[OUTRO]` | Vocals | Vocal synthesis blocks |
| `[808]` | Bass | Sub-bass patterns |
| `[HIHAT]` | Drums | Hi-hat patterns |
| `[SNARE]` | Drums | Snare/clap hits |
| `[PIANO]` | Keys | Piano melodies |
| `[SYNTH]` | Synth | Pad/synth layers |

**Example Stage Script:**
```
[INTRO] Yo, check it
[HOOK] This is the hook duration:4.0
[VERSE] First verse with lyrics duration:8.0
[808] Deep bass glide
[HIHAT] Fast trap rolls
```

### ↩️ Undo/Redo System

MAEVN includes a comprehensive **GlobalUndoManager** for complete action history:

| Feature | Description |
|---------|-------------|
| **Per-Track History** | FX changes tracked per individual track |
| **Global Stack** | All actions (FX, timeline, models) in single history |
| **Jump-to-State** | Click any past action in history panel to revert instantly |
| **Transaction Support** | Group multiple actions into single undo-able operation |

**Tracked Action Types:**
- FX parameter changes
- Preset load/save operations
- Timeline block modifications
- Model loading/unloading
- Arrangement changes

---

## 📂 Repo Structure

```
MAEVN/
│
├── 📄 CMakeLists.txt              # JUCE + ONNX Runtime build configuration
├── 📄 README.md                   # This documentation
├── 📄 ARCHITECTURE.md             # Detailed system architecture
├── 📄 DEVELOPER_GUIDE.md          # Developer documentation
├── 📄 QUICKSTART.md               # Quick start guide for users
│
├── 📁 Source/                     # Core plugin source code
│   ├── PluginProcessor.cpp/h      # Audio processor (processBlock, DAW I/O)
│   ├── PluginEditor.cpp/h         # Main UI editor (timeline, panels)
│   ├── OnnxEngine.cpp/h           # ONNX Runtime C++ wrapper
│   ├── PatternEngine.cpp/h        # Stage-script parser + arrangement
│   ├── AIFXEngine.cpp/h           # Hybrid DSP + AI FX processing
│   ├── TimelineLane.cpp/h         # Per-track lane UI component
│   ├── FXPreset.cpp/h             # FX preset data structure
│   ├── FXPresetManager.cpp/h      # Preset storage and retrieval
│   ├── PresetBrowserComponent.cpp/h  # Preset browser UI
│   ├── GlobalUndoManager.cpp/h    # Undo/redo stack management
│   ├── UndoHistoryComponent.cpp/h # History panel UI
│   ├── CinematicAudioEnhancer.cpp/h  # Grammy-quality audio processing
│   ├── DSPModules.h               # Built-in DSP effect modules
│   └── Utilities.h                # Shared helper functions
│
├── 📁 Models/                     # ONNX models (auto-scanned at runtime)
│   ├── config.json                # Model role → file path mapping
│   ├── 📁 drums/
│   │   ├── 808_ddsp.onnx          # 808 sub-bass generator
│   │   ├── hihat_ddsp.onnx        # Hi-hat synthesis
│   │   └── snare_ddsp.onnx        # Snare/clap generator
│   ├── 📁 instruments/
│   │   ├── piano_ddsp.onnx        # Piano synthesis
│   │   └── synth_fm.onnx          # FM synth/pad
│   └── 📁 vocals/
│       ├── vocals_tts.onnx        # Text-to-Speech model (user-supplied)
│       └── vocals_hifigan.onnx    # HiFi-GAN vocoder (user-supplied)
│
├── 📁 Presets/                    # FX chain presets (.json)
│   ├── RadioVocals.json           # Radio-ready vocal preset
│   ├── Dirty808.json              # Heavy distorted bass
│   ├── WideHats.json              # Stereo widened hi-hats
│   └── CinematicVocals.json       # Grammy-quality vocal processing
│
├── 📁 Resources/                  # UI assets, icons, skins
├── 📁 scripts/                    # Build and export scripts
│
├── 🔧 setup_maevn_repo.bat        # Repository setup script (Windows)
└── 🔧 build_maevn_onnx.bat        # ONNX model generation script (Windows)
```

---

## 🛠️ Installation & Build

### Requirements

| Dependency | Version | Purpose |
|------------|---------|---------|
| **JUCE** | 7.0+ | Audio plugin framework |
| **ONNX Runtime** | 1.16+ | AI/ML inference engine |
| **CMake** | 3.20+ | Build system |
| **Python** | 3.10+ | ONNX model export scripts |
| **C++ Compiler** | C++17 | MSVC, GCC, or Clang |

### Build Steps

#### 1️⃣ Clone the Repository

```bash
git clone https://github.com/MavenSource/MAEVNSVOCALPIPELINE.git
cd MAEVNSVOCALPIPELINE
```

#### 2️⃣ Setup Repository Structure

**Windows:**
```batch
setup_maevn_repo.bat
```

**Linux/macOS:**
```bash
chmod +x setup_maevn_repo.sh
./setup_maevn_repo.sh
```

This creates the `/Models`, `/Presets`, and `/Resources` directories and generates `Models/config.json`.

#### 3️⃣ Generate Default ONNX Models

**Windows:**
```batch
build_maevn_onnx.bat
```

**Linux/macOS:**
```bash
python3 scripts/export_onnx_models.py
```

This exports optimized `.onnx` models for 808, hihat, snare, piano, and synth into `/Models/`.

#### 4️⃣ Add Your Vocal Models

Export your own TTS and vocoder models and place them in `/Models/vocals/`:
- `vocals_tts.onnx` — Text-to-Speech model
- `vocals_hifigan.onnx` — HiFi-GAN vocoder

#### 5️⃣ Configure and Build with CMake

```bash
# Configure (adjust paths to your JUCE and ONNX Runtime installations)
cmake -B Build -S . \
    -DJUCE_PATH="/path/to/JUCE" \
    -DONNXRUNTIME_PATH="/path/to/onnxruntime"

# Build Release
cmake --build Build --config Release
```

#### 6️⃣ Install the Plugin

```bash
# Install to system VST3 folder
cmake --install Build --config Release
```

**Default VST3 Locations:**
| Platform | Path |
|----------|------|
| Windows | `C:\Program Files\Common Files\VST3\` |
| macOS | `/Library/Audio/Plug-Ins/VST3/` |
| Linux | `~/.vst3/` |

---

## 🎵 Workflow in DAW

Here's the typical workflow for using MAEVN in your production:

```
┌─────────────────────────────────────────────────────────────────┐
│  1. INSERT    │  Load MAEVN VST3 on a track in your DAW        │
├─────────────────────────────────────────────────────────────────┤
│  2. SCRIPT    │  Write stage-script with [HOOK], [VERSE], etc. │
├─────────────────────────────────────────────────────────────────┤
│  3. PARSE     │  Click "Parse Script" → blocks appear on lanes │
├─────────────────────────────────────────────────────────────────┤
│  4. FX        │  Select FX mode (DSP/AI/Hybrid) per track      │
├─────────────────────────────────────────────────────────────────┤
│  5. PRESETS   │  Browse and apply FX presets from library      │
├─────────────────────────────────────────────────────────────────┤
│  6. EXPERIMENT│  Use Undo History to try different settings    │
├─────────────────────────────────────────────────────────────────┤
│  7. EXPORT    │  Render stems (dry or FX processed) to DAW     │
└─────────────────────────────────────────────────────────────────┘
```

**Step-by-Step:**

1. **Insert MAEVN** — Add the plugin to a track in your DAW (Reaper, Ableton, FL Studio, etc.)

2. **Set BPM** — Adjust the BPM slider to match your project tempo

3. **Write Stage Script** — Enter your lyrics and instrument cues:
   ```
   [INTRO] Yo, check it out
   [HOOK] This is the hook, catchy and clean duration:4.0
   [VERSE] First verse dropping heat duration:8.0
   [808] Deep bass glide
   [HIHAT] Fast trap rolls
   ```

4. **Parse Script** — Click "Parse Script" to generate timeline blocks

5. **Apply FX** — Select FX mode for each track and load presets

6. **Experiment** — Use the Undo History panel to safely try different settings

7. **Export** — Render your final audio with or without FX processing

---

## 🚧 Roadmap

| Feature | Status | Description |
|---------|--------|-------------|
| **DAW Automation Hooks** | 🔜 Planned | Expose FX parameters as DAW-automatable (Ghost Choir, Tone Shaper) |
| **Drag-to-MIDI/Audio** | 🔜 Planned | Drag `[HOOK]` blocks directly into DAW timeline |
| **Loop Region Sync** | 🔜 Planned | Auto-fit arrangement to DAW selection |
| **Instrument Sequencer** | 🔜 Planned | Pattern editor for hi-hat rolls, 808 glides |
| **Community Preset Packs** | 🔜 Planned | Import/export preset bundles |
| **GPU Acceleration** | 🔬 Research | CUDA/DirectML support for faster inference |
| **Model Marketplace** | 🔬 Research | Download community-shared ONNX models |

---

## 🤝 Contributor's Guide

Welcome to MAEVN development! This section covers everything you need to contribute to the project.

### Coding Standards

#### Languages & Frameworks

| Technology | Usage |
|------------|-------|
| **C++17** | Core plugin code (JUCE + ONNX Runtime) |
| **Python 3.10+** | ONNX model export scripts |
| **JSON** | Presets, configuration files |

#### Style Guidelines

- **Brace Style:** New line for opening braces
- **Indentation:** 4 spaces (no tabs)
- **Naming:** `camelCase` for methods, `PascalCase` for classes
- **Headers:** `.h` extension, implementations in `.cpp`
- **Smart Pointers:** Prefer `std::unique_ptr` over raw pointers
- **Documentation:** Doxygen-style `///` comments for public APIs

**Example:**
```cpp
/// Processes audio through the FX chain.
/// @param buffer The audio buffer to process
/// @param numSamples Number of samples to process
void AIFXEngine::process(juce::AudioBuffer<float>& buffer, int numSamples)
{
    // Implementation
}
```

### Module Ownership

| Module | Responsibility | Notes |
|--------|---------------|-------|
| `PluginProcessor.*` | Core DSP pipeline, DAW I/O | Must maintain JUCE API compatibility |
| `PluginEditor.*` | GUI, user interaction | Component-based architecture |
| `OnnxEngine.*` | AI inference wrapper | Thread-safe, hot-reload support |
| `PatternEngine.*` | Timeline parsing | BPM sync, block management |
| `AIFXEngine.*` | FX processing | DSP + AI hybrid chains |
| `FXPreset*` | Preset data structures | JSON serialization |
| `GlobalUndoManager.*` | Action history | Transaction support |
| `Utilities.h` | Shared helpers | Constants, macros |

### Workflow

#### Branching Strategy

| Branch | Purpose |
|--------|---------|
| `main` | Stable, release-ready builds |
| `dev` | Active development |
| `feature/*` | New features (e.g., `feature/onnx-hot-reload`) |
| `fix/*` | Bug fixes (e.g., `fix/buffer-underrun`) |
| `docs/*` | Documentation updates |

#### Pull Request Process

1. **Fork** the repository
2. **Branch** from `dev`: `git checkout -b feature/your-feature`
3. **Implement** your changes with tests
4. **Update** documentation as needed
5. **Submit** PR with descriptive title and body
6. **Review** — At least 1 contributor approval required

#### Commit Messages

Follow [Conventional Commits](https://www.conventionalcommits.org/):

```
feat: add AI hybrid FX chain
fix: resolve buffer underrun in processBlock
docs: update README with contributor guide
refactor: simplify OnnxEngine inference call
perf: optimize DSP effect processing
test: add unit tests for preset loading
```

### Testing

#### Unit Testing

- Framework: GoogleTest (planned)
- Coverage targets:
  - Preset load/save operations
  - Undo/Redo stack behavior
  - PatternEngine parsing
  - ONNX inference benchmarks

#### Manual Testing Checklist

| DAW | Platform | Status |
|-----|----------|--------|
| Reaper | Windows | ✅ Primary |
| Ableton Live | macOS | ✅ Verified |
| FL Studio | Cross-platform | ✅ Verified |
| Logic Pro | macOS | 🔜 Planned |

**Test Procedure:**
1. Load MAEVN in DAW
2. Parse sample stage-script
3. Verify block generation on timeline
4. Test all FX modes (Off/DSP/AI/Hybrid)
5. Load/save presets
6. Test undo/redo functionality
7. Monitor CPU usage (<5% at 44.1kHz, 512 samples)

### Contribution Principles

| Principle | Description |
|-----------|-------------|
| ⚡ **Performance First** | All DSP paths must be real-time safe (<1ms per buffer) |
| 🧩 **Modularity** | New instruments/models must be pluggable via `/Models/config.json` |
| 🧹 **No Dead Code** | Experimental code must use `#ifdef EXPERIMENTAL` |
| 📖 **Transparency** | All ONNX exports include scripts in `/scripts/` |
| 🔄 **Flexibility** | Support both DSP-only fallback and AI-enhanced modes |

### Getting Started for Contributors

1. **Clone and Setup:**
   ```bash
   git clone https://github.com/MavenSource/MAEVNSVOCALPIPELINE.git
   cd MAEVNSVOCALPIPELINE
   setup_maevn_repo.bat  # or ./setup_maevn_repo.sh
   ```

2. **Generate Models:**
   ```bash
   build_maevn_onnx.bat  # or python3 scripts/export_onnx_models.py
   ```

3. **Add Vocal Models:**
   Place your `vocals_tts.onnx` and `vocals_hifigan.onnx` in `/Models/vocals/`

4. **Build:**
   ```bash
   cmake -B Build -S . -DJUCE_PATH="/path/to/JUCE" -DONNXRUNTIME_PATH="/path/to/onnxruntime"
   cmake --build Build --config Release
   ```

5. **Test in DAW:**
   Load `MAEVN.vst3` and verify functionality

6. **Contribute:**
   Submit issues and PRs on GitHub!

---

<div align="center">

## ⚔️ MAEVN = Dynamic · Flexible · Trap-Ready

**All-in-one VST for vocals + instruments + FX + arrangement**

*AI-powered · DAW-synced · Real-time performance*

---

📖 [Quick Start](QUICKSTART.md) · 🏗️ [Architecture](ARCHITECTURE.md) · 👨‍💻 [Developer Guide](DEVELOPER_GUIDE.md)

---

Made with ❤️ by the MAEVN Team

</div>
