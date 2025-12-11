# DMA2D Driver Module

A comprehensive, production-ready DMA2D (Chrom-Art Accelerator) driver for STM32F4/F7 series microcontrollers with hardware-accelerated 2D graphics operations.

## Features

- **Complete API**: Full DMA2D functionality including fill, copy, blending, and format conversion
- **Thread Safety**: Optional mutex-based protection for multi-threaded applications
- **Error Handling**: Comprehensive error reporting and status tracking
- **Interrupt Support**: Both polling and interrupt-driven operations
- **Performance Monitoring**: Transfer statistics and performance tracking
- **Debug Support**: Optional debug output for development
- **Validation**: Parameter validation and self-test capabilities
- **Documentation**: Extensive inline documentation and examples

## Supported Operations

- **Fill Operations**: Register-to-memory fill with solid colors
- **Copy Operations**: Memory-to-memory data transfer
- **Blending**: Alpha blending between two layers
- **Format Conversion**: Pixel format conversion during transfer
- **Interrupt Mode**: Non-blocking operations with callbacks

## Supported Color Formats

- ARGB8888 (32-bit with alpha)
- RGB888 (24-bit)
- RGB565 (16-bit)
- ARGB1555 (16-bit with alpha)
- ARGB4444 (16-bit with alpha)

## Quick Start

### 1. Include Headers

```c
#include "dma2d.h"
```

### 2. Initialize DMA2D

```c
DMA2D_Config config = {
    .mode = DMA2D_MODE_R2M,
    .color_mode = DMA2D_FORMAT_ARGB8888,
    .output_offset = 0
};

HAL_StatusTypeDef status = DMA2D_Init(&config);
if (status != HAL_OK) {
    // Handle initialization error
}
```

### 3. Basic Fill Operation

```c
// Fill a 100x100 area with red color
uint32_t framebuffer[100 * 100];
uint32_t red_color = DMA2D_MakeColor(255, 0, 0, 255); // Red with full alpha

status = DMA2D_StartFill(red_color, framebuffer, 100, 100);
if (status != HAL_OK) {
    // Handle fill error
}
```

### 4. Copy Operation

```c
// Copy image data to framebuffer
const uint32_t *source_image = get_source_image();
uint32_t *destination_fb = get_framebuffer();

status = DMA2D_StartTransfer(source_image, destination_fb, 320, 240);
if (status != HAL_OK) {
    // Handle copy error
}
```

### 5. Alpha Blending

```c
// Blend two images with alpha transparency
const uint32_t *foreground = get_foreground_image();
const uint32_t *background = get_background_image();
uint32_t *result = get_result_buffer();

status = DMA2D_StartBlending(foreground, background, result, 320, 240);
if (status != HAL_OK) {
    // Handle blending error
}
```

## Configuration Options

### Thread Safety

Enable thread safety by defining `DMA2D_USE_MUTEX`:

```c
#define DMA2D_USE_MUTEX
```

This requires CMSIS-RTOS to be available.

### Debug Output

Enable debug output by defining `DMA2D_ENABLE_DEBUG`:

```c
#define DMA2D_ENABLE_DEBUG
```

This will output debug information via printf().

## Advanced Usage

### Interrupt-Driven Operations

```c
// Register callback functions
DMA2D_RegisterTransferCompleteCallback(my_complete_callback);
DMA2D_RegisterTransferErrorCallback(my_error_callback);

// Start interrupt-driven operation
status = DMA2D_StartFill_IT(red_color, framebuffer, 100, 100);
if (status == HAL_OK) {
    // Operation started, will complete asynchronously
}
```

### Layer Configuration

```c
// Configure foreground layer for blending
DMA2D_LayerConfig fg_config = {
    .input_color_mode = DMA2D_INPUT_ARGB8888,
    .input_alpha_mode = DMA2D_ALPHA_COMBINE,
    .input_alpha = 128,  // 50% transparency
    .input_offset = 0
};

status = DMA2D_ConfigLayer(DMA2D_FOREGROUND_LAYER, &fg_config);
```

