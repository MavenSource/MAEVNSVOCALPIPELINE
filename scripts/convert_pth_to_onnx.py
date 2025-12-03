#!/usr/bin/env python3
"""
MAEVN PTH to ONNX Conversion Script
====================================
Converts PyTorch .pth vocal models to ONNX format for use with the MAEVN Vocal Pipeline.

This script supports:
- TTS (Text-to-Speech) models
- HiFi-GAN vocoder models
- Custom vocal synthesis models

Usage:
    python convert_pth_to_onnx.py --input model.pth --output model.onnx --model-type tts
    python convert_pth_to_onnx.py --input hifigan.pth --output vocoder.onnx --model-type hifigan
"""

import argparse
import os
import sys
import json

try:
    import torch
    import torch.nn as nn
    TORCH_AVAILABLE = True
except ImportError:
    TORCH_AVAILABLE = False
    print("Warning: PyTorch not available. Install with: pip install torch")

try:
    import onnx
    ONNX_AVAILABLE = True
except ImportError:
    ONNX_AVAILABLE = False


class SimpleTTSModel(nn.Module):
    """
    Simple TTS model architecture for demonstration.
    Replace with your actual TTS model architecture.
    """
    def __init__(self, input_dim=512, hidden_dim=512, output_dim=80, num_layers=3):
        super().__init__()
        self.input_dim = input_dim
        self.hidden_dim = hidden_dim
        self.output_dim = output_dim
        
        layers = []
        layers.append(nn.Linear(input_dim, hidden_dim))
        layers.append(nn.ReLU())
        
        for _ in range(num_layers - 2):
            layers.append(nn.Linear(hidden_dim, hidden_dim))
            layers.append(nn.ReLU())
        
        layers.append(nn.Linear(hidden_dim, output_dim))
        
        self.network = nn.Sequential(*layers)
    
    def forward(self, x):
        return self.network(x)


