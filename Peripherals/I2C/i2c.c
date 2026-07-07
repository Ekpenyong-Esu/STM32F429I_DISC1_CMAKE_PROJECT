/**
  ******************************************************************************
  * @file    i2c.c
  * @brief   I2C module implementation
  * @details This file provides code for the configuration
  *          and initialization of the I2C peripheral.
  * @version 1.0
  * @date    2025-04-15
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "i2c.h"
#include "log.h"
#include "main.h"

/* Private defines -----------------------------------------------------------*/

/**
 * @brief I2C clock speed constants
 */
#define I2C_CLOCK_SPEED_STANDARD     100000U     /**< Standard mode: 100 kHz */
#define I2C_CLOCK_SPEED_FAST         400000U     /**< Fast mode: 400 kHz */

/* Private variables ---------------------------------------------------------*/
/**
 * @brief   I2C3 handle structure
 * @details Used by HAL functions for I2C3 peripheral operations
 */
I2C_HandleTypeDef hi2c3;

/* Private function prototypes -----------------------------------------------*/
static I2C_StatusTypeDef I2C_ConvertHALStatus(HAL_StatusTypeDef halStatus);
static void I2Cx_Error(void);
static void I2C_BusRecovery(void);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Convert HAL status to I2C status
 * @param   halStatus HAL status code
 * @retval  I2C_StatusTypeDef Converted status
 */
static I2C_StatusTypeDef I2C_ConvertHALStatus(HAL_StatusTypeDef halStatus)
{
    switch (halStatus)
    {
        case HAL_OK:
            return I2C_OK;
        case HAL_ERROR:
            return I2C_ERROR;
        case HAL_BUSY:
            return I2C_BUSY;
        case HAL_TIMEOUT:
            return I2C_TIMEOUT;
        default:
            return I2C_ERROR;
    }
}

/* Public functions ----------------------------------------------------------*/

/**
  * @brief  I2C Initialization Function
  * @details Configures the I2C3 peripheral with the following settings:
  *          - Clock speed: 100 kHz (standard mode)
  *          - Duty cycle: 50% (2:1 ratio)
  *          - 7-bit addressing mode
  *          - Own address: 0x00 (acts as master only)
  *          - Dual addressing mode: Disabled
  *          - General call mode: Disabled
  *          - Clock stretching: Enabled (NOSTRETCH disabled)
  *
  * @note   I2C3 is commonly used for communication with sensors,
  *         audio codec, or EEPROM on the STM32F429 board
  * @param  None
  * @retval None
  */
void I2C_Init(void)
{
    log_debug("I2C: Initializing I2C3");
  hi2c3.Instance = I2C3;                               /* Select I2C3 peripheral */
  hi2c3.Init.ClockSpeed = I2C_CLOCK_SPEED_STANDARD;    /* 100 kHz clock (standard mode) */
  hi2c3.Init.DutyCycle = I2C_DUTYCYCLE_2;              /* 50% duty cycle */
  hi2c3.Init.OwnAddress1 = 0;                          /* Own address when in slave mode (not used) */
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT; /* 7-bit addressing mode */
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE; /* Dual addressing disabled */
  hi2c3.Init.OwnAddress2 = 0;                          /* Second own address (not used) */
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE; /* General call mode disabled */
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;    /* Clock stretching enabled */

  /* Initialize the I2C peripheral with the specified parameters */
  HAL_I2C_Init(&hi2c3);
  log_debug("I2C: I2C3 initialized successfully");
}

/**
 * @brief   Initializes I2C peripheral with custom configuration
 * @details Allows custom configuration of I2C parameters
 * @param   config Pointer to I2C configuration structure
 * @retval  I2C_StatusTypeDef Operation status
 */
I2C_StatusTypeDef I2C_Init_Custom(const I2C_ConfigTypeDef* config)
{
    if (config == NULL)
    {
        return I2C_INVALID_PARAM;
    }

    /* Configure I2C with custom parameters */
    hi2c3.Instance = I2C3;
    hi2c3.Init.ClockSpeed = config->ClockSpeed;
    hi2c3.Init.DutyCycle = config->DutyCycle;
    hi2c3.Init.OwnAddress1 = config->OwnAddress1;
    hi2c3.Init.AddressingMode = config->AddressingMode;
    hi2c3.Init.DualAddressMode = config->DualAddressMode;
    hi2c3.Init.OwnAddress2 = config->OwnAddress2;
    hi2c3.Init.GeneralCallMode = config->GeneralCallMode;
    hi2c3.Init.NoStretchMode = config->NoStretchMode;

    HAL_StatusTypeDef halStatus = HAL_I2C_Init(&hi2c3);
    return I2C_ConvertHALStatus(halStatus);
}

