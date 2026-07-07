/**
  ******************************************************************************
  * @file    accel.c
  * @brief   MMA8452Q Accelerometer driver implementation
  * @details This file provides the implementation for the MMA8452Q
  *          3-axis digital accelerometer driver.
  * @version 1.0
  * @date    2025-09-01
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "accel.h"
#include "accel_constants.h"
#include "spi.h"
#include <string.h>

/* Private defines -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
#define ACCEL_SPI_TIMEOUT        100U    /**< SPI timeout for accelerometer operations */
#define ACCEL_CALIBRATION_SAMPLES 100U   /**< Number of samples for calibration */

/* Constants for register values and calculations */
#define ACCEL_HP_FILTER_ENABLE   0x10U   /**< High-pass filter enable value */
#define ACCEL_SENSITIVITY_2G     4096.0f /**< Sensitivity for 2g range (2^14 / 2) */
#define ACCEL_SENSITIVITY_4G     2048.0f /**< Sensitivity for 4g range (2^14 / 4) */
#define ACCEL_SENSITIVITY_8G     1024.0f /**< Sensitivity for 8g range (2^14 / 8) */
#define ACCEL_RANGE_MASK         0x38U   /**< Mask for range bits 3-5 in CTRL_REG1 */
#define ACCEL_SOFT_RESET_VALUE   0x80U   /**< Soft reset register value */
#define ACCEL_CALIBRATION_DELAY  10U     /**< Delay between calibration samples (ms) */
#define ACCEL_RESET_DELAY        50U     /**< Delay after reset (ms) */
#define ACCEL_STABILITY_THRESHOLD 0.5f   /**< Stability threshold for self-test */
#define ACCEL_GRAVITY_1G_4G_RANGE 8192   /**< 1g value in 4g range for calibration */
// ...existing code...

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static uint8_t currentRange = ACCEL_RANGE_2G;  /**< Current measurement range */
static uint8_t currentMode = ACCEL_MODE_STANDBY; /**< Current operating mode */
// ...existing code...


/* Private function prototypes -----------------------------------------------*/
static ACCEL_StatusTypeDef ACCEL_WriteRegister(uint8_t reg, uint8_t value);
static ACCEL_StatusTypeDef ACCEL_ReadRegister(uint8_t reg, uint8_t* value);
static ACCEL_StatusTypeDef ACCEL_ReadRegisters(uint8_t reg, uint8_t* buffer, uint16_t length);
static int16_t ACCEL_ConvertRawToSigned(uint8_t msb, uint8_t lsb);

/**
 * @brief   Initialize the MMA8452Q accelerometer
 * @details Configures the accelerometer with default settings
 * @param   None
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_Init(void)
{
    ACCEL_ConfigTypeDef defaultConfig = {
        .DataRate = ACCEL_ODR_100HZ,
        .Range = ACCEL_RANGE_2G,
        .Mode = ACCEL_MODE_ACTIVE,
        .HighPassFilter = false,
        .LowNoise = false
    };

    return ACCEL_Init_Custom(&defaultConfig);
}

/**
 * @brief   Initialize accelerometer with custom configuration
 * @details Allows custom configuration of accelerometer parameters
 * @param   config Pointer to accelerometer configuration structure
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_Init_Custom(const ACCEL_ConfigTypeDef* config)
{
    ACCEL_StatusTypeDef status = ACCEL_OK;

    if (config == NULL)
    {
        return ACCEL_INVALID_PARAM;
    }

    /* Check if device is ready */
    status = ACCEL_IsReady();
    if (status != ACCEL_OK)
    {
        return status;
    }

    /* Put device in standby mode for configuration */
    status = ACCEL_WriteRegister(ACCEL_REG_CTRL_REG1, 0x00);
    if (status != ACCEL_OK)
    {
        return status;
    }

    /* Configure measurement range */
    status = ACCEL_WriteRegister(ACCEL_REG_XYZ_DATA_CFG, config->Range);
    if (status != ACCEL_OK)
    {
        return status;
    }
    currentRange = config->Range;

    /* Configure data rate and other settings */
    uint8_t ctrlReg1 = (config->DataRate << 3) | (config->LowNoise ? 0x04 : 0x00);
    status = ACCEL_WriteRegister(ACCEL_REG_CTRL_REG1, ctrlReg1);
    if (status != ACCEL_OK)
    {
        return status;
    }

    /* Configure high-pass filter if enabled */
    if (config->HighPassFilter)
    {
        status = ACCEL_WriteRegister(ACCEL_REG_HP_FILTER_CUTOFF, ACCEL_HP_FILTER_ENABLE);
        if (status != ACCEL_OK)
        {
            return status;
        }
    }

    /* Set operating mode */
    status = ACCEL_SetMode(config->Mode);
    if (status != ACCEL_OK)
    {
        return status;
    }

    return ACCEL_OK;
}

