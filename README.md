
🎚 MAEVN — Dynamic Vocal + Instrument Generator (VST3) MAEVN is an all-in-one AI-powered VST3 plugin built with JUCE + ONNX Runtime, designed for producers, sound designers, and experimental artists. It combines AI-driven vocal synthesis, trap-inspired instruments, hybrid DSP/AI FX chains, timeline arrangement, and full DAW integration into a single plugin. 🚀 Features 🎤 Vocals AI Text-to-Speech (TTS) engine (user-supplied or default ONNX models). Vocoder integration for mel-spectrogram → waveform. Emotion & cadence parsing from Lyrical Stage Script Input. Hooks, verses, spoken word parsed into timeline blocks. 🥁 Instruments 808s — AI sub-bass generator (DDSP-inspired). Hi-Hats — Noise + envelope synthesis (trap rolls). Snare/Clap — Hybrid noise/body generator. Piano — Lightweight additive/DDSP piano model. Synth/Pad — FM-based ambient pad generator. Hot reload of ONNX models from /Models/ directory. 🎛 FX Chains Hybrid DSP + AI FX per track. Modes: Off, DSP, AI, Hybrid. Save/load FX presets (.json). Preset Browser with categories + tags + search + tag cloud. One-click preview of FX chains. 🎼 Timeline & Arrangement Stage-script input ([HOOK], [VERSE], [808], etc.) auto-parsed into blocks. BPM-aware quantization + DAW sync. Supports [PIANO], [808], [HIHAT] blocks as instrument lanes. Drag-to-audio/MIDI planned. ↩️ Undo / Redo System Per-track FX history. Global Undo/Redo Stack (FX, arrangement, models, timeline). Undo History Panel — click any past action to revert instantly. 📂 Repo Structure MAEVN/ │ ├── CMakeLists.txt # JUCE + ONNX build config ├── README.md # This file │ ├── Source/ # Core plugin source │ ├── PluginProcessor.* # Audio processor (processBlock, routing) │ ├── PluginEditor.* # UI (timeline, preset panels) │ ├── OnnxEngine.* # ONNX runtime wrapper │ ├── PatternEngine.* # Arrangement/timeline parser │ ├── AIFXEngine.* # DSP + AI FX chains │ ├── TimelineLane.* # Per-track lane UI │ ├── FXPreset.* # FX preset struct (save/load JSON) │ ├── FXPresetManager.* # Preset storage system │ ├── PresetBrowserComponent.* # Preset browser UI (search, tags, preview) │ ├── GlobalUndoManager.* # Undo/redo stack │ ├── UndoHistoryComponent.* # History panel UI │ └── Utilities.h │ ├── Models/ # ONNX models (auto-scanned at runtime) │ ├── drums/ │ │ ├── 808_ddsp.onnx │ │ ├── hihat_ddsp.onnx │ │ └── snare_ddsp.onnx │ ├── instruments/ │ │ ├── piano_ddsp.onnx │ │ └── synth_fm.onnx │ └── vocals/ │ ├── vocals_tts.onnx # You supply │ └── vocals_hifigan.onnx # You supply │ └── config.json │ ├── Presets/ # FX chain presets (.json) │ ├── RadioVocals.json │ ├── Dirty808.json │ └── WideHats.json │ ├── Resources/ # UI skins, icons, defaults │ ├── setup_maevn_repo.bat # Prepares repo tree + config.json └── build_maevn_onnx.bat # Generates default ONNX models (except vocals) ⚡️ Installation & Build Requirements JUCE 7+ ONNX Runtime SDK (C++ runtime) CMake 3.20+ Python 3.10+ (for ONNX export scripts) Steps Clone repo. Run: setup_maevn_repo.bat (Sets up folders, config, presets, Models tree). Run: build_maevn_onnx.bat (Exports default instrument ONNX models → placed in Models/). Add your own vocals_tts.onnx + vocals_hifigan.onnx into Models/vocals/. Build with CMake → generate .vst3. Drop into your DAW’s VST3 folder. 🎵 Workflow in DAW Insert MAEVN into your DAW. Paste a stage-script input (e.g., [INTRO] Spoken word, [HOOK] Catchy chant, [808] Glide). MAEVN parses → generates vocals + instrument blocks. Select FX mode (DSP/AI/Hybrid) per track. Browse FX presets (Trap Vocals, Dirty 808). Use Undo History to experiment safely. Export stems (dry or FX processed) into DAW. 🚧 Roadmap DAW Automation Hooks (Ghost Choir, Tone Shaper, etc.). Drag-to-MIDI/Audio (drop [HOOK] into DAW timeline). Region Sync (fit arrangement to DAW selection). Instrument Pattern Sequencer (hi-hat rolls, 808 glides). Community Preset Packs (import/export). ⚔️ MAEVN = Dynamic · Flexible · Trap-ready All-in-one VST for vocals + instruments + FX + arrangement, AI-powered, DAW-synced. 👉 Do you want me to make this README developer-oriented (more setup/build details, API-level explanations), or producer-oriented (less code, more “how to make music”)? You said: ADVANCED developer-oriented Voice Clone Infinity Builder said: Perfect ⚡ — here’s a precision-tuned, advanced developer-oriented README for the MAEVN VST3 repo. This version emphasizes system architecture, build pipelines, ONNX runtime integration, and dev workflows for contributors and engineers. 🎚 MAEVN — AI-Powered Vocal + Instrument Generator (VST3) MAEVN is a JUCE-based VST3 plugin with ONNX Runtime integration that combines: 🎤 AI vocals (TTS + vocoder) 🥁 Trap-inspired instruments (808, hi-hats, snare, piano, synth) 🎛 Hybrid FX chains (DSP + ONNX AI FX) 🎼 Stage-script parser (arrangement blocks [HOOK], [VERSE], [808]) ↩️ Global Undo/Redo system It’s designed as an end-to-end AI DAW tool, providing runtime ONNX inference inside VST3 with real-time timeline arrangement, FX automation, and DAW sync. 🏗 System Architecture 🔑 Core Components JUCE Plugin Layer PluginProcessor → processBlock() wires DAW audio I/O to PatternEngine + OnnxEngine + AIFXEngine. PluginEditor → UI for timeline lanes, preset browser, undo history. ONNX Engine OnnxEngine wraps ONNX Runtime C++ API. Hot reload of .onnx models from /Models/ directory. Supports multiple instrument/vocal models concurrently. Pattern Engine Parses Lyrical Stage Script Input → block arrangement. Handles quantization, BPM sync, DAW playhead integration. Dispatches instrument triggers + vocal synthesis. AI FX Engine DSP FX: compressor, EQ, reverb, limiter. AI FX: ONNX models for autotune, AI mastering. Hybrid mode = DSP + AI sequential chain. Timeline Lanes Each track lane (VOCAL, 808, HiHat, etc.) has: FX Mode Selector (Off/DSP/AI/Hybrid). Preset Load/Save. Block visualization. Preset System FXPreset struct (category, tags, params). JSON serialization/deserialization. Preset Browser Component: Category filter (Vocal, 808, Synth, etc.) Search bar (name/tag filtering) Tag cloud (clickable chips) ▶ Preview button (applies FX to short audio slice) Undo/Redo GlobalUndoManager: action history stack (FX, models, timeline). UndoHistoryComponent: GUI list of all past actions. Jump-to-history: click any past state to instantly roll back. 📂 Repo Structure MAEVN/ │ ├── CMakeLists.txt # Build config (JUCE + ONNX Runtime) ├── README.md # This file │ ├── Source/ # Core source │ ├── PluginProcessor.* # processBlock(), DAW I/O │ ├── PluginEditor.* # GUI editor │ ├── OnnxEngine.* # ONNX runtime C++ wrapper │ ├── PatternEngine.* # Stage-script parser + arrangement │ ├── AIFXEngine.* # Hybrid DSP + AI FX │ ├── TimelineLane.* # Track lane GUI + FX toggles │ ├── FXPreset.* # FX preset schema │ ├── FXPresetManager.* # Preset I/O │ ├── PresetBrowserComponent.* # Preset browser UI │ ├── GlobalUndoManager.* # Undo/redo system │ ├── UndoHistoryComponent.* # History panel UI │ └── Utilities.h # Shared helpers │ ├── Models/ # ONNX models (auto-detected) │ ├── drums/ │ │ ├── 808_ddsp.onnx │ │ ├── hihat_ddsp.onnx │ │ └── snare_ddsp.onnx │ ├── instruments/ │ │ ├── piano_ddsp.onnx │ │ └── synth_fm.onnx │ └── vocals/ │ ├── vocals_tts.onnx │ └── vocals_hifigan.onnx │ └── config.json # Model role → path map │ ├── Presets/ # FX presets │ ├── RadioVocals.json │ ├── Dirty808.json │ └── WideHats.json │ ├── Resources/ # Icons, UI skins │ ├── setup_maevn_repo.bat # Prepares repo tree + config.json └── build_maevn_onnx.bat # Exports default instrument ONNX models ⚙️ Build Instructions Requirements JUCE 7+ ONNX Runtime C++ SDK CMake 3.20+ Python 3.10+ (for ONNX model export scripts) Steps Repo Setup setup_maevn_repo.bat Creates /Models, /Presets, /Resources. Writes Models/config.json. Generate Default ONNX Models build_maevn_onnx.bat Exports lightweight default .onnx models for: 808, hihat, snare, piano, synth Optimizes them → places in /Models/. Add Vocals Export your own TTS + vocoder: vocals_tts.onnx vocals_hifigan.onnx Place them in /Models/vocals/. Build Plugin cmake -B Build -S . -DJUCE_PATH="C:/JUCE" -DONNXRUNTIME_PATH="C:/onnxruntime" cmake --build Build --config Release Outputs MAEVN.vst3. Install Copy MAEVN.vst3 → your DAW’s VST3 folder. 🧪 Developer Notes Audio Pipeline (processBlock) Parse DAW playhead (BPM, transport). PatternEngine: parse active blocks → generate triggers. OnnxEngine: run inference on relevant models. AIFXEngine: apply FX (DSP/AI/Hybrid). Output → DAW buffer. Model Hot Reload /Models/config.json maps logical roles (VOCAL, 808) → file paths. MAEVN auto-reloads if models change on disk. Presets JSON format, e.g.: { "name": "RadioVocals", "mode": 3, "params": { "reverbMix": 0.4, "compressorThreshold": -12.0 }, "category": "Vocal", "tags": ["Trap", "Radio", "Clean"] } Category + tags drive search and tag cloud UI. Undo/Redo Each action → ActionState snapshot (JSON). Stored in GlobalUndoManager. UI list in UndoHistoryComponent. 🚧 Roadmap Automation Hooks: expose FX params as DAW automatable. Drag-to-MIDI/Audio: drop [HOOK] blocks into DAW timeline. Loop Region Sync: detect DAW selection and auto-fit arrangement. Instrument Sequencer: hi-hat rolls, 808 glides. Preset Exchange: import/export community preset packs. ⚔️ MAEVN is a developer-first experimental AI DAW tool, designed for flexibility, extensibility, and precision control. It’s not just a plugin — it’s a framework for AI-based music generation inside DAWs. 👉 Do you want me to also write a step-by-step contributor’s guide (coding standards, PR process, module ownership) to make the repo team-ready? You said: YES Voice Clone Infinity Builder said: Perfect ⚡ — let’s add a Contributor’s Guide section to the README so any developer joining the project knows how to work with the codebase, coding style, and workflow. This makes MAEVN repo team-ready for collaboration. 🤝 Contributor’s Guide Welcome to MAEVN Development. This guide outlines the coding standards, workflow, and contribution process for the VST3 project. 🔑 Coding Standards Language & Frameworks C++17 for plugin code (JUCE + ONNX Runtime). Python 3.10+ for ONNX model export scripts. Style Use JUCE code style (brace on new line, 4 spaces). Header files .h, implementation .cpp. Prefer auto for iterators and template-heavy code. Avoid raw pointers — use std::unique_ptr or juce::ScopedPointer. Document every class and major method with /// Doxygen comments. 🧩 Module Ownership Module Owner Role Notes PluginProcessor.* Core DSP pipeline Must not break JUCE API PluginEditor.* GUI + DAW integration OnnxEngine.* AI inference Wraps ONNX runtime PatternEngine.* Timeline arrangement Parses stage-script input AIFXEngine.* FX processing DSP + AI hybrid FXPreset* Preset storage/management JSON format GlobalUndoManager.* History + state mgmt Undo/redo UndoHistoryComponent.* GUI for undo stack Utilities.h Shared helpers Constants, macros 🔄 Workflow Branching main → stable, release-ready builds. dev → active development branch. Feature branches: feature/onnx-hot-reload feature/fx-browser-ui feature/undo-history Pull Requests Fork → branch off dev. Implement feature/fix. Write/update unit tests if applicable. Run lint + build checks. Submit PR → reviewed by at least 1 other contributor. Commits Follow Conventional Commits: feat: add AI hybrid FX chain fix: resolve buffer underrun in processBlock docs: update README with contributor guide refactor: simplify OnnxEngine inference call 🧪 Testing Unit tests (GoogleTest) for: Preset load/save. Undo/Redo stack. PatternEngine parsing. Manual testing in DAWs: Reaper (Windows). Ableton Live (macOS). FL Studio (cross-platform). 📦 Build Environment Dependencies JUCE 7+ ONNX Runtime (C++ API) CMake 3.20+ Building cmake -B Build -S . -DJUCE_PATH="C:/JUCE" -DONNXRUNTIME_PATH="C:/onnxruntime" cmake --build Build --config Release 🧠 Contribution Principles Performance First → All DSP paths must run realtime-safe (<1ms per buffer). Modularity → New instruments/models must be pluggable via /Models/config.json. No Dead Code → If experimental, mark #ifdef EXPERIMENTAL. Transparency → All ONNX exports should include scripts in scripts/. Flexibility → Allow both DSP-only fallback and AI-enhanced modes. 🚀 How to Get Started Clone repo. Run setup_maevn_repo.bat to bootstrap structure. Run build_maevn_onnx.bat to generate baseline ONNX models. Add vocal .onnx models into /Models/vocals/. Build with CMake → load MAEVN.vst3 in your DAW. Submit issues/PRs for fixes and new features. ⚔️ With this guide, MAEVN is team-ready for contributions from both audio DSP engineers and ML developers
🎚 MAEVN — Dynamic Vocal + Instrument Generator (VST3)

