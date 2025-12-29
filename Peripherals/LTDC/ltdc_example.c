/**
 * @file ltdc_example.c
 * @brief LTDC driver example implementations for STM32F429 Discovery board
 * @details This file provides example implementations demonstrating how to use
 *          the LTDC driver for controlling the LCD display on the STM32F429 Discovery board.
 * @version 1.0
 * @date 2025-09-03
 * @author STM32 Team
 */

/* Includes ------------------------------------------------------------------*/
#include "ltdc_example.h"
#include "fb_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Private defines -----------------------------------------------------------*/
#define EXAMPLE_DELAY_MS        2000
#define ANIMATION_FRAME_DELAY   50
#define MAX_MESSAGE_LEN         256

/* Private variables ---------------------------------------------------------*/
static LTDC_Driver_t ltdcDriver;
static bool examplesInitialized = false;
static uint32_t framebuffer1 = 0;
static uint32_t framebuffer2 = 0;

/* Private function prototypes -----------------------------------------------*/
static void LTDC_PrintMessage(const char* message);
static void LTDC_Delay(uint32_t ms);
static HAL_StatusTypeDef LTDC_ValidateExample(void);



/* Public Functions ----------------------------------------------------------*/

/**
 * @brief Initialize all LTDC examples
 * @details Sets up all necessary components for LTDC examples
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_ExamplesInit(void) {
    LTDC_PrintMessage("Initializing LTDC Examples...");

    /* Initialize hardware */
    HAL_StatusTypeDef status = LTDC_HW_Init();
    if (status != HAL_OK) {
        LTDC_PrintMessage("Error: LTDC hardware initialization failed");
        return status;
    }

    /* Initialize LTDC driver */
    status = LTDC_Driver_Init(&ltdcDriver, &hltdc);
    if (status != HAL_OK) {
        LTDC_PrintMessage("Error: LTDC driver initialization failed");
        return status;
    }

    /* Configure display */
    LTDC_DisplayConfig_t displayConfig = {
        .width = LTDC_DISPLAY_WIDTH,
        .height = LTDC_DISPLAY_HEIGHT,
        .backgroundColor = LTDC_COLOR_BLACK,
        .hsyncActiveLow = true,
        .vsyncActiveLow = true,
        .dataEnableActiveLow = false,
        .pixelClockInverted = true
    };

    status = LTDC_ConfigureDisplay(&ltdcDriver, &displayConfig);
    if (status != HAL_OK) {
        LTDC_PrintMessage("Error: LTDC display configuration failed");
        return status;
    }

    /* Initialize framebuffer manager (double-buffering in SDRAM) */
    if (fb_manager_init(LTDC_DISPLAY_WIDTH, LTDC_DISPLAY_HEIGHT) != 0) {
        LTDC_PrintMessage("Error: fb_manager initialization failed");
        return HAL_ERROR;
    }

    /* Get active and backbuffer addresses managed by fb_manager */
    framebuffer1 = fb_manager_get_active_address();
    framebuffer2 = fb_manager_get_backbuffer_address();

    /* Configure layer 0 */
    LTDC_LayerConfig_t layerConfig = {
        .framebufferAddress = framebuffer1,
        .windowX0 = 0,
        .windowY0 = 0,
        /* inclusive end coordinates */
        .windowX1 = LTDC_DISPLAY_WIDTH - 1,
        .windowY1 = LTDC_DISPLAY_HEIGHT - 1,
        .imageWidth = LTDC_DISPLAY_WIDTH,
        .imageHeight = LTDC_DISPLAY_HEIGHT,
        .pixelFormat = LTDC_PIXEL_FORMAT_RGB565_ENUM,
        .alpha = 255,
        .alpha0 = 0,
        .blendMode = LTDC_BLEND_CONSTANT_ALPHA,
        .backgroundColor = LTDC_COLOR_BLACK,
        .enabled = true
    };

    status = LTDC_ConfigureLayer(&ltdcDriver, 0, &layerConfig);
    if (status != HAL_OK) {
        LTDC_PrintMessage("Error: Layer 0 configuration failed");
        return status;
    }

    /* Configure layer 1 */
    layerConfig.framebufferAddress = framebuffer2;
    layerConfig.alpha = 128; /* Semi-transparent */
    status = LTDC_ConfigureLayer(&ltdcDriver, 1, &layerConfig);
    if (status != HAL_OK) {
        LTDC_PrintMessage("Error: Layer 1 configuration failed");
        return status;
    }

    /* Enable layer 0 */
    LTDC_EnableLayer(&ltdcDriver, 0);

    /* Turn on display */
    LTDC_DisplayOn(&ltdcDriver);

    examplesInitialized = true;
    LTDC_PrintMessage("LTDC Examples initialized successfully");

    return HAL_OK;
}

