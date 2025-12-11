# SD Card Peripheral Driver

## Overview

This SD Card driver provides a comprehensive interface for accessing SD cards on the STM32F429I Discovery board using the SDIO peripheral. It supports basic read/write operations, DMA transfers, performance testing, and card management functions.

## Features

- **Card Detection**: Automatic detection of SD card presence
- **Write Protection**: Check and handle write-protected cards
- **Multiple Transfer Modes**: 
  - Blocking (polling) mode
  - Non-blocking DMA mode
- **Performance Testing**: Built-in speed benchmarking
- **Card Information**: Retrieve detailed card specifications
- **Format Support**: Quick format functionality
- **Error Handling**: Comprehensive error reporting and timeout management

## Hardware Requirements

- STM32F429I Discovery board
- SD card (SDHC/SDXC supported)
- Proper SDIO connections (typically built into the Discovery board)

### Pin Configuration

The SDIO interface uses the following pins on STM32F429I:
- **SDIO_CK**: PC12 (Clock)
- **SDIO_CMD**: PD2 (Command)
- **SDIO_D0**: PC8 (Data 0)
- **SDIO_D1**: PC9 (Data 1)
- **SDIO_D2**: PC10 (Data 2)
- **SDIO_D3**: PC11 (Data 3)
- **Card Detect**: PC13 (optional, board-specific)

## Configuration

### HAL Configuration

Ensure the following is enabled in `stm32f4xx_hal_conf.h`:
```c
#define HAL_SD_MODULE_ENABLED
```

### Clock Configuration

The SDIO peripheral requires proper clock configuration. Typical settings:
- SDIO Clock: 48 MHz (derived from PLL48CLK)
- Card Clock: Configurable (typically 1-25 MHz depending on card type)

## API Reference

### Core Functions

#### Initialization
```c
SDCARD_StatusTypeDef SDCARD_Init(void);
```
Initialize the SD card interface and detect the card.

#### Card Detection
```c
bool SDCARD_IsCardPresent(void);
bool SDCARD_IsWriteProtected(void);
```
Check card presence and write protection status.

#### Card Information
```c
SDCARD_StatusTypeDef SDCARD_GetCardInfo(SDCARD_CardInfoTypeDef* pCardInfo);
uint64_t SDCARD_GetCapacity(void);
HAL_SD_CardStateTypeDef SDCARD_GetCardState(void);
```
Retrieve card specifications and current state.

### Data Transfer Functions

#### Blocking Operations
```c
SDCARD_StatusTypeDef SDCARD_ReadBlocks(uint8_t* pData, uint32_t BlockAdd, 
                                      uint32_t NumberOfBlocks, uint32_t Timeout);
SDCARD_StatusTypeDef SDCARD_WriteBlocks(uint8_t* pData, uint32_t BlockAdd, 
                                       uint32_t NumberOfBlocks, uint32_t Timeout);
```
Synchronous read/write operations with timeout.

#### DMA Operations
```c
SDCARD_StatusTypeDef SDCARD_ReadBlocks_DMA(uint8_t* pData, uint32_t BlockAdd, 
                                          uint32_t NumberOfBlocks);
SDCARD_StatusTypeDef SDCARD_WriteBlocks_DMA(uint8_t* pData, uint32_t BlockAdd, 
                                           uint32_t NumberOfBlocks);
```
Asynchronous read/write operations using DMA.

### Utility Functions

#### Performance Testing
```c
SDCARD_StatusTypeDef SDCARD_TestPerformance(uint32_t* pReadSpeed, uint32_t* pWriteSpeed);
```
Measure read and write speeds in KB/s.

#### Format
```c
SDCARD_StatusTypeDef SDCARD_Format(uint8_t fullFormat);
```
Format the SD card (quick or full format).

### Status Types

```c
typedef enum {
    SDCARD_STATUS_OK = 0,
    SDCARD_STATUS_ERROR,
    SDCARD_STATUS_BUSY,
    SDCARD_STATUS_TIMEOUT,
    SDCARD_STATUS_NOT_READY,
    SDCARD_STATUS_NO_CARD,
    SDCARD_STATUS_WRITE_PROTECTED
} SDCARD_StatusTypeDef;
```

## Usage Examples

### Basic Usage

```c
#include "sdcard.h"

int main(void) {
    SDCARD_StatusTypeDef status;
    uint8_t buffer[512];
    
    // Initialize SD card
    status = SDCARD_Init();
    if (status != SDCARD_STATUS_OK) {
        // Handle initialization error
        return -1;
    }
    
    // Check if card is present
    if (!SDCARD_IsCardPresent()) {
        // No card detected
        return -1;
    }
    
    // Read first block
    status = SDCARD_ReadBlocks(buffer, 0, 1, SDCARD_TIMEOUT_READ);
    if (status == SDCARD_STATUS_OK) {
        // Process read data
    }
    
    return 0;
}
```

### DMA Usage

