/**
 * @file dma2d.h
 * @brief DMA2D (Chrom-Art Accelerator) peripheral driver for STM32F4/F7 series
 * @author GitHub Copilot
 * @date 2025
 * @version 2.0.0
 *
 * @details
 * This driver provides a comprehensive, thread-safe interface for the DMA2D peripheral
 * on STM32 microcontrollers. It supports hardware-accelerated 2D graphics operations
 * including pixel format conversion, alpha blending, and area filling.
 *
 * Key Features:
 * - Hardware-accelerated 2D graphics operations
 * - Multiple color format support (ARGB8888, RGB888, RGB565, ARGB1555, ARGB4444)
 * - Alpha blending with configurable transparency
 * - Interrupt-driven and polling modes
 * - Comprehensive error handling and status reporting
 * - Thread-safe operations with optional mutex protection
 * - Configurable timeout handling
 * - Memory-efficient design for embedded systems
 *
 * Supported Operations:
 * - Register-to-Memory (R2M): Fill areas with solid colors
 * - Memory-to-Memory (M2M): Copy data between buffers
 * - Memory-to-Memory with PFC: Convert pixel formats during copy
 * - Memory-to-Memory with Blending: Alpha blend two images
 *
 * Usage Example:
 * @code
 * // Initialize DMA2D
 * DMA2D_Config config = {
 *     .mode = DMA2D_MODE_R2M,
 *     .color_mode = DMA2D_FORMAT_ARGB8888,
 *     .output_offset = 0,
 *     .red_value = 255,
 *     .green_value = 0,
 *     .blue_value = 0,
 *     .alpha_value = 255
 * };
 *
 * if (DMA2D_Init(&config) == HAL_OK) {
 *     // Fill a 320x240 area with red
 *     DMA2D_StartFill(DMA2D_COLOR_RED, framebuffer, 320, 240);
 * }
 * @endcode
 *
 * Thread Safety:
 * - All public functions are thread-safe when DMA2D_USE_MUTEX is defined
 * - Internal state is protected with mutex during multi-threaded access
 * - Callbacks are executed in interrupt context and should be kept short
 *
 * Error Handling:
 * - All functions return HAL_StatusTypeDef for comprehensive error reporting
 * - Use DMA2D_GetStatus() to retrieve detailed error information
 * - Error callbacks can be registered for asynchronous error handling
 *
 * Performance Considerations:
 * - DMA2D operates independently of CPU, enabling parallel processing
 * - Use interrupt mode for non-blocking operations
 * - Batch operations when possible to minimize setup overhead
 * - Consider SDRAM bandwidth limitations for large transfers
 */

#ifndef DMA2D_DRIVER_H
#define DMA2D_DRIVER_H

/* Includes */
#include "stm32f4xx.h"
#include "stm32f4xx_hal_dma2d.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Configuration Options */
/**
 * @def DMA2D_USE_MUTEX
 * @brief Enable thread safety with mutex protection
 * Define this macro to enable mutex-based thread safety for all DMA2D operations
 */
// #define DMA2D_USE_MUTEX

/**
 * @def DMA2D_ENABLE_DEBUG
 * @brief Enable debug output for development
 * Define this macro to enable debug printf statements
 */
// #define DMA2D_ENABLE_DEBUG

/**
 * @def DMA2D_DEFAULT_TIMEOUT
 * @brief Default timeout for DMA2D operations (milliseconds)
 */
#define DMA2D_DEFAULT_TIMEOUT        1000U

/**
 * @def DMA2D_MAX_WIDTH
 * @brief Maximum supported width for DMA2D operations
 */
#define DMA2D_MAX_WIDTH              2048U

/**
 * @def DMA2D_MAX_HEIGHT
 * @brief Maximum supported height for DMA2D operations
 */
#define DMA2D_MAX_HEIGHT             2048U

/**
 * @def DMA2D_MAX_OFFSET
 * @brief Maximum supported line offset
 */
#define DMA2D_MAX_OFFSET             2047U

/* DMA2D Color Format Definitions */
#define DMA2D_FORMAT_ARGB8888        DMA2D_OUTPUT_ARGB8888
#define DMA2D_FORMAT_RGB888          DMA2D_OUTPUT_RGB888
#define DMA2D_FORMAT_RGB565          DMA2D_OUTPUT_RGB565
#define DMA2D_FORMAT_ARGB1555        DMA2D_OUTPUT_ARGB1555
#define DMA2D_FORMAT_ARGB4444        DMA2D_OUTPUT_ARGB4444