/**
 * @brief   Deinitializes I2C peripheral
 * @details Disables I2C peripheral and releases resources
 * @param   None
 * @retval  I2C_StatusTypeDef Operation status
 */
I2C_StatusTypeDef I2C_DeInit(void)
{
    HAL_StatusTypeDef halStatus = HAL_I2C_DeInit(&hi2c3);
    return I2C_ConvertHALStatus(halStatus);
}

/**
 * @brief   Transmit data to I2C slave device
 * @details Sends data buffer to specified I2C slave address with automatic error recovery
 * @param   DevAddress Target device address (7-bit or 10-bit)
 * @param   pData Pointer to data buffer to transmit
 * @param   Size Number of bytes to transmit
 * @param   Timeout Timeout duration in milliseconds
 * @retval  I2C_StatusTypeDef Operation status
 * @note    On HAL error, automatically calls I2Cx_Error() to reset the I2C bus
 */
I2C_StatusTypeDef I2C_Master_Transmit(uint16_t DevAddress, uint8_t* pData, uint16_t Size, uint32_t Timeout)
{
    if (pData == NULL || Size == 0)
    {
        return I2C_INVALID_PARAM;
    }

    HAL_StatusTypeDef halStatus = HAL_I2C_Master_Transmit(&hi2c3, DevAddress, pData, Size, Timeout);
    if (halStatus != HAL_OK)
    {
        /* Re-Initialize the BUS on error for automatic recovery */
        I2Cx_Error();
        return I2C_ConvertHALStatus(halStatus);
    }

    return I2C_OK;
}

/**
 * @brief   Receive data from I2C slave device
 * @details Receives data from specified I2C slave address with automatic error recovery
 * @param   DevAddress Target device address (7-bit or 10-bit)
 * @param   pData Pointer to data buffer to receive
 * @param   Size Number of bytes to receive
 * @param   Timeout Timeout duration in milliseconds
 * @retval  I2C_StatusTypeDef Operation status
 * @note    On HAL error, automatically calls I2Cx_Error() to reset the I2C bus
 */
I2C_StatusTypeDef I2C_Master_Receive(uint16_t DevAddress, uint8_t* pData, uint16_t Size, uint32_t Timeout)
{
    if (pData == NULL || Size == 0)
    {
        return I2C_INVALID_PARAM;
    }

    HAL_StatusTypeDef halStatus = HAL_I2C_Master_Receive(&hi2c3, DevAddress, pData, Size, Timeout);
    if (halStatus != HAL_OK)
    {
        /* Re-Initialize the BUS on error for automatic recovery */
        I2Cx_Error();
        return I2C_ConvertHALStatus(halStatus);
    }

    return I2C_OK;
}

/**
 * @brief   Transmit and receive data in single transaction
 * @details Performs write followed by read operation with automatic error recovery
 * @param   DevAddress Target device address (7-bit or 10-bit)
 * @param   pTxData Pointer to transmit data buffer
 * @param   TxSize Number of bytes to transmit
 * @param   pRxData Pointer to receive data buffer
 * @param   RxSize Number of bytes to receive
 * @param   Timeout Timeout duration in milliseconds
 * @retval  I2C_StatusTypeDef Operation status
 * @note    On HAL error, automatically calls I2Cx_Error() to reset the I2C bus
 */
I2C_StatusTypeDef I2C_Master_TransmitReceive(uint16_t DevAddress,
                                           uint8_t* pTxData, uint16_t TxSize,
                                           uint8_t* pRxData, uint16_t RxSize,
                                           uint32_t Timeout)
{
    if (pTxData == NULL || pRxData == NULL || TxSize == 0 || RxSize == 0)
    {
        return I2C_INVALID_PARAM;
    }

    HAL_StatusTypeDef halStatus = HAL_I2C_Master_Transmit(&hi2c3, DevAddress, pTxData, TxSize, Timeout);
    if (halStatus != HAL_OK)
    {
        /* Re-Initialize the BUS on error for automatic recovery */
        I2Cx_Error();
        return I2C_ConvertHALStatus(halStatus);
    }

    halStatus = HAL_I2C_Master_Receive(&hi2c3, DevAddress, pRxData, RxSize, Timeout);
    if (halStatus != HAL_OK)
    {
        /* Re-Initialize the BUS on error for automatic recovery */
        I2Cx_Error();
        return I2C_ConvertHALStatus(halStatus);
    }

    return I2C_OK;
}

