/**
 ******************************************************************************
 * @file    sdcard.c
 * @author  Mahonri
 * @brief   SD Card peripheral driver implementation for STM32F429I Discovery board
 *          This file provides the implementation of SD Card functions
 ******************************************************************************
 * @attention
 *
 * This software is provided as-is, without any express or implied warranties.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "sdcard.h"
#include <string.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* SDIO pins configuration for STM32F429I-Discovery */
#define SDCARD_SDIO_CLK_PIN              GPIO_PIN_12
#define SDCARD_SDIO_CLK_GPIO_PORT        GPIOC
#define SDCARD_SDIO_CLK_GPIO_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()

#define SDCARD_SDIO_CMD_PIN              GPIO_PIN_2
#define SDCARD_SDIO_CMD_GPIO_PORT        GPIOD
#define SDCARD_SDIO_CMD_GPIO_CLK_ENABLE() __HAL_RCC_GPIOD_CLK_ENABLE()

#define SDCARD_SDIO_D0_PIN               GPIO_PIN_8
#define SDCARD_SDIO_D1_PIN               GPIO_PIN_9
#define SDCARD_SDIO_D2_PIN               GPIO_PIN_10
#define SDCARD_SDIO_D3_PIN               GPIO_PIN_11
#define SDCARD_SDIO_D_GPIO_PORT          GPIOC
#define SDCARD_SDIO_D_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOC_CLK_ENABLE()

/* Card detect pin (if available) */
#define SDCARD_DETECT_PIN                GPIO_PIN_13
#define SDCARD_DETECT_GPIO_PORT          GPIOC
#define SDCARD_DETECT_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOC_CLK_ENABLE()

/* Write protect pin (if available) */
#define SDCARD_WP_PIN                    GPIO_PIN_14
#define SDCARD_WP_GPIO_PORT              GPIOC
#define SDCARD_WP_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()

/* DMA configuration */
#define SDCARD_DMA_CLK_ENABLE()          __HAL_RCC_DMA2_CLK_ENABLE()
#define SDCARD_DMA_STREAM                DMA2_Stream3
#define SDCARD_DMA_CHANNEL               DMA_CHANNEL_4
#define SDCARD_DMA_IRQn                  DMA2_Stream3_IRQn

/* Performance test constants */
#define SDCARD_PERF_TEST_BLOCKS          1024U    /* Number of blocks for performance test */
#define SDCARD_PERF_TEST_PATTERN         0xA5U    /* Test pattern for write test */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static SD_HandleTypeDef hsd;
static DMA_HandleTypeDef hdma_sdio;
static volatile bool transferComplete = false;
static volatile bool transferError = false;

/* Private function prototypes -----------------------------------------------*/
static void SDCARD_MspInit(void);
static void SDCARD_MspDeInit(void);
static SDCARD_StatusTypeDef SDCARD_HALStatusToSDStatus(HAL_StatusTypeDef halStatus);
static void SDCARD_DMA_Config(void);

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  Initialize the SD Card peripheral
 * @retval SDCARD_StatusTypeDef: Status of the operation
 */
SDCARD_StatusTypeDef SDCARD_Init(void)
{
    HAL_StatusTypeDef halStatus;

    /* Initialize MSP */
    SDCARD_MspInit();

    /* Configure SD Card handle */
    hsd.Instance = SDIO;
    hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
    hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
    hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
    hsd.Init.BusWide = SDIO_BUS_WIDE_1B;
    hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
    hsd.Init.ClockDiv = 118; /* 48MHz / (118 + 2) = 400kHz for initialization */

    /* Initialize SD Card */
    halStatus = HAL_SD_Init(&hsd);
    if (halStatus != HAL_OK)
    {
        return SDCARD_HALStatusToSDStatus(halStatus);
    }

    /* Configure wide bus mode */
    halStatus = HAL_SD_ConfigWideBusOperation(&hsd, SDIO_BUS_WIDE_4B);
    if (halStatus != HAL_OK)
    {
        return SDCARD_HALStatusToSDStatus(halStatus);
    }

    /* Configure DMA for better performance */
    SDCARD_DMA_Config();

    return SDCARD_STATUS_OK;
}

/**
 * @brief  Deinitialize the SD Card peripheral
 * @retval SDCARD_StatusTypeDef: Status of the operation
 */