/* DMA2D Input Color Formats (use HAL definitions) */
#ifndef DMA2D_INPUT_ARGB8888
#define DMA2D_INPUT_ARGB8888         DMA2D_INPUT_ARGB8888
#endif
#ifndef DMA2D_INPUT_RGB888
#define DMA2D_INPUT_RGB888           DMA2D_INPUT_RGB888
#endif
#ifndef DMA2D_INPUT_RGB565
#define DMA2D_INPUT_RGB565           DMA2D_INPUT_RGB565
#endif
#ifndef DMA2D_INPUT_ARGB1555
#define DMA2D_INPUT_ARGB1555         DMA2D_INPUT_ARGB1555
#endif
#ifndef DMA2D_INPUT_ARGB4444
#define DMA2D_INPUT_ARGB4444         DMA2D_INPUT_ARGB4444
#endif

/* DMA2D Operating Mode Definitions */
#define DMA2D_MODE_R2M               DMA2D_R2M                    /**< Register to Memory */
#define DMA2D_MODE_M2M               DMA2D_M2M                    /**< Memory to Memory */
#define DMA2D_MODE_M2M_PFC           DMA2D_M2M_PFC                /**< Memory to Memory with PFC */
#define DMA2D_MODE_M2M_BLEND         DMA2D_M2M_BLEND              /**< Memory to Memory with Blending */

/* DMA2D Alpha Mode Definitions */
#define DMA2D_ALPHA_NO_MODIF         DMA2D_NO_MODIF_ALPHA         /**< No alpha modification */
#define DMA2D_ALPHA_REPLACE          DMA2D_REPLACE_ALPHA          /**< Replace alpha */
#define DMA2D_ALPHA_COMBINE          DMA2D_COMBINE_ALPHA          /**< Combine alpha */

/* DMA2D Layer Definitions (use HAL definitions) */
#ifndef DMA2D_FOREGROUND_LAYER
#define DMA2D_FOREGROUND_LAYER       DMA2D_FOREGROUND_LAYER       /**< Foreground layer */
#endif
#ifndef DMA2D_BACKGROUND_LAYER
#define DMA2D_BACKGROUND_LAYER       DMA2D_BACKGROUND_LAYER       /**< Background layer */
#endif

/* DMA2D State Definitions */
#define DMA2D_STATE_RESET            HAL_DMA2D_STATE_RESET        /**< DMA2D not initialized */
#define DMA2D_STATE_READY            HAL_DMA2D_STATE_READY        /**< DMA2D initialized and ready */
#define DMA2D_STATE_BUSY             HAL_DMA2D_STATE_BUSY         /**< DMA2D transfer in progress */
#define DMA2D_STATE_TIMEOUT          HAL_DMA2D_STATE_TIMEOUT      /**< DMA2D timeout occurred */
#define DMA2D_STATE_ERROR            HAL_DMA2D_STATE_ERROR        /**< DMA2D error occurred */

/* Predefined Colors (ARGB8888 format) */
#define DMA2D_COLOR_RED              0xFFFF0000U                  /**< Pure red */
#define DMA2D_COLOR_GREEN            0xFF00FF00U                  /**< Pure green */
#define DMA2D_COLOR_BLUE             0xFF0000FFU                  /**< Pure blue */
#define DMA2D_COLOR_WHITE            0xFFFFFFFFU                  /**< White */
#define DMA2D_COLOR_BLACK            0xFF000000U                  /**< Black */
#define DMA2D_COLOR_YELLOW           0xFFFFFF00U                  /**< Yellow */
#define DMA2D_COLOR_CYAN             0xFF00FFFFU                  /**< Cyan */
#define DMA2D_COLOR_MAGENTA          0xFFFF00FFU                  /**< Magenta */
#define DMA2D_COLOR_TRANSPARENT      0x00000000U                  /**< Fully transparent */

/* DMA2D Status Structure */
typedef struct {
    bool initialized;                      /**< DMA2D initialization status */
    uint32_t last_error;                   /**< Last error code from HAL */
    uint32_t transfer_count;               /**< Number of successful transfers */
    uint32_t error_count;                  /**< Number of errors occurred */
    uint32_t state;                        /**< Current DMA2D state */
    uint32_t total_bytes_transferred;      /**< Total bytes transferred */
} DMA2D_Status;

