/**
  ******************************************************************************
  * @file    ir_distance.c
  * @brief   IR distance sensor driver implementation
  * @details This file provides the implementation of IR distance sensors
  *          using ADC for analog distance measurement.
  * @version 1.0
  * @date    2025-01-13
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "ir_distance.h"
#include "log.h"
#include "adc.h"
#include "stm32f4xx_hal.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Private defines -----------------------------------------------------------*/

/** @defgroup IR_DISTANCE_Private_Defines Private Defines
 * @{
 */

/* ADC conversion constants */
#define IR_DISTANCE_ADC_MAX_VALUE       4095U   /* 12-bit ADC */
#define IR_DISTANCE_VOLTAGE_MAX         3.3f    /* 3.3V reference */

/* GP2Y0A21YK calibration curve constants */
#define IR_DISTANCE_GP2Y0A21YK_A        4.8f
#define IR_DISTANCE_GP2Y0A21YK_B        -0.8f
#define IR_DISTANCE_GP2Y0A21YK_K        27.86f

/* GP2Y0A02YK calibration curve constants */
#define IR_DISTANCE_GP2Y0A02YK_A        5.0f
#define IR_DISTANCE_GP2Y0A02YK_B        -0.7f
#define IR_DISTANCE_GP2Y0A02YK_K        13.5f

/* GP2Y0A41SK calibration curve constants */
#define IR_DISTANCE_GP2Y0A41SK_A        4.9f
#define IR_DISTANCE_GP2Y0A41SK_B        -0.9f
#define IR_DISTANCE_GP2Y0A41SK_K        65.0f

/* GP2Y0A51SK calibration curve constants */
#define IR_DISTANCE_GP2Y0A51SK_A        5.1f
#define IR_DISTANCE_GP2Y0A51SK_B        -0.9f
#define IR_DISTANCE_GP2Y0A51SK_K        120.0f

/** @} */

/* Private variables ---------------------------------------------------------*/

/** @defgroup IR_DISTANCE_Private_Variables Private Variables
 * @{
 */

/* Predefined calibration curves for different sensors */
static const IR_DISTANCE_CustomCurve_t gp2y0a21yk_curve = {
    .numPoints = 8,
    .points = {
        {100, 3580}, {150, 2500}, {200, 1900}, {250, 1500},
        {300, 1250}, {400, 950}, {500, 780}, {800, 480}
    }
};

static const IR_DISTANCE_CustomCurve_t gp2y0a02yk_curve = {
    .numPoints = 8,
    .points = {
        {200, 3200}, {300, 2200}, {400, 1600}, {500, 1250},
        {600, 1050}, {800, 800}, {1000, 650}, {1500, 450}
    }
};

static const IR_DISTANCE_CustomCurve_t gp2y0a41sk_curve = {
    .numPoints = 6,
    .points = {
        {40, 3800}, {60, 3200}, {80, 2700}, {100, 2300},
        {150, 1700}, {300, 900}
    }
};

static const IR_DISTANCE_CustomCurve_t gp2y0a51sk_curve = {
    .numPoints = 6,
    .points = {
        {20, 3900}, {30, 3500}, {50, 2900}, {70, 2400},
        {100, 1800}, {150, 1200}
    }
};

/** @} */

