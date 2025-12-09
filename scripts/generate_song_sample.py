#!/usr/bin/env python3
"""
MAEVN Song Generation Sample Script
====================================
Generates a 59-second music sample demonstrating the MAEVN vocal pipeline
with all instruments: vocals, 808 bass, hi-hats, snares, piano, and synth.

This script produces a real, playable audio file with:
- Synthesized vocals using formant synthesis
- 808 sub-bass with glides
- Trap-style hi-hat patterns
- Snare/clap drums
- Piano chords
- Synth pads

Output: A 59-second (00:59) stereo WAV file at 44.1kHz, 16-bit
"""

import numpy as np
from scipy import signal
from scipy.io import wavfile
import os
import sys
import io

# Configure UTF-8 encoding for stdout to prevent UnicodeEncodeError
if sys.stdout.encoding != 'utf-8':
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

# Audio Constants
SAMPLE_RATE = 44100
DURATION = 59  # seconds (00:59 duration as requested)
BPM = 140
BEAT_DURATION = 60.0 / BPM
SAMPLES_PER_BEAT = int(SAMPLE_RATE * BEAT_DURATION)
NUM_SAMPLES = int(SAMPLE_RATE * DURATION)

# Musical Constants (C Minor scale for trap vibe)
ROOT_NOTE = 36  # C2 for bass
SCALE_NOTES = [0, 3, 5, 7, 10, 12]  # Minor pentatonic scale offsets

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
    sustain_samples = num_samples - attack_samples - decay_samples - release_samples
    
    if sustain_samples < 0:
        sustain_samples = 0
        release_samples = num_samples - attack_samples - decay_samples
    
    # Attack
    if attack_samples > 0:
        envelope[:attack_samples] = np.linspace(0, 1, attack_samples)
    
    # Decay
    start = attack_samples
    end = start + decay_samples
    if end > start:
        envelope[start:min(end, num_samples)] = np.linspace(1, sustain, min(decay_samples, num_samples - start))
    
    # Sustain
    start = attack_samples + decay_samples
    end = start + sustain_samples
    if end > start:
        envelope[start:min(end, num_samples)] = sustain
    
    # Release
    start = num_samples - release_samples
    if start >= 0 and start < num_samples:
        envelope[start:] = np.linspace(sustain, 0, len(envelope[start:]))
    
    return envelope

def generate_808_bass(start_sample, duration_beats, root_note, glide_to=None):
    """Generate 808 sub-bass with optional glide."""
    duration = duration_beats * BEAT_DURATION
    num_samples = int(duration * SAMPLE_RATE)
    t = np.linspace(0, duration, num_samples)
    
    # Base frequency with glide
    freq_start = midi_to_freq(root_note)
    if glide_to is not None:
        freq_end = midi_to_freq(glide_to)
        freq = np.linspace(freq_start, freq_end, num_samples)
    else:
        freq = np.full(num_samples, freq_start)
    
    # Generate sub-bass with harmonics
    phase = np.cumsum(2 * np.pi * freq / SAMPLE_RATE)
    wave = np.sin(phase)
    
    # Add some harmonics for punch
    wave += 0.3 * np.sin(2 * phase)
    wave += 0.1 * np.sin(3 * phase)
    
    # Apply saturation for warmth
    wave = np.tanh(wave * 2.0) * 0.8
    
    # Apply envelope
    envelope = generate_envelope(0.005, 0.1, 0.6, duration * 0.5, duration)
    wave *= envelope
    
    return start_sample, wave * 0.5  # Reduce level for mixing

def generate_hihat(start_sample, closed=True, velocity=0.8):
    """Generate hi-hat using filtered noise."""
    if closed:
        duration = 0.05
        cutoff = 8000
    else:
        duration = 0.2
        cutoff = 6000
    
    num_samples = int(duration * SAMPLE_RATE)
    
    # White noise
    noise = np.random.randn(num_samples)
    
    # High-pass filter
    nyquist = SAMPLE_RATE / 2
    high = cutoff / nyquist
    b, a = signal.butter(2, high, btype='high')
    filtered = signal.filtfilt(b, a, noise)
    
    # Envelope
    envelope = np.exp(-np.linspace(0, 8 if closed else 4, num_samples))
    
    wave = filtered * envelope * velocity * 0.15
    
    return start_sample, wave

