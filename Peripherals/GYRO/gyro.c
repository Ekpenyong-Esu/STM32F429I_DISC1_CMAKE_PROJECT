/**
 * @file gyro.c
 * @brief GYRO driver implementation for L3GD20 gyroscope
 * @details This file provides the implementation for the L3GD20 gyroscope
 *          sensor configuration and control on STM32F429 Discovery board.
 * @version 1.0
 * @date 2025-09-03
 * @author STM32 Team
 */

/* Includes ------------------------------------------------------------------*/
#include "gyro.h"
#include "main.h"
#include <stdio.h>

/* Private function prototypes -----------------------------------------------*/
static HAL_StatusTypeDef GYRO_WriteRegister(GYRO_Handle_t *handle, uint8_t reg, uint8_t value);
static HAL_StatusTypeDef GYRO_ReadRegister(GYRO_Handle_t *handle, uint8_t reg, uint8_t *value);
static HAL_StatusTypeDef GYRO_ReadMultiple(GYRO_Handle_t *handle, uint8_t reg, uint8_t *buffer, uint8_t count);
static HAL_StatusTypeDef GYRO_ValidateHandle(GYRO_Handle_t *handle);
static void GYRO_CSLow(GYRO_Handle_t *handle);
static void GYRO_CSHigh(GYRO_Handle_t *handle);

/**
 * @brief Get device name string from device ID
 * @param deviceId: Device ID value
 * @return const char*: Device name string
 */
static const char* GYRO_GetDeviceName(uint8_t deviceId) {
    switch (deviceId) {
        case L3GD20_DEVICE_ID:
            return "L3GD20";
        case L3GD20H_DEVICE_ID:
            return "L3GD20H";
        case L3G4200D_DEVICE_ID:
            return "L3G4200D";
        default:
            return "Unknown";
    }
}

/**
 * @brief Initialize GYRO sensor
 * @param handle: Pointer to GYRO handle structure
 * @param hspi: Pointer to SPI handle
 * @param csPort: Chip select GPIO port
 * @param csPin: Chip select GPIO pin
 * @param config: Pointer to configuration structure
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_Init(GYRO_Handle_t *handle, SPI_HandleTypeDef *hspi,
                           GPIO_TypeDef *csPort, uint16_t csPin,
                           GYRO_Config_t *config) {
    if (handle == NULL || hspi == NULL || csPort == NULL || config == NULL) {
        return HAL_ERROR;
    }

    /* Initialize handle */
    handle->hspi = hspi;
    handle->csPort = csPort;
    handle->csPin = csPin;
    handle->config = *config;
    handle->initialized = false;
    handle->errorCode = GYRO_ERROR_NONE;

    /* Set CS pin high initially */
    GYRO_CSHigh(handle);

    /* Small delay for sensor startup */
    HAL_Delay(10);  // Increased delay for L3G4200D compatibility

    /* Check device ID */
    uint8_t deviceId = 0;
    HAL_StatusTypeDef status = GYRO_GetDeviceID(handle, &deviceId);
    if (status != HAL_OK) {
        handle->errorCode = GYRO_ERROR_SPI;
        return status;
    }

    /* Check if device ID matches any supported variant */
    if (deviceId != L3GD20_DEVICE_ID &&
        deviceId != L3GD20H_DEVICE_ID &&
        deviceId != L3G4200D_DEVICE_ID) {
        printf("ERROR: Unsupported gyro device ID: 0x%02X (expected 0x%02X, 0x%02X, or 0x%02X)\r\n",
               deviceId, L3GD20_DEVICE_ID, L3GD20H_DEVICE_ID, L3G4200D_DEVICE_ID);
        handle->errorCode = GYRO_ERROR_DEVICE_ID;
        return HAL_ERROR;
    }

    /* Print detected device information */
    printf("✓ Gyroscope detected: %s (ID: 0x%02X)\r\n", GYRO_GetDeviceName(deviceId), deviceId);

    /* Configure CTRL_REG1 */
    uint8_t ctrl1 = config->powerMode | config->outputDataRate |
                    config->bandwidth | config->axes;
    status = GYRO_WriteRegister(handle, L3GD20_CTRL_REG1_ADDR, ctrl1);
    if (status != HAL_OK) {
        handle->errorCode = GYRO_ERROR_CONFIG;
        return status;
    }

    /* Configure CTRL_REG4 */
    uint8_t ctrl4 = config->blockDataUpdate | config->endianness | config->fullScale;
    status = GYRO_WriteRegister(handle, L3GD20_CTRL_REG4_ADDR, ctrl4);
    if (status != HAL_OK) {
        handle->errorCode = GYRO_ERROR_CONFIG;
        return status;
    }

    /* Enable all axes if not specified */
    if (config->axes == 0) {
        status = GYRO_SetAxes(handle, GYRO_XYZ_ENABLE);
        if (status != HAL_OK) {
            handle->errorCode = GYRO_ERROR_CONFIG;
            return status;
        }
    }

    handle->initialized = true;
    return HAL_OK;
}