SDCARD_StatusTypeDef SDCARD_DeInit(void)
{
    HAL_StatusTypeDef halStatus;

    /* Deinitialize SD Card */
    halStatus = HAL_SD_DeInit(&hsd);
    if (halStatus != HAL_OK)
    {
        return SDCARD_HALStatusToSDStatus(halStatus);
    }

    /* Deinitialize DMA */
    HAL_DMA_DeInit(&hdma_sdio);

    /* Deinitialize MSP */
    SDCARD_MspDeInit();

    return SDCARD_STATUS_OK;
}

/**
 * @brief  Get SD Card information
 * @param  pCardInfo: Pointer to card information structure
 * @retval SDCARD_StatusTypeDef: Status of the operation
 */
SDCARD_StatusTypeDef SDCARD_GetCardInfo(SDCARD_CardInfoTypeDef* pCardInfo)
{
    HAL_SD_CardInfoTypeDef halCardInfo;
    HAL_StatusTypeDef halStatus;

    if (pCardInfo == NULL)
    {
        return SDCARD_STATUS_ERROR;
    }

    /* Get card information from HAL */
    halStatus = HAL_SD_GetCardInfo(&hsd, &halCardInfo);
    if (halStatus != HAL_OK)
    {
        return SDCARD_HALStatusToSDStatus(halStatus);
    }

    /* Copy information to our structure */
    pCardInfo->CardType = halCardInfo.CardType;
    pCardInfo->CardVersion = halCardInfo.CardVersion;
    pCardInfo->Class = halCardInfo.Class;
    pCardInfo->RelCardAdd = halCardInfo.RelCardAdd;
    pCardInfo->BlockNbr = halCardInfo.BlockNbr;
    pCardInfo->BlockSize = halCardInfo.BlockSize;
    pCardInfo->LogBlockNbr = halCardInfo.LogBlockNbr;
    pCardInfo->LogBlockSize = halCardInfo.LogBlockSize;
    pCardInfo->CardSize = (uint32_t)(((uint64_t)halCardInfo.LogBlockNbr * halCardInfo.LogBlockSize) >> 20); /* Size in MB */
    pCardInfo->WriteProtected = SDCARD_IsWriteProtected();

    return SDCARD_STATUS_OK;
}

/**
 * @brief  Check if SD Card is present
 * @retval bool: true if card is present, false otherwise
 */
bool SDCARD_IsCardPresent(void)
{
    /* Check card detect pin if available */
    if (HAL_GPIO_ReadPin(SDCARD_DETECT_GPIO_PORT, SDCARD_DETECT_PIN) == GPIO_PIN_RESET)
    {
        return true; /* Card present (assuming active low) */
    }

    /* Alternative: Try to get card state */
    HAL_SD_CardStateTypeDef cardState = HAL_SD_GetCardState(&hsd);
    return (cardState != HAL_SD_CARD_ERROR);
}

/**
 * @brief  Check if SD Card is write protected
 * @retval bool: true if card is write protected, false otherwise
 */
bool SDCARD_IsWriteProtected(void)
{
    /* Check write protect pin if available */
    return (HAL_GPIO_ReadPin(SDCARD_WP_GPIO_PORT, SDCARD_WP_PIN) == GPIO_PIN_SET);
}

/**
 * @brief  Read block(s) from SD Card
 * @param  pData: Pointer to buffer to store read data
 * @param  BlockAddr: Block address to read from
 * @param  NumOfBlocks: Number of blocks to read
 * @param  Timeout: Timeout value in milliseconds
 * @retval SDCARD_StatusTypeDef: Status of the operation
 */
SDCARD_StatusTypeDef SDCARD_ReadBlocks(uint8_t* pData, uint32_t BlockAddr,
                                       uint32_t NumOfBlocks, uint32_t Timeout)
{
    HAL_StatusTypeDef halStatus;

    if (pData == NULL || NumOfBlocks == 0)
    {
        return SDCARD_STATUS_ERROR;
    }

    if (!SDCARD_IsCardPresent())
    {
        return SDCARD_STATUS_NO_CARD;
    }

    /* Read blocks */
    halStatus = HAL_SD_ReadBlocks(&hsd, pData, BlockAddr, NumOfBlocks, Timeout);

    return SDCARD_HALStatusToSDStatus(halStatus);
}

/**
 * @brief  Write block(s) to SD Card
 * @param  pData: Pointer to data buffer to write
 * @param  BlockAddr: Block address to write to
 * @param  NumOfBlocks: Number of blocks to write
 * @param  Timeout: Timeout value in milliseconds
 * @retval SDCARD_StatusTypeDef: Status of the operation
 */
