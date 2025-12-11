# CAN (Controller Area Network) Driver

A comprehensive CAN bus driver for STM32F429 microcontrollers, providing robust communication capabilities for automotive and industrial applications.

## Features

- **Full CAN Support**: Standard (11-bit) and Extended (29-bit) frame formats
- **Flexible Configuration**: Multiple baud rates, operating modes, and filter options
- **Error Handling**: Comprehensive error detection and reporting
- **Interrupt Support**: Callback-based interrupt handling for efficient operation
- **Filter Management**: Configurable acceptance filters for message filtering
- **Multiple Modes**: Normal, Loopback, Silent, and Silent-Loopback modes
- **Statistics Tracking**: Transmission and reception counters

## Architecture

The CAN driver consists of three main files:

- `can.h` - Header file with API definitions and data structures
- `can.c` - Implementation file with core functionality
- `can_example.c` - Comprehensive examples and usage patterns

## API Overview

### Initialization and Configuration

```c
// Basic CAN configuration
CAN_Config config = {
    .mode = CAN_MODE_NORMAL,
    .baud_rate = CAN_BAUD_500KBPS,
    .auto_retransmission = true,
    .auto_bus_off_recovery = true,
    .time_triggered_comm = false
};

// Initialize CAN
HAL_StatusTypeDef status = CAN_Init(&config);
```

### Frame Transmission

```c
// Create a CAN frame
CAN_Frame tx_frame = {
    .id = 0x123,
    .frame_type = CAN_FRAME_STANDARD,
    .data_length = 8,
    .remote_request = false
};
memcpy(tx_frame.data, your_data, 8);

// Transmit the frame
status = CAN_Transmit(&tx_frame, 100); // 100ms timeout
```

### Frame Reception

```c
// Receive a CAN frame
CAN_Frame rx_frame;
status = CAN_Receive(&rx_frame, 1000); // 1 second timeout

if (status == HAL_OK) {
    // Process received frame
    printf("Received ID: 0x%X, Data: %02X %02X...\n",
           rx_frame.id, rx_frame.data[0], rx_frame.data[1]);
}
```

### Filter Configuration

```c
// Configure acceptance filter
CAN_FilterConfig filter = {
    .filter_bank = 0,
    .mode = CAN_FILTER_MODE_ID_MASK,
    .scale = CAN_FILTER_SCALE_32BIT,
    .id = 0x100,
    .mask = 0x700,
    .enable = true
};

status = CAN_ConfigFilter(&filter);
```

### Interrupt Callbacks

```c
// Register callback functions
CAN_RegisterTxCallback(my_tx_callback);
CAN_RegisterRxCallback(my_rx_callback);
CAN_RegisterErrorCallback(my_error_callback);

// Enable interrupts
CAN_EnableInterrupts();
```

## Supported Baud Rates

- 10 kbps
- 20 kbps
- 50 kbps
- 100 kbps
- 125 kbps
- 250 kbps
- 500 kbps
- 1000 kbps

## Operating Modes

- **Normal Mode**: Standard CAN operation
- **Loopback Mode**: Internal loopback for testing
- **Silent Mode**: Listen-only mode for bus monitoring
- **Silent-Loopback Mode**: Combined silent and loopback

## Error Types

- Stuff Error
- Form Error
- Acknowledgment Error
- Bit Recessive Error
- Bit Dominant Error
- CRC Error
- Bus Off
- Bus Passive
- Bus Warning

## Usage Examples

### Basic Communication

```c
#include "can.h"

// Initialize CAN
CAN_Config config = {
    .mode = CAN_MODE_NORMAL,
    .baud_rate = CAN_BAUD_500KBPS
};
CAN_Init(&config);

// Send a message
CAN_Frame frame = {.id = 0x100, .data_length = 4};
frame.data[0] = 0xAA;
CAN_Transmit(&frame, 100);
```

### Advanced Configuration

```c
// Configure multiple filters
CAN_FilterConfig filter1 = {.filter_bank = 0, .id = 0x100, .mask = 0x7FF};
CAN_FilterConfig filter2 = {.filter_bank = 1, .id = 0x200, .mask = 0x7F0};
CAN_ConfigFilter(&filter1);
CAN_ConfigFilter(&filter2);

// Set up interrupt handling
CAN_RegisterRxCallback(process_received_frame);
CAN_EnableInterrupts();
```

### Error Monitoring

```c
CAN_Status status;
CAN_GetStatus(&status);

if (status.last_error != CAN_ERROR_NONE) {
    printf("CAN Error: %d\n", status.last_error);
    CAN_ClearErrors();
}
```

## Hardware Requirements

- STM32F429 microcontroller
- CAN transceiver (e.g., MCP2551, TJA1050)
- Proper CAN bus termination (120Ω resistors)
- CAN_H and CAN_L signal lines

## Pin Configuration

The CAN peripheral uses specific pins on STM32F429:

- CAN1_RX: PA11 or PB8 or PD0 or PI9
- CAN1_TX: PA12 or PB9 or PD1 or PH13
- CAN2_RX: PB5 or PB12
- CAN2_TX: PB6 or PB13

## Integration Notes

1. **Clock Configuration**: Ensure CAN peripheral clock is enabled
2. **GPIO Setup**: Configure CAN pins as alternate function
3. **NVIC Setup**: Enable CAN interrupts if using interrupt mode
4. **Transceiver**: Connect appropriate CAN transceiver to STM32 pins

## Performance Considerations

- Maximum throughput depends on baud rate and message length
- Interrupt mode provides better real-time performance
- Filter configuration affects CPU usage during reception
- Buffer sizes should be tuned based on application requirements

## Troubleshooting

### Common Issues

1. **No Communication**: Check transceiver power and bus termination
2. **Error Frames**: Verify baud rate configuration on all nodes
3. **Lost Messages**: Ensure adequate buffer sizes and processing speed
4. **Filter Issues**: Verify filter configuration matches expected IDs

### Debug Tips

- Use loopback mode for self-testing
- Monitor error counters with `CAN_GetStatus()`
- Enable interrupts for real-time error reporting
- Check bus voltage levels with oscilloscope

## Dependencies

- STM32F4xx HAL Library
- CMSIS Core
- Standard C libraries (stdint.h, stdbool.h, string.h)

## Thread Safety

The driver is not thread-safe by default. Use appropriate synchronization mechanisms (mutexes, semaphores) when accessing from multiple threads or interrupt contexts.

## License

This CAN driver is provided as-is for educational and development purposes.
