/**
  ******************************************************************************
  * @file    laser_distance.c
  * @brief   Laser distance sensor driver implementation
  * @details This file provides the implementation of laser distance sensors
  *          using I2C interface for communication.
  * @version 1.0
  * @date    2025-01-13
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "laser_distance.h"
#include "log.h"
#include "i2c.h"
#include "stm32f4xx_hal.h"
#include <stdlib.h>
#include <string.h>

/* Private defines -----------------------------------------------------------*/

/** @defgroup LASER_DISTANCE_Private_Defines Private Defines
 * @{
 */

/* VL53L0X Register definitions */
#define VL53L0X_REG_IDENTIFICATION_MODEL_ID              0xC0
#define VL53L0X_REG_IDENTIFICATION_REVISION_ID           0xC2
#define VL53L0X_REG_PRE_RANGE_CONFIG_VCSEL_PERIOD        0x50
#define VL53L0X_REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD      0x70
#define VL53L0X_REG_SYSRANGE_START                       0x00
#define VL53L0X_REG_RESULT_INTERRUPT_STATUS              0x13
#define VL53L0X_REG_RESULT_RANGE_STATUS                  0x14
#define VL53L0X_REG_I2C_SLAVE_DEVICE_ADDRESS             0x8A

/* VL53L0X Constants */
#define VL53L0X_EXPECTED_DEVICE_ID                       0xEE
#define VL53L0X_READOUT_AVERAGING_SAMPLE_PERIOD          0x30
#define VL53L0X_VCSEL_PERIOD_PRE_RANGE                   0x18
#define VL53L0X_VCSEL_PERIOD_FINAL_RANGE                 0x08

/* TFmini constants */
#define TFMINI_FRAME_SIZE                                9
#define TFMINI_START_BYTE                                0x59

/** @} */

/* Private variables ---------------------------------------------------------*/

/** @defgroup LASER_DISTANCE_Private_Variables Private Variables
 * @{
 */

/* Status string table */
static const char* statusStrings[] = {
    "OK",
    "ERROR",
    "BUSY",
    "TIMEOUT",
    "INVALID_PARAM",
    "NOT_INITIALIZED",
    "OUT_OF_RANGE",
    "I2C_ERROR"
};

/** @} */

/* Private function prototypes -----------------------------------------------*/
static LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_ValidateConfig(LASER_DISTANCE_Config_t *config);
static LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_VL53L0X_Init(LASER_DISTANCE_Handle_t *hlaser);
static LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_VL53L0X_ReadRange(LASER_DISTANCE_Handle_t *hlaser);
static LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_TFmini_ReadRange(LASER_DISTANCE_Handle_t *hlaser);
static uint16_t LASER_DISTANCE_VL53L0X_DecodeRange(uint8_t *data);
static uint16_t LASER_DISTANCE_TFmini_DecodeRange(uint8_t *data);
static LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_I2C_WriteReg(LASER_DISTANCE_Handle_t *hlaser,
                                                               uint8_t reg,
                                                               uint8_t value);
static LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_I2C_ReadReg(LASER_DISTANCE_Handle_t *hlaser,
                                                              uint8_t reg,
                                                              uint8_t *value);
static LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_I2C_ReadMulti(LASER_DISTANCE_Handle_t *hlaser,
                                                                uint8_t reg,
                                                                uint8_t *buffer,
                                                                uint16_t length);