SDCARD_StatusTypeDef SDCARD_WriteBlocks(const uint8_t* pData, uint32_t BlockAddr,
                                        uint32_t NumOfBlocks, uint32_t Timeout)
{
    HAL_StatusTypeDef halStatus;

    if (pData == NULL || NumOfBlocks == 0)
    {
        return SDCARD_STATUS_ERROR;
    }

    if (!SDCARD_IsCardPresent())
    {
        return SDCARD_STATUS_NO_CARD;
    }

    if (SDCARD_IsWriteProtected())
    {
        return SDCARD_STATUS_WRITE_PROTECTED;
    }

    /* Write blocks */
    halStatus = HAL_SD_WriteBlocks(&hsd, (uint8_t*)pData, BlockAddr, NumOfBlocks, Timeout);

    return SDCARD_HALStatusToSDStatus(halStatus);
}

/**
 * @brief  Read block(s) from SD Card using DMA
 * @param  pData: Pointer to buffer to store read data
 * @param  BlockAddr: Block address to read from
 * @param  NumOfBlocks: Number of blocks to read
 * @retval SDCARD_StatusTypeDef: Status of the operation
 */
SDCARD_StatusTypeDef SDCARD_ReadBlocks_DMA(uint8_t* pData, uint32_t BlockAddr,
                                           uint32_t NumOfBlocks)
{
    HAL_StatusTypeDef halStatus;

    if (pData == NULL || NumOfBlocks == 0)
    {
        return SDCARD_STATUS_ERROR;
    }

    if (!SDCARD_IsCardPresent())
    {
        return SDCARD_STATUS_NO_CARD;
    }

    /* Reset transfer flags */
    transferComplete = false;
    transferError = false;

    /* Read blocks with DMA */
    halStatus = HAL_SD_ReadBlocks_DMA(&hsd, pData, BlockAddr, NumOfBlocks);

    return SDCARD_HALStatusToSDStatus(halStatus);
}

/**
 * @brief  Write block(s) to SD Card using DMA
 * @param  pData: Pointer to data buffer to write
 * @param  BlockAddr: Block address to write to
 * @param  NumOfBlocks: Number of blocks to write
 * @retval SDCARD_StatusTypeDef: Status of the operation
 */
SDCARD_StatusTypeDef SDCARD_WriteBlocks_DMA(const uint8_t* pData, uint32_t BlockAddr,
                                            uint32_t NumOfBlocks)
{
    HAL_StatusTypeDef halStatus;

    if (pData == NULL || NumOfBlocks == 0)
    {
        return SDCARD_STATUS_ERROR;
    }

    if (!SDCARD_IsCardPresent())
    {
        return SDCARD_STATUS_NO_CARD;
    }

    if (SDCARD_IsWriteProtected())
    {
        return SDCARD_STATUS_WRITE_PROTECTED;
    }

    /* Reset transfer flags */
    transferComplete = false;
    transferError = false;

    /* Write blocks with DMA */
    halStatus = HAL_SD_WriteBlocks_DMA(&hsd, (uint8_t*)pData, BlockAddr, NumOfBlocks);

    return SDCARD_HALStatusToSDStatus(halStatus);
}

/**
 * @brief  Erase block(s) on SD Card
 * @param  StartAddr: Start block address to erase
 * @param  EndAddr: End block address to erase
 * @param  Timeout: Timeout value in milliseconds
 * @retval SDCARD_StatusTypeDef: Status of the operation
 */
SDCARD_StatusTypeDef SDCARD_Erase(uint32_t StartAddr, uint32_t EndAddr, uint32_t Timeout)
{
    HAL_StatusTypeDef halStatus;

    if (StartAddr > EndAddr)
    {
        return SDCARD_STATUS_ERROR;
    }

    if (!SDCARD_IsCardPresent())
    {
        return SDCARD_STATUS_NO_CARD;
    }

    if (SDCARD_IsWriteProtected())
    {
        return SDCARD_STATUS_WRITE_PROTECTED;
    }

    /* Erase blocks */
    halStatus = HAL_SD_Erase(&hsd, StartAddr, EndAddr);

    if (halStatus != HAL_OK)
    {
        return SDCARD_HALStatusToSDStatus(halStatus);
    }

    /* Wait for erase to complete */
    uint32_t tickStart = HAL_GetTick();
    while ((HAL_SD_GetCardState(&hsd) != HAL_SD_CARD_TRANSFER) &&
           ((HAL_GetTick() - tickStart) < Timeout))
    {
        /* Wait */
    }

    if (HAL_SD_GetCardState(&hsd) != HAL_SD_CARD_TRANSFER)
    {
        return SDCARD_STATUS_TIMEOUT;
    }

    return SDCARD_STATUS_OK;
}

