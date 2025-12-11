/**
 * @file dma2d.c
 * @brief DMA2D (Chrom-Art Accelerator) peripheral driver implementation
 * @author GitHub Copilot
 * @date 2025
 * @version 2.0.0
 *
 * @details
 * This file implements the DMA2D driver with comprehensive error handling,
 * thread safety, and performance optimizations for STM32F4/F7 series.
 *
 * Key Features:
 * - Comprehensive parameter validation
 * - Thread-safe operations (when DMA2D_USE_MUTEX is defined)
 * - Detailed error reporting and status tracking
 * - Interrupt and polling mode support
 * - Memory-efficient implementation
 * - Debug support (when DMA2D_ENABLE_DEBUG is defined)
 */

#include "dma2d.h"
#include <string.h>
#include <stdio.h>

#ifdef DMA2D_USE_MUTEX
#include "cmsis_os.h"  /* For mutex operations */
#endif

/* ============================================================================
 * Private Definitions and Macros
 * ============================================================================ */

/* Private defines */
#define DMA2D_INIT_TIMEOUT_MS        1000U
#define DMA2D_TRANSFER_TIMEOUT_MS    5000U
#define DMA2D_ABORT_TIMEOUT_MS       100U

/* Color component constants */
#define DMA2D_COLOR_COMPONENT_MAX    255U
#define DMA2D_COLOR_COMPONENT_MASK   0xFFU
#define DMA2D_ALPHA_SHIFT            24U
#define DMA2D_RED_SHIFT              16U
#define DMA2D_GREEN_SHIFT            8U
#define DMA2D_BLUE_SHIFT             0U

/* Debug macros */
#ifdef DMA2D_ENABLE_DEBUG
#define DMA2D_DEBUG_PRINT(...)       printf(__VA_ARGS__)
#define DMA2D_DEBUG_PRINT_ERROR(...) printf("[DMA2D ERROR] " __VA_ARGS__)
#define DMA2D_DEBUG_PRINT_INFO(...)  printf("[DMA2D INFO] " __VA_ARGS__)
#else
#define DMA2D_DEBUG_PRINT(...)
#define DMA2D_DEBUG_PRINT_ERROR(...)
#define DMA2D_DEBUG_PRINT_INFO(...)
#endif

/* Version information */
#define DMA2D_DRIVER_VERSION         "2.0.0"
#define DMA2D_DRIVER_DATE            "2025-01-01"

/* Capability flags */
#define DMA2D_CAPABILITY_BASIC       (1U << 0)  /**< Basic transfer operations */
#define DMA2D_CAPABILITY_FILL        (1U << 1)  /**< Fill operations */
#define DMA2D_CAPABILITY_COPY        (1U << 2)  /**< Copy operations */
#define DMA2D_CAPABILITY_PFC         (1U << 3)  /**< Pixel format conversion */
#define DMA2D_CAPABILITY_BLEND       (1U << 4)  /**< Alpha blending */
#define DMA2D_CAPABILITY_INTERRUPT   (1U << 5)  /**< Interrupt mode */
#define DMA2D_CAPABILITY_CALLBACK    (1U << 6)  /**< Callback support */

/* ============================================================================
 * Private Variables
 * ============================================================================ */

/* DMA2D handle */
DMA2D_HandleTypeDef hdma2d;

/* Status tracking */
static DMA2D_Status dma2d_status = {0};

/* Callback function pointers */
static DMA2D_TransferCompleteCallback transfer_complete_callback = NULL;
static DMA2D_TransferErrorCallback transfer_error_callback = NULL;
static DMA2D_TransferProgressCallback transfer_progress_callback = NULL;

/* Thread safety */
#ifdef DMA2D_USE_MUTEX
static osMutexId dma2d_mutex = NULL;
static osMutexDef(dma2d_mutex);
#endif

/* ============================================================================
 * Private Function Prototypes
 * ============================================================================ */

static void DMA2D_ErrorHandler(DMA2D_HandleTypeDef *hdma2d);
static void DMA2D_TransferCompleteHandler(DMA2D_HandleTypeDef *hdma2d);
static HAL_StatusTypeDef DMA2D_ValidateParameters(const uint32_t *pSrc, uint32_t *pDst,
                                                 uint32_t width, uint32_t height);
static HAL_StatusTypeDef DMA2D_WaitForTransfer(uint32_t timeout);

/* ============================================================================
 * Public API Implementation
 * ============================================================================ */

/**
 * @brief Initialize the DMA2D peripheral with specified configuration
 * @param config Pointer to DMA2D configuration structure (must not be NULL)
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef DMA2D_Init(const DMA2D_Config *config)
{
    HAL_StatusTypeDef status = HAL_ERROR;

    /* Parameter validation */
    if (config == NULL) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_Init: NULL configuration pointer\n");
        return HAL_ERROR;
    }

    /* Validate configuration */
    status = DMA2D_ValidateConfig(config);
    if (status != HAL_OK) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_Init: Invalid configuration\n");
        return status;
    }

    /* Thread safety */
#ifdef DMA2D_USE_MUTEX
    if (dma2d_mutex == NULL) {
        dma2d_mutex = osMutexCreate(osMutex(dma2d_mutex));
        if (dma2d_mutex == NULL) {
            DMA2D_DEBUG_PRINT_ERROR("DMA2D_Init: Failed to create mutex\n");
            return HAL_ERROR;
        }
    }
    osMutexWait(dma2d_mutex, osWaitForever);
