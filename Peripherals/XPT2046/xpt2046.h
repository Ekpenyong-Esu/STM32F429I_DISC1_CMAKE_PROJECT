/**
  ******************************************************************************
  * @file    xpt2046.h
  * @brief   XPT2046 Resistive Touchscreen driver interface for STM32F429 Discovery Board
  * @details This file contains function prototypes and definitions for
  *          the XPT2046 resistive touchscreen controller using SPI interface.
  *          Based on touchscreen.h pattern for STMPE811 but adapted for XPT2046.
  * @version 1.0
  * @date    2025-02-11
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
#define XPT2046_TIMEOUT                 1000U
#define XPT2046_MAX_TOUCHES             1       /* Single touch support */

/* XPT2046 Constants */
#define XPT2046_MAX_X                   4095    /* 12-bit ADC resolution */
#define XPT2046_MAX_Y                   4095    /* 12-bit ADC resolution */
#define XPT2046_MIN_PRESSURE            5      /* Minimum pressure threshold */

/* Display mapping constants - adjust to your display (Native resolution) */
#define XPT2046_DISPLAY_WIDTH           320     /* Native Width (Portrait) */
#define XPT2046_DISPLAY_HEIGHT          480     /* Native Height (Portrait) */

/* XPT2046 Command Definitions (12-bit ADC, differential reference) */
#define XPT2046_CMD_START               0x80    /* Start bit */
#define XPT2046_CMD_12BIT               0x00    /* 12-bit conversion */
#define XPT2046_CMD_8BIT                0x08    /* 8-bit conversion */
#define XPT2046_CMD_DIFF                0x00    /* Differential reference */
#define XPT2046_CMD_SINGLE              0x04    /* Single-ended reference */
#define XPT2046_CMD_POWERDOWN_DISABLE   0x00    /* Power down between conversions */
#define XPT2046_CMD_POWERDOWN_REF_OFF   0x01    /* Ref off, ADC on */
#define XPT2046_CMD_POWERDOWN_REF_ON    0x02    /* Ref on, ADC off */
#define XPT2046_CMD_POWERDOWN_ALWAYS_ON 0x03    /* Always powered */

/* Channel select commands */
#define XPT2046_CHANNEL_X               0x50    /* X position (Y+ to GND) */
#define XPT2046_CHANNEL_Y               0x10    /* Y position (X+ to GND) */
#define XPT2046_CHANNEL_Z1              0x30    /* Z1 pressure */
#define XPT2046_CHANNEL_Z2              0x40    /* Z2 pressure */
#define XPT2046_CHANNEL_TEMP0           0x00    /* Temperature 0 */
#define XPT2046_CHANNEL_TEMP1           0x70    /* Temperature 1 */
#define XPT2046_CHANNEL_VBAT            0x20    /* Battery voltage */
#define XPT2046_CHANNEL_AUX             0x60    /* Auxiliary input */

/* Complete command bytes for common operations */
#define XPT2046_CMD_READ_X      (XPT2046_CMD_START | XPT2046_CHANNEL_X | XPT2046_CMD_12BIT)
#define XPT2046_CMD_READ_Y      (XPT2046_CMD_START | XPT2046_CHANNEL_Y | XPT2046_CMD_12BIT)
#define XPT2046_CMD_READ_Z1     (XPT2046_CMD_START | XPT2046_CHANNEL_Z1 | XPT2046_CMD_12BIT)
#define XPT2046_CMD_READ_Z2     (XPT2046_CMD_START | XPT2046_CHANNEL_Z2 | XPT2046_CMD_12BIT)

/* Calibration and filtering */
#define XPT2046_SAMPLES                 8       /* Number of samples for averaging */
#define XPT2046_DEBOUNCE_COUNT          3       /* Debounce count for stable reading */
#define XPT2046_SMOOTHING_THRESHOLD     10      /* Coordinate smoothing threshold */
#define XPT2046_GESTURE_THRESHOLD       20      /* Minimum movement for gesture */
#define XPT2046_LONG_PRESS_TIME         1000    /* Minimum time for long press (ms) */

/* Default calibration values (raw touch ranges)
 * Updated to measured corner values (user-provided) for portrait orientation.
 * Left/top raw values ~511..559, right/top raw values ~3136..3292,
 * vertical raw min/max observed ~479..3784.
 */
