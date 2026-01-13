/**
  ******************************************************************************
  * @file    ir_distance.h
  * @brief   IR distance sensor driver interface
  * @details This file provides the interface for IR distance sensors
  *          like GP2Y0A21YK, using ADC for analog distance measurement.
  * @version 1.0
  * @date    2025-01-13
  ******************************************************************************
  */

#ifndef __IR_DISTANCE_H__
#define __IR_DISTANCE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "adc.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief IR distance sensor status enumeration
 */
typedef enum {
    IR_DISTANCE_OK = 0,           /**< Operation completed successfully */
    IR_DISTANCE_ERROR,            /**< General error occurred */
    IR_DISTANCE_BUSY,             /**< Sensor is busy measuring */
    IR_DISTANCE_TIMEOUT,          /**< Measurement timeout */
    IR_DISTANCE_INVALID_PARAM,    /**< Invalid parameter provided */
    IR_DISTANCE_NOT_INITIALIZED,  /**< Driver not initialized */
    IR_DISTANCE_OUT_OF_RANGE      /**< Distance out of sensor range */
} IR_DISTANCE_StatusTypeDef;

/**
 * @brief IR distance sensor type enumeration
 */
typedef enum {
    IR_DISTANCE_GP2Y0A21YK = 0,   /**< Sharp GP2Y0A21YK (10-80cm) */
    IR_DISTANCE_GP2Y0A02YK,       /**< Sharp GP2Y0A02YK (20-150cm) */
    IR_DISTANCE_GP2Y0A41SK,       /**< Sharp GP2Y0A41SK (4-30cm) */
    IR_DISTANCE_GP2Y0A51SK,       /**< Sharp GP2Y0A51SK (2-15cm) */
    IR_DISTANCE_CUSTOM            /**< Custom sensor with user-defined curve */
} IR_DISTANCE_SensorType_t;

/**
 * @brief IR distance sensor configuration structure
 */
typedef struct {
    IR_DISTANCE_SensorType_t sensorType;  /**< Sensor type */
    uint16_t minDistance;        /**< Minimum measurable distance in mm */
    uint16_t maxDistance;        /**< Maximum measurable distance in mm */
    uint16_t averagingSamples;   /**< Number of samples for averaging */
    uint32_t measurementTimeout; /**< Measurement timeout in ms */
    float voltageScale;          /**< ADC voltage scaling factor */
    float voltageOffset;         /**< ADC voltage offset */
} IR_DISTANCE_Config_t;

/**
 * @brief IR distance sensor calibration point
 */
typedef struct {
    uint16_t distance;           /**< Distance in mm */
    uint16_t adcValue;           /**< Corresponding ADC value */
} IR_DISTANCE_CalibrationPoint_t;

/**
 * @brief IR distance sensor custom curve structure
 */
typedef struct {
    uint8_t numPoints;           /**< Number of calibration points */
    IR_DISTANCE_CalibrationPoint_t points[10]; /**< Calibration points */
} IR_DISTANCE_CustomCurve_t;

/**
 * @brief IR distance sensor handle structure
 */
typedef struct {
    ADC_HandleStruct *hadc;     /**< ADC handle */
    uint32_t channel;            /**< ADC channel */
    IR_DISTANCE_Config_t config; /**< Sensor configuration */
    IR_DISTANCE_CustomCurve_t customCurve; /**< Custom calibration curve */

    uint16_t lastAdcValue;       /**< Last ADC reading */
    uint16_t lastDistance;       /**< Last calculated distance in mm */
    bool isInitialized;          /**< Initialization status */
} IR_DISTANCE_Handle_t;

/* Exported constants --------------------------------------------------------*/

/* Default configuration values */
#define IR_DISTANCE_DEFAULT_AVERAGING_SAMPLES    5U
#define IR_DISTANCE_DEFAULT_MEASUREMENT_TIMEOUT  100U    /* 100ms */
#define IR_DISTANCE_DEFAULT_VOLTAGE_SCALE        3.3f    /* 3.3V reference */
#define IR_DISTANCE_DEFAULT_VOLTAGE_OFFSET       0.0f

/* Sensor specific ranges (mm) */
#define IR_DISTANCE_GP2Y0A21YK_MIN               100U    /* 10cm */
#define IR_DISTANCE_GP2Y0A21YK_MAX               800U    /* 80cm */
#define IR_DISTANCE_GP2Y0A02YK_MIN               200U    /* 20cm */
#define IR_DISTANCE_GP2Y0A02YK_MAX               1500U   /* 150cm */
#define IR_DISTANCE_GP2Y0A41SK_MIN               40U     /* 4cm */
#define IR_DISTANCE_GP2Y0A41SK_MAX               300U    /* 30cm */
#define IR_DISTANCE_GP2Y0A51SK_MIN               20U     /* 2cm */
#define IR_DISTANCE_GP2Y0A51SK_MAX               150U    /* 15cm */

/* Exported functions ------------------------------------------------------- */

