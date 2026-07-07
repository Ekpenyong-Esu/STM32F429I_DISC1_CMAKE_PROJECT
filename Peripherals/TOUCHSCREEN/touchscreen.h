/**
  ******************************************************************************
  * @file    touchscreen.h
  * @brief   STMPE811 Touchscreen driver interface for STM32F429 Discovery Board
  * @details This file contains function prototypes and definitions for
  *          the STMPE811 resistive touchscreen controller on the
  *          STM32F429 Discovery board.
  * @version 1.0
  * @date    2025-09-04
  ******************************************************************************
  */

#ifndef TOUCHSCREEN_H
#define TOUCHSCREEN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported constants --------------------------------------------------------*/
#define TS_TIMEOUT                      1000
#define TS_MAX_TOUCHES                  1       /* Single touch support */

/* STMPE811 Constants */
#define STMPE811_I2C_ADDRESS            0x82    /* 8-bit I2C address (7-bit << 1), matches HAL DevAddress */
#define STMPE811_CHIP_ID                0x0811
#define STMPE811_MAX_X                  4095
#define STMPE811_MAX_Y                  4095

/* Display mapping constants */
#define TS_DISPLAY_WIDTH                240
#define TS_DISPLAY_HEIGHT               320

/* GPIO Pin Definitions */
#define TS_INT_PIN                      GPIO_PIN_15   /* PA15 - Touch INT */
#define TS_INT_GPIO_PORT                GPIOA
#define TS_INT_EXTI_IRQn                EXTI15_10_IRQn
#define TS_INT_NVIC_PRIORITY            0x0F             /* EXTI NVIC priority for touch wake */

/* I2C Definitions */
#define TS_I2C                          I2C3
#define TS_I2C_CLK_ENABLE()             __HAL_RCC_I2C3_CLK_ENABLE()
#define TS_I2C_CLK_DISABLE()            __HAL_RCC_I2C3_CLK_DISABLE()
#define TS_I2C_SCL_PIN                  GPIO_PIN_8    /* PA8 */
#define TS_I2C_SCL_GPIO_PORT            GPIOA
#define TS_I2C_SDA_PIN                  GPIO_PIN_9    /* PC9 */
#define TS_I2C_SDA_GPIO_PORT            GPIOC

/* STMPE811 Register Addresses */
#define STMPE811_REG_CHIP_ID            0x00
#define STMPE811_REG_ID_VER             0x02
#define STMPE811_REG_SYS_CTRL1          0x03
#define STMPE811_REG_SYS_CTRL2          0x04
#define STMPE811_REG_SPI_CFG            0x08
#define STMPE811_REG_INT_CTRL           0x09
#define STMPE811_REG_INT_EN             0x0A
#define STMPE811_REG_INT_STA            0x0B
#define STMPE811_REG_GPIO_EN            0x0C
#define STMPE811_REG_GPIO_INT_STA       0x0D
#define STMPE811_REG_ADC_CTRL1          0x20
#define STMPE811_REG_ADC_CTRL2          0x21
#define STMPE811_REG_ADC_CAPT           0x22
#define STMPE811_REG_ADC_DATA_CH0       0x30
#define STMPE811_REG_ADC_DATA_CH1       0x32
#define STMPE811_REG_ADC_DATA_CH4       0x38
#define STMPE811_REG_ADC_DATA_CH5       0x3A
#define STMPE811_REG_ADC_DATA_CH6       0x3C
#define STMPE811_REG_ADC_DATA_CH7       0x3E
#define STMPE811_REG_TSC_CTRL           0x40
#define STMPE811_REG_TSC_CFG            0x41
#define STMPE811_REG_WDW_TR_X           0x42
#define STMPE811_REG_WDW_TR_Y           0x44
#define STMPE811_REG_WDW_BL_X           0x46
#define STMPE811_REG_WDW_BL_Y           0x48
#define STMPE811_REG_FIFO_TH            0x4A
#define STMPE811_REG_FIFO_STA           0x4B
#define STMPE811_REG_FIFO_SIZE          0x4C
#define STMPE811_REG_TSC_DATA_X         0x4D
#define STMPE811_REG_TSC_DATA_Y         0x4F
#define STMPE811_REG_TSC_DATA_Z         0x51
#define STMPE811_REG_TSC_DATA_XYZ       0x52
#define STMPE811_REG_TSC_FRACT_XYZ      0x56
#define STMPE811_REG_TSC_DATA           0x57
#define STMPE811_REG_TSC_I_DRIVE        0x58
#define STMPE811_REG_TSC_SHIELD         0x59
#define STMPE811_REG_TSC_DATA_NON_INC   0xD7

