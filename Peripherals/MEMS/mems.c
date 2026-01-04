/**
  ******************************************************************************
  * @file    mems.c
  * @brief   MEMS sensors driver implementation for STM32F429 Discovery Board
  * @details This file provides the implementation of MEMS sensor functions
  *          for the L3GD20 gyroscope on the STM32F429 Discovery board.
  * @version 1.0
  * @date    2025-09-04
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "mems.h"
#include <stdlib.h>
#include <string.h>

/* Private constants ---------------------------------------------------------*/
#define MEMS_CALIBRATION_SAMPLES_DEFAULT    100
#define MEMS_DELAY_MS(x)                   HAL_Delay(x)

/* Private variables ---------------------------------------------------------*/
static const char* MEMS_DEVICE_NAME = "L3GD20";
static const float MEMS_SENSITIVITY_FACTORS[] = {
    L3GD20_SENSITIVITY_250DPS,
    L3GD20_SENSITIVITY_500DPS,
    L3GD20_SENSITIVITY_2000DPS
};

/* Private function prototypes -----------------------------------------------*/
static MEMS_StatusTypeDef MEMS_WriteRegister(MEMS_HandleTypeDef *hmems, uint8_t addr, uint8_t data);
static MEMS_StatusTypeDef MEMS_ReadRegister(MEMS_HandleTypeDef *hmems, uint8_t addr, uint8_t *data);
static void MEMS_CS_High(MEMS_HandleTypeDef *hmems);
static void MEMS_CS_Low(MEMS_HandleTypeDef *hmems);
static MEMS_StatusTypeDef MEMS_InitGPIO(MEMS_HandleTypeDef *hmems);
static MEMS_StatusTypeDef MEMS_VerifyDevice(MEMS_HandleTypeDef *hmems);

/* Exported functions --------------------------------------------------------*/

/**
 * @brief Initialize MEMS sensors
 * @param hmems Pointer to MEMS handle structure
 * @param hspi Pointer to SPI handle
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_Init(MEMS_HandleTypeDef *hmems, SPI_HandleTypeDef *hspi)
{
    MEMS_StatusTypeDef status = MEMS_OK;

    /* Check parameters */
    if (hmems == NULL || hspi == NULL) {
        return MEMS_INVALID_PARAM;
    }

    /* Initialize structure */
    memset(hmems, 0, sizeof(MEMS_HandleTypeDef));
    hmems->hspi = hspi;

    /* Initialize GPIO pins */
    status = MEMS_InitGPIO(hmems);
    if (status != MEMS_OK) {
        return status;
    }

    /* SPI must be initialized by the application (use Peripherals/SPI SPI_Init() or SPI_Init_Custom()).
       MEMS will use the provided SPI handle and will NOT reconfigure it. */

    /* Set CS high initially */
    MEMS_CS_High(hmems);
    MEMS_DELAY_MS(1);

    /* Verify device presence */
    status = MEMS_VerifyDevice(hmems);
    if (status != MEMS_OK) {
        return status;
    }

    /* Set default configuration */
    MEMS_GyroConfigTypeDef default_config = {
        .OutputDataRate = MEMS_GYRO_ODR_95Hz,
        .FullScale = MEMS_GYRO_FULLSCALE_250,
        .Bandwidth = MEMS_GYRO_BANDWIDTH_1,
        .XAxisEnable = true,
        .YAxisEnable = true,
        .ZAxisEnable = true,
        .PowerDownMode = false
    };

    status = MEMS_GyroConfig(hmems, &default_config);
    if (status != MEMS_OK) {
        return status;
    }

    hmems->IsInitialized = true;
    return MEMS_OK;
}

/**
 * @brief Deinitialize MEMS sensors
 * @param hmems Pointer to MEMS handle structure
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_DeInit(MEMS_HandleTypeDef *hmems)
{
    if (hmems == NULL) {
        return MEMS_INVALID_PARAM;
    }

    /* Power down the device */
    MEMS_SetPowerMode(hmems, true);

    /* Reset structure */
    hmems->IsInitialized = false;
    hmems->IsCalibrated = false;

    return MEMS_OK;
}

