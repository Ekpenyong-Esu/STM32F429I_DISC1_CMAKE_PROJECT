# UART Module Documentation

## Overview
This UART module provides a flexible, modular implementation for UART communication on the STM32F429I-DISC1 microcontroller. It supports multiple transfer modes (Blocking, Interrupt, and DMA) and includes a ring buffer implementation for efficient data handling.

## Architecture

### Core Components

1. **Base UART Interface** (`uart.h`, `uart.c`)
   - Provides the core UART interface and types
   - Handles initialization and basic UART operations
   - Supports dynamic mode selection (Blocking/Interrupt/DMA)

2. **Configuration** (`uart_config.h`)
   - Contains all UART configuration parameters
   - Defines default settings (baud rate, word length, etc.)
   - Specifies buffer sizes and timeout values
   - Configures DMA channels and streams

### Transfer Modes

3. **Blocking Mode** (`uart_blocking.h`, `uart_blocking.c`)
   - Implements synchronous UART operations
   - Suitable for simple, low-throughput applications
   - Blocks until transfer is complete

4. **Interrupt Mode** (`uart_interrupt.h`, `uart_interrupt.c`)
   - Implements interrupt-driven UART operations
   - Better for responsive applications
   - Non-blocking operation with callbacks

5. **DMA Mode** (`uart_dma.h`, `uart_dma.c`)
   - Implements DMA-based UART operations
   - Optimal for high-throughput applications
   - Minimal CPU overhead

### Data Management

6. **Ring Buffer** (`uart_ring_buffer.h`, `uart_ring_buffer.c`)
   - Implements circular buffer for data management
   - Used primarily with DMA and interrupt modes
   - Prevents data loss during high-speed transfers

### Usage Example

7. **Example Implementation** (`uart_example.h`, `uart_example.c`)
   - Provides example usage of all UART features
   - Demonstrates proper initialization and usage
   - Shows integration with the main application

## Dependencies

```
uart.h
  ├── uart_types.h
  └── uart_config.h
       └── uart_ring_buffer.h

uart_blocking.h
  ├── uart.h
  └── uart_ring_buffer.h

uart_interrupt.h
  ├── uart.h
  └── uart_config.h

uart_dma.h
  ├── uart.h
  └── uart_config.h
```

## Usage

### 1. Initialization

```c
// Configure UART
UART_Config_t config = {
    .instance = USART1,
    .baudRate = 115200,
    .wordLength = UART_WORDLENGTH_8B,
    .stopBits = UART_STOPBITS_1,
    .parity = UART_PARITY_NONE,
    .mode = UART_MODE_TX_RX
};

// Initialize UART
UART_Handle_t uartHandle;
UART_Init(&uartHandle, &config);
```

### 2. Data Transfer

```c
// Transmit data
uint8_t data[] = "Hello World!";
UART_Transmit(&uartHandle, data, strlen((char*)data), 1000);

// Receive data
uint8_t rxBuffer[32];
UART_Receive(&uartHandle, rxBuffer, sizeof(rxBuffer), 1000);
```

### 3. Ring Buffer Usage

```c
// Initialize ring buffer
UART_RingBuffer_Init();

// Receive data using ring buffer
uint8_t data[32];
UART_RingBuffer_Receive(&uartHandle, data, sizeof(data));
```

## Configuration Options

Key configuration parameters in `uart_config.h`:

```c
/* Default UART configuration */
#define UART_DEFAULT_BAUDRATE     115200
#define UART_DEFAULT_WORDLENGTH   UART_WORDLENGTH_8B
#define UART_DEFAULT_STOPBITS     UART_STOPBITS_1
#define UART_DEFAULT_PARITY       UART_PARITY_NONE
#define UART_DEFAULT_MODE         UART_MODE_TX_RX

/* Buffer sizes */
#define RING_BUFFER_SIZE         512
#define RX_BUFFER_SIZE          512
#define TX_BUFFER_SIZE          512
```

## Integration with Main Application

The UART module is integrated into the main application through the following steps:

1. Include required headers in `main.c`:
   ```c
   #include "Peripherals/UART/uart.h"
   ```

2. Initialize UART in the system initialization:
   ```c
   /* Initialize system components */
   SYS_Init();
   ```

3. Use the example implementation or create custom UART handling:
   ```c
   UART_Example_MainLoop();
   ```

## Error Handling

The module includes comprehensive error handling:
- Status codes for all operations
- Timeout management
- Buffer overflow protection
- DMA error recovery

## Contributing

When modifying or extending this UART module:
1. Maintain the modular architecture
2. Follow the established error handling patterns
3. Update documentation for new features
4. Add examples for new functionality
5. Ensure backward compatibility

## License

This UART module is part of the STM32F429I-DISC1 bare metal implementation and follows the project's licensing terms.