#define STMPE811_REG_IO_AF              0x17

#define STMPE811_PIN_4                  0x10
#define STMPE811_PIN_5                  0x20
#define STMPE811_PIN_6                  0x40
#define STMPE811_PIN_7                  0x80

#define TS_SMOOTHING_THRESHOLD  4



/* Control Register Bit Definitions */
#define STMPE811_SYS_CTRL1_HIBERNATE    0x01
#define STMPE811_SYS_CTRL2_ADC_OFF      0x01
#define STMPE811_SYS_CTRL2_TSC_OFF      0x02
#define STMPE811_SYS_CTRL2_GPIO_OFF     0x04
#define STMPE811_SYS_CTRL2_TS_OFF       0x08

#define STMPE811_INT_CTRL_POL_HIGH      0x04
#define STMPE811_INT_CTRL_POL_LOW       0x00
#define STMPE811_INT_CTRL_EDGE          0x02
#define STMPE811_INT_CTRL_LEVEL         0x00
#define STMPE811_INT_CTRL_ENABLE        0x01
#define STMPE811_INT_CTRL_DISABLE       0x00

#define STMPE811_INT_EN_TOUCH_DET       0x01
#define STMPE811_INT_EN_FIFO_TH         0x02
#define STMPE811_INT_EN_FIFO_OFLOW      0x04
#define STMPE811_INT_EN_FIFO_FULL       0x08
#define STMPE811_INT_EN_FIFO_EMPTY      0x10
#define STMPE811_INT_EN_TEMP_SENS       0x20
#define STMPE811_INT_EN_ADC             0x40
#define STMPE811_INT_EN_GPIO            0x80

#define STMPE811_TSC_CTRL_EN            0x01
#define STMPE811_TSC_CTRL_XYZ           0x00
#define STMPE811_TSC_CTRL_XY            0x02

#define STMPE811_TS_CTRL_STATUS         0x80

#define STMPE811_TSC_CFG_1_SAMPLE       0x00
#define STMPE811_TSC_CFG_2_SAMPLE       0x40
#define STMPE811_TSC_CFG_4_SAMPLE       0x80
#define STMPE811_TSC_CFG_8_SAMPLE       0xC0
#define STMPE811_TSC_CFG_DELAY_10US     0x00
#define STMPE811_TSC_CFG_DELAY_50US     0x08
#define STMPE811_TSC_CFG_DELAY_100US    0x10
#define STMPE811_TSC_CFG_DELAY_500US    0x18
#define STMPE811_TSC_CFG_DELAY_1MS      0x20
#define STMPE811_TSC_CFG_DELAY_5MS      0x28
#define STMPE811_TSC_CFG_DELAY_10MS     0x30
#define STMPE811_TSC_CFG_DELAY_50MS     0x38
#define STMPE811_TSC_CFG_SETTLE_10US    0x00
#define STMPE811_TSC_CFG_SETTLE_100US   0x01
#define STMPE811_TSC_CFG_SETTLE_500US   0x02
#define STMPE811_TSC_CFG_SETTLE_1MS     0x03
#define STMPE811_TSC_CFG_SETTLE_5MS     0x04
#define STMPE811_TSC_CFG_SETTLE_10MS    0x05
#define STMPE811_TSC_CFG_SETTLE_50MS    0x06
#define STMPE811_TSC_CFG_SETTLE_100MS   0x07

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Touchscreen status enumeration
 */
typedef enum {
    TS_OK = 0,                          /**< Operation completed successfully */
    TS_ERROR,                           /**< General error occurred */
    TS_BUSY,                            /**< Touchscreen is busy */
    TS_TIMEOUT_ERROR,                   /**< Operation timed out */
    TS_INVALID_PARAM,                   /**< Invalid parameter provided */
    TS_NOT_INITIALIZED,                 /**< Device not initialized */
    TS_COMMUNICATION_ERROR,             /**< I2C communication error */
    TS_DEVICE_NOT_FOUND                 /**< STMPE811 device not found */
} TS_StatusTypeDef;

/**
 * @brief Touch state enumeration
 */
