# USB Peripheral Driver for STM32F429I-DISC1

This directory contains USB peripheral drivers and examples for the STM32F429I-Discovery board using the STM32 HAL library.

## Overview

The USB implementation provides both Host and Device functionality with a focus on CDC (Communication Device Class) support. The driver is built on top of the STM32 USB Host Library and provides a simplified API for common USB operations.

## Files

- `usb.h` - Main USB driver header file
- `usb.c` - Main USB driver implementation
- `usb_example.h` - USB examples header file
- `usb_example.c` - USB examples implementation
- `README.md` - This documentation file

## Features

### USB Host Mode
- CDC (Communication Device Class) support
- Device enumeration and management
- Data transmission and reception
- Line coding configuration
- Device information retrieval

### USB Examples
- Host CDC demonstration
- Echo/loopback functionality
- Test data transmission
- Device connection monitoring
- Status reporting

## USB Host CDC Features

- **Data Communication**: Send and receive data via USB CDC
- **Line Coding**: Configure baud rate, stop bits, parity, and data bits
- **Device Detection**: Automatic device connection/disconnection handling
- **Error Handling**: Comprehensive error reporting and recovery
- **Callbacks**: User-defined callbacks for events

## API Reference

### Core Functions

```c
USB_StatusTypeDef USB_Init(USB_ConfigTypeDef *config);
USB_StatusTypeDef USB_DeInit(void);
USB_StatusTypeDef USB_Start(void);
USB_StatusTypeDef USB_Stop(void);
```

### USB Host Functions

```c
USB_StatusTypeDef USB_Host_Init(void);
USB_StatusTypeDef USB_Host_Process(void);
USB_HostStateTypeDef USB_Host_GetState(void);
```

### USB Host CDC Functions

```c
USB_StatusTypeDef USB_Host_CDC_Transmit(uint8_t *data, uint16_t length);
USB_StatusTypeDef USB_Host_CDC_Receive(uint8_t *data, uint16_t length);
USB_StatusTypeDef USB_Host_CDC_SetLineCoding(CDC_LineCodingTypeDef *linecoding);
USB_StatusTypeDef USB_Host_CDC_GetLineCoding(CDC_LineCodingTypeDef *linecoding);
```

### Utility Functions

```c
uint8_t USB_IsDeviceConnected(void);
uint32_t USB_GetConnectedDeviceVID(void);
uint32_t USB_GetConnectedDevicePID(void);
```

## Usage Examples

### Basic USB Host CDC Setup

```c
#include "usb.h"
#include "usb_example.h"

int main(void)
{
    // System initialization
    HAL_Init();
    SystemClock_Config();
    
    // Initialize USB Host CDC example
    USB_Example_Init(USB_EXAMPLE_HOST_CDC);
    
    // Wait for device connection
    USB_Example_WaitForDeviceConnection(5000);
    
    // Run CDC demo
    USB_Example_Host_CDC_Demo();
    
    while(1)
    {
        // Process USB state machine
        USB_Example_Process();
        
        // Your application code here
        HAL_Delay(10);
    }
}
```

### Sending Data via USB CDC

```c
void SendData(void)
{
    const char *message = "Hello from STM32F429I!\r\n";
    
    if (USB_IsDeviceConnected())
    {
        USB_Host_CDC_Transmit((uint8_t*)message, strlen(message));
    }
}
```

### Receiving Data via USB CDC

```c
void ReceiveData(void)
{
    uint8_t rx_buffer[256];
    
    if (USB_IsDeviceConnected())
    {
        USB_Host_CDC_Receive(rx_buffer, sizeof(rx_buffer));
    }
}

// Implement the callback to handle received data
void USB_DataReceivedCallback(const uint8_t *data, uint16_t length)
{
    // Process received data
    // Echo it back
    USB_Host_CDC_Transmit((uint8_t*)data, length);
}
```

### Configure CDC Line Coding

```c
void ConfigureCDC(void)
{
    CDC_LineCodingTypeDef linecoding;
    
    linecoding.b.dwDTERate = 115200;     // Baud rate
    linecoding.b.bCharFormat = 0;        // 1 stop bit
    linecoding.b.bParityType = 0;        // No parity
    linecoding.b.bDataBits = 8;          // 8 data bits
    
    USB_Host_CDC_SetLineCoding(&linecoding);
}
```

## Hardware Requirements

### STM32F429I-DISC1 Board
- USB OTG HS port (micro USB connector)
- External power supply (if using high-power USB devices)

### USB Cable
- Micro USB cable for connecting USB devices
- USB OTG adapter (if needed for connecting standard USB devices)

## Pin Configuration

The USB OTG HS pins are typically configured as follows:
- **USB_OTG_HS_DM**: PB14 (Data Minus)
- **USB_OTG_HS_DP**: PB15 (Data Plus)
- **USB_OTG_HS_ID**: PB12 (OTG ID)
- **USB_OTG_HS_VBUS**: PB13 (VBUS)

## Dependencies

- STM32F4xx HAL Library
- STM32 USB Host Library
- FreeRTOS (if using RTOS features)

## Configuration

The USB configuration is typically handled by STM32CubeMX, but key settings include:

1. **USB OTG HS Mode**: Host
2. **PHY Interface**: ULPI or Embedded
3. **Speed**: High Speed or Full Speed
4. **DMA**: Optional
5. **Low Power Mode**: Optional

## Error Handling

The driver provides comprehensive error handling:

- `USB_STATUS_OK`: Operation successful
- `USB_STATUS_ERROR`: General error
- `USB_STATUS_BUSY`: Device busy
- `USB_STATUS_TIMEOUT`: Operation timeout
- `USB_STATUS_NOT_SUPPORTED`: Feature not supported

## Troubleshooting

### Common Issues

1. **Device Not Detected**
   - Check USB cable connection
   - Verify power supply
   - Ensure device is CDC compatible

2. **Data Transmission Errors**
   - Check line coding configuration
   - Verify buffer sizes
   - Ensure proper timing

3. **Enumeration Failures**
   - Check USB Host configuration
   - Verify clock settings
   - Test with different devices

### Debug Tips

- Use `USB_Example_PrintConnectionStatus()` to monitor connection state
- Check device VID/PID with `USB_GetConnectedDeviceVID()` and `USB_GetConnectedDevicePID()`
- Monitor USB Host state with `USB_Host_GetState()`

## Future Enhancements

- USB Device mode implementation
- Mass Storage Class (MSC) support
- Human Interface Device (HID) support
- USB OTG mode support
- Power management features

## License

This code is provided under the STMicroelectronics license terms.

## Support

For questions and support, refer to:
- STM32F429I-DISC1 User Manual
- STM32F4xx Reference Manual
- STM32CubeF4 documentation
- STM32 Community forums