/**
 * @brief Configure gyroscope sensor
 * @param hmems Pointer to MEMS handle structure
 * @param config Pointer to gyroscope configuration structure
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_GyroConfig(MEMS_HandleTypeDef *hmems, MEMS_GyroConfigTypeDef *config)
{
    MEMS_StatusTypeDef status = MEMS_OK;
    uint8_t ctrl_reg1 = 0;
    uint8_t ctrl_reg4 = 0;

    if (hmems == NULL || config == NULL) {
        return MEMS_INVALID_PARAM;
    }

    if (!hmems->IsInitialized) {
        return MEMS_NOT_INITIALIZED;
    }

    /* Configure CTRL_REG1 */
    if (!config->PowerDownMode) {
        ctrl_reg1 |= L3GD20_NORMAL_MODE;
    }

    /* Set output data rate */
    switch (config->OutputDataRate) {
        case MEMS_GYRO_ODR_95Hz:
            ctrl_reg1 |= L3GD20_ODR_95Hz;
            break;
        case MEMS_GYRO_ODR_190Hz:
            ctrl_reg1 |= L3GD20_ODR_190Hz;
            break;
        case MEMS_GYRO_ODR_380Hz:
            ctrl_reg1 |= L3GD20_ODR_380Hz;
            break;
        case MEMS_GYRO_ODR_760Hz:
            ctrl_reg1 |= L3GD20_ODR_760Hz;
            break;
        default:
            return MEMS_INVALID_PARAM;
    }

    /* Set bandwidth */
    switch (config->Bandwidth) {
        case MEMS_GYRO_BANDWIDTH_1:
            ctrl_reg1 |= L3GD20_BANDWIDTH_1;
            break;
        case MEMS_GYRO_BANDWIDTH_2:
            ctrl_reg1 |= L3GD20_BANDWIDTH_2;
            break;
        case MEMS_GYRO_BANDWIDTH_3:
            ctrl_reg1 |= L3GD20_BANDWIDTH_3;
            break;
        case MEMS_GYRO_BANDWIDTH_4:
            ctrl_reg1 |= L3GD20_BANDWIDTH_4;
            break;
        default:
            return MEMS_INVALID_PARAM;
    }

    /* Enable axes */
    if (config->XAxisEnable) {
        ctrl_reg1 |= 0x01;
    }
    if (config->YAxisEnable) {
        ctrl_reg1 |= 0x02;
    }
    if (config->ZAxisEnable) {
        ctrl_reg1 |= 0x04;
    }

    /* Configure CTRL_REG4 */
    switch (config->FullScale) {
        case MEMS_GYRO_FULLSCALE_250:
            ctrl_reg4 |= L3GD20_FULLSCALE_250;
            break;
        case MEMS_GYRO_FULLSCALE_500:
            ctrl_reg4 |= L3GD20_FULLSCALE_500;
            break;
        case MEMS_GYRO_FULLSCALE_2000:
            ctrl_reg4 |= L3GD20_FULLSCALE_2000;
            break;
        default:
            return MEMS_INVALID_PARAM;
    }

    /* Write configuration registers */
    status = MEMS_WriteRegister(hmems, L3GD20_CTRL_REG1_ADDR, ctrl_reg1);
    if (status != MEMS_OK) {
        return status;
    }

    status = MEMS_WriteRegister(hmems, L3GD20_CTRL_REG4_ADDR, ctrl_reg4);
    if (status != MEMS_OK) {
        return status;
    }

    /* Store configuration */
    hmems->GyroConfig = *config;

    /* Wait for configuration to take effect */
    MEMS_DELAY_MS(10);

    return MEMS_OK;
}

/**
 * @brief Read gyroscope raw data
 * @param hmems Pointer to MEMS handle structure
 * @param axes Pointer to store raw axes data
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_GyroReadRaw(MEMS_HandleTypeDef *hmems, MEMS_AxesRawTypeDef *axes)
{
    MEMS_StatusTypeDef status = MEMS_OK;
    uint8_t data[6];

    if (hmems == NULL || axes == NULL) {
        return MEMS_INVALID_PARAM;
    }

    if (!hmems->IsInitialized) {
        return MEMS_NOT_INITIALIZED;
    }

    /* Read 6 bytes starting from OUT_X_L register */
    status = MEMS_ReadRegisters(hmems, L3GD20_OUT_X_L_ADDR, data, 6);
    if (status != MEMS_OK) {
        return status;
    }

    /* Combine low and high bytes */
    axes->X = (int16_t)((data[1] << 8) | data[0]);
    axes->Y = (int16_t)((data[3] << 8) | data[2]);
    axes->Z = (int16_t)((data[5] << 8) | data[4]);

    return MEMS_OK;
}