/* Exported functions -------------------------------------------------------*/

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
                                                LASER_DISTANCE_SensorType_t sensorType)
{
    if (hlaser == NULL || hi2c == NULL) {
        log_error("LASER_DISTANCE: Invalid parameters provided to LASER_DISTANCE_Init");
        return LASER_DISTANCE_INVALID_PARAM;
    }

    log_debug("LASER_DISTANCE: Initializing laser distance sensor with sensor type %d", sensorType);

    /* Initialize structure */
    memset(hlaser, 0, sizeof(LASER_DISTANCE_Handle_t));
    hlaser->hi2c = hi2c;

    /* Set default configuration based on sensor type */
    LASER_DISTANCE_Config_t default_config = LASER_DISTANCE_GetDefaultConfig(sensorType);
    LASER_DISTANCE_Config(hlaser, &default_config);

    /* Initialize specific sensor */
    LASER_DISTANCE_StatusTypeDef status;
    switch (sensorType) {
        case LASER_DISTANCE_VL53L0X:
            log_debug("LASER_DISTANCE: Initializing VL53L0X sensor");
            status = LASER_DISTANCE_VL53L0X_Init(hlaser);
            break;
        case LASER_DISTANCE_VL53L1X:
            /* VL53L1X initialization would go here */
            log_warning("LASER_DISTANCE: VL53L1X not implemented yet");
            status = LASER_DISTANCE_ERROR;  /* Not implemented yet */
            break;
        case LASER_DISTANCE_TFMINI:
            /* TFmini doesn't need special initialization */
            log_debug("LASER_DISTANCE: TFmini sensor initialized");
            status = LASER_DISTANCE_OK;
            break;
        case LASER_DISTANCE_CUSTOM:
            log_debug("LASER_DISTANCE: Custom sensor initialized");
            status = LASER_DISTANCE_OK;
            break;
        default:
            log_error("LASER_DISTANCE: Invalid sensor type %d", sensorType);
            status = LASER_DISTANCE_INVALID_PARAM;
            break;
    }

    if (status == LASER_DISTANCE_OK) {
        hlaser->isInitialized = true;
        log_info("LASER_DISTANCE: Laser distance sensor initialized successfully");
    } else {
        log_error("LASER_DISTANCE: Failed to initialize sensor, status: %d", status);
    }

    return status;
}

/**
 * @brief   Deinitialize laser distance sensor
 * @details Stops sensor and releases resources
 * @param   hlaser Pointer to laser distance sensor handle
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_DeInit(LASER_DISTANCE_Handle_t *hlaser)
{
    if (hlaser == NULL) {
        return LASER_DISTANCE_INVALID_PARAM;
    }

    /* Stop continuous mode if active */
    LASER_DISTANCE_StopContinuous(hlaser);

    hlaser->isInitialized = false;

    return LASER_DISTANCE_OK;
}

/**
 * @brief   Configure laser distance sensor parameters
 * @details Sets sensor configuration parameters
 * @param   hlaser Pointer to laser distance sensor handle
 * @param   config Pointer to configuration structure
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_Config(LASER_DISTANCE_Handle_t *hlaser,
                                                  LASER_DISTANCE_Config_t *config)
{
    if (hlaser == NULL || config == NULL) {
        return LASER_DISTANCE_INVALID_PARAM;
    }

    /* Validate configuration */
    if (LASER_DISTANCE_ValidateConfig(config) != LASER_DISTANCE_OK) {
        return LASER_DISTANCE_INVALID_PARAM;
    }

    /* Store configuration */
    hlaser->config = *config;

    return LASER_DISTANCE_OK;
}

/**
 * @brief   Start continuous measurement mode
 * @details Configures sensor for continuous distance measurement
 * @param   hlaser Pointer to laser distance sensor handle
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_StartContinuous(LASER_DISTANCE_Handle_t *hlaser)
{
    if (hlaser == NULL || !hlaser->isInitialized) {
        return LASER_DISTANCE_NOT_INITIALIZED;
    }

    /* Implementation depends on sensor type */
    switch (hlaser->config.sensorType) {
        case LASER_DISTANCE_VL53L0X:
            /* VL53L0X continuous mode setup */
            return LASER_DISTANCE_I2C_WriteReg(hlaser, VL53L0X_REG_SYSRANGE_START, 0x02);
        case LASER_DISTANCE_TFMINI:
            /* TFmini is always in continuous mode */
            hlaser->isMeasuring = true;
            return LASER_DISTANCE_OK;
        default:
            return LASER_DISTANCE_ERROR;
    }
}