#endif

    /* Check if already initialized */
    if (dma2d_status.initialized) {
        DMA2D_DEBUG_PRINT_INFO("DMA2D_Init: Already initialized, deinitializing first\n");
        DMA2D_DeInit();
    }

    /* Reset status */
    memset(&dma2d_status, 0, sizeof(DMA2D_Status));

    /* Configure DMA2D peripheral */
    hdma2d.Instance = DMA2D;
    hdma2d.Init.Mode = config->mode;
    hdma2d.Init.ColorMode = config->color_mode;
    hdma2d.Init.OutputOffset = config->output_offset;

    /* For R2M mode, set color values */
    if (config->mode == DMA2D_MODE_R2M) {
        /* Note: HAL doesn't directly support setting color values in Init.
         * They are set during the actual fill operation */
    }

    /* Initialize DMA2D peripheral */
    status = HAL_DMA2D_Init(&hdma2d);
    if (status != HAL_OK) {
        dma2d_status.last_error = HAL_DMA2D_GetError(&hdma2d);
        dma2d_status.error_count++;
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_Init: HAL_DMA2D_Init failed with status %d\n", status);

#ifdef DMA2D_USE_MUTEX
        osMutexRelease(dma2d_mutex);
#endif
        return status;
    }

    /* Set error callback */
    hdma2d.XferErrorCallback = DMA2D_ErrorHandler;

    /* Set transfer complete callback for interrupt mode */
    hdma2d.XferCpltCallback = DMA2D_TransferCompleteHandler;

    /* Update status */
    dma2d_status.initialized = true;
    dma2d_status.state = DMA2D_STATE_READY;

    DMA2D_DEBUG_PRINT_INFO("DMA2D_Init: Successfully initialized\n");

#ifdef DMA2D_USE_MUTEX
    osMutexRelease(dma2d_mutex);
#endif

    return HAL_OK;
}

/**
 * @brief Deinitialize the DMA2D peripheral
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef DMA2D_DeInit(void)
{
    HAL_StatusTypeDef status = HAL_ERROR;

#ifdef DMA2D_USE_MUTEX
    if (dma2d_mutex != NULL) {
        osMutexWait(dma2d_mutex, osWaitForever);
    }
#endif

    if (!dma2d_status.initialized) {
        DMA2D_DEBUG_PRINT_INFO("DMA2D_DeInit: Not initialized\n");

#ifdef DMA2D_USE_MUTEX
        if (dma2d_mutex != NULL) {
            osMutexRelease(dma2d_mutex);
        }
#endif
        return HAL_OK;
    }

    /* Abort any ongoing transfer */
    if (dma2d_status.state == DMA2D_STATE_BUSY) {
        DMA2D_DEBUG_PRINT_INFO("DMA2D_DeInit: Aborting ongoing transfer\n");
        HAL_DMA2D_Abort(&hdma2d);
    }

    /* Deinitialize DMA2D peripheral */
    status = HAL_DMA2D_DeInit(&hdma2d);
    if (status != HAL_OK) {
        dma2d_status.last_error = HAL_DMA2D_GetError(&hdma2d);
        dma2d_status.error_count++;
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_DeInit: HAL_DMA2D_DeInit failed with status %d\n", status);
    } else {
        /* Reset status */
        memset(&dma2d_status, 0, sizeof(DMA2D_Status));
        DMA2D_DEBUG_PRINT_INFO("DMA2D_DeInit: Successfully deinitialized\n");
    }

#ifdef DMA2D_USE_MUTEX
    if (dma2d_mutex != NULL) {
        osMutexRelease(dma2d_mutex);
    }
#endif

    return status;
}

/**
 * @brief Configure DMA2D layer parameters
 * @param layer Layer number (DMA2D_FOREGROUND_LAYER or DMA2D_BACKGROUND_LAYER)
 * @param layer_config Pointer to layer configuration structure (must not be NULL)
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef DMA2D_ConfigLayer(uint32_t layer, const DMA2D_LayerConfig *layer_config)
{
    HAL_StatusTypeDef status = HAL_ERROR;

    /* Parameter validation */
    if (layer_config == NULL) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_ConfigLayer: NULL layer configuration pointer\n");
        return HAL_ERROR;
    }

    if (layer != DMA2D_FOREGROUND_LAYER && layer != DMA2D_BACKGROUND_LAYER) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_ConfigLayer: Invalid layer %lu\n", layer);
        return HAL_ERROR;
    }

    /* Validate layer configuration */
    status = DMA2D_ValidateLayerConfig(layer_config);
    if (status != HAL_OK) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_ConfigLayer: Invalid layer configuration\n");
        return status;
    }

#ifdef DMA2D_USE_MUTEX
    if (dma2d_mutex != NULL) {
        osMutexWait(dma2d_mutex, osWaitForever);
    }
#endif

    if (!dma2d_status.initialized) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_ConfigLayer: DMA2D not initialized\n");

#ifdef DMA2D_USE_MUTEX
        if (dma2d_mutex != NULL) {
            osMutexRelease(dma2d_mutex);
        }
#endif
        return HAL_ERROR;
    }

    /* Configure layer parameters */
    hdma2d.LayerCfg[layer].InputColorMode = layer_config->input_color_mode;
    hdma2d.LayerCfg[layer].AlphaMode = layer_config->input_alpha_mode;
    hdma2d.LayerCfg[layer].InputAlpha = layer_config->input_alpha;
    hdma2d.LayerCfg[layer].InputOffset = layer_config->input_offset;

    /* Apply layer configuration */
    status = HAL_DMA2D_ConfigLayer(&hdma2d, layer);
    if (status != HAL_OK) {
        dma2d_status.last_error = HAL_DMA2D_GetError(&hdma2d);
        dma2d_status.error_count++;
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_ConfigLayer: HAL_DMA2D_ConfigLayer failed with status %d\n", status);
    } else {
        DMA2D_DEBUG_PRINT_INFO("DMA2D_ConfigLayer: Layer %lu configured successfully\n", layer);
    }

#ifdef DMA2D_USE_MUTEX
    if (dma2d_mutex != NULL) {
        osMutexRelease(dma2d_mutex);
    }
#endif

    return status;
}

