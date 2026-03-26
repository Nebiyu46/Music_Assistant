import numpy as np
import scipy.io.wavfile as wav

# 1. Configuration
sample_rate = 44100  # Standard audio quality (Hz)
duration = 30        # Duration in seconds

# 2. Generate the White Noise
# 'normal' creates Gaussian White Noise (statistically "purest")
# Mean = 0, Standard Deviation = 1
noise = np.random.normal(0, 1, sample_rate * duration)

# 3. Normalization (Critical!)
# Audio signals must stay between -1.0 and 1.0 to avoid clipping/distortion.
# We divide by the maximum absolute value to scale it perfectly.
noise = noise / np.max(np.abs(noise))

# 4. Save to WAV file
# We convert to float32 for high quality
wav.write('pure_white_noise.wav', sample_rate, noise.astype(np.float32))

print("Done! Saved as 'pure_white_noise.wav'")