/**
 * @brief   Stop continuous measurement mode
 * @details Stops continuous measurement
 * @param   hlaser Pointer to laser distance sensor handle
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_StopContinuous(LASER_DISTANCE_Handle_t *hlaser)
{
    if (hlaser == NULL) {
        return LASER_DISTANCE_INVALID_PARAM;
    }

    hlaser->isMeasuring = false;

    /* Implementation depends on sensor type */
    switch (hlaser->config.sensorType) {
        case LASER_DISTANCE_VL53L0X:
            /* VL53L0X stop continuous mode */
            return LASER_DISTANCE_I2C_WriteReg(hlaser, VL53L0X_REG_SYSRANGE_START, 0x01);
        default:
            return LASER_DISTANCE_OK;
    }
}

/**
 * @brief   Perform single distance measurement
 * @details Triggers single measurement and returns result
 * @param   hlaser Pointer to laser distance sensor handle
 * @retval  uint16_t Distance in millimeters (0 if measurement failed)
 */
uint16_t LASER_DISTANCE_MeasureDistance(LASER_DISTANCE_Handle_t *hlaser)
{
    if (hlaser == NULL || !hlaser->isInitialized) {
        log_error("LASER_DISTANCE: Sensor not initialized");
        return 0;
    }

    log_debug("LASER_DISTANCE: Starting distance measurement, sensor type: %d", hlaser->config.sensorType);

    LASER_DISTANCE_StatusTypeDef status;

    /* Perform measurement based on sensor type */
    switch (hlaser->config.sensorType) {
        case LASER_DISTANCE_VL53L0X:
            status = LASER_DISTANCE_VL53L0X_ReadRange(hlaser);
            break;
        case LASER_DISTANCE_TFMINI:
            status = LASER_DISTANCE_TFmini_ReadRange(hlaser);
            break;
        default:
            log_error("LASER_DISTANCE: Unsupported sensor type %d", hlaser->config.sensorType);
            return 0;
    }

    if (status == LASER_DISTANCE_OK) {
        hlaser->lastMeasurement.timestamp = HAL_GetTick();
        log_debug("LASER_DISTANCE: Measurement complete - Distance: %d mm, Status: %d",
                 hlaser->lastMeasurement.distance, hlaser->lastMeasurement.rangeStatus);
        return hlaser->lastMeasurement.distance;
    } else {
        log_warning("LASER_DISTANCE: Measurement failed with status: %d", status);
    }

    return 0;
}

/**
 * @brief   Get last measurement data
 * @details Returns complete measurement data structure
 * @param   hlaser Pointer to laser distance sensor handle
 * @param   measurement Pointer to measurement data structure
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_GetMeasurement(LASER_DISTANCE_Handle_t *hlaser,
                                                          LASER_DISTANCE_Measurement_t *measurement)
{
    if (hlaser == NULL || measurement == NULL) {
        return LASER_DISTANCE_INVALID_PARAM;
    }

    *measurement = hlaser->lastMeasurement;

    return LASER_DISTANCE_OK;
}

/**
 * @brief   Get last measured distance
 * @details Returns the last measured distance in millimeters
 * @param   hlaser Pointer to laser distance sensor handle
 * @retval  uint16_t Distance in millimeters (0 if no valid measurement)
 */
uint16_t LASER_DISTANCE_GetDistance(LASER_DISTANCE_Handle_t *hlaser)
{
    if (hlaser == NULL) {
        return 0;
    }

    return hlaser->lastMeasurement.distance;
}

/**
 * @brief   Check if new measurement is available
 * @details Returns true if new measurement data is ready
 * @param   hlaser Pointer to laser distance sensor handle
 * @retval  bool True if measurement is ready
 */
bool LASER_DISTANCE_IsMeasurementReady(LASER_DISTANCE_Handle_t *hlaser)
{
    if (hlaser == NULL) {
        return false;
    }

    /* Implementation depends on sensor type */
    switch (hlaser->config.sensorType) {
        case LASER_DISTANCE_VL53L0X: {
            uint8_t status;
            if (LASER_DISTANCE_I2C_ReadReg(hlaser, VL53L0X_REG_RESULT_INTERRUPT_STATUS, &status) == LASER_DISTANCE_OK) {
                return (status & 0x07) != 0;
            }
            break;
        }
        case LASER_DISTANCE_TFMINI:
            /* TFmini always has data available in continuous mode */
            return hlaser->isMeasuring;
        default:
            break;
    }

    return false;
}

