# LTDC (RGB) Driver for STM32F429 Discovery Board

This folder contains a minimal, clean driver for the STM32F429 LTDC (RGB interface) peripheral. It provides display initialization, layer management, framebuffer control, and a minimal pixel drawing API. All SPI/ILI9341 logic is excluded—this is for pure LTDC (RGB) only.

Note: The legacy SPI-based ILI9341 implementation (`Peripherals/LTDC/spiltdc.*`) has been removed in favor of the new dedicated driver `Peripherals/ILI9341/`. Use `ili9341.h` and `ili9341.c` for SPI-driven LCD initialization and control.

## Features

- Simple LTDC (RGB) initialization and configuration
- Double-buffered framebuffer management (see fb_manager)
- Minimal pixel drawing (no complex shapes/fonts)
- Layer/window configuration
- No SPI/ILI9341 or non-LTDC code

## Hardware Configuration

### STM32F429I-DISC1 LCD Display

| Specification | Value |
|---------------|-------|
| Display Size  | 2.4" TFT LCD |
| Resolution    | 240 x 320 pixels |
| Interface     | LTDC (RGB) |
| Colors        | 16.7M (24-bit) |
| Orientation   | Portrait/Landscape |

### GPIO Pin Configuration (LTDC only)

```c
// RGB Data Lines (24-bit)
// Red[7:0]   - PI15, PJ0-PJ6
// Green[7:0] - PJ7-PJ11, PK0-PK2
// Blue[7:0]  - PG12, PJ13-PJ15, PK3-PK6

// Control Signals
// HSYNC      - PI10
// VSYNC      - PI9
// DE         - PF10
// CLK        - PG7

// Backlight Control
// LCD_BL     - PK3 (or timer PWM for brightness control)
```

### Memory Configuration

```c
// SDRAM for framebuffers (external memory)
#define SDRAM_DEVICE_ADDR    0xD0000000
#define FRAMEBUFFER_SIZE     (240 * 320 * 2)  // RGB565 format

// Layer 0 framebuffer
#define LAYER0_FRAMEBUFFER   (SDRAM_DEVICE_ADDR)
// Layer 1 framebuffer  
#define LAYER1_FRAMEBUFFER   (SDRAM_DEVICE_ADDR + FRAMEBUFFER_SIZE)
```

## Quick Start

### 1. Basic Usage

```c
#include "ltdc.h"

int main(void) {
    // System initialization
    HAL_Init();
    SystemClock_Config();
    
    // Initialize LTDC hardware
    LTDC_HW_Init();
    
    // Initialize LTDC driver
    LTDC_Driver_t ltdcDriver;
    LTDC_Driver_Init(&ltdcDriver, &hltdc);
    
    // Configure display
    LTDC_DisplayConfig_t displayConfig = {
        .width = LTDC_DISPLAY_WIDTH,
        .height = LTDC_DISPLAY_HEIGHT,
        .backgroundColor = LTDC_COLOR_BLACK,
        .hsyncActiveLow = true,
        .vsyncActiveLow = true,
        .dataEnableActiveLow = false,
        .pixelClockInverted = true
    };
    
    LTDC_ConfigureDisplay(&ltdcDriver, &displayConfig);
    
    // Allocate and configure framebuffer
    uint32_t framebuffer = LAYER0_FRAMEBUFFER;
    
    LTDC_LayerConfig_t layerConfig = {
        .framebufferAddress = framebuffer,
        .windowX0 = 0,
        .windowY0 = 0,
        .windowX1 = LTDC_DISPLAY_WIDTH,
        .windowY1 = LTDC_DISPLAY_HEIGHT,
        .imageWidth = LTDC_DISPLAY_WIDTH,
        .imageHeight = LTDC_DISPLAY_HEIGHT,
        .pixelFormat = LTDC_PIXEL_FORMAT_RGB565_ENUM,
        .alpha = 255,
        .alpha0 = 0,
        .blendMode = LTDC_BLEND_CONSTANT_ALPHA,
        .backgroundColor = LTDC_COLOR_BLACK,
        .enabled = true
    };
    
    LTDC_ConfigureLayer(&ltdcDriver, 0, &layerConfig);
    LTDC_EnableLayer(&ltdcDriver, 0);
    LTDC_DisplayOn(&ltdcDriver);
    
    // Clear screen
    LTDC_ClearFramebuffer(&ltdcDriver, 0, LTDC_COLOR_BLUE);
    
    while (1) {
        // Main application loop
    }
}
```

### 2. Drawing Operations

