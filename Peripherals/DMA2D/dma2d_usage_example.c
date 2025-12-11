/**
 * @file dma2d_usage_example.c
 * @brief Example usage of the comprehensive DMA2D driver API
 * @author GitHub Copilot
 * @date 2025
 *
 * @details
 * This file demonstrates how to use the new comprehensive DMA2D driver
 * with best practices, error handling, and advanced features.
 */

#include "dma2d.h"
#include <stdio.h>
#include <string.h>

// Constants for color values and dimensions
#define COLOR_MAX_VALUE         255U
#define GRADIENT_STEP           50U
#define TEST_SIZE               50U
#define GRADIENT_CONSTANT       100U
#define COPY_SIZE               100U
#define GRADIENT_DIVISOR        199U

// Example callback functions
void transfer_complete_callback(DMA2D_HandleTypeDef *hdma2d) {
    printf("DMA2D transfer completed successfully!\n");
}

void transfer_error_callback(DMA2D_HandleTypeDef *hdma2d) {
    printf("DMA2D transfer error occurred!\n");
}

// Example framebuffer (assuming 320x240 ARGB8888 display)
#define FRAMEBUFFER_WIDTH  320
#define FRAMEBUFFER_HEIGHT 240
#define FRAMEBUFFER_SIZE   (FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT)
static uint32_t framebuffer[FRAMEBUFFER_SIZE];

/**
 * @brief Example 1: Basic initialization and fill operation
 */
