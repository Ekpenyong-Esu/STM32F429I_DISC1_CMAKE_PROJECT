/**
  ******************************************************************************
  * @file    laser_distance.h
  * @brief   Laser distance sensor driver interface
  * @details This file provides the interface for laser distance sensors
  *          like VL53L0X, using I2C interface for communication.
  * @version 1.0
  * @date    2025-01-13
  ******************************************************************************
  */

#ifndef __LASER_DISTANCE_H__
#define __LASER_DISTANCE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Laser distance sensor status enumeration
 */
typedef enum {
    LASER_DISTANCE_OK = 0,           /**< Operation completed successfully */
    LASER_DISTANCE_ERROR,            /**< General error occurred */
    LASER_DISTANCE_BUSY,             /**< Sensor is busy measuring */
    LASER_DISTANCE_TIMEOUT,          /**< Measurement timeout */
    LASER_DISTANCE_INVALID_PARAM,    /**< Invalid parameter provided */
    LASER_DISTANCE_NOT_INITIALIZED,  /**< Driver not initialized */
    LASER_DISTANCE_OUT_OF_RANGE,     /**< Distance out of sensor range */
    LASER_DISTANCE_I2C_ERROR         /**< I2C communication error */
} LASER_DISTANCE_StatusTypeDef;

/**
 * @brief Laser distance sensor type enumeration
 */
typedef enum {
    LASER_DISTANCE_VL53L0X = 0,      /**< ST VL53L0X (30-2000mm) */
    LASER_DISTANCE_VL53L1X,          /**< ST VL53L1X (40-4000mm) */
    LASER_DISTANCE_TFMINI,           /**< Benewake TFmini (30-12000mm) */
    LASER_DISTANCE_CUSTOM            /**< Custom sensor with user-defined protocol */
} LASER_DISTANCE_SensorType_t;

/**
 * @brief Laser distance sensor configuration structure
 */
typedef struct {
    LASER_DISTANCE_SensorType_t sensorType;  /**< Sensor type */
    uint16_t minDistance;        /**< Minimum measurable distance in mm */
    uint16_t maxDistance;        /**< Maximum measurable distance in mm */
    uint16_t averagingSamples;   /**< Number of samples for averaging */
    uint32_t measurementTimeout; /**< Measurement timeout in ms */
    uint8_t i2cAddress;          /**< I2C slave address */
    bool longRangeMode;          /**< Enable long range mode (if supported) */
    bool highAccuracyMode;       /**< Enable high accuracy mode (if supported) */
} LASER_DISTANCE_Config_t;

/**
 * @brief Laser distance sensor measurement data structure
 */
typedef struct {
    uint16_t distance;           /**< Measured distance in mm */
    uint16_t ambientRate;        /**< Ambient light rate (VL53L0X) */
    uint16_t signalRate;         /**< Signal rate (VL53L0X) */
    uint8_t rangeStatus;         /**< Range status (VL53L0X) */
    uint32_t timestamp;          /**< Measurement timestamp */
} LASER_DISTANCE_Measurement_t;

/**
 * @brief Laser distance sensor handle structure
 */
typedef struct {
    I2C_HandleTypeDef *hi2c;     /**< I2C handle */
    LASER_DISTANCE_Config_t config; /**< Sensor configuration */

    LASER_DISTANCE_Measurement_t lastMeasurement; /**< Last measurement data */
    bool isInitialized;          /**< Initialization status */
    bool isMeasuring;            /**< Measurement in progress flag */
} LASER_DISTANCE_Handle_t;

/* Exported constants --------------------------------------------------------*/

/* Default configuration values */
#define LASER_DISTANCE_DEFAULT_AVERAGING_SAMPLES    5U
#define LASER_DISTANCE_DEFAULT_MEASUREMENT_TIMEOUT  500U    /* 500ms */
#define LASER_DISTANCE_DEFAULT_I2C_ADDRESS          0x52U   /* VL53L0X default */

/* Sensor specific ranges (mm) */
#define LASER_DISTANCE_VL53L0X_MIN                  30U     /* 3cm */
#define LASER_DISTANCE_VL53L0X_MAX                  2000U   /* 200cm */
#define LASER_DISTANCE_VL53L1X_MIN                  40U     /* 4cm */
#define LASER_DISTANCE_VL53L1X_MAX                  4000U   /* 400cm */
#define LASER_DISTANCE_TFMINI_MIN                   30U     /* 3cm */
#define LASER_DISTANCE_TFMINI_MAX                   12000U  /* 1200cm */

/* VL53L0X I2C addresses */
#define LASER_DISTANCE_VL53L0X_DEFAULT_ADDR         0x52U
#define LASER_DISTANCE_VL53L0X_ALT_ADDR             0x54U