def generate_snare(start_sample, velocity=0.9):
    """Generate snare/clap with body and noise."""
    duration = 0.25
    num_samples = int(duration * SAMPLE_RATE)
    t = np.linspace(0, duration, num_samples)
    
    # Snare body (tonal component)
    freq = 180
    body = np.sin(2 * np.pi * freq * t)
    body_envelope = np.exp(-t * 30)
    body *= body_envelope
    
    # Noise component
    noise = np.random.randn(num_samples)
    b, a = signal.butter(2, [0.05, 0.4], btype='band')
    noise = signal.filtfilt(b, a, noise)
    noise_envelope = np.exp(-t * 15)
    noise *= noise_envelope
    
    wave = (body * 0.4 + noise * 0.6) * velocity * 0.35
    
    return start_sample, wave

def generate_piano_chord(start_sample, midi_notes, duration_beats):
    """Generate piano chord using additive synthesis."""
    duration = duration_beats * BEAT_DURATION
    num_samples = int(duration * SAMPLE_RATE)
    t = np.linspace(0, duration, num_samples)
    
    wave = np.zeros(num_samples)
    
    for note in midi_notes:
        freq = midi_to_freq(note)
        
        # Rich harmonic content
        note_wave = np.sin(2 * np.pi * freq * t)
        note_wave += 0.5 * np.sin(2 * np.pi * freq * 2 * t)
        note_wave += 0.25 * np.sin(2 * np.pi * freq * 3 * t)
        note_wave += 0.125 * np.sin(2 * np.pi * freq * 4 * t)
        
        wave += note_wave
    
    # Normalize
    wave /= len(midi_notes)
    
    # Apply envelope
    envelope = generate_envelope(0.01, 0.1, 0.4, 0.5, duration)
    wave *= envelope
    
    return start_sample, wave * 0.2

def generate_synth_pad(start_sample, midi_note, duration_beats):
    """Generate FM synth pad."""
    duration = duration_beats * BEAT_DURATION
    num_samples = int(duration * SAMPLE_RATE)
    t = np.linspace(0, duration, num_samples)
    
    carrier_freq = midi_to_freq(midi_note)
    modulator_freq = carrier_freq * 2.0
    
    # FM synthesis with modulation index
    mod_index = 2.0
    modulator = np.sin(2 * np.pi * modulator_freq * t) * mod_index
    wave = np.sin(2 * np.pi * carrier_freq * t + modulator)
    
    # Add detuned layer
    detune = 0.02
    wave2 = np.sin(2 * np.pi * carrier_freq * (1 + detune) * t + modulator)
    wave3 = np.sin(2 * np.pi * carrier_freq * (1 - detune) * t + modulator)
    wave = (wave + wave2 + wave3) / 3
    
    # Apply slow LFO for movement
    lfo = 0.5 + 0.5 * np.sin(2 * np.pi * 0.5 * t)
    wave *= lfo
    
    # Apply envelope
    envelope = generate_envelope(0.3, 0.2, 0.7, 0.8, duration)
    wave *= envelope
    
    return start_sample, wave * 0.12

