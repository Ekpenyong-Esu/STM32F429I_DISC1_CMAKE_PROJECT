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
static void LTDC_DrawTestPattern(LTDC_Driver_t *driver, uint8_t layer, uint8_t patternType);

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

    /* Allocate framebuffers */
    framebuffer1 = LTDC_AllocateFramebuffer(LTDC_PIXEL_FORMAT_RGB565_ENUM);
    framebuffer2 = LTDC_AllocateFramebuffer(LTDC_PIXEL_FORMAT_RGB565_ENUM);

    if (framebuffer1 == 0 || framebuffer2 == 0) {
        LTDC_PrintMessage("Error: Framebuffer allocation failed");
        return HAL_ERROR;
    }

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
    if (LTDC_ValidateExample() != HAL_OK) {
        return HAL_ERROR;
    }

    LTDC_PrintMessage("=== Drawing Functions Example ===");

    /* Clear screen */
    LTDC_DisableLayer(&ltdcDriver, 1);
    LTDC_SetActiveLayer(&ltdcDriver, 0);
    LTDC_ClearFramebuffer(&ltdcDriver, 0, LTDC_COLOR_BLACK);

    /* Draw lines */
    LTDC_PrintMessage("Drawing lines...");
    LTDC_Point_t start = {10, 10};
    LTDC_Point_t end = {230, 50};
    LTDC_DrawLine(&ltdcDriver, start, end, LTDC_COLOR_RED);

    start.x = 10; start.y = 60;
    end.x = 230; end.y = 100;
    LTDC_DrawLine(&ltdcDriver, start, end, LTDC_COLOR_GREEN);

    start.x = 10; start.y = 110;
    end.x = 230; end.y = 150;
    LTDC_DrawLine(&ltdcDriver, start, end, LTDC_COLOR_BLUE);
    LTDC_Delay(EXAMPLE_DELAY_MS);

    /* Draw rectangles */
    LTDC_PrintMessage("Drawing rectangles...");
    LTDC_Rect_t rect = {20, 160, 60, 40};
    LTDC_DrawRectangle(&ltdcDriver, &rect, LTDC_COLOR_YELLOW, false);

    rect.x = 100;
    LTDC_DrawRectangle(&ltdcDriver, &rect, LTDC_COLOR_CYAN, true);

    rect.x = 180;
    LTDC_DrawRectangle(&ltdcDriver, &rect, LTDC_COLOR_MAGENTA, false);
    LTDC_Delay(EXAMPLE_DELAY_MS);

    /* Draw circles */
    LTDC_PrintMessage("Drawing circles...");
    LTDC_Point_t center = {60, 250};
    LTDC_DrawCircle(&ltdcDriver, center, 30, LTDC_COLOR_WHITE, false);

    center.x = 120;
    LTDC_DrawCircle(&ltdcDriver, center, 25, LTDC_COLOR_RED, true);

    center.x = 180;
    LTDC_DrawCircle(&ltdcDriver, center, 20, LTDC_COLOR_BLUE, false);
    LTDC_Delay(EXAMPLE_DELAY_MS);

    /* Draw complex pattern */
    LTDC_PrintMessage("Drawing complex pattern...");
    LTDC_ClearFramebuffer(&ltdcDriver, 0, LTDC_COLOR_BLACK);

    /* Draw grid pattern */
    for (int x = 0; x < LTDC_DISPLAY_WIDTH; x += 20) {
        start.x = x; start.y = 0;
        end.x = x; end.y = LTDC_DISPLAY_HEIGHT;
        LTDC_DrawLine(&ltdcDriver, start, end, LTDC_COLOR_DARKGRAY);
    }

    for (int y = 0; y < LTDC_DISPLAY_HEIGHT; y += 20) {
        start.x = 0; start.y = y;
        end.x = LTDC_DISPLAY_WIDTH; end.y = y;
        LTDC_DrawLine(&ltdcDriver, start, end, LTDC_COLOR_DARKGRAY);
    }

    /* Draw diagonal lines */
    for (int i = 0; i < 5; i++) {
        start.x = i * 50; start.y = 0;
        end.x = LTDC_DISPLAY_WIDTH; end.y = LTDC_DISPLAY_HEIGHT - i * 50;
        LTDC_DrawLine(&ltdcDriver, start, end, LTDC_COLOR_GREEN);
    }

    LTDC_Delay(EXAMPLE_DELAY_MS);

    LTDC_PrintMessage("Drawing functions example completed");
    return HAL_OK;
}