### Status Monitoring

```c
DMA2D_Status status_info;
DMA2D_GetStatus(&status_info);

printf("Transfers completed: %lu\n", status_info.transfer_count);
printf("Total bytes transferred: %lu\n", status_info.total_bytes_transferred);
printf("Error count: %lu\n", status_info.error_count);
```

## API Reference

### Core Functions

- `DMA2D_Init()` - Initialize DMA2D peripheral
- `DMA2D_DeInit()` - Deinitialize DMA2D peripheral
- `DMA2D_StartTransfer()` - Start memory-to-memory transfer (polling)
- `DMA2D_StartFill()` - Start register-to-memory fill (polling)
- `DMA2D_StartBlending()` - Start alpha blending (polling)
- `DMA2D_StartTransfer_IT()` - Start transfer (interrupt mode)
- `DMA2D_StartFill_IT()` - Start fill (interrupt mode)
- `DMA2D_StartBlending_IT()` - Start blending (interrupt mode)

### Utility Functions

- `DMA2D_MakeColor()` - Create ARGB8888 color value
- `DMA2D_GetColorComponents()` - Extract color components
- `DMA2D_IsBusy()` - Check if DMA2D is busy
- `DMA2D_GetStatus()` - Get detailed status information
- `DMA2D_SelfTest()` - Run self-test

### Callback Registration

- `DMA2D_RegisterTransferCompleteCallback()` - Register completion callback
- `DMA2D_RegisterTransferErrorCallback()` - Register error callback
- `DMA2D_RegisterTransferProgressCallback()` - Register progress callback

## Error Handling

The driver provides comprehensive error handling:

```c
HAL_StatusTypeDef status = DMA2D_StartTransfer(src, dst, width, height);
if (status != HAL_OK) {
    const char *error_str = DMA2D_GetErrorString(status);
    printf("DMA2D Error: %s\n", error_str);

    DMA2D_Status dma2d_status;
    DMA2D_GetStatus(&dma2d_status);
    printf("Last HAL error: 0x%08lX\n", dma2d_status.last_error);
}
```

## Performance Considerations

- **Memory Alignment**: Ensure buffers are properly aligned for optimal performance
- **Color Format**: Choose appropriate color formats for your application
- **Transfer Size**: Larger transfers are more efficient than many small ones
- **Interrupt vs Polling**: Use interrupt mode for non-blocking operations
- **Layer Configuration**: Configure layers once, then reuse for multiple operations

## Integration with STM32CubeMX

1. Enable DMA2D peripheral in STM32CubeMX
2. Configure clock settings for DMA2D
3. Add this driver to your project
4. Include the header file in your source files
5. Call `DMA2D_Init()` after HAL initialization

## Dependencies

- STM32F4xx or STM32F7xx HAL library
- CMSIS core (for interrupt handling)
- Optional: CMSIS-RTOS (for thread safety)

## Limitations

- Maximum transfer dimensions: 8192x8192 pixels
- Color format conversion is hardware-accelerated but may have limitations
- Alpha blending requires specific layer configurations
- Some advanced features may not be available on all STM32 variants

## Examples

See `dma2d_example.c` for comprehensive usage examples including:
- Basic fill operations
- Image copying
- Alpha blending
- Format conversion
- Error handling patterns
- Performance optimization techniques

## Version History

- **v2.0.0**: Complete rewrite with best practices, thread safety, comprehensive error handling
- **v1.0.0**: Initial release with basic DMA2D functionality

## License

This driver is provided as-is for educational and development purposes. Please refer to your STM32 license agreements for production use.

## Support

For issues and questions:
1. Check the inline documentation
2. Review the example code
3. Verify your STM32CubeMX configuration
4. Ensure proper HAL library integration