/**
 * @brief Read gyroscope data in engineering units (dps)
 * @param hmems Pointer to MEMS handle structure
 * @param axes Pointer to store axes data in dps
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_GyroRead(MEMS_HandleTypeDef *hmems, MEMS_AxesTypeDef *axes)
{
    MEMS_StatusTypeDef status = MEMS_OK;
    MEMS_AxesRawTypeDef raw_data;

    if (hmems == NULL || axes == NULL) {
        return MEMS_INVALID_PARAM;
    }

    /* Read raw data */
    status = MEMS_GyroReadRaw(hmems, &raw_data);
    if (status != MEMS_OK) {
        return status;
    }

    /* Convert to engineering units */
    axes->X = MEMS_ConvertToDPS(raw_data.X, hmems->GyroConfig.FullScale);
    axes->Y = MEMS_ConvertToDPS(raw_data.Y, hmems->GyroConfig.FullScale);
    axes->Z = MEMS_ConvertToDPS(raw_data.Z, hmems->GyroConfig.FullScale);

    /* Apply calibration if available */
    if (hmems->IsCalibrated) {
        axes->X -= hmems->CalibrationOffset.X;
        axes->Y -= hmems->CalibrationOffset.Y;
        axes->Z -= hmems->CalibrationOffset.Z;
    }

    return MEMS_OK;
}

/**
 * @brief Read device temperature
 * @param hmems Pointer to MEMS handle structure
 * @param temperature Pointer to store temperature value
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_ReadTemperature(MEMS_HandleTypeDef *hmems, float *temperature)
{
    MEMS_StatusTypeDef status = MEMS_OK;
    uint8_t temp_raw = 0U;

    if (hmems == NULL || temperature == NULL) {
        return MEMS_INVALID_PARAM;
    }

    if (!hmems->IsInitialized) {
        return MEMS_NOT_INITIALIZED;
    }

    /* Read temperature register */
    status = MEMS_ReadRegister(hmems, L3GD20_OUT_TEMP_ADDR, &temp_raw);
    if (status != MEMS_OK) {
        return status;
    }

    /* Convert to temperature (simplified conversion) */
    *temperature = MEMS_TEMPERATURE_OFFSET + ((float)((int8_t)temp_raw));
    hmems->Temperature = *temperature;

    return MEMS_OK;
}

/**
 * @brief Configure interrupt settings
 * @param hmems Pointer to MEMS handle structure
 * @param config Pointer to interrupt configuration structure
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_ConfigureInterrupt(MEMS_HandleTypeDef *hmems, MEMS_InterruptConfigTypeDef *config)
{
    MEMS_StatusTypeDef status = MEMS_OK;
    uint8_t ctrl_reg3 = 0;

    if (hmems == NULL || config == NULL) {
        return MEMS_INVALID_PARAM;
    }

    if (!hmems->IsInitialized) {
        return MEMS_NOT_INITIALIZED;
    }

    /* Configure CTRL_REG3 for interrupt settings */
    if (config->InterruptEnable) {
        ctrl_reg3 |= 0x80;
    }

    if (config->BootStatusEnable) {
        ctrl_reg3 |= 0x40;
    }

    if (config->ActiveLevel) {
        ctrl_reg3 |= 0x20;
    }

    if (config->OutputType) {
        ctrl_reg3 |= 0x10;
    }

    if (config->DataReadyEnable) {
        ctrl_reg3 |= 0x08;
    }

    if (config->WatermarkEnable) {
        ctrl_reg3 |= 0x04;
    }

    if (config->OverrunEnable) {
        ctrl_reg3 |= 0x02;
    }

    if (config->EmptyEnable) {
        ctrl_reg3 |= 0x01;
    }

    /* Write interrupt configuration */
    status = MEMS_WriteRegister(hmems, L3GD20_CTRL_REG3_ADDR, ctrl_reg3);
    if (status != MEMS_OK) {
        return status;
    }

    /* Store configuration */
    hmems->IntConfig = *config;

    return MEMS_OK;
}