#define XPT2046_RAW_X_MIN               200     /* Measured minimum X (left) */
#define XPT2046_RAW_X_MAX               3900   /* Measured maximum X (right) */
#define XPT2046_RAW_Y_MIN               200     /* Measured minimum Y (bottom) */
#define XPT2046_RAW_Y_MAX               3900    /* Measured maximum Y (top) */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief XPT2046 Status enumeration
 */
typedef enum {
    XPT2046_OK = 0,                     /**< Operation completed successfully */
    XPT2046_ERROR,                      /**< General error occurred */
    XPT2046_BUSY,                       /**< Touch controller is busy */
    XPT2046_TIMEOUT_ERROR,              /**< Operation timed out */
    XPT2046_INVALID_PARAM,              /**< Invalid parameter provided */
    XPT2046_NOT_INITIALIZED,            /**< Device not initialized */
    XPT2046_COMMUNICATION_ERROR,        /**< SPI communication error */
    XPT2046_NO_TOUCH                    /**< No touch detected */
} XPT2046_StatusTypeDef;

/**
 * @brief Touch state enumeration
 */
typedef enum {
    XPT2046_TOUCH_RELEASED = 0,         /**< No touch detected */
    XPT2046_TOUCH_PRESSED,              /**< Touch detected */
    XPT2046_TOUCH_MOVING                /**< Touch moving */
} XPT2046_TouchStateTypeDef;

/**
 * @brief Touch gesture enumeration
 */
typedef enum {
    XPT2046_GESTURE_NONE = 0,           /**< No gesture */
    XPT2046_GESTURE_TAP,                /**< Single tap */
    XPT2046_GESTURE_DOUBLE_TAP,         /**< Double tap */
    XPT2046_GESTURE_LONG_PRESS,         /**< Long press */
    XPT2046_GESTURE_SWIPE_UP,           /**< Swipe up */
    XPT2046_GESTURE_SWIPE_DOWN,         /**< Swipe down */
    XPT2046_GESTURE_SWIPE_LEFT,         /**< Swipe left */
    XPT2046_GESTURE_SWIPE_RIGHT         /**< Swipe right */
} XPT2046_GestureTypeDef;

/**
 * @brief Touch point structure
 */
typedef struct {
    uint16_t X;                         /**< X coordinate (display space) */
    uint16_t Y;                         /**< Y coordinate (display space) */
    uint16_t Z;                         /**< Pressure (Z coordinate) */
    uint16_t RawX;                      /**< Raw X coordinate (ADC value) */
    uint16_t RawY;                      /**< Raw Y coordinate (ADC value) */
    XPT2046_TouchStateTypeDef State;    /**< Touch state */
    uint32_t Timestamp;                 /**< Touch timestamp */
} XPT2046_TouchPointTypeDef;

/**
 * @brief Touch data structure
 */
typedef struct {
    uint8_t TouchCount;                 /**< Number of active touches */
    XPT2046_TouchPointTypeDef Points[XPT2046_MAX_TOUCHES]; /**< Touch points */
    XPT2046_GestureTypeDef Gesture;     /**< Detected gesture */
    uint32_t GestureTimestamp;          /**< Gesture timestamp */
} XPT2046_TouchDataTypeDef;

/**
 * @brief Calibration data structure
 */
typedef struct {
    uint16_t MinX;                      /**< Minimum raw X value */
    uint16_t MaxX;                      /**< Maximum raw X value */
    uint16_t MinY;                      /**< Minimum raw Y value */
    uint16_t MaxY;                      /**< Maximum raw Y value */
    float ScaleX;                       /**< X scaling factor */
    float ScaleY;                       /**< Y scaling factor */
    int16_t OffsetX;                    /**< X offset */
    int16_t OffsetY;                    /**< Y offset */
    bool SwapXY;                        /**< Swap X and Y coordinates */
    bool FlipX;                         /**< Flip X coordinate */
    bool FlipY;                         /**< Flip Y coordinate */
    bool IsCalibrated;                  /**< Calibration status */
} XPT2046_CalibrationTypeDef;

/**
 * @brief XPT2046 configuration structure
 */
typedef struct {
    uint8_t Samples;                    /**< Number of samples for averaging */
    uint16_t PressureThreshold;         /**< Pressure threshold */
    bool InterruptEnable;               /**< Interrupt enable */
    uint8_t DebounceCount;              /**< Debounce count */
    bool Use12Bit;                      /**< Use 12-bit mode (vs 8-bit) */
} XPT2046_ConfigTypeDef;

