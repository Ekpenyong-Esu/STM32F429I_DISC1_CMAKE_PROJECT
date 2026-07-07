# IR Driver for Infrared Communication

## Overview

This driver provides a comprehensive interface for infrared (IR) communication on the STM32F429 Discovery board. It supports both IR transmission and reception with multiple protocols including NEC, RC5, SIRC, and custom formats. The driver uses timers for carrier generation and input capture for precise timing control.

## Features

- **Multiple Protocols**: Support for NEC, RC5, SIRC, and custom IR protocols
- **Full Duplex Operation**: Separate transmit and receive functionality
- **Carrier Generation**: Configurable carrier frequency (36-56 kHz) with adjustable duty cycle
- **Precise Timing**: Microsecond-accurate timing using hardware timers
- **Event-Driven**: Callback system for asynchronous operation
- **Error Handling**: Comprehensive error detection and reporting
- **HAL Integration**: Full STM32 HAL library compatibility

## Hardware Configuration

### STM32F429 Discovery Board Connections

- **IR TX**: PB5 (TIM3_CH2) - Infrared transmitter output
- **IR RX**: PB6 (TIM4_CH1) - Infrared receiver input

### External Hardware Requirements

- **IR LED**: Connected to TX pin through current-limiting resistor
- **IR Receiver**: TSOP series (e.g., TSOP4838) connected to RX pin
- **Power Supply**: 3.3V for logic, appropriate voltage for IR LED

### Timer Configuration

- **TIM3**: PWM generation for 38kHz carrier (TX)
- **TIM4**: Input capture for timing measurement (RX)

## Supported Protocols

### NEC Protocol
- **Data Format**: 32 bits (Address + ~Address + Command + ~Command)
- **Timing**: 9ms header, 562μs bit mark, variable space (562μs/1.687ms)
- **Frequency**: 38 kHz carrier
- **Usage**: Most common in consumer electronics

### RC5 Protocol
- **Data Format**: 14 bits (Start + Toggle + Address + Command)
- **Timing**: Manchester encoding, 1.778ms bit time
- **Frequency**: 36 kHz carrier
- **Usage**: Philips devices, European standard

### SIRC Protocol
- **Data Format**: 12/15/20 bits (Command + Address)
- **Timing**: 2.4ms header, 600μs bit mark, variable space
- **Frequency**: 40 kHz carrier
- **Usage**: Sony devices

## API Reference

### Initialization Functions

```c
HAL_StatusTypeDef IR_Init(IR_Handle_t *handle, TIM_HandleTypeDef *htimCarrier,
                         TIM_HandleTypeDef *htimCapture, GPIO_TypeDef *txPort,
                         uint16_t txPin, GPIO_TypeDef *rxPort, uint16_t rxPin,
                         uint32_t txChannel, uint32_t rxChannel, IR_Config_t *config);
HAL_StatusTypeDef IR_DeInit(IR_Handle_t *handle);
```

### Transmission Functions

```c
HAL_StatusTypeDef IR_TransmitNEC(IR_Handle_t *handle, uint8_t address, uint8_t command);
HAL_StatusTypeDef IR_TransmitRC5(IR_Handle_t *handle, uint8_t address, uint8_t command);
HAL_StatusTypeDef IR_TransmitSIRC(IR_Handle_t *handle, uint8_t address, uint8_t command);
HAL_StatusTypeDef IR_TransmitCustom(IR_Handle_t *handle, IR_Pulse_t *pulses, uint16_t count);
```

### Reception Functions

```c
HAL_StatusTypeDef IR_StartReceive(IR_Handle_t *handle);
HAL_StatusTypeDef IR_StopReceive(IR_Handle_t *handle);
HAL_StatusTypeDef IR_GetFrame(IR_Handle_t *handle, IR_Frame_t *frame);
```

### Configuration Functions

```c
HAL_StatusTypeDef IR_ConfigureCarrier(IR_Handle_t *handle, uint32_t frequency, uint8_t dutyCycle);
HAL_StatusTypeDef IR_SetTolerance(IR_Handle_t *handle, uint16_t tolerance);
HAL_StatusTypeDef IR_SetEventCallback(IR_Handle_t *handle, 
                                     void (*callback)(IR_Event_t event, IR_Frame_t *frame));
```