```c
void DMA_Example(void) {
    uint8_t writeBuffer[1024];
    uint8_t readBuffer[1024];
    
    // Initialize card
    SDCARD_Init();
    
    // Prepare data
    memset(writeBuffer, 0xAA, sizeof(writeBuffer));
    
    // Start DMA write
    SDCARD_WriteBlocks_DMA(writeBuffer, 100, 2);
    
    // Wait for completion (implement proper callback handling)
    while(SDCARD_GetCardState() != HAL_SD_CARD_TRANSFER);
    
    // Start DMA read
    SDCARD_ReadBlocks_DMA(readBuffer, 100, 2);
    
    // Wait for completion
    while(SDCARD_GetCardState() != HAL_SD_CARD_TRANSFER);
}
```

### Performance Testing

```c
void Performance_Test(void) {
    uint32_t readSpeed, writeSpeed;
    
    SDCARD_Init();
    
    if (SDCARD_TestPerformance(&readSpeed, &writeSpeed) == SDCARD_STATUS_OK) {
        printf("Read Speed: %lu KB/s\n", readSpeed);
        printf("Write Speed: %lu KB/s\n", writeSpeed);
    }
}
```

## Constants and Timeouts

| Constant | Value | Description |
|----------|-------|-------------|
| `SDCARD_BLOCK_SIZE` | 512 | Standard SD card block size |
| `SDCARD_TIMEOUT_READ` | 3000 | Read operation timeout (ms) |
| `SDCARD_TIMEOUT_WRITE` | 3000 | Write operation timeout (ms) |
| `SDCARD_TIMEOUT_ERASE` | 30000 | Erase operation timeout (ms) |

## Performance Guidelines

### Speed Classes
- **Class 2**: Minimum 2 MB/s sustained write
- **Class 4**: Minimum 4 MB/s sustained write
- **Class 6**: Minimum 6 MB/s sustained write
- **Class 10**: Minimum 10 MB/s sustained write

### Optimization Tips

1. **Use DMA**: For better performance and CPU efficiency
2. **Block Alignment**: Read/write multiple blocks when possible
3. **Buffer Management**: Use properly aligned buffers for DMA
4. **Error Handling**: Always check return status
5. **Card Quality**: Use high-quality, fast SD cards for better performance

## Error Handling

The driver provides comprehensive error handling:

```c
SDCARD_StatusTypeDef status = SDCARD_ReadBlocks(buffer, 0, 1, SDCARD_TIMEOUT_READ);

switch (status) {
    case SDCARD_STATUS_OK:
        // Operation successful
        break;
    case SDCARD_STATUS_NO_CARD:
        // Card not present
        break;
    case SDCARD_STATUS_WRITE_PROTECTED:
        // Card is write protected
        break;
    case SDCARD_STATUS_TIMEOUT:
        // Operation timed out
        break;
    case SDCARD_STATUS_ERROR:
        // General error occurred
        break;
}
```

## Interrupt Handling

The driver supports interrupt-driven operations through callbacks:

```c
// Implement these callbacks in your application
void SDCARD_TxCpltCallback(void);  // DMA transmit complete
void SDCARD_RxCpltCallback(void);  // DMA receive complete
void SDCARD_ErrorCallback(void);   // Error occurred
```

## File System Integration

This driver provides low-level block access. For file system support, integrate with:
- **FatFS**: Popular FAT file system implementation
- **LittleFS**: Embedded file system for flash storage
- **Custom**: Implement your own simple file system

Example FatFS integration:
```c
// In diskio.c for FatFS
DSTATUS disk_initialize(BYTE pdrv) {
    if (SDCARD_Init() == SDCARD_STATUS_OK) {
        return RES_OK;
    }
    return RES_ERROR;
}

DRESULT disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count) {
    if (SDCARD_ReadBlocks(buff, sector, count, SDCARD_TIMEOUT_READ) == SDCARD_STATUS_OK) {
        return RES_OK;
    }
    return RES_ERROR;
}
```

## Testing

The module includes comprehensive examples:
- `SDCARD_BasicExample()`: Basic initialization and card info
- `SDCARD_ReadWriteExample()`: Read/write operations
- `SDCARD_DMAExample()`: DMA transfer operations
- `SDCARD_PerformanceExample()`: Speed testing
- `SDCARD_FormatExample()`: Card formatting
- `SDCARD_FileOperationsExample()`: Simulated file operations

## Troubleshooting

### Common Issues

1. **Card Not Detected**
   - Check physical connections
   - Verify card is properly inserted
   - Check card detect pin configuration

2. **Slow Performance**
   - Use DMA for large transfers
   - Ensure proper clock configuration
   - Check card speed class

3. **Write Errors**
   - Check write protection switch
   - Verify card is not full
   - Check for card corruption

4. **DMA Issues**
   - Ensure buffers are properly aligned
   - Check DMA configuration
   - Verify interrupt priorities

### Debug Tips

1. Use the built-in performance test to verify card functionality
2. Check card information to verify proper detection
3. Start with small block transfers before attempting large operations
4. Monitor card state during DMA operations

## Notes

- This driver is specifically designed for STM32F429I Discovery board
- Only tested with SDHC/SDXC cards (up to 32GB+)
- DMA operations require proper buffer alignment
- Always check return status for all operations
- Format operations will erase all data on the card

## Author

Mahonri - STM32F429I Discovery Board SD Card Driver

## License

This software is provided as-is, without any express or implied warranties.