/**
 * @brief Basic LTDC initialization and display example
 * @details Demonstrates basic LTDC setup and simple display operations
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_BasicExample(void) {
    if (LTDC_ValidateExample() != HAL_OK) {
        return HAL_ERROR;
    }

    LTDC_PrintMessage("=== Basic LTDC Example ===");

    /* Clear screen with different colors */
    LTDC_PrintMessage("Clearing screen with red...");
    LTDC_ClearFramebuffer(&ltdcDriver, 0, LTDC_COLOR_RED);
    LTDC_Delay(EXAMPLE_DELAY_MS);

    LTDC_PrintMessage("Clearing screen with green...");
    LTDC_ClearFramebuffer(&ltdcDriver, 0, LTDC_COLOR_GREEN);
    LTDC_Delay(EXAMPLE_DELAY_MS);

    LTDC_PrintMessage("Clearing screen with blue...");
    LTDC_ClearFramebuffer(&ltdcDriver, 0, LTDC_COLOR_BLUE);
    LTDC_Delay(EXAMPLE_DELAY_MS);

    /* Draw pixels */
    LTDC_PrintMessage("Drawing random pixels...");
    LTDC_ClearFramebuffer(&ltdcDriver, 0, LTDC_COLOR_BLACK);

    for (int i = 0; i < 1000; i++) {
        uint16_t x = rand() % LTDC_DISPLAY_WIDTH;
        uint16_t y = rand() % LTDC_DISPLAY_HEIGHT;
        uint32_t color = rand() & 0xFFFF; /* Random RGB565 color */
        LTDC_DrawPixel(&ltdcDriver, x, y, color);
    }
    LTDC_Delay(EXAMPLE_DELAY_MS);

    LTDC_PrintMessage("Basic example completed");
    return HAL_OK;
}

/**
 * @brief Layer management example
 * @details Demonstrates layer configuration, enabling/disabling, and blending
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_LayerExample(void) {
    if (LTDC_ValidateExample() != HAL_OK) {
        return HAL_ERROR;
    }

    LTDC_PrintMessage("=== Layer Management Example ===");

    /* Set up different colors on each layer */
    LTDC_PrintMessage("Setting up layer backgrounds...");
    LTDC_ClearFramebuffer(&ltdcDriver, 0, LTDC_COLOR_RED);
    LTDC_ClearFramebuffer(&ltdcDriver, 1, LTDC_COLOR_BLUE);

    /* Show only layer 0 */
    LTDC_PrintMessage("Showing only layer 0 (red)...");
    LTDC_EnableLayer(&ltdcDriver, 0);
    LTDC_DisableLayer(&ltdcDriver, 1);
    LTDC_Delay(EXAMPLE_DELAY_MS);

    /* Show only layer 1 */
    LTDC_PrintMessage("Showing only layer 1 (blue)...");
    LTDC_DisableLayer(&ltdcDriver, 0);
    LTDC_EnableLayer(&ltdcDriver, 1);
    LTDC_Delay(EXAMPLE_DELAY_MS);

    /* Show both layers with different alpha values */
    LTDC_PrintMessage("Blending both layers...");
    LTDC_EnableLayer(&ltdcDriver, 0);
    LTDC_EnableLayer(&ltdcDriver, 1);

    /* Fade layer 1 from opaque to transparent */
    for (uint8_t alpha = 255; alpha > 0; alpha -= 10) {
        LTDC_SetLayerAlpha(&ltdcDriver, 1, alpha);
        LTDC_Delay(100);
    }

    /* Fade layer 1 from transparent to opaque */
    for (uint8_t alpha = 0; alpha < 255; alpha += 10) {
        LTDC_SetLayerAlpha(&ltdcDriver, 1, alpha);
        LTDC_Delay(100);
    }

    /* Test layer positioning */
    LTDC_PrintMessage("Testing layer positioning...");
    LTDC_Rect_t window = {60, 80, 120, 160};
    LTDC_SetLayerWindow(&ltdcDriver, 1, &window);
    LTDC_Delay(EXAMPLE_DELAY_MS);

    /* Reset to full screen */
    window.x = 0;
    window.y = 0;
    window.width = LTDC_DISPLAY_WIDTH;
    window.height = LTDC_DISPLAY_HEIGHT;
    LTDC_SetLayerWindow(&ltdcDriver, 1, &window);

    LTDC_PrintMessage("Layer management example completed");
    return HAL_OK;
}

