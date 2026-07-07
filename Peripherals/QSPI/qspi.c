/**
  ******************************************************************************
  * @file    qspi.c
  * @brief   QSPI driver implementation for STM32F429 Discovery Board
  * @details This file provides the implementation of QSPI Flash memory functions
  *          using SPI interface for the STM32F429 Discovery board.
  *          Note: STM32F429 doesn't have hardware QSPI, so this implementation
  *          uses SPI with software-controlled chip select for Flash memory.
  * @version 1.0
  * @date    2025-09-04
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "qspi.h"
#include "log.h"
#include <string.h>
#include <stdio.h>

/* Private constants ---------------------------------------------------------*/
#define QSPI_DELAY_MS(x)                HAL_Delay(x)
#define QSPI_MAX_WAIT_TIME              5000    /* Maximum wait time in ms */
#define QSPI_DUMMY_CYCLES               8       /* Dummy cycles for fast read */
#define QSPI_WRITE_ENABLE_TIMEOUT       100     /* Write enable timeout */
#define QSPI_ERASE_TIMEOUT              10000   /* Erase timeout */
#define QSPI_PROGRAM_TIMEOUT            1000    /* Program timeout */
#define QSPI_JEDEC_ID_LENGTH            3       /* JEDEC ID length */
#define QSPI_UNIQUE_ID_LENGTH           8       /* Unique ID length */
#define QSPI_STATUS_REG_SIZE            1       /* Status register size */
#define QSPI_RESET_ENABLE_CMD           0x66    /* Reset Enable command */
#define QSPI_RESET_CMD                  0x99    /* Reset command */
#define QSPI_CHIP_ERASE_TIMEOUT         60000   /* Chip erase timeout (60 seconds) */

/* Private variables ---------------------------------------------------------*/
static QSPI_HandleStructTypeDef *g_hqspi = NULL;

/* Private function prototypes -----------------------------------------------*/
static QSPI_StatusTypeDef QSPI_InitGPIO(void);
static QSPI_StatusTypeDef QSPI_InitSPI(QSPI_HandleStructTypeDef *hqspi_struct);
static QSPI_StatusTypeDef QSPI_SendCommand(QSPI_HandleStructTypeDef *hqspi_struct, uint8_t command);
static QSPI_StatusTypeDef QSPI_SendCommandWithAddress(QSPI_HandleStructTypeDef *hqspi_struct,
                                                      uint8_t command, uint32_t address);
static QSPI_StatusTypeDef QSPI_SendData(QSPI_HandleStructTypeDef *hqspi_struct,
                                        const uint8_t *data, uint16_t size);
static QSPI_StatusTypeDef QSPI_ReceiveData(QSPI_HandleStructTypeDef *hqspi_struct,
                                          uint8_t *data, uint16_t size);
static void QSPI_ChipSelect(bool select);
static QSPI_StatusTypeDef QSPI_AutoDetectMemory(QSPI_HandleStructTypeDef *hqspi_struct);

/* Exported functions --------------------------------------------------------*/

/**
 * @brief Initialize QSPI system
 * @param hqspi_struct Pointer to QSPI handle structure
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_Init(QSPI_HandleStructTypeDef *hqspi_struct)
{
    log_debug("QSPI: Initializing QSPI");

    QSPI_StatusTypeDef status = QSPI_OK;

    /* Check parameters */
    if (hqspi_struct == NULL) {
        return QSPI_INVALID_PARAM;
    }

    /* Initialize structure */
    memset(hqspi_struct, 0, sizeof(QSPI_HandleStructTypeDef));
    hqspi_struct->Timeout = QSPI_TIMEOUT_DEFAULT;
    g_hqspi = hqspi_struct;

    /* Set default configuration */
    hqspi_struct->Config = QSPI_GetDefaultConfig();

    /* Initialize GPIO pins */
    status = QSPI_InitGPIO();
    if (status != QSPI_OK) {
        return status;
    }

    /* Initialize SPI peripheral */
    status = QSPI_InitSPI(hqspi_struct);
    if (status != QSPI_OK) {
        return status;
    }

    /* Deselect chip initially */
    QSPI_ChipSelect(false);

    /* Wait for power-up and exit deep power down if needed */
    QSPI_DELAY_MS(10);
    QSPI_ExitDeepPowerDown(hqspi_struct);
    QSPI_DELAY_MS(5);

    /* Auto-detect memory type */
    status = QSPI_AutoDetectMemory(hqspi_struct);
    if (status != QSPI_OK) {
        return status;
    }

    hqspi_struct->IsInitialized = true;

    log_debug("QSPI: QSPI initialized successfully");

    return QSPI_OK;
}