/**
 * @brief Calibrate gyroscope sensor
 * @param hmems Pointer to MEMS handle structure
 * @param samples Number of samples for calibration
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_CalibrateGyroscope(MEMS_HandleTypeDef *hmems, uint16_t samples)
{
    MEMS_StatusTypeDef status = MEMS_OK;
    MEMS_AxesTypeDef sum = {0.0f, 0.0f, 0.0f};
    MEMS_AxesTypeDef reading;
    uint16_t valid_samples = 0;

    if (hmems == NULL) {
        return MEMS_INVALID_PARAM;
    }

    if (!hmems->IsInitialized) {
        return MEMS_NOT_INITIALIZED;
    }

    if (samples == 0) {
        samples = MEMS_CALIBRATION_SAMPLES_DEFAULT;
    }

    /* Accumulate samples */
    for (uint16_t i = 0; i < samples; i++) {
        status = MEMS_GyroRead(hmems, &reading);
        if (status == MEMS_OK) {
            sum.X += reading.X;
            sum.Y += reading.Y;
            sum.Z += reading.Z;
            valid_samples++;
        }
        MEMS_DELAY_MS(10);
    }

    if (valid_samples == 0) {
        return MEMS_ERROR;
    }

    /* Calculate average offset */
    hmems->CalibrationOffset.X = sum.X / (float)valid_samples;
    hmems->CalibrationOffset.Y = sum.Y / (float)valid_samples;
    hmems->CalibrationOffset.Z = sum.Z / (float)valid_samples;

    hmems->IsCalibrated = true;

    return MEMS_OK;
}

/**
 * @brief Get device information
 * @param hmems Pointer to MEMS handle structure
 * @param info Pointer to store device information
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_GetDeviceInfo(MEMS_HandleTypeDef *hmems, MEMS_DeviceInfoTypeDef *info)
{
    MEMS_StatusTypeDef status = MEMS_OK;

    if (hmems == NULL || info == NULL) {
        return MEMS_INVALID_PARAM;
    }

    /* Read WHO_AM_I register */
    status = MEMS_ReadRegister(hmems, L3GD20_WHO_AM_I_ADDR, &info->WhoAmI);
    if (status != MEMS_OK) {
        return status;
    }

    /* Fill device information */
    info->DeviceName = MEMS_DEVICE_NAME;
    info->IsPresent = (info->WhoAmI == L3GD20_WHO_AM_I_VALUE);
    info->Version = 1;

    return MEMS_OK;
}

/**
 * @brief Read device status register
 * @param hmems Pointer to MEMS handle structure
 * @param status Pointer to store status value
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_ReadStatus(MEMS_HandleTypeDef *hmems, uint8_t *status_reg)
{
    if (hmems == NULL || status_reg == NULL) {
        return MEMS_INVALID_PARAM;
    }

    if (!hmems->IsInitialized) {
        return MEMS_NOT_INITIALIZED;
    }

    return MEMS_ReadRegister(hmems, L3GD20_STATUS_REG_ADDR, status_reg);
}

/**
 * @brief Set gyroscope power mode
 * @param hmems Pointer to MEMS handle structure
 * @param power_down Power down mode (true = power down, false = normal)
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_SetPowerMode(MEMS_HandleTypeDef *hmems, bool power_down)
{
    MEMS_StatusTypeDef status = MEMS_OK;
    uint8_t ctrl_reg1 = 0;

    if (hmems == NULL) {
        return MEMS_INVALID_PARAM;
    }

    /* Read current CTRL_REG1 */
    status = MEMS_ReadRegister(hmems, L3GD20_CTRL_REG1_ADDR, &ctrl_reg1);
    if (status != MEMS_OK) {
        return status;
    }

    /* Modify power mode bit */
    if (power_down) {
        ctrl_reg1 &= ~L3GD20_NORMAL_MODE;
    } else {
        ctrl_reg1 |= L3GD20_NORMAL_MODE;
    }

    /* Write back the register */
    status = MEMS_WriteRegister(hmems, L3GD20_CTRL_REG1_ADDR, ctrl_reg1);
    if (status != MEMS_OK) {
        return status;
    }

    hmems->GyroConfig.PowerDownMode = power_down;

    return MEMS_OK;
}