MAEVN is an all-in-one AI-powered VST3 plugin built with JUCE + ONNX Runtime, designed for producers, sound designers, and experimental artists. It combines AI-driven vocal synthesis, trap-inspired instruments, hybrid DSP/AI FX chains, timeline arrangement, and full DAW integration into a single plugin.

🚀 Features

🎤 Vocals

AI Text-to-Speech (TTS) engine (user-supplied or default ONNX models).
Vocoder integration for mel-spectrogram → waveform.
Emotion & cadence parsing from Lyrical Stage Script Input.
Hooks, verses, spoken word parsed into timeline blocks.
🥁 Instruments

808s — AI sub-bass generator (DDSP-inspired).
Hi-Hats — Noise + envelope synthesis (trap rolls).
Snare/Clap — Hybrid noise/body generator.
Piano — Lightweight additive/DDSP piano model.
Synth/Pad — FM-based ambient pad generator.
Hot reload of ONNX models from /Models/ directory.
🎛 FX Chains

Hybrid DSP + AI FX per track.
Modes: Off, DSP, AI, Hybrid.
Save/load FX presets (.json).
Preset Browser with categories + tags + search + tag cloud.
One-click preview of FX chains.
🎼 Timeline & Arrangement

Stage-script input ([HOOK], [VERSE], [808], etc.) auto-parsed into blocks.
BPM-aware quantization + DAW sync.
Supports [PIANO], [808], [HIHAT] blocks as instrument lanes.
Drag-to-audio/MIDI planned.
↩️ Undo / Redo System

