#!/usr/bin/env python3
"""
MAEVN Text-to-Vocal Sample Generator
=====================================
Generates 5 text-to-vocal samples demonstrating the MAEVN Vocal Pipeline
for functional transparency.

This script showcases:
- Formant-based vocal synthesis
- Text-to-Speech processing
- Multiple vocal styles and emotions
- Integration with the MAEVN pipeline

Output: 5 stereo WAV files at 44.1kHz, 16-bit
"""

import numpy as np
from scipy import signal
from scipy.io import wavfile
import os
import sys
import json
import io

# Configure UTF-8 encoding for stdout to prevent UnicodeEncodeError
if sys.stdout.encoding != 'utf-8':
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

# Audio Constants
SAMPLE_RATE = 44100
DEFAULT_BPM = 120

# Musical Constants
PITCH_MAP = {
    'C2': 36, 'D2': 38, 'E2': 40, 'F2': 41, 'G2': 43, 'A2': 45, 'B2': 47,
    'C3': 48, 'D3': 50, 'E3': 52, 'F3': 53, 'G3': 55, 'A3': 57, 'B3': 59,
    'C4': 60, 'D4': 62, 'E4': 64, 'F4': 65, 'G4': 67, 'A4': 69, 'B4': 71,
    'C5': 72, 'D5': 74, 'E5': 76
}


def midi_to_freq(midi_note):
    """Convert MIDI note number to frequency in Hz."""
    return 440.0 * (2.0 ** ((midi_note - 69) / 12.0))


def generate_envelope(attack, decay, sustain, release, duration, sample_rate=SAMPLE_RATE):
    """Generate ADSR envelope."""
    num_samples = int(duration * sample_rate)
    envelope = np.zeros(num_samples)
    
    attack_samples = int(attack * sample_rate)
    decay_samples = int(decay * sample_rate)
    release_samples = int(release * sample_rate)
    sustain_samples = max(0, num_samples - attack_samples - decay_samples - release_samples)
    
    # Attack
    if attack_samples > 0:
        envelope[:attack_samples] = np.linspace(0, 1, attack_samples)
    
    # Decay
    start = attack_samples
    end = min(start + decay_samples, num_samples)
    if end > start:
        envelope[start:end] = np.linspace(1, sustain, end - start)
    
    # Sustain
    start = attack_samples + decay_samples
    end = min(start + sustain_samples, num_samples)
    if end > start:
        envelope[start:end] = sustain
    
    # Release
    start = max(0, num_samples - release_samples)
    if start < num_samples:
        envelope[start:] = np.linspace(sustain, 0, len(envelope[start:]))
    
    return envelope