/* Private function prototypes -----------------------------------------------*/
static IR_DISTANCE_StatusTypeDef IR_DISTANCE_ValidateConfig(IR_DISTANCE_Config_t *config);
static uint16_t IR_DISTANCE_ReadAdc(IR_DISTANCE_Handle_t *hird);
static uint16_t IR_DISTANCE_CalculateDistanceFromCurve(IR_DISTANCE_Handle_t *hird, uint16_t adcValue);
static uint16_t IR_DISTANCE_Interpolate(uint16_t x, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
static float IR_DISTANCE_AdcToVoltage(uint16_t adcValue, float scale, float offset);

/* Exported functions -------------------------------------------------------*/

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
                                          IR_DISTANCE_SensorType_t sensorType)
{
    if (hird == NULL || hadc == NULL) {
        log_error("IR_DISTANCE: Invalid parameters provided to IR_DISTANCE_Init");
        return IR_DISTANCE_INVALID_PARAM;
    }

    log_debug("IR_DISTANCE: Initializing IR distance sensor with ADC channel %d, sensor type %d", channel, sensorType);

    /* Initialize structure */
    memset(hird, 0, sizeof(IR_DISTANCE_Handle_t));
    hird->hadc = hadc;
    hird->channel = channel;

    /* Set default configuration based on sensor type */
    IR_DISTANCE_Config_t default_config = IR_DISTANCE_GetDefaultConfig(sensorType);
    IR_DISTANCE_Config(hird, &default_config);

    /* Load predefined calibration curve */
    switch (sensorType) {
        case IR_DISTANCE_GP2Y0A21YK:
            hird->customCurve = gp2y0a21yk_curve;
            log_debug("IR_DISTANCE: Loaded GP2Y0A21YK calibration curve");
            break;
        case IR_DISTANCE_GP2Y0A02YK:
            hird->customCurve = gp2y0a02yk_curve;
            log_debug("IR_DISTANCE: Loaded GP2Y0A02YK calibration curve");
            break;
        case IR_DISTANCE_GP2Y0A41SK:
            hird->customCurve = gp2y0a41sk_curve;
            log_debug("IR_DISTANCE: Loaded GP2Y0A41SK calibration curve");
            break;
        case IR_DISTANCE_GP2Y0A51SK:
            hird->customCurve = gp2y0a51sk_curve;
            log_debug("IR_DISTANCE: Loaded GP2Y0A51SK calibration curve");
            break;
        case IR_DISTANCE_CUSTOM:
            /* User must provide custom curve */
            log_debug("IR_DISTANCE: Using custom calibration curve");
            break;
        default:
            log_error("IR_DISTANCE: Invalid sensor type %d", sensorType);
            return IR_DISTANCE_INVALID_PARAM;
    }

    /* Configure ADC channel */
    ADC_ConfigChannel(hadc, channel, ADC_SAMPLETIME_56CYCLES);
    log_debug("IR_DISTANCE: ADC channel configured");

    hird->isInitialized = true;

    log_info("IR_DISTANCE: IR distance sensor initialized successfully");
    return IR_DISTANCE_OK;
}

/**
 * @brief   Deinitialize IR distance sensor
 * @details Stops ADC and releases resources
 * @param   hird Pointer to IR distance sensor handle
 * @retval  IR_DISTANCE_StatusTypeDef Operation status
 */
IR_DISTANCE_StatusTypeDef IR_DISTANCE_DeInit(IR_DISTANCE_Handle_t *hird)
{
    if (hird == NULL) {
        return IR_DISTANCE_INVALID_PARAM;
    }

    hird->isInitialized = false;

    return IR_DISTANCE_OK;
}

/**
 * @brief   Configure IR distance sensor parameters
 * @details Sets sensor configuration parameters
 * @param   hird Pointer to IR distance sensor handle
 * @param   config Pointer to configuration structure
 * @retval  IR_DISTANCE_StatusTypeDef Operation status
 */
IR_DISTANCE_StatusTypeDef IR_DISTANCE_Config(IR_DISTANCE_Handle_t *hird,
                                            IR_DISTANCE_Config_t *config)
{
    if (hird == NULL || config == NULL) {
        return IR_DISTANCE_INVALID_PARAM;
    }

    /* Validate configuration */
    if (IR_DISTANCE_ValidateConfig(config) != IR_DISTANCE_OK) {
        return IR_DISTANCE_INVALID_PARAM;
    }

    /* Store configuration */
    hird->config = *config;

    return IR_DISTANCE_OK;
}

/**
 * @brief   Perform single distance measurement
 * @details Reads ADC and converts to distance
 * @param   hird Pointer to IR distance sensor handle
 * @retval  uint16_t Distance in millimeters (0 if measurement failed)
 */