def generate_vocal_formant(start_sample, text, duration_beats, pitch_midi):
    """Generate synthesized vocal using formant synthesis."""
    duration = duration_beats * BEAT_DURATION
    num_samples = int(duration * SAMPLE_RATE)
    t = np.linspace(0, duration, num_samples)
    
    # Formant frequencies for different vowels (simplified)
    formants = {
        'a': [730, 1090, 2440],
        'e': [530, 1840, 2480],
        'i': [270, 2290, 3010],
        'o': [570, 840, 2410],
        'u': [440, 1020, 2240],
    }
    
    # Generate glottal pulse train (carrier)
    f0 = midi_to_freq(pitch_midi)
    phase = 2 * np.pi * f0 * t
    
    # Glottal source with harmonics
    source = np.zeros(num_samples)
    for h in range(1, 15):
        amp = 1.0 / (h ** 0.8)
        source += amp * np.sin(h * phase)
    
    # Apply formant filtering
    wave = np.zeros(num_samples)
    vowel_sequence = ['o', 'a', 'i', 'e', 'u']
    
    for i, vowel in enumerate(vowel_sequence):
        if vowel in formants:
            start_idx = i * num_samples // len(vowel_sequence)
            end_idx = (i + 1) * num_samples // len(vowel_sequence)
            segment = source[start_idx:end_idx].copy()
            
            # Apply formant filters
            for formant_freq in formants[vowel]:
                nyquist = SAMPLE_RATE / 2
                low = max(0.01, (formant_freq - 100) / nyquist)
                high = min(0.99, (formant_freq + 100) / nyquist)
                if low < high:
                    b, a = signal.butter(2, [low, high], btype='band')
                    filtered = signal.filtfilt(b, a, segment)
                    wave[start_idx:end_idx] += filtered * 0.3
    
    # Add some noise for consonants
    noise = np.random.randn(num_samples) * 0.02
    noise_env = np.zeros(num_samples)
    for i in range(0, num_samples, num_samples // 5):
        end = min(i + 1000, num_samples)
        noise_env[i:end] = np.linspace(1, 0, end - i)
    wave += noise * noise_env
    
    # Apply envelope
    envelope = generate_envelope(0.05, 0.1, 0.6, 0.3, duration)
    wave *= envelope
    
    return start_sample, wave * 0.35

def apply_reverb(audio, decay=0.3, room_size=0.4):
    """Apply simple reverb effect."""
    num_samples = len(audio)
    reverb_length = num_samples + int(SAMPLE_RATE * 1.5)
    reverb = np.zeros(reverb_length)
    reverb[:num_samples] = audio
    
    # Multiple delay taps
    delays = [int(SAMPLE_RATE * d) for d in [0.023, 0.041, 0.067, 0.089, 0.113]]
    gains = [0.6, 0.5, 0.4, 0.3, 0.2]
    
    for delay, gain in zip(delays, gains):
        # Ensure we don't exceed buffer bounds
        end_idx = min(delay + num_samples, reverb_length)
        samples_to_add = end_idx - delay
        if samples_to_add > 0:
            reverb[delay:end_idx] += audio[:samples_to_add] * gain * decay * room_size
    
    return reverb[:num_samples]

def apply_compression(audio, threshold=-12, ratio=4):
    """Apply simple compression."""
    threshold_linear = 10 ** (threshold / 20)
    
    compressed = audio.copy()
    above_threshold = np.abs(audio) > threshold_linear
    
    if np.any(above_threshold):
        sign = np.sign(audio[above_threshold])
        compressed[above_threshold] = sign * (
            threshold_linear + (np.abs(audio[above_threshold]) - threshold_linear) / ratio
        )
    
    return compressed

def apply_limiter(audio, ceiling=-0.1):
    """Apply brick-wall limiter."""
    ceiling_linear = 10 ** (ceiling / 20)
    return np.clip(audio, -ceiling_linear, ceiling_linear)

def mix_to_stereo(audio_events, pan_positions):
    """Mix mono audio events to stereo with panning."""
    stereo = np.zeros((2, NUM_SAMPLES))
    
    for (start, wave), pan in zip(audio_events, pan_positions):
        # Handle negative start positions
        wave_start = 0
        if start < 0:
            wave_start = -start
            start = 0
        
        # Skip if start is beyond the buffer
        if start >= NUM_SAMPLES:
            continue
        
        end = min(start + len(wave) - wave_start, NUM_SAMPLES)
        actual_len = end - start
        
        # Skip if nothing to add
        if actual_len <= 0:
            continue
        
        # Calculate panning gains
        left_gain = np.sqrt(0.5 * (1 - pan))
        right_gain = np.sqrt(0.5 * (1 + pan))
        
        stereo[0, start:end] += wave[wave_start:wave_start+actual_len] * left_gain
        stereo[1, start:end] += wave[wave_start:wave_start+actual_len] * right_gain
    
    return stereo

def generate_song():
    """Generate a complete 60-second song with all instruments."""
    print("=" * 60)
    print("MAEVN Song Generation")
    print("=" * 60)
    print(f"Sample Rate: {SAMPLE_RATE} Hz")
    print(f"Duration: {DURATION} seconds")
    print(f"BPM: {BPM}")
    print("-" * 60)
    
    audio_events = []
    pan_positions = []
    
    # Calculate total beats
    total_beats = int(DURATION * BPM / 60)
    
    # Song structure (in beats)
    INTRO_END = 16
    VERSE1_END = 48
    HOOK1_END = 64
    VERSE2_END = 96
    HOOK2_END = 112
    BRIDGE_END = 128
    OUTRO_START = 128
    
    print("\nGenerating song sections...")
    
    # ========================================
    # DRUMS: Hi-Hats
    # ========================================
    print("  - Generating hi-hats...")
    hihat_pattern = [1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0]  # 16th notes
    for beat in range(INTRO_END // 2, total_beats):  # Start after intro
        for i, hit in enumerate(hihat_pattern):
            if hit:
                sample = int((beat + i * 0.25) * SAMPLES_PER_BEAT)
                if sample < NUM_SAMPLES:
                    velocity = 0.6 + 0.3 * (i % 4 == 0)  # Accent on beats
                    event = generate_hihat(sample, closed=True, velocity=velocity)
                    audio_events.append(event)
                    pan_positions.append(-0.2)  # Slight left pan
        
        # Add open hi-hats on off-beats
        if beat % 4 == 2:  # Every other beat
            sample = int(beat * SAMPLES_PER_BEAT)
            if sample < NUM_SAMPLES:
                event = generate_hihat(sample, closed=False, velocity=0.5)
                audio_events.append(event)
                pan_positions.append(0.3)  # Slight right pan
    
    # ========================================
    # DRUMS: Snares
    # ========================================
    print("  - Generating snares...")
    for beat in range(INTRO_END, total_beats):
        if beat % 4 in [1, 3]:  # Snare on 2 and 4
            sample = int(beat * SAMPLES_PER_BEAT)
            if sample < NUM_SAMPLES:
                event = generate_snare(sample, velocity=0.85)
                audio_events.append(event)
                pan_positions.append(0.0)  # Center
    
    # ========================================
    # BASS: 808
    # ========================================
    print("  - Generating 808 bass...")
    bass_pattern = [
        (0, 2, ROOT_NOTE, None),
        (3, 1, ROOT_NOTE + 3, ROOT_NOTE),  # Glide
        (6, 2, ROOT_NOTE + 5, None),
        (10, 1, ROOT_NOTE + 7, ROOT_NOTE + 5),  # Glide
        (12, 4, ROOT_NOTE, None),
    ]
    
    for beat in range(INTRO_END // 2, total_beats, 16):  # Repeat pattern every 16 beats
        for offset, dur, note, glide in bass_pattern:
            actual_beat = beat + offset
            if actual_beat < total_beats:
                sample = int(actual_beat * SAMPLES_PER_BEAT)
                event = generate_808_bass(sample, dur, note, glide)
                audio_events.append(event)
                pan_positions.append(0.0)  # Center for bass
    
    # ========================================
    # PIANO: Chords
    # ========================================
    print("  - Generating piano chords...")
    # C minor chord voicings
    chord_progression = [
        ([60, 63, 67], 4),  # Cm
        ([58, 62, 65], 4),  # Bb
        ([56, 60, 63], 4),  # Ab
        ([55, 58, 63], 4),  # G
    ]
    
    for beat in range(INTRO_END, total_beats, 16):
        for i, (chord, duration) in enumerate(chord_progression):
            actual_beat = beat + i * 4
            if actual_beat < total_beats:
                sample = int(actual_beat * SAMPLES_PER_BEAT)
                event = generate_piano_chord(sample, chord, duration)
                audio_events.append(event)
                pan_positions.append(-0.3)  # Slight left
    
    # ========================================
    # SYNTH: Pads
    # ========================================
    print("  - Generating synth pads...")
    for beat in range(HOOK1_END - 16, total_beats, 32):
        for i, offset in enumerate([0, 8, 16, 24]):
            actual_beat = beat + offset
            if actual_beat < total_beats:
                sample = int(actual_beat * SAMPLES_PER_BEAT)
                note = 48 + SCALE_NOTES[i % len(SCALE_NOTES)]  # C3 + scale
                event = generate_synth_pad(sample, note, 8)
                audio_events.append(event)
                pan_positions.append(0.4)  # Right pan
    
    # ========================================
    # VOCALS: Synthesized
    # ========================================
    print("  - Generating vocals...")
    
    # Intro vocal (soft entry)
    event = generate_vocal_formant(int(8 * SAMPLES_PER_BEAT), "hey", 4, 60)
    audio_events.append(event)
    pan_positions.append(0.0)
    
    # Hook vocal phrases
    hook_lyrics = [
        ("we", 2, 60),
        ("rise", 4, 63),
        ("up", 2, 65),
        ("high", 4, 67),
    ]
    
    for hook_start in [VERSE1_END, VERSE2_END, BRIDGE_END]:
        for i, (text, dur, pitch) in enumerate(hook_lyrics):
            beat = hook_start + i * dur
            if beat < total_beats:
                sample = int(beat * SAMPLES_PER_BEAT)
                event = generate_vocal_formant(sample, text, dur, pitch)
                audio_events.append(event)
                pan_positions.append(0.0)
    
    # Verse vocals
    verse_phrases = [
        ("check", 2, 58),
        ("it", 1, 60),
        ("out", 2, 63),
        ("now", 3, 60),
    ]
    
    for verse_start in [INTRO_END, HOOK1_END]:
        for i, (text, dur, pitch) in enumerate(verse_phrases):
            beat = verse_start + 4 + i * 4
            if beat < total_beats:
                sample = int(beat * SAMPLES_PER_BEAT)
                event = generate_vocal_formant(sample, text, dur, pitch)
                audio_events.append(event)
                pan_positions.append(0.0)
    
    # ========================================
    # MIXING
    # ========================================
    print("\nMixing audio tracks...")
    
    # Mix to stereo
    stereo_audio = mix_to_stereo(audio_events, pan_positions)
    
    # Apply effects
    print("Applying effects...")
    
    # Apply reverb to both channels
    for ch in range(2):
        stereo_audio[ch] = apply_reverb(stereo_audio[ch], decay=0.25, room_size=0.35)
    
    # Apply compression
    for ch in range(2):
        stereo_audio[ch] = apply_compression(stereo_audio[ch], threshold=-15, ratio=3)
    
    # Apply limiter
    for ch in range(2):
        stereo_audio[ch] = apply_limiter(stereo_audio[ch], ceiling=-0.1)
    
    # Normalize
    max_amplitude = np.max(np.abs(stereo_audio))
    if max_amplitude > 0:
        stereo_audio /= max_amplitude
        stereo_audio *= 0.9  # Leave some headroom
    
    # Convert to 16-bit integer
    stereo_int16 = (stereo_audio * 32767).astype(np.int16)
    
    # Interleave channels for WAV file
    output = np.zeros((NUM_SAMPLES, 2), dtype=np.int16)
    output[:, 0] = stereo_int16[0]
    output[:, 1] = stereo_int16[1]
    
    return output

def main():
    """Main function to generate and save the song."""
    # Generate the song
    audio_data = generate_song()
    
    # Determine output path
    script_dir = os.path.dirname(os.path.abspath(__file__))
    repo_dir = os.path.dirname(script_dir)
    output_dir = os.path.join(repo_dir, "output")
    
    # Create output directory if it doesn't exist
    os.makedirs(output_dir, exist_ok=True)
    
    output_file = os.path.join(output_dir, "maevn_song_sample.wav")
    
    # Save the audio file
    print(f"\nSaving audio to: {output_file}")
    wavfile.write(output_file, SAMPLE_RATE, audio_data)
    
    # Print statistics
    file_size = os.path.getsize(output_file)
    print("\n" + "=" * 60)
    print("SONG GENERATION COMPLETE")
    print("=" * 60)
    print(f"Output File: {output_file}")
    print(f"File Size: {file_size / (1024 * 1024):.2f} MB")
    print(f"Duration: {DURATION} seconds")
    print(f"Format: 16-bit stereo WAV @ {SAMPLE_RATE} Hz")
    print(f"BPM: {BPM}")
    print("-" * 60)
    print("Instruments used:")
    print("  [OK] Synthesized Vocals (formant synthesis)")
    print("  [OK] 808 Sub-Bass (with glides)")
    print("  [OK] Hi-Hats (closed and open)")
    print("  [OK] Snare/Clap")
    print("  [OK] Piano Chords")
    print("  [OK] FM Synth Pads")
    print("-" * 60)
    print("Effects applied:")
    print("  [OK] Reverb (room simulation)")
    print("  [OK] Compression (dynamics control)")
    print("  [OK] Limiter (peak control)")
    print("  [OK] Stereo panning")
    print("=" * 60)
    
    return output_file

if __name__ == "__main__":
    main()