class VocalSynthesizer:
    """
    Formant-based vocal synthesizer for MAEVN pipeline.
    Simulates the human vocal tract using formant filtering.
    """
    
    # Formant frequencies for vowels (F1, F2, F3)
    FORMANTS = {
        'a': [730, 1090, 2440],   # "ah" as in "father"
        'e': [530, 1840, 2480],   # "eh" as in "bed"
        'i': [270, 2290, 3010],   # "ee" as in "see"
        'o': [570, 840, 2410],    # "oh" as in "go"
        'u': [440, 1020, 2240],   # "oo" as in "moon"
        'ae': [660, 1720, 2410],  # "a" as in "cat"
        'uh': [520, 1190, 2390],  # "uh" as in "but"
    }
    
    # Consonant noise characteristics
    CONSONANTS = {
        's': {'freq': 6000, 'bandwidth': 4000, 'duration': 0.1},
        'sh': {'freq': 3500, 'bandwidth': 2000, 'duration': 0.12},
        'f': {'freq': 8000, 'bandwidth': 6000, 'duration': 0.08},
        't': {'freq': 4000, 'bandwidth': 3000, 'duration': 0.03},
        'k': {'freq': 2500, 'bandwidth': 1500, 'duration': 0.04},
        'ch': {'freq': 4500, 'bandwidth': 2500, 'duration': 0.08},
    }
    
    def __init__(self, sample_rate=SAMPLE_RATE):
        self.sample_rate = sample_rate
    
    def generate_glottal_pulse(self, duration, f0, vibrato_depth=0.02, vibrato_rate=5.0):
        """
        Generate glottal pulse train with optional vibrato.
        
        Args:
            duration: Duration in seconds
            f0: Fundamental frequency (Hz)
            vibrato_depth: Vibrato depth (fraction of f0)
            vibrato_rate: Vibrato rate (Hz)
        
        Returns:
            Glottal pulse train
        """
        num_samples = int(duration * self.sample_rate)
        t = np.arange(num_samples) / self.sample_rate
        
        # Apply vibrato
        vibrato = 1.0 + vibrato_depth * np.sin(2 * np.pi * vibrato_rate * t)
        freq = f0 * vibrato
        
        # Generate glottal source with harmonics
        phase = np.cumsum(2 * np.pi * freq / self.sample_rate)
        source = np.zeros(num_samples)
        
        # Add harmonics with decreasing amplitude
        for h in range(1, 20):
            amp = 1.0 / (h ** 1.2)  # Spectral rolloff
            source += amp * np.sin(h * phase)
        
        # Normalize - properly handle zero case
        max_val = np.max(np.abs(source))
        source /= max_val + 1e-8
        
        return source
    
    def apply_formants(self, source, formant_freqs, bandwidth=100):
        """
        Apply formant filtering to glottal source.
        
        Args:
            source: Glottal pulse train
            formant_freqs: List of formant frequencies [F1, F2, F3]
            bandwidth: Formant bandwidth (Hz)
        
        Returns:
            Formant-filtered signal
        """
        nyquist = self.sample_rate / 2
        output = np.zeros_like(source)
        
        for formant in formant_freqs:
            low = max(0.01, (formant - bandwidth) / nyquist)
            high = min(0.99, (formant + bandwidth) / nyquist)
            
            if low < high:
                b, a = signal.butter(2, [low, high], btype='band')
                filtered = signal.filtfilt(b, a, source)
                output += filtered
        
        return output
    
    def synthesize_vowel(self, vowel, duration, pitch, amplitude=0.5, vibrato=True):
        """
        Synthesize a single vowel sound.
        
        Args:
            vowel: Vowel character ('a', 'e', 'i', 'o', 'u')
            duration: Duration in seconds
            pitch: MIDI note number or frequency
            amplitude: Output amplitude (0-1)
            vibrato: Whether to apply vibrato
        
        Returns:
            Synthesized vowel audio
        """
        if isinstance(pitch, str):
            pitch = PITCH_MAP.get(pitch, 60)
        
        f0 = midi_to_freq(pitch) if pitch < 200 else pitch
        
        # Get formants for this vowel
        formants = self.FORMANTS.get(vowel.lower(), self.FORMANTS['a'])
        
        # Generate glottal source
        vibrato_depth = 0.015 if vibrato else 0.0
        source = self.generate_glottal_pulse(duration, f0, vibrato_depth)
        
        # Apply formant filtering
        output = self.apply_formants(source, formants)
        
        # Apply envelope
        envelope = generate_envelope(0.02, 0.05, 0.7, 0.15, duration, self.sample_rate)
        output *= envelope
        
        # Normalize and apply amplitude
        output /= np.max(np.abs(output) + 1e-8)
        output *= amplitude
        
        return output
    
    def synthesize_consonant(self, consonant, amplitude=0.3):
        """
        Synthesize a consonant sound using filtered noise.
        
        Args:
            consonant: Consonant type ('s', 'sh', 'f', 't', 'k', 'ch')
            amplitude: Output amplitude
        
        Returns:
            Synthesized consonant audio
        """
        params = self.CONSONANTS.get(consonant.lower(), self.CONSONANTS['s'])
        
        duration = params['duration']
        num_samples = int(duration * self.sample_rate)
        
        # Generate noise
        noise = np.random.randn(num_samples)
        
        # Apply bandpass filter
        nyquist = self.sample_rate / 2
        low = max(0.01, (params['freq'] - params['bandwidth'] / 2) / nyquist)
        high = min(0.99, (params['freq'] + params['bandwidth'] / 2) / nyquist)
        
        if low < high:
            b, a = signal.butter(2, [low, high], btype='band')
            output = signal.filtfilt(b, a, noise)
        else:
            output = noise
        
        # Apply short envelope
        envelope = np.exp(-np.linspace(0, 10, num_samples))
        output *= envelope * amplitude
        
        return output
    
    def synthesize_word(self, phonemes, duration, base_pitch, pitch_contour=None):
        """
        Synthesize a word from a sequence of phonemes.
        
        Args:
            phonemes: List of phonemes (vowels and consonants)
            duration: Total duration in seconds
            base_pitch: Base MIDI note
            pitch_contour: Optional pitch contour (list of pitch offsets)
        
        Returns:
            Synthesized word audio
        """
        num_phonemes = len(phonemes)
        phoneme_duration = duration / num_phonemes
        
        output_segments = []
        
        for i, phoneme in enumerate(phonemes):
            # Calculate pitch for this phoneme
            if pitch_contour and i < len(pitch_contour):
                pitch = base_pitch + pitch_contour[i]
            else:
                pitch = base_pitch
            
            # Synthesize phoneme
            if phoneme.lower() in self.FORMANTS:
                segment = self.synthesize_vowel(phoneme, phoneme_duration, pitch)
            elif phoneme.lower() in self.CONSONANTS:
                segment = self.synthesize_consonant(phoneme)
                # Pad consonant to match vowel duration
                if len(segment) < int(phoneme_duration * self.sample_rate):
                    padding = np.zeros(int(phoneme_duration * self.sample_rate) - len(segment))
                    segment = np.concatenate([segment, padding])
            else:
                # Default to 'a' vowel
                segment = self.synthesize_vowel('a', phoneme_duration, pitch)
            
            output_segments.append(segment)
        
        # Concatenate all segments
        output = np.concatenate(output_segments)
        
        return output
    
    def text_to_phonemes(self, text):
        """
        Simple text-to-phoneme conversion (basic implementation).
        
        Args:
            text: Input text
        
        Returns:
            List of phonemes
        """
        # Simple vowel detection
        vowels = set('aeiouAEIOU')
        phonemes = []
        
        for char in text.lower():
            if char in vowels:
                phonemes.append(char)
            elif char in 'bcdfghjklmnpqrstvwxyz':
                # Map common consonants
                if char == 's':
                    phonemes.append('s')
                elif char == 'h':
                    phonemes.append('a')  # Silent or breathy
                elif char in 'tdkp':
                    phonemes.append(char if char in self.CONSONANTS else 't')
                elif char == 'f':
                    phonemes.append('f')
                elif char == 'r':
                    phonemes.append('a')  # Approximate
                elif char == 'l':
                    phonemes.append('e')  # Approximate
                elif char == 'n':
                    phonemes.append('a')  # Nasal approximation
                elif char == 'm':
                    phonemes.append('u')  # Approximation
        
        return phonemes if phonemes else ['a']


