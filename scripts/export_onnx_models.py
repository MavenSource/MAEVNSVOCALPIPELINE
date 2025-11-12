#!/usr/bin/env python3
"""
MAEVN ONNX Model Export Script
Generates lightweight ONNX models for instrument synthesis
"""

import torch
import torch.nn as nn
import torch.onnx
import os
import numpy as np

class SimpleDDSP808(nn.Module):
    """Simple DDSP-inspired 808 bass generator"""
    def __init__(self):
        super().__init__()
        self.fc1 = nn.Linear(128, 256)
        self.fc2 = nn.Linear(256, 512)
        self.fc3 = nn.Linear(512, 1024)
        
    def forward(self, x):
        x = torch.relu(self.fc1(x))
        x = torch.relu(self.fc2(x))
        x = torch.tanh(self.fc3(x))
        return x

class SimpleHiHat(nn.Module):
    """Simple hi-hat generator"""
    def __init__(self):
        super().__init__()
        self.fc1 = nn.Linear(64, 128)
        self.fc2 = nn.Linear(128, 256)
        self.fc3 = nn.Linear(256, 512)
        
    def forward(self, x):
        x = torch.relu(self.fc1(x))
        x = torch.relu(self.fc2(x))
        x = torch.tanh(self.fc3(x))
        return x

class SimpleSnare(nn.Module):
    """Simple snare generator"""
    def __init__(self):
        super().__init__()
        self.fc1 = nn.Linear(64, 128)
        self.fc2 = nn.Linear(128, 256)
        self.fc3 = nn.Linear(256, 512)
        
    def forward(self, x):
        x = torch.relu(self.fc1(x))
        x = torch.relu(self.fc2(x))
        x = torch.tanh(self.fc3(x))
        return x

class SimplePiano(nn.Module):
    """Simple piano generator"""
    def __init__(self):
        super().__init__()
        self.fc1 = nn.Linear(256, 512)
        self.fc2 = nn.Linear(512, 1024)
        self.fc3 = nn.Linear(1024, 2048)
        
    def forward(self, x):
        x = torch.relu(self.fc1(x))
        x = torch.relu(self.fc2(x))
        x = torch.tanh(self.fc3(x))
        return x

class SimpleSynth(nn.Module):
    """Simple FM synth generator"""
    def __init__(self):
        super().__init__()
        self.fc1 = nn.Linear(128, 256)
        self.fc2 = nn.Linear(256, 512)
        self.fc3 = nn.Linear(512, 1024)
        
    def forward(self, x):
        x = torch.relu(self.fc1(x))
        x = torch.relu(self.fc2(x))
        x = torch.tanh(self.fc3(x))
        return x

def export_model(model, dummy_input, output_path, model_name):
    """Export PyTorch model to ONNX format"""
    print(f"Exporting {model_name} to {output_path}")
    
    # Create directory if it doesn't exist
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    
    # Export to ONNX
    torch.onnx.export(
        model,
        dummy_input,
        output_path,
        export_params=True,
        opset_version=13,
        do_constant_folding=True,
        input_names=['input'],
        output_names=['output'],
        dynamic_axes={
            'input': {0: 'batch_size'},
            'output': {0: 'batch_size'}
        }
    )
    
    print(f"✓ {model_name} exported successfully")

def main():
    """Main export function"""
    print("=" * 60)
    print("MAEVN ONNX Model Export")
    print("=" * 60)
    
    # Set to evaluation mode
    torch.set_grad_enabled(False)
    
    # Export 808 model
    model_808 = SimpleDDSP808()
    model_808.eval()
    dummy_808 = torch.randn(1, 128)
    export_model(model_808, dummy_808, "../Models/drums/808_ddsp.onnx", "808 Bass")
    
    # Export hi-hat model
    model_hihat = SimpleHiHat()
    model_hihat.eval()
    dummy_hihat = torch.randn(1, 64)
    export_model(model_hihat, dummy_hihat, "../Models/drums/hihat_ddsp.onnx", "Hi-Hat")
    
    # Export snare model
    model_snare = SimpleSnare()
    model_snare.eval()
    dummy_snare = torch.randn(1, 64)
    export_model(model_snare, dummy_snare, "../Models/drums/snare_ddsp.onnx", "Snare")
    
    # Export piano model
    model_piano = SimplePiano()
    model_piano.eval()
    dummy_piano = torch.randn(1, 256)
    export_model(model_piano, dummy_piano, "../Models/instruments/piano_ddsp.onnx", "Piano")
    
    # Export synth model
    model_synth = SimpleSynth()
    model_synth.eval()
    dummy_synth = torch.randn(1, 128)
    export_model(model_synth, dummy_synth, "../Models/instruments/synth_fm.onnx", "Synth")
    
    print("=" * 60)
    print("All models exported successfully!")
    print("Note: Vocal models (TTS + HiFiGAN) must be supplied by user")
    print("=" * 60)

if __name__ == "__main__":
    main()