```c
// Set active layer
LTDC_SetActiveLayer(&ltdcDriver, 0);

// Draw pixels
LTDC_DrawPixel(&ltdcDriver, 100, 150, LTDC_COLOR_RED);

// Draw lines
LTDC_Point_t start = {10, 10};
LTDC_Point_t end = {230, 50};
LTDC_DrawLine(&ltdcDriver, start, end, LTDC_COLOR_GREEN);

// Draw rectangles
LTDC_Rect_t rect = {50, 100, 100, 80};
LTDC_DrawRectangle(&ltdcDriver, &rect, LTDC_COLOR_BLUE, true); // Filled

// Draw circles
LTDC_Point_t center = {120, 160};
LTDC_DrawCircle(&ltdcDriver, center, 30, LTDC_COLOR_YELLOW, false); // Outline
```

### 3. Layer Management

```c
// Configure multiple layers
LTDC_LayerConfig_t layer0Config = {
    .framebufferAddress = LAYER0_FRAMEBUFFER,
    .pixelFormat = LTDC_PIXEL_FORMAT_RGB565_ENUM,
    .alpha = 255,  // Opaque
    // ... other settings
};

LTDC_LayerConfig_t layer1Config = {
    .framebufferAddress = LAYER1_FRAMEBUFFER,
    .pixelFormat = LTDC_PIXEL_FORMAT_RGB565_ENUM,
    .alpha = 128,  // Semi-transparent
    // ... other settings
};

LTDC_ConfigureLayer(&ltdcDriver, 0, &layer0Config);
LTDC_ConfigureLayer(&ltdcDriver, 1, &layer1Config);

// Enable both layers
LTDC_EnableLayer(&ltdcDriver, 0);
LTDC_EnableLayer(&ltdcDriver, 1);

// Adjust transparency
LTDC_SetLayerAlpha(&ltdcDriver, 1, 64); // More transparent

// Position layers
LTDC_SetLayerPosition(&ltdcDriver, 1, 50, 50);
```

### 4. Color Format Handling

```c
// Convert between color formats
uint32_t rgb888Color = 0xFF5733;  // Orange in RGB888
uint32_t rgb565Color = LTDC_RGB888_To_RGB565(rgb888Color);
uint32_t backToRgb888 = LTDC_RGB565_To_RGB888(rgb565Color);

// Use predefined colors
LTDC_ClearFramebuffer(&ltdcDriver, 0, LTDC_COLOR_RED);
LTDC_DrawPixel(&ltdcDriver, 50, 50, LTDC_COLOR_CYAN);

// Create custom colors (RGB565 format)
uint32_t customOrange = (0x1F << 11) | (0x2F << 5) | 0x06;
LTDC_DrawPixel(&ltdcDriver, 100, 100, customOrange);
```

### 5. Animation Example

```c
// Simple animation loop
int ballX = 50, ballY = 50;
int velX = 2, velY = 3;

while (1) {
    // Clear screen
    LTDC_ClearFramebuffer(&ltdcDriver, 0, LTDC_COLOR_BLACK);
    
    // Update position
    ballX += velX;
    ballY += velY;
    
    // Bounce off edges
    if (ballX <= 10 || ballX >= LTDC_DISPLAY_WIDTH - 10) {
        velX = -velX;
    }
    if (ballY <= 10 || ballY >= LTDC_DISPLAY_HEIGHT - 10) {
        velY = -velY;
    }
    
    // Draw ball
    LTDC_Point_t center = {ballX, ballY};
    LTDC_DrawCircle(&ltdcDriver, center, 10, LTDC_COLOR_WHITE, true);
    
    HAL_Delay(50);
}
```

## API Reference

### Data Structures

#### LTDC_Driver_t
Main driver handle structure (internal - do not modify directly).

#### LTDC_LayerConfig_t
```c
typedef struct {
    uint32_t framebufferAddress;        // Framebuffer start address
    uint16_t windowX0, windowY0;        // Window position
    uint16_t windowX1, windowY1;        // Window size
    uint16_t imageWidth, imageHeight;   // Image dimensions
    LTDC_PixelFormat_t pixelFormat;     // Pixel format
    uint8_t alpha;                      // Layer alpha (0-255)
    uint8_t alpha0;                     // Transparent pixel alpha
    LTDC_BlendMode_t blendMode;         // Blending mode
    uint32_t backgroundColor;           // Background color
    bool enabled;                       // Enable status
} LTDC_LayerConfig_t;
```