/**
 * @brief   Change I2C address
 * @details Changes sensor I2C slave address (VL53L0X only)
 * @param   hlaser Pointer to laser distance sensor handle
 * @param   newAddress New I2C address
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_ChangeAddress(LASER_DISTANCE_Handle_t *hlaser,
                                                         uint8_t newAddress)
{
    if (hlaser == NULL || !hlaser->isInitialized) {
        return LASER_DISTANCE_NOT_INITIALIZED;
    }

    if (hlaser->config.sensorType != LASER_DISTANCE_VL53L0X) {
        return LASER_DISTANCE_INVALID_PARAM;
    }

    /* Write new address to sensor */
    LASER_DISTANCE_StatusTypeDef status = LASER_DISTANCE_I2C_WriteReg(hlaser,
                                                                     VL53L0X_REG_I2C_SLAVE_DEVICE_ADDRESS,
                                                                     newAddress >> 1);

    if (status == LASER_DISTANCE_OK) {
        hlaser->config.i2cAddress = newAddress;
    }

    return status;
}

/**
 * @brief   Perform sensor calibration
 * @details Calibrates sensor offset and crosstalk (VL53L0X only)
 * @param   hlaser Pointer to laser distance sensor handle
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_Calibrate(LASER_DISTANCE_Handle_t *hlaser)
{
    if (hlaser == NULL || !hlaser->isInitialized) {
        return LASER_DISTANCE_NOT_INITIALIZED;
    }

    if (hlaser->config.sensorType != LASER_DISTANCE_VL53L0X) {
        return LASER_DISTANCE_INVALID_PARAM;
    }

    /* Basic calibration sequence for VL53L0X */
    /* This is a simplified version - full calibration requires reference surfaces */

    /* Set VCSEL periods */
    LASER_DISTANCE_I2C_WriteReg(hlaser, VL53L0X_REG_PRE_RANGE_CONFIG_VCSEL_PERIOD, VL53L0X_VCSEL_PERIOD_PRE_RANGE);
    LASER_DISTANCE_I2C_WriteReg(hlaser, VL53L0X_REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD, VL53L0X_VCSEL_PERIOD_FINAL_RANGE);

    return LASER_DISTANCE_OK;
}

/**
 * @brief   Get default configuration for sensor type
 * @details Returns default configuration for specified sensor
 * @param   sensorType Type of laser distance sensor
 * @retval  LASER_DISTANCE_Config_t Default configuration
 */