/**
 * @brief Deinitialize GYRO sensor
 * @param handle: Pointer to GYRO handle structure
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_DeInit(GYRO_Handle_t *handle) {
    if (GYRO_ValidateHandle(handle) != HAL_OK) {
        return HAL_ERROR;
    }

    /* Put sensor in power-down mode */
    HAL_StatusTypeDef status = GYRO_WriteRegister(handle, L3GD20_CTRL_REG1_ADDR, GYRO_POWER_DOWN);

    /* Mark as not initialized */
    handle->initialized = false;
    handle->errorCode = GYRO_ERROR_NONE;

    return status;
}

/**
 * @brief Read gyroscope data
 * @param handle: Pointer to GYRO handle structure
 * @param data: Pointer to data structure
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_ReadData(GYRO_Handle_t *handle, GYRO_Data_t *data) {
    if (GYRO_ValidateHandle(handle) != HAL_OK || data == NULL) {
        return HAL_ERROR;
    }

    uint8_t buffer[6];
    HAL_StatusTypeDef status = GYRO_ReadMultiple(handle, L3GD20_OUT_X_L_ADDR, buffer, 6);

    if (status == HAL_OK) {
        /* Combine low and high bytes for each axis */
        data->x = (int16_t)((buffer[1] << 8) | buffer[0]);
        data->y = (int16_t)((buffer[3] << 8) | buffer[2]);
        data->z = (int16_t)((buffer[5] << 8) | buffer[4]);
    } else {
        handle->errorCode = GYRO_ERROR_READ;
    }

    return status;
}

/**
 * @brief Read single axis data
 * @param handle: Pointer to GYRO handle structure
 * @param axis: Axis to read (0=X, 1=Y, 2=Z)
 * @param value: Pointer to store the value
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_ReadAxis(GYRO_Handle_t *handle, uint8_t axis, int16_t *value) {
    if (GYRO_ValidateHandle(handle) != HAL_OK || value == NULL || axis > 2) {
        return HAL_ERROR;
    }

    uint8_t buffer[2];
    uint8_t regAddr;

    switch (axis) {
        case 0: regAddr = L3GD20_OUT_X_L_ADDR; break;
        case 1: regAddr = L3GD20_OUT_Y_L_ADDR; break;
        case 2: regAddr = L3GD20_OUT_Z_L_ADDR; break;
        default: return HAL_ERROR;
    }

    HAL_StatusTypeDef status = GYRO_ReadMultiple(handle, regAddr, buffer, 2);

    if (status == HAL_OK) {
        *value = (int16_t)((buffer[1] << 8) | buffer[0]);
    } else {
        handle->errorCode = GYRO_ERROR_READ;
    }

    return status;
}

/**
 * @brief Read temperature
 * @param handle: Pointer to GYRO handle structure
 * @param temperature: Pointer to store temperature value
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_ReadTemperature(GYRO_Handle_t *handle, int8_t *temperature) {
    if (GYRO_ValidateHandle(handle) != HAL_OK || temperature == NULL) {
        return HAL_ERROR;
    }

    uint8_t temp;
    HAL_StatusTypeDef status = GYRO_ReadRegister(handle, L3GD20_OUT_TEMP_ADDR, &temp);

    if (status == HAL_OK) {
        *temperature = (int8_t)temp;
    } else {
        handle->errorCode = GYRO_ERROR_READ;
    }

    return status;
}

/**
 * @brief Check if new data is available
 * @param handle: Pointer to GYRO handle structure
 * @param dataReady: Pointer to store data ready status
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_IsDataReady(GYRO_Handle_t *handle, bool *dataReady) {
    if (GYRO_ValidateHandle(handle) != HAL_OK || dataReady == NULL) {
        return HAL_ERROR;
    }

    uint8_t status_reg;
    HAL_StatusTypeDef status = GYRO_ReadRegister(handle, L3GD20_STATUS_REG_ADDR, &status_reg);

    if (status == HAL_OK) {
        *dataReady = (status_reg & 0x08) ? true : false; /* Check ZYXDA bit */
    } else {
        handle->errorCode = GYRO_ERROR_READ;
    }

    return status;
}