/**
 * @brief Color and pixel format example
 * @details Demonstrates different pixel formats and color conversions
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_ColorFormatExample(void) {
    if (LTDC_ValidateExample() != HAL_OK) {
        return HAL_ERROR;
    }

    LTDC_PrintMessage("=== Color Format Example ===");

    /* Test color conversions */
    LTDC_PrintMessage("Testing color conversions...");

    uint32_t testColors[] = {
        0xFF0000, /* Red */
        0x00FF00, /* Green */
        0x0000FF, /* Blue */
        0xFFFF00, /* Yellow */
        0xFF00FF, /* Magenta */
        0x00FFFF, /* Cyan */
        0xFFFFFF  /* White */
    };

    for (size_t i = 0; i < sizeof(testColors) / sizeof(testColors[0]); i++) {
        uint32_t rgb888 = testColors[i];
        uint32_t rgb565 = LTDC_RGB888_To_RGB565(rgb888);
        uint32_t backToRgb888 = LTDC_RGB565_To_RGB888(rgb565);

        char message[MAX_MESSAGE_LEN];
        snprintf(message, sizeof(message),
                "RGB888: 0x%06lX -> RGB565: 0x%04lX -> RGB888: 0x%06lX",
                (unsigned long)rgb888, (unsigned long)rgb565, (unsigned long)backToRgb888);
        LTDC_PrintMessage(message);

        /* Draw color bars */
        LTDC_Rect_t rect = {(uint16_t)(i * 30), 50, 25, 100};
        LTDC_DrawRectangle(&ltdcDriver, &rect, rgb565, true);
    }

    LTDC_Delay(EXAMPLE_DELAY_MS);

    /* Test gradient */
    LTDC_PrintMessage("Drawing gradient...");
    for (int x = 0; x < LTDC_DISPLAY_WIDTH; x++) {
        uint8_t intensity = (x * 255) / LTDC_DISPLAY_WIDTH;
        uint32_t color = (intensity << 16) | (intensity << 8) | intensity; /* Grayscale */
        uint32_t rgb565 = LTDC_RGB888_To_RGB565(color);

        for (int y = 180; y < 250; y++) {
            LTDC_DrawPixel(&ltdcDriver, x, y, rgb565);
        }
    }

    LTDC_Delay(EXAMPLE_DELAY_MS);

    LTDC_PrintMessage("Color format example completed");
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
    LTDC_DrawTestPattern(&ltdcDriver, 0, 1);

    LTDC_SetFramebuffer(&ltdcDriver, 0, framebuffer2);
    LTDC_ClearFramebuffer(&ltdcDriver, 0, LTDC_COLOR_BLUE);
    LTDC_DrawTestPattern(&ltdcDriver, 0, 2);

    /* Switch between framebuffers */
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            LTDC_SetFramebuffer(&ltdcDriver, 0, framebuffer1);
            LTDC_PrintMessage("Switched to framebuffer 1");
        } else {
            LTDC_SetFramebuffer(&ltdcDriver, 0, framebuffer2);
            LTDC_PrintMessage("Switched to framebuffer 2");
        }
        LTDC_Delay(500);
    }

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
HAL_StatusTypeDef LTDC_AnimationExample(void) {
    if (LTDC_ValidateExample() != HAL_OK) {
        return HAL_ERROR;
    }

    LTDC_PrintMessage("=== Animation Example ===");

    /* Bouncing ball animation */
    LTDC_PrintMessage("Bouncing ball animation...");

    int ballX = 50, ballY = 50;
    int ballVelX = 3, ballVelY = 2;
    int ballRadius = 15;

    for (int frame = 0; frame < 200; frame++) {
        /* Clear screen */
        LTDC_ClearFramebuffer(&ltdcDriver, 0, LTDC_COLOR_BLACK);

        /* Update ball position */
        ballX += ballVelX;
        ballY += ballVelY;

        /* Bounce off walls */
        if (ballX <= ballRadius || ballX >= (LTDC_DISPLAY_WIDTH - ballRadius)) {
            ballVelX = -ballVelX;
        }
        if (ballY <= ballRadius || ballY >= (LTDC_DISPLAY_HEIGHT - ballRadius)) {
            ballVelY = -ballVelY;
        }

        /* Draw ball */
        LTDC_Point_t center = {(uint16_t)ballX, (uint16_t)ballY};
        LTDC_DrawCircle(&ltdcDriver, center, ballRadius, LTDC_COLOR_WHITE, true);

        /* Draw trail */
        for (int i = 1; i <= 5; i++) {
            LTDC_Point_t trailCenter = {
                (uint16_t)(ballX - ballVelX * i),
                (uint16_t)(ballY - ballVelY * i)
            };
            uint32_t trailColor = LTDC_COLOR_GRAY >> i; /* Fading trail */
            LTDC_DrawCircle(&ltdcDriver, trailCenter, ballRadius - i, trailColor, true);
        }

        LTDC_Delay(ANIMATION_FRAME_DELAY);
    }

    /* Rotating lines animation */
    LTDC_PrintMessage("Rotating lines animation...");

    LTDC_Point_t centerPoint = {LTDC_DISPLAY_WIDTH / 2, LTDC_DISPLAY_HEIGHT / 2};

    for (int angle = 0; angle < 720; angle += 5) {
        LTDC_ClearFramebuffer(&ltdcDriver, 0, LTDC_COLOR_BLACK);

        /* Draw multiple rotating lines */
        for (int line = 0; line < 8; line++) {
            double rad = (angle + line * 45) * M_PI / 180.0;
            int length = 80;

            LTDC_Point_t end = {
                (uint16_t)(centerPoint.x + length * cos(rad)),
                (uint16_t)(centerPoint.y + length * sin(rad))
            };

            uint32_t lineColor = (line % 3 == 0) ? LTDC_COLOR_RED :
                                (line % 3 == 1) ? LTDC_COLOR_GREEN : LTDC_COLOR_BLUE;

            LTDC_DrawLine(&ltdcDriver, centerPoint, end, lineColor);
        }

        LTDC_Delay(ANIMATION_FRAME_DELAY);
    }

    LTDC_PrintMessage("Animation example completed");
    return HAL_OK;
}