#### LTDC_DisplayConfig_t
```c
typedef struct {
    uint16_t width, height;             // Display dimensions
    uint32_t backgroundColor;           // Background color
    bool hsyncActiveLow;                // H-sync polarity
    bool vsyncActiveLow;                // V-sync polarity
    bool dataEnableActiveLow;           // Data enable polarity
    bool pixelClockInverted;            // Pixel clock polarity
} LTDC_DisplayConfig_t;
```

### Enumerations

#### LTDC_PixelFormat_t
```c
typedef enum {
    LTDC_PIXEL_FORMAT_ARGB8888_ENUM,    // 32-bit ARGB
    LTDC_PIXEL_FORMAT_RGB888_ENUM,      // 24-bit RGB
    LTDC_PIXEL_FORMAT_RGB565_ENUM,      // 16-bit RGB
    LTDC_PIXEL_FORMAT_ARGB1555_ENUM,    // 16-bit ARGB1555
    LTDC_PIXEL_FORMAT_ARGB4444_ENUM,    // 16-bit ARGB4444
    LTDC_PIXEL_FORMAT_L8_ENUM,          // 8-bit luminance
    LTDC_PIXEL_FORMAT_AL44_ENUM,        // 8-bit alpha-luminance
    LTDC_PIXEL_FORMAT_AL88_ENUM         // 16-bit alpha-luminance
} LTDC_PixelFormat_t;
```

#### LTDC_BlendMode_t
```c
typedef enum {
    LTDC_BLEND_CONSTANT_ALPHA,          // Constant alpha blending
    LTDC_BLEND_PIXEL_ALPHA,             // Per-pixel alpha blending
    LTDC_BLEND_NO_BLENDING              // No blending (opaque)
} LTDC_BlendMode_t;
```

### Core Functions

#### Initialization
```c
HAL_StatusTypeDef LTDC_Driver_Init(LTDC_Driver_t *driver, LTDC_HandleTypeDef *hltdc);
HAL_StatusTypeDef LTDC_Driver_DeInit(LTDC_Driver_t *driver);
HAL_StatusTypeDef LTDC_ConfigureDisplay(LTDC_Driver_t *driver, LTDC_DisplayConfig_t *config);
HAL_StatusTypeDef LTDC_HW_Init(void);
```

#### Layer Management
```c
HAL_StatusTypeDef LTDC_ConfigureLayer(LTDC_Driver_t *driver, uint8_t layer, LTDC_LayerConfig_t *config);
HAL_StatusTypeDef LTDC_EnableLayer(LTDC_Driver_t *driver, uint8_t layer);
HAL_StatusTypeDef LTDC_DisableLayer(LTDC_Driver_t *driver, uint8_t layer);
HAL_StatusTypeDef LTDC_SetActiveLayer(LTDC_Driver_t *driver, uint8_t layer);
HAL_StatusTypeDef LTDC_SetLayerAlpha(LTDC_Driver_t *driver, uint8_t layer, uint8_t alpha);
HAL_StatusTypeDef LTDC_SetLayerPosition(LTDC_Driver_t *driver, uint8_t layer, uint16_t x, uint16_t y);
```

#### Framebuffer Operations
```c
HAL_StatusTypeDef LTDC_SetFramebuffer(LTDC_Driver_t *driver, uint8_t layer, uint32_t address);
HAL_StatusTypeDef LTDC_ClearFramebuffer(LTDC_Driver_t *driver, uint8_t layer, uint32_t color);
HAL_StatusTypeDef LTDC_FillFramebuffer(LTDC_Driver_t *driver, uint8_t layer, uint32_t color);
HAL_StatusTypeDef LTDC_CopyFramebuffer(LTDC_Driver_t *driver, uint8_t srcLayer, uint8_t dstLayer);
```

#### Drawing Functions
```c
HAL_StatusTypeDef LTDC_DrawPixel(LTDC_Driver_t *driver, uint16_t x, uint16_t y, uint32_t color);
HAL_StatusTypeDef LTDC_DrawLine(LTDC_Driver_t *driver, LTDC_Point_t start, LTDC_Point_t end, uint32_t color);
HAL_StatusTypeDef LTDC_DrawRectangle(LTDC_Driver_t *driver, LTDC_Rect_t *rect, uint32_t color, bool filled);
HAL_StatusTypeDef LTDC_DrawCircle(LTDC_Driver_t *driver, LTDC_Point_t center, uint16_t radius, uint32_t color, bool filled);
```