/* DMA2D Configuration Structure */
typedef struct {
    uint32_t mode;                        /**< Operating mode (R2M, M2M, etc.) */
    uint32_t color_mode;                  /**< Output color format */
    uint32_t output_offset;               /**< Output line offset (pixels to skip per line) */
    uint32_t red_value;                   /**< Red component for R2M mode (0-255) */
    uint32_t green_value;                 /**< Green component for R2M mode (0-255) */
    uint32_t blue_value;                  /**< Blue component for R2M mode (0-255) */
    uint32_t alpha_value;                 /**< Alpha component for R2M mode (0-255) */
} DMA2D_Config;

/* DMA2D Layer Configuration Structure */
typedef struct {
    uint32_t input_color_mode;            /**< Input color format */
    uint32_t input_alpha_mode;            /**< Alpha mode for input */
    uint32_t input_alpha;                 /**< Alpha value for input (0-255) */
    uint32_t input_offset;                /**< Input line offset (pixels to skip per line) */
} DMA2D_LayerConfig;

/* DMA2D Rectangle Structure for area operations */
typedef struct {
    uint32_t x;                           /**< X coordinate of top-left corner */
    uint32_t y;                           /**< Y coordinate of top-left corner */
    uint32_t width;                       /**< Rectangle width in pixels */
    uint32_t height;                      /**< Rectangle height in pixels */
} DMA2D_Rectangle;

/* Function Pointer Types for Callbacks */
typedef void (*DMA2D_TransferCompleteCallback)(DMA2D_HandleTypeDef *hdma2d);
typedef void (*DMA2D_TransferErrorCallback)(DMA2D_HandleTypeDef *hdma2d);
typedef void (*DMA2D_TransferProgressCallback)(DMA2D_HandleTypeDef *hdma2d, uint32_t progress);

/* ============================================================================
 * Public API Functions
 * ============================================================================ */

/**
 * @brief Initialize the DMA2D peripheral with specified configuration
 * @param config Pointer to DMA2D configuration structure (must not be NULL)
 * @return HAL_StatusTypeDef
 *         - HAL_OK: Initialization successful
 *         - HAL_ERROR: Invalid parameters or initialization failed
 *         - HAL_BUSY: DMA2D peripheral is busy
 *         - HAL_TIMEOUT: Initialization timeout
 */
HAL_StatusTypeDef DMA2D_Init(const DMA2D_Config *config);

/**
 * @brief Deinitialize the DMA2D peripheral
 * @return HAL_StatusTypeDef
 *         - HAL_OK: Deinitialization successful
 *         - HAL_ERROR: Deinitialization failed
 */
HAL_StatusTypeDef DMA2D_DeInit(void);

/**
 * @brief Configure DMA2D layer parameters
 * @param layer Layer number (DMA2D_FOREGROUND_LAYER or DMA2D_BACKGROUND_LAYER)
 * @param layer_config Pointer to layer configuration structure (must not be NULL)
 * @return HAL_StatusTypeDef
 *         - HAL_OK: Layer configuration successful
 *         - HAL_ERROR: Invalid parameters
 *         - HAL_BUSY: DMA2D is busy
 */
HAL_StatusTypeDef DMA2D_ConfigLayer(uint32_t layer, const DMA2D_LayerConfig *layer_config);

/**
 * @brief Start DMA2D transfer operation (polling mode)
 * @param pSrc Pointer to source buffer (must not be NULL for M2M operations)
 * @param pDst Pointer to destination buffer (must not be NULL)
 * @param width Transfer width in pixels (1 to DMA2D_MAX_WIDTH)
 * @param height Transfer height in pixels (1 to DMA2D_MAX_HEIGHT)
 * @return HAL_StatusTypeDef
 *         - HAL_OK: Transfer started successfully
 *         - HAL_ERROR: Invalid parameters
 *         - HAL_BUSY: DMA2D is busy
 */
HAL_StatusTypeDef DMA2D_StartTransfer(const uint32_t *pSrc, uint32_t *pDst, uint32_t width, uint32_t height);

/**
 * @brief Start DMA2D register-to-memory fill operation (polling mode)
 * @param color Fill color value (ARGB8888 format)
 * @param pDst Pointer to destination buffer (must not be NULL)
 * @param width Fill width in pixels (1 to DMA2D_MAX_WIDTH)
 * @param height Fill height in pixels (1 to DMA2D_MAX_HEIGHT)
 * @return HAL_StatusTypeDef
 *         - HAL_OK: Fill operation started successfully
 *         - HAL_ERROR: Invalid parameters
 *         - HAL_BUSY: DMA2D is busy
 */
