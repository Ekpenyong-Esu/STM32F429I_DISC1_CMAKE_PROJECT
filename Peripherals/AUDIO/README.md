# Audio Driver for STM32F429

This directory contains a comprehensive audio driver implementation for the STM32F429 microcontroller, supporting both SAI (Serial Audio Interface) and I2S audio protocols.

## Features

- **Multiple Audio Interfaces**: Support for SAI and I2S interfaces
- **Flexible Configuration**: Configurable sample rates, bit depths, and channel configurations
- **DMA Support**: Efficient DMA-based audio data transfer
- **Codec Integration**: Built-in support for audio codecs (WM8994 example)
- **Buffer Management**: Circular buffer implementation for continuous audio streaming
- **Volume Control**: Digital volume control with mute functionality
- **Audio Processing**: Basic audio processing functions (mixing, filtering)
- **Statistics**: Performance monitoring and error tracking
- **Comprehensive Examples**: Multiple example functions demonstrating usage

## Files

- `audio.h` - Main header file with function prototypes and type definitions
- `audio.c` - Core audio driver implementation
- `audio_example.c` - Example functions demonstrating audio functionality

## Supported Audio Formats

- **Sample Rates**: 8kHz, 11.025kHz, 16kHz, 22.05kHz, 32kHz, 44.1kHz, 48kHz, 96kHz
- **Bit Depths**: 16-bit, 24-bit, 32-bit
- **Channels**: Mono and Stereo
- **Interfaces**: SAI (Serial Audio Interface), I2S (Inter-IC Sound)

## Hardware Requirements

### SAI Interface (Recommended)
- SAI1 peripheral on STM32F429
- Audio codec (e.g., WM8994, CS42L51)
- I2C interface for codec control
- External audio PLL configuration

### I2S Interface (Alternative)
- SPI3 peripheral configured for I2S
- Compatible I2S audio codec
- Master clock (MCK) output support

## Pin Configuration

### SAI Interface
```
SAI1_MCK  -> PE2   (Audio Master Clock)
SAI1_SD   -> PE4   (Audio Data)
SAI1_FS   -> PE5   (Audio Frame Sync)
SAI1_SCK  -> PE6   (Audio Serial Clock)
```

### I2S Interface
```
I2S3_WS   -> PC0   (Word Select)
I2S3_CK   -> PC10  (Serial Clock)
I2S3_SD   -> PC12  (Serial Data)
```

## Usage Examples

### Basic Initialization
```c
#include "audio.h"

void main(void) {
    // Initialize with default settings (SAI, 44.1kHz, 16-bit, Stereo)
    AUDIO_Init();

    // Or initialize with custom configuration
    AUDIO_ConfigTypeDef config = {
        .Interface = AUDIO_INTERFACE_SAI,
        .SampleRate = AUDIO_FREQ_48K,
        .BitDepth = AUDIO_FORMAT_16BIT,
        .Channels = AUDIO_CHANNEL_STEREO,
        .BufferSize = 4096,
        .EnableDMA = true,
        .EnableInterrupts = true
    };
    AUDIO_Init_Custom(&config);
}
```

### Audio Playback
```c
// Generate or load audio data
uint8_t audioData[4096];

// Write data to audio buffer
AUDIO_WriteBuffer(audioData, sizeof(audioData));

// Start playback
AUDIO_Play();

// Playback runs in background via DMA
// ... do other tasks ...

// Stop playback when done
AUDIO_Stop();
```

### Audio Recording
```c
uint8_t recordBuffer[4096];
uint32_t bytesRead;

// Start recording
AUDIO_Record();

// Wait for recording to complete
HAL_Delay(1000);

// Stop recording
AUDIO_Stop();

// Read recorded data
AUDIO_ReadBuffer(recordBuffer, sizeof(recordBuffer), &bytesRead);
```

### Volume Control
```c
// Set volume to 50%
AUDIO_SetVolume(50);

// Mute audio
AUDIO_SetMute(true);

// Unmute audio
AUDIO_SetMute(false);
```

### Audio Processing
```c
// Mix two audio streams
AUDIO_MixStreams(stream1, stream2, output, sampleCount);

// Apply gain
AUDIO_SetGain(1.5f);  // 1.5x amplification
```

## Example Functions

The `audio_example.c` file contains several example functions:

- `AUDIO_Example_Init()` - Basic initialization
- `AUDIO_Example_Playback()` - Sine wave generation and playback
- `AUDIO_Example_Recording()` - Audio recording demonstration
- `AUDIO_Example_VolumeControl()` - Volume control demonstration
- `AUDIO_Example_AudioMixing()` - Audio stream mixing
- `AUDIO_Example_Statistics()` - Performance statistics
- `AUDIO_Example_RunAll()` - Complete demonstration

## Integration with Main Application

Add the following to your main.c:

```c
#include "audio.h"
#include "audio_example.h"

// In main function
int main(void) {
    // Initialize HAL and system clocks
    HAL_Init();
    SystemClock_Config();

    // Run audio examples
    AUDIO_Example_RunAll();

    while (1) {
        // Main application loop
    }
}
```

## Configuration Options

### Audio PLL Settings
The driver configures the audio PLL for proper clock generation:
- PLL_M: 8
- PLL_N: 344
- PLL_P: 7
- PLL_Q: 7

### Buffer Sizes
- Default buffer size: 4096 bytes
- Minimum buffer size: 256 bytes
- Maximum buffer size: 16384 bytes

### DMA Configuration
- DMA2 Stream 1 Channel 0 for SAI
- Circular mode for continuous streaming
- High priority for audio data

## Error Handling

The driver provides comprehensive error handling:

- `AUDIO_OK` - Operation successful
- `AUDIO_ERROR` - General error
- `AUDIO_BUSY` - Peripheral busy
- `AUDIO_TIMEOUT` - Operation timeout
- `AUDIO_INVALID_PARAM` - Invalid parameter
- `AUDIO_NOT_READY` - System not ready
- `AUDIO_OVERFLOW` - Buffer overflow
- `AUDIO_UNDERFLOW` - Buffer underflow

## Performance Considerations

- Use DMA for continuous audio streaming to minimize CPU usage
- Buffer sizes should be multiples of audio frame sizes
- Higher sample rates require more processing power
- Monitor CPU usage statistics for optimization

## Codec Integration

The driver includes placeholder functions for codec control:
- `AUDIO_CodecInit()` - Initialize audio codec
- `AUDIO_Codec_WriteRegister()` - Write codec register
- `AUDIO_Codec_ReadRegister()` - Read codec register

These functions need to be implemented based on your specific audio codec.

## Troubleshooting

### Common Issues
1. **No Audio Output**: Check codec power and I2C communication
2. **Distorted Audio**: Verify sample rate and bit depth settings
3. **Buffer Overflows**: Increase buffer size or reduce sample rate
4. **DMA Errors**: Check DMA channel configuration and priorities

### Debug Information
Use `AUDIO_GetStatistics()` to monitor:
- Sample count
- Buffer overflows/underflows
- Synchronization errors
- CPU usage

## Dependencies

- STM32F4xx HAL library
- CMSIS core
- Math library (for audio processing functions)

## Future Enhancements

- Support for additional audio codecs
- USB audio class support
- Advanced audio processing (equalizer, reverb)
- Multi-channel audio support
- Bluetooth audio integration

## License

This audio driver is provided as-is for educational and development purposes.