/**
 * @brief  Get SD Card state
 * @retval HAL_SD_CardStateTypeDef: Current card state
 */
HAL_SD_CardStateTypeDef SDCARD_GetCardState(void)
{
    return HAL_SD_GetCardState(&hsd);
}

/**
 * @brief  Abort ongoing SD Card operation
 * @retval SDCARD_StatusTypeDef: Status of the operation
 */
SDCARD_StatusTypeDef SDCARD_Abort(void)
{
    HAL_StatusTypeDef halStatus;

    halStatus = HAL_SD_Abort(&hsd);

    return SDCARD_HALStatusToSDStatus(halStatus);
}

/**
 * @brief  Format SD Card (creates basic file system structure)
 * @param  FormatType: Type of format (0 = Quick format, 1 = Full format)
 * @retval SDCARD_StatusTypeDef: Status of the operation
 */
SDCARD_StatusTypeDef SDCARD_Format(uint8_t FormatType)
{
    SDCARD_StatusTypeDef status;
    SDCARD_CardInfoTypeDef cardInfo;
    uint8_t zeroBuffer[SDCARD_BLOCK_SIZE];
    uint32_t i;

    if (!SDCARD_IsCardPresent())
    {
        return SDCARD_STATUS_NO_CARD;
    }

    if (SDCARD_IsWriteProtected())
    {
        return SDCARD_STATUS_WRITE_PROTECTED;
    }

    /* Get card information */
    status = SDCARD_GetCardInfo(&cardInfo);
    if (status != SDCARD_STATUS_OK)
    {
        return status;
    }

    /* Prepare zero buffer */
    memset(zeroBuffer, 0, sizeof(zeroBuffer));

    if (FormatType == 0) /* Quick format */
    {
        /* Clear first few blocks (boot sector, FAT tables) */
        for (i = 0; i < 32; i++)
        {
            status = SDCARD_WriteBlocks(zeroBuffer, i, 1, SDCARD_TIMEOUT_WRITE);
            if (status != SDCARD_STATUS_OK)
            {
                return status;
            }
        }
    }
    else /* Full format */
    {
        /* Clear all blocks (WARNING: This will take a long time!) */
        for (i = 0; i < cardInfo.LogBlockNbr; i += 32)
        {
            uint32_t blocksToWrite = (cardInfo.LogBlockNbr - i) > 32 ? 32 : (cardInfo.LogBlockNbr - i);
            status = SDCARD_WriteBlocks(zeroBuffer, i, blocksToWrite, SDCARD_TIMEOUT_WRITE);
            if (status != SDCARD_STATUS_OK)
            {
                return status;
            }
        }
    }

    return SDCARD_STATUS_OK;
}

/**
 * @brief  Get SD Card capacity in bytes
 * @retval uint64_t: Card capacity in bytes
 */
uint64_t SDCARD_GetCapacity(void)
{
    SDCARD_CardInfoTypeDef cardInfo;

    if (SDCARD_GetCardInfo(&cardInfo) == SDCARD_STATUS_OK)
    {
        return ((uint64_t)cardInfo.LogBlockNbr * cardInfo.LogBlockSize);
    }

    return 0;
}

/**
 * @brief  Get free space on SD Card in bytes
 * @retval uint64_t: Free space in bytes
 */
uint64_t SDCARD_GetFreeSpace(void)
{
    /* This would require file system integration to determine actual free space */
    /* For now, return total capacity as a placeholder */
    return SDCARD_GetCapacity();
}

/**
 * @brief  Test SD Card performance (read/write speed)
 * @param  pReadSpeed: Pointer to store read speed in KB/s
 * @param  pWriteSpeed: Pointer to store write speed in KB/s
 * @retval SDCARD_StatusTypeDef: Status of the operation
 */
