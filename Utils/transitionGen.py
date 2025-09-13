import numpy as np
import wave
import sys

# test_tone params
sample_rate = 48000
tone_duration = 1.0   # s
silence_duration = 0.0
num_tones = 3

ampSteps = 100  # Numbers of measures points

if len(sys.argv) < 4:
    print("Use: python analyze_transfer.py test_tone.wav processed.wav output.h")
    sys.exit(1)

file_ref = sys.argv[1]      # original test_tone.wav
file_proc = sys.argv[2]     # processed file
file_out = sys.argv[3]      # output .h


def read_wav(filename):
    with wave.open(filename, "rb") as wf:
        n_channels = wf.getnchannels()
        n_frames = wf.getnframes()
        raw = wf.readframes(n_frames)

    data = np.frombuffer(raw, dtype=np.int16).astype(np.float32)
    if n_channels > 1:
        data = data[::n_channels]  # only first channel
    return data


# load both files
ref_data = read_wav(file_ref)
proc_data = read_wav(file_proc)

# length of test tone for one freq
tone_samples = int(tone_duration * sample_rate)
silence_samples = int(silence_duration * sample_rate)

tran = np.zeros((num_tones, ampSteps), dtype=np.float32)

for tone_idx in range(num_tones):
    start = tone_idx * (tone_samples + silence_samples)
    end = start + tone_samples

    ref_segment = ref_data[start:end]
    proc_segment = proc_data[start:end]

    step_len = tone_samples // ampSteps

    for j in range(ampSteps):
        seg_start = j * step_len
        seg_end = seg_start + step_len

        ref_val = np.mean(np.abs(ref_segment[seg_start:seg_end]))
        proc_val = np.mean(np.abs(proc_segment[seg_start:seg_end]))

        if ref_val == 0:
            ratio = 0.0
        else:
            ratio = proc_val / ref_val

        tran[tone_idx, j] = ratio


# save to .h file
with open(file_out, "w") as f:
    f.write(f"#ifndef TRANSFER_FUNCTION_H\n")
    f.write(f"#define TRANSFER_FUNCTION_H\n\n")
    f.write(f"#define AMP_STEPS {ampSteps}\n\n")
    f.write("float tran[3][AMP_STEPS] = {\n")

    for i in range(num_tones):
        f.write("    {\n")
        for j in range(ampSteps):
            f.write(f"        {tran[i, j]:.6f},\n")
        f.write("    }")
        if i < num_tones - 1:
            f.write(",\n")
        else:
            f.write("\n")

    f.write("};\n\n#endif // TRANSFER_FUNCTION_H\n")

print(f"Save to {file_out}")