/**
 * @brief XPT2046 Handle structure
 */
typedef struct {
    SPI_HandleTypeDef *hspi;            /**< SPI handle */
    GPIO_TypeDef *CS_Port;              /**< Chip Select GPIO port */
    uint16_t CS_Pin;                    /**< Chip Select GPIO pin */
    GPIO_TypeDef *IRQ_Port;             /**< Interrupt GPIO port */
    uint16_t IRQ_Pin;                   /**< Interrupt GPIO pin */
    XPT2046_ConfigTypeDef Config;       /**< Configuration */
    XPT2046_CalibrationTypeDef Calibration; /**< Calibration data */
    XPT2046_TouchDataTypeDef TouchData; /**< Current touch data */
    XPT2046_TouchDataTypeDef PrevTouchData; /**< Previous touch data */
    bool IsInitialized;                 /**< Initialization status */
    bool InterruptMode;                 /**< Interrupt mode status (driver-managed) */
    uint32_t LastTouchTime;             /**< Last touch timestamp */
} XPT2046_HandleTypeDef;

/* Exported function prototypes ---------------------------------------------*/

/* Initialization and configuration functions */

/**
 * @brief   Initialize XPT2046 touchscreen controller
 * @details Configures SPI and GPIO pins for the touchscreen. If the SPI
 *          peripheral is shared with a faster device (for example ILI9488),
 *          the driver will temporarily lower the SPI baud-rate prescaler to
 *          a safe value during ADC reads and restore it afterwards.
 * @param   hxpt Pointer to XPT2046 handle structure
 * @param   hspi Pointer to SPI handle
 * @param   cs_port Chip select GPIO port
 * @param   cs_pin Chip select GPIO pin
 * @param   irq_port Interrupt GPIO port
 * @param   irq_pin Interrupt GPIO pin
 * @retval  XPT2046_StatusTypeDef Status of the operation
 */
XPT2046_StatusTypeDef XPT2046_Init(XPT2046_HandleTypeDef *hxpt,
                                  SPI_HandleTypeDef *hspi,
                                  GPIO_TypeDef *cs_port, uint16_t cs_pin,
                                  GPIO_TypeDef *irq_port, uint16_t irq_pin);


void XPT2046_PrintRawCoordinates(XPT2046_HandleTypeDef *hxpt);


/**
 * @brief   Deinitialize XPT2046 touchscreen controller
 * @param   hxpt Pointer to XPT2046 handle structure
 * @retval  XPT2046_StatusTypeDef Status of the operation
 */
XPT2046_StatusTypeDef XPT2046_DeInit(XPT2046_HandleTypeDef *hxpt);

/**
 * @brief   Configure XPT2046 touchscreen parameters
 * @param   hxpt Pointer to XPT2046 handle structure
 * @param   config Pointer to configuration structure
 * @retval  XPT2046_StatusTypeDef Status of the operation
 */
XPT2046_StatusTypeDef XPT2046_Configure(XPT2046_HandleTypeDef *hxpt,
                                       XPT2046_ConfigTypeDef *config);

/**
 * @brief   Reset XPT2046 touchscreen controller
 * @param   hxpt Pointer to XPT2046 handle structure
 * @retval  XPT2046_StatusTypeDef Status of the operation
 */
XPT2046_StatusTypeDef XPT2046_Reset(XPT2046_HandleTypeDef *hxpt);

/* Touch detection and reading functions */

/**
 * @brief   Get touch state with coordinates and pressed status
 * @param   hxpt Pointer to XPT2046 handle structure
 * @param   x Pointer to store X coordinate
 * @param   y Pointer to store Y coordinate
 * @param   pressed Pointer to store pressed status
 * @retval  XPT2046_StatusTypeDef Status of the operation
 */
XPT2046_StatusTypeDef XPT2046_GetTouchState(XPT2046_HandleTypeDef *hxpt,
                                           uint16_t *x,
                                           uint16_t *y,
                                           uint8_t *pressed);

/**
 * @brief   Check if touchscreen is currently touched
 * @param   hxpt Pointer to XPT2046 handle structure
 * @retval  bool True if touched, false otherwise
 */
bool XPT2046_IsTouched(XPT2046_HandleTypeDef *hxpt);