class SimpleHiFiGAN(nn.Module):
    """
    Simple HiFi-GAN vocoder architecture for demonstration.
    Replace with your actual HiFi-GAN model architecture.
    """
    def __init__(self, mel_channels=80, upsample_rates=[8, 8, 2, 2], 
                 upsample_kernel_sizes=[16, 16, 4, 4], 
                 upsample_initial_channel=512):
        super().__init__()
        self.mel_channels = mel_channels
        
        # Simplified vocoder with upsampling
        self.conv_pre = nn.Conv1d(mel_channels, upsample_initial_channel, 7, 1, 3)
        
        self.ups = nn.ModuleList()
        in_channels = upsample_initial_channel
        for i, (u, k) in enumerate(zip(upsample_rates, upsample_kernel_sizes)):
            out_channels = in_channels // 2
            self.ups.append(
                nn.ConvTranspose1d(in_channels, out_channels, k, u, padding=(k-u)//2)
            )
            in_channels = out_channels
        
        self.conv_post = nn.Conv1d(out_channels, 1, 7, 1, 3)
        self.tanh = nn.Tanh()
    
    def forward(self, x):
        x = self.conv_pre(x)
        for up in self.ups:
            x = torch.relu(x)
            x = up(x)
        x = torch.relu(x)
        x = self.conv_post(x)
        x = self.tanh(x)
        return x.squeeze(1)


class VocalEmbeddingModel(nn.Module):
    """
    Vocal embedding model that can load and use custom .pth weights.
    This model architecture supports various vocal synthesis tasks.
    """
    def __init__(self, config=None):
        super().__init__()
        
        # Default configuration
        self.config = config or {
            'input_dim': 256,
            'hidden_dim': 512,
            'output_dim': 80,  # Mel-spectrogram bins
            'num_layers': 4
        }
        
        # Build network
        layers = []
        in_dim = self.config['input_dim']
        hidden_dim = self.config['hidden_dim']
        
        # Input projection
        layers.append(nn.Linear(in_dim, hidden_dim))
        layers.append(nn.LayerNorm(hidden_dim))
        layers.append(nn.ReLU())
        
        # Hidden layers
        for _ in range(self.config['num_layers'] - 2):
            layers.append(nn.Linear(hidden_dim, hidden_dim))
            layers.append(nn.LayerNorm(hidden_dim))
            layers.append(nn.ReLU())
        
        # Output projection
        layers.append(nn.Linear(hidden_dim, self.config['output_dim']))
        
        self.network = nn.Sequential(*layers)
    
    def forward(self, x):
        return self.network(x)


def load_pth_model(pth_path, model_type='tts', config=None):
    """
    Load a PyTorch model from a .pth file.
    
    Args:
        pth_path: Path to the .pth file
        model_type: Type of model ('tts', 'hifigan', 'vocal_embedding')
        config: Optional model configuration dict
    
    Returns:
        Loaded PyTorch model
    """
    if not TORCH_AVAILABLE:
        raise RuntimeError("PyTorch is required. Install with: pip install torch")
    
    if not os.path.exists(pth_path):
        raise FileNotFoundError(f"Model file not found: {pth_path}")
    
    print(f"Loading .pth model from: {pth_path}")
    print("WARNING: Only load .pth files from trusted sources. "
          "Untrusted files may contain malicious code.")
    
    # Load the checkpoint
    # Note: weights_only=False is required for full model loading but poses security risks
    # with untrusted files. Only load models from trusted sources.
    checkpoint = torch.load(pth_path, map_location='cpu', weights_only=False)
    
    # Determine model architecture
    if model_type == 'tts':
        model = SimpleTTSModel(
            input_dim=config.get('input_dim', 512) if config else 512,
            hidden_dim=config.get('hidden_dim', 512) if config else 512,
            output_dim=config.get('output_dim', 80) if config else 80
        )
    elif model_type == 'hifigan':
        model = SimpleHiFiGAN(
            mel_channels=config.get('mel_channels', 80) if config else 80
        )
    elif model_type == 'vocal_embedding':
        model = VocalEmbeddingModel(config)
    else:
        raise ValueError(f"Unknown model type: {model_type}")
    
    # Load state dict
    if isinstance(checkpoint, dict):
        if 'model' in checkpoint:
            state_dict = checkpoint['model']
        elif 'state_dict' in checkpoint:
            state_dict = checkpoint['state_dict']
        elif 'generator' in checkpoint:  # For HiFi-GAN
            state_dict = checkpoint['generator']
        else:
            state_dict = checkpoint
    else:
        state_dict = checkpoint
    
    # Try to load state dict
    try:
        model.load_state_dict(state_dict, strict=False)
        print("Model weights loaded successfully")
    except Exception as e:
        print(f"Warning: Could not load all weights ({e})")
        print("Proceeding with randomly initialized weights")
    
    model.eval()
    return model


def export_to_onnx(model, output_path, model_type='tts', config=None):
    """
    Export a PyTorch model to ONNX format.
    
    Args:
        model: PyTorch model
        output_path: Output path for the ONNX file
        model_type: Type of model ('tts', 'hifigan', 'vocal_embedding')
        config: Optional configuration
    """
    if not TORCH_AVAILABLE:
        raise RuntimeError("PyTorch is required. Install with: pip install torch")
    
    print(f"Exporting model to ONNX: {output_path}")
    
    # Create dummy input based on model type
    if model_type == 'tts':
        # TTS input: text embeddings [batch, seq_len, embed_dim]
        input_dim = config.get('input_dim', 512) if config else 512
        dummy_input = torch.randn(1, input_dim)
        input_names = ['text_embedding']
        output_names = ['mel_spectrogram']
        dynamic_axes = {
            'text_embedding': {0: 'batch_size'},
            'mel_spectrogram': {0: 'batch_size'}
        }
    elif model_type == 'hifigan':
        # HiFi-GAN input: mel-spectrogram [batch, mel_channels, time]
        mel_channels = config.get('mel_channels', 80) if config else 80
        dummy_input = torch.randn(1, mel_channels, 100)
        input_names = ['mel_spectrogram']
        output_names = ['audio']
        dynamic_axes = {
            'mel_spectrogram': {0: 'batch_size', 2: 'time'},
            'audio': {0: 'batch_size', 1: 'time'}
        }
    elif model_type == 'vocal_embedding':
        input_dim = config.get('input_dim', 256) if config else 256
        dummy_input = torch.randn(1, input_dim)
        input_names = ['input']
        output_names = ['output']
        dynamic_axes = {
            'input': {0: 'batch_size'},
            'output': {0: 'batch_size'}
        }
    else:
        raise ValueError(f"Unknown model type: {model_type}")
    
    # Create output directory
    os.makedirs(os.path.dirname(output_path) if os.path.dirname(output_path) else '.', exist_ok=True)
    
    # Export to ONNX
    torch.onnx.export(
        model,
        dummy_input,
        output_path,
        export_params=True,
        opset_version=18,
        do_constant_folding=True,
        input_names=input_names,
        output_names=output_names,
        dynamic_axes=dynamic_axes
    )
    
    print(f"✓ Model exported successfully to: {output_path}")
    
    # Verify ONNX model if onnx is available
    if ONNX_AVAILABLE:
        try:
            onnx_model = onnx.load(output_path)
            onnx.checker.check_model(onnx_model)
            print("✓ ONNX model validation passed")
        except Exception as e:
            print(f"Warning: ONNX validation failed: {e}")
    
    return output_path


def create_default_vocal_model(output_dir, model_type='tts'):
    """
    Create and export a default vocal model for the MAEVN pipeline.
    This is useful when no .pth file is provided.
    
    Args:
        output_dir: Output directory for the ONNX model
        model_type: Type of model to create ('tts' or 'hifigan')
    
    Returns:
        Path to the exported ONNX model
    """
    if not TORCH_AVAILABLE:
        raise RuntimeError("PyTorch is required. Install with: pip install torch")
    
    print(f"Creating default {model_type} model...")
    
    os.makedirs(output_dir, exist_ok=True)
    
    if model_type == 'tts':
        model = SimpleTTSModel(input_dim=512, hidden_dim=512, output_dim=80)
        output_path = os.path.join(output_dir, 'vocals_tts.onnx')
        config = {'input_dim': 512, 'hidden_dim': 512, 'output_dim': 80}
    elif model_type == 'hifigan':
        model = SimpleHiFiGAN(mel_channels=80)
        output_path = os.path.join(output_dir, 'vocals_hifigan.onnx')
        config = {'mel_channels': 80}
    else:
        raise ValueError(f"Unknown model type: {model_type}")
    
    model.eval()
    export_to_onnx(model, output_path, model_type, config)
    
    return output_path


def update_model_config(models_dir, model_name, model_path):
    """
    Update the Models/config.json to include the new vocal model.
    
    Args:
        models_dir: Path to the Models directory
        model_name: Name of the model (e.g., 'vocal_tts', 'vocal_hifigan')
        model_path: Relative path to the model file
    """
    config_path = os.path.join(models_dir, 'config.json')
    
    if os.path.exists(config_path):
        with open(config_path, 'r') as f:
            config = json.load(f)
    else:
        config = {}
    
    config[model_name] = model_path
    
    with open(config_path, 'w') as f:
        json.dump(config, f, indent=2)
    
    print(f"✓ Updated config.json with {model_name}: {model_path}")


def main():
    parser = argparse.ArgumentParser(
        description='Convert PyTorch .pth vocal models to ONNX for MAEVN Vocal Pipeline'
    )
    parser.add_argument(
        '--input', '-i',
        type=str,
        help='Path to input .pth file'
    )
    parser.add_argument(
        '--output', '-o',
        type=str,
        help='Path to output .onnx file'
    )
    parser.add_argument(
        '--model-type', '-t',
        type=str,
        choices=['tts', 'hifigan', 'vocal_embedding'],
        default='tts',
        help='Type of model (default: tts)'
    )
    parser.add_argument(
        '--create-default',
        action='store_true',
        help='Create default vocal models if no input is provided'
    )
    parser.add_argument(
        '--output-dir',
        type=str,
        default='../Models/vocals',
        help='Output directory for default models (default: ../Models/vocals)'
    )
    parser.add_argument(
        '--input-dim',
        type=int,
        default=512,
        help='Input dimension for TTS model (default: 512)'
    )
    parser.add_argument(
        '--hidden-dim',
        type=int,
        default=512,
        help='Hidden dimension for model (default: 512)'
    )
    parser.add_argument(
        '--output-dim',
        type=int,
        default=80,
        help='Output dimension (mel channels) (default: 80)'
    )
    
    args = parser.parse_args()
    
    print("=" * 60)
    print("MAEVN PTH to ONNX Converter")
    print("=" * 60)
    
    config = {
        'input_dim': args.input_dim,
        'hidden_dim': args.hidden_dim,
        'output_dim': args.output_dim,
        'mel_channels': args.output_dim
    }
    
    if args.create_default or not args.input:
        # Create default models
        script_dir = os.path.dirname(os.path.abspath(__file__))
        output_dir = os.path.join(script_dir, args.output_dir)
        
        print("\nCreating default vocal models for MAEVN pipeline...")
        
        # Create TTS model
        tts_path = create_default_vocal_model(output_dir, 'tts')
        
        # Create HiFi-GAN model
        hifigan_path = create_default_vocal_model(output_dir, 'hifigan')
        
        # Update config.json
        models_dir = os.path.dirname(output_dir)
        update_model_config(models_dir, 'vocal_tts', 'vocals/vocals_tts.onnx')
        update_model_config(models_dir, 'vocal_hifigan', 'vocals/vocals_hifigan.onnx')
        
        print("\n" + "=" * 60)
        print("Default vocal models created successfully!")
        print(f"  TTS Model: {tts_path}")
        print(f"  HiFi-GAN Model: {hifigan_path}")
        print("=" * 60)
    else:
        # Convert provided .pth file
        if not args.output:
            # Generate output path from input
            base = os.path.splitext(args.input)[0]
            args.output = f"{base}.onnx"
        
        model = load_pth_model(args.input, args.model_type, config)
        export_to_onnx(model, args.output, args.model_type, config)
        
        print("\n" + "=" * 60)
        print("Conversion complete!")
        print(f"  Input: {args.input}")
        print(f"  Output: {args.output}")
        print("=" * 60)


if __name__ == "__main__":
    main()
