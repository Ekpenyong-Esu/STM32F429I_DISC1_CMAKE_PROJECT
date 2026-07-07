# ADC Peripheral Driver for STM32F429I Discovery Board

## Overview

This ADC driver provides a clean, easy-to-use interface for analog-to-digital conversion on the STM32F429I Discovery board. It wraps the STM32 HAL library functions with a simplified API that's perfect for learning embedded programming.

## Features

- **HAL Library Integration**: Built on top of STM32 HAL for reliability
- **Simple API**: Easy-to-understand function names and parameters
- **Multiple Channels**: Support for all ADC channels (PA0-PA7, PB0-PB1, PC0-PC5)
- **Flexible Configuration**: Configurable resolution, sampling time, and conversion modes
- **Voltage Conversion**: Built-in functions to convert raw ADC values to voltages
- **Error Handling**: Comprehensive error checking and status reporting
- **Examples**: Ready-to-use example code for common ADC operations

## Quick Start

### Basic ADC Reading

```c
#include "adc.h"

// 1. Configure the ADC
ADC_ConfigTypeDef adc_config = {
    .channel = ADC_CHANNEL_0_PA0,        // Read from PA0 pin
    .resolution = ADC_RESOLUTION_12BIT,  // 12-bit resolution (0-4095)
    .sampling_time = ADC_SAMPLING_84_CYCLES, // 84 ADC clock cycles
    .conv_mode = ADC_MODE_SINGLE,        // Single conversion mode
    .dma_enabled = false                 // No DMA for now
};

// 2. Initialize the ADC
ADC_StatusTypeDef status = ADC_Init(&hadc1, &adc_config);
if (status != ADC_STATUS_OK) {
    // Handle error
}

// 3. Read the ADC value
uint32_t raw_value;
status = ADC_ReadChannel(&hadc1, ADC_CHANNEL_0_PA0, &raw_value);
if (status == ADC_STATUS_OK) {
    printf("ADC Value: %lu\n", raw_value);
}
```

### Reading Voltage

```c
// Read voltage directly (0-3.3V range)
float voltage = ADC_ReadChannelVoltage(&hadc1, ADC_CHANNEL_0_PA0);
if (voltage >= 0.0f) {
    printf("Voltage: %.3f V\n", voltage);
}
```

## API Reference

### Initialization and Configuration

- `ADC_Init()` - Initialize ADC with configuration
- `ADC_DeInit()` - Deinitialize ADC
- `ADC_ConfigChannel()` - Configure specific ADC channel
- `ADC_Calibrate()` - Calibrate ADC (STM32F4 note: limited calibration)

### Basic Operations

- `ADC_StartConversion()` - Start ADC conversion
- `ADC_PollForConversion()` - Wait for conversion to complete
- `ADC_GetValue()` - Get raw ADC conversion result
- `ADC_ReadChannel()` - Complete read operation (config + start + poll + get)

### Voltage Operations

- `ADC_RawToVoltage()` - Convert raw ADC value to voltage
- `ADC_VoltageToRaw()` - Convert voltage to raw ADC value
- `ADC_ReadChannelVoltage()` - Read channel and return voltage

### Advanced Features

- `ADC_StartContinuousConversion()` - Start continuous conversions
- `ADC_StopContinuousConversion()` - Stop continuous conversions
- `ADC_StartDMA()` - Start DMA-based conversions
- `ADC_ReadMultiChannel()` - Read multiple channels

### Utility Functions

- `ADC_GetMaxValue()` - Get maximum value for resolution
- `ADC_GetChannelName()` - Get human-readable channel name
- `ADC_GetStatusString()` - Get human-readable status string

## Channel Mapping

| Channel Enum | Pin | Description |
|-------------|-----|-------------|
| ADC_CHANNEL_0_PA0 | PA0 | Analog input 0 |
| ADC_CHANNEL_1_PA1 | PA1 | Analog input 1 |
| ... | ... | ... |
| ADC_CHANNEL_15_PC5 | PC5 | Analog input 15 |
| ADC_CHANNEL_TEMP | Internal | Temperature sensor |
| ADC_CHANNEL_VREF | Internal | Voltage reference |
| ADC_CHANNEL_VBAT | Internal | Battery voltage |

## Configuration Options

### Resolution

- `ADC_RESOLUTION_12BIT` - 0-4095 (default)
- `ADC_RESOLUTION_10BIT` - 0-1023
- `ADC_RESOLUTION_8BIT`  - 0-255
- `ADC_RESOLUTION_6BIT`  - 0-63

### Sampling Time

- `ADC_SAMPLING_3_CYCLES` - Fastest (less accurate)
- `ADC_SAMPLING_15_CYCLES` - Fast
- `ADC_SAMPLING_84_CYCLES` - Default
- `ADC_SAMPLING_480_CYCLES` - Slowest (most accurate)

### Conversion Mode

- `ADC_MODE_SINGLE` - One conversion per trigger
- `ADC_MODE_CONTINUOUS` - Continuous conversions

## Examples

See `adc_example.c` and `adc_example.h` for complete working examples including:

- Basic ADC reading
- Multi-channel voltage measurement
- Continuous monitoring
- Internal sensor reading (temperature, Vref, Vbat)
- Error handling

### Running the Examples

To run all examples, call `ADC_Example_RunAll()` from your main function:

```c
#include "adc_example.h"

int main(void) {
    // Initialize system
    SYS_Init();

    // Run all ADC examples
    ADC_Example_RunAll();

    while (1) {
        // Your application code
    }
}
```

Individual examples can be called separately:

- `ADC_Example_BasicReading()` - Raw ADC values
- `ADC_Example_VoltageReading()` - Voltage measurements
- `ADC_Example_MultiChannelReading()` - Multiple channels
- `ADC_Example_InternalSensors()` - Temperature and internal references
- `ADC_Example_ContinuousConversion()` - Continuous mode

## Important Notes

1. **GPIO Configuration**: The driver automatically configures GPIO pins for analog input
2. **Clock Configuration**: ADC clocks are enabled automatically
3. **Calibration**: STM32F4 has limited calibration compared to newer STM32 series
4. **Reference Voltage**: Assumes 3.3V reference voltage
5. **Error Values**: Functions return -1.0f or -273.15f on error for voltage/temperature

## HAL Library Usage

This driver uses the following HAL functions:

- `HAL_ADC_Init()` - Initialize ADC peripheral
- `HAL_ADC_ConfigChannel()` - Configure ADC channel
- `HAL_ADC_Start()` - Start conversion
- `HAL_ADC_PollForConversion()` - Wait for completion
- `HAL_ADC_GetValue()` - Get conversion result
- `HAL_GPIO_Init()` - Configure GPIO pins

The HAL library handles the low-level register configuration, timing, and hardware abstraction.
