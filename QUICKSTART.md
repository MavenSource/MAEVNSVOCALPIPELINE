# MAEVN Quick Start Guide

## Installation (5 minutes)

### Prerequisites
- Windows 10/11, macOS 10.15+, or Linux
- DAW that supports VST3 (Reaper, Ableton, FL Studio, etc.)
- 4GB RAM minimum
- 500MB disk space

### Step 1: Download
```bash
git clone https://github.com/MavenSource/MAEVNSVOCALPIPELINE.git
cd MAEVNSVOCALPIPELINE
```

### Step 2: Setup
```bash
# Windows
setup_maevn_repo.bat

# Linux/macOS  
chmod +x setup_maevn_repo.sh
./setup_maevn_repo.sh
```

### Step 3: Generate Models
```bash
# Windows
build_maevn_onnx.bat

# Linux/macOS
python3 scripts/export_onnx_models.py
```

### Step 4: Build (requires JUCE and ONNX Runtime)
```bash
# Configure
cmake -B Build -S . -DJUCE_PATH="/path/to/JUCE" -DONNXRUNTIME_PATH="/path/to/onnxruntime"

# Build
cmake --build Build --config Release

# Install
cmake --install Build
```

### Step 5: Load in DAW
1. Open your DAW
2. Rescan plugins
3. Load "MAEVN" VST3
4. Start creating!

---

## First Track (10 minutes)

### 1. Launch MAEVN
Insert MAEVN on a new track in your DAW.

### 2. Set BPM
Set the BPM slider to match your project (default: 120).

### 3. Write Stage Script
In the stage script input box, enter:

```
[INTRO] Yo, check it
[HOOK] This is the hook, catchy and clean duration:4.0
[VERSE] First verse with some lyrics duration:8.0
[808] Deep bass pattern
[HIHAT] Fast trap hats
```

### 4. Parse Script
Click "Parse Script" button. You'll see timeline blocks appear in the track lanes.

### 5. Apply FX
1. Click on the "Vocals" track
2. Open preset browser
3. Select "RadioVocals" preset
4. Click "Load"

### 6. Play
Press play in your DAW and hear the generated audio!

---

## Stage Script Syntax

### Block Format
```
[TYPE] Content duration:X.X
```

### Supported Block Types

| Type | Description | Track |
|------|-------------|-------|
| `[INTRO]` | Intro section | Vocals |
| `[HOOK]` | Hook/Chorus | Vocals |
| `[VERSE]` | Verse section | Vocals |
| `[BRIDGE]` | Bridge section | Vocals |
| `[OUTRO]` | Outro section | Vocals |
| `[808]` | 808 bass | Bass |
| `[HIHAT]` | Hi-hats | Drums |
| `[SNARE]` | Snare | Drums |
| `[PIANO]` | Piano | Keys |
| `[SYNTH]` | Synth/Pad | Synth |

### Examples

**Simple Hook:**
```
[HOOK] Catchy hook lyrics here
```

**Custom Duration:**
```
[VERSE] Long verse duration:16.0
```

**Instrumental Pattern:**
```
[808] C2-C2-C3-C2 glide pattern
[HIHAT] 16th note rolls
```

---

## FX Presets

### Using Presets

1. **Browse Presets:**
   - Click preset browser
   - See categories: Vocal, 808, HiHat, etc.

2. **Search:**
   - Type in search box
   - Searches name, tags, description

3. **Filter by Tag:**
   - Click tag chips
   - e.g., "Trap", "Clean", "AI"

4. **Preview:**
   - Click preview button
   - Hear FX applied to sample

5. **Load:**
   - Click preset to load
   - FX applied to current track

### Included Presets

**RadioVocals** (Hybrid)
- Compressed and bright vocals
- AI enhancement
- Perfect for trap/pop

**Dirty808** (DSP)
- Heavy distorted bass
- Deep sub emphasis
- Aggressive compression

**WideHats** (DSP)
- Stereo widened hi-hats
- Bright and crisp
- Clean reverb tail

### Creating Custom Presets

1. Configure FX parameters
2. Click "Save Preset"
3. Enter name, category, tags
4. Click "Save"

---

## FX Modes

### Off
No processing. Audio passes through unchanged.

### DSP
Traditional DSP effects:
- Compressor
- 3-band EQ
- Reverb
- Limiter

### AI
AI-powered effects using ONNX models:
- AI autotune
- AI mastering
- Custom AI effects

### Hybrid
Best of both worlds:
1. DSP effects applied first
2. AI effects applied second
3. Combined processing chain

---

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| Ctrl+Z | Undo |
| Ctrl+Y | Redo |
| Ctrl+S | Save preset |
| Ctrl+P | Parse script |
| Space | Play/Pause (DAW) |

---

## Tips & Tricks

### 1. Quantization
Enable quantization to snap blocks to beat grid:
```cpp
// In code
patternEngine.setQuantizationEnabled(true);
```

### 2. BPM Automation
MAEVN syncs with DAW BPM automatically. Change DAW tempo, and MAEVN follows.

### 3. Undo/Redo
Every action is tracked. Click any item in undo history to jump to that state.

### 4. Model Hot Reload
Change .onnx files while plugin is running. They'll reload automatically.

### 5. Preset Tags
Use consistent tags for better organization:
- Genre: Trap, Pop, Hip-Hop
- Style: Clean, Dirty, Vintage
- Processing: AI, Hybrid, DSP

---

## Troubleshooting

### Plugin Doesn't Load
**Problem:** DAW can't find MAEVN.vst3

**Solution:**
1. Check VST3 folder location:
   - Windows: `C:\Program Files\Common Files\VST3\`
   - macOS: `/Library/Audio/Plug-Ins/VST3/`
   - Linux: `~/.vst3/`
2. Copy MAEVN.vst3 manually
3. Rescan plugins in DAW

### No Audio Output
**Problem:** Plugin loads but no sound

**Solution:**
1. Check FX mode isn't "Off"
2. Verify models loaded (check log)
3. Ensure blocks are in timeline
4. Check DAW routing

### High CPU Usage
**Problem:** CPU >50%

**Solution:**
1. Increase buffer size in DAW
2. Disable AI FX temporarily
3. Use lighter presets (DSP only)
4. Close other applications

### Models Not Loading
**Problem:** "Model not found" errors

**Solution:**
1. Run `build_maevn_onnx.bat` to generate models
2. Check `Models/config.json` paths
3. Verify .onnx files exist
4. Check file permissions

---

## Next Steps

### Explore Presets
Try all included presets to understand FX possibilities.

### Learn Stage Scripts
Experiment with different block types and durations.

### Read Developer Guide
For advanced features: `DEVELOPER_GUIDE.md`

### Check Architecture
Understand internals: `ARCHITECTURE.md`

### Join Community
- GitHub Issues: Report bugs
- Discussions: Share presets
- Discord: Chat with users

---

## Resources

- **Documentation:** `/docs` folder
- **Examples:** `/examples` folder
- **Presets:** `/Presets` folder
- **Models:** `/Models` folder

---

## Support

For help:
1. Check documentation
2. Search GitHub issues
3. Open new issue with:
   - OS and DAW version
   - Steps to reproduce
   - Error messages
   - Log files

---

## License

See LICENSE file for details.

---

**Happy creating with MAEVN! 🎵**