/**
 * @brief Get device ID
 * @param handle: Pointer to GYRO handle structure
 * @param deviceId: Pointer to store device ID
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_GetDeviceID(GYRO_Handle_t *handle, uint8_t *deviceId) {
    if (handle == NULL || deviceId == NULL) {
        return HAL_ERROR;
    }

    return GYRO_ReadRegister(handle, L3GD20_WHO_AM_I_ADDR, deviceId);
}

/**
 * @brief Configure interrupt
 * @param handle: Pointer to GYRO handle structure
 * @param config: Interrupt configuration
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_ConfigInterrupt(GYRO_Handle_t *handle, uint8_t config) {
    if (GYRO_ValidateHandle(handle) != HAL_OK) {
        return HAL_ERROR;
    }

    return GYRO_WriteRegister(handle, L3GD20_CTRL_REG3_ADDR, config);
}

/**
 * @brief Set full scale range
 * @param handle: Pointer to GYRO handle structure
 * @param fullScale: Full scale value
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_SetFullScale(GYRO_Handle_t *handle, uint8_t fullScale) {
    if (GYRO_ValidateHandle(handle) != HAL_OK) {
        return HAL_ERROR;
    }

    uint8_t ctrl4;
    HAL_StatusTypeDef status = GYRO_ReadRegister(handle, L3GD20_CTRL_REG4_ADDR, &ctrl4);
    if (status != HAL_OK) {
        return status;
    }

    /* Clear full scale bits and set new value */
    ctrl4 = (ctrl4 & 0xCF) | (fullScale & 0x30);
    status = GYRO_WriteRegister(handle, L3GD20_CTRL_REG4_ADDR, ctrl4);

    if (status == HAL_OK) {
        handle->config.fullScale = fullScale;
    }

    return status;
}

/**
 * @brief Set output data rate
 * @param handle: Pointer to GYRO handle structure
 * @param dataRate: Output data rate value
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_SetOutputDataRate(GYRO_Handle_t *handle, uint8_t dataRate) {
    if (GYRO_ValidateHandle(handle) != HAL_OK) {
        return HAL_ERROR;
    }

    uint8_t ctrl1;
    HAL_StatusTypeDef status = GYRO_ReadRegister(handle, L3GD20_CTRL_REG1_ADDR, &ctrl1);
    if (status != HAL_OK) {
        return status;
    }

    /* Clear data rate bits and set new value */
    ctrl1 = (ctrl1 & 0x3F) | (dataRate & 0xC0);
    status = GYRO_WriteRegister(handle, L3GD20_CTRL_REG1_ADDR, ctrl1);

    if (status == HAL_OK) {
        handle->config.outputDataRate = dataRate;
    }

    return status;
}

/**
 * @brief Enable/disable axes
 * @param handle: Pointer to GYRO handle structure
 * @param axes: Axes to enable/disable
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_SetAxes(GYRO_Handle_t *handle, uint8_t axes) {
    if (GYRO_ValidateHandle(handle) != HAL_OK) {
        return HAL_ERROR;
    }

    uint8_t ctrl1;
    HAL_StatusTypeDef status = GYRO_ReadRegister(handle, L3GD20_CTRL_REG1_ADDR, &ctrl1);
    if (status != HAL_OK) {
        return status;
    }

    /* Clear axes bits and set new value */
    ctrl1 = (ctrl1 & 0xF8) | (axes & 0x07);
    status = GYRO_WriteRegister(handle, L3GD20_CTRL_REG1_ADDR, ctrl1);

    if (status == HAL_OK) {
        handle->config.axes = axes;
    }

    return status;
}

/**
 * @brief Convert raw data to angular velocity (dps)
 * @param rawValue: Raw sensor value
 * @param fullScale: Current full scale setting
 * @return float: Angular velocity in degrees per second
 */
float GYRO_ConvertToDPS(int16_t rawValue, uint8_t fullScale) {
    float sensitivity;

    switch (fullScale) {
        case GYRO_FULLSCALE_250:
            sensitivity = 8.75f; /* mdps/LSB */
            break;
        case GYRO_FULLSCALE_500:
            sensitivity = 17.5f; /* mdps/LSB */
            break;
        case GYRO_FULLSCALE_2000:
            sensitivity = 70.0f; /* mdps/LSB */
            break;
        default:
            sensitivity = 8.75f;
            break;
    }

    return (float)rawValue * sensitivity / 1000.0f; /* Convert mdps to dps */
}

/**
 * @brief Get last error code
 * @param handle: Pointer to GYRO handle structure
 * @return uint32_t: Error code
 */
uint32_t GYRO_GetError(GYRO_Handle_t *handle) {
    if (handle == NULL) {
        return GYRO_ERROR_INIT;
    }
    return handle->errorCode;
}