Per-track FX history.
Global Undo/Redo Stack (FX, arrangement, models, timeline).
Undo History Panel — click any past action to revert instantly.
📂 Repo Structure

Copy
MAEVN/
 ├── CMakeLists.txt          # JUCE + ONNX build config
 ├── README.md               # This file
 ├── Source/                 # Core plugin source
 │   ├── PluginProcessor.*   # Audio processor (processBlock, routing)
 │   ├── PluginEditor.*      # UI (timeline, preset panels)
 │   ├── OnnxEngine.*        # ONNX runtime wrapper
 │   ├── PatternEngine.*     # Arrangement/timeline parser
 │   ├── AIFXEngine.*        # DSP + AI FX chains
 │   ├── TimelineLane.*      # Per-track lane UI
 │   ├── FXPreset.*          # FX preset struct (save/load JSON)
 │   ├── FXPresetManager.*   # Preset storage system
 │   ├── PresetBrowserComponent.*  # Preset browser UI (search, tags, preview)
 │   ├── GlobalUndoManager.* # Undo/redo stack
 │   ├── UndoHistoryComponent.* # History panel UI
 │   └── Utilities.h 
 ├── Models/                 # ONNX models (auto-scanned at runtime)
 │   ├── drums/
 │   │   ├── 808_ddsp.onnx
 │   │   ├── hihat_ddsp.onnx
 │   │   └── snare_ddsp.onnx
 │   ├── instruments/
 │   │   ├── piano_ddsp.onnx
 │   │   └── synth_fm.onnx
 │   └── vocals/
 │       ├── vocals_tts.onnx       # You supply
 │       └── vocals_hifigan.onnx   # You supply
 └── config.json 
 ├── Presets/                # FX chain presets (.json)
 │   ├── RadioVocals.json 
 │   ├── Dirty808.json 
 │   └── WideHats.json 
 ├── Resources/              # UI skins, icons, defaults
 ├── setup_maevn_repo.bat    # Prepares repo tree + config.json
 └── build_maevn_onnx.bat    # Generates default ONNX models (except vocals)