SDCARD_StatusTypeDef SDCARD_TestPerformance(uint32_t* pReadSpeed, uint32_t* pWriteSpeed)
{
    SDCARD_StatusTypeDef status;
    uint8_t testBuffer[SDCARD_BLOCK_SIZE * 16]; /* Test with 16 blocks */
    uint32_t startTick, endTick, duration;
    uint32_t testBlocks = 16;
    uint32_t testStartAddr = 1000; /* Use a safe area of the card */

    if (pReadSpeed == NULL || pWriteSpeed == NULL)
    {
        return SDCARD_STATUS_ERROR;
    }

    if (!SDCARD_IsCardPresent())
    {
        return SDCARD_STATUS_NO_CARD;
    }

    /* Fill test buffer with pattern */
    for (uint32_t i = 0; i < sizeof(testBuffer); i++)
    {
        testBuffer[i] = (uint8_t)(i % 256);
    }

    /* Test write speed */
    startTick = HAL_GetTick();
    status = SDCARD_WriteBlocks(testBuffer, testStartAddr, testBlocks, SDCARD_TIMEOUT_WRITE);
    endTick = HAL_GetTick();

    if (status != SDCARD_STATUS_OK)
    {
        return status;
    }

    duration = endTick - startTick;
    if (duration > 0)
    {
        *pWriteSpeed = (testBlocks * SDCARD_BLOCK_SIZE) / duration; /* KB/s */
    }
    else
    {
        *pWriteSpeed = 0;
    }

    /* Clear buffer for read test */
    memset(testBuffer, 0, sizeof(testBuffer));

    /* Test read speed */
    startTick = HAL_GetTick();
    status = SDCARD_ReadBlocks(testBuffer, testStartAddr, testBlocks, SDCARD_TIMEOUT_READ);
    endTick = HAL_GetTick();

    if (status != SDCARD_STATUS_OK)
    {
        return status;
    }

    duration = endTick - startTick;
    if (duration > 0)
    {
        *pReadSpeed = (testBlocks * SDCARD_BLOCK_SIZE) / duration; /* KB/s */
    }
    else
    {
        *pReadSpeed = 0;
    }

    return SDCARD_STATUS_OK;
}

/**
 * @brief  SD Card transfer complete callback (for DMA operations)
 * @note   This function should be implemented by the user
 * @retval None
 */
__weak void SDCARD_TxCpltCallback(void)
{
    /* NOTE: This function should not be modified, when the callback is needed,
             the SDCARD_TxCpltCallback could be implemented in the user file
    */
    transferComplete = true;
}

/**
 * @brief  SD Card receive complete callback (for DMA operations)
 * @note   This function should be implemented by the user
 * @retval None
 */
__weak void SDCARD_RxCpltCallback(void)
{
    /* NOTE: This function should not be modified, when the callback is needed,
             the SDCARD_RxCpltCallback could be implemented in the user file
    */
    transferComplete = true;
}

/**
 * @brief  SD Card error callback
 * @note   This function should be implemented by the user
 * @retval None
 */