HAL_StatusTypeDef DMA2D_StartFill(uint32_t color, uint32_t *pDst, uint32_t width, uint32_t height);

/**
 * @brief Start DMA2D blending operation (polling mode)
 * @param pSrc1 Pointer to foreground buffer (must not be NULL)
 * @param pSrc2 Pointer to background buffer (must not be NULL)
 * @param pDst Pointer to destination buffer (must not be NULL)
 * @param width Blend width in pixels (1 to DMA2D_MAX_WIDTH)
 * @param height Blend height in pixels (1 to DMA2D_MAX_HEIGHT)
 * @return HAL_StatusTypeDef
 *         - HAL_OK: Blending operation started successfully
 *         - HAL_ERROR: Invalid parameters
 *         - HAL_BUSY: DMA2D is busy
 */
HAL_StatusTypeDef DMA2D_StartBlending(const uint32_t *pSrc1, const uint32_t *pSrc2, uint32_t *pDst, uint32_t width, uint32_t height);

/**
 * @brief Start DMA2D transfer operation (interrupt mode)
 * @param pSrc Pointer to source buffer (must not be NULL for M2M operations)
 * @param pDst Pointer to destination buffer (must not be NULL)
 * @param width Transfer width in pixels (1 to DMA2D_MAX_WIDTH)
 * @param height Transfer height in pixels (1 to DMA2D_MAX_HEIGHT)
 * @return HAL_StatusTypeDef
 *         - HAL_OK: Transfer started successfully
 *         - HAL_ERROR: Invalid parameters
 *         - HAL_BUSY: DMA2D is busy
 */
HAL_StatusTypeDef DMA2D_StartTransfer_IT(const uint32_t *pSrc, uint32_t *pDst, uint32_t width, uint32_t height);

/**
 * @brief Start DMA2D register-to-memory fill operation (interrupt mode)
 * @param color Fill color value (ARGB8888 format)
 * @param pDst Pointer to destination buffer (must not be NULL)
 * @param width Fill width in pixels (1 to DMA2D_MAX_WIDTH)
 * @param height Fill height in pixels (1 to DMA2D_MAX_HEIGHT)
 * @return HAL_StatusTypeDef
 *         - HAL_OK: Fill operation started successfully
 *         - HAL_ERROR: Invalid parameters
 *         - HAL_BUSY: DMA2D is busy
 */
HAL_StatusTypeDef DMA2D_StartFill_IT(uint32_t color, uint32_t *pDst, uint32_t width, uint32_t height);

/**
 * @brief Start DMA2D blending operation (interrupt mode)
 * @param pSrc1 Pointer to foreground buffer (must not be NULL)
 * @param pSrc2 Pointer to background buffer (must not be NULL)
 * @param pDst Pointer to destination buffer (must not be NULL)
 * @param width Blend width in pixels (1 to DMA2D_MAX_WIDTH)
 * @param height Blend height in pixels (1 to DMA2D_MAX_HEIGHT)
 * @return HAL_StatusTypeDef
 *         - HAL_OK: Blending operation started successfully
 *         - HAL_ERROR: Invalid parameters
 *         - HAL_BUSY: DMA2D is busy
 */
HAL_StatusTypeDef DMA2D_StartBlending_IT(const uint32_t *pSrc1, const uint32_t *pSrc2, uint32_t *pDst, uint32_t width, uint32_t height);

/**
 * @brief Poll for DMA2D transfer completion
 * @param timeout Timeout value in milliseconds (use DMA2D_DEFAULT_TIMEOUT for default)
 * @return HAL_StatusTypeDef
 *         - HAL_OK: Transfer completed successfully
 *         - HAL_ERROR: Transfer failed
 *         - HAL_BUSY: Transfer still in progress
 *         - HAL_TIMEOUT: Transfer timeout
 */
HAL_StatusTypeDef DMA2D_PollForTransfer(uint32_t timeout);

/**
 * @brief Abort ongoing DMA2D transfer
 * @return HAL_StatusTypeDef
 *         - HAL_OK: Abort successful
 *         - HAL_ERROR: Abort failed
 */
HAL_StatusTypeDef DMA2D_Abort(void);