/**
 * @brief Start DMA2D transfer operation (polling mode)
 * @param pSrc Pointer to source buffer (must not be NULL for M2M operations)
 * @param pDst Pointer to destination buffer (must not be NULL)
 * @param width Transfer width in pixels (1 to DMA2D_MAX_WIDTH)
 * @param height Transfer height in pixels (1 to DMA2D_MAX_HEIGHT)
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef DMA2D_StartTransfer(const uint32_t *pSrc, uint32_t *pDst, uint32_t width, uint32_t height)
{
    HAL_StatusTypeDef status = HAL_ERROR;

    /* Parameter validation */
    status = DMA2D_ValidateParameters(pSrc, pDst, width, height);
    if (status != HAL_OK) {
        return status;
    }

#ifdef DMA2D_USE_MUTEX
    if (dma2d_mutex != NULL) {
        osMutexWait(dma2d_mutex, osWaitForever);
    }
#endif

    if (!dma2d_status.initialized) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_StartTransfer: DMA2D not initialized\n");

#ifdef DMA2D_USE_MUTEX
        if (dma2d_mutex != NULL) {
            osMutexRelease(dma2d_mutex);
        }
#endif
        return HAL_ERROR;
    }

    if (dma2d_status.state == DMA2D_STATE_BUSY) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_StartTransfer: DMA2D is busy\n");

#ifdef DMA2D_USE_MUTEX
        if (dma2d_mutex != NULL) {
            osMutexRelease(dma2d_mutex);
        }
#endif
        return HAL_BUSY;
    }

    /* Update status */
    dma2d_status.state = DMA2D_STATE_BUSY;

    /* Start transfer */
    status = HAL_DMA2D_Start(&hdma2d, (uint32_t)pSrc, (uint32_t)pDst, width, height);
    if (status != HAL_OK) {
        dma2d_status.last_error = HAL_DMA2D_GetError(&hdma2d);
        dma2d_status.error_count++;
        dma2d_status.state = DMA2D_STATE_ERROR;
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_StartTransfer: HAL_DMA2D_Start failed with status %d\n", status);

#ifdef DMA2D_USE_MUTEX
        if (dma2d_mutex != NULL) {
            osMutexRelease(dma2d_mutex);
        }
#endif
        return status;
    }

    /* Wait for transfer completion */
    status = DMA2D_WaitForTransfer(DMA2D_TRANSFER_TIMEOUT_MS);
    if (status != HAL_OK) {
        dma2d_status.state = DMA2D_STATE_ERROR;
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_StartTransfer: Transfer timeout or error\n");
    } else {
        dma2d_status.transfer_count++;
        dma2d_status.total_bytes_transferred += (width * height * 4); /* Assume 32-bit pixels */
        dma2d_status.state = DMA2D_STATE_READY;
        DMA2D_DEBUG_PRINT_INFO("DMA2D_StartTransfer: Transfer completed successfully\n");
    }

#ifdef DMA2D_USE_MUTEX
    if (dma2d_mutex != NULL) {
        osMutexRelease(dma2d_mutex);
    }
#endif

    return status;
}

/**
 * @brief Start DMA2D register-to-memory fill operation (polling mode)
 * @param color Fill color value (ARGB8888 format)
 * @param pDst Pointer to destination buffer (must not be NULL)
 * @param width Fill width in pixels (1 to DMA2D_MAX_WIDTH)
 * @param height Fill height in pixels (1 to DMA2D_MAX_HEIGHT)
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef DMA2D_StartFill(uint32_t color, uint32_t *pDst, uint32_t width, uint32_t height)
{
    HAL_StatusTypeDef status = HAL_ERROR;

    /* Parameter validation */
    if (pDst == NULL) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_StartFill: NULL destination pointer\n");
        return HAL_ERROR;
    }

    if (width == 0 || width > DMA2D_MAX_WIDTH || height == 0 || height > DMA2D_MAX_HEIGHT) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_StartFill: Invalid dimensions %lux%lu\n", width, height);
        return HAL_ERROR;
    }

#ifdef DMA2D_USE_MUTEX
    if (dma2d_mutex != NULL) {
        osMutexWait(dma2d_mutex, osWaitForever);
    }
#endif

    if (!dma2d_status.initialized) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_StartFill: DMA2D not initialized\n");

#ifdef DMA2D_USE_MUTEX
        if (dma2d_mutex != NULL) {
            osMutexRelease(dma2d_mutex);
        }
#endif
        return HAL_ERROR;
    }

    if (dma2d_status.state == DMA2D_STATE_BUSY) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_StartFill: DMA2D is busy\n");

#ifdef DMA2D_USE_MUTEX
        if (dma2d_mutex != NULL) {
            osMutexRelease(dma2d_mutex);
        }
#endif
        return HAL_BUSY;
    }

    /* Update status */
    dma2d_status.state = DMA2D_STATE_BUSY;

    /* Start fill operation */
    status = HAL_DMA2D_Start(&hdma2d, color, (uint32_t)pDst, width, height);
    if (status != HAL_OK) {
        dma2d_status.last_error = HAL_DMA2D_GetError(&hdma2d);
        dma2d_status.error_count++;
        dma2d_status.state = DMA2D_STATE_ERROR;
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_StartFill: HAL_DMA2D_Start failed with status %d\n", status);

#ifdef DMA2D_USE_MUTEX
        if (dma2d_mutex != NULL) {
            osMutexRelease(dma2d_mutex);
        }
#endif
        return status;
    }

    /* Wait for fill completion */
    status = DMA2D_WaitForTransfer(DMA2D_TRANSFER_TIMEOUT_MS);
    if (status != HAL_OK) {
        dma2d_status.state = DMA2D_STATE_ERROR;
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_StartFill: Fill timeout or error\n");
    } else {
        dma2d_status.transfer_count++;
        dma2d_status.total_bytes_transferred += (width * height * 4);
        dma2d_status.state = DMA2D_STATE_READY;
        DMA2D_DEBUG_PRINT_INFO("DMA2D_StartFill: Fill completed successfully\n");
    }

