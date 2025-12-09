# MAEVN + Audacity Integration Guide

## 🎯 Overview

This guide provides step-by-step instructions for integrating MAEVN with **Audacity 3.x** (the free, open-source audio editor). MAEVN is built as a VST3 plugin, and Audacity has supported VST3 effects since version 3.0.

---

## ⚠️ Important Limitations

**Audacity VST3 Support:**
- ✅ **VST3 Effects** — Fully supported
- ❌ **VST3 Instruments** — **NOT supported**

Since MAEVN is both an instrument synthesizer and effects processor, **only the effects/FX processing capabilities** of MAEVN will work in Audacity. The AI vocal synthesis and instrument generation features require a full DAW that supports VST instruments (like Reaper, Ableton, FL Studio, Logic Pro, etc.).

### What Works in Audacity ✅
- **FX Processing Chain** — Compressor, EQ, Reverb, Limiter
- **AI-Powered Effects** — AI Autotune, AI Mastering (if models are loaded)
- **Hybrid DSP + AI FX** — Combined processing chains
- **Preset System** — Load and apply FX presets to audio tracks

### What Doesn't Work in Audacity ❌
- **AI Vocal Synthesis** — Text-to-speech generation
- **Instrument Generation** — 808s, hi-hats, snares, piano, synth pads
- **Stage Script Parsing** — Timeline arrangement features
- **MIDI Input** — Instrument triggering

### Recommended Workflow
If you need the full MAEVN experience (vocals + instruments), use a full-featured DAW like:
- **Reaper** (affordable, cross-platform)
- **Ableton Live**
- **FL Studio**
- **Logic Pro** (macOS)
- **Studio One**

For Audacity users, MAEVN can serve as a powerful **effects processor** for existing audio tracks.

---

## 📋 Prerequisites

### System Requirements
| Requirement | Specification |
|-------------|---------------|
| **Operating System** | Windows 10/11, macOS 10.15+, or Linux |
| **Audacity Version** | 3.0 or higher (VST3 support required) |
| **Architecture** | 64-bit (plugin and Audacity must match) |
| **RAM** | 4GB minimum, 8GB recommended |
| **Disk Space** | 500MB for plugin + models |
| **CPU** | Multi-core processor recommended for AI processing |

### Download Audacity
If you don't have Audacity installed:
1. Visit: https://www.audacityteam.org/download/
2. Download the latest version (3.x or higher)
3. Install following the on-screen instructions
4. Verify version: Open Audacity → `Help` → `About Audacity`

---

## 🛠️ Installation

### Step 1: Build or Obtain MAEVN Plugin