### Utility Functions

```c
IR_State_t IR_GetState(IR_Handle_t *handle);
uint32_t IR_GetError(IR_Handle_t *handle);
HAL_StatusTypeDef IR_ClearError(IR_Handle_t *handle);
```

## Configuration Options

### Protocols
- `IR_PROTOCOL_NEC`: NEC infrared protocol
- `IR_PROTOCOL_RC5`: RC5 infrared protocol  
- `IR_PROTOCOL_SIRC`: Sony SIRC protocol
- `IR_PROTOCOL_CUSTOM`: User-defined protocol

### Carrier Frequencies
- **36 kHz**: RC5 standard frequency
- **38 kHz**: Most common frequency (NEC, default)
- **40 kHz**: Sony SIRC frequency
- **56 kHz**: Some proprietary protocols

### States
- `IR_STATE_IDLE`: Ready for operation
- `IR_STATE_RECEIVING`: Actively receiving
- `IR_STATE_TRANSMITTING`: Actively transmitting
- `IR_STATE_PROCESSING`: Processing received data
- `IR_STATE_ERROR`: Error condition

### Events
- `IR_EVENT_FRAME_RECEIVED`: Valid frame decoded
- `IR_EVENT_FRAME_TRANSMITTED`: Transmission complete
- `IR_EVENT_REPEAT_RECEIVED`: Repeat code detected
- `IR_EVENT_ERROR_TIMEOUT`: Reception timeout
- `IR_EVENT_ERROR_PROTOCOL`: Protocol decode error
- `IR_EVENT_ERROR_OVERFLOW`: Buffer overflow

## Usage Examples

### Basic Initialization

```c
#include "ir.h"

IR_Handle_t hir;
TIM_HandleTypeDef htim3, htim4;

// Configure IR settings
IR_Config_t config = {
    .protocol = IR_PROTOCOL_NEC,
    .carrierFreq = 38000,
    .dutyCycle = 33,
    .tolerance = 200,
    .autoRepeat = true,
    .invertSignal = false
};

// Initialize IR driver
if (IR_Init(&hir, &htim3, &htim4, GPIOB, GPIO_PIN_5,
           GPIOB, GPIO_PIN_6, TIM_CHANNEL_2, TIM_CHANNEL_1, &config) == HAL_OK) {
    printf("IR driver initialized successfully!\\n");
}
```

### Transmitting IR Commands

```c
// NEC protocol transmission
IR_TransmitNEC(&hir, 0x12, 0x34);  // Address: 0x12, Command: 0x34

// RC5 protocol transmission  
IR_TransmitRC5(&hir, 0x05, 0x15);  // Address: 0x05, Command: 0x15

// SIRC protocol transmission
IR_TransmitSIRC(&hir, 0x07, 0x25); // Address: 0x07, Command: 0x25
```

### Custom Protocol Transmission

```c
// Define custom pulse sequence
IR_Pulse_t customPulses[] = {
    {9000, 4500},   // Header: 9ms mark, 4.5ms space
    {562, 1687},    // Bit 1: 562μs mark, 1.687ms space
    {562, 562},     // Bit 0: 562μs mark, 562μs space
    {562, 0}        // Stop bit: 562μs mark
};

// Transmit custom sequence
IR_TransmitCustom(&hir, customPulses, sizeof(customPulses)/sizeof(customPulses[0]));
```

### Receiving IR Commands

```c
// Start receiver
IR_StartReceive(&hir);

// Poll for received frames
IR_Frame_t frame;
while (running) {
    if (IR_GetFrame(&hir, &frame) == HAL_OK) {
        printf("Received: Protocol=%d, Address=0x%02X, Command=0x%02X\\n",
               frame.protocol, frame.address, frame.command);
    }
    HAL_Delay(10);
}

// Stop receiver
IR_StopReceive(&hir);
```

### Event-Driven Reception