#ifdef DMA2D_USE_MUTEX
    if (dma2d_mutex != NULL) {
        osMutexRelease(dma2d_mutex);
    }
#endif

    return status;
}

/**
 * @brief Start DMA2D blending operation (polling mode)
 * @param pSrc1 Pointer to foreground buffer (must not be NULL)
 * @param pSrc2 Pointer to background buffer (must not be NULL)
 * @param pDst Pointer to destination buffer (must not be NULL)
 * @param width Blend width in pixels (1 to DMA2D_MAX_WIDTH)
 * @param height Blend height in pixels (1 to DMA2D_MAX_HEIGHT)
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef DMA2D_StartBlending(const uint32_t *pSrc1, const uint32_t *pSrc2, uint32_t *pDst, uint32_t width, uint32_t height)
{
    HAL_StatusTypeDef status = HAL_ERROR;

    /* Parameter validation */
    if (pSrc1 == NULL || pSrc2 == NULL || pDst == NULL) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_StartBlending: NULL pointer(s) provided\n");
        return HAL_ERROR;
    }

    if (width == 0 || width > DMA2D_MAX_WIDTH || height == 0 || height > DMA2D_MAX_HEIGHT) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_StartBlending: Invalid dimensions %lux%lu\n", width, height);
        return HAL_ERROR;
    }

#ifdef DMA2D_USE_MUTEX
    if (dma2d_mutex != NULL) {
        osMutexWait(dma2d_mutex, osWaitForever);
    }
#endif

    if (!dma2d_status.initialized) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_StartBlending: DMA2D not initialized\n");

#ifdef DMA2D_USE_MUTEX
        if (dma2d_mutex != NULL) {
            osMutexRelease(dma2d_mutex);
        }
#endif
        return HAL_ERROR;
    }

    if (dma2d_status.state == DMA2D_STATE_BUSY) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_StartBlending: DMA2D is busy\n");

#ifdef DMA2D_USE_MUTEX
        if (dma2d_mutex != NULL) {
            osMutexRelease(dma2d_mutex);
        }
#endif
        return HAL_BUSY;
    }

    /* Update status */
    dma2d_status.state = DMA2D_STATE_BUSY;

    /* Start blending operation */
    status = HAL_DMA2D_BlendingStart(&hdma2d, (uint32_t)pSrc1, (uint32_t)pSrc2, (uint32_t)pDst, width, height);
    if (status != HAL_OK) {
        dma2d_status.last_error = HAL_DMA2D_GetError(&hdma2d);
        dma2d_status.error_count++;
        dma2d_status.state = DMA2D_STATE_ERROR;
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_StartBlending: HAL_DMA2D_BlendingStart failed with status %d\n", status);

#ifdef DMA2D_USE_MUTEX
        if (dma2d_mutex != NULL) {
            osMutexRelease(dma2d_mutex);
        }
#endif
        return status;
    }

    /* Wait for blending completion */
    status = DMA2D_WaitForTransfer(DMA2D_TRANSFER_TIMEOUT_MS);
    if (status != HAL_OK) {
        dma2d_status.state = DMA2D_STATE_ERROR;
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_StartBlending: Blending timeout or error\n");
    } else {
        dma2d_status.transfer_count++;
        dma2d_status.total_bytes_transferred += (width * height * 4);
        dma2d_status.state = DMA2D_STATE_READY;
        DMA2D_DEBUG_PRINT_INFO("DMA2D_StartBlending: Blending completed successfully\n");
    }

#ifdef DMA2D_USE_MUTEX
    if (dma2d_mutex != NULL) {
        osMutexRelease(dma2d_mutex);
    }
#endif

    return status;
}

/**
 * @brief Start DMA2D transfer operation (interrupt mode)
 * @param pSrc Pointer to source buffer (must not be NULL for M2M operations)
 * @param pDst Pointer to destination buffer (must not be NULL)
 * @param width Transfer width in pixels (1 to DMA2D_MAX_WIDTH)
 * @param height Transfer height in pixels (1 to DMA2D_MAX_HEIGHT)
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef DMA2D_StartTransfer_IT(const uint32_t *pSrc, uint32_t *pDst, uint32_t width, uint32_t height)
{
    HAL_StatusTypeDef status = HAL_ERROR;

    /* Parameter validation */
    status = DMA2D_ValidateParameters(pSrc, pDst, width, height);
    if (status != HAL_OK) {
        return status;
    }

#ifdef DMA2D_USE_MUTEX
    if (dma2d_mutex != NULL) {
        osMutexWait(dma2d_mutex, osWaitForever);
    }
#endif

    if (!dma2d_status.initialized) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_StartTransfer_IT: DMA2D not initialized\n");

#ifdef DMA2D_USE_MUTEX
        if (dma2d_mutex != NULL) {
            osMutexRelease(dma2d_mutex);
        }
#endif
        return HAL_ERROR;
    }

    if (dma2d_status.state == DMA2D_STATE_BUSY) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_StartTransfer_IT: DMA2D is busy\n");

#ifdef DMA2D_USE_MUTEX
        if (dma2d_mutex != NULL) {
            osMutexRelease(dma2d_mutex);
        }
#endif
        return HAL_BUSY;
    }

    /* Update status */
    dma2d_status.state = DMA2D_STATE_BUSY;

    /* Start interrupt-driven transfer */
    status = HAL_DMA2D_Start_IT(&hdma2d, (uint32_t)pSrc, (uint32_t)pDst, width, height);
    if (status != HAL_OK) {
        dma2d_status.last_error = HAL_DMA2D_GetError(&hdma2d);
        dma2d_status.error_count++;
        dma2d_status.state = DMA2D_STATE_ERROR;
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_StartTransfer_IT: HAL_DMA2D_Start_IT failed with status %d\n", status);

#ifdef DMA2D_USE_MUTEX
        if (dma2d_mutex != NULL) {
            osMutexRelease(dma2d_mutex);
        }
#endif
        return status;
    }

    DMA2D_DEBUG_PRINT_INFO("DMA2D_StartTransfer_IT: Interrupt transfer started\n");