/**
 * @brief Deinitialize QSPI system
 * @param hqspi_struct Pointer to QSPI handle structure
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_DeInit(QSPI_HandleStructTypeDef *hqspi_struct)
{
    if (hqspi_struct == NULL || !hqspi_struct->IsInitialized) {
        return QSPI_INVALID_PARAM;
    }

    /* Disable memory mapped mode if enabled */
    if (hqspi_struct->IsMemoryMapped) {
        QSPI_DisableMemoryMappedMode(hqspi_struct);
    }

    /* Put device in deep power down */
    QSPI_EnterDeepPowerDown(hqspi_struct);

    /* Deinitialize SPI */
    if (hqspi_struct->hspi != NULL) {
        HAL_SPI_DeInit(hqspi_struct->hspi);
    }

    /* Reset structure */
    memset(hqspi_struct, 0, sizeof(QSPI_HandleStructTypeDef));

    return QSPI_OK;
}

/**
 * @brief Configure QSPI parameters
 * @param hqspi_struct Pointer to QSPI handle structure
 * @param config Pointer to configuration structure
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_Configure(QSPI_HandleStructTypeDef *hqspi_struct, QSPI_ConfigTypeDef *config)
{
    if (hqspi_struct == NULL || config == NULL) {
        return QSPI_INVALID_PARAM;
    }

    /* Copy configuration */
    memcpy(&hqspi_struct->Config, config, sizeof(QSPI_ConfigTypeDef));

    /* Reconfigure SPI if needed */
    if (hqspi_struct->IsInitialized) {
        return QSPI_InitSPI(hqspi_struct);
    }

    return QSPI_OK;
}

/**
 * @brief Reset QSPI Flash memory
 * @param hqspi_struct Pointer to QSPI handle structure
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_Reset(QSPI_HandleStructTypeDef *hqspi_struct)
{
    if (hqspi_struct == NULL || !hqspi_struct->IsInitialized) {
        return QSPI_INVALID_PARAM;
    }

    /* Software reset sequence - varies by manufacturer */
    QSPI_ChipSelect(true);
    uint8_t reset_enable = QSPI_RESET_ENABLE_CMD;  /* Reset Enable command */
    QSPI_SendData(hqspi_struct, &reset_enable, 1);
    QSPI_ChipSelect(false);

    QSPI_DELAY_MS(1);

    QSPI_ChipSelect(true);
    uint8_t reset_cmd = QSPI_RESET_CMD;     /* Reset command */
    QSPI_SendData(hqspi_struct, &reset_cmd, 1);
    QSPI_ChipSelect(false);

    QSPI_DELAY_MS(10);  /* Wait for reset completion */

    return QSPI_OK;
}

/**
 * @brief Get default QSPI configuration
 * @retval QSPI_ConfigTypeDef Default configuration structure
 */
QSPI_ConfigTypeDef QSPI_GetDefaultConfig(void)
{
    QSPI_ConfigTypeDef config;

    config.ClockPrescaler = QSPI_CLOCK_PRESCALER;
    config.FifoThreshold = QSPI_FIFO_THRESHOLD;
    config.SampleShifting = 0;
    config.FlashSize = QSPI_FLASH_SIZE;
    config.ChipSelectHighTime = QSPI_CHIP_SELECT_HIGH_TIME;
    config.ClockMode = 0;
    config.DualFlash = false;

    return config;
}

/**
 * @brief Get memory information
 * @param hqspi_struct Pointer to QSPI handle structure
 * @param memInfo Pointer to memory info structure
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_GetMemoryInfo(QSPI_HandleStructTypeDef *hqspi_struct, QSPI_MemoryInfoTypeDef *memInfo)
{
    if (hqspi_struct == NULL || memInfo == NULL || !hqspi_struct->IsInitialized) {
        return QSPI_INVALID_PARAM;
    }

    /* Copy memory information */
    memcpy(memInfo, &hqspi_struct->MemInfo, sizeof(QSPI_MemoryInfoTypeDef));

    return QSPI_OK;
}

/**
 * @brief Read JEDEC ID from Flash memory
 * @param hqspi_struct Pointer to QSPI handle structure
 * @param device_id Pointer to buffer for device ID (3 bytes)
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_ReadID(QSPI_HandleStructTypeDef *hqspi_struct, uint8_t *device_id)
{
    if (hqspi_struct == NULL || device_id == NULL || !hqspi_struct->IsInitialized) {
        return QSPI_INVALID_PARAM;
    }

    QSPI_StatusTypeDef status;

    QSPI_ChipSelect(true);

    /* Send Read ID command */
    status = QSPI_SendCommand(hqspi_struct, QSPI_CMD_READ_ID);
    if (status == QSPI_OK) {
        /* Receive 3 bytes of ID */
        status = QSPI_ReceiveData(hqspi_struct, device_id, QSPI_JEDEC_ID_LENGTH);
    }

    QSPI_ChipSelect(false);

    return status;
}

