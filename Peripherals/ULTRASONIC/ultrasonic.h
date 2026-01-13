/**
  ******************************************************************************
  * @file    ultrasonic.h
  * @brief   Ultrasonic distance sensor driver interface
  * @details This file provides the interface for ultrasonic distance sensors
  *          like HC-SR04, using timer input capture for echo pulse measurement.
  * @version 1.0
  * @date    2025-01-13
  ******************************************************************************
  */

#ifndef __ULTRASONIC_H__
#define __ULTRASONIC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Ultrasonic sensor status enumeration
 */
typedef enum {
    ULTRASONIC_OK = 0,           /**< Operation completed successfully */
    ULTRASONIC_ERROR,            /**< General error occurred */
    ULTRASONIC_BUSY,             /**< Sensor is busy measuring */
    ULTRASONIC_TIMEOUT,          /**< Measurement timeout */
    ULTRASONIC_INVALID_PARAM,    /**< Invalid parameter provided */
    ULTRASONIC_NOT_INITIALIZED   /**< Driver not initialized */
} ULTRASONIC_StatusTypeDef;

/**
 * @brief Ultrasonic sensor configuration structure
 */
typedef struct {
    uint32_t triggerTimeout;     /**< Trigger pulse timeout in ms */
    uint32_t measurementTimeout; /**< Echo measurement timeout in ms */
    uint16_t minDistance;        /**< Minimum measurable distance in mm */
    uint16_t maxDistance;        /**< Maximum measurable distance in mm */
    uint8_t temperature;         /**< Ambient temperature in Celsius (for speed correction) */
} ULTRASONIC_Config_t;

/**
 * @brief Ultrasonic sensor GPIO pins structure
 */
typedef struct {
    GPIO_TypeDef *triggerPort;   /**< GPIO port for trigger pin */
    uint16_t triggerPin;         /**< GPIO pin for trigger signal */
    GPIO_TypeDef *echoPort;      /**< GPIO port for echo pin */
    uint16_t echoPin;            /**< GPIO pin for echo signal */
} ULTRASONIC_Pins_t;

/**
 * @brief Ultrasonic sensor handle structure
 */
typedef struct {
    TIM_HandleTypeDef *htim;     /**< Timer handle for input capture */
    uint32_t channel;            /**< Timer channel for input capture */
    ULTRASONIC_Pins_t pins;      /**< GPIO pin configuration */
    ULTRASONIC_Config_t config;  /**< Sensor configuration */

    volatile uint32_t echoStart; /**< Echo pulse start time */
    volatile uint32_t echoEnd;   /**< Echo pulse end time */
    volatile bool measurementDone; /**< Measurement completion flag */

    uint16_t lastDistance;       /**< Last measured distance in mm */
    bool isInitialized;          /**< Initialization status */
} ULTRASONIC_Handle_t;

/* Exported constants --------------------------------------------------------*/

/* Default configuration values */
#define ULTRASONIC_DEFAULT_TRIGGER_TIMEOUT    1000U   /* 1 second */
#define ULTRASONIC_DEFAULT_MEASUREMENT_TIMEOUT 100000U /* 100ms */
#define ULTRASONIC_DEFAULT_MIN_DISTANCE       20U     /* 2cm */
#define ULTRASONIC_DEFAULT_MAX_DISTANCE       4000U   /* 4m */
#define ULTRASONIC_DEFAULT_TEMPERATURE        20U     /* 20°C */

/* HC-SR04 timing constants */
#define ULTRASONIC_TRIGGER_PULSE_WIDTH        10U     /* 10us trigger pulse */
#define ULTRASONIC_SPEED_OF_SOUND             343U    /* m/s at 20°C */
#define ULTRASONIC_TIMEOUT_DISTANCE           4000U   /* 4m timeout */

/* Conversion factors */
#define ULTRASONIC_MM_PER_US                  0.1715f /* mm per microsecond (at 20°C) */

/* Exported functions ------------------------------------------------------- */

/**
 * @brief   Initialize ultrasonic sensor
 * @details Configures GPIO pins and timer for distance measurement
 * @param   hultra Pointer to ultrasonic sensor handle
 * @param   htim Pointer to timer handle for input capture
 * @param   channel Timer input capture channel
 * @param   pins Pointer to GPIO pin configuration
 * @retval  ULTRASONIC_StatusTypeDef Operation status
 */