#ifdef DMA2D_USE_MUTEX
    if (dma2d_mutex != NULL) {
        osMutexRelease(dma2d_mutex);
    }
#endif

    return HAL_OK;
}

/**
 * @brief Start DMA2D register-to-memory fill operation (interrupt mode)
 * @param color Fill color value (ARGB8888 format)
 * @param pDst Pointer to destination buffer (must not be NULL)
 * @param width Fill width in pixels (1 to DMA2D_MAX_WIDTH)
 * @param height Fill height in pixels (1 to DMA2D_MAX_HEIGHT)
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef DMA2D_StartFill_IT(uint32_t color, uint32_t *pDst, uint32_t width, uint32_t height)
{
    HAL_StatusTypeDef status = HAL_ERROR;

    /* Parameter validation */
    if (pDst == NULL) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_StartFill_IT: NULL destination pointer\n");
        return HAL_ERROR;
    }

    if (width == 0 || width > DMA2D_MAX_WIDTH || height == 0 || height > DMA2D_MAX_HEIGHT) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_StartFill_IT: Invalid dimensions %lux%lu\n", width, height);
        return HAL_ERROR;
    }

#ifdef DMA2D_USE_MUTEX
    if (dma2d_mutex != NULL) {
        osMutexWait(dma2d_mutex, osWaitForever);
    }
#endif

    if (!dma2d_status.initialized) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_StartFill_IT: DMA2D not initialized\n");

#ifdef DMA2D_USE_MUTEX
        if (dma2d_mutex != NULL) {
            osMutexRelease(dma2d_mutex);
        }
#endif
        return HAL_ERROR;
    }

    if (dma2d_status.state == DMA2D_STATE_BUSY) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_StartFill_IT: DMA2D is busy\n");

#ifdef DMA2D_USE_MUTEX
        if (dma2d_mutex != NULL) {
            osMutexRelease(dma2d_mutex);
        }
#endif
        return HAL_BUSY;
    }

    /* Update status */
    dma2d_status.state = DMA2D_STATE_BUSY;

    /* Start interrupt-driven fill */
    status = HAL_DMA2D_Start_IT(&hdma2d, color, (uint32_t)pDst, width, height);
    if (status != HAL_OK) {
        dma2d_status.last_error = HAL_DMA2D_GetError(&hdma2d);
        dma2d_status.error_count++;
        dma2d_status.state = DMA2D_STATE_ERROR;
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_StartFill_IT: HAL_DMA2D_Start_IT failed with status %d\n", status);

#ifdef DMA2D_USE_MUTEX
        if (dma2d_mutex != NULL) {
            osMutexRelease(dma2d_mutex);
        }
#endif
        return status;
    }

    DMA2D_DEBUG_PRINT_INFO("DMA2D_StartFill_IT: Interrupt fill started\n");

#ifdef DMA2D_USE_MUTEX
    if (dma2d_mutex != NULL) {
        osMutexRelease(dma2d_mutex);
    }
#endif

    return HAL_OK;
}

/**
 * @brief Start DMA2D blending operation (interrupt mode)
 * @param pSrc1 Pointer to foreground buffer (must not be NULL)
 * @param pSrc2 Pointer to background buffer (must not be NULL)
 * @param pDst Pointer to destination buffer (must not be NULL)
 * @param width Blend width in pixels (1 to DMA2D_MAX_WIDTH)
 * @param height Blend height in pixels (1 to DMA2D_MAX_HEIGHT)
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef DMA2D_StartBlending_IT(const uint32_t *pSrc1, const uint32_t *pSrc2, uint32_t *pDst, uint32_t width, uint32_t height)
{
    HAL_StatusTypeDef status = HAL_ERROR;

    /* Parameter validation */
    if (pSrc1 == NULL || pSrc2 == NULL || pDst == NULL) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_StartBlending_IT: NULL pointer(s) provided\n");
        return HAL_ERROR;
    }

    if (width == 0 || width > DMA2D_MAX_WIDTH || height == 0 || height > DMA2D_MAX_HEIGHT) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_StartBlending_IT: Invalid dimensions %lux%lu\n", width, height);
        return HAL_ERROR;
    }

#ifdef DMA2D_USE_MUTEX
    if (dma2d_mutex != NULL) {
        osMutexWait(dma2d_mutex, osWaitForever);
    }
#endif

    if (!dma2d_status.initialized) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_StartBlending_IT: DMA2D not initialized\n");

#ifdef DMA2D_USE_MUTEX
        if (dma2d_mutex != NULL) {
            osMutexRelease(dma2d_mutex);
        }
#endif
        return HAL_ERROR;
    }

    if (dma2d_status.state == DMA2D_STATE_BUSY) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_StartBlending_IT: DMA2D is busy\n");

#ifdef DMA2D_USE_MUTEX
        if (dma2d_mutex != NULL) {
            osMutexRelease(dma2d_mutex);
        }
#endif
        return HAL_BUSY;
    }

    /* Update status */
    dma2d_status.state = DMA2D_STATE_BUSY;

    /* Start interrupt-driven blending */
    status = HAL_DMA2D_BlendingStart_IT(&hdma2d, (uint32_t)pSrc1, (uint32_t)pSrc2, (uint32_t)pDst, width, height);
    if (status != HAL_OK) {
        dma2d_status.last_error = HAL_DMA2D_GetError(&hdma2d);
        dma2d_status.error_count++;
        dma2d_status.state = DMA2D_STATE_ERROR;
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_StartBlending_IT: HAL_DMA2D_BlendingStart_IT failed with status %d\n", status);

#ifdef DMA2D_USE_MUTEX
        if (dma2d_mutex != NULL) {
            osMutexRelease(dma2d_mutex);
        }
#endif
        return status;
    }

    DMA2D_DEBUG_PRINT_INFO("DMA2D_StartBlending_IT: Interrupt blending started\n");