/**
 * @brief Reset device to default settings
 * @param hmems Pointer to MEMS handle structure
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_Reset(MEMS_HandleTypeDef *hmems)
{
    MEMS_StatusTypeDef status = MEMS_OK;

    if (hmems == NULL) {
        return MEMS_INVALID_PARAM;
    }

    /* Reset all control registers to default values */
    status = MEMS_WriteRegister(hmems, L3GD20_CTRL_REG1_ADDR, 0x07);
    if (status != MEMS_OK) {
        return status;
    }

    status = MEMS_WriteRegister(hmems, L3GD20_CTRL_REG2_ADDR, 0x00);
    if (status != MEMS_OK) {
        return status;
    }

    status = MEMS_WriteRegister(hmems, L3GD20_CTRL_REG3_ADDR, 0x00);
    if (status != MEMS_OK) {
        return status;
    }

    status = MEMS_WriteRegister(hmems, L3GD20_CTRL_REG4_ADDR, 0x00);
    if (status != MEMS_OK) {
        return status;
    }

    status = MEMS_WriteRegister(hmems, L3GD20_CTRL_REG5_ADDR, 0x00);
    if (status != MEMS_OK) {
        return status;
    }

    /* Reset calibration */
    hmems->IsCalibrated = false;
    memset(&hmems->CalibrationOffset, 0, sizeof(MEMS_AxesTypeDef));

    return MEMS_OK;
}

/**
 * @brief Perform self-test
 * @param hmems Pointer to MEMS handle structure
 * @param result Pointer to store self-test result
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_SelfTest(MEMS_HandleTypeDef *hmems, bool *result)
{
    MEMS_StatusTypeDef status = MEMS_OK;
    MEMS_AxesRawTypeDef data_normal, data_test;
    uint8_t ctrl_reg4;
    int16_t diff_x, diff_y, diff_z;

    if (hmems == NULL || result == NULL) {
        return MEMS_INVALID_PARAM;
    }

    if (!hmems->IsInitialized) {
        return MEMS_NOT_INITIALIZED;
    }

    *result = false;

    /* Read normal data */
    status = MEMS_GyroReadRaw(hmems, &data_normal);
    if (status != MEMS_OK) {
        return status;
    }

    /* Enable self-test */
    status = MEMS_ReadRegister(hmems, L3GD20_CTRL_REG4_ADDR, &ctrl_reg4);
    if (status != MEMS_OK) {
        return status;
    }

    ctrl_reg4 |= 0x02; /* Enable self-test */
    status = MEMS_WriteRegister(hmems, L3GD20_CTRL_REG4_ADDR, ctrl_reg4);
    if (status != MEMS_OK) {
        return status;
    }

    MEMS_DELAY_MS(100); /* Wait for self-test to stabilize */

    /* Read self-test data */
    status = MEMS_GyroReadRaw(hmems, &data_test);
    if (status != MEMS_OK) {
        return status;
    }

    /* Disable self-test */
    ctrl_reg4 &= ~0x02;
    status = MEMS_WriteRegister(hmems, L3GD20_CTRL_REG4_ADDR, ctrl_reg4);
    if (status != MEMS_OK) {
        return status;
    }

    /* Calculate differences */
    diff_x = abs(data_test.X - data_normal.X);
    diff_y = abs(data_test.Y - data_normal.Y);
    diff_z = abs(data_test.Z - data_normal.Z);

    /* Check if differences are within acceptable range */
    if (diff_x > 100 && diff_y > 100 && diff_z > 100 &&
        diff_x < 1000 && diff_y < 1000 && diff_z < 1000) {
        *result = true;
    }

    return MEMS_OK;
}