uint16_t IR_DISTANCE_MeasureDistance(IR_DISTANCE_Handle_t *hird)
{
    if (hird == NULL || !hird->isInitialized) {
        log_error("IR_DISTANCE: Sensor not initialized");
        return 0;
    }

    log_debug("IR_DISTANCE: Starting distance measurement with %d averaging samples", hird->config.averagingSamples);

    /* Read ADC value with averaging */
    uint32_t adcSum = 0;
    uint16_t validSamples = 0;

    for (uint16_t i = 0; i < hird->config.averagingSamples; i++) {
        uint16_t adcValue = IR_DISTANCE_ReadAdc(hird);
        if (adcValue > 0) {
            adcSum += adcValue;
            validSamples++;
        }
        HAL_Delay(1);  /* Small delay between samples */
    }

    if (validSamples == 0) {
        log_warning("IR_DISTANCE: No valid ADC samples obtained");
        return 0;
    }

    /* Calculate average */
    hird->lastAdcValue = adcSum / validSamples;

    /* Convert to distance */
    hird->lastDistance = IR_DISTANCE_AdcToDistance(hird, hird->lastAdcValue);

    /* Validate distance range */
    if (!IR_DISTANCE_IsValidDistance(hird, hird->lastDistance)) {
        log_warning("IR_DISTANCE: Measured distance %d mm is out of valid range", hird->lastDistance);
        return 0;
    }

    log_debug("IR_DISTANCE: Measurement complete - ADC: %d, Distance: %d mm", hird->lastAdcValue, hird->lastDistance);
    return hird->lastDistance;
}

/**
 * @brief   Get last measured distance
 * @details Returns the last measured distance in millimeters
 * @param   hird Pointer to IR distance sensor handle
 * @retval  uint16_t Distance in millimeters (0 if no valid measurement)
 */
uint16_t IR_DISTANCE_GetDistance(IR_DISTANCE_Handle_t *hird)
{
    if (hird == NULL) {
        return 0;
    }

    return hird->lastDistance;
}

/**
 * @brief   Get last ADC reading
 * @details Returns the last raw ADC value
 * @param   hird Pointer to IR distance sensor handle
 * @retval  uint16_t Raw ADC value
 */
uint16_t IR_DISTANCE_GetAdcValue(IR_DISTANCE_Handle_t *hird)
{
    if (hird == NULL) {
        return 0;
    }

    return hird->lastAdcValue;
}

/**
 * @brief   Convert ADC value to distance
 * @details Converts raw ADC reading to distance using sensor curve
 * @param   hird Pointer to IR distance sensor handle
 * @param   adcValue Raw ADC value
 * @retval  uint16_t Distance in millimeters
 */
uint16_t IR_DISTANCE_AdcToDistance(IR_DISTANCE_Handle_t *hird, uint16_t adcValue)
{
    if (hird == NULL) {
        return 0;
    }

    /* Use calibration curve for conversion */
    return IR_DISTANCE_CalculateDistanceFromCurve(hird, adcValue);
}

/**
 * @brief   Convert distance to ADC value
 * @details Converts distance to expected ADC value using sensor curve
 * @param   hird Pointer to IR distance sensor handle
 * @param   distance Distance in millimeters
 * @retval  uint16_t Expected ADC value
 */
uint16_t IR_DISTANCE_DistanceToAdc(IR_DISTANCE_Handle_t *hird, uint16_t distance)
{
    if (hird == NULL || hird->customCurve.numPoints < 2) {
        return 0;
    }

    /* Find the appropriate segment in the calibration curve */
    for (uint8_t i = 0; i < hird->customCurve.numPoints - 1; i++) {
        uint16_t d1 = hird->customCurve.points[i].distance;
        uint16_t d2 = hird->customCurve.points[i + 1].distance;
        uint16_t a1 = hird->customCurve.points[i].adcValue;
        uint16_t a2 = hird->customCurve.points[i + 1].adcValue;

        if (distance >= d1 && distance <= d2) {
            return IR_DISTANCE_Interpolate(distance, d1, a1, d2, a2);
        }
    }

    return 0;
}

