/**
  ******************************************************************************
  * @file    ov7670.h
  * @brief   OV7670 CMOS Camera Sensor Driver for STM32F429I-DISC1
  * @details This file contains function prototypes and definitions for
  *          OV7670 CMOS camera sensor using I2C interface for control
  *          and DCMI interface for video data capture.
  * @version 1.0
  * @date    2026-01-20
  ******************************************************************************
  */

#ifndef __OV7670_H__
#define __OV7670_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "i2c.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported constants --------------------------------------------------------*/

/** @defgroup OV7670_Specifications Specifications
 * @{
 */
#define OV7670_I2C_ADDRESS            0x42    /**< OV7670 I2C address (7-bit: 0x21) */
#define OV7670_ID                     0x7670  /**< OV7670 chip ID */

#define OV7670_MAX_WIDTH              640     /**< Maximum image width */
#define OV7670_MAX_HEIGHT             480     /**< Maximum image height */

#define OV7670_QVGA_WIDTH             320     /**< QVGA width */
#define OV7670_QVGA_HEIGHT            240     /**< QVGA height */
#define OV7670_QQVGA_WIDTH            160     /**< QQVGA width */
#define OV7670_QQVGA_HEIGHT           120     /**< QQVGA height */

/* Color formats */
#define OV7670_FORMAT_RGB565          0x00    /**< RGB565 format */
#define OV7670_FORMAT_RGB555          0x01    /**< RGB555 format */
#define OV7670_FORMAT_YUV422          0x02    /**< YUV422 format */
#define OV7670_FORMAT_GRAYSCALE       0x03    /**< Grayscale format */

/* Test patterns */
#define OV7670_TEST_PATTERN_NONE      0x00    /**< No test pattern */
#define OV7670_TEST_PATTERN_1         0x01    /**< Test pattern 1 */
#define OV7670_TEST_PATTERN_2         0x02    /**< Test pattern 2 */
#define OV7670_TEST_PATTERN_BARS      0x03    /**< Color bars */

/** @} */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief OV7670 Status enumeration
 */
typedef enum {
    OV7670_OK = 0,                  /**< Operation completed successfully */
    OV7670_ERROR,                   /**< General error occurred */
    OV7670_BUSY,                    /**< Sensor is busy */
    OV7670_TIMEOUT,                 /**< Operation timed out */
    OV7670_INVALID_PARAM,           /**< Invalid parameter provided */
    OV7670_NOT_INITIALIZED,         /**< Driver not initialized */
    OV7670_I2C_ERROR,               /**< I2C communication error */
    OV7670_INVALID_ID               /**< Invalid sensor ID */
} OV7670_StatusTypeDef;

/**
 * @brief OV7670 Resolution enumeration
 */
typedef enum {
    OV7670_RES_QQVGA = 0,           /**< 160x120 QQVGA */
    OV7670_RES_QVGA,                /**< 320x240 QVGA */
    OV7670_RES_VGA                  /**< 640x480 VGA */
} OV7670_ResolutionTypeDef;

/**
 * @brief OV7670 Color format enumeration
 */
typedef enum {
    OV7670_FMT_RGB565 = 0,          /**< RGB565 */
    OV7670_FMT_RGB555,              /**< RGB555 */
    OV7670_FMT_YUV422,              /**< YUV422 */
    OV7670_FMT_GRAYSCALE            /**< Grayscale */
} OV7670_FormatTypeDef;

/**
 * @brief OV7670 Configuration structure
 */
typedef struct {
    OV7670_ResolutionTypeDef resolution;    /**< Image resolution */
    OV7670_FormatTypeDef format;            /**< Color format */
    uint8_t brightness;                      /**< Brightness (0-255) */
    uint8_t contrast;                        /**< Contrast (0-255) */
    uint8_t saturation;                      /**< Saturation (0-255) */
    bool flip_horizontal;                    /**< Horizontal flip */
    bool flip_vertical;                      /**< Vertical flip */
    bool night_mode;                         /**< Night mode enable */
    uint8_t test_pattern;                    /**< Test pattern mode */
} OV7670_Config_t;

/**
 * @brief OV7670 Handle structure
 */
typedef struct {
    I2C_HandleTypeDef *hi2c;         /**< I2C handle */
    DCMI_HandleTypeDef *hdcmi;       /**< DCMI handle */
    OV7670_Config_t config;          /**< Current configuration */
    bool initialized;                /**< Initialization status */
    uint16_t chip_id;                /**< Chip ID */
} OV7670_Handle_t;

/* Exported functions -------------------------------------------------------*/

/** @defgroup OV7670_Init Initialization and Configuration
 * @{
 */