⚡️ Installation & Build Requirements

JUCE 7+
ONNX Runtime SDK (C++ runtime)
CMake 3.20+
Python 3.10+ (for ONNX export scripts)
Steps

Clone repo.
Run: setup_maevn_repo.bat (Sets up folders, config, presets, Models tree).
Run: build_maevn_onnx.bat (Exports default instrument ONNX models → placed in Models/).
Add your own vocals_tts.onnx + vocals_hifigan.onnx into Models/vocals/.
Build with CMake → generate .vst3.
Drop into your DAW’s VST3 folder.
🎵 Workflow in DAW

Insert MAEVN into your DAW.
Paste a stage-script input (e.g., [INTRO] Spoken word, [HOOK] Catchy chant, [808] Glide).
MAEVN parses → generates vocals + instrument blocks.
Select FX mode (DSP/AI/Hybrid) per track.
Browse FX presets (Trap Vocals, Dirty 808).
Use Undo History to experiment safely.
Export stems (dry or FX processed) into DAW.
🚧 Roadmap

DAW Automation Hooks (Ghost Choir, Tone Shaper, etc.).
Drag-to-MIDI/Audio (drop [HOOK] into DAW timeline).
Region Sync (fit arrangement to DAW selection).
Instrument Pattern Sequencer (hi-hat rolls, 808 glides).
Community Preset Packs (import/export).
⚔️ MAEVN = Dynamic · Flexible · Trap-ready
All-in-one VST for vocals + instruments + FX + arrangement, AI-powered, DAW-synced.