/**
 * @brief   Deinitialize the accelerometer
 * @details Puts the accelerometer in standby mode
 * @param   None
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_DeInit(void)
{
    return ACCEL_SetMode(ACCEL_MODE_STANDBY);
}

/**
 * @brief   Read acceleration data from all axes
 * @details Reads raw and converted acceleration data
 * @param   data Pointer to data structure to store results
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_ReadData(ACCEL_DataTypeDef* data)
{
    if (data == NULL)
    {
        return ACCEL_INVALID_PARAM;
    }

    ACCEL_StatusTypeDef status = ACCEL_OK;
    int16_t xRaw = 0;
    int16_t yRaw = 0;
    int16_t zRaw = 0;

    /* Read raw data */
    status = ACCEL_ReadRawData(&xRaw, &yRaw, &zRaw);
    if (status != ACCEL_OK)
    {
        return status;
    }

    /* Store raw data */
    data->X = xRaw;
    data->Y = yRaw;
    data->Z = zRaw;

    /* Convert to g-force */
    data->X_g = ACCEL_ConvertToG(xRaw, currentRange);
    data->Y_g = ACCEL_ConvertToG(yRaw, currentRange);
    data->Z_g = ACCEL_ConvertToG(zRaw, currentRange);

    return ACCEL_OK;
}

/**
 * @brief   Read raw acceleration data
 * @details Reads 14-bit raw data from accelerometer
 * @param   xAxis Pointer to store X-axis data
 * @param   yAxis Pointer to store Y-axis data
 * @param   zAxis Pointer to store Z-axis data
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_ReadRawData(int16_t* xAxis, int16_t* yAxis, int16_t* zAxis)
{
    if (xAxis == NULL || yAxis == NULL || zAxis == NULL)
    {
        return ACCEL_INVALID_PARAM;
    }

    uint8_t buffer[6];
    ACCEL_StatusTypeDef status = ACCEL_OK;

    /* Read 6 bytes starting from OUT_X_MSB register */
    status = ACCEL_ReadRegisters(ACCEL_REG_OUT_X_MSB, buffer, 6);
    if (status != ACCEL_OK)
    {
        return status;
    }

    /* Convert to 14-bit signed values */
    *xAxis = ACCEL_ConvertRawToSigned(buffer[0], buffer[1]);
    *yAxis = ACCEL_ConvertRawToSigned(buffer[2], buffer[3]);
    *zAxis = ACCEL_ConvertRawToSigned(buffer[4], buffer[5]);

    return ACCEL_OK;
}

/**
 * @brief   Convert raw data to g-force
 * @details Converts 14-bit raw data to acceleration in g
 * @param   raw Raw 14-bit acceleration data
 * @param   range Measurement range (2, 4, or 8g)
 * @retval  float Acceleration in g
 */
float ACCEL_ConvertToG(int16_t raw, uint8_t range)
{
    float sensitivity = ACCEL_SENSITIVITY_2G; /* Default to 2g */

    /* Determine sensitivity based on range */
    switch (range)
    {
        case ACCEL_RANGE_2G:
            sensitivity = ACCEL_SENSITIVITY_2G;  /* 2^14 / 2 */
            break;
        case ACCEL_RANGE_4G:
            sensitivity = ACCEL_SENSITIVITY_4G;  /* 2^14 / 4 */
            break;
        case ACCEL_RANGE_8G:
            sensitivity = ACCEL_SENSITIVITY_8G;  /* 2^14 / 8 */
            break;
        default:
            sensitivity = ACCEL_SENSITIVITY_2G;  /* Default to 2g */
            break;
    }

    return (float)raw / sensitivity;
}