/* Calibration functions */

/**
 * @brief   Calibrate touchscreen
 * @param   hxpt Pointer to XPT2046 handle structure
 * @retval  XPT2046_StatusTypeDef Status of the operation
 */
XPT2046_StatusTypeDef XPT2046_Calibrate(XPT2046_HandleTypeDef *hxpt);

/**
 * @brief   Set calibration data
 * @param   hxpt Pointer to XPT2046 handle structure
 * @param   calibration Pointer to calibration data
 * @retval  XPT2046_StatusTypeDef Status of the operation
 */
XPT2046_StatusTypeDef XPT2046_SetCalibration(XPT2046_HandleTypeDef *hxpt,
                                            XPT2046_CalibrationTypeDef *calibration);

/**
 * @brief   Get calibration data
 * @param   hxpt Pointer to XPT2046 handle structure
 * @param   calibration Pointer to store calibration data
 * @retval  XPT2046_StatusTypeDef Status of the operation
 */
XPT2046_StatusTypeDef XPT2046_GetCalibration(XPT2046_HandleTypeDef *hxpt,
                                            XPT2046_CalibrationTypeDef *calibration);

/* Gesture recognition is handled internally by the driver when needed. */

/* Interrupt functions (minimal) */

/**
 * @brief   Enable/disable interrupt handling for the touch IRQ
 * @param   hxpt Pointer to XPT2046 handle
 * @param   enable Enable/disable flag
 * @retval  XPT2046_StatusTypeDef
 */
XPT2046_StatusTypeDef XPT2046_EnableInterrupt(XPT2046_HandleTypeDef *hxpt, bool enable);

/**
 * @brief   Ensure EXTI/NVIC is configured for touch IRQ
 * @param   hxpt Pointer to XPT2046 handle
 * @retval  XPT2046_StatusTypeDef
 */
XPT2046_StatusTypeDef XPT2046_ITConfig(XPT2046_HandleTypeDef *hxpt);

/**
 * @brief   Service pending touch IRQs (call from main loop or LVGL task)
 * @note    ISR only sets a pending flag; call this to perform SPI reads and
 *          update driver's cached touch state.
 */
void XPT2046_ServiceIRQ(void);



/* Callbacks are not supported in the simplified driver. */

/* Utility functions */

/**
 * @brief   Get pressure value
 * @param   hxpt Pointer to XPT2046 handle structure
 * @param   pressure Pointer to store pressure value
 * @retval  XPT2046_StatusTypeDef Status of the operation
 */
XPT2046_StatusTypeDef XPT2046_GetPressure(XPT2046_HandleTypeDef *hxpt,
                                         uint16_t *pressure);

/**
 * @brief   Set pressure threshold
 * @param   hxpt Pointer to XPT2046 handle structure
 * @param   threshold Pressure threshold value
 * @retval  XPT2046_StatusTypeDef Status of the operation
 */
XPT2046_StatusTypeDef XPT2046_SetThreshold(XPT2046_HandleTypeDef *hxpt,
                                          uint16_t threshold);

/* Configuration helpers */

/**
 * @brief   Get default configuration
 * @retval  XPT2046_ConfigTypeDef Default configuration structure
 */
XPT2046_ConfigTypeDef XPT2046_GetDefaultConfig(void);

/* Board-specific MSP functions (implemented in xpt2046_board.c) */

/**
 * @brief   Initialize MSP (GPIO pins and clocks)
 * @param   cs_port Chip select port
 * @param   cs_pin Chip select pin
 * @param   irq_port Interrupt port
 * @param   irq_pin Interrupt pin
 */
void XPT2046_MspInit(GPIO_TypeDef *cs_port, uint16_t cs_pin,
                    GPIO_TypeDef *irq_port, uint16_t irq_pin);

/**
 * @brief   Deinitialize MSP
 * @param   cs_port Chip select port
 * @param   cs_pin Chip select pin
 * @param   irq_port Interrupt port
 * @param   irq_pin Interrupt pin
 */
void XPT2046_MspDeInit(GPIO_TypeDef *cs_port, uint16_t cs_pin,
                      GPIO_TypeDef *irq_port, uint16_t irq_pin);

/* Global touchscreen handle removed (no deferred IRQ handling) */

#ifdef __cplusplus
}
#endif

#endif /* __XPT2046_H__ */