👉 Do you want me to make this README developer-oriented (more setup/build details, API-level explanations), or producer-oriented (less code, more “how to make music”)? You said: ADVANCED developer-oriented

Voice Clone Infinity Builder said: Perfect ⚡ — here’s a precision-tuned, advanced developer-oriented README for the MAEVN VST3 repo. This version emphasizes system architecture, build pipelines, ONNX runtime integration, and dev workflows for contributors and engineers.

🎚 MAEVN — AI-Powered Vocal + Instrument Generator (VST3)

MAEVN is a JUCE-based VST3 plugin with ONNX Runtime integration that combines:

🎤 AI vocals (TTS + vocoder)
🥁 Trap-inspired instruments (808, hi-hats, snare, piano, synth)
🎛 Hybrid FX chains (DSP + ONNX AI FX)
🎼 Stage-script parser (arrangement blocks [HOOK], [VERSE], [808])
↩️ Global Undo/Redo system
It’s designed as an end-to-end AI DAW tool, providing runtime ONNX inference inside VST3 with real-time timeline arrangement, FX automation, and DAW sync.

🏗 System Architecture

🔑 Core Components

JUCE Plugin Layer

PluginProcessor → processBlock() wires DAW audio I/O to PatternEngine + OnnxEngine + AIFXEngine.

PluginEditor → UI for timeline lanes, preset browser, undo history.

ONNX Engine

OnnxEngine wraps ONNX Runtime C++ API.

Hot reload of .onnx models from /Models/ directory.

Supports multiple instrument/vocal models concurrently.

Pattern Engine

Parses Lyrical Stage Script Input → block arrangement.

Handles quantization, BPM sync, DAW playhead integration.

Dispatches instrument triggers + vocal synthesis.

AI FX Engine

DSP FX: compressor, EQ, reverb, limiter.

AI FX: ONNX models for autotune, AI mastering.

Hybrid mode = DSP + AI sequential chain.

Timeline Lanes

Each track lane (VOCAL, 808, HiHat, etc.) has:

FX Mode Selector (Off/DSP/AI/Hybrid).
Preset Load/Save.
Block visualization.
Preset System

FXPreset struct (category, tags, params).

JSON serialization/deserialization.

Preset Browser Component:

Category filter (Vocal, 808, Synth, etc.)
Search bar (name/tag filtering)
Tag cloud (clickable chips)
▶ Preview button (applies FX to short audio slice)
Undo/Redo

GlobalUndoManager: action history stack (FX, models, timeline).

UndoHistoryComponent: GUI list of all past actions.

Jump-to-history: click any past state to instantly roll back.

📂 Repo Structure

Copy
MAEVN/
 ├── CMakeLists.txt          # Build config (JUCE + ONNX Runtime)
 ├── README.md               # This file
 ├── Source/                 # Core source
 │   ├── PluginProcessor.*   # processBlock(), DAW I/O
 │   ├── PluginEditor.*      # GUI editor
 │   ├── OnnxEngine.*        # ONNX runtime C++ wrapper
 │   ├── PatternEngine.*     # Stage-script parser + arrangement
 │   ├── AIFXEngine.*        # Hybrid DSP + AI FX
 │   ├── TimelineLane.*      # Track lane GUI + FX toggles
 │   ├── FXPreset.*          # FX preset schema
 │   ├── FXPresetManager.*   # Preset I/O
 │   ├── PresetBrowserComponent.* # Preset browser UI
 │   ├── GlobalUndoManager.* # Undo/redo system
 │   ├── UndoHistoryComponent.* # History panel UI
 │   └── Utilities.h         # Shared helpers
 ├── Models/                 # ONNX models (auto-detected)
 │   ├── drums/
 │   │   ├── 808_ddsp.onnx
 │   │   ├── hihat_ddsp.onnx
 │   │   └── snare_ddsp.onnx
 │   ├── instruments/
 │   │   ├── piano_ddsp.onnx
 │   │   └── synth_fm.onnx
 │   └── vocals/
 │       ├── vocals_tts.onnx
 │       └── vocals_hifigan.onnx
 │   └── config.json         # Model role → path map
 ├── Presets/                # FX presets
 │   ├── RadioVocals.json
 │   ├── Dirty808.json
 │   └── WideHats.json
 ├── Resources/              # Icons, UI skins
 ├── setup_maevn_repo.bat    # Prepares repo tree + config.json
 └── build_maevn_onnx.bat    # Exports default instrument ONNX models
⚙️ Build Instructions
Requirements

JUCE 7+
ONNX Runtime C++ SDK
CMake 3.20+
Python 3.10+ (for ONNX model export scripts)
Steps

Repo Setup

setup_maevn_repo.bat Creates /Models, /Presets, /Resources.

Writes Models/config.json.

Generate Default ONNX Models

build_maevn_onnx.bat Exports lightweight default .onnx models for: 808, hihat, snare, piano, synth.

Optimizes them → places in /Models/.

Add Vocals

Export your own TTS + vocoder: vocals_tts.onnx, vocals_hifigan.onnx.

Place them in /Models/vocals/.

Build Plugin

cmake -B Build -S . -DJUCE_PATH="C:/JUCE" -DONNXRUNTIME_PATH="C:/onnxruntime".

cmake --build Build --config Release Outputs MAEVN.vst3.

Install

Copy MAEVN.vst3 → your DAW’s VST3 folder.

🧪 Developer Notes

Audio Pipeline (processBlock)

Parse DAW playhead (BPM, transport).

PatternEngine: parse active blocks → generate triggers.

OnnxEngine: run inference on relevant models.

AIFXEngine: apply FX (DSP/AI/Hybrid).

Output → DAW buffer.

Model Hot Reload

/Models/config.json maps logical roles (VOCAL, 808) → file paths.

MAEVN auto-reloads if models change on disk.

Presets

JSON format, e.g.:

Copy
{
  "name": "RadioVocals",
  "mode": 3,
  "params": {
    "reverbMix": 0.4,
    "compressorThreshold": -12.0
  },
  "category": "Vocal",
  "tags": ["Trap", "Radio", "Clean"]
}
Category + tags drive search and tag cloud UI.

Undo/Redo
Each action → ActionState snapshot (JSON).
Stored in GlobalUndoManager.
UI list in UndoHistoryComponent.
🚧 Roadmap

Automation Hooks: expose FX params as DAW automatable.
Drag-to-MIDI/Audio: drop [HOOK] blocks into DAW timeline.
Loop Region Sync: detect DAW selection and auto-fit arrangement.
Instrument Sequencer: hi-hat rolls, 808 glides.
Preset Exchange: import/export community preset packs.
⚔️ **MAEVN is a developer-first experimental AI DAW tool