LASER_DISTANCE_Config_t LASER_DISTANCE_GetDefaultConfig(LASER_DISTANCE_SensorType_t sensorType)
{
    LASER_DISTANCE_Config_t config = {
        .sensorType = sensorType,
        .averagingSamples = LASER_DISTANCE_DEFAULT_AVERAGING_SAMPLES,
        .measurementTimeout = LASER_DISTANCE_DEFAULT_MEASUREMENT_TIMEOUT,
        .i2cAddress = LASER_DISTANCE_DEFAULT_I2C_ADDRESS,
        .longRangeMode = false,
        .highAccuracyMode = false
    };

    /* Set range based on sensor type */
    switch (sensorType) {
        case LASER_DISTANCE_VL53L0X:
            config.minDistance = LASER_DISTANCE_VL53L0X_MIN;
            config.maxDistance = LASER_DISTANCE_VL53L0X_MAX;
            break;
        case LASER_DISTANCE_VL53L1X:
            config.minDistance = LASER_DISTANCE_VL53L1X_MIN;
            config.maxDistance = LASER_DISTANCE_VL53L1X_MAX;
            break;
        case LASER_DISTANCE_TFMINI:
            config.minDistance = LASER_DISTANCE_TFMINI_MIN;
            config.maxDistance = LASER_DISTANCE_TFMINI_MAX;
            break;
        case LASER_DISTANCE_CUSTOM:
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
 * @param   hlaser Pointer to laser distance sensor handle
 * @param   distance Distance to validate
 * @retval  bool True if distance is valid
 */
bool LASER_DISTANCE_IsValidDistance(LASER_DISTANCE_Handle_t *hlaser, uint16_t distance)
{
    if (hlaser == NULL) {
        return false;
    }

    return (distance >= hlaser->config.minDistance && distance <= hlaser->config.maxDistance);
}

/**
 * @brief   Get sensor status string
 * @details Returns human-readable status description
 * @param   status Status code
 * @retval  const char* Status description string
 */
const char* LASER_DISTANCE_GetStatusString(LASER_DISTANCE_StatusTypeDef status)
{
    if (status >= sizeof(statusStrings) / sizeof(statusStrings[0])) {
        return "UNKNOWN";
    }

    return statusStrings[status];
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Validate configuration
 * @details Checks if configuration parameters are valid
 * @param   config Pointer to configuration structure
 * @retval  LASER_DISTANCE_StatusTypeDef Validation status
 */
static LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_ValidateConfig(LASER_DISTANCE_Config_t *config)
{
    if (config->minDistance >= config->maxDistance) {
        return LASER_DISTANCE_INVALID_PARAM;
    }

    if (config->averagingSamples == 0 || config->averagingSamples > 100) {
        return LASER_DISTANCE_INVALID_PARAM;
    }

    if (config->i2cAddress < 0x08 || config->i2cAddress > 0x77) {
        return LASER_DISTANCE_INVALID_PARAM;
    }

    return LASER_DISTANCE_OK;
}

/**
 * @brief   Initialize VL53L0X sensor
 * @details Performs VL53L0X specific initialization sequence
 * @param   hlaser Pointer to laser distance sensor handle
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
static LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_VL53L0X_Init(LASER_DISTANCE_Handle_t *hlaser)
{
    uint8_t deviceId;

    log_debug("LASER_DISTANCE: Initializing VL53L0X sensor");

    /* Check device ID */
    if (LASER_DISTANCE_I2C_ReadReg(hlaser, VL53L0X_REG_IDENTIFICATION_MODEL_ID, &deviceId) != LASER_DISTANCE_OK) {
        log_error("LASER_DISTANCE: Failed to read VL53L0X device ID");
        return LASER_DISTANCE_I2C_ERROR;
    }

    log_debug("LASER_DISTANCE: VL53L0X device ID: 0x%02X", deviceId);

    if (deviceId != VL53L0X_EXPECTED_DEVICE_ID) {
        log_error("LASER_DISTANCE: Invalid VL53L0X device ID: 0x%02X, expected: 0x%02X",
                 deviceId, VL53L0X_EXPECTED_DEVICE_ID);
        return LASER_DISTANCE_ERROR;
    }

    /* Basic initialization sequence */
    /* This is a simplified version - full initialization requires more steps */

    /* Set readout averaging sample period */
    LASER_DISTANCE_I2C_WriteReg(hlaser, VL53L0X_READOUT_AVERAGING_SAMPLE_PERIOD, 0x30);

    /* Set VCSEL periods */
    LASER_DISTANCE_I2C_WriteReg(hlaser, VL53L0X_REG_PRE_RANGE_CONFIG_VCSEL_PERIOD, VL53L0X_VCSEL_PERIOD_PRE_RANGE);
    LASER_DISTANCE_I2C_WriteReg(hlaser, VL53L0X_REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD, VL53L0X_VCSEL_PERIOD_FINAL_RANGE);

    log_debug("LASER_DISTANCE: VL53L0X initialization completed");
    return LASER_DISTANCE_OK;
}

/**
 * @brief   Read range from VL53L0X sensor
 * @details Performs single range measurement with VL53L0X
 * @param   hlaser Pointer to laser distance sensor handle
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
static LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_VL53L0X_ReadRange(LASER_DISTANCE_Handle_t *hlaser)
{
    uint8_t rangeData[12];
    uint32_t startTime = HAL_GetTick();

    /* Start single measurement */
    LASER_DISTANCE_I2C_WriteReg(hlaser, VL53L0X_REG_SYSRANGE_START, 0x01);

    /* Wait for measurement completion */
    while ((HAL_GetTick() - startTime) < hlaser->config.measurementTimeout) {
        uint8_t status;
        if (LASER_DISTANCE_I2C_ReadReg(hlaser, VL53L0X_REG_RESULT_INTERRUPT_STATUS, &status) == LASER_DISTANCE_OK) {
            if (status & 0x07) {
                break;
            }
        }
        HAL_Delay(1);
    }

    /* Read range data */
    if (LASER_DISTANCE_I2C_ReadMulti(hlaser, VL53L0X_REG_RESULT_RANGE_STATUS, rangeData, 12) != LASER_DISTANCE_OK) {
        return LASER_DISTANCE_I2C_ERROR;
    }

    /* Decode range */
    hlaser->lastMeasurement.distance = LASER_DISTANCE_VL53L0X_DecodeRange(rangeData);
    hlaser->lastMeasurement.rangeStatus = rangeData[0];
    hlaser->lastMeasurement.signalRate = (rangeData[7] << 8) | rangeData[6];
    hlaser->lastMeasurement.ambientRate = (rangeData[9] << 8) | rangeData[8];

    /* Clear interrupt */
    LASER_DISTANCE_I2C_WriteReg(hlaser, VL53L0X_REG_SYSRANGE_START, 0x00);

    return LASER_DISTANCE_OK;
}

/**
 * @brief   Read range from TFmini sensor
 * @details Reads distance data from TFmini serial protocol
 * @param   hlaser Pointer to laser distance sensor handle
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
static LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_TFmini_ReadRange(LASER_DISTANCE_Handle_t *hlaser)
{
    /* TFmini uses UART, not I2C - this is a placeholder */
    /* Implementation would require UART interface */

    hlaser->lastMeasurement.distance = 0;  /* Placeholder */

    return LASER_DISTANCE_ERROR;  /* Not implemented */
}