/**
 * @brief Read unique ID from Flash memory
 * @param hqspi_struct Pointer to QSPI handle structure
 * @param uniqueID Pointer to buffer for unique ID (8 bytes)
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_ReadUniqueID(QSPI_HandleStructTypeDef *hqspi_struct, uint8_t *uniqueID)
{
    if (hqspi_struct == NULL || uniqueID == NULL || !hqspi_struct->IsInitialized) {
        return QSPI_INVALID_PARAM;
    }

    QSPI_StatusTypeDef status;

    QSPI_ChipSelect(true);

    /* Send Read Unique ID command with dummy bytes */
    status = QSPI_SendCommand(hqspi_struct, QSPI_CMD_READ_UNIQUE_ID);
    if (status == QSPI_OK) {
        /* Send 4 dummy bytes */
        uint8_t dummy[4] = {0x00, 0x00, 0x00, 0x00};
        status = QSPI_SendData(hqspi_struct, dummy, 4);

        if (status == QSPI_OK) {
            /* Receive 8 bytes of unique ID */
            status = QSPI_ReceiveData(hqspi_struct, uniqueID, QSPI_UNIQUE_ID_LENGTH);
        }
    }

    QSPI_ChipSelect(false);

    return status;
}

/**
 * @brief Get Flash memory status
 * @param hqspi_struct Pointer to QSPI handle structure
 * @param status Pointer to status byte
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_GetStatus(QSPI_HandleStructTypeDef *hqspi_struct, uint8_t *status)
{
    if (hqspi_struct == NULL || status == NULL || !hqspi_struct->IsInitialized) {
        return QSPI_INVALID_PARAM;
    }

    QSPI_StatusTypeDef result;

    QSPI_ChipSelect(true);

    /* Send Read Status Register command */
    result = QSPI_SendCommand(hqspi_struct, QSPI_CMD_READ_STATUS_REG);
    if (result == QSPI_OK) {
        /* Receive status byte */
        result = QSPI_ReceiveData(hqspi_struct, status, QSPI_STATUS_REG_SIZE);
    }

    QSPI_ChipSelect(false);

    return result;
}

/**
 * @brief Wait for write operation to complete
 * @param hqspi_struct Pointer to QSPI handle structure
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_WaitForWriteEnd(QSPI_HandleStructTypeDef *hqspi_struct)
{
    if (hqspi_struct == NULL || !hqspi_struct->IsInitialized) {
        return QSPI_INVALID_PARAM;
    }

    uint32_t start_time = HAL_GetTick();
    uint8_t status;

    while ((HAL_GetTick() - start_time) < hqspi_struct->Timeout) {
        if (QSPI_GetStatus(hqspi_struct, &status) != QSPI_OK) {
            return QSPI_ERROR;
        }

        if ((status & QSPI_SR_BUSY) == 0) {
            return QSPI_OK;  /* Write operation completed */
        }

        QSPI_DELAY_MS(1);
    }

    return QSPI_TIMEOUT;
}

/**
 * @brief Enable write operations
 * @param hqspi_struct Pointer to QSPI handle structure
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_WriteEnable(QSPI_HandleStructTypeDef *hqspi_struct)
{
    if (hqspi_struct == NULL || !hqspi_struct->IsInitialized) {
        return QSPI_INVALID_PARAM;
    }

    QSPI_StatusTypeDef status;

    QSPI_ChipSelect(true);
    status = QSPI_SendCommand(hqspi_struct, QSPI_CMD_WRITE_ENABLE);
    QSPI_ChipSelect(false);

    if (status != QSPI_OK) {
        return status;
    }

    /* Verify write enable latch is set */
    uint8_t status_reg;
    uint32_t start_time = HAL_GetTick();

    while ((HAL_GetTick() - start_time) < QSPI_WRITE_ENABLE_TIMEOUT) {
        if (QSPI_GetStatus(hqspi_struct, &status_reg) == QSPI_OK) {
            if (status_reg & QSPI_SR_WEL) {
                return QSPI_OK;
            }
        }
        QSPI_DELAY_MS(1);
    }

    return QSPI_ERROR;
}

