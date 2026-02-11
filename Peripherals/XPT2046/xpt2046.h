/**
  ******************************************************************************
  * @file    xpt2046.h
  * @brief   XPT2046 Resistive Touchscreen Controller Driver Header
  * @details Compatible with ILI9488 4.0" TFT LCD Touch Screen Module
  *          Implements proper SPI communication protocol per XPT2046 datasheet
  * @version 2.0
  * @date    2025-02-10
  * @author  Rewritten for STM32F429I with proper datasheet compliance
  ******************************************************************************
  * @note    XPT2046 Specifications:
  *          - 12-bit ADC resolution (4096 levels)
  *          - SPI Mode 0 (CPOL=0, CPHA=0) or Mode 3 (CPOL=1, CPHA=1)
  *          - Max SPI clock: 2.5 MHz (conservative: 2 MHz recommended)
  *          - Supply voltage: 2.4V to 5.25V
  *          - Touch detection via IRQ pin (active low)
  *          - Integrated with ILI9488 LCD on 14-pin modules
  ******************************************************************************
  */

#ifndef XPT2046_H
#define XPT2046_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief XPT2046 status enumeration
 */
typedef enum {
    XPT2046_OK = 0,
    XPT2046_ERROR,
    XPT2046_TIMEOUT,
    XPT2046_INVALID_PARAM,
    XPT2046_NOT_INITIALIZED,
    XPT2046_NO_TOUCH
} XPT2046_StatusTypeDef;

/**
 * @brief Touch state enumeration
 */
typedef enum {
    XPT2046_STATE_RELEASED = 0,
    XPT2046_STATE_PRESSED,
    XPT2046_STATE_HELD
} XPT2046_TouchState_t;

/**
 * @brief Calibration parameters
 * @note  Default values work for most ILI9488 4.0" modules but may need tuning
 */
typedef struct {
    uint16_t x_min;                 /**< Minimum X ADC value (typical: 200-400) */
    uint16_t x_max;                 /**< Maximum X ADC value (typical: 3700-3900) */
    uint16_t y_min;                 /**< Minimum Y ADC value (typical: 200-400) */
    uint16_t y_max;                 /**< Maximum Y ADC value (typical: 3700-3900) */
    uint16_t pressure_threshold;    /**< Minimum pressure to register touch (typical: 200) */
} XPT2046_Calibration_t;

/**
 * @brief XPT2046 configuration structure
 */
typedef struct {
    GPIO_TypeDef *cs_port;          /**< Chip select GPIO port */
    uint16_t cs_pin;                /**< Chip select GPIO pin */
    GPIO_TypeDef *irq_port;         /**< Interrupt GPIO port */
    uint16_t irq_pin;               /**< Interrupt GPIO pin */
    uint16_t width;                 /**< Display width */
    uint16_t height;                /**< Display height */
    bool flip_x;                    /**< Flip X coordinate */
    bool flip_y;                    /**< Flip Y coordinate */
    bool swap_xy;                   /**< Swap X and Y coordinates */
} XPT2046_Config_t;

/**
 * @brief Touch point structure
 */
typedef struct {
    uint16_t x;                     /**< X coordinate */
    uint16_t y;                     /**< Y coordinate */
    uint16_t pressure;              /**< Touch pressure */
    XPT2046_TouchState_t state;     /**< Touch state */
} XPT2046_TouchPoint_t;

/**
 * @brief XPT2046 handle structure
 */
typedef struct {
    XPT2046_Config_t config;        /**< Configuration */
    XPT2046_Calibration_t cal;      /**< Calibration parameters */
    XPT2046_TouchPoint_t touch;     /**< Current touch point */
    uint32_t last_touch_time;       /**< Last touch timestamp */
    bool initialized;               /**< Initialization flag */
} XPT2046_Handle_t;

/* Exported constants --------------------------------------------------------*/

/** @defgroup XPT2046_Exported_Constants XPT2046 Constants
 * @{
 */

/* XPT2046 Hardware Limits */
#define XPT2046_MAX_X           4095    /**< Maximum X ADC value (12-bit) */
#define XPT2046_MAX_Y           4095    /**< Maximum Y ADC value (12-bit) */
#define XPT2046_MAX_PRESSURE    4095    /**< Maximum pressure value (12-bit) */