/**
 * @brief   Decode range data from VL53L0X
 * @details Extracts distance from VL53L0X range data
 * @param   data Pointer to range data buffer
 * @retval  uint16_t Distance in millimeters
 */
static uint16_t LASER_DISTANCE_VL53L0X_DecodeRange(uint8_t *data)
{
    return ((data[11] << 8) | data[10]);
}

/**
 * @brief   Write register via I2C
 * @details Writes single byte to sensor register
 * @param   hlaser Pointer to laser distance sensor handle
 * @param   reg Register address
 * @param   value Value to write
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
static LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_I2C_WriteReg(LASER_DISTANCE_Handle_t *hlaser,
                                                               uint8_t reg,
                                                               uint8_t value)
{
    if (I2C_Mem_Write(hlaser->config.i2cAddress, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, 100) == I2C_OK) {
        return LASER_DISTANCE_OK;
    }

    return LASER_DISTANCE_I2C_ERROR;
}

/**
 * @brief   Read register via I2C
 * @details Reads single byte from sensor register
 * @param   hlaser Pointer to laser distance sensor handle
 * @param   reg Register address
 * @param   value Pointer to store read value
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
static LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_I2C_ReadReg(LASER_DISTANCE_Handle_t *hlaser,
                                                              uint8_t reg,
                                                              uint8_t *value)
{
    if (I2C_Mem_Read(hlaser->config.i2cAddress, reg, I2C_MEMADD_SIZE_8BIT, value, 1, 100) == I2C_OK) {
        return LASER_DISTANCE_OK;
    }

    return LASER_DISTANCE_I2C_ERROR;
}

/**
 * @brief   Read multiple registers via I2C
 * @details Reads multiple bytes from sensor registers
 * @param   hlaser Pointer to laser distance sensor handle
 * @param   reg Starting register address
 * @param   buffer Pointer to data buffer
 * @param   length Number of bytes to read
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
static LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_I2C_ReadMulti(LASER_DISTANCE_Handle_t *hlaser,
                                                                uint8_t reg,
                                                                uint8_t *buffer,
                                                                uint16_t length)
{
    if (I2C_Mem_Read(hlaser->config.i2cAddress, reg, I2C_MEMADD_SIZE_8BIT, buffer, length, 100) == I2C_OK) {
        return LASER_DISTANCE_OK;
    }

    return LASER_DISTANCE_I2C_ERROR;
}