def apply_reverb(audio, decay=0.3, room_size=0.4):
    """Apply reverb effect to audio."""
    num_samples = len(audio)
    reverb_length = num_samples + int(SAMPLE_RATE * 2)
    reverb = np.zeros(reverb_length)
    reverb[:num_samples] = audio
    
    # Multiple delay taps for realistic reverb
    delays = [int(SAMPLE_RATE * d) for d in [0.023, 0.041, 0.067, 0.089, 0.113, 0.151]]
    gains = [0.6, 0.5, 0.4, 0.3, 0.25, 0.2]
    
    for delay, gain in zip(delays, gains):
        end_idx = min(delay + num_samples, reverb_length)
        samples_to_add = end_idx - delay
        if samples_to_add > 0:
            reverb[delay:end_idx] += audio[:samples_to_add] * gain * decay * room_size
    
    return reverb[:num_samples]


def apply_limiter(audio, ceiling=-0.1):
    """Apply brick-wall limiter."""
    ceiling_linear = 10 ** (ceiling / 20)
    return np.clip(audio, -ceiling_linear, ceiling_linear)


def normalize_audio(audio, target_db=-3):
    """Normalize audio to target dB level.
    
    Args:
        audio: Input audio array (not modified)
        target_db: Target level in dB
    
    Returns:
        New normalized audio array
    """
    target_linear = 10 ** (target_db / 20)
    max_amplitude = np.max(np.abs(audio))
    if max_amplitude > 0:
        normalized = audio / max_amplitude * target_linear
    else:
        normalized = audio.copy()
    return normalized