/**
 * @brief   Set custom calibration curve
 * @details Configures custom distance-to-ADC mapping
 * @param   hird Pointer to IR distance sensor handle
 * @param   curve Pointer to custom calibration curve
 * @retval  IR_DISTANCE_StatusTypeDef Operation status
 */
IR_DISTANCE_StatusTypeDef IR_DISTANCE_SetCustomCurve(IR_DISTANCE_Handle_t *hird,
                                                    IR_DISTANCE_CustomCurve_t *curve)
{
    if (hird == NULL || curve == NULL || curve->numPoints < 2) {
        return IR_DISTANCE_INVALID_PARAM;
    }

    hird->customCurve = *curve;
    hird->config.sensorType = IR_DISTANCE_CUSTOM;

    return IR_DISTANCE_OK;
}

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
                                                    uint16_t adcValue)
{
    if (hird == NULL || hird->customCurve.numPoints >= 10) {
        return IR_DISTANCE_INVALID_PARAM;
    }

    /* Add new calibration point */
    hird->customCurve.points[hird->customCurve.numPoints].distance = distance;
    hird->customCurve.points[hird->customCurve.numPoints].adcValue = adcValue;
    hird->customCurve.numPoints++;

    /* Sort points by distance (simple bubble sort) */
    for (uint8_t i = 0; i < hird->customCurve.numPoints - 1; i++) {
        for (uint8_t j = 0; j < hird->customCurve.numPoints - i - 1; j++) {
            if (hird->customCurve.points[j].distance > hird->customCurve.points[j + 1].distance) {
                IR_DISTANCE_CalibrationPoint_t temp = hird->customCurve.points[j];
                hird->customCurve.points[j] = hird->customCurve.points[j + 1];
                hird->customCurve.points[j + 1] = temp;
            }
        }
    }

    return IR_DISTANCE_OK;
}

/**
 * @brief   Get default configuration for sensor type
 * @details Returns default configuration for specified sensor
 * @param   sensorType Type of IR distance sensor
 * @retval  IR_DISTANCE_Config_t Default configuration
 */
IR_DISTANCE_Config_t IR_DISTANCE_GetDefaultConfig(IR_DISTANCE_SensorType_t sensorType)
{
    IR_DISTANCE_Config_t config = {
        .sensorType = sensorType,
        .averagingSamples = IR_DISTANCE_DEFAULT_AVERAGING_SAMPLES,
        .measurementTimeout = IR_DISTANCE_DEFAULT_MEASUREMENT_TIMEOUT,
        .voltageScale = IR_DISTANCE_DEFAULT_VOLTAGE_SCALE,
        .voltageOffset = IR_DISTANCE_DEFAULT_VOLTAGE_OFFSET
    };

    /* Set range based on sensor type */
    switch (sensorType) {
        case IR_DISTANCE_GP2Y0A21YK:
            config.minDistance = IR_DISTANCE_GP2Y0A21YK_MIN;
            config.maxDistance = IR_DISTANCE_GP2Y0A21YK_MAX;
            break;
        case IR_DISTANCE_GP2Y0A02YK:
            config.minDistance = IR_DISTANCE_GP2Y0A02YK_MIN;
            config.maxDistance = IR_DISTANCE_GP2Y0A02YK_MAX;
            break;
        case IR_DISTANCE_GP2Y0A41SK:
            config.minDistance = IR_DISTANCE_GP2Y0A41SK_MIN;
            config.maxDistance = IR_DISTANCE_GP2Y0A41SK_MAX;
            break;
        case IR_DISTANCE_GP2Y0A51SK:
            config.minDistance = IR_DISTANCE_GP2Y0A51SK_MIN;
            config.maxDistance = IR_DISTANCE_GP2Y0A51SK_MAX;
            break;
        case IR_DISTANCE_CUSTOM:
            config.minDistance = 10;
            config.maxDistance = 1000;
            break;
        default:
            config.minDistance = 10;
            config.maxDistance = 1000;
            break;
    }

    return config;
}

