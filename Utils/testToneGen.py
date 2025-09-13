import numpy as np
import wave
import struct

# Params
sample_rate = 48000  # Hz
duration_tone = 1.0  # s
duration_silence = 0.0  # s

freq_1 = 200.0  # Hz (A4)
freq_2 = 1000.0  # Hz (E5)
freq_3 = 8000.0  # Hz (A5)

ampStart = 0.1  # starts amplitude (0..1)
ampStop = 0.8   # end amplitude (0..1)

# Sinus generate
def generate_tone(freq, duration, amp_start, amp_stop, sample_rate):
    t = np.linspace(0, duration, int(sample_rate * duration), endpoint=False)
    amplitude = np.linspace(amp_start, amp_stop, len(t))
    tone = amplitude * np.sin(2 * np.pi * freq * t)
    return tone

# Silence generate
def generate_silence(duration, sample_rate):
    return np.zeros(int(sample_rate * duration))

# Generate three tones with silence
tone1 = generate_tone(freq_1, duration_tone, ampStart, ampStop, sample_rate)
silence = generate_silence(duration_silence, sample_rate)
tone2 = generate_tone(freq_2, duration_tone, ampStart, ampStop, sample_rate)
tone3 = generate_tone(freq_3, duration_tone, ampStart, ampStop, sample_rate)

# Glue together
signal = np.concatenate([tone1, silence, tone2, silence, tone3])

# Normalisation to 16-bit PCM
signal_int16 = np.int16(signal / np.max(np.abs(signal)) * 32767)

# Save a file
with wave.open("test_tones.wav", "w") as wf:
    wf.setnchannels(1)  # mono
    wf.setsampwidth(2)  # 16-bit
    wf.setframerate(sample_rate)
    wf.writeframes(signal_int16.tobytes())

print("File 'test_tones.wav' was generated.")