```c
// Define event callback
void IR_EventCallback(IR_Event_t event, IR_Frame_t *frame) {
    switch (event) {
        case IR_EVENT_FRAME_RECEIVED:
            printf("Frame received: Address=0x%02X, Command=0x%02X\\n",
                   frame->address, frame->command);
            break;
        case IR_EVENT_ERROR_TIMEOUT:
            printf("Reception timeout\\n");
            break;
        default:
            break;
    }
}

// Set callback and start receiving
IR_SetEventCallback(&hir, IR_EventCallback);
IR_StartReceive(&hir);
```

### Carrier Configuration

```c
// Configure for different protocols
IR_ConfigureCarrier(&hir, 36000, 33);  // RC5: 36kHz, 33%
IR_ConfigureCarrier(&hir, 38000, 33);  // NEC: 38kHz, 33%  
IR_ConfigureCarrier(&hir, 40000, 33);  // SIRC: 40kHz, 33%

// Adjust timing tolerance
IR_SetTolerance(&hir, 300);  // 300μs tolerance
```

## Performance Considerations

### Timing Accuracy
- **Timer Resolution**: 1μs using 1MHz timer clock
- **Carrier Accuracy**: ±1% frequency stability required
- **Protocol Tolerance**: Configurable timing windows

### Memory Usage
- **Handle Size**: ~200 bytes
- **RX Buffer**: 256 × 32-bit timing values (1KB)
- **TX Buffer**: 128 × pulse structures (512 bytes)
- **Flash Usage**: ~8KB (driver + examples)

### Interrupt Handling
- **Input Capture**: High-priority interrupt for timing accuracy
- **PWM Generation**: Timer-based carrier modulation
- **Event Processing**: Background processing with callbacks

## Integration with STM32CubeMX

1. **Configure Timers**:
   - TIM3: PWM Generation mode, Channel 2
   - TIM4: Input Capture mode, Channel 1

2. **Configure GPIO**:
   - PB5: Alternate Function (AF2_TIM3)
   - PB6: Alternate Function (AF2_TIM4)

3. **Enable Interrupts**:
   - TIM4 Global Interrupt (for input capture)
   - Optional: TIM3 Update Interrupt (for transmission timing)

4. **Clock Configuration**:
   - Ensure timer clocks are properly configured
   - APB1 timer clock should be stable

## Troubleshooting

### Common Issues

1. **No IR Reception**
   - Check IR receiver connections and power
   - Verify timer configuration and interrupts
   - Ensure proper pull-up on RX pin

2. **Weak IR Transmission**
   - Check IR LED current limiting resistor
   - Verify carrier frequency matches receiver
   - Ensure adequate power supply

3. **Protocol Decode Errors**
   - Adjust timing tolerance
   - Check for electrical noise
   - Verify protocol selection

### Debug Tips

```c
// Monitor IR state
IR_State_t state = IR_GetState(&hir);
printf("IR State: %d\\n", state);

// Check error codes
uint32_t error = IR_GetError(&hir);
if (error != IR_ERROR_NONE) {
    printf("IR Error: 0x%08X\\n", error);
    IR_ClearError(&hir);
}

// Monitor received timing
// Enable debug output in driver for raw timing analysis
```

### Hardware Verification

1. **Oscilloscope Analysis**:
   - Verify carrier frequency and duty cycle
   - Check timing accuracy of transmitted pulses
   - Analyze received signal quality

2. **Logic Analyzer**:
   - Capture complete IR frames
   - Verify protocol timing compliance
   - Debug timing-related issues

## Dependencies

- STM32F4xx HAL Library
- Timer HAL module (TIM)
- GPIO HAL module
- Standard C library

## Protocol References

- **NEC Protocol**: Most widely used consumer IR protocol
- **RC5 Protocol**: Philips RC5 standard (IEC 60728-11)
- **SIRC Protocol**: Sony Infrared Remote Control system

## License

This driver is provided under the STM32 software license terms.

## Version History

- **v1.0**: Initial release with NEC, RC5, SIRC support
- Complete protocol implementation
- Event-driven architecture
- Comprehensive examples and documentation
- HAL library integration