typedef enum {
    TS_TOUCH_RELEASED = 0,              /**< No touch detected */
    TS_TOUCH_PRESSED,                   /**< Touch detected */
    TS_TOUCH_MOVING                     /**< Touch moving */
} TS_TouchStateTypeDef;

/**
 * @brief Touch gesture enumeration
 */
typedef enum {
    TS_GESTURE_NONE = 0,                /**< No gesture */
    TS_GESTURE_TAP,                     /**< Single tap */
    TS_GESTURE_DOUBLE_TAP,              /**< Double tap */
    TS_GESTURE_LONG_PRESS,              /**< Long press */
    TS_GESTURE_SWIPE_UP,                /**< Swipe up */
    TS_GESTURE_SWIPE_DOWN,              /**< Swipe down */
    TS_GESTURE_SWIPE_LEFT,              /**< Swipe left */
    TS_GESTURE_SWIPE_RIGHT              /**< Swipe right */
} TS_GestureTypeDef;

/**
 * @brief Touch point structure
 */
typedef struct {
    uint16_t X;                         /**< X coordinate */
    uint16_t Y;                         /**< Y coordinate */
    uint16_t Z;                         /**< Pressure (Z coordinate) */
    TS_TouchStateTypeDef State;         /**< Touch state */
    uint32_t Timestamp;                 /**< Touch timestamp */
} TS_TouchPointTypeDef;

/**
 * @brief Touch data structure
 */
typedef struct {
    uint8_t TouchCount;                 /**< Number of active touches */
    TS_TouchPointTypeDef Points[TS_MAX_TOUCHES]; /**< Touch points */
    TS_GestureTypeDef Gesture;          /**< Detected gesture */
    uint32_t GestureTimestamp;          /**< Gesture timestamp */
} TS_TouchDataTypeDef;

/**
 * @brief Calibration data structure
 */
typedef struct {
    uint16_t MinX;                      /**< Minimum X value */
    uint16_t MaxX;                      /**< Maximum X value */
    uint16_t MinY;                      /**< Minimum Y value */
    uint16_t MaxY;                      /**< Maximum Y value */
    float ScaleX;                       /**< X scaling factor */
    float ScaleY;                       /**< Y scaling factor */
    int16_t OffsetX;                    /**< X offset */
    int16_t OffsetY;                    /**< Y offset */
    bool IsCalibrated;                  /**< Calibration status */
} TS_CalibrationTypeDef;

/**
 * @brief Touchscreen configuration structure
 */
typedef struct {
    uint8_t SampleTime;                 /**< ADC sample time */
    uint8_t AverageControl;             /**< Averaging control */
    uint8_t TouchDetectDelay;           /**< Touch detection delay */
    uint8_t PanelDriverSettlingTime;    /**< Panel driver settling time */
    uint16_t PressureThreshold;         /**< Pressure threshold */
    bool InterruptEnable;               /**< Interrupt enable */
    bool FIFOEnable;                    /**< FIFO enable */
    uint8_t FIFOThreshold;              /**< FIFO threshold */
} TS_ConfigTypeDef;


/**
 * @brief Touchscreen handle structure
 */
typedef struct {
    I2C_HandleTypeDef *hi2c;            /**< I2C handle */
    TS_ConfigTypeDef Config;            /**< Configuration */
    TS_CalibrationTypeDef Calibration;  /**< Calibration data */
    TS_TouchDataTypeDef TouchData;      /**< Current touch data */
    TS_TouchDataTypeDef PrevTouchData;  /**< Previous touch data */
    bool IsInitialized;                 /**< Initialization status */
    bool InterruptMode;                 /**< Interrupt mode status */
    uint32_t LastTouchTime;             /**< Last touch timestamp */
    uint16_t DeviceID;                  /**< Device ID */
    void (*TouchCallback)(void);        /**< Touch detected callback */
    void (*ReleaseCallback)(void);      /**< Touch released callback */
    void (*GestureCallback)(TS_GestureTypeDef gesture); /**< Gesture callback */
} TS_HandleTypeDef;

/* Exported function prototypes ---------------------------------------------*/

/* Initialization and configuration functions */
TS_StatusTypeDef TS_Init(TS_HandleTypeDef *hts, I2C_HandleTypeDef *hi2c);
TS_StatusTypeDef TS_DeInit(TS_HandleTypeDef *hts);
TS_StatusTypeDef TS_Configure(TS_HandleTypeDef *hts, TS_ConfigTypeDef *config);
TS_StatusTypeDef TS_Reset(TS_HandleTypeDef *hts);