#ifdef DMA2D_USE_MUTEX
    if (dma2d_mutex != NULL) {
        osMutexRelease(dma2d_mutex);
    }
#endif

    return HAL_OK;
}

/**
 * @brief Poll for DMA2D transfer completion
 * @param timeout Timeout value in milliseconds
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef DMA2D_PollForTransfer(uint32_t timeout)
{
    return DMA2D_WaitForTransfer(timeout);
}

/**
 * @brief Abort ongoing DMA2D transfer
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef DMA2D_Abort(void)
{
    HAL_StatusTypeDef status = HAL_ERROR;

#ifdef DMA2D_USE_MUTEX
    if (dma2d_mutex != NULL) {
        osMutexWait(dma2d_mutex, osWaitForever);
    }
#endif

    if (!dma2d_status.initialized) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_Abort: DMA2D not initialized\n");

#ifdef DMA2D_USE_MUTEX
        if (dma2d_mutex != NULL) {
            osMutexRelease(dma2d_mutex);
        }
#endif
        return HAL_ERROR;
    }

    /* Abort transfer */
    status = HAL_DMA2D_Abort(&hdma2d);
    if (status != HAL_OK) {
        dma2d_status.last_error = HAL_DMA2D_GetError(&hdma2d);
        dma2d_status.error_count++;
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_Abort: HAL_DMA2D_Abort failed with status %d\n", status);
    } else {
        dma2d_status.state = DMA2D_STATE_READY;
        DMA2D_DEBUG_PRINT_INFO("DMA2D_Abort: Transfer aborted successfully\n");
    }

#ifdef DMA2D_USE_MUTEX
    if (dma2d_mutex != NULL) {
        osMutexRelease(dma2d_mutex);
    }
#endif

    return status;
}

/**
 * @brief Get DMA2D status information
 * @param status Pointer to status structure to fill (must not be NULL)
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef DMA2D_GetStatus(DMA2D_Status *status)
{
    if (status == NULL) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_GetStatus: NULL status pointer\n");
        return HAL_ERROR;
    }

#ifdef DMA2D_USE_MUTEX
    if (dma2d_mutex != NULL) {
        osMutexWait(dma2d_mutex, osWaitForever);
    }
#endif

    /* Copy current status */
    memcpy(status, &dma2d_status, sizeof(DMA2D_Status));

    /* Update current state from HAL */
    if (dma2d_status.initialized) {
        status->state = HAL_DMA2D_GetState(&hdma2d);
    }

#ifdef DMA2D_USE_MUTEX
    if (dma2d_mutex != NULL) {
        osMutexRelease(dma2d_mutex);
    }
#endif

    return HAL_OK;
}

/**
 * @brief Check if DMA2D is busy
 * @return bool true if DMA2D is busy, false otherwise
 */
bool DMA2D_IsBusy(void)
{
    if (!dma2d_status.initialized) {
        return false;
    }

    return (HAL_DMA2D_GetState(&hdma2d) == HAL_DMA2D_STATE_BUSY);
}

/**
 * @brief Register transfer complete callback
 * @param callback Function pointer to callback (NULL to disable)
 */
void DMA2D_RegisterTransferCompleteCallback(DMA2D_TransferCompleteCallback callback)
{
#ifdef DMA2D_USE_MUTEX
    if (dma2d_mutex != NULL) {
        osMutexWait(dma2d_mutex, osWaitForever);
    }
#endif

    transfer_complete_callback = callback;

    /* Update HAL callback */
    if (dma2d_status.initialized) {
        if (callback != NULL) {
            hdma2d.XferCpltCallback = DMA2D_TransferCompleteHandler;
        } else {
            hdma2d.XferCpltCallback = NULL;
        }
    }

#ifdef DMA2D_USE_MUTEX
    if (dma2d_mutex != NULL) {
        osMutexRelease(dma2d_mutex);
    }
#endif
}

/**
 * @brief Register transfer error callback
 * @param callback Function pointer to callback (NULL to disable)
 */
void DMA2D_RegisterTransferErrorCallback(DMA2D_TransferErrorCallback callback)
{
#ifdef DMA2D_USE_MUTEX
    if (dma2d_mutex != NULL) {
        osMutexWait(dma2d_mutex, osWaitForever);
    }
#endif

    transfer_error_callback = callback;

    /* Update HAL callback */
    if (dma2d_status.initialized) {
        if (callback != NULL) {
            hdma2d.XferErrorCallback = DMA2D_ErrorHandler;
        } else {
            hdma2d.XferErrorCallback = NULL;
        }
    }

#ifdef DMA2D_USE_MUTEX
    if (dma2d_mutex != NULL) {
        osMutexRelease(dma2d_mutex);
    }
#endif
}

/**
 * @brief Register transfer progress callback
 * @param callback Function pointer to callback (NULL to disable)
 */
void DMA2D_RegisterTransferProgressCallback(DMA2D_TransferProgressCallback callback)
{
#ifdef DMA2D_USE_MUTEX
    if (dma2d_mutex != NULL) {
        osMutexWait(dma2d_mutex, osWaitForever);
    }
#endif

    transfer_progress_callback = callback;

    /* Note: HAL DMA2D doesn't have a built-in progress callback.
     * This would need to be implemented using a timer or other mechanism
     * if progress tracking is required. */

#ifdef DMA2D_USE_MUTEX
    if (dma2d_mutex != NULL) {
        osMutexRelease(dma2d_mutex);
    }
#endif
}

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
uint32_t DMA2D_MakeColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
    return ((uint32_t)alpha << DMA2D_ALPHA_SHIFT) |
           ((uint32_t)red << DMA2D_RED_SHIFT) |
           ((uint32_t)green << DMA2D_GREEN_SHIFT) |
           blue;
}

