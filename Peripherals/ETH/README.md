# Ethernet Driver

This is a basic Ethernet driver for STM32F4 series microcontrollers, built on top of STM32 HAL (Hardware Abstraction Layer). It provides a low-level interface for Ethernet operations that can be used as a foundation for network protocol implementations.

## Features

- Basic Ethernet frame transmission and reception
- Configurable MAC address, speed, and duplex mode
- Interrupt-based communication
- Buffer management for TX/RX operations
- Callback support for transmit/receive completion
- Compatible with RMII/MII interfaces

## Files

- `eth.h` - Header file with function declarations and type definitions
- `eth.c` - Implementation of the Ethernet driver
- `eth_example.c` - Example usage code demonstrating ARP and ICMP operations
- `README.md` - This documentation file

## Architecture

The Ethernet driver provides a simplified interface over the STM32 HAL ETH functions:

- **ETH_Handle_t**: Main handle structure containing HAL handle and configuration
- **ETH_Config_t**: Configuration structure for MAC address, speed, duplex, etc.
- **ETH_Frame_t**: Structure representing an Ethernet frame with destination/source MAC, type, and payload

## Usage

### 1. Include the Header

```c
#include "eth.h"
```

### 2. Declare Ethernet Handle and Buffers

```c
ETH_Handle_t ethHandle;
uint8_t txBuffer[1518];  // Maximum Ethernet frame size
uint8_t rxBuffer[1518];
```

### 3. Configure Ethernet

```c
ETH_Config_t ethConfig = {
    .macAddr = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55},
    .speed = ETH_SPEED_100M,
    .duplexMode = ETH_MODE_FULLDUPLEX,
    .checksumMode = ETH_CHECKSUM_BY_SOFTWARE,
    .mediaInterface = ETH_MEDIA_INTERFACE_RMII
};
```

### 4. Initialize Ethernet

```c
// Assign buffers
ethHandle.rxBuffer = rxBuffer;
ethHandle.txBuffer = txBuffer;
ethHandle.rxBufferSize = sizeof(rxBuffer);
ethHandle.txBufferSize = sizeof(txBuffer);

// Initialize
if (ETH_Init(&ethHandle, &ethConfig) != HAL_OK) {
    // Handle error
}
```

### 5. Start Communication

```c
if (ETH_Start(&ethHandle) != HAL_OK) {
    // Handle error
}
```

### 6. Implement Callbacks (Optional)

```c
void ETH_TxCpltCallback(ETH_HandleTypeDef *heth) {
    // Transmission completed
}

void ETH_RxCpltCallback(ETH_HandleTypeDef *heth) {
    // Frame received
}

void ETH_ErrorCallback(ETH_HandleTypeDef *heth) {
    // Error occurred
}
```

### 7. Transmit Ethernet Frame

```c
ETH_Frame_t frame;
uint8_t destMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Broadcast
uint8_t payload[] = {0x01, 0x02, 0x03, 0x04};

memcpy(frame.destination, destMac, 6);
memcpy(frame.source, ethConfig.macAddr, 6);
frame.type = 0x0800; // IPv4
frame.payload = payload;
frame.payloadLength = sizeof(payload);

if (ETH_TransmitFrame(&ethHandle, &frame) == HAL_OK) {
    // Frame queued for transmission
}
```

### 8. Receive Ethernet Frame

```c
ETH_Frame_t receivedFrame;

if (ETH_ReceiveFrame(&ethHandle, &receivedFrame) == HAL_OK) {
    // Process received frame
    switch (receivedFrame.type) {
        case 0x0806: // ARP
            // Handle ARP
            break;
        case 0x0800: // IPv4
            // Handle IPv4
            break;
    }
}
```

### 9. Cleanup

```c
ETH_Stop(&ethHandle);
ETH_DeInit(&ethHandle);
```

## Configuration Parameters

### MAC Address
6-byte array representing the device's MAC address.

### Speed
- `ETH_SPEED_10M` - 10 Mbps
- `ETH_SPEED_100M` - 100 Mbps

### Duplex Mode
- `ETH_MODE_HALFDUPLEX` - Half duplex
- `ETH_MODE_FULLDUPLEX` - Full duplex

### Checksum Mode
- `ETH_CHECKSUM_BY_HARDWARE` - Hardware checksum calculation
- `ETH_CHECKSUM_BY_SOFTWARE` - Software checksum calculation

### Media Interface
- `ETH_MEDIA_INTERFACE_MII` - Media Independent Interface
- `ETH_MEDIA_INTERFACE_RMII` - Reduced Media Independent Interface

## Hardware Requirements

- STM32F4 microcontroller with Ethernet peripheral
- Ethernet PHY chip (e.g., LAN8742, DP83848)
- Proper RMII/MII pin connections
- 25MHz crystal for RMII reference clock

## Integration Notes

1. **PHY Configuration**: The driver assumes the PHY is properly configured and linked
2. **Buffer Management**: TX/RX buffers must be properly aligned and sized
3. **Interrupt Handling**: Ethernet interrupts should be enabled and handled in stm32f4xx_it.c
4. **Clock Configuration**: Ethernet clocks must be enabled in system initialization

## Example Applications

See `eth_example.c` for examples of:
- ARP request transmission
- ICMP echo request (ping) transmission
- Basic frame reception and processing

## Limitations

- This is a low-level driver - it doesn't implement network protocols
- No automatic PHY link detection or configuration
- Limited error handling and recovery
- No support for VLAN tagging or advanced features

## Future Enhancements

- PHY auto-negotiation and link status monitoring
- Jumbo frame support
- VLAN tagging
- Advanced error handling and statistics
- Integration with TCP/IP stacks (LWIP, etc.)

## License

This code is provided as-is for educational and development purposes.