/* Touch detection and reading functions */
TS_StatusTypeDef TS_ReadTouchData(TS_HandleTypeDef *hts);
TS_StatusTypeDef TS_GetTouchData(TS_HandleTypeDef *hts, TS_TouchDataTypeDef *touch_data);
TS_StatusTypeDef TS_GetSingleTouch(TS_HandleTypeDef *hts,
                                  uint16_t *xPos,
                                  uint16_t *yPos);
bool TS_IsTouched(TS_HandleTypeDef *hts);
uint8_t TS_GetTouchCount(TS_HandleTypeDef *hts);

/* Calibration functions */
TS_StatusTypeDef TS_Calibrate(TS_HandleTypeDef *hts);
TS_StatusTypeDef TS_SetCalibration(TS_HandleTypeDef *hts, TS_CalibrationTypeDef *calibration);
TS_StatusTypeDef TS_GetCalibration(TS_HandleTypeDef *hts, TS_CalibrationTypeDef *calibration);
TS_StatusTypeDef TS_SaveCalibration(TS_HandleTypeDef *hts);
TS_StatusTypeDef TS_LoadCalibration(TS_HandleTypeDef *hts);

/* Gesture recognition functions */
TS_StatusTypeDef TS_DetectGesture(TS_HandleTypeDef *hts);
TS_GestureTypeDef TS_GetLastGesture(TS_HandleTypeDef *hts);
TS_StatusTypeDef TS_EnableGestureDetection(TS_HandleTypeDef *hts, bool enable);

/* Interrupt functions */
TS_StatusTypeDef TS_EnableInterrupt(TS_HandleTypeDef *hts, bool enable);
TS_StatusTypeDef TS_ITConfig(TS_HandleTypeDef *hts);
void TS_IRQHandler(TS_HandleTypeDef *hts);

/* Global touchscreen handle for interrupt handling */
extern TS_HandleTypeDef *g_hts;

/* Callback registration functions */
TS_StatusTypeDef TS_RegisterCallbacks(TS_HandleTypeDef *hts,
                                     void (*touch_callback)(void),
                                     void (*release_callback)(void),
                                     void (*gesture_callback)(TS_GestureTypeDef));

/*
 * @brief Service pending touchscreen interrupt (deferred from EXTI ISR)
 * @details Call from main loop/LVGL task to clear STMPE811 INT and run callbacks
 */
void TS_ServiceIRQ(void);

/* Utility functions */
TS_StatusTypeDef TS_GetDeviceInfo(TS_HandleTypeDef *hts, uint16_t *device_id, uint8_t *version);
//TS_StatusTypeDef TS_ConvertCoordinates(TS_HandleTypeDef *hts, uint16_t raw_x, uint16_t raw_y,
 //                                     uint16_t *display_x, uint16_t *display_y);
 TS_StatusTypeDef TS_GetTouchState(TS_HandleTypeDef *hts,
                                  uint16_t *x, uint16_t *y,
                                  uint8_t *pressed);
TS_StatusTypeDef TS_SetThreshold(TS_HandleTypeDef *hts, uint16_t threshold);
TS_StatusTypeDef TS_GetPressure(TS_HandleTypeDef *hts, uint16_t *pressure);

/* Low-level I/O functions */
TS_StatusTypeDef TS_ReadRegister(TS_HandleTypeDef *hts, uint8_t reg, uint8_t *data);
TS_StatusTypeDef TS_ReadRegisterMulti(TS_HandleTypeDef *hts, uint8_t reg, uint8_t *data, uint16_t size);
TS_StatusTypeDef TS_WriteRegister(TS_HandleTypeDef *hts, uint8_t reg, uint8_t data);
TS_StatusTypeDef TS_ReadRegister16(TS_HandleTypeDef *hts, uint8_t reg, uint16_t *data);
TS_StatusTypeDef TS_WriteRegister16(TS_HandleTypeDef *hts, uint8_t reg, uint16_t data);

/* Configuration helpers */
TS_ConfigTypeDef TS_GetDefaultConfig(void);

#ifdef __cplusplus
}
#endif

#endif /* TOUCHSCREEN_H */