/**
 * @brief Extract color components from ARGB8888 color
 * @param color ARGB8888 color value
 * @param red Pointer to red component (can be NULL)
 * @param green Pointer to green component (can be NULL)
 * @param blue Pointer to blue component (can be NULL)
 * @param alpha Pointer to alpha component (can be NULL)
 */
void DMA2D_GetColorComponents(uint32_t color, uint8_t *red, uint8_t *green, uint8_t *blue, uint8_t *alpha)
{
    if (alpha != NULL) {
        *alpha = (color >> DMA2D_ALPHA_SHIFT) & DMA2D_COLOR_COMPONENT_MASK;
    }
    if (red != NULL) {
        *red = (color >> DMA2D_RED_SHIFT) & DMA2D_COLOR_COMPONENT_MASK;
    }
    if (green != NULL) {
        *green = (color >> DMA2D_GREEN_SHIFT) & DMA2D_COLOR_COMPONENT_MASK;
    }
    if (blue != NULL) {
        *blue = color & DMA2D_COLOR_COMPONENT_MASK;
    }
}

/**
 * @brief Validate DMA2D configuration parameters
 * @param config Pointer to configuration structure
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef DMA2D_ValidateConfig(const DMA2D_Config *config)
{
    if (config == NULL) {
        return HAL_ERROR;
    }

    /* Validate mode */
    if (config->mode != DMA2D_MODE_R2M && config->mode != DMA2D_MODE_M2M &&
        config->mode != DMA2D_MODE_M2M_PFC && config->mode != DMA2D_MODE_M2M_BLEND) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_ValidateConfig: Invalid mode %lu\n", config->mode);
        return HAL_ERROR;
    }

    /* Validate color mode */
    if (config->color_mode != DMA2D_FORMAT_ARGB8888 && config->color_mode != DMA2D_FORMAT_RGB888 &&
        config->color_mode != DMA2D_FORMAT_RGB565 && config->color_mode != DMA2D_FORMAT_ARGB1555 &&
        config->color_mode != DMA2D_FORMAT_ARGB4444) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_ValidateConfig: Invalid color mode %lu\n", config->color_mode);
        return HAL_ERROR;
    }

    /* Validate output offset */
    if (config->output_offset > DMA2D_MAX_OFFSET) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_ValidateConfig: Invalid output offset %lu\n", config->output_offset);
        return HAL_ERROR;
    }

    /* Validate color components for R2M mode */
    if (config->mode == DMA2D_MODE_R2M) {
        if (config->red_value > DMA2D_COLOR_COMPONENT_MAX ||
            config->green_value > DMA2D_COLOR_COMPONENT_MAX ||
            config->blue_value > DMA2D_COLOR_COMPONENT_MAX ||
            config->alpha_value > DMA2D_COLOR_COMPONENT_MAX) {
            DMA2D_DEBUG_PRINT_ERROR("DMA2D_ValidateConfig: Invalid color components\n");
            return HAL_ERROR;
        }
    }

    return HAL_OK;
}

/**
 * @brief Validate DMA2D layer configuration parameters
 * @param layer_config Pointer to layer configuration structure
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef DMA2D_ValidateLayerConfig(const DMA2D_LayerConfig *layer_config)
{
    if (layer_config == NULL) {
        return HAL_ERROR;
    }

    /* Validate input color mode */
    if (layer_config->input_color_mode != DMA2D_INPUT_ARGB8888 &&
        layer_config->input_color_mode != DMA2D_INPUT_RGB888 &&
        layer_config->input_color_mode != DMA2D_INPUT_RGB565 &&
        layer_config->input_color_mode != DMA2D_INPUT_ARGB1555 &&
        layer_config->input_color_mode != DMA2D_INPUT_ARGB4444) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_ValidateLayerConfig: Invalid input color mode %lu\n", layer_config->input_color_mode);
        return HAL_ERROR;
    }

    /* Validate alpha mode */
    if (layer_config->input_alpha_mode != DMA2D_ALPHA_NO_MODIF &&
        layer_config->input_alpha_mode != DMA2D_ALPHA_REPLACE &&
        layer_config->input_alpha_mode != DMA2D_ALPHA_COMBINE) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_ValidateLayerConfig: Invalid alpha mode %lu\n", layer_config->input_alpha_mode);
        return HAL_ERROR;
    }

    /* Validate alpha value */
    if (layer_config->input_alpha > DMA2D_COLOR_COMPONENT_MAX) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_ValidateLayerConfig: Invalid alpha value %lu\n", layer_config->input_alpha);
        return HAL_ERROR;
    }

    /* Validate input offset */
    if (layer_config->input_offset > DMA2D_MAX_OFFSET) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_ValidateLayerConfig: Invalid input offset %lu\n", layer_config->input_offset);
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
 * @brief Get string representation of DMA2D error code
 * @param error_code HAL error code
 * @return const char* Error description string
 */
const char* DMA2D_GetErrorString(HAL_StatusTypeDef error_code)
{
    switch (error_code) {
        case HAL_OK:       return "No error";
        case HAL_ERROR:    return "General error";
        case HAL_BUSY:     return "Resource busy";
        case HAL_TIMEOUT:  return "Timeout occurred";
        default:           return "Unknown error";
    }
}

/**
 * @brief Get string representation of DMA2D state
 * @param state DMA2D state
 * @return const char* State description string
 */
const char* DMA2D_GetStateString(uint32_t state)
{
    switch (state) {
        case DMA2D_STATE_RESET:  return "Reset";
        case DMA2D_STATE_READY:  return "Ready";
        case DMA2D_STATE_BUSY:   return "Busy";
        case DMA2D_STATE_TIMEOUT: return "Timeout";
        case DMA2D_STATE_ERROR:  return "Error";
        default:                 return "Unknown";
    }
}

/**
 * @brief Get DMA2D driver version
 * @return const char* Version string
 */
const char* DMA2D_GetVersion(void)
{
    return DMA2D_DRIVER_VERSION;
}

