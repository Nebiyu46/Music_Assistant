# mic_3

STM32F411 microphone FFT project — samples audio via ADC, computes a 4k-point RFFT
using CMSIS-DSP, finds the dominant frequency, maps it to a MIDI note and reports
results over USB CDC. The project was generated with STM32CubeMX and uses CMake.

## Quick Start

- Requirements: ARM GCC toolchain (e.g. `gcc-arm-none-eabi`), `cmake`, `ninja` or `make`,
	and the ST-Link tools or other programmer for flashing.

1. Create a build directory and configure:

```powershell
cd C:\Users\ADMIN\Documents\Music_assistant\mic_3
mkdir build
cd build
cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=..\cmake\gcc-arm-none-eabi.cmake ..
ninja
```

2. Flash using your programmer (example with `st-flash` / `openocd` / ST-Link GUI):

```powershell
# Example (adjust path and file name produced by your build):
st-flash write Debug\mic_3.bin 0x08000000
```

## USB Usage

- The device exposes a USB CDC (virtual COM) port. The firmware prints the detected
	dominant frequency and the mapped MIDI note over USB.
- You can also send a simple song text format over USB to load `SongNote_t` entries:

```
START
0,60,500
500,62,500
END
```

`START` clears the buffer, lines of `start_ms,midi_note,duration_ms` are parsed,
and `END` triggers parsing on the device.

## Notes

- Sampling setup: `ADC1` triggered by `TIM2` TRGO, DMA fills a 4096-sample buffer.
- FFT length: 4096, code assumes a 12 kHz sampling rate when converting bins → Hz.
- DSP: uses `arm_rfft_fast_f32`, `arm_cmplx_mag_f32`, and `arm_max_f32` from CMSIS-DSP.

## Ignored files

/build/
/Debug/
/CMakeFiles/
/CMakeCache.txt
/compile_commands.json
*.o
*.elf
*.bin
.vscode/
*.swp