/**
 * @brief Disable write operations
 * @param hqspi_struct Pointer to QSPI handle structure
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_WriteDisable(QSPI_HandleStructTypeDef *hqspi_struct)
{
    if (hqspi_struct == NULL || !hqspi_struct->IsInitialized) {
        return QSPI_INVALID_PARAM;
    }

    QSPI_StatusTypeDef status;

    QSPI_ChipSelect(true);
    status = QSPI_SendCommand(hqspi_struct, QSPI_CMD_WRITE_DISABLE);
    QSPI_ChipSelect(false);

    return status;
}

/**
 * @brief Read data from Flash memory
 * @param hqspi_struct Pointer to QSPI handle structure
 * @param address Start address to read from
 * @param data Pointer to data buffer
 * @param size Number of bytes to read
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_Read(QSPI_HandleStructTypeDef *hqspi_struct, uint32_t address, uint8_t *data, uint32_t size)
{
    if (hqspi_struct == NULL || data == NULL || !hqspi_struct->IsInitialized) {
        return QSPI_INVALID_PARAM;
    }

    if (!QSPI_IsAddressValid(address, size)) {
        return QSPI_INVALID_PARAM;
    }

    QSPI_StatusTypeDef status;

    QSPI_ChipSelect(true);

    /* Send Read Data command with address */
    status = QSPI_SendCommandWithAddress(hqspi_struct, QSPI_CMD_READ_DATA, address);
    if (status == QSPI_OK) {
        /* Receive data */
        status = QSPI_ReceiveData(hqspi_struct, data, size);
    }

    QSPI_ChipSelect(false);

    return status;
}

/**
 * @brief Fast read data from Flash memory
 * @param hqspi_struct Pointer to QSPI handle structure
 * @param address Start address to read from
 * @param data Pointer to data buffer
 * @param size Number of bytes to read
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_FastRead(QSPI_HandleStructTypeDef *hqspi_struct, uint32_t address, uint8_t *data, uint32_t size)
{
    if (hqspi_struct == NULL || data == NULL || !hqspi_struct->IsInitialized) {
        return QSPI_INVALID_PARAM;
    }

    if (!QSPI_IsAddressValid(address, size)) {
        return QSPI_INVALID_PARAM;
    }

    QSPI_StatusTypeDef status;

    QSPI_ChipSelect(true);

    /* Send Fast Read command with address */
    status = QSPI_SendCommandWithAddress(hqspi_struct, QSPI_CMD_FAST_READ, address);
    if (status == QSPI_OK) {
        /* Send dummy byte */
        uint8_t dummy = 0x00;
        status = QSPI_SendData(hqspi_struct, &dummy, 1);

        if (status == QSPI_OK) {
            /* Receive data */
            status = QSPI_ReceiveData(hqspi_struct, data, size);
        }
    }

    QSPI_ChipSelect(false);

    return status;
}

/**
 * @brief Quad read data from Flash memory (emulated using SPI)
 * @param hqspi_struct Pointer to QSPI handle structure
 * @param address Start address to read from
 * @param data Pointer to data buffer
 * @param size Number of bytes to read
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_QuadRead(QSPI_HandleStructTypeDef *hqspi_struct, uint32_t address, uint8_t *data, uint32_t size)
{
    /* For STM32F429 without hardware QSPI, fall back to fast read */
    return QSPI_FastRead(hqspi_struct, address, data, size);
}

/* Private function implementations ------------------------------------------*/

/**
 * @brief Initialize GPIO pins for QSPI
 * @retval QSPI_StatusTypeDef Status of the operation
 */
static QSPI_StatusTypeDef QSPI_InitGPIO(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIO clocks */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();

    /* Configure SPI pins */
    /* SCK pin */
    GPIO_InitStruct.Pin = GPIO_PIN_3;  /* PB3 - SPI1_SCK */
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* MISO pin */
    GPIO_InitStruct.Pin = GPIO_PIN_4;  /* PB4 - SPI1_MISO */
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* MOSI pin */
    GPIO_InitStruct.Pin = GPIO_PIN_5;  /* PB5 - SPI1_MOSI */
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* CS pin - software controlled */
    GPIO_InitStruct.Pin = GPIO_PIN_6;  /* PB6 - CS */
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Alternate = 0;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Set CS high initially */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);

    return QSPI_OK;
}

/**
 * @brief Initialize SPI peripheral for QSPI communication
 * @param hqspi_struct Pointer to QSPI handle structure
 * @retval QSPI_StatusTypeDef Status of the operation
 */
static QSPI_StatusTypeDef QSPI_InitSPI(QSPI_HandleStructTypeDef *hqspi_struct)
{
    static SPI_HandleTypeDef hspi1;

    /* Enable SPI clock */
    __HAL_RCC_SPI1_CLK_ENABLE();

    /* Configure SPI */
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;  /* Adjust as needed */
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 10;

    if (HAL_SPI_Init(&hspi1) != HAL_OK) {
        return QSPI_ERROR;
    }

    hqspi_struct->hspi = &hspi1;
    return QSPI_OK;
}

/**
 * @brief Send a command to the QSPI Flash
 * @param hqspi_struct Pointer to QSPI handle structure
 * @param command Command byte to send
 * @retval QSPI_StatusTypeDef Status of the operation
 */
static QSPI_StatusTypeDef QSPI_SendCommand(QSPI_HandleStructTypeDef *hqspi_struct, uint8_t command)
{
    if (HAL_SPI_Transmit(hqspi_struct->hspi, &command, 1, hqspi_struct->Timeout) != HAL_OK) {
        return QSPI_ERROR;
    }
    return QSPI_OK;
}