/**
 * @brief   Check if accelerometer is ready
 * @details Verifies device communication and ID
 * @param   None
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_IsReady(void)
{
    uint8_t deviceId = 0;

    ACCEL_StatusTypeDef status = ACCEL_GetDeviceID(&deviceId);
    if (status != ACCEL_OK)
    {
        return status;
    }

    if (deviceId != ACCEL_DEVICE_ID)
    {
        return ACCEL_NOT_READY;
    }

    return ACCEL_OK;
}

/**
 * @brief   Get device ID
 * @details Reads the WHO_AM_I register
 * @param   deviceId Pointer to store device ID
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_GetDeviceID(uint8_t* deviceId)
{
    if (deviceId == NULL)
    {
        return ACCEL_INVALID_PARAM;
    }

    return ACCEL_ReadRegister(ACCEL_REG_WHO_AM_I, deviceId);
}

/**
 * @brief   Set operating mode
 * @details Changes between standby, active, and sleep modes
 * @param   mode Operating mode
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_SetMode(uint8_t mode)
{
    uint8_t ctrlReg1 = 0;
    ACCEL_StatusTypeDef status = ACCEL_OK;

    /* Read current CTRL_REG1 value */
    status = ACCEL_ReadRegister(ACCEL_REG_CTRL_REG1, &ctrlReg1);
    if (status != ACCEL_OK)
    {
        return status;
    }

    /* Modify mode bits */
    switch (mode)
    {
        case ACCEL_MODE_STANDBY:
            ctrlReg1 &= ~0x01;  /* Clear active bit */
            break;
        case ACCEL_MODE_ACTIVE:
            ctrlReg1 |= 0x01;   /* Set active bit */
            break;
        case ACCEL_MODE_SLEEP:
            /* Sleep mode requires additional configuration */
            ctrlReg1 |= 0x02;   /* Set sleep mode bit */
            break;
        default:
            return ACCEL_INVALID_PARAM;
    }

    /* Write back the modified register */
    status = ACCEL_WriteRegister(ACCEL_REG_CTRL_REG1, ctrlReg1);
    if (status == ACCEL_OK)
    {
        currentMode = mode;
    }

    return status;
}

/**
 * @brief   Get current operating mode
 * @details Reads the current system mode
 * @param   mode Pointer to store current mode
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_GetMode(uint8_t* mode)
{
    if (mode == NULL)
    {
        return ACCEL_INVALID_PARAM;
    }

    uint8_t sysmod = 0;
    ACCEL_StatusTypeDef status = ACCEL_ReadRegister(ACCEL_REG_SYSMOD, &sysmod);

    if (status == ACCEL_OK)
    {
        *mode = sysmod & 0x03;  /* Extract mode bits */
    }

    return status;
}

/**
 * @brief   Set output data rate
 * @details Configures the accelerometer sampling rate
 * @param   odr Output data rate
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_SetDataRate(uint8_t odr)
{
    if (odr > ACCEL_ODR_1_56HZ)
    {
        return ACCEL_INVALID_PARAM;
    }

    uint8_t ctrlReg1 = 0;
    ACCEL_StatusTypeDef status = ACCEL_OK;

    /* Read current CTRL_REG1 value */
    status = ACCEL_ReadRegister(ACCEL_REG_CTRL_REG1, &ctrlReg1);
    if (status != ACCEL_OK)
    {
        return status;
    }

    /* Clear data rate bits and set new value */
    ctrlReg1 &= ~ACCEL_RANGE_MASK;  /* Clear bits 3-5 */
    ctrlReg1 |= (odr << 3);

    return ACCEL_WriteRegister(ACCEL_REG_CTRL_REG1, ctrlReg1);
}

/**
 * @brief   Set measurement range
 * @details Configures the accelerometer measurement range
 * @param   range Measurement range (±2g, ±4g, ±8g)
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_SetRange(uint8_t range)
{
    if (range > ACCEL_RANGE_8G)
    {
        return ACCEL_INVALID_PARAM;
    }

    ACCEL_StatusTypeDef status = ACCEL_WriteRegister(ACCEL_REG_XYZ_DATA_CFG, range);
    if (status == ACCEL_OK)
    {
        currentRange = range;
    }

    return status;
}

/**
 * @brief   Get current measurement range
 * @details Reads the current range setting
 * @param   range Pointer to store current range
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_GetRange(uint8_t* range)
{
    if (range == NULL)
    {
        return ACCEL_INVALID_PARAM;
    }

    return ACCEL_ReadRegister(ACCEL_REG_XYZ_DATA_CFG, range);
}

/**
 * @brief   Enable/disable high-pass filter
 * @details Controls the high-pass filter for DC offset removal
 * @param   enable Enable (true) or disable (false)
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_EnableHighPassFilter(bool enable)
{
    uint8_t hpFilterCutoff = 0;

    if (enable)
    {
        hpFilterCutoff = ACCEL_HP_FILTER_ENABLE;  /* Enable high-pass filter */
    }
    else
    {
        hpFilterCutoff = 0x00;  /* Disable high-pass filter */
    }

    return ACCEL_WriteRegister(ACCEL_REG_HP_FILTER_CUTOFF, hpFilterCutoff);
}