/**
 * @brief   Check if distance is within sensor range
 * @details Validates distance against sensor capabilities
 * @param   hird Pointer to IR distance sensor handle
 * @param   distance Distance to validate
 * @retval  bool True if distance is valid
 */
bool IR_DISTANCE_IsValidDistance(IR_DISTANCE_Handle_t *hird, uint16_t distance)
{
    if (hird == NULL) {
        return false;
    }

    return (distance >= hird->config.minDistance && distance <= hird->config.maxDistance);
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Validate configuration
 * @details Checks if configuration parameters are valid
 * @param   config Pointer to configuration structure
 * @retval  IR_DISTANCE_StatusTypeDef Validation status
 */
static IR_DISTANCE_StatusTypeDef IR_DISTANCE_ValidateConfig(IR_DISTANCE_Config_t *config)
{
    if (config->minDistance >= config->maxDistance) {
        return IR_DISTANCE_INVALID_PARAM;
    }

    if (config->averagingSamples == 0 || config->averagingSamples > 100) {
        return IR_DISTANCE_INVALID_PARAM;
    }

    if (config->voltageScale <= 0.0f) {
        return IR_DISTANCE_INVALID_PARAM;
    }

    return IR_DISTANCE_OK;
}

/**
 * @brief   Read ADC value
 * @details Performs single ADC conversion
 * @param   hird Pointer to IR distance sensor handle
 * @retval  uint16_t ADC value (0 if failed)
 */
static uint16_t IR_DISTANCE_ReadAdc(IR_DISTANCE_Handle_t *hird)
{
    if (hird == NULL || hird->hadc == NULL) {
        return 0;
    }

    uint32_t adcValue;
    if (ADC_ReadChannel(hird->hadc, hird->channel, &adcValue) == HAL_OK) {
        return (uint16_t)adcValue;
    }

    return 0;
}

/**
 * @brief   Calculate distance from calibration curve
 * @details Uses piecewise linear interpolation on calibration curve
 * @param   hird Pointer to IR distance sensor handle
 * @param   adcValue Raw ADC value
 * @retval  uint16_t Distance in millimeters
 */
static uint16_t IR_DISTANCE_CalculateDistanceFromCurve(IR_DISTANCE_Handle_t *hird, uint16_t adcValue)
{
    if (hird->customCurve.numPoints < 2) {
        return 0;
    }

    /* Find the appropriate segment in the calibration curve */
    for (uint8_t i = 0; i < hird->customCurve.numPoints - 1; i++) {
        uint16_t a1 = hird->customCurve.points[i].adcValue;
        uint16_t a2 = hird->customCurve.points[i + 1].adcValue;
        uint16_t d1 = hird->customCurve.points[i].distance;
        uint16_t d2 = hird->customCurve.points[i + 1].distance;

        if (adcValue <= a1 && adcValue >= a2) {
            return IR_DISTANCE_Interpolate(adcValue, a1, d1, a2, d2);
        }
    }

    return 0;
}

/**
 * @brief   Linear interpolation
 * @details Calculates y value for given x using linear interpolation
 * @param   x Value to interpolate
 * @param   x1 First x coordinate
 * @param   y1 First y coordinate
 * @param   x2 Second x coordinate
 * @param   y2 Second y coordinate
 * @retval  uint16_t Interpolated y value
 */
static uint16_t IR_DISTANCE_Interpolate(uint16_t x, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    if (x1 == x2) {
        return y1;
    }

    return y1 + ((y2 - y1) * (x - x1)) / (x2 - x1);
}

/**
 * @brief   Convert ADC value to voltage
 * @details Converts raw ADC reading to voltage
 * @param   adcValue Raw ADC value
 * @param   scale Voltage scaling factor
 * @param   offset Voltage offset
 * @retval  float Voltage value
 */
static float IR_DISTANCE_AdcToVoltage(uint16_t adcValue, float scale, float offset)
{
    return ((adcValue * scale) / IR_DISTANCE_ADC_MAX_VALUE) + offset;
}