/**
 * @brief Send a command with 24-bit address to the QSPI Flash
 * @param hqspi_struct Pointer to QSPI handle structure
 * @param command Command byte to send
 * @param address 24-bit address
 * @retval QSPI_StatusTypeDef Status of the operation
 */
static QSPI_StatusTypeDef QSPI_SendCommandWithAddress(QSPI_HandleStructTypeDef *hqspi_struct,
                                                      uint8_t command, uint32_t address)
{
    uint8_t cmd_addr[4];

    cmd_addr[0] = command;
    cmd_addr[1] = (address >> 16) & 0xFF;  /* A23-A16 */
    cmd_addr[2] = (address >> 8) & 0xFF;   /* A15-A8 */
    cmd_addr[3] = address & 0xFF;          /* A7-A0 */

    if (HAL_SPI_Transmit(hqspi_struct->hspi, cmd_addr, 4, hqspi_struct->Timeout) != HAL_OK) {
        return QSPI_ERROR;
    }
    return QSPI_OK;
}

/**
 * @brief Send data to the QSPI Flash
 * @param hqspi_struct Pointer to QSPI handle structure
 * @param data Pointer to data buffer
 * @param size Number of bytes to send
 * @retval QSPI_StatusTypeDef Status of the operation
 */
static QSPI_StatusTypeDef QSPI_SendData(QSPI_HandleStructTypeDef *hqspi_struct,
                                        const uint8_t *data, uint16_t size)
{
    if (HAL_SPI_Transmit(hqspi_struct->hspi, (uint8_t*)data, size, hqspi_struct->Timeout) != HAL_OK) {
        return QSPI_ERROR;
    }
    return QSPI_OK;
}

/**
 * @brief Receive data from the QSPI Flash
 * @param hqspi_struct Pointer to QSPI handle structure
 * @param data Pointer to data buffer
 * @param size Number of bytes to receive
 * @retval QSPI_StatusTypeDef Status of the operation
 */
static QSPI_StatusTypeDef QSPI_ReceiveData(QSPI_HandleStructTypeDef *hqspi_struct,
                                          uint8_t *data, uint16_t size)
{
    if (HAL_SPI_Receive(hqspi_struct->hspi, data, size, hqspi_struct->Timeout) != HAL_OK) {
        return QSPI_ERROR;
    }
    return QSPI_OK;
}

/**
 * @brief Control chip select signal
 * @param select True to select chip, false to deselect
 */
static void QSPI_ChipSelect(bool select)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, select ? GPIO_PIN_RESET : GPIO_PIN_SET);
    /* Small delay for signal stability */
    for (volatile int i = 0; i < 10; i++);
}

/**
 * @brief Auto-detect Flash memory type
 * @param hqspi_struct Pointer to QSPI handle structure
 * @retval QSPI_StatusTypeDef Status of the operation
 */
static QSPI_StatusTypeDef QSPI_AutoDetectMemory(QSPI_HandleStructTypeDef *hqspi_struct)
{
    uint8_t jedec_id[QSPI_JEDEC_ID_LENGTH];
    QSPI_StatusTypeDef status;

    status = QSPI_ReadID(hqspi_struct, jedec_id);
    if (status != QSPI_OK) {
        return status;
    }

    /* Parse JEDEC ID */
    hqspi_struct->MemInfo.ManufacturerID = jedec_id[0];
    hqspi_struct->MemInfo.DeviceID1 = jedec_id[1];
    hqspi_struct->MemInfo.DeviceID2 = jedec_id[2];

    /* Set default memory parameters */
    hqspi_struct->MemInfo.PageSize = QSPI_PAGE_SIZE;
    hqspi_struct->MemInfo.SectorSize = QSPI_SECTOR_SIZE;
    hqspi_struct->MemInfo.BlockSize = QSPI_BLOCK_SIZE;

    /* Identify common Flash memories */
    switch (jedec_id[0]) {
        case 0x20: /* Micron/ST */
            strcpy(hqspi_struct->MemInfo.DeviceName, "Micron/ST Flash");
            hqspi_struct->MemInfo.FlashSize = 16 * 1024 * 1024; /* 16MB default */
            break;
        case 0xEF: /* Winbond */
            strcpy(hqspi_struct->MemInfo.DeviceName, "Winbond Flash");
            hqspi_struct->MemInfo.FlashSize = 16 * 1024 * 1024; /* 16MB default */
            break;
        case 0xC2: /* Macronix */
            strcpy(hqspi_struct->MemInfo.DeviceName, "Macronix Flash");
            hqspi_struct->MemInfo.FlashSize = 16 * 1024 * 1024; /* 16MB default */
            break;
        default:
            strcpy(hqspi_struct->MemInfo.DeviceName, "Unknown Flash");
            hqspi_struct->MemInfo.FlashSize = 16 * 1024 * 1024; /* 16MB default */
            break;
    }

    return QSPI_OK;
}