/**
 * @brief Drawing functions example
 * @details Demonstrates various drawing primitives (pixels, lines, shapes)
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_DrawingExample(void) {
    /* Simplified: complex drawing demos removed to keep examples minimal.
       Use application-level drawing or a GUI library for shapes/lines. */
    if (LTDC_ValidateExample() != HAL_OK) return HAL_ERROR;
    LTDC_PrintMessage("Drawing example omitted in simplified build");
    return HAL_OK;
}

/**
 * @brief Color and pixel format example
 * @details Demonstrates different pixel formats and color conversions
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_ColorFormatExample(void) {
    if (LTDC_ValidateExample() != HAL_OK) return HAL_ERROR;
    LTDC_PrintMessage("=== Color Format Example (simplified) ===");

    // Minimal: draw three vertical color bars using LTDC_DrawPixel
    int barWidth = LTDC_DISPLAY_WIDTH / 3;
    uint32_t colors[] = {LTDC_COLOR_RED, LTDC_COLOR_GREEN, LTDC_COLOR_BLUE};
    for (int i = 0; i < 3; i++) {
        for (uint16_t y = 0; y < LTDC_DISPLAY_HEIGHT; ++y) {
            for (uint16_t x = i * barWidth; x < (i + 1) * barWidth; ++x) {
                LTDC_DrawPixel(&ltdcDriver, x, y, colors[i]);
            }
        }
    }

    return HAL_OK;
}

/**
 * @brief Framebuffer management example
 * @details Demonstrates framebuffer operations and memory management
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_FramebufferExample(void) {
    if (LTDC_ValidateExample() != HAL_OK) {
        return HAL_ERROR;
    }

    LTDC_PrintMessage("=== Framebuffer Management Example ===");

    /* Test framebuffer switching */
    LTDC_PrintMessage("Testing framebuffer switching...");

    /* Draw different patterns on each framebuffer */
    LTDC_SetFramebuffer(&ltdcDriver, 0, framebuffer1);
    LTDC_ClearFramebuffer(&ltdcDriver, 0, LTDC_COLOR_RED);
    // Minimal: draw a small white square
    for (uint16_t y = 10; y < 30; ++y) {
        for (uint16_t x = 10; x < 30; ++x) {
            LTDC_DrawPixel(&ltdcDriver, x, y, LTDC_COLOR_WHITE);
        }
    }

    LTDC_SetFramebuffer(&ltdcDriver, 0, framebuffer2);
    LTDC_ClearFramebuffer(&ltdcDriver, 0, LTDC_COLOR_BLUE);
    // Minimal: draw a small green square
    for (uint16_t y = 40; y < 60; ++y) {
        for (uint16_t x = 40; x < 60; ++x) {
            LTDC_DrawPixel(&ltdcDriver, x, y, LTDC_COLOR_GREEN);
        }
    }

    /* Switch between framebuffers once to verify swapping */
    LTDC_SetFramebuffer(&ltdcDriver, 0, framebuffer1);
    LTDC_PrintMessage("Switched to framebuffer 1");
    LTDC_Delay(500);

    LTDC_SetFramebuffer(&ltdcDriver, 0, framebuffer2);
    LTDC_PrintMessage("Switched to framebuffer 2");
    LTDC_Delay(500);

    /* Test framebuffer copy */
    LTDC_PrintMessage("Testing framebuffer copy...");
    LTDC_SetFramebuffer(&ltdcDriver, 1, framebuffer2);
    LTDC_EnableLayer(&ltdcDriver, 1);
    LTDC_SetLayerAlpha(&ltdcDriver, 1, 128);

    /* Copy framebuffer 1 to framebuffer 2 */
    LTDC_CopyFramebuffer(&ltdcDriver, 0, 1);
    LTDC_Delay(EXAMPLE_DELAY_MS);

    LTDC_DisableLayer(&ltdcDriver, 1);

    LTDC_PrintMessage("Framebuffer management example completed");
    return HAL_OK;
}

/**
 * @brief Animation and graphics example
 * @details Demonstrates animated graphics and advanced drawing techniques
 * @return HAL_StatusTypeDef: HAL status
 */


/**
 * @brief Performance test example
 * @details Performance testing and optimization techniques
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_PerformanceExample(void) {
    if (LTDC_ValidateExample() != HAL_OK) return HAL_ERROR;
    LTDC_PrintMessage("Performance example reduced: basic clear/pixel check only");

    /* Quick sanity check */
    LTDC_ClearFramebuffer(&ltdcDriver, 0, LTDC_COLOR_BLACK);
    for (int i = 0; i < 100; i++) {
        uint16_t x = i % LTDC_DISPLAY_WIDTH;
        uint16_t y = (i / LTDC_DISPLAY_WIDTH) % LTDC_DISPLAY_HEIGHT;
        LTDC_DrawPixel(&ltdcDriver, x, y, LTDC_COLOR_WHITE);
    }

    LTDC_PrintMessage("Performance quick check done");
    return HAL_OK;
}