/**
 * @brief Enable/disable specific axis
 * @param hmems Pointer to MEMS handle structure
 * @param axis_mask Axis mask (bit 0=X, bit 1=Y, bit 2=Z)
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_SetAxisEnable(MEMS_HandleTypeDef *hmems, uint8_t axis_mask)
{
    MEMS_StatusTypeDef status = MEMS_OK;
    uint8_t ctrl_reg1;

    if (hmems == NULL) {
        return MEMS_INVALID_PARAM;
    }

    if (!hmems->IsInitialized) {
        return MEMS_NOT_INITIALIZED;
    }

    /* Read current CTRL_REG1 */
    status = MEMS_ReadRegister(hmems, L3GD20_CTRL_REG1_ADDR, &ctrl_reg1);
    if (status != MEMS_OK) {
        return status;
    }

    /* Clear axis enable bits */
    ctrl_reg1 &= ~0x07;

    /* Set new axis enable bits */
    ctrl_reg1 |= (axis_mask & 0x07);

    /* Write back the register */
    status = MEMS_WriteRegister(hmems, L3GD20_CTRL_REG1_ADDR, ctrl_reg1);
    if (status != MEMS_OK) {
        return status;
    }

    /* Update configuration */
    hmems->GyroConfig.XAxisEnable = (axis_mask & 0x01) ? true : false;
    hmems->GyroConfig.YAxisEnable = (axis_mask & 0x02) ? true : false;
    hmems->GyroConfig.ZAxisEnable = (axis_mask & 0x04) ? true : false;

    return MEMS_OK;
}

/**
 * @brief Read multiple registers
 * @param hmems Pointer to MEMS handle structure
 * @param start_addr Starting register address
 * @param data Pointer to store read data
 * @param length Number of bytes to read
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_ReadRegisters(MEMS_HandleTypeDef *hmems, uint8_t start_addr,
                                     uint8_t *data, uint8_t length)
{
    HAL_StatusTypeDef hal_status;
    uint8_t tx_buffer[1];

    if (hmems == NULL || data == NULL || length == 0) {
        return MEMS_INVALID_PARAM;
    }

    if (hmems->hspi == NULL) {
        return MEMS_NOT_INITIALIZED;
    }

    /* Prepare command byte */
    tx_buffer[0] = start_addr | L3GD20_READ_CMD;
    if (length > 1) {
        tx_buffer[0] |= L3GD20_MULTIPLEBYTE_CMD;
    }

    /* Select device */
    MEMS_CS_Low(hmems);

    /* Send address */
    hal_status = HAL_SPI_Transmit(hmems->hspi, tx_buffer, 1, MEMS_SPI_TIMEOUT);
    if (hal_status != HAL_OK) {
        MEMS_CS_High(hmems);
        return MEMS_COMMUNICATION_ERROR;
    }

    /* Read data */
    hal_status = HAL_SPI_Receive(hmems->hspi, data, length, MEMS_SPI_TIMEOUT);
    if (hal_status != HAL_OK) {
        MEMS_CS_High(hmems);
        return MEMS_COMMUNICATION_ERROR;
    }

    /* Deselect device */
    MEMS_CS_High(hmems);

    return MEMS_OK;
}

/**
 * @brief Write multiple registers
 * @param hmems Pointer to MEMS handle structure
 * @param start_addr Starting register address
 * @param data Pointer to data to write
 * @param length Number of bytes to write
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_WriteRegisters(MEMS_HandleTypeDef *hmems, uint8_t start_addr,
                                      uint8_t *data, uint8_t length)
{
    HAL_StatusTypeDef hal_status;
    uint8_t tx_buffer[1];

    if (hmems == NULL || data == NULL || length == 0) {
        return MEMS_INVALID_PARAM;
    }

    if (hmems->hspi == NULL) {
        return MEMS_NOT_INITIALIZED;
    }

    /* Prepare command byte */
    tx_buffer[0] = start_addr;
    if (length > 1) {
        tx_buffer[0] |= L3GD20_MULTIPLEBYTE_CMD;
    }

    /* Select device */
    MEMS_CS_Low(hmems);

    /* Send address */
    hal_status = HAL_SPI_Transmit(hmems->hspi, tx_buffer, 1, MEMS_SPI_TIMEOUT);
    if (hal_status != HAL_OK) {
        MEMS_CS_High(hmems);
        return MEMS_COMMUNICATION_ERROR;
    }

    /* Write data */
    hal_status = HAL_SPI_Transmit(hmems->hspi, data, length, MEMS_SPI_TIMEOUT);
    if (hal_status != HAL_OK) {
        MEMS_CS_High(hmems);
        return MEMS_COMMUNICATION_ERROR;
    }

    /* Deselect device */
    MEMS_CS_High(hmems);

    return MEMS_OK;
}