ULTRASONIC_StatusTypeDef ULTRASONIC_Init(ULTRASONIC_Handle_t *hultra,
                                        TIM_HandleTypeDef *htim,
                                        uint32_t channel,
                                        ULTRASONIC_Pins_t *pins);

/**
 * @brief   Deinitialize ultrasonic sensor
 * @details Stops timer and releases resources
 * @param   hultra Pointer to ultrasonic sensor handle
 * @retval  ULTRASONIC_StatusTypeDef Operation status
 */
ULTRASONIC_StatusTypeDef ULTRASONIC_DeInit(ULTRASONIC_Handle_t *hultra);

/**
 * @brief   Configure ultrasonic sensor parameters
 * @details Sets sensor configuration parameters
 * @param   hultra Pointer to ultrasonic sensor handle
 * @param   config Pointer to configuration structure
 * @retval  ULTRASONIC_StatusTypeDef Operation status
 */
ULTRASONIC_StatusTypeDef ULTRASONIC_Config(ULTRASONIC_Handle_t *hultra,
                                          ULTRASONIC_Config_t *config);

/**
 * @brief   Start distance measurement
 * @details Triggers ultrasonic pulse and starts measurement
 * @param   hultra Pointer to ultrasonic sensor handle
 * @retval  ULTRASONIC_StatusTypeDef Operation status
 */
ULTRASONIC_StatusTypeDef ULTRASONIC_StartMeasurement(ULTRASONIC_Handle_t *hultra);

/**
 * @brief   Get measured distance
 * @details Returns the last measured distance in millimeters
 * @param   hultra Pointer to ultrasonic sensor handle
 * @retval  uint16_t Distance in millimeters (0 if no valid measurement)
 */
uint16_t ULTRASONIC_GetDistance(ULTRASONIC_Handle_t *hultra);

/**
 * @brief   Check if measurement is complete
 * @details Returns true if a measurement cycle has finished
 * @param   hultra Pointer to ultrasonic sensor handle
 * @retval  bool True if measurement is complete
 */
bool ULTRASONIC_IsMeasurementComplete(ULTRASONIC_Handle_t *hultra);

/**
 * @brief   Wait for measurement completion
 * @details Blocks until measurement is complete or timeout occurs
 * @param   hultra Pointer to ultrasonic sensor handle
 * @param   timeout Timeout in milliseconds
 * @retval  ULTRASONIC_StatusTypeDef Operation status
 */
ULTRASONIC_StatusTypeDef ULTRASONIC_WaitForMeasurement(ULTRASONIC_Handle_t *hultra,
                                                      uint32_t timeout);

/**
 * @brief   Perform single distance measurement
 * @details Triggers measurement and waits for completion
 * @param   hultra Pointer to ultrasonic sensor handle
 * @retval  uint16_t Distance in millimeters (0 if measurement failed)
 */
uint16_t ULTRASONIC_MeasureDistance(ULTRASONIC_Handle_t *hultra);

/**
 * @brief   Set ambient temperature for speed correction
 * @details Updates temperature for accurate distance calculation
 * @param   hultra Pointer to ultrasonic sensor handle
 * @param   temperature Temperature in Celsius
 * @retval  ULTRASONIC_StatusTypeDef Operation status
 */
ULTRASONIC_StatusTypeDef ULTRASONIC_SetTemperature(ULTRASONIC_Handle_t *hultra,
                                                  uint8_t temperature);

/**
 * @brief   Get default configuration
 * @details Returns a default configuration structure
 * @param   None
 * @retval  ULTRASONIC_Config_t Default configuration
 */
ULTRASONIC_Config_t ULTRASONIC_GetDefaultConfig(void);

/**
 * @brief   Timer input capture callback
 * @details Should be called from timer interrupt handler
 * @param   hultra Pointer to ultrasonic sensor handle
 * @retval  None
 */
void ULTRASONIC_TIM_IC_CaptureCallback(ULTRASONIC_Handle_t *hultra);

#ifdef __cplusplus
}
#endif

#endif /* __ULTRASONIC_H__ */