#### Display Control
```c
HAL_StatusTypeDef LTDC_DisplayOn(LTDC_Driver_t *driver);
HAL_StatusTypeDef LTDC_DisplayOff(LTDC_Driver_t *driver);
HAL_StatusTypeDef LTDC_SetBackgroundColor(LTDC_Driver_t *driver, uint32_t color);
```

#### Utility Functions
```c
uint32_t LTDC_ConvertColor(uint32_t color, LTDC_PixelFormat_t fromFormat, LTDC_PixelFormat_t toFormat);
uint32_t LTDC_RGB888_To_RGB565(uint32_t rgb888);
uint32_t LTDC_RGB565_To_RGB888(uint16_t rgb565);
uint32_t LTDC_ARGB8888_To_RGB565(uint32_t argb8888);
```

#### Error Handling
```c
uint32_t LTDC_GetError(LTDC_Driver_t *driver);
HAL_StatusTypeDef LTDC_ClearError(LTDC_Driver_t *driver);
```

## Examples

### Running Examples

```c
#include "ltdc_example.h"

int main(void) {
    // System initialization
    HAL_Init();
    SystemClock_Config();
    
    // Initialize examples
    LTDC_ExamplesInit();
    
    // Run specific examples
    LTDC_BasicExample();
    LTDC_LayerExample();
    LTDC_DrawingExample();
    LTDC_ColorFormatExample();
    LTDC_FramebufferExample();
    LTDC_AnimationExample();
    LTDC_PerformanceExample();
    LTDC_ErrorHandlingExample();
    
    // Cleanup
    LTDC_ExamplesCleanup();
    
    while (1) {
        // Main application loop
    }
}
```

### Example Descriptions

1. **Basic Example**: Display initialization and basic color operations
2. **Layer Example**: Multi-layer configuration and alpha blending
3. **Drawing Example**: Drawing primitives (lines, rectangles, circles)
4. **Color Format Example**: Color space conversions and pixel formats
5. **Framebuffer Example**: Memory management and buffer operations
6. **Animation Example**: Animated graphics and real-time drawing
7. **Performance Example**: Timing tests and optimization techniques
8. **Error Handling Example**: Error detection and recovery procedures

## Integration Guide

### 1. Add Files to Project

Copy these files to your project:
- `ltdc.h` - Header file
- `ltdc.c` - Implementation file
- `ltdc_example.h` - Example header (optional)
- `ltdc_example.c` - Example implementation (optional)

### 2. Include in Build System

#### For STM32CubeIDE:
1. Add files to your `Src` and `Inc` directories
2. Files will be automatically included in build

#### For Makefile:
```makefile
# Add to C sources
C_SOURCES += Peripherals/LTDC/ltdc.c
C_SOURCES += Peripherals/LTDC/ltdc_example.c

# Add to include paths
C_INCLUDES += -IPeripherals/LTDC
```

#### For CMake:
```cmake
# Add source files
target_sources(${PROJECT_NAME} PRIVATE
    Peripherals/LTDC/ltdc.c
    Peripherals/LTDC/ltdc_example.c
)

# Add include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    Peripherals/LTDC
)
```

### 3. System Integration

#### Clock Configuration
Ensure LTDC and LCD clocks are properly configured:

```c
// In system clock configuration
void SystemClock_Config(void) {
    // Configure PLL for LTDC clock
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
    PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
    PeriphClkInitStruct.PLLSAI.PLLSAIR = 4;
    PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_8;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
}
```

#### GPIO Configuration
Configure LTDC GPIO pins:

```c
void LTDC_MspInit(LTDC_HandleTypeDef *hltdc) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    if (hltdc->Instance == LTDC) {
        // Enable GPIO clocks
        __HAL_RCC_GPIOF_CLK_ENABLE();
        __HAL_RCC_GPIOG_CLK_ENABLE();
        __HAL_RCC_GPIOI_CLK_ENABLE();
        __HAL_RCC_GPIOJ_CLK_ENABLE();
        __HAL_RCC_GPIOK_CLK_ENABLE();
        
        // Configure LTDC pins
        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
        HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
        
        // Configure more pins...
    }
}
```

#### SDRAM Configuration
Configure external SDRAM for framebuffers:

```c
// Initialize SDRAM for framebuffers
void SDRAM_Init(void) {
    // Configure FMC for SDRAM
    // This is typically done in the FMC driver
}
```

## Performance Considerations

### Memory Management
- **SDRAM Usage**: Use external SDRAM for framebuffers (internal RAM is limited)
- **DMA2D Integration**: Consider using DMA2D for hardware-accelerated operations
- **Memory Alignment**: Ensure framebuffers are properly aligned for optimal performance

