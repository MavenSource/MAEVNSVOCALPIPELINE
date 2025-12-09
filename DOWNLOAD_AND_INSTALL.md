# MAEVN Plugin - Download and Installation Guide

## 📥 Quick Download

**Get the latest pre-built MAEVN VST3 plugin for your platform:**

👉 **[Download from GitHub Releases](https://github.com/MavenSource/MAEVNSVOCALPIPELINE/releases/latest)**

### Available Packages

| Platform | File Name | Contents |
|----------|-----------|----------|
| 🪟 **Windows** | `MAEVN-Windows-x64.zip` | VST3 plugin + models + presets |
| 🍎 **macOS** | `MAEVN-macOS-Universal.zip` | VST3 + AU plugins + models + presets |
| 🐧 **Linux** | `MAEVN-Linux-x64.tar.gz` | VST3 plugin + models + presets |

---

## 🪟 Windows Installation

### Requirements
- Windows 10 or 11 (64-bit)
- VST3-compatible DAW (Reaper, FL Studio, Ableton Live, etc.)

### Installation Steps

1. **Download** `MAEVN-Windows-x64.zip` from the [releases page](https://github.com/MavenSource/MAEVNSVOCALPIPELINE/releases/latest)

2. **Extract** the ZIP file to a temporary location

3. **Copy the VST3 plugin:**
   - Navigate to the `VST3/` folder in the extracted files
   - Copy `MAEVN.vst3` to your VST3 plugins folder:
     ```
     C:\Program Files\Common Files\VST3\
     ```
   - You may need administrator privileges

4. **Copy Models and Presets (optional but recommended):**
   - Copy the `Models/` and `Presets/` folders to:
     ```
     C:\Users\YourUsername\AppData\Roaming\MAEVN\
     ```
   - Or place them in the same directory as the VST3 file

5. **Restart your DAW**

6. **Rescan plugins** in your DAW if necessary

7. **Load MAEVN** on an audio or MIDI track and start creating!

### Troubleshooting Windows

- **Plugin doesn't appear:** Ensure VST3 scanning is enabled in your DAW
- **Missing models error:** Make sure Models/ folder is in the correct location
- **Loading error:** Install Visual C++ Redistributable if not already installed

---

## 🍎 macOS Installation

### Requirements
- macOS 10.13 (High Sierra) or later
- Universal Binary supports both Intel and Apple Silicon Macs
- VST3 or AU-compatible DAW

### Installation Steps

1. **Download** `MAEVN-macOS-Universal.zip` from the [releases page](https://github.com/MavenSource/MAEVNSVOCALPIPELINE/releases/latest)

2. **Extract** the ZIP file (double-click in Finder)

3. **Install VST3 Plugin:**
   - Open the extracted folder and navigate to `VST3/`
   - Copy `MAEVN.vst3` to:
     ```
     /Library/Audio/Plug-Ins/VST3/
     ```
     or
     ```
     ~/Library/Audio/Plug-Ins/VST3/
     ```

4. **Install AU Plugin (Optional):**
   - Navigate to `AU/` folder
   - Copy `MAEVN.component` to:
     ```
     /Library/Audio/Plug-Ins/Components/
     ```
     or
     ```
     ~/Library/Audio/Plug-Ins/Components/
     ```

5. **Copy Models and Presets:**
   - Copy the `Models/` and `Presets/` folders to:
     ```
     ~/Library/Application Support/MAEVN/
     ```
   - You may need to create the MAEVN folder first

6. **Restart your DAW**

7. **Rescan plugins** if necessary

8. **Load MAEVN** and start producing!

### Troubleshooting macOS

- **"MAEVN is damaged" or Gatekeeper warning:**
  - Open Terminal and run:
    ```bash
    xattr -cr /Library/Audio/Plug-Ins/VST3/MAEVN.vst3
    xattr -cr /Library/Audio/Plug-Ins/Components/MAEVN.component
    ```
  - Or right-click the plugin and select "Open" with Option key held

- **Plugin not showing up:** Check that you're looking in the right format (VST3 or AU) in your DAW

- **Models not loading:** Verify the Models folder path is correct

---

## 🐧 Linux Installation

### Requirements
- Ubuntu 20.04+ or equivalent distribution (64-bit)
- VST3-compatible DAW (Reaper, Ardour, Bitwig, etc.)
- Required system libraries (see below)

### System Dependencies

Install required libraries:

```bash
sudo apt-get update
sudo apt-get install -y \
    libasound2 \
    libjack-jackd2-0 \
    libfreetype6 \
    libx11-6 \
    libxext6 \
    libxrandr2 \
    libxinerama1 \
    libxcursor1
```

### Installation Steps

1. **Download** `MAEVN-Linux-x64.tar.gz` from the [releases page](https://github.com/MavenSource/MAEVNSVOCALPIPELINE/releases/latest)

2. **Extract** the archive:
   ```bash
   tar -xzf MAEVN-Linux-x64.tar.gz
   cd MAEVN-Linux
   ```

3. **Install VST3 Plugin:**
   ```bash
   # System-wide installation
   sudo cp -r VST3/MAEVN.vst3 /usr/lib/vst3/
   
   # Or user installation
   mkdir -p ~/.vst3
   cp -r VST3/MAEVN.vst3 ~/.vst3/
   ```

4. **Copy Models and Presets:**
   ```bash
   mkdir -p ~/.config/MAEVN
   cp -r Models ~/.config/MAEVN/
   cp -r Presets ~/.config/MAEVN/
   ```

5. **Restart your DAW**

6. **Rescan plugins** in your DAW

7. **Load MAEVN** and enjoy!

### Troubleshooting Linux

- **Library errors:** Make sure all dependencies are installed
- **Permission errors:** Check file permissions on VST3 folder
- **Plugin not found:** Verify VST3 path matches your DAW's settings

---

## 🎤 Adding Vocal Models (All Platforms)

The pre-built plugin includes all **instrument models** (808, hi-hats, snare, piano, synth). 

To use **AI vocal synthesis**, you need to provide your own ONNX vocal models:

1. **Obtain or train vocal models:**
   - `vocals_tts.onnx` - Text-to-Speech model
   - `vocals_hifigan.onnx` - HiFi-GAN vocoder

2. **Convert PyTorch models to ONNX:**
   ```bash
   python scripts/convert_pth_to_onnx.py --input your_model.pth --output vocals_tts.onnx --model-type tts
   ```

3. **Place models in the vocal models folder:**
   - **Windows:** `C:\Users\YourUsername\AppData\Roaming\MAEVN\Models\vocals\`
   - **macOS:** `~/Library/Application Support/MAEVN/Models/vocals/`
   - **Linux:** `~/.config/MAEVN/Models/vocals/`

4. **Restart MAEVN** - it will auto-detect and load the new models

See the [main README](README.md#-embedding-custom-pth-models) for detailed conversion instructions.

---

## ✅ Verification

After installation, verify MAEVN is working:

1. **Open your DAW** (Reaper, Ableton, FL Studio, Logic, etc.)

2. **Create a new track** (audio or instrument track)

3. **Add MAEVN** to the track as an insert effect or instrument

4. **Check the UI** - you should see:
   - Timeline panel
   - FX controls
   - Preset browser
   - Instrument lanes (808, HiHat, Snare, Piano, Synth)

5. **Test presets:**
   - Click "Browse Presets"
   - Try loading "RadioVocals" or "Dirty808"
   - You should hear the effect when audio passes through

---

## 🎛️ Supported DAWs

MAEVN has been tested with:

| DAW | Windows | macOS | Linux | Notes |
|-----|---------|-------|-------|-------|
| **Reaper** | ✅ | ✅ | ✅ | Primary testing platform |
| **Ableton Live** | ✅ | ✅ | - | VST3/AU support |
| **FL Studio** | ✅ | ✅ | - | VST3 support |
| **Logic Pro** | - | ✅ | - | AU support |
| **Studio One** | ✅ | ✅ | - | VST3 support |
| **Bitwig Studio** | ✅ | ✅ | ✅ | VST3 support |
| **Ardour** | ✅ | ✅ | ✅ | VST3 support |

*MAEVN should work with any VST3-compatible DAW*

---

## 📚 Next Steps

Once installed, check out:

- **[Quick Start Guide](QUICKSTART.md)** - Get started with MAEVN
- **[README](README.md)** - Full feature documentation
- **[Architecture Guide](ARCHITECTURE.md)** - Technical details
- **[Audio Samples](https://github.com/MavenSource/MAEVNSVOCALPIPELINE/actions/workflows/generate-audio-samples.yml)** - Listen to MAEVN in action

---

## 💬 Support

Need help?

- 🐛 [Report Issues](https://github.com/MavenSource/MAEVNSVOCALPIPELINE/issues)
- 💡 [Feature Requests](https://github.com/MavenSource/MAEVNSVOCALPIPELINE/issues)
- 📖 [Documentation](https://github.com/MavenSource/MAEVNSVOCALPIPELINE)

---

Made with ❤️ by the MAEVN Team