/**
 * @brief Get current full scale setting
 * @param hmems Pointer to MEMS handle structure
 * @param full_scale Pointer to store current full scale
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_GetFullScale(MEMS_HandleTypeDef *hmems, MEMS_GyroFullScaleTypeDef *full_scale)
{
    if (hmems == NULL || full_scale == NULL) {
        return MEMS_INVALID_PARAM;
    }

    if (!hmems->IsInitialized) {
        return MEMS_NOT_INITIALIZED;
    }

    *full_scale = hmems->GyroConfig.FullScale;

    return MEMS_OK;
}

/**
 * @brief Convert raw data to engineering units
 * @param raw_data Raw sensor data
 * @param full_scale Current full scale setting
 * @retval float Converted value in dps
 */
float MEMS_ConvertToDPS(int16_t raw_data, MEMS_GyroFullScaleTypeDef full_scale)
{
    float sensitivity;

    /* Get sensitivity based on full scale */
    switch (full_scale) {
        case MEMS_GYRO_FULLSCALE_250:
            sensitivity = MEMS_SENSITIVITY_FACTORS[0];
            break;
        case MEMS_GYRO_FULLSCALE_500:
            sensitivity = MEMS_SENSITIVITY_FACTORS[1];
            break;
        case MEMS_GYRO_FULLSCALE_2000:
            sensitivity = MEMS_SENSITIVITY_FACTORS[2];
            break;
        default:
            sensitivity = MEMS_SENSITIVITY_FACTORS[0];
            break;
    }

    /* Convert from millidegrees to degrees per second */
    return ((float)raw_data * sensitivity) / 1000.0f;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Write single register
 * @param hmems Pointer to MEMS handle structure
 * @param addr Register address
 * @param data Data to write
 * @retval MEMS_StatusTypeDef Status of the operation
 */
static MEMS_StatusTypeDef MEMS_WriteRegister(MEMS_HandleTypeDef *hmems, uint8_t addr, uint8_t data)
{
    return MEMS_WriteRegisters(hmems, addr, &data, 1);
}

/**
 * @brief Read single register
 * @param hmems Pointer to MEMS handle structure
 * @param addr Register address
 * @param data Pointer to store read data
 * @retval MEMS_StatusTypeDef Status of the operation
 */
static MEMS_StatusTypeDef MEMS_ReadRegister(MEMS_HandleTypeDef *hmems, uint8_t addr, uint8_t *data)
{
    return MEMS_ReadRegisters(hmems, addr, data, 1);
}

/**
 * @brief Set CS pin high (deselect device)
 */
static void MEMS_CS_High(MEMS_HandleTypeDef *hmems)
{
    if (hmems != NULL && hmems->CS_Port != NULL && hmems->CS_Pin != 0U) {
        HAL_GPIO_WritePin(hmems->CS_Port, hmems->CS_Pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(MEMS_CS_GPIO_PORT, MEMS_CS_PIN, GPIO_PIN_SET);
    }
}

/**
 * @brief Set CS pin low (select device)
 */
static void MEMS_CS_Low(MEMS_HandleTypeDef *hmems)
{
    if (hmems != NULL && hmems->CS_Port != NULL && hmems->CS_Pin != 0U) {
        HAL_GPIO_WritePin(hmems->CS_Port, hmems->CS_Pin, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(MEMS_CS_GPIO_PORT, MEMS_CS_PIN, GPIO_PIN_RESET);
    }
}

/**
 * @brief Initialize GPIO pins for MEMS sensor
 * @retval MEMS_StatusTypeDef Status of the operation
 */
static MEMS_StatusTypeDef MEMS_InitGPIO(MEMS_HandleTypeDef *hmems)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable common GPIO clocks used by default pins */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();

    /* Configure CS pin: prefer user-specified in handle if provided */
    if (hmems != NULL && hmems->CS_Port != NULL && hmems->CS_Pin != 0U) {
        /* Try to enable clock for provided port (best-effort) */
        if (hmems->CS_Port == GPIOA) { __HAL_RCC_GPIOA_CLK_ENABLE(); }
        else if (hmems->CS_Port == GPIOB) { __HAL_RCC_GPIOB_CLK_ENABLE(); }
        else if (hmems->CS_Port == GPIOC) { __HAL_RCC_GPIOC_CLK_ENABLE(); }
        else if (hmems->CS_Port == GPIOD) { __HAL_RCC_GPIOD_CLK_ENABLE(); }
        else if (hmems->CS_Port == GPIOE) { __HAL_RCC_GPIOE_CLK_ENABLE(); }
        else if (hmems->CS_Port == GPIOF) { __HAL_RCC_GPIOF_CLK_ENABLE(); }
        else if (hmems->CS_Port == GPIOG) { __HAL_RCC_GPIOG_CLK_ENABLE(); }

        GPIO_InitStruct.Pin = hmems->CS_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(hmems->CS_Port, &GPIO_InitStruct);
    } else {
        /* Configure default CS pin */
        GPIO_InitStruct.Pin = MEMS_CS_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(MEMS_CS_GPIO_PORT, &GPIO_InitStruct);
    }

    /* Configure interrupt pins */
    GPIO_InitStruct.Pin = MEMS_INT1_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(MEMS_INT1_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = MEMS_INT2_PIN;
    HAL_GPIO_Init(MEMS_INT2_GPIO_PORT, &GPIO_InitStruct);

    /* Configure SPI pins */

    return MEMS_OK;
}

/**
 * @brief Configure chip-select pin for a MEMS handle
 * @param hmems Pointer to MEMS handle structure
 * @param csPort GPIO port for CS
 * @param csPin GPIO pin number for CS
 */
void MEMS_SetCS(MEMS_HandleTypeDef *hmems, GPIO_TypeDef *csPort, uint16_t csPin)
{
    if (hmems == NULL) {
        return;
    }

    hmems->CS_Port = csPort;
    hmems->CS_Pin = csPin;

    if (csPort == NULL || csPin == 0U) {
        return;
    }

    /* Try to enable clock for provided port (best-effort) */
    if (csPort == GPIOA) { __HAL_RCC_GPIOA_CLK_ENABLE(); }
    else if (csPort == GPIOB) { __HAL_RCC_GPIOB_CLK_ENABLE(); }
    else if (csPort == GPIOC) { __HAL_RCC_GPIOC_CLK_ENABLE(); }
    else if (csPort == GPIOD) { __HAL_RCC_GPIOD_CLK_ENABLE(); }
    else if (csPort == GPIOE) { __HAL_RCC_GPIOE_CLK_ENABLE(); }
    else if (csPort == GPIOF) { __HAL_RCC_GPIOF_CLK_ENABLE(); }
    else if (csPort == GPIOG) { __HAL_RCC_GPIOG_CLK_ENABLE(); }

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = csPin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(csPort, &GPIO_InitStruct);

    /* Deassert CS */
    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_SET);

    GPIO_InitStruct.Pin = MEMS_SPI_SCK_PIN | MEMS_SPI_MISO_PIN | MEMS_SPI_MOSI_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI5;
    HAL_GPIO_Init(MEMS_SPI_SCK_GPIO_PORT, &GPIO_InitStruct);

    /* MEMS_SetCS is a void helper, no return value */
    (void)0;
}

/* Note: SPI initialization is intentionally omitted — the application must initialize SPI (e.g., call `SPI_Init()` in Peripherals/SPI or `SPI_Init_Custom()`).
   This driver will use the provided `SPI_HandleTypeDef *hspi` and will not reconfigure or initialize the peripheral. */

/**
 * @brief Verify device presence by reading WHO_AM_I register
 * @param hmems Pointer to MEMS handle structure
 * @retval MEMS_StatusTypeDef Status of the operation
 */
static MEMS_StatusTypeDef MEMS_VerifyDevice(MEMS_HandleTypeDef *hmems)
{
    MEMS_StatusTypeDef status = MEMS_OK;
    uint8_t who_am_i = 0;
    uint8_t retry_count = 0;

    /* Try multiple times to read WHO_AM_I */
    while (retry_count < MEMS_MAX_RETRIES) {
        status = MEMS_ReadRegister(hmems, L3GD20_WHO_AM_I_ADDR, &who_am_i);

        if (status == MEMS_OK && who_am_i == L3GD20_WHO_AM_I_VALUE) {
            return MEMS_OK;
        }

        retry_count++;
        MEMS_DELAY_MS(10);
    }

    return MEMS_DEVICE_NOT_FOUND;
}
