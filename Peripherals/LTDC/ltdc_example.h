/**
 * @file ltdc_example.h
 * @brief LTDC driver example declarations for STM32F429 Discovery board
 * @details This file provides example implementations demonstrating how to use
 *          the LTDC driver for controlling the LCD display on the STM32F429 Discovery board.
 * @version 1.0
 * @date 2025-09-03
 * @author STM32 Team
 */

#ifndef LTDC_EXAMPLE_H
#define LTDC_EXAMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ltdc.h"

/* Function prototypes -------------------------------------------------------*/

/**
 * @brief Basic LTDC initialization and display example
 * @details Demonstrates basic LTDC setup and simple display operations
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_BasicExample(void);

/**
 * @brief Layer management example
 * @details Demonstrates layer configuration, enabling/disabling, and blending
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_LayerExample(void);

/**
 * @brief Drawing functions example
 * @details Demonstrates various drawing primitives (pixels, lines, shapes)
 * @return HAL_StatusTypeDef: HAL status
 */

/* Minimal example set: keep only the essential demos */
HAL_StatusTypeDef LTDC_ColorFormatExample(void);
HAL_StatusTypeDef LTDC_FramebufferExample(void);
HAL_StatusTypeDef LTDC_ErrorHandlingExample(void);

/* Helper function prototypes -----------------------------------------------*/

/**
 * @brief Initialize all LTDC examples
 * @details Sets up all necessary components for LTDC examples
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_ExamplesInit(void);

/**
 * @brief Display help information
 * @details Shows available examples and usage instructions
 */
void LTDC_DisplayHelp(void);

/**
 * @brief Wait for user input or delay
 * @details Helper function to pause execution
 */
void LTDC_WaitForInput(void);

/**
 * @brief Cleanup resources
 * @details Cleanup function for examples
 */
void LTDC_ExamplesCleanup(void);

/* Framebuffer allocation/free handled by fb_manager; not provided here. */

/**
 * @brief Generate test pattern
 * @details Generates various test patterns for display testing
 * @param driver: LTDC driver handle
 * @param layer: Target layer
 * @param pattern: Pattern type (0-9)
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_GenerateTestPattern(LTDC_Driver_t *driver, uint8_t layer, uint8_t pattern);

#ifdef __cplusplus
}
#endif

#endif /* LTDC_EXAMPLE_H */