/**
 * @brief   Initialize IR distance sensor
 * @details Configures ADC channel for distance measurement
 * @param   hird Pointer to IR distance sensor handle
 * @param   hadc Pointer to ADC handle
 * @param   channel ADC channel for sensor output
 * @param   sensorType Type of IR distance sensor
 * @retval  IR_DISTANCE_StatusTypeDef Operation status
 */
IR_DISTANCE_StatusTypeDef IR_DISTANCE_Init(IR_DISTANCE_Handle_t *hird,
                                          ADC_HandleStruct *hadc,
                                          uint32_t channel,
                                          IR_DISTANCE_SensorType_t sensorType);

/**
 * @brief   Deinitialize IR distance sensor
 * @details Stops ADC and releases resources
 * @param   hird Pointer to IR distance sensor handle
 * @retval  IR_DISTANCE_StatusTypeDef Operation status
 */
IR_DISTANCE_StatusTypeDef IR_DISTANCE_DeInit(IR_DISTANCE_Handle_t *hird);

/**
 * @brief   Configure IR distance sensor parameters
 * @details Sets sensor configuration parameters
 * @param   hird Pointer to IR distance sensor handle
 * @param   config Pointer to configuration structure
 * @retval  IR_DISTANCE_StatusTypeDef Operation status
 */
IR_DISTANCE_StatusTypeDef IR_DISTANCE_Config(IR_DISTANCE_Handle_t *hird,
                                            IR_DISTANCE_Config_t *config);

/**
 * @brief   Perform single distance measurement
 * @details Reads ADC and converts to distance
 * @param   hird Pointer to IR distance sensor handle
 * @retval  uint16_t Distance in millimeters (0 if measurement failed)
 */
uint16_t IR_DISTANCE_MeasureDistance(IR_DISTANCE_Handle_t *hird);

/**
 * @brief   Get last measured distance
 * @details Returns the last measured distance in millimeters
 * @param   hird Pointer to IR distance sensor handle
 * @retval  uint16_t Distance in millimeters (0 if no valid measurement)
 */
uint16_t IR_DISTANCE_GetDistance(IR_DISTANCE_Handle_t *hird);

/**
 * @brief   Get last ADC reading
 * @details Returns the last raw ADC value
 * @param   hird Pointer to IR distance sensor handle
 * @retval  uint16_t Raw ADC value
 */
uint16_t IR_DISTANCE_GetAdcValue(IR_DISTANCE_Handle_t *hird);

/**
 * @brief   Convert ADC value to distance
 * @details Converts raw ADC reading to distance using sensor curve
 * @param   hird Pointer to IR distance sensor handle
 * @param   adcValue Raw ADC value
 * @retval  uint16_t Distance in millimeters
 */
uint16_t IR_DISTANCE_AdcToDistance(IR_DISTANCE_Handle_t *hird, uint16_t adcValue);

/**
 * @brief   Convert distance to ADC value
 * @details Converts distance to expected ADC value using sensor curve
 * @param   hird Pointer to IR distance sensor handle
 * @param   distance Distance in millimeters
 * @retval  uint16_t Expected ADC value
 */
uint16_t IR_DISTANCE_DistanceToAdc(IR_DISTANCE_Handle_t *hird, uint16_t distance);

/**
 * @brief   Set custom calibration curve
 * @details Configures custom distance-to-ADC mapping
 * @param   hird Pointer to IR distance sensor handle
 * @param   curve Pointer to custom calibration curve
 * @retval  IR_DISTANCE_StatusTypeDef Operation status
 */
IR_DISTANCE_StatusTypeDef IR_DISTANCE_SetCustomCurve(IR_DISTANCE_Handle_t *hird,
                                                    IR_DISTANCE_CustomCurve_t *curve);

/**
 * @brief   Calibrate sensor at specific distance
 * @details Adds calibration point for improved accuracy
 * @param   hird Pointer to IR distance sensor handle
 * @param   distance Actual distance in millimeters
 * @param   adcValue Corresponding ADC reading
 * @retval  IR_DISTANCE_StatusTypeDef Operation status
 */
IR_DISTANCE_StatusTypeDef IR_DISTANCE_CalibratePoint(IR_DISTANCE_Handle_t *hird,
                                                    uint16_t distance,
                                                    uint16_t adcValue);

/**
 * @brief   Get default configuration for sensor type
 * @details Returns default configuration for specified sensor
 * @param   sensorType Type of IR distance sensor
 * @retval  IR_DISTANCE_Config_t Default configuration
 */
IR_DISTANCE_Config_t IR_DISTANCE_GetDefaultConfig(IR_DISTANCE_SensorType_t sensorType);

/**
 * @brief   Check if distance is within sensor range
 * @details Validates distance against sensor capabilities
 * @param   hird Pointer to IR distance sensor handle
 * @param   distance Distance to validate
 * @retval  bool True if distance is valid
 */
bool IR_DISTANCE_IsValidDistance(IR_DISTANCE_Handle_t *hird, uint16_t distance);

#ifdef __cplusplus
}
#endif

#endif /* __IR_DISTANCE_H__ */
