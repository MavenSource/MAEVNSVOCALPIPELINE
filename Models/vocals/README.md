# Vocal Models

This directory contains vocal ONNX models for the MAEVN plugin.

## Required Models

You need to supply the following vocal models:

- `vocals_tts.onnx` - Text-to-Speech model for vocal synthesis
- `vocals_hifigan.onnx` - HiFi-GAN vocoder for waveform generation

## How to Obtain

You can:
1. Export your own TTS models using frameworks like Coqui TTS, ESPnet, or SpeechT5
2. Use pre-trained HiFi-GAN vocoder weights from the official repository

## Model Requirements

- Input/Output format should be compatible with the ONNX Runtime
- TTS model should accept text embeddings and output mel-spectrograms
- HiFi-GAN should accept mel-spectrograms and output waveforms

## Example Export

```python
# Example: Export HiFi-GAN to ONNX
import torch

# Load your pre-trained HiFi-GAN model
model = load_hifigan_model()
model.eval()

# Create dummy input (mel-spectrogram)
dummy_mel = torch.randn(1, 80, 100)

# Export to ONNX
torch.onnx.export(
    model,
    dummy_mel,
    "vocals_hifigan.onnx",
    opset_version=18,
    input_names=['mel'],
    output_names=['audio']
)
```