/**
 * @brief   Enable/disable low noise mode
 * @details Controls low noise mode for improved accuracy
 * @param   enable Enable (true) or disable (false)
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_EnableLowNoise(bool enable)
{
    uint8_t ctrlReg1 = 0;
    ACCEL_StatusTypeDef status = ACCEL_OK;

    /* Read current CTRL_REG1 value */
    status = ACCEL_ReadRegister(ACCEL_REG_CTRL_REG1, &ctrlReg1);
    if (status != ACCEL_OK)
    {
        return status;
    }

    if (enable)
    {
        ctrlReg1 |= 0x04;   /* Set low noise bit */
    }
    else
    {
        ctrlReg1 &= ~0x04;  /* Clear low noise bit */
    }

    return ACCEL_WriteRegister(ACCEL_REG_CTRL_REG1, ctrlReg1);
}

/**
 * @brief   Configure interrupts
 * @details Sets up interrupt sources and routing
 * @param   config Pointer to interrupt configuration
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_ConfigInterrupts(const ACCEL_IntConfigTypeDef* config)
{
    if (config == NULL)
    {
        return ACCEL_INVALID_PARAM;
    }

    uint8_t ctrlReg4 = 0x00;
    uint8_t ctrlReg5 = 0x00;

    /* Configure interrupt sources (CTRL_REG4) */
    if (config->DataReady) {
        ctrlReg4 |= 0x01;
    }
    if (config->Motion) {
        ctrlReg4 |= 0x04;
    }
    if (config->Freefall) {
        ctrlReg4 |= 0x04;  /* Same bit as motion */
    }
    if (config->Tap) {
        ctrlReg4 |= 0x08;
    }

    /* Configure interrupt routing (CTRL_REG5) - route to INT1 pin */
    if (config->DataReady) {
        ctrlReg5 |= 0x01;
    }
    if (config->Motion) {
        ctrlReg5 |= 0x04;
    }
    if (config->Freefall) {
        ctrlReg5 |= 0x04;
    }
    if (config->Tap) {
        ctrlReg5 |= 0x08;
    }

    /* Write interrupt configuration registers */
    ACCEL_StatusTypeDef status = ACCEL_WriteRegister(ACCEL_REG_CTRL_REG4, ctrlReg4);
    if (status != ACCEL_OK)
    {
        return status;
    }

    return ACCEL_WriteRegister(ACCEL_REG_CTRL_REG5, ctrlReg5);
}

/**
 * @brief   Get interrupt source
 * @details Reads which interrupt sources are active
 * @param   intSource Pointer to store interrupt source flags
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_GetInterruptSource(uint8_t* intSource)
{
    if (intSource == NULL)
    {
        return ACCEL_INVALID_PARAM;
    }

    return ACCEL_ReadRegister(ACCEL_REG_INT_SOURCE, intSource);
}

/**
 * @brief   Calibrate accelerometer
 * @details Performs offset calibration for all axes
 * @param   None
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_Calibrate(void)
{
    int32_t xSum = 0;
    int32_t ySum = 0;
    int32_t zSum = 0;
    int16_t xRaw = 0;
    int16_t yRaw = 0;
    int16_t zRaw = 0;
    ACCEL_StatusTypeDef status = ACCEL_OK;

    /* Collect calibration samples */
    for (uint16_t i = 0; i < ACCEL_CALIBRATION_SAMPLES; i++)
    {
        status = ACCEL_ReadRawData(&xRaw, &yRaw, &zRaw);
        if (status != ACCEL_OK)
        {
            return status;
        }

        xSum += xRaw;
        ySum += yRaw;
        zSum += zRaw;

        /* Small delay between samples */
        HAL_Delay(ACCEL_CALIBRATION_DELAY);
    }

    /* Calculate average offsets */
    int8_t xOffset = (int8_t)(-(xSum / ACCEL_CALIBRATION_SAMPLES) / 8);
    int8_t yOffset = (int8_t)(-(ySum / ACCEL_CALIBRATION_SAMPLES) / 8);
    int8_t zOffset = (int8_t)((ACCEL_GRAVITY_1G_4G_RANGE - (zSum / ACCEL_CALIBRATION_SAMPLES)) / 8);  /* 1g = 8192 for 4g range */

    /* Apply calculated offsets */
    return ACCEL_SetOffset(xOffset, yOffset, zOffset);
}

