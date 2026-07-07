# DMA Peripheral Driver for STM32F429I-DISC1

## Overview

DMA (Direct Memory Access) implementation for STM32F429I Discovery board providing high-performance data transfer capabilities without CPU intervention.

## Hardware Configuration

### DMA Controllers
- **DMA1**: 8 streams, 8 channels per stream for basic peripherals
- **DMA2**: 8 streams, 8 channels per stream for high-speed peripherals
- **DMA2D**: 2D graphics DMA controller (see DMA2D folder)

### STM32F429I-DISC1 DMA Usage

#### DMA1 Typical Assignments
- **Stream 0**: SPI3_RX (Channel 0)
- **Stream 1**: I2C1_RX (Channel 1), SPI3_TX (Channel 0)
- **Stream 2**: SPI1_RX (Channel 3), I2C3_RX (Channel 3)
- **Stream 3**: SPI1_TX (Channel 3), I2C2_RX (Channel 7)
- **Stream 4**: SPI2_RX (Channel 0), I2C3_TX (Channel 3)
- **Stream 5**: SPI2_TX (Channel 0), I2C1_TX (Channel 1)
- **Stream 6**: I2C2_TX (Channel 7), USART2_TX (Channel 4)
- **Stream 7**: I2C1_TX (Channel 1), USART2_RX (Channel 4)

#### DMA2 Typical Assignments
- **Stream 0**: ADC1 (Channel 0), SPI1_RX (Channel 3)
- **Stream 1**: ADC3 (Channel 2), SPI1_TX (Channel 3)
- **Stream 2**: SPI1_RX (Channel 3), ADC2 (Channel 1)
- **Stream 3**: SPI1_TX (Channel 3), ADC2 (Channel 1)
- **Stream 4**: ADC1 (Channel 0), SPI1_RX (Channel 3)
- **Stream 5**: ADC3 (Channel 2), SPI1_TX (Channel 3)
- **Stream 6**: USART1_TX (Channel 4), SDIO (Channel 4)
- **Stream 7**: USART1_RX (Channel 4)

## Features

- **Transfer Types**:
  - Memory to memory transfer
  - Memory to peripheral transfer
  - Peripheral to memory transfer

- **Transfer Modes**:
  - Normal mode (single transfer)
  - Circular buffer mode
  - Double buffer mode

- **Advanced Features**:
  - FIFO mode with configurable threshold
  - Burst transfers
  - Priority levels (Low, Medium, High, Very High)
  - Transfer complete/error interrupts
  - Data width support (8-bit, 16-bit, 32-bit)

## Files

- `dma.h` - DMA driver header with API definitions
- `dma.c` - DMA driver implementation
- `dma_example.c` - Comprehensive usage examples
- `README.md` - This documentation file

## API Reference

### Initialization Functions
```c
HAL_StatusTypeDef DMA_Init(DMA_Handle_t *handle, DMA_Config_t *config);
HAL_StatusTypeDef DMA_DeInit(DMA_Handle_t *handle);
```

### Transfer Functions
```c
HAL_StatusTypeDef DMA_StartTransfer(DMA_Handle_t *handle, uint32_t srcAddr, uint32_t dstAddr, uint32_t dataLength);
HAL_StatusTypeDef DMA_StartPeriphToMem(DMA_Handle_t *handle, uint32_t periphAddr, uint32_t memAddr, uint32_t dataLength);
HAL_StatusTypeDef DMA_StartMemToPeriph(DMA_Handle_t *handle, uint32_t memAddr, uint32_t periphAddr, uint32_t dataLength);
HAL_StatusTypeDef DMA_StopTransfer(DMA_Handle_t *handle);
```

### Status Functions
```c
bool DMA_IsTransferComplete(DMA_Handle_t *handle);
uint32_t DMA_GetError(DMA_Handle_t *handle);
```

### Utility Functions
```c
IRQn_Type DMA_GetStreamIRQ(DMA_Stream_TypeDef *stream);
void DMA_IRQHandler(DMA_Handle_t *handle);
```

## Usage Examples