/**
 * @brief Get DMA2D status information
 * @param status Pointer to status structure to fill (must not be NULL)
 * @return HAL_StatusTypeDef
 *         - HAL_OK: Status retrieved successfully
 *         - HAL_ERROR: Invalid parameters
 */
HAL_StatusTypeDef DMA2D_GetStatus(DMA2D_Status *status);

/**
 * @brief Check if DMA2D is busy
 * @return bool true if DMA2D is busy, false otherwise
 */
bool DMA2D_IsBusy(void);

/**
 * @brief Register transfer complete callback
 * @param callback Function pointer to callback (NULL to disable)
 * @note Callback is called from interrupt context, keep it short
 */
void DMA2D_RegisterTransferCompleteCallback(DMA2D_TransferCompleteCallback callback);

/**
 * @brief Register transfer error callback
 * @param callback Function pointer to callback (NULL to disable)
 * @note Callback is called from interrupt context, keep it short
 */
void DMA2D_RegisterTransferErrorCallback(DMA2D_TransferErrorCallback callback);

/**
 * @brief Register transfer progress callback (if supported)
 * @param callback Function pointer to callback (NULL to disable)
 * @note This feature may not be available on all STM32 series
 */
void DMA2D_RegisterTransferProgressCallback(DMA2D_TransferProgressCallback callback);

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/**
 * @brief Convert RGB888 color to ARGB8888 format
 * @param red Red component (0-255)
 * @param green Green component (0-255)
 * @param blue Blue component (0-255)
 * @param alpha Alpha component (0-255)
 * @return uint32_t ARGB8888 color value
 */
uint32_t DMA2D_MakeColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);

/**
 * @brief Extract color components from ARGB8888 color
 * @param color ARGB8888 color value
 * @param red Pointer to red component (can be NULL)
 * @param green Pointer to green component (can be NULL)
 * @param blue Pointer to blue component (can be NULL)
 * @param alpha Pointer to alpha component (can be NULL)
 */
void DMA2D_GetColorComponents(uint32_t color, uint8_t *red, uint8_t *green, uint8_t *blue, uint8_t *alpha);

/**
 * @brief Validate DMA2D configuration parameters
 * @param config Pointer to configuration structure
 * @return HAL_StatusTypeDef
 *         - HAL_OK: Configuration is valid
 *         - HAL_ERROR: Invalid configuration
 */
HAL_StatusTypeDef DMA2D_ValidateConfig(const DMA2D_Config *config);

/**
 * @brief Validate DMA2D layer configuration parameters
 * @param layer_config Pointer to layer configuration structure
 * @return HAL_StatusTypeDef
 *         - HAL_OK: Configuration is valid
 *         - HAL_ERROR: Invalid configuration
 */
HAL_StatusTypeDef DMA2D_ValidateLayerConfig(const DMA2D_LayerConfig *layer_config);

/**
 * @brief Get string representation of DMA2D error code
 * @param error_code HAL error code
 * @return const char* Error description string
 */
const char* DMA2D_GetErrorString(HAL_StatusTypeDef error_code);

/**
 * @brief Get string representation of DMA2D state
 * @param state DMA2D state
 * @return const char* State description string
 */
const char* DMA2D_GetStateString(uint32_t state);

/* ============================================================================
 * Advanced Features (Optional)
 * ============================================================================ */

/**
 * @brief Enable DMA2D hardware acceleration for LCD operations
 * @note This function configures DMA2D for optimal LCD framebuffer operations
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef DMA2D_EnableLCDMode(void);

/**
 * @brief Configure DMA2D for optimal SDRAM access
 * @note This function optimizes DMA2D settings for external SDRAM
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef DMA2D_EnableSDRAMMode(void);

/**
 * @brief Perform DMA2D self-test
 * @return HAL_StatusTypeDef
 *         - HAL_OK: Self-test passed
 *         - HAL_ERROR: Self-test failed
 */
HAL_StatusTypeDef DMA2D_SelfTest(void);

/* ============================================================================
 * Version Information
 * ============================================================================ */

/**
 * @brief Get DMA2D driver version
 * @return const char* Version string
 */
const char* DMA2D_GetVersion(void);

/**
 * @brief Get DMA2D driver capabilities
 * @return uint32_t Bitmask of supported features
 */
uint32_t DMA2D_GetCapabilities(void);

/* External DMA2D handle for interrupt handlers */
extern DMA2D_HandleTypeDef hdma2d;

#endif /* DMA2D_DRIVER_H */