/**
 * @brief   Set manual offset
 * @details Sets manual offset values for calibration
 * @param   xOffset X-axis offset
 * @param   yOffset Y-axis offset
 * @param   zOffset Z-axis offset
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_SetOffset(int8_t xOffset, int8_t yOffset, int8_t zOffset)
{
    ACCEL_StatusTypeDef status = ACCEL_OK;

    status = ACCEL_WriteRegister(ACCEL_REG_OFF_X, (uint8_t)xOffset);
    if (status != ACCEL_OK)
    {
        return status;
    }

    status = ACCEL_WriteRegister(ACCEL_REG_OFF_Y, (uint8_t)yOffset);
    if (status != ACCEL_OK)
    {
        return status;
    }

    return ACCEL_WriteRegister(ACCEL_REG_OFF_Z, (uint8_t)zOffset);
}

/**
 * @brief   Get current offset values
 * @details Reads current offset calibration values
 * @param   xOffset Pointer to store X-axis offset
 * @param   yOffset Pointer to store Y-axis offset
 * @param   zOffset Pointer to store Z-axis offset
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_GetOffset(int8_t* xOffset, int8_t* yOffset, int8_t* zOffset)
{
    if (xOffset == NULL || yOffset == NULL || zOffset == NULL)
    {
        return ACCEL_INVALID_PARAM;
    }

    uint8_t offset = 0;
    ACCEL_StatusTypeDef status = ACCEL_OK;

    status = ACCEL_ReadRegister(ACCEL_REG_OFF_X, &offset);
    if (status != ACCEL_OK)
    {
        return status;
    }
    *xOffset = (int8_t)offset;

    status = ACCEL_ReadRegister(ACCEL_REG_OFF_Y, &offset);
    if (status != ACCEL_OK)
    {
        return status;
    }
    *yOffset = (int8_t)offset;

    status = ACCEL_ReadRegister(ACCEL_REG_OFF_Z, &offset);
    if (status != ACCEL_OK)
    {
        return status;
    }
    *zOffset = (int8_t)offset;

    return ACCEL_OK;
}

/**
 * @brief   Perform self-test
 * @details Runs built-in self-test function
 * @param   None
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_SelfTest(void)
{
    /* MMA8452Q self-test requires specific sequence */
    /* This is a simplified implementation */

    ACCEL_DataTypeDef dataBefore;
    ACCEL_DataTypeDef dataAfter;
    ACCEL_StatusTypeDef status = ACCEL_OK;

    /* Read data before self-test */
    status = ACCEL_ReadData(&dataBefore);
    if (status != ACCEL_OK)
    {
        return status;
    }

    /* Enable self-test (bit 7 in CTRL_REG2) */
    status = ACCEL_WriteRegister(ACCEL_REG_CTRL_REG2, ACCEL_CTRL_REG2_SOFT_RESET);
    if (status != ACCEL_OK)
    {
        return status;
    }

    /* Wait for self-test to complete */
    HAL_Delay(ACCEL_RESET_DELAY_MS);

    /* Read data during self-test */
    status = ACCEL_ReadData(&dataAfter);
    if (status != ACCEL_OK)
    {
        return status;
    }

    /* Disable self-test */
    status = ACCEL_WriteRegister(ACCEL_REG_CTRL_REG2, 0x00);
    if (status != ACCEL_OK)
    {
        return status;
    }

    /* Check if self-test response is within expected range */
    /* Typical self-test should show significant change in output */
    float deltaX = dataAfter.X_g - dataBefore.X_g;
    float deltaY = dataAfter.Y_g - dataBefore.Y_g;
    float deltaZ = dataAfter.Z_g - dataBefore.Z_g;

    /* Check if changes are significant (self-test should cause ~1g change) */
    if ((deltaX < ACCEL_STABILITY_THRESHOLD) && (deltaY < ACCEL_STABILITY_THRESHOLD) && (deltaZ < ACCEL_STABILITY_THRESHOLD))
    {
        return ACCEL_ERROR;  /* Self-test failed */
    }

    return ACCEL_OK;
}

/**
 * @brief   Get accelerometer status string
 * @details Converts status code to human-readable string
 * @param   status Accelerometer status code
 * @retval  const char* Status description string
 */
