# I2C Driver for STM32F429

This directory contains a comprehensive I2C (Inter-Integrated Circuit) driver implementation for the STM32F429 microcontroller, providing both basic and advanced communication capabilities.

## Files Overview

- **`i2c.h`** - Header file containing function prototypes, type definitions, and constants
- **`i2c.c`** - Main implementation file with all I2C communication functions
- **`i2c_example.c`** - Example code demonstrating various I2C operations

## Features

### Core Functionality
- **Basic Communication**: Master transmit and receive operations
- **Memory Operations**: Read/write to I2C memory devices (EEPROM, etc.)
- **Device Management**: Device readiness checking and bus scanning
- **Error Handling**: Comprehensive error detection and reporting
- **Custom Configuration**: Flexible I2C parameter configuration

### Supported Operations
- `I2C_Master_Transmit()` - Send data to I2C slave devices
- `I2C_Master_Receive()` - Receive data from I2C slave devices
- `I2C_Master_TransmitReceive()` - Combined transmit/receive operations
- `I2C_Mem_Write()` / `I2C_Mem_Read()` - Memory device operations
- `I2C_IsDeviceReady()` - Check if device is responding
- `I2C_ScanBus()` - Scan for connected I2C devices
- `I2C_GetError()` - Get detailed error information

## Usage Examples

### Basic Initialization
```c
#include "i2c.h"

// Initialize I2C with default settings (100 kHz, standard mode)
I2C_Init();
```

### Custom Configuration
```c
I2C_ConfigTypeDef customConfig = {
    .ClockSpeed = 400000,                      // 400 kHz (fast mode)
    .DutyCycle = I2C_DUTYCYCLE_16_9,          // 16:9 duty cycle
    .AddressingMode = I2C_ADDRESSINGMODE_7BIT, // 7-bit addressing
    .OwnAddress1 = 0x00,                       // Master mode
    .DualAddressMode = I2C_DUALADDRESS_DISABLE,
    .GeneralCallMode = I2C_GENERALCALL_DISABLE,
    .NoStretchMode = I2C_NOSTRETCH_DISABLE
};

I2C_Init_Custom(&customConfig);
```

### Communicating with Devices
```c
// Transmit data to a device
uint8_t txData[] = {0x01, 0x02, 0x03};
I2C_StatusTypeDef status = I2C_Master_Transmit(DEVICE_ADDR << 1, txData,
                                             sizeof(txData), I2C_TIMEOUT_DEFAULT);

// Receive data from a device
uint8_t rxData[4];
status = I2C_Master_Receive(DEVICE_ADDR << 1, rxData,
                           sizeof(rxData), I2C_TIMEOUT_DEFAULT);
```

### EEPROM Operations
```c
// Write to EEPROM
uint8_t data[] = {0xAA, 0xBB, 0xCC};
I2C_Mem_Write(EEPROM_ADDR << 1, 0x0000, I2C_MEMADD_SIZE_16BIT,
              data, sizeof(data), I2C_TIMEOUT_LONG);

// Read from EEPROM
uint8_t readData[3];
I2C_Mem_Read(EEPROM_ADDR << 1, 0x0000, I2C_MEMADD_SIZE_16BIT,
             readData, sizeof(readData), I2C_TIMEOUT_DEFAULT);
```

### Device Scanning
```c
// Scan for connected devices
uint8_t devices[16];
uint8_t count = I2C_ScanBus(devices, 16, I2C_TIMEOUT_SHORT);

printf("Found %d devices:\n", count);
for(uint8_t i = 0; i < count; i++) {
    printf("Device at 0x%02X\n", devices[i]);
}
```

## Configuration Constants

### Timeout Values
- `I2C_TIMEOUT_DEFAULT` - 1000ms (standard operations)
- `I2C_TIMEOUT_SHORT` - 100ms (quick operations)
- `I2C_TIMEOUT_LONG` - 5000ms (memory operations)

### Clock Speeds
- `I2C_CLOCK_SPEED_STANDARD` - 100 kHz (standard mode)
- `I2C_CLOCK_SPEED_FAST` - 400 kHz (fast mode)

### Address Ranges
- `I2C_ADDR_MIN` - 0x08 (minimum valid address)
- `I2C_ADDR_MAX` - 0x77 (maximum valid address)

## Error Handling

The driver provides comprehensive error handling with the following status codes:

- `I2C_OK` - Operation successful
- `I2C_ERROR` - General error
- `I2C_BUSY` - Bus is busy
- `I2C_TIMEOUT` - Operation timed out
- `I2C_NACK` - No acknowledge received
- `I2C_INVALID_PARAM` - Invalid parameter

Use `I2C_GetStatusString()` to convert status codes to human-readable strings.

## Hardware Configuration

The driver is configured for **I2C3** peripheral with the following default settings:

- **Clock Speed**: 100 kHz (standard mode)
- **Duty Cycle**: 2:1 (50% duty cycle)
- **Addressing**: 7-bit mode
- **Clock Stretching**: Enabled
- **General Call**: Disabled

### Pin Configuration (STM32F429-Discovery)
- **SCL**: PC9 (AF4)
- **SDA**: PA8 (AF4)

## Running Examples

To run the comprehensive examples:

```c
#include "i2c_example.h"

// Run all I2C examples
I2C_RunExamples();
```

This will demonstrate:
1. Basic transmit/receive operations
2. Memory read/write operations
3. Device scanning
4. Error handling scenarios
5. Custom configuration

## Integration Notes

### Dependencies
- STM32F4xx HAL Library
- System error handler (`Error_Handler()`)
- Standard C libraries (`string.h`, `stdint.h`)

### Thread Safety
The driver is not thread-safe. Implement mutex protection for multi-threaded applications.

### Power Management
Consider I2C peripheral clock gating for low-power applications.

## Troubleshooting

### Common Issues
1. **No Acknowledge (NACK)**: Check device address and connections
2. **Timeout Errors**: Verify pull-up resistors and bus capacitance
3. **Busy Errors**: Ensure proper bus arbitration and timing

### Debug Tips
- Use `I2C_ScanBus()` to verify device connectivity
- Check `I2C_GetError()` for detailed HAL error codes
- Verify I2C pin configurations in STM32CubeMX

## Performance Considerations

- **Standard Mode (100 kHz)**: Suitable for most applications
- **Fast Mode (400 kHz)**: Higher throughput but requires better signal integrity
- **Memory Operations**: Include write cycle delays for EEPROM devices
- **Bus Loading**: Limit bus capacitance for reliable high-speed operation

---

For more detailed information, refer to the STM32F429 Reference Manual and I2C specification.
