/**
 * @file dma2d.c
 * @brief DMA2D (Chrom-Art Accelerator) peripheral driver implementation for STM32F4 series
 * @author GitHub Copilot
 * @date 2025
 * @version 2.0.0
 *
 * @details
 * This file implements the DMA2D peripheral driver for STM32F429I-DISC1 board.
 * It provides hardware-accelerated 2D graphics operations including pixel format
 * conversion, alpha blending, and area filling.
 *
 * Key Features:
 * - Hardware-accelerated 2D graphics operations
 * - Multiple color format support
 * - Alpha blending with configurable transparency
 * - Interrupt-driven and polling modes
 * - Comprehensive error handling
 * - Memory-efficient design for embedded systems
 */

/* Includes ------------------------------------------------------------------*/
#include "dma2d.h"
#include "log.h"
#include <string.h>

/* Private defines -----------------------------------------------------------*/
#define DMA2D_TIMEOUT_DEFAULT    1000U   /**< Default timeout in milliseconds */

/* Private variables ---------------------------------------------------------*/
DMA2D_HandleTypeDef hdma2d;              /**< DMA2D HAL handle */
static DMA2D_Status dma2d_status = {0};  /**< DMA2D status structure */

/* Callback function pointers */
static DMA2D_TransferCompleteCallback transfer_complete_callback = NULL;
static DMA2D_TransferErrorCallback transfer_error_callback = NULL;
static DMA2D_TransferProgressCallback transfer_progress_callback = NULL;

/* Private function prototypes -----------------------------------------------*/
static HAL_StatusTypeDef DMA2D_ValidateConfigInternal(const DMA2D_Config *config);
static HAL_StatusTypeDef DMA2D_ValidateLayerConfigInternal(const DMA2D_LayerConfig *layer_config);
static HAL_StatusTypeDef DMA2D_ValidateTransferParams(const uint32_t *pSrc, uint32_t *pDst, uint32_t width, uint32_t height);
static void DMA2D_UpdateStatus(HAL_StatusTypeDef result);
static const char* DMA2D_GetErrorStringInternal(HAL_StatusTypeDef error_code);
static const char* DMA2D_GetStateStringInternal(uint32_t state);

/* Exported DMA2D handle for interrupt handlers */
DMA2D_HandleTypeDef hdma2d;

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Internal configuration validation
 * @param config Pointer to configuration structure
 * @return HAL_StatusTypeDef
 */
static HAL_StatusTypeDef DMA2D_ValidateConfigInternal(const DMA2D_Config *config)
{
    if (config == NULL) {
        log_error("DMA2D configuration is NULL");
        return HAL_ERROR;
    }

    /* Validate mode */
    if (config->mode != DMA2D_MODE_R2M &&
        config->mode != DMA2D_MODE_M2M &&
        config->mode != DMA2D_MODE_M2M_PFC &&
        config->mode != DMA2D_MODE_M2M_BLEND) {
        log_error("Invalid DMA2D mode: %lu", (unsigned long)config->mode);
        return HAL_ERROR;
    }

    /* Validate color mode */
    if (config->color_mode != DMA2D_FORMAT_ARGB8888 &&
        config->color_mode != DMA2D_FORMAT_RGB888 &&
        config->color_mode != DMA2D_FORMAT_RGB565 &&
        config->color_mode != DMA2D_FORMAT_ARGB1555 &&
        config->color_mode != DMA2D_FORMAT_ARGB4444) {
        log_error("Invalid DMA2D color mode: %lu", (unsigned long)config->color_mode);
        return HAL_ERROR;
    }

    /* Validate output offset */
    if (config->output_offset > DMA2D_MAX_OFFSET) {
        log_error("Invalid DMA2D output offset: %lu", (unsigned long)config->output_offset);
        return HAL_ERROR;
    }

    /* Validate color components for R2M mode */
    if (config->mode == DMA2D_MODE_R2M) {
        if (config->red_value > 255 || config->green_value > 255 ||
            config->blue_value > 255 || config->alpha_value > 255) {
            log_error("Invalid DMA2D color components: R=%lu, G=%lu, B=%lu, A=%lu",
                     (unsigned long)config->red_value, (unsigned long)config->green_value,
                     (unsigned long)config->blue_value, (unsigned long)config->alpha_value);
            return HAL_ERROR;
        }
    }

    return HAL_OK;
}

/**
 * @brief Internal layer configuration validation
 * @param layer_config Pointer to layer configuration structure
 * @return HAL_StatusTypeDef
 */