/**
 * @brief Error handling example
 * @details Demonstrates error detection and recovery procedures
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_ErrorHandlingExample(void) {
    if (LTDC_ValidateExample() != HAL_OK) return HAL_ERROR;

    LTDC_PrintMessage("Error handling example: minimal checks");

    /* Invalid coordinate check */
    if (LTDC_DrawPixel(&ltdcDriver, LTDC_DISPLAY_WIDTH, 0, LTDC_COLOR_RED) != HAL_OK) {
        LTDC_PrintMessage("Invalid coordinate correctly detected");
        LTDC_ClearError(&ltdcDriver);
    }

    /* Invalid layer check */
    if (LTDC_EnableLayer(&ltdcDriver, LTDC_MAX_LAYERS + 1) != HAL_OK) {
        LTDC_PrintMessage("Invalid layer correctly detected");
        LTDC_ClearError(&ltdcDriver);
    }

    return HAL_OK;
}

/**
 * @brief Display help information
 * @details Shows available examples and usage instructions
 */
void LTDC_DisplayHelp(void) {
    LTDC_PrintMessage("=== LTDC Driver Examples Help ===");
    LTDC_PrintMessage("Available examples:");
    LTDC_PrintMessage("1. Basic Display        - LTDC_BasicExample()");
    LTDC_PrintMessage("2. Layer Management     - LTDC_LayerExample()");
    LTDC_PrintMessage("3. Color Formats        - LTDC_ColorFormatExample()");
    LTDC_PrintMessage("4. Framebuffer Ops      - LTDC_FramebufferExample()");
    LTDC_PrintMessage("5. Quick Checks         - LTDC_PerformanceExample() (reduced)");
    LTDC_PrintMessage("");
    LTDC_PrintMessage("Usage:");
    LTDC_PrintMessage("1. Call LTDC_ExamplesInit() first");
    LTDC_PrintMessage("2. Call any example function");
    LTDC_PrintMessage("3. Call LTDC_ExamplesCleanup() when done");
}

/**
 * @brief Wait for user input or delay
 * @details Helper function to pause execution
 */
void LTDC_WaitForInput(void) {
    LTDC_PrintMessage("Press any key to continue...");
    LTDC_Delay(2000); /* Simulate waiting for input */
}

/**
 * @brief Cleanup resources
 * @details Cleanup function for examples
 */
void LTDC_ExamplesCleanup(void) {
    if (examplesInitialized) {
        LTDC_DisplayOff(&ltdcDriver);

        /* Framebuffers are managed by fb_manager (no-op cleanup here) */
        framebuffer1 = 0;
        framebuffer2 = 0;

        LTDC_Driver_DeInit(&ltdcDriver);

        examplesInitialized = false;
        LTDC_PrintMessage("LTDC Examples cleanup completed");
    }
}

/**
 * @brief Allocate framebuffer memory
 * @details Allocates memory for framebuffer based on format
 * @param format: Pixel format
 * @return uint32_t: Framebuffer address (0 if failed)
 */
/* Framebuffer allocation/free are handled by fb_manager (no malloc/free here) */

/**
 * @brief Generate test pattern
 * @details Generates various test patterns for display testing
 * @param driver: LTDC driver handle
 * @param layer: Target layer
 * @param pattern: Pattern type (0-9)
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_GenerateTestPattern(LTDC_Driver_t *driver, uint8_t layer, uint8_t pattern) {
    if (driver == NULL || layer >= LTDC_MAX_LAYERS) {
        return HAL_ERROR;
    }

   LTDC_GenerateTestPattern(driver, layer, pattern);
    return HAL_OK;
}

/* Private Functions ---------------------------------------------------------*/

/**
 * @brief Print status message
 * @param message: Message to print
 */
static void LTDC_PrintMessage(const char* message) {
    /* In a real application, this would output to UART, LCD, or debug interface */
    printf("[LTDC]: %s\n", message);
}

/**
 * @brief Delay function
 * @param ms: Delay in milliseconds
 */
static void LTDC_Delay(uint32_t ms) {
    HAL_Delay(ms);
}

/**
 * @brief Validate example initialization
 * @return HAL_StatusTypeDef: HAL status
 */
static HAL_StatusTypeDef LTDC_ValidateExample(void) {
    if (!examplesInitialized) {
        LTDC_PrintMessage("Error: Examples not initialized. Call LTDC_ExamplesInit() first.");
        return HAL_ERROR;
    }
    return HAL_OK;
}

/* Simple drawing helper implementations (kept in example to avoid complicating driver) */