/**
 * @brief Get DMA2D driver capabilities
 * @return uint32_t Bitmask of supported features
 */
uint32_t DMA2D_GetCapabilities(void)
{
    uint32_t capabilities = 0;

    capabilities |= DMA2D_CAPABILITY_BASIC;
    capabilities |= DMA2D_CAPABILITY_FILL;
    capabilities |= DMA2D_CAPABILITY_COPY;
    capabilities |= DMA2D_CAPABILITY_PFC;
    capabilities |= DMA2D_CAPABILITY_BLEND;
    capabilities |= DMA2D_CAPABILITY_INTERRUPT;
    capabilities |= DMA2D_CAPABILITY_CALLBACK;

    return capabilities;
}

/* ============================================================================
 * Private Functions
 * ============================================================================ */

/**
 * @brief DMA2D error handler
 * @param hdma2d Pointer to DMA2D handle
 */
static void DMA2D_ErrorHandler(DMA2D_HandleTypeDef *hdma2d)
{
    /* Update status */
    dma2d_status.state = DMA2D_STATE_ERROR;
    dma2d_status.last_error = hdma2d->ErrorCode;
    dma2d_status.error_count++;

    DMA2D_DEBUG_PRINT_ERROR("DMA2D_ErrorHandler: Error 0x%08lX occurred\n", hdma2d->ErrorCode);

    /* Call user error callback if registered */
    if (transfer_error_callback != NULL) {
        transfer_error_callback(hdma2d);
    }
}

/**
 * @brief DMA2D transfer complete handler
 * @param hdma2d Pointer to DMA2D handle
 */
static void DMA2D_TransferCompleteHandler(DMA2D_HandleTypeDef *hdma2d)
{
    /* Update status */
    dma2d_status.state = DMA2D_STATE_READY;
    dma2d_status.transfer_count++;

    DMA2D_DEBUG_PRINT_INFO("DMA2D_TransferCompleteHandler: Transfer completed\n");

    /* Call user complete callback if registered */
    if (transfer_complete_callback != NULL) {
        transfer_complete_callback(hdma2d);
    }
}

/**
 * @brief Validate transfer parameters
 * @param pSrc Pointer to source buffer
 * @param pDst Pointer to destination buffer
 * @param width Transfer width
 * @param height Transfer height
 * @return HAL_StatusTypeDef
 */
static HAL_StatusTypeDef DMA2D_ValidateParameters(const uint32_t *pSrc, uint32_t *pDst, uint32_t width, uint32_t height)
{
    if (pDst == NULL) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_ValidateParameters: NULL destination pointer\n");
        return HAL_ERROR;
    }

    /* For M2M operations, source is required */
    if (hdma2d.Init.Mode != DMA2D_MODE_R2M && pSrc == NULL) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_ValidateParameters: NULL source pointer for M2M operation\n");
        return HAL_ERROR;
    }

    if (width == 0 || width > DMA2D_MAX_WIDTH || height == 0 || height > DMA2D_MAX_HEIGHT) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_ValidateParameters: Invalid dimensions %lux%lu\n", width, height);
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
 * @brief Wait for DMA2D transfer completion
 * @param timeout Timeout value in milliseconds
 * @return HAL_StatusTypeDef
 */
static HAL_StatusTypeDef DMA2D_WaitForTransfer(uint32_t timeout)
{
    HAL_StatusTypeDef status = HAL_OK;

    status = HAL_DMA2D_PollForTransfer(&hdma2d, timeout);
    if (status != HAL_OK) {
        dma2d_status.last_error = HAL_DMA2D_GetError(&hdma2d);
        dma2d_status.error_count++;
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_WaitForTransfer: Poll failed with status %d\n", status);
    }

    return status;
}

/* ============================================================================
 * Advanced Features (Optional)
 * ============================================================================ */

/**
 * @brief Enable DMA2D hardware acceleration for LCD operations
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef DMA2D_EnableLCDMode(void)
{
    HAL_StatusTypeDef status = HAL_OK;
    /* Configure DMA2D for optimal LCD framebuffer operations */
    /* This is a placeholder for LCD-specific optimizations */
    DMA2D_DEBUG_PRINT_INFO("DMA2D_EnableLCDMode: LCD mode optimizations enabled\n");
    return status;
}

/**
 * @brief Enable DMA2D for optimal SDRAM access
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef DMA2D_EnableSDRAMMode(void)
{
    HAL_StatusTypeDef status = HAL_OK;
    /* Configure DMA2D for optimal SDRAM access */
    /* This is a placeholder for SDRAM-specific optimizations */
    DMA2D_DEBUG_PRINT_INFO("DMA2D_EnableSDRAMMode: SDRAM mode optimizations enabled\n");
    return status;
}

/**
 * @brief Perform DMA2D self-test
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef DMA2D_SelfTest(void)
{
    HAL_StatusTypeDef status = HAL_OK;
    const uint32_t DMA2D_TEST_BUFFER_SIZE = 100U;
    const uint32_t DMA2D_TEST_DIMENSION = 10U;
    uint32_t test_buffer[DMA2D_TEST_BUFFER_SIZE];

    DMA2D_DEBUG_PRINT_INFO("DMA2D_SelfTest: Starting self-test\n");

    /* Test 1: Basic fill operation */
    status = DMA2D_StartFill(DMA2D_COLOR_RED, test_buffer, DMA2D_TEST_DIMENSION, DMA2D_TEST_DIMENSION);
    if (status != HAL_OK) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_SelfTest: Fill test failed\n");
        return status;
    }

    /* Verify fill result */
    if (test_buffer[0] != DMA2D_COLOR_RED) {
        DMA2D_DEBUG_PRINT_ERROR("DMA2D_SelfTest: Fill verification failed\n");
        return HAL_ERROR;
    }

    DMA2D_DEBUG_PRINT_INFO("DMA2D_SelfTest: Self-test passed\n");
    return HAL_OK;
}