/* Utility Functions ---------------------------------------------------------*/

/**
 * @brief Check if address and size are valid
 * @param address Start address
 * @param size Data size
 * @retval true if valid, false otherwise
 */
bool QSPI_IsAddressValid(uint32_t address, uint32_t size)
{
    /* Check for basic validation - adjust based on actual Flash size */
    return (address + size) <= (16 * 1024 * 1024);  /* 16MB max */
}

/**
 * @brief Get sector start address for given address
 * @param address Any address within the sector
 * @retval Sector start address
 */
uint32_t QSPI_GetSectorAddress(uint32_t address)
{
    return address & ~(QSPI_SECTOR_SIZE - 1);
}

/**
 * @brief Get block start address for given address
 * @param address Any address within the block
 * @retval Block start address
 */
uint32_t QSPI_GetBlockAddress(uint32_t address)
{
    return address & ~(QSPI_BLOCK_SIZE - 1);
}

/**
 * @brief Get status string for QSPI status code
 * @param status Status code
 * @retval Pointer to status string
 */
const char* QSPI_GetStatusString(QSPI_StatusTypeDef status)
{
    switch (status) {
        case QSPI_OK:             return "OK";
        case QSPI_ERROR:          return "Error";
        case QSPI_BUSY:           return "Busy";
        case QSPI_TIMEOUT:        return "Timeout";
        case QSPI_INVALID_PARAM:  return "Invalid Parameter";
        case QSPI_NOT_SUPPORTED:  return "Not Supported";
        case QSPI_WRITE_PROTECTED: return "Write Protected";
        case QSPI_ERASE_ERROR:    return "Erase Error";
        case QSPI_PROGRAM_ERROR:  return "Program Error";
        default:                  return "Unknown";
    }
}

/* Write and Erase implementations ------------------------------------------*/

/**
 * @brief Write page to Flash memory
 * @param hqspi_struct Pointer to QSPI handle structure
 * @param address Start address to write to
 * @param data Pointer to data buffer
 * @param size Number of bytes to write (max 256)
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_WritePage(QSPI_HandleStructTypeDef *hqspi_struct, uint32_t address, const uint8_t *data, uint32_t size)
{
    if (hqspi_struct == NULL || data == NULL || !hqspi_struct->IsInitialized) {
        return QSPI_INVALID_PARAM;
    }

    if (size == 0 || size > QSPI_PAGE_SIZE) {
        return QSPI_INVALID_PARAM;
    }

    if (!QSPI_IsAddressValid(address, size)) {
        return QSPI_INVALID_PARAM;
    }

    /* Check if address is page-aligned for optimal performance */
    if ((address % QSPI_PAGE_SIZE) + size > QSPI_PAGE_SIZE) {
        return QSPI_INVALID_PARAM;  /* Would cross page boundary */
    }

    QSPI_StatusTypeDef status;

    /* Enable write operations */
    status = QSPI_WriteEnable(hqspi_struct);
    if (status != QSPI_OK) {
        return status;
    }

    /* Send Page Program command */
    QSPI_ChipSelect(true);

    status = QSPI_SendCommandWithAddress(hqspi_struct, QSPI_CMD_PAGE_PROGRAM, address);
    if (status == QSPI_OK) {
        /* Send data */
        status = QSPI_SendData(hqspi_struct, data, size);
    }

    QSPI_ChipSelect(false);

    if (status != QSPI_OK) {
        return status;
    }

    /* Wait for programming to complete */
    status = QSPI_WaitForWriteEnd(hqspi_struct);
    if (status != QSPI_OK) {
        return QSPI_PROGRAM_ERROR;
    }

    return QSPI_OK;
}

/**
 * @brief Quad write page to Flash memory (emulated using standard SPI)
 * @param hqspi_struct Pointer to QSPI handle structure
 * @param address Start address to write to
 * @param data Pointer to data buffer
 * @param size Number of bytes to write (max 256)
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_QuadWritePage(QSPI_HandleStructTypeDef *hqspi_struct, uint32_t address, const uint8_t *data, uint32_t size)
{
    /* For STM32F429 without hardware QSPI, fall back to normal page write */
    return QSPI_WritePage(hqspi_struct, address, data, size);
}