/**
 * @brief Performance test example
 * @details Performance testing and optimization techniques
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_PerformanceExample(void) {
    if (LTDC_ValidateExample() != HAL_OK) {
        return HAL_ERROR;
    }

    LTDC_PrintMessage("=== Performance Test Example ===");

    /* Test pixel drawing speed */
    LTDC_PrintMessage("Testing pixel drawing performance...");

    uint32_t startTime = HAL_GetTick();
    for (int i = 0; i < 10000; i++) {
        uint16_t x = i % LTDC_DISPLAY_WIDTH;
        uint16_t y = (i / LTDC_DISPLAY_WIDTH) % LTDC_DISPLAY_HEIGHT;
        LTDC_DrawPixel(&ltdcDriver, x, y, LTDC_COLOR_WHITE);
    }
    uint32_t endTime = HAL_GetTick();

    char message[MAX_MESSAGE_LEN];
    snprintf(message, sizeof(message), "Drew 10000 pixels in %lu ms", (unsigned long)(endTime - startTime));
    LTDC_PrintMessage(message);

    /* Test line drawing speed */
    LTDC_PrintMessage("Testing line drawing performance...");
    LTDC_ClearFramebuffer(&ltdcDriver, 0, LTDC_COLOR_BLACK);

    startTime = HAL_GetTick();
    for (int i = 0; i < 1000; i++) {
        LTDC_Point_t start = {0, (uint16_t)(i % LTDC_DISPLAY_HEIGHT)};
        LTDC_Point_t end = {LTDC_DISPLAY_WIDTH - 1, (uint16_t)((i + 100) % LTDC_DISPLAY_HEIGHT)};
        LTDC_DrawLine(&ltdcDriver, start, end, LTDC_COLOR_GREEN);
    }
    endTime = HAL_GetTick();

    snprintf(message, sizeof(message), "Drew 1000 lines in %lu ms", (unsigned long)(endTime - startTime));
    LTDC_PrintMessage(message);

    /* Test framebuffer clear speed */
    LTDC_PrintMessage("Testing framebuffer clear performance...");

    startTime = HAL_GetTick();
    for (int i = 0; i < 100; i++) {
        LTDC_ClearFramebuffer(&ltdcDriver, 0, LTDC_COLOR_RED);
    }
    endTime = HAL_GetTick();

    snprintf(message, sizeof(message), "Cleared framebuffer 100 times in %lu ms", (unsigned long)(endTime - startTime));
    LTDC_PrintMessage(message);

    LTDC_PrintMessage("Performance test completed");
    return HAL_OK;
}