__weak void SDCARD_ErrorCallback(void)
{
    /* NOTE: This function should not be modified, when the callback is needed,
             the SDCARD_ErrorCallback could be implemented in the user file
    */
    transferError = true;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Initialize SD Card MSP
 * @retval None
 */
static void SDCARD_MspInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable peripheral clocks */
    __HAL_RCC_SDIO_CLK_ENABLE();
    SDCARD_SDIO_CLK_GPIO_CLK_ENABLE();
    SDCARD_SDIO_CMD_GPIO_CLK_ENABLE();
    SDCARD_SDIO_D_GPIO_CLK_ENABLE();
    SDCARD_DETECT_GPIO_CLK_ENABLE();
    SDCARD_WP_GPIO_CLK_ENABLE();

    /* Configure SDIO CLK pin */
    GPIO_InitStruct.Pin = SDCARD_SDIO_CLK_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
    HAL_GPIO_Init(SDCARD_SDIO_CLK_GPIO_PORT, &GPIO_InitStruct);

    /* Configure SDIO CMD pin */
    GPIO_InitStruct.Pin = SDCARD_SDIO_CMD_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
    HAL_GPIO_Init(SDCARD_SDIO_CMD_GPIO_PORT, &GPIO_InitStruct);

    /* Configure SDIO Data pins (D0-D3) */
    GPIO_InitStruct.Pin = SDCARD_SDIO_D0_PIN | SDCARD_SDIO_D1_PIN |
                          SDCARD_SDIO_D2_PIN | SDCARD_SDIO_D3_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
    HAL_GPIO_Init(SDCARD_SDIO_D_GPIO_PORT, &GPIO_InitStruct);

    /* Configure Card Detect pin */
    GPIO_InitStruct.Pin = SDCARD_DETECT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = 0;
    HAL_GPIO_Init(SDCARD_DETECT_GPIO_PORT, &GPIO_InitStruct);

    /* Configure Write Protect pin */
    GPIO_InitStruct.Pin = SDCARD_WP_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = 0;
    HAL_GPIO_Init(SDCARD_WP_GPIO_PORT, &GPIO_InitStruct);

    /* Configure SDIO interrupt */
    HAL_NVIC_SetPriority(SDIO_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(SDIO_IRQn);
}

/**
 * @brief  Deinitialize SD Card MSP
 * @retval None
 */
static void SDCARD_MspDeInit(void)
{
    /* Disable SDIO interrupt */
    HAL_NVIC_DisableIRQ(SDIO_IRQn);

    /* Deinitialize GPIO pins */
    HAL_GPIO_DeInit(SDCARD_SDIO_CLK_GPIO_PORT, SDCARD_SDIO_CLK_PIN);
    HAL_GPIO_DeInit(SDCARD_SDIO_CMD_GPIO_PORT, SDCARD_SDIO_CMD_PIN);
    HAL_GPIO_DeInit(SDCARD_SDIO_D_GPIO_PORT, SDCARD_SDIO_D0_PIN | SDCARD_SDIO_D1_PIN |
                                             SDCARD_SDIO_D2_PIN | SDCARD_SDIO_D3_PIN);
    HAL_GPIO_DeInit(SDCARD_DETECT_GPIO_PORT, SDCARD_DETECT_PIN);
    HAL_GPIO_DeInit(SDCARD_WP_GPIO_PORT, SDCARD_WP_PIN);

    /* Disable peripheral clock */
    __HAL_RCC_SDIO_CLK_DISABLE();
}

/**
 * @brief  Convert HAL status to SD Card status
 * @param  halStatus: HAL status
 * @retval SDCARD_StatusTypeDef: Corresponding SD Card status
 */
static SDCARD_StatusTypeDef SDCARD_HALStatusToSDStatus(HAL_StatusTypeDef halStatus)
{
    switch (halStatus)
    {
        case HAL_OK:
            return SDCARD_STATUS_OK;
        case HAL_ERROR:
            return SDCARD_STATUS_ERROR;
        case HAL_BUSY:
            return SDCARD_STATUS_BUSY;
        case HAL_TIMEOUT:
            return SDCARD_STATUS_TIMEOUT;
        default:
            return SDCARD_STATUS_ERROR;
    }
}

/**
 * @brief  Configure DMA for SDIO
 * @retval None
 */
static void SDCARD_DMA_Config(void)
{
    /* Enable DMA clock */
    SDCARD_DMA_CLK_ENABLE();

    /* Configure DMA handle */
    hdma_sdio.Instance = SDCARD_DMA_STREAM;
    hdma_sdio.Init.Channel = SDCARD_DMA_CHANNEL;
    hdma_sdio.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_sdio.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_sdio.Init.MemInc = DMA_MINC_ENABLE;
    hdma_sdio.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_sdio.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_sdio.Init.Mode = DMA_PFCTRL;
    hdma_sdio.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_sdio.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_sdio.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma_sdio.Init.MemBurst = DMA_MBURST_INC4;
    hdma_sdio.Init.PeriphBurst = DMA_PBURST_INC4;

    /* Initialize DMA */
    HAL_DMA_Init(&hdma_sdio);

    /* Associate DMA handle with SDIO handle */
    __HAL_LINKDMA(&hsd, hdmarx, hdma_sdio);
    __HAL_LINKDMA(&hsd, hdmatx, hdma_sdio);

    /* Configure DMA interrupt */
    HAL_NVIC_SetPriority(SDCARD_DMA_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(SDCARD_DMA_IRQn);
}

/**
 * @brief  SDIO interrupt handler
 * @retval None
 */
void SDIO_IRQHandler(void)
{
    HAL_SD_IRQHandler(&hsd);
}

/**
 * @brief  DMA interrupt handler
 * @retval None
 */
void DMA2_Stream3_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_sdio);
}

/**
 * @brief  HAL SD Card Tx complete callback
 * @param  hsd: SD handle
 * @retval None
 */
void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd)
{
    UNUSED(hsd);
    SDCARD_TxCpltCallback();
}

/**
 * @brief  HAL SD Card Rx complete callback
 * @param  hsd: SD handle
 * @retval None
 */
void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{
    UNUSED(hsd);
    SDCARD_RxCpltCallback();
}

/**
 * @brief  HAL SD Card error callback
 * @param  hsd: SD handle
 * @retval None
 */
void HAL_SD_ErrorCallback(SD_HandleTypeDef *hsd)
{
    UNUSED(hsd);
    SDCARD_ErrorCallback();
}