/* VL53L0X Register addresses */
#define LASER_DISTANCE_VL53L0X_REG_SYSRANGE_START   0x00U
#define LASER_DISTANCE_VL53L0X_REG_RESULT_RANGE_STATUS 0x14U
#define LASER_DISTANCE_VL53L0X_REG_RESULT_INTERRUPT_STATUS 0x4FU
#define LASER_DISTANCE_VL53L0X_REG_I2C_SLAVE_DEVICE_ADDRESS 0x8AU

/* Exported functions ------------------------------------------------------- */

/**
 * @brief   Initialize laser distance sensor
 * @details Configures I2C communication and initializes sensor
 * @param   hlaser Pointer to laser distance sensor handle
 * @param   hi2c Pointer to I2C handle
 * @param   sensorType Type of laser distance sensor
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_Init(LASER_DISTANCE_Handle_t *hlaser,
                                                I2C_HandleTypeDef *hi2c,
                                                LASER_DISTANCE_SensorType_t sensorType);

/**
 * @brief   Deinitialize laser distance sensor
 * @details Stops sensor and releases resources
 * @param   hlaser Pointer to laser distance sensor handle
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_DeInit(LASER_DISTANCE_Handle_t *hlaser);

/**
 * @brief   Configure laser distance sensor parameters
 * @details Sets sensor configuration parameters
 * @param   hlaser Pointer to laser distance sensor handle
 * @param   config Pointer to configuration structure
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_Config(LASER_DISTANCE_Handle_t *hlaser,
                                                  LASER_DISTANCE_Config_t *config);

/**
 * @brief   Start continuous measurement mode
 * @details Configures sensor for continuous distance measurement
 * @param   hlaser Pointer to laser distance sensor handle
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_StartContinuous(LASER_DISTANCE_Handle_t *hlaser);

/**
 * @brief   Stop continuous measurement mode
 * @details Stops continuous measurement
 * @param   hlaser Pointer to laser distance sensor handle
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_StopContinuous(LASER_DISTANCE_Handle_t *hlaser);

/**
 * @brief   Perform single distance measurement
 * @details Triggers single measurement and returns result
 * @param   hlaser Pointer to laser distance sensor handle
 * @retval  uint16_t Distance in millimeters (0 if measurement failed)
 */
uint16_t LASER_DISTANCE_MeasureDistance(LASER_DISTANCE_Handle_t *hlaser);

/**
 * @brief   Get last measurement data
 * @details Returns complete measurement data structure
 * @param   hlaser Pointer to laser distance sensor handle
 * @param   measurement Pointer to measurement data structure
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_GetMeasurement(LASER_DISTANCE_Handle_t *hlaser,
                                                          LASER_DISTANCE_Measurement_t *measurement);

/**
 * @brief   Get last measured distance
 * @details Returns the last measured distance in millimeters
 * @param   hlaser Pointer to laser distance sensor handle
 * @retval  uint16_t Distance in millimeters (0 if no valid measurement)
 */
uint16_t LASER_DISTANCE_GetDistance(LASER_DISTANCE_Handle_t *hlaser);

/**
 * @brief   Check if new measurement is available
 * @details Returns true if new measurement data is ready
 * @param   hlaser Pointer to laser distance sensor handle
 * @retval  bool True if measurement is ready
 */
bool LASER_DISTANCE_IsMeasurementReady(LASER_DISTANCE_Handle_t *hlaser);

/**
 * @brief   Change I2C address
 * @details Changes sensor I2C slave address (VL53L0X only)
 * @param   hlaser Pointer to laser distance sensor handle
 * @param   newAddress New I2C address
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_ChangeAddress(LASER_DISTANCE_Handle_t *hlaser,
                                                         uint8_t newAddress);

/**
 * @brief   Perform sensor calibration
 * @details Calibrates sensor offset and crosstalk (VL53L0X only)
 * @param   hlaser Pointer to laser distance sensor handle
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_Calibrate(LASER_DISTANCE_Handle_t *hlaser);

/**
 * @brief   Get default configuration for sensor type
 * @details Returns default configuration for specified sensor
 * @param   sensorType Type of laser distance sensor
 * @retval  LASER_DISTANCE_Config_t Default configuration
 */
LASER_DISTANCE_Config_t LASER_DISTANCE_GetDefaultConfig(LASER_DISTANCE_SensorType_t sensorType);

/**
 * @brief   Check if distance is within sensor range
 * @details Validates distance against sensor capabilities
 * @param   hlaser Pointer to laser distance sensor handle
 * @param   distance Distance to validate
 * @retval  bool True if distance is valid
 */
bool LASER_DISTANCE_IsValidDistance(LASER_DISTANCE_Handle_t *hlaser, uint16_t distance);

/**
 * @brief   Get sensor status string
 * @details Returns human-readable status description
 * @param   status Status code
 * @retval  const char* Status description string
 */
const char* LASER_DISTANCE_GetStatusString(LASER_DISTANCE_StatusTypeDef status);

#ifdef __cplusplus
}
#endif

#endif /* __LASER_DISTANCE_H__ */
