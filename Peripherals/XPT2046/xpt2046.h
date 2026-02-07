/**
  ******************************************************************************
  * @file    xpt2046.h
  * @brief   XPT2046 Resistive Touchscreen Controller Driver for STM32F429I-DISC1
  * @details This file contains function prototypes and definitions for
  *          XPT2046 resistive touchscreen controller using SPI interface.
  * @version 1.0
  * @date    2025-01-19
  ******************************************************************************
  */

#ifndef __XPT2046_H__
#define __XPT2046_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "spi.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported constants --------------------------------------------------------*/

/** @defgroup XPT2046_Display_Specifications Specifications
 * @{
 */
#define XPT2046_MAX_X                  4095    /**< Maximum X coordinate */
#define XPT2046_MAX_Y                  4095    /**< Maximum Y coordinate */
#define XPT2046_MIN_PRESSURE           10      /**< Minimum pressure threshold */
#define XPT2046_MAX_PRESSURE           4095    /**< Maximum pressure value */

/* Touch states */
#define XPT2046_STATE_RELEASED         0       /**< Touch released */
#define XPT2046_STATE_PRESSED          1       /**< Touch pressed */
#define XPT2046_STATE_HELD             2       /**< Touch held */

/** @} */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief XPT2046 Status enumeration
 */
typedef enum {
    XPT2046_OK = 0,                 /**< Operation completed successfully */
    XPT2046_ERROR,                  /**< General error occurred */
    XPT2046_BUSY,                   /**< Controller is busy */
    XPT2046_TIMEOUT,                /**< Operation timed out */
    XPT2046_INVALID_PARAM,          /**< Invalid parameter provided */
    XPT2046_NOT_INITIALIZED,        /**< Driver not initialized */
    XPT2046_NO_TOUCH                /**< No touch detected */
} XPT2046_StatusTypeDef;

/**
 * @brief XPT2046 Touch point structure
 */
typedef struct {
    uint16_t x;                     /**< X coordinate */
    uint16_t y;                     /**< Y coordinate */
    uint16_t pressure;              /**< Touch pressure */
    uint8_t state;                  /**< Touch state */
} XPT2046_TouchPoint_t;

/**
 * @brief XPT2046 Configuration structure
 */
typedef struct {
    GPIO_TypeDef *cs_port;          /**< Chip select port */
    uint16_t cs_pin;                /**< Chip select pin */
    GPIO_TypeDef *irq_port;         /**< Interrupt port */
    uint16_t irq_pin;               /**< Interrupt pin */
    uint16_t width;                 /**< Display width for coordinate mapping */
    uint16_t height;                /**< Display height for coordinate mapping */
    bool flip_x;                    /**< Flip X coordinates */
    bool flip_y;                    /**< Flip Y coordinates */
    uint16_t calibration[7];        /**< Calibration matrix (6 coefficients + offset) */
} XPT2046_Config_t;

/**
 * @brief XPT2046 Handle structure
 */
typedef struct {
    XPT2046_Config_t config;        /**< Configuration */
    XPT2046_TouchPoint_t touch;     /**< Current touch point */
    bool initialized;               /**< Initialization status */
    uint32_t last_touch_time;       /**< Last touch timestamp */
} XPT2046_Handle_t;

/**
 * @brief   Board support hooks (weak by default)
 * @details Override these to configure GPIO clocks for CS/IRQ pins.
 */
void XPT2046_MspInit(GPIO_TypeDef *cs_port, uint16_t cs_pin,
                     GPIO_TypeDef *irq_port, uint16_t irq_pin);
void XPT2046_MspDeInit(GPIO_TypeDef *cs_port, uint16_t cs_pin,
                       GPIO_TypeDef *irq_port, uint16_t irq_pin);

/* Exported functions -------------------------------------------------------*/

/** @defgroup XPT2046_Init Initialization and Configuration
 * @{
 */

/**
 * @brief   Initialize XPT2046 touchscreen controller
 * @details Configures SPI and initializes the touchscreen
 * @param   hxpt Pointer to XPT2046 handle
 * @param   cs_port Chip select port
 * @param   cs_pin Chip select pin
 * @param   irq_port Interrupt port
 * @param   irq_pin Interrupt pin
 * @param   width Display width
 * @param   height Display height
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_Init(XPT2046_Handle_t *hxpt,
                                  GPIO_TypeDef *cs_port, uint16_t cs_pin,
                                  GPIO_TypeDef *irq_port, uint16_t irq_pin,
                                  uint16_t width, uint16_t height);

/**
 * @brief   Deinitialize XPT2046 touchscreen controller
 * @details Releases resources and disables the touchscreen
 * @param   hxpt Pointer to XPT2046 handle
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_DeInit(XPT2046_Handle_t *hxpt);

/**
 * @brief   Configure XPT2046 touchscreen parameters
 * @details Sets touchscreen configuration options
 * @param   hxpt Pointer to XPT2046 handle
 * @param   config Pointer to configuration structure
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_Config(XPT2046_Handle_t *hxpt, XPT2046_Config_t *config);

/**
 * @brief   Set calibration matrix
 * @param   hxpt Pointer to XPT2046 handle
 * @param   calibration Calibration matrix (7 values)
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_SetCalibration(XPT2046_Handle_t *hxpt, uint16_t *calibration);

/** @} */

/** @defgroup XPT2046_Touch Touch Detection and Reading
 * @{
 */

/**
 * @brief   Check if touchscreen is touched
 * @param   hxpt Pointer to XPT2046 handle
 * @retval  bool True if touched, false otherwise
 */
bool XPT2046_IsTouched(XPT2046_Handle_t *hxpt);

/**
 * @brief   Read touch coordinates and pressure
 * @param   hxpt Pointer to XPT2046 handle
 * @param   touch Pointer to touch point structure
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_ReadTouch(XPT2046_Handle_t *hxpt, XPT2046_TouchPoint_t *touch);

/**
 * @brief   Update touch state
 * @param   hxpt Pointer to XPT2046 handle
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_Update(XPT2046_Handle_t *hxpt);

/**
 * @brief   Get current touch point
 * @param   hxpt Pointer to XPT2046 handle
 * @param   touch Pointer to touch point structure to fill
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_GetTouch(XPT2046_Handle_t *hxpt, XPT2046_TouchPoint_t *touch);

/** @} */

/** @defgroup XPT2046_Calibration Touchscreen Calibration
 * @{
 */

/**
 * @brief   Start calibration process
 * @param   hxpt Pointer to XPT2046 handle
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_StartCalibration(XPT2046_Handle_t *hxpt);

/**
 * @brief   Get calibration point
 * @param   hxpt Pointer to XPT2046 handle
 * @param   point_index Calibration point index (0-4)
 * @param   x_display Expected X coordinate on display
 * @param   y_display Expected Y coordinate on display
 * @param   x_touch Pointer to store raw X touch value
 * @param   y_touch Pointer to store raw Y touch value
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_GetCalibrationPoint(XPT2046_Handle_t *hxpt,
                                                 uint8_t point_index,
                                                 uint16_t x_display, uint16_t y_display,
                                                 uint16_t *x_touch, uint16_t *y_touch);

/**
 * @brief   Compute calibration matrix
 * @param   hxpt Pointer to XPT2046 handle
 * @param   points Array of calibration points (5 points, each with display and touch coordinates)
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_ComputeCalibration(XPT2046_Handle_t *hxpt,
                                                uint16_t points[5][4]); // [point][x_display, y_display, x_touch, y_touch]

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __XPT2046_H__ */