#### Option A: Download Pre-Built Plugin (if available)
1. Visit the [MAEVN releases](https://github.com/MavenSource/MAEVNSVOCALPIPELINE/releases) page
2. Download the latest release for your platform
3. Extract the archive to a temporary location

#### Option B: Build from Source

**Clone the Repository:**
```bash
git clone https://github.com/MavenSource/MAEVNSVOCALPIPELINE.git
cd MAEVNSVOCALPIPELINE
```

**Setup Repository (Windows):**
```batch
setup_maevn_repo.bat
```

**Setup Repository (Linux/macOS):**
```bash
chmod +x setup_maevn_repo.sh
./setup_maevn_repo.sh
```

**Install Build Dependencies:**
```bash
# Install Python dependencies for ONNX model generation
pip install -r scripts/requirements.txt

# Generate default ONNX models (for FX processing)
python3 scripts/export_onnx_models.py
```

**Build the Plugin:**
```bash
# Configure CMake (adjust paths for your system)
cmake -B Build -S . \
    -DJUCE_PATH="/path/to/JUCE" \
    -DONNXRUNTIME_PATH="/path/to/onnxruntime" \
    -DCMAKE_BUILD_TYPE=Release

# Build the VST3
cmake --build Build --config Release
```

The compiled plugin will be located in:
- Windows: `Build/MAEVN_artefacts/Release/VST3/MAEVN.vst3`
- macOS: `Build/MAEVN_artefacts/Release/VST3/MAEVN.vst3`
- Linux: `Build/MAEVN_artefacts/Release/VST3/MAEVN.vst3`

---

### Step 2: Install MAEVN to System VST3 Folder

Copy the `MAEVN.vst3` file to your system's VST3 plugin directory:

#### Windows
```batch
# Copy to system VST3 folder
copy "Build\MAEVN_artefacts\Release\VST3\MAEVN.vst3" "C:\Program Files\Common Files\VST3\"
```

**Manual Installation (Windows):**
1. Navigate to `Build\MAEVN_artefacts\Release\VST3\`
2. Copy `MAEVN.vst3` (entire folder)
3. Paste into `C:\Program Files\Common Files\VST3\`
4. You may need administrator permissions

#### macOS
```bash
# Copy to system VST3 folder
sudo cp -R Build/MAEVN_artefacts/Release/VST3/MAEVN.vst3 /Library/Audio/Plug-Ins/VST3/
```

**Manual Installation (macOS):**
1. Open Finder
2. Navigate to `Build/MAEVN_artefacts/Release/VST3/`
3. Copy `MAEVN.vst3` bundle
4. Paste into `/Library/Audio/Plug-Ins/VST3/`
5. You may need to authenticate with your password

#### Linux
```bash
# Copy to user VST3 folder
mkdir -p ~/.vst3
cp -R Build/MAEVN_artefacts/Release/VST3/MAEVN.vst3 ~/.vst3/

# Or copy to system VST3 folder
sudo cp -R Build/MAEVN_artefacts/Release/VST3/MAEVN.vst3 /usr/local/lib/vst3/
```

---

### Step 3: Copy Models and Presets

MAEVN requires ONNX models and preset files to function:

#### Windows
```batch
# Copy Models folder to plugin location
xcopy /E /I "Models" "C:\Program Files\Common Files\VST3\MAEVN.vst3\Contents\Models"

# Copy Presets folder
xcopy /E /I "Presets" "C:\Program Files\Common Files\VST3\MAEVN.vst3\Contents\Presets"
```

#### macOS
```bash
# Copy Models folder to plugin bundle
sudo cp -R Models /Library/Audio/Plug-Ins/VST3/MAEVN.vst3/Contents/

# Copy Presets folder
sudo cp -R Presets /Library/Audio/Plug-Ins/VST3/MAEVN.vst3/Contents/
```

#### Linux
```bash
# Copy Models folder
cp -R Models ~/.vst3/MAEVN.vst3/Contents/

# Copy Presets folder
cp -R Presets ~/.vst3/MAEVN.vst3/Contents/
```

---

### Step 4: Enable Plugin in Audacity

1. **Launch Audacity**

2. **Open Plugin Manager:**
   - Go to `Edit` → `Preferences` (Windows/Linux)
   - Or `Audacity` → `Preferences` (macOS)
   - Select `Effects` from the left sidebar
   - Click `Manage` button (or `Plugin Manager`)

3. **Rescan for Plugins:**
   - Click `Rescan` or `Rescan All` button
   - Audacity will scan the VST3 directories
   - This may take 30-60 seconds

4. **Locate MAEVN:**
   - In the plugin list, look for "MAEVN" or "MavenSource MAEVN"
   - It should appear under the VST3 category
   - Check the box next to MAEVN to enable it

5. **Apply Changes:**
   - Click `OK` to save preferences
   - Close and reopen Audacity (optional, but recommended)

---

## 🎚️ Using MAEVN in Audacity

### Basic FX Processing Workflow

#### 1. Import or Record Audio
- Import an audio file: `File` → `Open` or drag-and-drop
- Or record new audio: Click the red record button

#### 2. Select Audio Region
- Use the Selection Tool (F1)
- Click and drag to select the audio you want to process
- Or press `Ctrl+A` (Windows/Linux) / `Cmd+A` (macOS) to select all

#### 3. Apply MAEVN Effect
- Go to `Effect` menu → `VST3` submenu (or just `Effect`)
- Select **MAEVN** from the list
- The MAEVN plugin window will open

#### 4. Configure FX Settings
**FX Mode Selection:**
- **Off** — Bypass (no processing)
- **DSP** — Traditional effects (Compressor → EQ → Reverb → Limiter)
- **AI** — AI-powered effects (requires AI models)
- **Hybrid** — DSP + AI combined processing

**Parameter Controls:**
- **Compressor:** Threshold, Ratio, Attack, Release
- **EQ:** Low, Mid, High gain controls (3-band)
- **Reverb:** Room Size, Damping, Wet/Dry Mix
- **Limiter:** Ceiling, Release time

#### 5. Load FX Preset (Optional)
- Click **Preset Browser** button
- Browse available presets:
  - `RadioVocals.json` — Bright, compressed vocal sound
  - `Dirty808.json` — Heavy bass distortion (for bass-heavy tracks)
  - `WideHats.json` — Stereo-widened effect
  - `CinematicVocals.json` — Grammy-quality vocal processing
- Click preset to load parameters
- Preview the effect if supported

#### 6. Preview and Apply
- Click **Preview** button to hear the effect (if available)
- Adjust parameters as needed
- Click **Apply** or **OK** to process the audio
- Wait for processing to complete

#### 7. Undo if Needed
- If you don't like the result: `Edit` → `Undo` (Ctrl+Z)
- Try different presets or adjust parameters

---

### Example Use Cases

#### Use Case 1: Vocal Enhancement
**Goal:** Make vocals sound radio-ready

1. Import vocal track
2. Select entire vocal region
3. Apply MAEVN effect
4. Set FX Mode to **Hybrid**
5. Load `RadioVocals.json` preset
6. Adjust parameters:
   - Compressor Threshold: `-12 dB`
   - EQ High Gain: `+3 dB`
   - Reverb Mix: `30%`
7. Apply effect
8. Export processed audio

#### Use Case 2: Bass Enhancement
**Goal:** Add punch and depth to bass-heavy track

1. Import bass/808 track
2. Select bass region
3. Apply MAEVN effect
4. Set FX Mode to **DSP**
5. Load `Dirty808.json` preset
6. Adjust parameters:
   - EQ Low Gain: `+4 dB`
   - Compressor Ratio: `4:1`
7. Apply effect

#### Use Case 3: Cinematic Vocal Processing
**Goal:** Apply Grammy-quality vocal effects

1. Import vocal track
2. Select vocal performance
3. Apply MAEVN effect
4. Set FX Mode to **Hybrid**
5. Load `CinematicVocals.json` preset
6. Fine-tune Ghost Choir and Tone Shaper controls
7. Apply effect
8. Export final master

---

## 🎛️ Understanding MAEVN FX Modes

### Off Mode
- **Processing:** None (bypass)
- **CPU Usage:** Minimal
- **Use Case:** Compare dry vs. processed audio

### DSP Mode
- **Processing Chain:**
  1. Compressor (dynamics control)
  2. 3-Band EQ (frequency shaping)
  3. Reverb (spatial effects)
  4. Limiter (peak control)
- **CPU Usage:** Low-Medium
- **Use Case:** Traditional mixing, mastering

### AI Mode
- **Processing Chain:**
  1. AI Autotune (pitch correction)
  2. AI Mastering (intelligent dynamics)
  3. Custom AI effects (if models loaded)
- **CPU Usage:** Medium-High
- **Use Case:** Modern AI-enhanced production

### Hybrid Mode
- **Processing Chain:**
  1. DSP effects (Compressor → EQ → Reverb → Limiter)
  2. AI effects (Autotune → Mastering)
- **CPU Usage:** High
- **Use Case:** Best of both worlds, professional results

---

## 🔧 Troubleshooting

### Plugin Not Appearing in Audacity

**Problem:** MAEVN doesn't show up in the Effects menu

**Solutions:**
1. **Verify VST3 Location:**
   - Check that `MAEVN.vst3` is in the correct folder:
     - Windows: `C:\Program Files\Common Files\VST3\`
     - macOS: `/Library/Audio/Plug-Ins/VST3/`
     - Linux: `~/.vst3/` or `/usr/local/lib/vst3/`

2. **Check Architecture Match:**
   - Ensure both Audacity and MAEVN are 64-bit (or both 32-bit)
   - On macOS: Check if you need Intel or Apple Silicon version

3. **Rescan Plugins:**
   - `Edit` → `Preferences` → `Effects` → `Manage`
   - Click `Rescan All`
   - Look for MAEVN in the plugin list
   - Enable the checkbox next to MAEVN

4. **Restart Audacity:**
   - Completely close Audacity
   - Relaunch and check the Effects menu again

5. **Check Audacity Version:**
   - MAEVN requires Audacity 3.0+ for VST3 support
   - Verify: `Help` → `About Audacity`
   - Update if version is below 3.0

---

### Plugin Loads But No Audio Processing

**Problem:** Effect applies but audio sounds unchanged

**Solutions:**
1. **Check FX Mode:**
   - Ensure FX mode is not set to "Off"
   - Try "DSP" mode first (simplest)

2. **Verify Models Loaded:**
   - Check that `Models/` folder is in plugin location
   - AI mode requires valid ONNX models
   - Try DSP mode to isolate model issues

3. **Check Input Selection:**
   - Ensure you've selected audio before applying effect
   - Highlight the waveform region to process

4. **Increase Processing Time:**
   - Some presets need time to process
   - Watch for Audacity's progress bar

---

### High CPU Usage / Crackling Audio

**Problem:** Audio stutters or CPU usage is very high

**Solutions:**
1. **Use DSP Mode Instead of AI/Hybrid:**
   - AI processing is more CPU-intensive
   - DSP mode is lightweight and real-time safe

2. **Increase Buffer Size:**
   - `Edit` → `Preferences` → `Devices`
   - Increase Latency (higher buffer size)

3. **Process in Segments:**
   - Select smaller sections of audio at a time
   - Apply effect to 10-20 second chunks

4. **Disable Unnecessary Effects:**
   - Use minimal preset with fewer active modules
   - Avoid stacking multiple heavy effects

5. **Close Other Applications:**
   - Free up system resources
   - Close browser tabs, video players, etc.

---

### "Model Not Found" Errors

**Problem:** AI mode fails with model loading errors

**Solutions:**
1. **Generate Default Models:**
   ```bash
   cd /path/to/MAEVNSVOCALPIPELINE
   python3 scripts/export_onnx_models.py
   ```

2. **Copy Models to Plugin:**
   - Ensure `Models/` folder is inside plugin bundle
   - Check `Models/config.json` exists

3. **Use DSP Mode:**
   - If AI models aren't critical, stick with DSP processing
   - DSP mode doesn't require ONNX models

4. **Check File Permissions:**
   - On Linux/macOS, ensure plugin can read Models folder
   - Fix permissions: `chmod -R 755 ~/.vst3/MAEVN.vst3/`

---

### Plugin Crashes Audacity

**Problem:** Audacity closes when loading MAEVN

**Solutions:**
1. **Check Compatibility:**
   - Verify 64-bit plugin with 64-bit Audacity
   - Check macOS architecture (Intel vs. Apple Silicon)

2. **Rebuild Plugin:**
   - Try rebuilding from source with latest dependencies
   - Ensure JUCE and ONNX Runtime are compatible versions

3. **Test in Standalone Mode:**
   - Build the standalone version of MAEVN
   - Test if issue is plugin-specific or code-level

4. **Report Bug:**
   - If crashes persist, open an issue on GitHub
   - Include: OS version, Audacity version, crash logs

---

## 📚 Additional Resources

### Documentation
- **README.md** — Project overview and features
- **QUICKSTART.md** — Quick start guide for DAW usage
- **DEVELOPER_GUIDE.md** — Technical documentation for contributors
- **ARCHITECTURE.md** — System architecture and design

### External Links
- **Audacity Manual:** https://manual.audacityteam.org/
- **Audacity VST Plugin Guide:** https://manual.audacityteam.org/man/effect_menu_vst.html
- **MAEVN GitHub:** https://github.com/MavenSource/MAEVNSVOCALPIPELINE
- **JUCE Framework:** https://juce.com/
- **ONNX Runtime:** https://onnxruntime.ai/

### Support
- **GitHub Issues:** https://github.com/MavenSource/MAEVNSVOCALPIPELINE/issues
- **Discussions:** https://github.com/MavenSource/MAEVNSVOCALPIPELINE/discussions

---

## 🚀 Going Beyond Audacity

While MAEVN's FX capabilities work well in Audacity, the full MAEVN experience (AI vocals, instruments, timeline arrangement) requires a DAW with VST instrument support.

### Recommended DAWs for Full MAEVN Experience

| DAW | Price | Platform | MAEVN Support |
|-----|-------|----------|---------------|
| **Reaper** | $60 (discounted) | Win/Mac/Linux | ✅ Full support |
| **Ableton Live** | $99-$749 | Win/Mac | ✅ Full support |
| **FL Studio** | $99-$499 | Win/Mac | ✅ Full support |
| **Logic Pro** | $199 | macOS | ✅ Full support |
| **Studio One** | $99-$399 | Win/Mac | ✅ Full support |
| **Bitwig Studio** | $299-$399 | Win/Mac/Linux | ✅ Full support |

**Why use a full DAW?**
- ✅ VST instrument support for AI vocals and 808s
- ✅ MIDI routing and sequencing
- ✅ Timeline arrangement with stage scripts
- ✅ Multi-track recording and mixing
- ✅ DAW automation for parameters
- ✅ Drag-and-drop MIDI/audio export

---

## 🎵 Example Workflow: Audacity + Full DAW

### Hybrid Approach
1. **Generate in DAW:**
   - Use MAEVN in Reaper/Ableton/etc. to generate AI vocals and instruments
   - Export stems as WAV files

2. **Edit in Audacity:**
   - Import stems into Audacity
   - Apply noise reduction, trimming, fades
   - Use MAEVN FX for additional processing

3. **Final Mix in DAW:**
   - Import processed stems back into DAW
   - Apply final mastering and export

---

## ✅ Summary Checklist

### Installation Checklist
- [ ] Audacity 3.0+ installed
- [ ] MAEVN.vst3 built or downloaded
- [ ] Plugin copied to system VST3 folder
- [ ] Models folder copied to plugin location
- [ ] Presets folder copied to plugin location
- [ ] Plugin enabled in Audacity Preferences
- [ ] Audacity restarted
- [ ] MAEVN appears in Effects menu

### Usage Checklist
- [ ] Audio track imported or recorded
- [ ] Audio region selected
- [ ] MAEVN effect opened from Effects menu
- [ ] FX mode configured (DSP/AI/Hybrid)
- [ ] Preset loaded (optional)
- [ ] Parameters adjusted
- [ ] Effect previewed and applied
- [ ] Result sounds as expected

---

## 📝 License

MAEVN is released under the MIT License. See the LICENSE file for details.

---

<div align="center">

**🎚️ MAEVN — Dynamic AI-Powered Audio Processing**

Made with ❤️ by the MAEVN Team

[GitHub](https://github.com/MavenSource/MAEVNSVOCALPIPELINE) • [Documentation](README.md) • [Support](https://github.com/MavenSource/MAEVNSVOCALPIPELINE/issues)

</div>