const char* ACCEL_GetStatusString(ACCEL_StatusTypeDef status)
{
    switch (status)
    {
        case ACCEL_OK:
            return "ACCEL_OK";
        case ACCEL_ERROR:
            return "ACCEL_ERROR";
        case ACCEL_BUSY:
            return "ACCEL_BUSY";
        case ACCEL_TIMEOUT:
            return "ACCEL_TIMEOUT";
        case ACCEL_INVALID_PARAM:
            return "ACCEL_INVALID_PARAM";
        case ACCEL_NOT_READY:
            return "ACCEL_NOT_READY";
        default:
            return "UNKNOWN_STATUS";
    }
}

/**
 * @brief   Write to accelerometer register
 * @details Low-level SPI write operation
 * @param   reg Register address
 * @param   value Value to write
 * @retval  ACCEL_StatusTypeDef Operation status
 */
static ACCEL_StatusTypeDef ACCEL_WriteRegister(uint8_t reg, uint8_t value)
{
    uint8_t txBuffer[2];

    /* Prepare write command */
    txBuffer[0] = ACCEL_SPI_WRITE_CMD | reg;
    txBuffer[1] = value;

    /* Send write command via SPI */
    if (SPI_Transmit(txBuffer, 2, ACCEL_SPI_TIMEOUT) != SPI_OK)
    {
        return ACCEL_ERROR;
    }

    return ACCEL_OK;
}

/**
 * @brief   Read from accelerometer register
 * @details Low-level SPI read operation
 * @param   reg Register address
 * @param   value Pointer to store read value
 * @retval  ACCEL_StatusTypeDef Operation status
 */
static ACCEL_StatusTypeDef ACCEL_ReadRegister(uint8_t reg, uint8_t* value)
{
    if (value == NULL)
    {
        return ACCEL_INVALID_PARAM;
    }

    uint8_t txBuffer[2];
    uint8_t rxBuffer[2];

    /* Prepare read command */
    txBuffer[0] = ACCEL_SPI_READ_CMD | reg;
    txBuffer[1] = 0x00;  /* Dummy byte for read */

    /* Send read command and receive response */
    if (SPI_TransmitReceive(txBuffer, rxBuffer, 2, ACCEL_SPI_TIMEOUT) != SPI_OK)
    {
        return ACCEL_ERROR;
    }

    *value = rxBuffer[1];
    return ACCEL_OK;
}

/**
 * @brief   Read multiple registers
 * @details Reads consecutive registers via SPI
 * @param   reg Starting register address
 * @param   buffer Buffer to store read data
 * @param   length Number of bytes to read
 * @retval  ACCEL_StatusTypeDef Operation status
 */
static ACCEL_StatusTypeDef ACCEL_ReadRegisters(uint8_t reg, uint8_t* buffer, uint16_t length)
{
    if (buffer == NULL || length == 0)
    {
        return ACCEL_INVALID_PARAM;
    }

    uint8_t txBuffer[length + 1];
    uint8_t rxBuffer[length + 1];

    /* Prepare read command */
    txBuffer[0] = ACCEL_SPI_READ_CMD | reg;
    memset(&txBuffer[1], 0x00, length);  /* Fill with dummy bytes */

    /* Send read command and receive response */
    if (SPI_TransmitReceive(txBuffer, rxBuffer, length + 1, ACCEL_SPI_TIMEOUT) != SPI_OK)
    {
        return ACCEL_ERROR;
    }

    /* Copy received data (skip command byte) */
    memcpy(buffer, &rxBuffer[1], length);
    return ACCEL_OK;
}

/**
 * @brief   Convert raw bytes to signed 14-bit value
 * @details Converts MSB and LSB bytes to signed 14-bit integer
 * @param   msb Most significant byte
 * @param   lsb Least significant byte
 * @retval  int16_t Signed 14-bit value
 */
static int16_t ACCEL_ConvertRawToSigned(uint8_t msb, uint8_t lsb)
{
    /* Combine MSB and LSB, then convert to signed 14-bit value */
    int16_t raw = (int16_t)(((uint16_t)msb << 8) | (uint16_t)lsb);

    /* Shift right by 2 to get 14-bit value, then sign extend */
    raw >>= 2;

    /* Sign extend from 14 bits to 16 bits */
    if (raw & ACCEL_SIGN_BIT_14)  /* Check if bit 13 is set (sign bit for 14-bit) */
    {
    raw |= (int16_t)ACCEL_SIGN_EXTEND_14;  /* Sign extend */
    }

    return raw;
}