/**
 * @brief   Write data to I2C memory device
 * @details Writes data to specified memory address in I2C EEPROM/Memory device.
 *          Includes automatic bus re-initialization on communication errors for robustness.
 * @param   DevAddress Target device address
 * @param   MemAddress Memory address to write to
 * @param   MemAddSize Size of memory address (1 or 2 bytes)
 * @param   pData Pointer to data buffer to write
 * @param   Size Number of bytes to write
 * @param   Timeout Timeout duration in milliseconds
 * @retval  I2C_StatusTypeDef Operation status
 * @note    On HAL error, automatically calls I2Cx_Error() to reset the I2C bus
 */
I2C_StatusTypeDef I2C_Mem_Write(uint16_t DevAddress, uint16_t MemAddress,
                               uint16_t MemAddSize, uint8_t* pData,
                               uint16_t Size, uint32_t Timeout)
{
    if (pData == NULL || Size == 0)
    {
        return I2C_INVALID_PARAM;
    }

    HAL_StatusTypeDef halStatus = HAL_I2C_Mem_Write(&hi2c3, DevAddress, MemAddress,
                                                   MemAddSize, pData, Size, Timeout);

    if (halStatus == HAL_OK)
    {
        return I2C_OK;
    }
    else
    {
        /* Re-Initialize the BUS on error for automatic recovery */
        I2Cx_Error();
        return I2C_ConvertHALStatus(halStatus);
    }
}

/**
 * @brief   Read data from I2C memory device
 * @details Reads data from specified memory address in I2C EEPROM/Memory device.
 *          Includes automatic bus re-initialization on communication errors for robustness.
 * @param   DevAddress Target device address
 * @param   MemAddress Memory address to read from
 * @param   MemAddSize Size of memory address (1 or 2 bytes)
 * @param   pData Pointer to data buffer to read into
 * @param   Size Number of bytes to read
 * @param   Timeout Timeout duration in milliseconds
 * @retval  I2C_StatusTypeDef Operation status
 * @note    On HAL error, automatically calls I2Cx_Error() to reset the I2C bus
 */
I2C_StatusTypeDef I2C_Mem_Read(uint16_t DevAddress, uint16_t MemAddress,
                              uint16_t MemAddSize, uint8_t* pData,
                              uint16_t Size, uint32_t Timeout)
{
    if (pData == NULL || Size == 0)
    {
        return I2C_INVALID_PARAM;
    }

    HAL_StatusTypeDef halStatus = HAL_I2C_Mem_Read(&hi2c3, DevAddress, MemAddress,
                                                  MemAddSize, pData, Size, Timeout);

    if (halStatus == HAL_OK)
    {
        return I2C_OK;
    }
    else
    {
        /* Re-Initialize the BUS on error for automatic recovery */
        I2Cx_Error();
        return I2C_ConvertHALStatus(halStatus);
    }
}


/**
 * @brief   Read data from I2C memory device with error recovery
 * @details Reads data from specified memory address in I2C EEPROM/Memory device.
 *          Includes automatic bus re-initialization on communication errors for robustness.
 * @param   DevAddress Target device address
 * @param   MemAddress Memory address to read from
 * @param   MemAddSize Size of memory address (1 or 2 bytes)
 * @param   pData Pointer to data buffer to read into
 * @param   Size Number of bytes to read
 * @param   Timeout Timeout duration in milliseconds
 * @retval  I2C_StatusTypeDef Operation status
 * @note    On HAL error, automatically calls I2Cx_Error() to reset the I2C bus
 */
I2C_StatusTypeDef I2C_Mem_Read_Multi(uint16_t DevAddress, uint16_t MemAddress,
                              uint16_t MemAddSize, uint8_t* pData,
                              uint16_t Size, uint32_t Timeout)
{
    if (pData == NULL || Size == 0)
    {
        return I2C_INVALID_PARAM;
    }

    HAL_StatusTypeDef halStatus = HAL_I2C_Mem_Read(&hi2c3, DevAddress, MemAddress,
                                                  MemAddSize, pData, Size, Timeout);

    if (halStatus == HAL_OK)
    {
        return I2C_OK;
    }
    else
    {
        /* Re-Initialize the BUS on error for automatic recovery */
        I2Cx_Error();
        return I2C_ConvertHALStatus(halStatus);
    }
}


/**
 * @brief   Check if I2C device is ready/responding
 * @details Tests if target device acknowledges its address
 * @param   DevAddress Target device address
 * @param   Trials Number of trials to attempt
 * @param   Timeout Timeout duration in milliseconds
 * @retval  I2C_StatusTypeDef Operation status
 */
I2C_StatusTypeDef I2C_IsDeviceReady(uint16_t DevAddress, uint32_t Trials, uint32_t Timeout)
{
    HAL_StatusTypeDef halStatus = HAL_I2C_IsDeviceReady(&hi2c3, DevAddress, Trials, Timeout);
    return I2C_ConvertHALStatus(halStatus);
}