/**
 * @brief Error handling example
 * @details Demonstrates error detection and recovery procedures
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_ErrorHandlingExample(void) {
    if (LTDC_ValidateExample() != HAL_OK) {
        return HAL_ERROR;
    }

    LTDC_PrintMessage("=== Error Handling Example ===");

    /* Test invalid parameters */
    LTDC_PrintMessage("Testing invalid parameter handling...");

    HAL_StatusTypeDef status = LTDC_DrawPixel(&ltdcDriver, LTDC_DISPLAY_WIDTH + 10, 50, LTDC_COLOR_RED);
    if (status != HAL_OK) {
        LTDC_PrintMessage("✓ Invalid coordinate detected correctly");
        uint32_t error = LTDC_GetError(&ltdcDriver);
        char message[MAX_MESSAGE_LEN];
        snprintf(message, sizeof(message), "Error code: %lu", (unsigned long)error);
        LTDC_PrintMessage(message);
        LTDC_ClearError(&ltdcDriver);
    }

    /* Test invalid layer */
    status = LTDC_EnableLayer(&ltdcDriver, 5);
    if (status != HAL_OK) {
        LTDC_PrintMessage("✓ Invalid layer number detected correctly");
        uint32_t error = LTDC_GetError(&ltdcDriver);
        char message[MAX_MESSAGE_LEN];
        snprintf(message, sizeof(message), "Error code: %lu", (unsigned long)error);
        LTDC_PrintMessage(message);
        LTDC_ClearError(&ltdcDriver);
    }

    /* Test memory boundaries */
    LTDC_PrintMessage("Testing memory boundary checks...");
    LTDC_Rect_t invalidRect = {200, 250, 100, 100}; /* Extends beyond display */
    status = LTDC_DrawRectangle(&ltdcDriver, &invalidRect, LTDC_COLOR_BLUE, true);
    if (status != HAL_OK) {
        LTDC_PrintMessage("✓ Rectangle boundary check working");
    }

    LTDC_PrintMessage("Error handling example completed");
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
    LTDC_PrintMessage("3. Drawing Functions    - LTDC_DrawingExample()");
    LTDC_PrintMessage("4. Color Formats        - LTDC_ColorFormatExample()");
    LTDC_PrintMessage("5. Framebuffer Ops      - LTDC_FramebufferExample()");
    LTDC_PrintMessage("6. Animation            - LTDC_AnimationExample()");
    LTDC_PrintMessage("7. Performance Test     - LTDC_PerformanceExample()");
    LTDC_PrintMessage("8. Error Handling       - LTDC_ErrorHandlingExample()");
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

        if (framebuffer1 != 0) {
            LTDC_FreeFramebuffer(framebuffer1);
            framebuffer1 = 0;
        }

        if (framebuffer2 != 0) {
            LTDC_FreeFramebuffer(framebuffer2);
            framebuffer2 = 0;
        }

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
uint32_t LTDC_AllocateFramebuffer(LTDC_PixelFormat_t format) {
    uint32_t size;

    switch (format) {
        case LTDC_PIXEL_FORMAT_RGB565_ENUM:
            size = LTDC_FB_SIZE_RGB565;
            break;
        case LTDC_PIXEL_FORMAT_RGB888_ENUM:
            size = LTDC_FB_SIZE_RGB888;
            break;
        case LTDC_PIXEL_FORMAT_ARGB8888_ENUM:
            size = LTDC_FB_SIZE_ARGB8888;
            break;
        default:
            size = LTDC_FB_SIZE_RGB565;
            break;
    }

    /* In a real application, this should allocate from SDRAM or external memory */
    /* For simulation purposes, we'll use malloc (not recommended for embedded) */
    void *buffer = malloc(size);
    if (buffer == NULL) {
        return 0;
    }

    /* Clear allocated memory */
    memset(buffer, 0, size);

    return (uint32_t)buffer;
}