/**
 * @brief Write data to Flash memory (multi-page write)
 * @param hqspi_struct Pointer to QSPI handle structure
 * @param address Start address to write to
 * @param data Pointer to data buffer
 * @param size Number of bytes to write
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_Write(QSPI_HandleStructTypeDef *hqspi_struct, uint32_t address, const uint8_t *data, uint32_t size)
{
    if (hqspi_struct == NULL || data == NULL || !hqspi_struct->IsInitialized) {
        return QSPI_INVALID_PARAM;
    }

    if (size == 0) {
        return QSPI_INVALID_PARAM;
    }

    if (!QSPI_IsAddressValid(address, size)) {
        return QSPI_INVALID_PARAM;
    }

    QSPI_StatusTypeDef status = QSPI_OK;
    uint32_t current_address = address;
    const uint8_t *current_data = data;
    uint32_t remaining_size = size;

    while (remaining_size > 0) {
        /* Calculate bytes to write in this page */
        uint32_t page_offset = current_address % QSPI_PAGE_SIZE;
        uint32_t bytes_to_write = QSPI_PAGE_SIZE - page_offset;

        if (bytes_to_write > remaining_size) {
            bytes_to_write = remaining_size;
        }

        /* Write the page */
        status = QSPI_WritePage(hqspi_struct, current_address, current_data, bytes_to_write);
        if (status != QSPI_OK) {
            return status;
        }

        /* Update pointers and counters */
        current_address += bytes_to_write;
        current_data += bytes_to_write;
        remaining_size -= bytes_to_write;
    }

    return QSPI_OK;
}

/**
 * @brief Erase sector (4KB)
 * @param hqspi_struct Pointer to QSPI handle structure
 * @param address Address within sector to erase
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_EraseSector(QSPI_HandleStructTypeDef *hqspi_struct, uint32_t address)
{
    if (hqspi_struct == NULL || !hqspi_struct->IsInitialized) {
        return QSPI_INVALID_PARAM;
    }

    if (!QSPI_IsAddressValid(address, 1)) {
        return QSPI_INVALID_PARAM;
    }

    QSPI_StatusTypeDef status;

    /* Enable write operations */
    status = QSPI_WriteEnable(hqspi_struct);
    if (status != QSPI_OK) {
        return status;
    }

    /* Send Sector Erase command */
    QSPI_ChipSelect(true);
    status = QSPI_SendCommandWithAddress(hqspi_struct, QSPI_CMD_SECTOR_ERASE, address);
    QSPI_ChipSelect(false);

    if (status != QSPI_OK) {
        return status;
    }

    /* Wait for erase to complete (sector erase can take several seconds) */
    uint32_t original_timeout = hqspi_struct->Timeout;
    hqspi_struct->Timeout = QSPI_ERASE_TIMEOUT;

    status = QSPI_WaitForWriteEnd(hqspi_struct);

    hqspi_struct->Timeout = original_timeout;

    if (status != QSPI_OK) {
        return QSPI_ERASE_ERROR;
    }

    return QSPI_OK;
}

/**
 * @brief Erase 32KB block
 * @param hqspi_struct Pointer to QSPI handle structure
 * @param address Address within block to erase
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_EraseBlock32K(QSPI_HandleStructTypeDef *hqspi_struct, uint32_t address)
{
    if (hqspi_struct == NULL || !hqspi_struct->IsInitialized) {
        return QSPI_INVALID_PARAM;
    }

    if (!QSPI_IsAddressValid(address, 1)) {
        return QSPI_INVALID_PARAM;
    }

    QSPI_StatusTypeDef status;

    /* Enable write operations */
    status = QSPI_WriteEnable(hqspi_struct);
    if (status != QSPI_OK) {
        return status;
    }

    /* Send 32KB Block Erase command */
    QSPI_ChipSelect(true);
    status = QSPI_SendCommandWithAddress(hqspi_struct, QSPI_CMD_BLOCK_ERASE_32K, address);
    QSPI_ChipSelect(false);

    if (status != QSPI_OK) {
        return status;
    }

    /* Wait for erase to complete */
    uint32_t original_timeout = hqspi_struct->Timeout;
    hqspi_struct->Timeout = QSPI_ERASE_TIMEOUT;

    status = QSPI_WaitForWriteEnd(hqspi_struct);

    hqspi_struct->Timeout = original_timeout;

    if (status != QSPI_OK) {
        return QSPI_ERASE_ERROR;
    }

    return QSPI_OK;
}