/**
 * @brief   Scan I2C bus for connected devices
 * @details Scans all valid I2C addresses and reports responding devices
 * @param   pDevices Pointer to array to store found device addresses
 * @param   MaxDevices Maximum number of devices to find
 * @param   Timeout Timeout per device check in milliseconds
 * @retval  uint8_t Number of devices found
 */
uint8_t I2C_ScanBus(uint8_t* pDevices, uint8_t MaxDevices, uint32_t Timeout)
{
    if (pDevices == NULL || MaxDevices == 0)
    {
        return 0;
    }

    uint8_t deviceCount = 0;

    /* Scan all valid I2C addresses (0x08 to 0x77) */
    for (uint8_t addr = I2C_ADDR_MIN; addr <= I2C_ADDR_MAX && deviceCount < MaxDevices; addr++)
    {
        /* Convert to 8-bit address format for HAL function */
        uint16_t devAddr = (uint16_t)addr << 1;

        if (HAL_I2C_IsDeviceReady(&hi2c3, devAddr, 1, Timeout) == HAL_OK)
        {
            pDevices[deviceCount++] = addr;
        }
    }

    return deviceCount;
}

/**
 * @brief   Get current I2C error status
 * @details Returns detailed error information from I2C peripheral
 * @param   None
 * @retval  uint32_t Error code (HAL_I2C_ERROR_* values)
 */
uint32_t I2C_GetError(void)
{
    return HAL_I2C_GetError(&hi2c3);
}


static void I2Cx_Error(void)
{
    /* De-initialize the I2C communication BUS */
    I2C_DeInit();

    /* Attempt to recover the bus if lines are stuck */
    I2C_BusRecovery();

    /* Re-Initialize the I2C communication BUS */
    I2C_Init();
}

/**
 * @brief Attempt to recover a stuck I2C bus by toggling SCL
 * @details Generates up to 9 clock pulses on SCL and issues a STOP condition
 */
static void I2C_BusRecovery(void)
{
        GPIO_InitTypeDef GPIO_InitStruct = {0};

        /* Enable GPIO clocks for I2C3 pins */
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();

        /* Configure SCL and SDA as open-drain outputs with pull-up */
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

        GPIO_InitStruct.Pin = I2C3_SCL_Pin;
        HAL_GPIO_Init(I2C3_SCL_GPIO_Port, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = I2C3_SDA_Pin;
        HAL_GPIO_Init(I2C3_SDA_GPIO_Port, &GPIO_InitStruct);

        /* Release lines high */
        HAL_GPIO_WritePin(I2C3_SCL_GPIO_Port, I2C3_SCL_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(I2C3_SDA_GPIO_Port, I2C3_SDA_Pin, GPIO_PIN_SET);
        HAL_Delay(1);

        /* If SDA is stuck low, toggle SCL to free it */
        if (HAL_GPIO_ReadPin(I2C3_SDA_GPIO_Port, I2C3_SDA_Pin) == GPIO_PIN_RESET)
        {
                for (uint8_t i = 0; i < 9; i++)
                {
                        HAL_GPIO_WritePin(I2C3_SCL_GPIO_Port, I2C3_SCL_Pin, GPIO_PIN_RESET);
                        HAL_Delay(1);
                        HAL_GPIO_WritePin(I2C3_SCL_GPIO_Port, I2C3_SCL_Pin, GPIO_PIN_SET);
                        HAL_Delay(1);
                }
        }

        /* Generate STOP condition: SDA low -> SCL high -> SDA high */
        HAL_GPIO_WritePin(I2C3_SDA_GPIO_Port, I2C3_SDA_Pin, GPIO_PIN_RESET);
        HAL_Delay(1);
        HAL_GPIO_WritePin(I2C3_SCL_GPIO_Port, I2C3_SCL_Pin, GPIO_PIN_SET);
        HAL_Delay(1);
        HAL_GPIO_WritePin(I2C3_SDA_GPIO_Port, I2C3_SDA_Pin, GPIO_PIN_SET);
        HAL_Delay(1);
}

/**
 * @brief   Get I2C status string
 * @details Converts I2C status code to human-readable string
 * @param   status I2C status code
 * @retval  const char* Status description string
 */
const char* I2C_GetStatusString(I2C_StatusTypeDef status)
{
    switch (status)
    {
        case I2C_OK:
            return "Operation completed successfully";
        case I2C_ERROR:
            return "General error occurred";
        case I2C_BUSY:
            return "I2C bus is busy";
        case I2C_TIMEOUT:
            return "Operation timed out";
        case I2C_NACK:
            return "No acknowledge received";
        case I2C_INVALID_PARAM:
            return "Invalid parameter provided";
        default:
            return "Unknown status";
    }
}
