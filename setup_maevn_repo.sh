#!/usr/bin/env bash
# MAEVN Repository Setup Script (Linux/macOS)
# Prepares directory structure and initial configuration
set -euo pipefail

printf '\n========================================\n'
printf 'MAEVN Repository Setup\n'
printf '========================================\n\n'

# Create directory structure
printf 'Creating directory structure...\n'
mkdir -p Models/drums Models/instruments Models/vocals Presets Resources Build
printf '\u2713 Directories created\n'

# Create default config.json if missing
if [[ -f Models/config.json ]]; then
  printf '\u2713 config.json already exists\n'
else
  printf 'Creating Models/config.json...\n'
  cat > Models/config.json <<'CONFIG'
{
  "808": "drums/808_ddsp.onnx",
  "hihat": "drums/hihat_ddsp.onnx",
  "snare": "drums/snare_ddsp.onnx",
  "piano": "instruments/piano_ddsp.onnx",
  "synth": "instruments/synth_fm.onnx",
  "vocal_tts": "vocals/vocals_tts.onnx",
  "vocal_hifigan": "vocals/vocals_hifigan.onnx"
}
CONFIG
  printf '\u2713 config.json created\n'
fi

# Create README for vocal models
printf 'Creating vocal models README...\n'
cat > Models/vocals/README.txt <<'README'
MAEVN Vocal Models Directory
=============================

Provide **production-ready** ONNX models for the vocal pipeline (no demo placeholders are included):

1. vocals_tts.onnx - Text-to-Speech ONNX model (text/phoneme embeddings ➜ mel-spectrograms)
2. vocals_hifigan.onnx - HiFiGAN vocoder ONNX model (mel-spectrograms ➜ waveform)

Guidance for full-scale models:
- Train/export at 44.1kHz or 48kHz to match your session sample rate.
- Target ONNX opset 11+ with dynamic batch/time dimensions for DAW performance.
- Verify latency and artifact-free output before shipping.

You can bring your own trained models or adapt existing ones:
- Coqui TTS: https://github.com/coqui-ai/TTS (export to ONNX)
- ONNX Model Zoo: https://github.com/onnx/models (vocoder/tts examples)

After placing the models here, update Models/config.json so MAEVN can load them.
README
printf '\u2713 Vocal models README created\n'

printf '\n========================================\n'
printf 'Setup Complete!\n'
printf '========================================\n\n'
printf 'Next steps:\n'
printf '1. Run scripts/export_onnx_models.py to generate instrument models.\n'
printf '2. Add your vocal ONNX models to Models/vocals/.\n'
printf '3. Build with CMake (update JUCE/ONNX Runtime paths):\n'
printf '   cmake -B Build -S . -DJUCE_PATH="/path/to/JUCE" -DONNXRUNTIME_PATH="/path/to/onnxruntime"\n'
printf '   cmake --build Build --config Release\n\n'