static HAL_StatusTypeDef DMA2D_ValidateLayerConfigInternal(const DMA2D_LayerConfig *layer_config)
{
    if (layer_config == NULL) {
        log_error("DMA2D layer configuration is NULL");
        return HAL_ERROR;
    }

    /* Validate input color mode */
    if (layer_config->input_color_mode != DMA2D_INPUT_ARGB8888 &&
        layer_config->input_color_mode != DMA2D_INPUT_RGB888 &&
        layer_config->input_color_mode != DMA2D_INPUT_RGB565 &&
        layer_config->input_color_mode != DMA2D_INPUT_ARGB1555 &&
        layer_config->input_color_mode != DMA2D_INPUT_ARGB4444) {
        log_error("Invalid DMA2D input color mode: %lu", (unsigned long)layer_config->input_color_mode);
        return HAL_ERROR;
    }

    /* Validate alpha mode */
    if (layer_config->input_alpha_mode != DMA2D_ALPHA_NO_MODIF &&
        layer_config->input_alpha_mode != DMA2D_ALPHA_REPLACE &&
        layer_config->input_alpha_mode != DMA2D_ALPHA_COMBINE) {
        log_error("Invalid DMA2D alpha mode: %lu", (unsigned long)layer_config->input_alpha_mode);
        return HAL_ERROR;
    }

    /* Validate alpha value */
    if (layer_config->input_alpha > 255) {
        log_error("Invalid DMA2D alpha value: %lu", (unsigned long)layer_config->input_alpha);
        return HAL_ERROR;
    }

    /* Validate input offset */
    if (layer_config->input_offset > DMA2D_MAX_OFFSET) {
        log_error("Invalid DMA2D input offset: %lu", (unsigned long)layer_config->input_offset);
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
 * @brief Validate transfer parameters
 * @param pSrc Pointer to source buffer
 * @param pDst Pointer to destination buffer
 * @param width Transfer width
 * @param height Transfer height
 * @return HAL_StatusTypeDef
 */
static HAL_StatusTypeDef DMA2D_ValidateTransferParams(const uint32_t *pSrc, uint32_t *pDst, uint32_t width, uint32_t height)
{
    if (pDst == NULL) {
        log_error("DMA2D destination buffer is NULL");
        return HAL_ERROR;
    }

    if (width == 0 || width > DMA2D_MAX_WIDTH) {
        log_error("Invalid DMA2D width: %lu", (unsigned long)width);
        return HAL_ERROR;
    }

    if (height == 0 || height > DMA2D_MAX_HEIGHT) {
        log_error("Invalid DMA2D height: %lu", (unsigned long)height);
        return HAL_ERROR;
    }

    /* For M2M operations, source buffer is required */
    if (hdma2d.Init.Mode != DMA2D_R2M && pSrc == NULL) {
        log_error("DMA2D source buffer is NULL for M2M operation");
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
 * @brief Update internal status structure
 * @param result HAL status result
 */
static void DMA2D_UpdateStatus(HAL_StatusTypeDef result)
{
    dma2d_status.state = hdma2d.State;
    dma2d_status.last_error = result;

    if (result == HAL_OK) {
        dma2d_status.transfer_count++;
        // Note: Bytes transferred calculation removed as ImageWidth/ImageHeight not available in LayerCfg
    } else if (result != HAL_BUSY) {
        dma2d_status.error_count++;
    }
}

/**
 * @brief Get error string (internal)
 * @param error_code HAL error code
 * @return const char* Error description
 */
static const char* DMA2D_GetErrorStringInternal(HAL_StatusTypeDef error_code)
{
    switch (error_code) {
        case HAL_OK:       return "No error";
        case HAL_ERROR:    return "General error";
        case HAL_BUSY:     return "Peripheral busy";
        case HAL_TIMEOUT:  return "Timeout occurred";
        default:           return "Unknown error";
    }
}

/**
 * @brief Get state string (internal)
 * @param state DMA2D state
 * @return const char* State description
 */
static const char* DMA2D_GetStateStringInternal(uint32_t state)
{
    switch (state) {
        case HAL_DMA2D_STATE_RESET:   return "Reset";
        case HAL_DMA2D_STATE_READY:   return "Ready";
        case HAL_DMA2D_STATE_BUSY:    return "Busy";
        case HAL_DMA2D_STATE_TIMEOUT: return "Timeout";
        case HAL_DMA2D_STATE_ERROR:   return "Error";
        default:                      return "Unknown";
    }
}

/* Exported functions -------------------------------------------------------*/

/**
 * @brief Initialize the DMA2D peripheral with specified configuration
 */
HAL_StatusTypeDef DMA2D_Init(const DMA2D_Config *config)
{
    HAL_StatusTypeDef result;

    log_debug("Initializing DMA2D peripheral");

    /* Validate configuration */
    result = DMA2D_ValidateConfigInternal(config);
    if (result != HAL_OK) {
        DMA2D_UpdateStatus(result);
        return result;
    }

    /* Enable DMA2D clock */
    __HAL_RCC_DMA2D_CLK_ENABLE();

    /* Initialize DMA2D handle */
    hdma2d.Instance = DMA2D;
    hdma2d.Init.Mode = config->mode;
    hdma2d.Init.ColorMode = config->color_mode;
    hdma2d.Init.OutputOffset = config->output_offset;

    /* Initialize HAL DMA2D */
    result = HAL_DMA2D_Init(&hdma2d);
    if (result != HAL_OK) {
        log_error("HAL_DMA2D_Init failed: %s", DMA2D_GetErrorStringInternal(result));
        DMA2D_UpdateStatus(result);
        return result;
    }

    /* Configure output color for R2M mode */
    if (config->mode == DMA2D_MODE_R2M) {
        uint32_t color = DMA2D_MakeColor((uint8_t)config->red_value,
                                        (uint8_t)config->green_value,
                                        (uint8_t)config->blue_value,
                                        (uint8_t)config->alpha_value);
        HAL_DMA2D_ConfigDeadTime(&hdma2d, 0); // No dead time
        HAL_DMA2D_ConfigLayer(&hdma2d, 1); // Configure output layer
        hdma2d.LayerCfg[1].InputColorMode = config->color_mode;
        hdma2d.LayerCfg[1].InputAlpha = color;
        hdma2d.LayerCfg[1].InputOffset = config->output_offset;
    }

    /* Update status */
    dma2d_status.initialized = true;
    DMA2D_UpdateStatus(result);

    log_info("DMA2D initialized successfully");
    return HAL_OK;
}

/**
 * @brief Deinitialize the DMA2D peripheral
 */
HAL_StatusTypeDef DMA2D_DeInit(void)
{
    HAL_StatusTypeDef result;

    log_debug("Deinitializing DMA2D peripheral");

    if (!dma2d_status.initialized) {
        log_warning("DMA2D not initialized");
        return HAL_ERROR;
    }

    /* Abort any ongoing transfer */
    if (hdma2d.State == HAL_DMA2D_STATE_BUSY) {
        result = HAL_DMA2D_Abort(&hdma2d);
        if (result != HAL_OK) {
            log_warning("Failed to abort ongoing DMA2D transfer: %s", DMA2D_GetErrorStringInternal(result));
        }
    }

    /* Deinitialize HAL DMA2D */
    result = HAL_DMA2D_DeInit(&hdma2d);
    if (result != HAL_OK) {
        log_error("HAL_DMA2D_DeInit failed: %s", DMA2D_GetErrorStringInternal(result));
        DMA2D_UpdateStatus(result);
        return result;
    }

    /* Disable DMA2D clock */
    __HAL_RCC_DMA2D_CLK_DISABLE();

    /* Reset status */
    memset(&dma2d_status, 0, sizeof(DMA2D_Status));
    dma2d_status.state = HAL_DMA2D_STATE_RESET;

    /* Clear callbacks */
    transfer_complete_callback = NULL;
    transfer_error_callback = NULL;
    transfer_progress_callback = NULL;

    log_info("DMA2D deinitialized successfully");
    return HAL_OK;
}

/**
 * @brief Configure DMA2D layer parameters
 */
HAL_StatusTypeDef DMA2D_ConfigLayer(uint32_t layer, const DMA2D_LayerConfig *layer_config)
{
    HAL_StatusTypeDef result;

    log_debug("Configuring DMA2D layer %lu", (unsigned long)layer);

    /* Validate parameters */
    result = DMA2D_ValidateLayerConfigInternal(layer_config);
    if (result != HAL_OK) {
        DMA2D_UpdateStatus(result);
        return result;
    }

    if (layer != DMA2D_FOREGROUND_LAYER && layer != DMA2D_BACKGROUND_LAYER) {
        log_error("Invalid DMA2D layer: %lu", (unsigned long)layer);
        DMA2D_UpdateStatus(HAL_ERROR);
        return HAL_ERROR;
    }

    if (!dma2d_status.initialized) {
        log_error("DMA2D not initialized");
        DMA2D_UpdateStatus(HAL_ERROR);
        return HAL_ERROR;
    }

    /* Configure layer */
    result = HAL_DMA2D_ConfigLayer(&hdma2d, layer);
    if (result != HAL_OK) {
        log_error("HAL_DMA2D_ConfigLayer failed: %s", DMA2D_GetErrorStringInternal(result));
        DMA2D_UpdateStatus(result);
        return result;
    }

    /* Set layer configuration */
    uint32_t layer_idx = (layer == DMA2D_FOREGROUND_LAYER) ? 0 : 1;
    hdma2d.LayerCfg[layer_idx].InputColorMode = layer_config->input_color_mode;
    hdma2d.LayerCfg[layer_idx].AlphaMode = layer_config->input_alpha_mode;
    hdma2d.LayerCfg[layer_idx].InputAlpha = layer_config->input_alpha;
    hdma2d.LayerCfg[layer_idx].InputOffset = layer_config->input_offset;

    log_debug("DMA2D layer %lu configured successfully", (unsigned long)layer);
    return HAL_OK;
}

/**
 * @brief Start DMA2D transfer operation (polling mode)
 */
HAL_StatusTypeDef DMA2D_StartTransfer(const uint32_t *pSrc, uint32_t *pDst, uint32_t width, uint32_t height)
{
    HAL_StatusTypeDef result;

    log_debug("Starting DMA2D transfer: %lux%lu", (unsigned long)width, (unsigned long)height);

    /* Validate parameters */
    result = DMA2D_ValidateTransferParams(pSrc, pDst, width, height);
    if (result != HAL_OK) {
        DMA2D_UpdateStatus(result);
        return result;
    }

    if (!dma2d_status.initialized) {
        log_error("DMA2D not initialized");
        DMA2D_UpdateStatus(HAL_ERROR);
        return HAL_ERROR;
    }

    if (DMA2D_IsBusy()) {
        log_warning("DMA2D is busy");
        return HAL_BUSY;
    }

    /* Start transfer based on mode */
    switch (hdma2d.Init.Mode) {
        case DMA2D_R2M:
            result = HAL_DMA2D_Start(&hdma2d, 0, (uint32_t)pDst, width, height);
            break;
        case DMA2D_M2M:
        case DMA2D_M2M_PFC:
            result = HAL_DMA2D_Start(&hdma2d, (uint32_t)pSrc, (uint32_t)pDst, width, height);
            break;
        case DMA2D_M2M_BLEND:
            result = HAL_DMA2D_BlendingStart(&hdma2d, (uint32_t)pSrc, 0, (uint32_t)pDst, width, height);
            break;
        default:
            log_error("Unsupported DMA2D mode: %lu", (unsigned long)hdma2d.Init.Mode);
            result = HAL_ERROR;
            break;
    }

    if (result != HAL_OK) {
        log_error("DMA2D transfer start failed: %s", DMA2D_GetErrorStringInternal(result));
        DMA2D_UpdateStatus(result);
        return result;
    }

    /* Poll for completion */
    result = HAL_DMA2D_PollForTransfer(&hdma2d, DMA2D_TIMEOUT_DEFAULT);
    if (result != HAL_OK) {
        log_error("DMA2D transfer poll failed: %s", DMA2D_GetErrorStringInternal(result));
        DMA2D_UpdateStatus(result);
        return result;
    }

    DMA2D_UpdateStatus(result);
    log_debug("DMA2D transfer completed successfully");
    return HAL_OK;
}

/**
 * @brief Start DMA2D register-to-memory fill operation (polling mode)
 */
HAL_StatusTypeDef DMA2D_StartFill(uint32_t color, uint32_t *pDst, uint32_t width, uint32_t height)
{
    HAL_StatusTypeDef result;

    log_debug("Starting DMA2D fill: color=0x%08lx, size=%lux%lu", (unsigned long)color, (unsigned long)width, (unsigned long)height);

    /* Validate parameters */
    result = DMA2D_ValidateTransferParams(NULL, pDst, width, height);
    if (result != HAL_OK) {
        DMA2D_UpdateStatus(result);
        return result;
    }

    if (!dma2d_status.initialized) {
        log_error("DMA2D not initialized");
        DMA2D_UpdateStatus(HAL_ERROR);
        return HAL_ERROR;
    }

    if (DMA2D_IsBusy()) {
        log_warning("DMA2D is busy");
        return HAL_BUSY;
    }

    /* Start fill operation */
    result = HAL_DMA2D_Start(&hdma2d, color, (uint32_t)pDst, width, height);
    if (result != HAL_OK) {
        log_error("DMA2D fill start failed: %s", DMA2D_GetErrorStringInternal(result));
        DMA2D_UpdateStatus(result);
        return result;
    }

    /* Poll for completion */
    result = HAL_DMA2D_PollForTransfer(&hdma2d, DMA2D_TIMEOUT_DEFAULT);
    if (result != HAL_OK) {
        log_error("DMA2D fill poll failed: %s", DMA2D_GetErrorStringInternal(result));
        DMA2D_UpdateStatus(result);
        return result;
    }

    DMA2D_UpdateStatus(result);
    log_debug("DMA2D fill completed successfully");
    return HAL_OK;
}

/**
 * @brief Start DMA2D blending operation (polling mode)
 */
HAL_StatusTypeDef DMA2D_StartBlending(const uint32_t *pSrc1, const uint32_t *pSrc2, uint32_t *pDst, uint32_t width, uint32_t height)
{
    HAL_StatusTypeDef result;

    log_debug("Starting DMA2D blending: size=%lux%lu", (unsigned long)width, (unsigned long)height);

    /* Validate parameters */
    result = DMA2D_ValidateTransferParams(pSrc1, pDst, width, height);
    if (result != HAL_OK) {
        DMA2D_UpdateStatus(result);
        return result;
    }

    if (pSrc2 == NULL) {
        log_error("DMA2D blending source 2 buffer is NULL");
        DMA2D_UpdateStatus(HAL_ERROR);
        return HAL_ERROR;
    }

    if (!dma2d_status.initialized) {
        log_error("DMA2D not initialized");
        DMA2D_UpdateStatus(HAL_ERROR);
        return HAL_ERROR;
    }

    if (DMA2D_IsBusy()) {
        log_warning("DMA2D is busy");
        return HAL_BUSY;
    }

    /* Configure foreground layer (source 1) */
    HAL_DMA2D_ConfigLayer(&hdma2d, DMA2D_FOREGROUND_LAYER);
    hdma2d.LayerCfg[0].InputColorMode = DMA2D_INPUT_ARGB8888;
    hdma2d.LayerCfg[0].InputOffset = 0;

    /* Configure background layer (source 2) */
    HAL_DMA2D_ConfigLayer(&hdma2d, DMA2D_BACKGROUND_LAYER);
    hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
    hdma2d.LayerCfg[1].InputOffset = 0;

    /* Start blending operation */
    result = HAL_DMA2D_BlendingStart(&hdma2d, (uint32_t)pSrc1, (uint32_t)pSrc2, (uint32_t)pDst, width, height);
    if (result != HAL_OK) {
        log_error("DMA2D blending start failed: %s", DMA2D_GetErrorStringInternal(result));
        DMA2D_UpdateStatus(result);
        return result;
    }

    /* Poll for completion */
    result = HAL_DMA2D_PollForTransfer(&hdma2d, DMA2D_TIMEOUT_DEFAULT);
    if (result != HAL_OK) {
        log_error("DMA2D blending poll failed: %s", DMA2D_GetErrorStringInternal(result));
        DMA2D_UpdateStatus(result);
        return result;
    }

    DMA2D_UpdateStatus(result);
    log_debug("DMA2D blending completed successfully");
    return HAL_OK;
}

/**
 * @brief Start DMA2D transfer operation (interrupt mode)
 */
HAL_StatusTypeDef DMA2D_StartTransfer_IT(const uint32_t *pSrc, uint32_t *pDst, uint32_t width, uint32_t height)
{
    HAL_StatusTypeDef result;

    log_debug("Starting DMA2D transfer (interrupt): %lux%lu", (unsigned long)width, (unsigned long)height);

    /* Validate parameters */
    result = DMA2D_ValidateTransferParams(pSrc, pDst, width, height);
    if (result != HAL_OK) {
        DMA2D_UpdateStatus(result);
        return result;
    }

    if (!dma2d_status.initialized) {
        log_error("DMA2D not initialized");
        DMA2D_UpdateStatus(HAL_ERROR);
        return HAL_ERROR;
    }

    if (DMA2D_IsBusy()) {
        log_warning("DMA2D is busy");
        return HAL_BUSY;
    }

    /* Start transfer based on mode */
    switch (hdma2d.Init.Mode) {
        case DMA2D_R2M:
            result = HAL_DMA2D_Start_IT(&hdma2d, 0, (uint32_t)pDst, width, height);
            break;
        case DMA2D_M2M:
        case DMA2D_M2M_PFC:
            result = HAL_DMA2D_Start_IT(&hdma2d, (uint32_t)pSrc, (uint32_t)pDst, width, height);
            break;
        case DMA2D_M2M_BLEND:
            result = HAL_DMA2D_BlendingStart_IT(&hdma2d, (uint32_t)pSrc, 0, (uint32_t)pDst, width, height);
            break;
        default:
            log_error("Unsupported DMA2D mode: %lu", (unsigned long)hdma2d.Init.Mode);
            result = HAL_ERROR;
            break;
    }

    if (result != HAL_OK) {
        log_error("DMA2D interrupt transfer start failed: %s", DMA2D_GetErrorStringInternal(result));
        DMA2D_UpdateStatus(result);
        return result;
    }

    DMA2D_UpdateStatus(result);
    return HAL_OK;
}

/**
 * @brief Start DMA2D register-to-memory fill operation (interrupt mode)
 */
HAL_StatusTypeDef DMA2D_StartFill_IT(uint32_t color, uint32_t *pDst, uint32_t width, uint32_t height)
{
    HAL_StatusTypeDef result;

    log_debug("Starting DMA2D fill (interrupt): color=0x%08lx, size=%lux%lu", (unsigned long)color, (unsigned long)width, (unsigned long)height);

    /* Validate parameters */
    result = DMA2D_ValidateTransferParams(NULL, pDst, width, height);
    if (result != HAL_OK) {
        DMA2D_UpdateStatus(result);
        return result;
    }

    if (!dma2d_status.initialized) {
        log_error("DMA2D not initialized");
        DMA2D_UpdateStatus(HAL_ERROR);
        return HAL_ERROR;
    }

    if (DMA2D_IsBusy()) {
        log_warning("DMA2D is busy");
        return HAL_BUSY;
    }

    /* Start fill operation */
    result = HAL_DMA2D_Start_IT(&hdma2d, color, (uint32_t)pDst, width, height);
    if (result != HAL_OK) {
        log_error("DMA2D interrupt fill start failed: %s", DMA2D_GetErrorStringInternal(result));
        DMA2D_UpdateStatus(result);
        return result;
    }

    DMA2D_UpdateStatus(result);
    return HAL_OK;
}

/**
 * @brief Start DMA2D blending operation (interrupt mode)
 */
HAL_StatusTypeDef DMA2D_StartBlending_IT(const uint32_t *pSrc1, const uint32_t *pSrc2, uint32_t *pDst, uint32_t width, uint32_t height)
{
    HAL_StatusTypeDef result;

    log_debug("Starting DMA2D blending (interrupt): size=%lux%lu", (unsigned long)width, (unsigned long)height);

    /* Validate parameters */
    result = DMA2D_ValidateTransferParams(pSrc1, pDst, width, height);
    if (result != HAL_OK) {
        DMA2D_UpdateStatus(result);
        return result;
    }

    if (pSrc2 == NULL) {
        log_error("DMA2D blending source 2 buffer is NULL");
        DMA2D_UpdateStatus(HAL_ERROR);
        return HAL_ERROR;
    }

    if (!dma2d_status.initialized) {
        log_error("DMA2D not initialized");
        DMA2D_UpdateStatus(HAL_ERROR);
        return HAL_ERROR;
    }

    if (DMA2D_IsBusy()) {
        log_warning("DMA2D is busy");
        return HAL_BUSY;
    }

    /* Configure foreground layer (source 1) */
    HAL_DMA2D_ConfigLayer(&hdma2d, DMA2D_FOREGROUND_LAYER);
    hdma2d.LayerCfg[0].InputColorMode = DMA2D_INPUT_ARGB8888;
    hdma2d.LayerCfg[0].InputOffset = 0;

    /* Configure background layer (source 2) */
    HAL_DMA2D_ConfigLayer(&hdma2d, DMA2D_BACKGROUND_LAYER);
    hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
    hdma2d.LayerCfg[1].InputOffset = 0;

    /* Start blending operation */
    result = HAL_DMA2D_BlendingStart_IT(&hdma2d, (uint32_t)pSrc1, (uint32_t)pSrc2, (uint32_t)pDst, width, height);
    if (result != HAL_OK) {
        log_error("DMA2D interrupt blending start failed: %s", DMA2D_GetErrorStringInternal(result));
        DMA2D_UpdateStatus(result);
        return result;
    }

    DMA2D_UpdateStatus(result);
    return HAL_OK;
}

/**
 * @brief Poll for DMA2D transfer completion
 */
HAL_StatusTypeDef DMA2D_PollForTransfer(uint32_t timeout)
{
    HAL_StatusTypeDef result;

    log_debug("Polling for DMA2D transfer completion, timeout=%lu", (unsigned long)timeout);

    if (!dma2d_status.initialized) {
        log_error("DMA2D not initialized");
        return HAL_ERROR;
    }

    result = HAL_DMA2D_PollForTransfer(&hdma2d, timeout);
    if (result != HAL_OK) {
        log_error("DMA2D poll failed: %s", DMA2D_GetErrorStringInternal(result));
    }

    DMA2D_UpdateStatus(result);
    return result;
}

/**
 * @brief Abort ongoing DMA2D transfer
 */
HAL_StatusTypeDef DMA2D_Abort(void)
{
    HAL_StatusTypeDef result;

    log_debug("Aborting DMA2D transfer");

    if (!dma2d_status.initialized) {
        log_error("DMA2D not initialized");
        return HAL_ERROR;
    }

    result = HAL_DMA2D_Abort(&hdma2d);
    if (result != HAL_OK) {
        log_error("DMA2D abort failed: %s", DMA2D_GetErrorStringInternal(result));
    } else {
        log_info("DMA2D transfer aborted successfully");
    }

    DMA2D_UpdateStatus(result);
    return result;
}

/**
 * @brief Get DMA2D status information
 */
HAL_StatusTypeDef DMA2D_GetStatus(DMA2D_Status *status)
{
    if (status == NULL) {
        log_error("DMA2D status pointer is NULL");
        return HAL_ERROR;
    }

    *status = dma2d_status;
    return HAL_OK;
}

/**
 * @brief Check if DMA2D is busy
 */
bool DMA2D_IsBusy(void)
{
    return (hdma2d.State == HAL_DMA2D_STATE_BUSY);
}

/**
 * @brief Register transfer complete callback
 */
void DMA2D_RegisterTransferCompleteCallback(DMA2D_TransferCompleteCallback callback)
{
    transfer_complete_callback = callback;
    log_debug("DMA2D transfer complete callback registered");
}

/**
 * @brief Register transfer error callback
 */
void DMA2D_RegisterTransferErrorCallback(DMA2D_TransferErrorCallback callback)
{
    transfer_error_callback = callback;
    log_debug("DMA2D transfer error callback registered");
}

/**
 * @brief Register transfer progress callback
 */
void DMA2D_RegisterTransferProgressCallback(DMA2D_TransferProgressCallback callback)
{
    transfer_progress_callback = callback;
    log_debug("DMA2D transfer progress callback registered");
}

/* Utility Functions ---------------------------------------------------------*/

/**
 * @brief Convert RGB888 color to ARGB8888 format
 */
uint32_t DMA2D_MakeColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
    return ((uint32_t)alpha << 24) | ((uint32_t)red << 16) | ((uint32_t)green << 8) | blue;
}

/**
 * @brief Extract color components from ARGB8888 color
 */
void DMA2D_GetColorComponents(uint32_t color, uint8_t *red, uint8_t *green, uint8_t *blue, uint8_t *alpha)
{
    if (alpha) *alpha = (color >> 24) & 0xFF;
    if (red)   *red   = (color >> 16) & 0xFF;
    if (green) *green = (color >> 8)  & 0xFF;
    if (blue)  *blue  = color & 0xFF;
}

/**
 * @brief Validate DMA2D configuration parameters
 */
HAL_StatusTypeDef DMA2D_ValidateConfig(const DMA2D_Config *config)
{
    return DMA2D_ValidateConfigInternal(config);
}

/**
 * @brief Validate DMA2D layer configuration parameters
 */
HAL_StatusTypeDef DMA2D_ValidateLayerConfig(const DMA2D_LayerConfig *layer_config)
{
    return DMA2D_ValidateLayerConfigInternal(layer_config);
}

/**
 * @brief Get string representation of DMA2D error code
 */
const char* DMA2D_GetErrorString(HAL_StatusTypeDef error_code)
{
    return DMA2D_GetErrorStringInternal(error_code);
}

/**
 * @brief Get string representation of DMA2D state
 */
const char* DMA2D_GetStateString(uint32_t state)
{
    return DMA2D_GetStateStringInternal(state);
}

/* Advanced Features ---------------------------------------------------------*/

/**
 * @brief Enable DMA2D hardware acceleration for LCD operations
 */
HAL_StatusTypeDef DMA2D_EnableLCDMode(void)
{
    log_info("Enabling DMA2D LCD mode");

    if (!dma2d_status.initialized) {
        log_error("DMA2D not initialized");
        return HAL_ERROR;
    }

    /* Configure for optimal LCD operations */
    hdma2d.Init.OutputOffset = 0;
    hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565; // Common LCD format

    HAL_StatusTypeDef result = HAL_DMA2D_Init(&hdma2d);
    if (result != HAL_OK) {
        log_error("Failed to configure DMA2D for LCD mode: %s", DMA2D_GetErrorStringInternal(result));
        return result;
    }

    log_info("DMA2D LCD mode enabled");
    return HAL_OK;
}

/**
 * @brief Configure DMA2D for optimal SDRAM access
 */
HAL_StatusTypeDef DMA2D_EnableSDRAMMode(void)
{
    log_info("Enabling DMA2D SDRAM mode");

    if (!dma2d_status.initialized) {
        log_error("DMA2D not initialized");
        return HAL_ERROR;
    }

    /* Configure for optimal SDRAM access */
    // Note: Specific SDRAM optimizations would depend on the memory controller setup
    // This is a placeholder for SDRAM-specific configurations

    log_info("DMA2D SDRAM mode enabled");
    return HAL_OK;
}

/**
 * @brief Perform DMA2D self-test
 */
HAL_StatusTypeDef DMA2D_SelfTest(void)
{
    log_info("Starting DMA2D self-test");

    if (!dma2d_status.initialized) {
        log_error("DMA2D not initialized");
        return HAL_ERROR;
    }

    // Simple self-test: try to fill a small area
    uint32_t test_buffer[16] = {0};
    HAL_StatusTypeDef result = DMA2D_StartFill(DMA2D_COLOR_RED, test_buffer, 4, 4);

    if (result != HAL_OK) {
        log_error("DMA2D self-test failed: %s", DMA2D_GetErrorStringInternal(result));
        return result;
    }

    // Verify the fill operation
    for (int i = 0; i < 16; i++) {
        if (test_buffer[i] != DMA2D_COLOR_RED) {
            log_error("DMA2D self-test verification failed at index %d", i);
            return HAL_ERROR;
        }
    }

    log_info("DMA2D self-test passed");
    return HAL_OK;
}

/* Version Information -------------------------------------------------------*/

/**
 * @brief Get DMA2D driver version
 */
const char* DMA2D_GetVersion(void)
{
    return "2.0.0";
}

/**
 * @brief Get DMA2D driver capabilities
 */
uint32_t DMA2D_GetCapabilities(void)
{
    uint32_t capabilities = 0;

    // Basic capabilities
    capabilities |= (1 << 0); // R2M support
    capabilities |= (1 << 1); // M2M support
    capabilities |= (1 << 2); // M2M_PFC support
    capabilities |= (1 << 3); // M2M_BLEND support
    capabilities |= (1 << 4); // Interrupt mode support
    capabilities |= (1 << 5); // Polling mode support

    // Color format support
    capabilities |= (1 << 8);  // ARGB8888
    capabilities |= (1 << 9);  // RGB888
    capabilities |= (1 << 10); // RGB565
    capabilities |= (1 << 11); // ARGB1555
    capabilities |= (1 << 12); // ARGB4444

    return capabilities;
}

/* HAL DMA2D Callbacks ------------------------------------------------------*/

/**
 * @brief Transfer complete callback from HAL
 */
void HAL_DMA2D_TransferCompleteCallback(DMA2D_HandleTypeDef *hdma2d_handle)
{
    if (hdma2d_handle == &hdma2d) {
        DMA2D_UpdateStatus(HAL_OK);
        log_debug("DMA2D transfer completed");

        if (transfer_complete_callback) {
            transfer_complete_callback(hdma2d_handle);
        }
    }
}

/**
 * @brief Transfer error callback from HAL
 */
void HAL_DMA2D_TransferErrorCallback(DMA2D_HandleTypeDef *hdma2d_handle)
{
    if (hdma2d_handle == &hdma2d) {
        DMA2D_UpdateStatus(HAL_ERROR);
        log_error("DMA2D transfer error occurred");

        if (transfer_error_callback) {
            transfer_error_callback(hdma2d_handle);
        }
    }
}

/**
 * @brief Transfer progress callback from HAL (if supported)
 */
void HAL_DMA2D_TransferProgressCallback(DMA2D_HandleTypeDef *hdma2d_handle, uint32_t progress)
{
    if (hdma2d_handle == &hdma2d) {
        log_debug("DMA2D transfer progress: %lu%%", (unsigned long)progress);

        if (transfer_progress_callback) {
            transfer_progress_callback(hdma2d_handle, progress);
        }
    }
}