void example_basic_fill(void) {
    printf("=== Example 1: Basic Fill Operation ===\n");

    // Initialize DMA2D
    DMA2D_Config config = {
        .mode = DMA2D_MODE_R2M,
        .color_mode = DMA2D_FORMAT_ARGB8888,
        .output_offset = 0
    };

    HAL_StatusTypeDef status = DMA2D_Init(&config);
    if (status != HAL_OK) {
        printf("Failed to initialize DMA2D: %s\n", DMA2D_GetErrorString(status));
        return;
    }

    // Create a red color
    uint32_t red_color = DMA2D_MakeColor(255, 0, 0, 255);

    // Fill the entire framebuffer with red
    status = DMA2D_StartFill(red_color, framebuffer, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
    if (status != HAL_OK) {
        printf("Fill operation failed: %s\n", DMA2D_GetErrorString(status));
    } else {
        printf("Fill operation completed successfully!\n");
    }

    // Get status information
    DMA2D_Status dma2d_status;
    DMA2D_GetStatus(&dma2d_status);
    printf("Total transfers: %u, Total bytes: %u\n",
           dma2d_status.transfer_count, dma2d_status.total_bytes_transferred);
}

/**
 * @brief Example 2: Memory-to-memory copy operation
 */
void example_copy_operation(void) {
    printf("\n=== Example 2: Copy Operation ===\n");

    // Create source data (a simple gradient)
    uint32_t source_data[100 * 100];
    for (int y = 0; y < 100; y++) {
        for (int x = 0; x < 100; x++) {
            uint8_t intensity = (x + y) * 255 / 199; // Simple gradient
            source_data[y * 100 + x] = DMA2D_MakeColor(intensity, intensity, intensity, 255);
        }
    }

    // Copy to framebuffer
    HAL_StatusTypeDef status = DMA2D_StartTransfer(source_data, framebuffer, 100, 100);
    if (status != HAL_OK) {
        printf("Copy operation failed: %s\n", DMA2D_GetErrorString(status));
    } else {
        printf("Copy operation completed successfully!\n");
    }
}

/**
 * @brief Example 3: Alpha blending operation
 */
void example_alpha_blending(void) {
    printf("\n=== Example 3: Alpha Blending ===\n");

    // Create foreground image (semi-transparent red rectangle)
    uint32_t foreground[50 * 50];
    uint32_t fg_color = DMA2D_MakeColor(255, 0, 0, 128); // Red with 50% alpha
    memset(foreground, fg_color, sizeof(foreground));

    // Create background image (solid blue)
    uint32_t background[50 * 50];
    uint32_t bg_color = DMA2D_MakeColor(0, 0, 255, 255); // Solid blue
    memset(background, bg_color, sizeof(background));

    // Result buffer
    uint32_t result[50 * 50];

    // Perform alpha blending
    HAL_StatusTypeDef status = DMA2D_StartBlending(foreground, background, result, 50, 50);
    if (status != HAL_OK) {
        printf("Blending operation failed: %s\n", DMA2D_GetErrorString(status));
    } else {
        printf("Blending operation completed successfully!\n");

        // Extract color components from result
        uint8_t r, g, b, a;
        DMA2D_GetColorComponents(result[0], &r, &g, &b, &a);
        printf("Result color: R=%d, G=%d, B=%d, A=%d\n", r, g, b, a);
    }
}

/**
 * @brief Example 4: Interrupt-driven operation
 */
void example_interrupt_operation(void) {
    printf("\n=== Example 4: Interrupt-Driven Operation ===\n");

    // Register callbacks
    DMA2D_RegisterTransferCompleteCallback(transfer_complete_callback);
    DMA2D_RegisterTransferErrorCallback(transfer_error_callback);

    // Create a green color
    uint32_t green_color = DMA2D_MakeColor(0, 255, 0, 255);

    // Start interrupt-driven fill operation
    HAL_StatusTypeDef status = DMA2D_StartFill_IT(green_color, framebuffer, 100, 100);
    if (status != HAL_OK) {
        printf("Interrupt fill operation failed to start: %s\n", DMA2D_GetErrorString(status));
        return;
    }

    printf("Interrupt fill operation started...\n");

    // Wait for completion (in a real application, you would do other work here)
    while (DMA2D_IsBusy()) {
        // Could do other processing here
    }

    printf("Interrupt fill operation completed!\n");
}

/**
 * @brief Example 5: Layer configuration for advanced operations
 */
void example_layer_configuration(void) {
    printf("\n=== Example 5: Layer Configuration ===\n");

    // Configure foreground layer for blending with specific alpha
    DMA2D_LayerConfig fg_config = {
        .input_color_mode = DMA2D_INPUT_ARGB8888,
        .input_alpha_mode = DMA2D_ALPHA_REPLACE,
        .input_alpha = 200,  // 78% opacity
        .input_offset = 0
    };

    HAL_StatusTypeDef status = DMA2D_ConfigLayer(DMA2D_FOREGROUND_LAYER, &fg_config);
    if (status != HAL_OK) {
        printf("Failed to configure foreground layer: %s\n", DMA2D_GetErrorString(status));
        return;
    }

    // Configure background layer
    DMA2D_LayerConfig bg_config = {
        .input_color_mode = DMA2D_INPUT_ARGB8888,
        .input_alpha_mode = DMA2D_ALPHA_NO_MODIF,
        .input_alpha = 255,
        .input_offset = 0
    };

    status = DMA2D_ConfigLayer(DMA2D_BACKGROUND_LAYER, &bg_config);
    if (status != HAL_OK) {
        printf("Failed to configure background layer: %s\n", DMA2D_GetErrorString(status));
        return;
    }

    printf("Layers configured successfully!\n");
}

/**
 * @brief Example 6: Error handling and validation
 */
void example_error_handling(void) {
    printf("\n=== Example 6: Error Handling ===\n");

    // Try to start a transfer with invalid parameters
    HAL_StatusTypeDef status = DMA2D_StartTransfer(NULL, framebuffer, 100, 100);
    if (status != HAL_OK) {
        printf("Expected error with NULL source: %s\n", DMA2D_GetErrorString(status));
    }

    // Try to start another operation while busy (if supported)
    if (DMA2D_IsBusy()) {
        status = DMA2D_StartFill(DMA2D_COLOR_BLUE, framebuffer, 50, 50);
        if (status == HAL_BUSY) {
            printf("Correctly detected busy state\n");
        }
    }

    // Run self-test
    status = DMA2D_SelfTest();
    if (status == HAL_OK) {
        printf("Self-test passed!\n");
    } else {
        printf("Self-test failed: %s\n", DMA2D_GetErrorString(status));
    }
}

/**
 * @brief Example 7: Performance monitoring
 */
void example_performance_monitoring(void) {
    printf("\n=== Example 7: Performance Monitoring ===\n");

    // Clear statistics
    DMA2D_DeInit();
    DMA2D_Config perf_config = {
        .mode = DMA2D_MODE_R2M,
        .color_mode = DMA2D_FORMAT_ARGB8888,
        .output_offset = 0,
        .red_value = 0,
        .green_value = 0,
        .blue_value = 0,
        .alpha_value = 0
    };
    DMA2D_Init(&perf_config);

    // Perform several operations
    for (int i = 0; i < 5; i++) {
        uint32_t color = DMA2D_MakeColor(i * 50, i * 50, i * 50, 255);
        DMA2D_StartFill(color, framebuffer, 50, 50);
    }

    // Get final statistics
    DMA2D_Status status;
    DMA2D_GetStatus(&status);

    printf("Performance Statistics:\n");
    printf("  Total transfers: %u\n", status.transfer_count);
    printf("  Total bytes transferred: %u\n", status.total_bytes_transferred);
    printf("  Error count: %u\n", status.error_count);
    printf("  Current state: %s\n", DMA2D_GetStateString(status.state));
}

/**
 * @brief Main example function demonstrating all features
 */
void dma2d_usage_examples(void) {
    printf("DMA2D Comprehensive Driver Usage Examples\n");
    printf("==========================================\n");

    // Run all examples
    example_basic_fill();
    example_copy_operation();
    example_alpha_blending();
    example_interrupt_operation();
    example_layer_configuration();
    example_error_handling();
    example_performance_monitoring();

    // Cleanup
    DMA2D_DeInit();

    printf("\nAll examples completed!\n");
}

/* ============================================================================
 * Integration Example for STM32F429I-DISC1
 * ============================================================================ */

/**
 * @brief Initialize DMA2D for STM32F429I-DISC1 board
 */
void init_dma2d_for_discovery(void) {
    // Enable DMA2D clock (usually done in STM32CubeMX)
    // __HAL_RCC_DMA2D_CLK_ENABLE();

    // Initialize with default configuration
    DMA2D_Config config = {
        .mode = DMA2D_MODE_R2M,
        .color_mode = DMA2D_FORMAT_ARGB8888,
        .output_offset = 0
    };

    if (DMA2D_Init(&config) == HAL_OK) {
        printf("DMA2D initialized for STM32F429I-DISC1\n");
    }
}

/**
 * @brief Example: Clear the display with a gradient
 */
void clear_display_gradient(void) {
    // Create a simple gradient pattern
    for (int y = 0; y < FRAMEBUFFER_HEIGHT; y++) {
        uint32_t color = DMA2D_MakeColor(
            (y * 255) / FRAMEBUFFER_HEIGHT,  // Red increases with Y
            100,                             // Constant green
            (255 - (y * 255) / FRAMEBUFFER_HEIGHT), // Blue decreases with Y
            255                              // Full alpha
        );

        // Fill one line at a time
        DMA2D_StartFill(color, &framebuffer[y * FRAMEBUFFER_WIDTH], FRAMEBUFFER_WIDTH, 1);
    }

    printf("Display cleared with gradient pattern\n");
}