def save_wav(audio, filepath, sample_rate=SAMPLE_RATE, stereo=True):
    """Save audio to WAV file."""
    # Ensure output directory exists
    os.makedirs(os.path.dirname(filepath) if os.path.dirname(filepath) else '.', exist_ok=True)
    
    # Convert to stereo if needed
    if stereo and len(audio.shape) == 1:
        stereo_audio = np.zeros((len(audio), 2))
        stereo_audio[:, 0] = audio
        stereo_audio[:, 1] = audio
        audio = stereo_audio
    
    # Normalize
    audio = normalize_audio(audio)
    
    # Convert to 16-bit
    audio_int16 = (audio * 32767).astype(np.int16)
    
    # Save
    wavfile.write(filepath, sample_rate, audio_int16)
    
    return filepath


# ============================================================================
# Sample Definitions
# ============================================================================

SAMPLES = [
    {
        'name': 'sample_01_hello_world',
        'description': 'Basic "Hello World" vocal synthesis demo',
        'text': 'Hello World',
        'phonemes': ['e', 'o', 'u', 'o'],
        'duration': 2.0,
        'pitch': 60,  # C4
        'pitch_contour': [0, -2, 0, 2],  # Melodic contour
        'style': 'neutral',
        'reverb': 0.3,
    },
    {
        'name': 'sample_02_rise_up',
        'description': 'Emotional "Rise Up" with ascending pitch',
        'text': 'Rise Up',
        'phonemes': ['a', 'i', 'u'],
        'duration': 2.5,
        'pitch': 58,  # Bb3
        'pitch_contour': [0, 4, 7],  # Ascending contour
        'style': 'emotional',
        'reverb': 0.4,
    },
    {
        'name': 'sample_03_melodic_hook',
        'description': 'Trap-style melodic hook',
        'text': 'Yeah Yeah Yeah',
        'phonemes': ['e', 'a', 'e', 'a', 'e', 'a'],
        'duration': 3.0,
        'pitch': 65,  # F4
        'pitch_contour': [0, -2, 0, -2, 0, -5],
        'style': 'trap',
        'reverb': 0.35,
    },
    {
        'name': 'sample_04_vocal_chop',
        'description': 'Chopped vocal effect for EDM/Hip-Hop',
        'text': 'Oh Oh Oh',
        'phonemes': ['o', 'o', 'o', 'o', 'o', 'o'],
        'duration': 2.0,
        'pitch': 67,  # G4
        'pitch_contour': [0, 0, 3, 3, 7, 7],
        'style': 'chop',
        'reverb': 0.5,
    },
    {
        'name': 'sample_05_smooth_vocal',
        'description': 'Smooth R&B style vocal',
        'text': 'Ooh Aah',
        'phonemes': ['u', 'a', 'u', 'a'],
        'duration': 3.0,
        'pitch': 55,  # G3
        'pitch_contour': [0, 2, 5, 7],
        'style': 'rnb',
        'reverb': 0.45,
    },
]