/**
 * @brief   Initialize OV7670 camera sensor
 * @details Configures I2C and DCMI interfaces and initializes the camera
 * @param   hov7670 Pointer to OV7670 handle
 * @param   hi2c I2C handle for camera control
 * @param   hdcmi DCMI handle for video capture
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_Init(OV7670_Handle_t *hov7670,
                                I2C_HandleTypeDef *hi2c,
                                DCMI_HandleTypeDef *hdcmi);

/**
 * @brief   Deinitialize OV7670 camera sensor
 * @details Releases resources and powers down the camera
 * @param   hov7670 Pointer to OV7670 handle
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_DeInit(OV7670_Handle_t *hov7670);

/**
 * @brief   Configure OV7670 camera parameters
 * @details Sets camera configuration options
 * @param   hov7670 Pointer to OV7670 handle
 * @param   config Pointer to configuration structure
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_Config(OV7670_Handle_t *hov7670, OV7670_Config_t *config);

/**
 * @brief   Reset OV7670 camera sensor
 * @details Performs software reset of the camera
 * @param   hov7670 Pointer to OV7670 handle
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_Reset(OV7670_Handle_t *hov7670);

/** @} */

/** @defgroup OV7670_Capture Image Capture
 * @{
 */

/**
 * @brief   Start image capture
 * @details Begins capturing image data to the specified buffer
 * @param   hov7670 Pointer to OV7670 handle
 * @param   buffer Destination buffer for image data
 * @param   length Buffer length in bytes
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_StartCapture(OV7670_Handle_t *hov7670,
                                        uint32_t *buffer, uint32_t length);

/**
 * @brief   Stop image capture
 * @details Stops the current capture operation
 * @param   hov7670 Pointer to OV7670 handle
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_StopCapture(OV7670_Handle_t *hov7670);

/**
 * @brief   Check if capture is complete
 * @param   hov7670 Pointer to OV7670 handle
 * @retval  bool True if capture complete, false otherwise
 */
bool OV7670_IsCaptureComplete(OV7670_Handle_t *hov7670);

/** @} */

/** @defgroup OV7670_Control Camera Control
 * @{
 */

/**
 * @brief   Set camera resolution
 * @param   hov7670 Pointer to OV7670 handle
 * @param   resolution New resolution
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_SetResolution(OV7670_Handle_t *hov7670,
                                         OV7670_ResolutionTypeDef resolution);

/**
 * @brief   Set color format
 * @param   hov7670 Pointer to OV7670 handle
 * @param   format New color format
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_SetFormat(OV7670_Handle_t *hov7670,
                                     OV7670_FormatTypeDef format);

/**
 * @brief   Set brightness
 * @param   hov7670 Pointer to OV7670 handle
 * @param   brightness Brightness value (0-255)
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_SetBrightness(OV7670_Handle_t *hov7670, uint8_t brightness);

/**
 * @brief   Set contrast
 * @param   hov7670 Pointer to OV7670 handle
 * @param   contrast Contrast value (0-255)
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_SetContrast(OV7670_Handle_t *hov7670, uint8_t contrast);

/**
 * @brief   Set saturation
 * @param   hov7670 Pointer to OV7670 handle
 * @param   saturation Saturation value (0-255)
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_SetSaturation(OV7670_Handle_t *hov7670, uint8_t saturation);

/**
 * @brief   Enable/disable horizontal flip
 * @param   hov7670 Pointer to OV7670 handle
 * @param   enable True to enable flip, false to disable
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_SetFlipHorizontal(OV7670_Handle_t *hov7670, bool enable);

/**
 * @brief   Enable/disable vertical flip
 * @param   hov7670 Pointer to OV7670 handle
 * @param   enable True to enable flip, false to disable
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_SetFlipVertical(OV7670_Handle_t *hov7670, bool enable);

/**
 * @brief   Enable/disable night mode
 * @param   hov7670 Pointer to OV7670 handle
 * @param   enable True to enable night mode, false to disable
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_SetNightMode(OV7670_Handle_t *hov7670, bool enable);

/**
 * @brief   Set test pattern
 * @param   hov7670 Pointer to OV7670 handle
 * @param   pattern Test pattern mode
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_SetTestPattern(OV7670_Handle_t *hov7670, uint8_t pattern);

/** @} */

/** @defgroup OV7670_Status Status and Information
 * @{
 */

/**
 * @brief   Get camera chip ID
 * @param   hov7670 Pointer to OV7670 handle
 * @param   chip_id Pointer to store chip ID
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_GetChipID(OV7670_Handle_t *hov7670, uint16_t *chip_id);

/**
 * @brief   Get current camera status
 * @param   hov7670 Pointer to OV7670 handle
 * @retval  OV7670_StatusTypeDef Current status
 */
OV7670_StatusTypeDef OV7670_GetStatus(OV7670_Handle_t *hov7670);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __OV7670_H__ */