/* XPT2046 Default Calibration for ILI9488 4.0" LCD (480x320) */
#define XPT2046_DEFAULT_X_MIN   300     /**< Default X minimum raw value */
#define XPT2046_DEFAULT_X_MAX   3800    /**< Default X maximum raw value */
#define XPT2046_DEFAULT_Y_MIN   300     /**< Default Y minimum raw value */
#define XPT2046_DEFAULT_Y_MAX   3800    /**< Default Y maximum raw value */
#define XPT2046_DEFAULT_PRESSURE_THRESHOLD  250  /**< Default pressure threshold */

/* XPT2046 Timing Constants */
#define XPT2046_SETTLE_TIME_US  10      /**< ADC settling time after command (μs) */
#define XPT2046_MAX_SPI_FREQ    2000000 /**< Maximum SPI frequency (2 MHz recommended) */

/**
 * @}
 */

/* Exported functions --------------------------------------------------------*/

/** @defgroup XPT2046_Exported_Functions XPT2046 Functions
 * @{
 */

/**
 * @brief   Initialize XPT2046 touchscreen controller
 * @param   hxpt Pointer to XPT2046 handle
 * @param   cs_port Chip select GPIO port
 * @param   cs_pin Chip select GPIO pin
 * @param   irq_port Interrupt GPIO port
 * @param   irq_pin Interrupt GPIO pin
 * @param   width Display width in pixels
 * @param   height Display height in pixels
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_Init(XPT2046_Handle_t *hxpt,
                                  GPIO_TypeDef *cs_port, uint16_t cs_pin,
                                  GPIO_TypeDef *irq_port, uint16_t irq_pin,
                                  uint16_t width, uint16_t height);

/**
 * @brief   Check if touchscreen is currently touched
 * @param   hxpt Pointer to XPT2046 handle
 * @retval  bool True if touched, false otherwise
 */
bool XPT2046_IsTouched(XPT2046_Handle_t *hxpt);

/**
 * @brief   Read current touch position and pressure
 * @param   hxpt Pointer to XPT2046 handle
 * @param   touch Pointer to touch point structure
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_ReadTouch(XPT2046_Handle_t *hxpt,
                                       XPT2046_TouchPoint_t *touch);

/**
 * @brief   Update touch state (call periodically)
 * @param   hxpt Pointer to XPT2046 handle
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_Update(XPT2046_Handle_t *hxpt);

/**
 * @brief   Set calibration parameters
 * @param   hxpt Pointer to XPT2046 handle
 * @param   x_min Minimum X ADC value
 * @param   x_max Maximum X ADC value
 * @param   y_min Minimum Y ADC value
 * @param   y_max Maximum Y ADC value
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_SetCalibration(XPT2046_Handle_t *hxpt,
                                            uint16_t x_min, uint16_t x_max,
                                            uint16_t y_min, uint16_t y_max);

/**
 * @brief   Configure touch transformations
 * @param   hxpt Pointer to XPT2046 handle
 * @param   swap_xy Swap X and Y coordinates
 * @param   flip_x Flip X coordinate
 * @param   flip_y Flip Y coordinate
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_SetTransform(XPT2046_Handle_t *hxpt,
                                          bool swap_xy, bool flip_x, bool flip_y);

/**
 * @brief   MSP initialization hook (weak, can be overridden)
 * @param   cs_port Chip select port
 * @param   cs_pin Chip select pin
 * @param   irq_port Interrupt port
 * @param   irq_pin Interrupt pin
 */
void XPT2046_MspInit(GPIO_TypeDef *cs_port, uint16_t cs_pin,
                    GPIO_TypeDef *irq_port, uint16_t irq_pin);

/**
 * @brief   MSP de-initialization hook (weak, can be overridden)
 * @param   cs_port Chip select port
 * @param   cs_pin Chip select pin
 * @param   irq_port Interrupt port
 * @param   irq_pin Interrupt pin
 */
void XPT2046_MspDeInit(GPIO_TypeDef *cs_port, uint16_t cs_pin,
                      GPIO_TypeDef *irq_port, uint16_t irq_pin);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* XPT2046_H */