def generate_sample(sample_config, synth, output_dir):
    """
    Generate a single text-to-vocal sample.
    
    Args:
        sample_config: Configuration dict for the sample
        synth: VocalSynthesizer instance
        output_dir: Output directory for WAV file
    
    Returns:
        Path to generated WAV file
    """
    name = sample_config['name']
    print(f"\n  Generating: {name}")
    print(f"    Text: \"{sample_config['text']}\"")
    print(f"    Style: {sample_config['style']}")
    
    # Synthesize the vocal
    audio = synth.synthesize_word(
        phonemes=sample_config['phonemes'],
        duration=sample_config['duration'],
        base_pitch=sample_config['pitch'],
        pitch_contour=sample_config.get('pitch_contour')
    )
    
    # Apply effects based on style
    style = sample_config['style']
    
    if style == 'trap':
        # Add slight distortion for trap style
        audio = np.tanh(audio * 1.5) * 0.8
    elif style == 'chop':
        # Add rhythmic chopping effect
        num_samples = len(audio)
        chop_period = num_samples // 6
        for i in range(0, num_samples, chop_period * 2):
            end = min(i + chop_period // 4, num_samples)
            audio[i:end] *= np.linspace(1, 0.3, end - i)
    elif style == 'rnb':
        # Smoother with more vibrato (already in synthesizer)
        pass
    elif style == 'emotional':
        # Add slight compression
        threshold = 0.3
        ratio = 3
        above = np.abs(audio) > threshold
        audio[above] = np.sign(audio[above]) * (threshold + (np.abs(audio[above]) - threshold) / ratio)
    
    # Apply reverb
    reverb_amount = sample_config.get('reverb', 0.3)
    audio = apply_reverb(audio, decay=reverb_amount, room_size=0.4)
    
    # Apply limiter
    audio = apply_limiter(audio)
    
    # Save WAV file
    output_path = os.path.join(output_dir, f"{name}.wav")
    save_wav(audio, output_path)
    
    # Print info
    file_size = os.path.getsize(output_path)
    print(f"    Duration: {sample_config['duration']:.1f}s")
    print(f"    Output: {output_path}")
    print(f"    Size: {file_size / 1024:.1f} KB")
    
    return output_path


def generate_manifest(samples_info, output_dir):
    """Generate a manifest JSON file with sample metadata."""
    manifest = {
        'title': 'MAEVN Text-to-Vocal Samples',
        'description': 'Functional transparency samples demonstrating the MAEVN Vocal Pipeline',
        'version': '1.0.0',
        'sample_rate': SAMPLE_RATE,
        'format': 'WAV 16-bit stereo',
        'samples': samples_info
    }
    
    manifest_path = os.path.join(output_dir, 'samples_manifest.json')
    with open(manifest_path, 'w', encoding='utf-8') as f:
        json.dump(manifest, f, indent=2)
    
    print(f"\nManifest saved: {manifest_path}")
    return manifest_path


def main():
    """Main function to generate all 5 text-to-vocal samples."""
    print("=" * 70)
    print("MAEVN Text-to-Vocal Sample Generator")
    print("=" * 70)
    print("Generating 5 samples for functional transparency demonstration")
    print("-" * 70)
    
    # Determine output directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    repo_dir = os.path.dirname(script_dir)
    output_dir = os.path.join(repo_dir, "output", "text_to_vocal_samples")
    
    # Create output directory
    os.makedirs(output_dir, exist_ok=True)
    print(f"\nOutput directory: {output_dir}")
    
    # Initialize synthesizer
    synth = VocalSynthesizer(SAMPLE_RATE)
    
    # Generate all samples
    samples_info = []
    generated_files = []
    
    print("\n" + "-" * 70)
    print("Generating samples...")
    
    for sample_config in SAMPLES:
        output_path = generate_sample(sample_config, synth, output_dir)
        generated_files.append(output_path)
        
        samples_info.append({
            'name': sample_config['name'],
            'description': sample_config['description'],
            'text': sample_config['text'],
            'duration': sample_config['duration'],
            'style': sample_config['style'],
            'file': os.path.basename(output_path)
        })
    
    # Generate manifest
    generate_manifest(samples_info, output_dir)
    
    # Print summary
    print("\n" + "=" * 70)
    print("TEXT-TO-VOCAL SAMPLE GENERATION COMPLETE")
    print("=" * 70)
    print(f"\nTotal samples generated: {len(generated_files)}")
    print(f"Output directory: {output_dir}")
    print("\nGenerated files:")
    
    total_size = 0
    for filepath in generated_files:
        size = os.path.getsize(filepath)
        total_size += size
        print(f"  [OK] {os.path.basename(filepath)} ({size / 1024:.1f} KB)")
    
    print(f"\nTotal size: {total_size / 1024:.1f} KB")
    print("-" * 70)
    print("\nSample descriptions:")
    for sample in SAMPLES:
        print(f"  • {sample['name']}: {sample['description']}")
    
    print("\n" + "=" * 70)
    print("MAEVN Vocal Pipeline - Functional Transparency Demonstration")
    print("=" * 70)
    
    return generated_files


if __name__ == "__main__":
    main()