### Pixel Format Selection
| Format | Bits/Pixel | Memory Usage | Performance | Color Quality |
|--------|------------|--------------|-------------|---------------|
| RGB565 | 16 | Low | High | Good |
| RGB888 | 24 | Medium | Medium | Excellent |
| ARGB8888 | 32 | High | Low | Excellent |

**Note:** Avoid using packed **RGB888 (24-bit)** framebuffers on the STM32 LTDC unless you explicitly handle stride/padding and DMA alignment. The LTDC and DMA engines fetch 32-bit-aligned bursts; packed 3-byte pixels can cause inefficient transfers or require per-line padding. Prefer **RGB565** (16-bit) or **ARGB8888** (32-bit) unless you have a specific need for 24-bit-packed memory and a well-tested stride implementation.

### Drawing Optimization
- **Batch Operations**: Group similar drawing operations together
- **Clipping**: Use layer windows to limit drawing areas
- **Double Buffering**: Use multiple framebuffers for smooth animation

```c
// Example: Double buffering
static uint32_t frontBuffer = LAYER0_FRAMEBUFFER;
static uint32_t backBuffer = LAYER1_FRAMEBUFFER;

void SwapBuffers(void) {
    uint32_t temp = frontBuffer;
    frontBuffer = backBuffer;
    backBuffer = temp;
    
    LTDC_SetFramebuffer(&ltdcDriver, 0, frontBuffer);
}
```

## Troubleshooting

### Common Issues

#### Display Not Working
1. **Check Clocks**: Verify LTDC and PLLSAI clocks are configured
2. **GPIO Configuration**: Ensure all LTDC pins are configured correctly
3. **Timing Parameters**: Verify timing matches display specifications
4. **Power Supply**: Check display power and backlight connections

#### Colors Incorrect
1. **Pixel Format**: Verify pixel format matches framebuffer data
2. **Byte Order**: Check RGB vs BGR byte ordering
3. **Color Conversion**: Verify color conversion functions
4. **Display Settings**: Check display controller configuration

#### Performance Issues
1. **Memory Bandwidth**: Use faster memory (SDRAM) for framebuffers
2. **Pixel Format**: Use RGB565 for better performance
3. **DMA2D**: Consider hardware acceleration for graphics operations
4. **Optimization**: Minimize drawing operations per frame

#### Layer Issues
1. **Alpha Blending**: Check alpha values and blending modes
2. **Layer Order**: Verify layer priorities and enable status
3. **Window Settings**: Check layer window coordinates
4. **Memory Layout**: Ensure framebuffers don't overlap

### Debug Tips

#### Enable Debug Output
```c
#define LTDC_DEBUG_ENABLED 1

#if LTDC_DEBUG_ENABLED
#define LTDC_DEBUG_PRINT(fmt, ...) printf("[LTDC_DEBUG]: " fmt "\n", ##__VA_ARGS__)
#else
#define LTDC_DEBUG_PRINT(fmt, ...)
#endif
```

#### Monitor Display Status
```c
// Check LTDC status
if (__HAL_LTDC_GET_FLAG(&hltdc, LTDC_FLAG_FU)) {
    printf("LTDC FIFO Underrun\n");
}

if (__HAL_LTDC_GET_FLAG(&hltdc, LTDC_FLAG_TE)) {
    printf("LTDC Transfer Error\n");
}
```

## Error Codes

| Error Code | Value | Description |
|------------|-------|-------------|
| LTDC_ERROR_NONE | 0x00 | No error |
| LTDC_ERROR_INVALID_PARAM | 0x01 | Invalid parameter |
| LTDC_ERROR_INIT_FAILED | 0x02 | Initialization failed |
| LTDC_ERROR_LAYER_CONFIG | 0x03 | Layer configuration failed |
| LTDC_ERROR_MEMORY_ALLOC | 0x04 | Memory allocation failed |
| LTDC_ERROR_INVALID_LAYER | 0x05 | Invalid layer number |
| LTDC_ERROR_FRAMEBUFFER | 0x06 | Framebuffer error |

## Version History

- **v1.0** (2025-09-03): Initial release
  - Complete LTDC driver implementation
  - Multi-layer support with alpha blending
  - Drawing primitives and color management
  - Comprehensive examples and documentation

## License

This software is provided as-is for educational and development purposes. Please refer to your STM32 license agreement for commercial usage terms.

## Support

For issues and questions:
1. Check the troubleshooting section above
2. Review example implementations
3. Consult STM32F429 reference manual for hardware details
4. Check STM32 HAL library documentation for API details