/* Private Functions ---------------------------------------------------------*/

/**
 * @brief Write to a register
 * @param handle: Pointer to GYRO handle structure
 * @param reg: Register address
 * @param value: Value to write
 * @return HAL_StatusTypeDef: HAL status
 */
static HAL_StatusTypeDef GYRO_WriteRegister(GYRO_Handle_t *handle, uint8_t reg, uint8_t value) {
    uint8_t txData[2];

    txData[0] = reg | L3GD20_SPI_WRITE;
    txData[1] = value;

    GYRO_CSLow(handle);
    HAL_StatusTypeDef status = HAL_SPI_Transmit(handle->hspi, txData, 2, GYRO_SPI_TIMEOUT);
    GYRO_CSHigh(handle);

    return status;
}

/**
 * @brief Read from a register
 * @param handle: Pointer to GYRO handle structure
 * @param reg: Register address
 * @param value: Pointer to store read value
 * @return HAL_StatusTypeDef: HAL status
 */
static HAL_StatusTypeDef GYRO_ReadRegister(GYRO_Handle_t *handle, uint8_t reg, uint8_t *value) {
    uint8_t txData[2] = {reg | L3GD20_SPI_READ, 0x00};
    uint8_t rxData[2] = {0xFF, 0xFF}; // Initialize with default values

    GYRO_CSLow(handle);

    // Add a small delay after CS low for device variants
    for(volatile int i = 0; i < 10; i++) {
        // Delay loop for CS setup
    }

    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(handle->hspi, txData, rxData, 2, GYRO_SPI_TIMEOUT);

    // Add a small delay before CS high
    for(volatile int i = 0; i < 5; i++) {
        // Delay loop for data hold
    }

    GYRO_CSHigh(handle);

    if (status == HAL_OK) {
        *value = rxData[1];  // Second byte contains the data
    }

    return status;
}

/**
 * @brief Read multiple bytes from consecutive registers
 * @param handle: Pointer to GYRO handle structure
 * @param reg: Starting register address
 * @param buffer: Buffer to store read data
 * @param count: Number of bytes to read
 * @return HAL_StatusTypeDef: HAL status
 */
static HAL_StatusTypeDef GYRO_ReadMultiple(GYRO_Handle_t *handle, uint8_t reg, uint8_t *buffer, uint8_t count) {
    // Use fixed-size buffers (maximum expected read is 6 bytes for XYZ data)
    uint8_t txData[8];  // Command + up to 7 data bytes
    uint8_t rxData[8];

    if (count > 7) {
        return HAL_ERROR;  // Prevent buffer overflow
    }

    // Prepare transmit data
    txData[0] = reg | L3GD20_SPI_READ | L3GD20_SPI_MULTIPLEBYTE;
    for (int i = 1; i <= count; i++) {
        txData[i] = 0x00;  // Dummy bytes
    }

    GYRO_CSLow(handle);
    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(handle->hspi, txData, rxData, count + 1, GYRO_SPI_TIMEOUT);
    GYRO_CSHigh(handle);

    if (status == HAL_OK) {
        // Copy received data (skip first byte which is response to command)
        for (int i = 0; i < count; i++) {
            buffer[i] = rxData[i + 1];
        }
    }

    return status;
}

/**
 * @brief Validate GYRO handle
 * @param handle: Pointer to GYRO handle structure
 * @return HAL_StatusTypeDef: HAL status
 */
static HAL_StatusTypeDef GYRO_ValidateHandle(GYRO_Handle_t *handle) {
    if (handle == NULL || !handle->initialized) {
        return HAL_ERROR;
    }
    return HAL_OK;
}

/**
 * @brief Set CS pin low
 * @param handle: Pointer to GYRO handle structure
 */
static void GYRO_CSLow(GYRO_Handle_t *handle) {
    HAL_GPIO_WritePin(handle->csPort, handle->csPin, GPIO_PIN_RESET);
    // Small delay for CS setup time (L3GD20 requires ~5ns setup time)
    for(volatile int i = 0; i < 5; i++) {
        // Delay loop
    }
}

/**
 * @brief Set CS pin high
 * @param handle: Pointer to GYRO handle structure
 */
static void GYRO_CSHigh(GYRO_Handle_t *handle) {
    // Small delay before releasing CS (L3GD20 requires ~5ns hold time)
    for(volatile int i = 0; i < 5; i++) {
        // Delay loop
    }
    HAL_GPIO_WritePin(handle->csPort, handle->csPin, GPIO_PIN_SET);
    // Small delay after releasing CS
    for(volatile int i = 0; i < 20; i++) {
        // Delay loop
    }
}