/**
 * @brief Erase 64KB block
 * @param hqspi_struct Pointer to QSPI handle structure
 * @param address Address within block to erase
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_EraseBlock64K(QSPI_HandleStructTypeDef *hqspi_struct, uint32_t address)
{
    if (hqspi_struct == NULL || !hqspi_struct->IsInitialized) {
        return QSPI_INVALID_PARAM;
    }

    if (!QSPI_IsAddressValid(address, 1)) {
        return QSPI_INVALID_PARAM;
    }

    QSPI_StatusTypeDef status;

    /* Enable write operations */
    status = QSPI_WriteEnable(hqspi_struct);
    if (status != QSPI_OK) {
        return status;
    }

    /* Send 64KB Block Erase command */
    QSPI_ChipSelect(true);
    status = QSPI_SendCommandWithAddress(hqspi_struct, QSPI_CMD_BLOCK_ERASE_64K, address);
    QSPI_ChipSelect(false);

    if (status != QSPI_OK) {
        return status;
    }

    /* Wait for erase to complete */
    uint32_t original_timeout = hqspi_struct->Timeout;
    hqspi_struct->Timeout = QSPI_ERASE_TIMEOUT;

    status = QSPI_WaitForWriteEnd(hqspi_struct);

    hqspi_struct->Timeout = original_timeout;

    if (status != QSPI_OK) {
        return QSPI_ERASE_ERROR;
    }

    return QSPI_OK;
}

/**
 * @brief Erase entire chip
 * @param hqspi_struct Pointer to QSPI handle structure
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_EraseChip(QSPI_HandleStructTypeDef *hqspi_struct)
{
    if (hqspi_struct == NULL || !hqspi_struct->IsInitialized) {
        return QSPI_INVALID_PARAM;
    }

    QSPI_StatusTypeDef status;

    /* Enable write operations */
    status = QSPI_WriteEnable(hqspi_struct);
    if (status != QSPI_OK) {
        return status;
    }

    /* Send Chip Erase command */
    QSPI_ChipSelect(true);
    status = QSPI_SendCommand(hqspi_struct, QSPI_CMD_CHIP_ERASE);
    QSPI_ChipSelect(false);

    if (status != QSPI_OK) {
        return status;
    }

    /* Wait for erase to complete (chip erase can take very long time) */
    uint32_t original_timeout = hqspi_struct->Timeout;
    hqspi_struct->Timeout = QSPI_CHIP_ERASE_TIMEOUT;  /* 60 seconds timeout for chip erase */

    status = QSPI_WaitForWriteEnd(hqspi_struct);

    hqspi_struct->Timeout = original_timeout;

    if (status != QSPI_OK) {
        return QSPI_ERASE_ERROR;
    }

    return QSPI_OK;
}

/**
 * @brief Enable memory mapped mode (stub implementation)
 * @param hqspi_struct Pointer to QSPI handle structure
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_EnableMemoryMappedMode(QSPI_HandleStructTypeDef *hqspi_struct)
{
    /* Suppress unused parameter warning */
    (void)hqspi_struct;

    /* Not supported on STM32F429 without hardware QSPI */
    return QSPI_NOT_SUPPORTED;
}

/**
 * @brief Disable memory mapped mode (stub implementation)
 * @param hqspi_struct Pointer to QSPI handle structure
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_DisableMemoryMappedMode(QSPI_HandleStructTypeDef *hqspi_struct)
{
    /* Suppress unused parameter warning */
    (void)hqspi_struct;

    /* Not supported on STM32F429 without hardware QSPI */
    return QSPI_NOT_SUPPORTED;
}

/**
 * @brief Enter deep power down mode
 * @param hqspi_struct Pointer to QSPI handle structure
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_EnterDeepPowerDown(QSPI_HandleStructTypeDef *hqspi_struct)
{
    if (hqspi_struct == NULL || !hqspi_struct->IsInitialized) {
        return QSPI_INVALID_PARAM;
    }

    QSPI_StatusTypeDef status;

    QSPI_ChipSelect(true);
    status = QSPI_SendCommand(hqspi_struct, QSPI_CMD_DEEP_POWER_DOWN);
    QSPI_ChipSelect(false);

    return status;
}

/**
 * @brief Exit deep power down mode
 * @param hqspi_struct Pointer to QSPI handle structure
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_ExitDeepPowerDown(QSPI_HandleStructTypeDef *hqspi_struct)
{
    if (hqspi_struct == NULL) {
        return QSPI_INVALID_PARAM;
    }

    QSPI_StatusTypeDef status;

    QSPI_ChipSelect(true);
    status = QSPI_SendCommand(hqspi_struct, QSPI_CMD_RELEASE_POWER_DOWN);
    QSPI_ChipSelect(false);

    /* Wait for device to wake up */
    QSPI_DELAY_MS(1);

    return status;
}

/* Weak callback implementations --------------------------------------------*/

/**
 * @brief Error callback function
 * @param hqspi_struct Pointer to QSPI handle structure
 */
__weak void QSPI_ErrorCallback(QSPI_HandleStructTypeDef *hqspi_struct)
{
    /* User can override this function */
    UNUSED(hqspi_struct);
}

/**
 * @brief Completion callback function
 * @param hqspi_struct Pointer to QSPI handle structure
 */
__weak void QSPI_CompletionCallback(QSPI_HandleStructTypeDef *hqspi_struct)
{
    /* User can override this function */
    UNUSED(hqspi_struct);
}