### Basic Memory to Memory Transfer
```c
DMA_Config_t config = {
    .stream = DMA2_Stream0,
    .channel = DMA_CHANNEL_0,
    .direction = DMA_MEMORY_TO_MEMORY,
    .mode = DMA_NORMAL,
    .priority = DMA_PRIORITY_MEDIUM,
    .dataSize = DMA_DATA_SIZE_WORD,
    .memInc = DMA_MINC_ENABLE,
    .periphInc = DMA_PINC_ENABLE,
    .fifoMode = DMA_FIFOMODE_DISABLE,
    .fifoThreshold = DMA_FIFO_THRESHOLD_FULL
};

DMA_Handle_t dma_handle;
DMA_Init(&dma_handle, &config);
DMA_StartTransfer(&dma_handle, src_addr, dst_addr, length);
```

### ADC with DMA (Continuous Conversion)
```c
DMA_Config_t adc_config = {
    .stream = DMA2_Stream4,        // ADC1 Stream
    .channel = DMA_CHANNEL_0,      // ADC1 Channel
    .direction = DMA_PERIPH_TO_MEMORY,
    .mode = DMA_CIRCULAR,          // Continuous conversion
    .priority = DMA_PRIORITY_HIGH,
    .dataSize = DMA_DATA_SIZE_HALFWORD,  // 16-bit ADC data
    .memInc = DMA_MINC_ENABLE,
    .periphInc = DMA_PINC_DISABLE,
    .fifoMode = DMA_FIFOMODE_DISABLE
};
```

### UART TX with DMA
```c
DMA_Config_t uart_config = {
    .stream = DMA1_Stream6,        // USART2 TX Stream
    .channel = DMA_CHANNEL_4,      // USART2 Channel
    .direction = DMA_MEMORY_TO_PERIPH,
    .mode = DMA_NORMAL,
    .priority = DMA_PRIORITY_LOW,
    .dataSize = DMA_DATA_SIZE_BYTE,
    .memInc = DMA_MINC_ENABLE,
    .periphInc = DMA_PINC_DISABLE,
    .fifoMode = DMA_FIFOMODE_DISABLE
};
```

## Interrupt Handling

Each DMA stream has its own interrupt. You need to implement the corresponding IRQ handlers in `stm32f4xx_it.c`:

```c
void DMA2_Stream0_IRQHandler(void) {
    DMA_IRQHandler(&my_dma_handle);
}
```

## Callback Functions

Implement these callback functions to handle transfer completion and errors:

```c
void DMA_TransferCompleteCallback(DMA_HandleTypeDef *hdma) {
    // Handle transfer completion
}

void DMA_TransferErrorCallback(DMA_HandleTypeDef *hdma) {
    // Handle transfer error
}
```

## Performance Considerations

1. **DMA2 vs DMA1**: Use DMA2 for high-speed peripherals (ADC, high-speed SPI/I2C)
2. **FIFO Mode**: Enable for burst transfers and when peripheral speed differs significantly from memory speed
3. **Priority**: Set appropriate priority levels to avoid conflicts
4. **Memory Alignment**: Align data buffers for optimal performance
5. **Cache Coherency**: Consider cache operations for high-speed transfers

## STM32F429I-DISC1 Specific Notes

- **External SDRAM**: Use FMC DMA for high-speed external memory access
- **LCD (LTDC)**: Uses dedicated DMA2D controller
- **Audio (I2S)**: Use DMA for continuous audio streaming
- **USB**: Has built-in DMA, external DMA not typically needed
- **Ethernet**: Has built-in DMA controller

## Common Use Cases on STM32F429I-DISC1

1. **ADC Sampling**: Continuous ADC conversion with circular DMA
2. **Audio Processing**: I2S DMA for audio codec communication
3. **Display Updates**: Memory to memory DMA for framebuffer operations
4. **Sensor Data**: SPI/I2C DMA for sensor communication
5. **Data Logging**: UART/SPI DMA for high-speed data transmission

## Troubleshooting

### Common Issues
1. **Transfer not starting**: Check stream/channel configuration
2. **Incomplete transfers**: Verify buffer sizes and alignment
3. **Interrupt not firing**: Ensure NVIC is properly configured
4. **Data corruption**: Check memory alignment and cache coherency

### Debug Tips
1. Use `DMA_GetError()` to check error status
2. Verify stream is not busy before starting new transfer
3. Check that peripheral is properly linked to DMA
4. Ensure correct stream/channel mapping for your peripheral