/**
 * @brief Free framebuffer memory
 * @details Frees previously allocated framebuffer memory
 * @param address: Framebuffer address
 */
void LTDC_FreeFramebuffer(uint32_t address) {
    if (address != 0) {
        free((void *)address);
    }
}

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

    LTDC_DrawTestPattern(driver, layer, pattern);
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

/**
 * @brief Draw test pattern
 * @param driver: LTDC driver handle
 * @param layer: Target layer
 * @param patternType: Pattern type
 */
static void LTDC_DrawTestPattern(LTDC_Driver_t *driver, uint8_t layer, uint8_t patternType) {
    LTDC_SetActiveLayer(driver, layer);

    switch (patternType) {
        case 1: /* Checkerboard */
            {
                for (int y = 0; y < LTDC_DISPLAY_HEIGHT; y += 20) {
                    for (int x = 0; x < LTDC_DISPLAY_WIDTH; x += 20) {
                        uint32_t color = ((x / 20) + (y / 20)) % 2 ? LTDC_COLOR_WHITE : LTDC_COLOR_BLACK;
                        LTDC_Rect_t rect = {(uint16_t)x, (uint16_t)y, 20, 20};
                        LTDC_DrawRectangle(driver, &rect, color, true);
                    }
                }
            }
            break;

        case 2: /* Color bars */
            {
                uint32_t colors[] = {LTDC_COLOR_RED, LTDC_COLOR_GREEN, LTDC_COLOR_BLUE,
                                   LTDC_COLOR_YELLOW, LTDC_COLOR_CYAN, LTDC_COLOR_MAGENTA, LTDC_COLOR_WHITE};
                int barWidth = LTDC_DISPLAY_WIDTH / 7;

                for (int i = 0; i < 7; i++) {
                    LTDC_Rect_t rect = {(uint16_t)(i * barWidth), 0, (uint16_t)barWidth, LTDC_DISPLAY_HEIGHT};
                    LTDC_DrawRectangle(driver, &rect, colors[i], true);
                }
            }
            break;

        default: /* Grid pattern */
            {
                LTDC_ClearFramebuffer(driver, layer, LTDC_COLOR_BLACK);

                /* Draw grid */
                for (int x = 0; x < LTDC_DISPLAY_WIDTH; x += 10) {
                    LTDC_Point_t start = {(uint16_t)x, 0};
                    LTDC_Point_t end = {(uint16_t)x, LTDC_DISPLAY_HEIGHT};
                    LTDC_DrawLine(driver, start, end, LTDC_COLOR_GRAY);
                }

                for (int y = 0; y < LTDC_DISPLAY_HEIGHT; y += 10) {
                    LTDC_Point_t start = {0, (uint16_t)y};
                    LTDC_Point_t end = {LTDC_DISPLAY_WIDTH, (uint16_t)y};
                    LTDC_DrawLine(driver, start, end, LTDC_COLOR_GRAY);
                }
            }
            break;
    }
}
