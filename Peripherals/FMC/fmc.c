/**
 * @file fmc.c
 * @brief Flexible Memory Controller (FMC) Driver Implementation
 * @author Generated for STM32F429
 * @date 2025
 *
 * This FMC driver provides implementation for external memory operations
 * on STM32F4 series, supporting SDRAM, NOR Flash, and NAND Flash memories.
 * Built on top of STM32 HAL library following best practices.
 */

#include "fmc.h"
#include <string.h>
#include "log.h"

/* Private defines -----------------------------------------------------------*/



/* Private function prototypes -----------------------------------------------*/

/*=============================================================================
 * SDRAM Functions
 *===========================================================================*/

/**
 * @brief Initialize FMC driver with SDRAM configuration
 */
HAL_StatusTypeDef FMC_Driver_SDRAM_Init(FMC_Driver_Handle_t *handle, FMC_Driver_SDRAM_Config_t *config) {
    log_debug("FMC: Initializing SDRAM");
    if (handle == NULL || config == NULL) {
        log_error("FMC: Invalid SDRAM init parameters");
        return HAL_ERROR;
    }

    /* Clear the handle to avoid using uninitialized fields */
    memset(handle, 0, sizeof(FMC_Driver_Handle_t));

    /* Configure SDRAM device instance */
    handle->hsdram.Instance = FMC_SDRAM_DEVICE;
    handle->hsdram.Init.SDBank = config->bank;
    handle->hsdram.Init.ColumnBitsNumber = config->columnBits;
    handle->hsdram.Init.RowBitsNumber = config->rowBits;
    handle->hsdram.Init.MemoryDataWidth = config->dataWidth;
    handle->hsdram.Init.InternalBankNumber = config->internalBanks;
    handle->hsdram.Init.CASLatency = config->casLatency;
    handle->hsdram.Init.WriteProtection = config->writeProtection;
    handle->hsdram.Init.SDClockPeriod = config->clockPeriod;
    handle->hsdram.Init.ReadBurst = config->readBurst;
    handle->hsdram.Init.ReadPipeDelay = config->readPipeDelay;

    /* SDRAM timing configuration */
    FMC_SDRAM_TimingTypeDef timing;
    timing.LoadToActiveDelay = config->loadToActiveDelay;
    timing.ExitSelfRefreshDelay = config->exitSelfRefreshDelay;
    timing.SelfRefreshTime = config->selfRefreshTime;
    timing.RowCycleDelay = config->rowCycleDelay;
    timing.WriteRecoveryTime = config->writeRecoveryTime;
    timing.RPDelay = config->rpDelay;
    timing.RCDDelay = config->rcdDelay;

    /* Initialize SDRAM controller */
    if (HAL_SDRAM_Init(&handle->hsdram, &timing) != HAL_OK) {
        handle->errorCode = FMC_DRIVER_ERROR_INIT;
        return HAL_ERROR;
    }

    /* SDRAM initialization sequence */
    FMC_SDRAM_CommandTypeDef command;
    uint32_t commandTarget = 0;

    /* Map Bank to Command Target */
    if (config->bank == FMC_SDRAM_BANK1) {
        commandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    } else {
        commandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
    }

    /* Step 1: Configure clock enable command */
    command.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
    command.CommandTarget = commandTarget;
    command.AutoRefreshNumber = 1;
    command.ModeRegisterDefinition = 0;
    if (HAL_SDRAM_SendCommand(&handle->hsdram, &command, FMC_SDRAM_TIMEOUT) != HAL_OK) {
        handle->errorCode = FMC_DRIVER_ERROR_CONFIG;
        return HAL_ERROR;
    }

    /* Delay (100µs minimum) */
    HAL_Delay(100);

    /* Step 2: Configure PALL (precharge all) command */
    command.CommandMode = FMC_SDRAM_CMD_PALL;
    command.CommandTarget = commandTarget;
    command.AutoRefreshNumber = 1;
    command.ModeRegisterDefinition = 0;
    if (HAL_SDRAM_SendCommand(&handle->hsdram, &command, FMC_SDRAM_TIMEOUT) != HAL_OK) {
        handle->errorCode = FMC_DRIVER_ERROR_CONFIG;
        return HAL_ERROR;
    }

    /* Step 3: Configure auto-refresh command */
    command.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
    command.CommandTarget = commandTarget;
    command.AutoRefreshNumber = 4;
    command.ModeRegisterDefinition = 0;
    if (HAL_SDRAM_SendCommand(&handle->hsdram, &command, FMC_SDRAM_TIMEOUT) != HAL_OK) {
        handle->errorCode = FMC_DRIVER_ERROR_CONFIG;
        return HAL_ERROR;
    }

    /* Step 4: Program mode register */
    volatile uint32_t modeReg = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_2 |
                       SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL |
                       (config->casLatency == FMC_SDRAM_CAS_LATENCY_2 ? SDRAM_MODEREG_CAS_LATENCY_2 : SDRAM_MODEREG_CAS_LATENCY_3) |
                       SDRAM_MODEREG_OPERATING_MODE_STANDARD |
                       SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

    command.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
    command.CommandTarget = commandTarget;
    command.AutoRefreshNumber = 1;
    command.ModeRegisterDefinition = modeReg;
    if (HAL_SDRAM_SendCommand(&handle->hsdram, &command, FMC_SDRAM_TIMEOUT) != HAL_OK) {
        handle->errorCode = FMC_DRIVER_ERROR_CONFIG;
        return HAL_ERROR;
    }

    /* Step 5: Set refresh rate (64ms / 4096 rows = 15.625µs per row)
     * Refresh rate = (15.625µs × SDRAM_CLK) - 20
     * For 84MHz SDRAM_CLK: (15.625µs × 84MHz) - 20 = 1292 */
    if (HAL_SDRAM_ProgramRefreshRate(&handle->hsdram, 1292) != HAL_OK) {
        handle->errorCode = FMC_DRIVER_ERROR_CONFIG;
        return HAL_ERROR;
    }

    /* Update handle state */
    handle->memoryType = FMC_DRIVER_MEMORY_SDRAM;
    handle->sdramConfig = *config;
    handle->initialized = true;
    handle->errorCode = FMC_DRIVER_ERROR_NONE;

    log_debug("FMC: SDRAM initialized successfully");
    return HAL_OK;
}

/**
 * @brief Write data to SDRAM
 */
HAL_StatusTypeDef FMC_Driver_SDRAM_Write(FMC_Driver_Handle_t *handle, uint32_t address, const uint8_t *data, uint32_t size) {
    if (handle == NULL || data == NULL || !handle->initialized) {
        return HAL_ERROR;
    }

    /* Use DMA for large transfers, direct write for small */
    if (size >= 32) {
        return HAL_SDRAM_Write_DMA(&handle->hsdram, (uint32_t *)address, (uint32_t *)data, size / 4);
    } else {
        /* Direct memory write */
        memcpy((void *)address, data, size);
        return HAL_OK;
    }
}

/**
 * @brief Read data from SDRAM
 */
HAL_StatusTypeDef FMC_Driver_SDRAM_Read(FMC_Driver_Handle_t *handle, uint32_t address, uint8_t *data, uint32_t size) {
    if (handle == NULL || data == NULL || !handle->initialized) {
        return HAL_ERROR;
    }

    /* Use DMA for large transfers, direct read for small */
    if (size >= 32) {
        return HAL_SDRAM_Read_DMA(&handle->hsdram, (uint32_t *)address, (uint32_t *)data, size / 4);
    } else {
        /* Direct memory read */
        memcpy(data, (void *)address, size);
        return HAL_OK;
    }
}

/**
 * @brief Test SDRAM memory
 */
bool FMC_Driver_SDRAM_Test(FMC_Driver_Handle_t *handle, uint32_t startAddr, uint32_t size) {
    if (handle == NULL || !handle->initialized) {
        return false;
    }

    /* Pattern test using 16-bit accesses (SDRAM is 16-bit wide) */
    uint16_t *pMem = (uint16_t *)startAddr;
    uint32_t numWords = size / 2;

    /* Write test pattern */
    for (uint32_t i = 0; i < numWords; i++) {
        pMem[i] = 0x55AA;
    }

    /* Read and verify */
    for (uint32_t i = 0; i < numWords; i++) {
        if (pMem[i] != 0x55AA) {
            return false;
        }
    }

    /* Inverse pattern test */
    for (uint32_t i = 0; i < numWords; i++) {
        pMem[i] = 0xAA55;
    }

    for (uint32_t i = 0; i < numWords; i++) {
        if (pMem[i] != 0xAA55) {
            return false;
        }
    }

    return true;
}

/*=============================================================================
 * NOR Flash Functions
 *===========================================================================*/

/**
 * @brief Initialize FMC driver with NOR Flash configuration
 */
HAL_StatusTypeDef FMC_Driver_NOR_Init(FMC_Driver_Handle_t *handle, FMC_Driver_NOR_Config_t *config) {
    if (handle == NULL || config == NULL) {
        return HAL_ERROR;
    }


    /* Configure NOR Flash device */
    handle->hsram.Instance = FMC_NORSRAM_DEVICE;
    handle->hsram.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
    handle->hsram.Init.NSBank = config->bank;
    handle->hsram.Init.DataAddressMux = config->dataAddressMux;
    handle->hsram.Init.MemoryType = config->memoryType;
    handle->hsram.Init.MemoryDataWidth = config->memoryDataWidth;
    handle->hsram.Init.BurstAccessMode = config->burstAccessMode;
    handle->hsram.Init.WaitSignalPolarity = config->waitSignalPolarity;
    handle->hsram.Init.WaitSignalActive = config->waitSignalActive;
    handle->hsram.Init.WriteOperation = config->writeOperation;
    handle->hsram.Init.WaitSignal = config->waitSignal;
    handle->hsram.Init.ExtendedMode = config->extendedMode;
    handle->hsram.Init.AsynchronousWait = config->asynchronousWait;
    handle->hsram.Init.WriteBurst = config->writeBurst;
    handle->hsram.Init.ContinuousClock = config->continuousClock;
    handle->hsram.Init.WriteFifo = config->writeFifo;
    handle->hsram.Init.PageSize = config->pageSize;

    /* Timing configuration */
    FMC_NORSRAM_TimingTypeDef timing;
    timing.AddressSetupTime = config->addressSetupTime;
    timing.AddressHoldTime = config->addressHoldTime;
    timing.DataSetupTime = config->dataSetupTime;
    timing.BusTurnAroundDuration = config->busTurnAroundDuration;
    timing.CLKDivision = config->clkDivision;
    timing.DataLatency = config->dataLatency;
    timing.AccessMode = config->accessMode;

    /* Initialize NOR controller */
    if (HAL_SRAM_Init(&handle->hsram, &timing, NULL) != HAL_OK) {
        handle->errorCode = FMC_DRIVER_ERROR_INIT;
        return HAL_ERROR;
    }

    /* Update handle state */
    handle->memoryType = FMC_DRIVER_MEMORY_NOR;
    handle->norConfig = *config;
    handle->initialized = true;
    handle->errorCode = FMC_DRIVER_ERROR_NONE;

    return HAL_OK;
}

/**
 * @brief Write data to NOR Flash
 */
HAL_StatusTypeDef FMC_Driver_NOR_Write(FMC_Driver_Handle_t *handle, uint32_t address, const uint8_t *data, uint32_t size) {
    if (handle == NULL || data == NULL || !handle->initialized) {
        return HAL_ERROR;
    }

    /* Write data using HAL SRAM functions */
    return HAL_SRAM_Write_16b(&handle->hsram, (uint32_t *)address, (uint16_t *)data, size / 2);
}

/**
 * @brief Read data from NOR Flash
 */
HAL_StatusTypeDef FMC_Driver_NOR_Read(FMC_Driver_Handle_t *handle, uint32_t address, uint8_t *data, uint32_t size) {
    if (handle == NULL || data == NULL || !handle->initialized) {
        return HAL_ERROR;
    }

    /* Read data using HAL SRAM functions */
    return HAL_SRAM_Read_16b(&handle->hsram, (uint32_t *)address, (uint16_t *)data, size / 2);
}

/**
 * @brief Erase NOR Flash sector
 */
HAL_StatusTypeDef FMC_Driver_NOR_EraseSector(FMC_Driver_Handle_t *handle, uint32_t address) {
    if (handle == NULL || !handle->initialized) {
        return HAL_ERROR;
    }

    /* NOR Flash erase requires specific command sequence */
    /* This is device-specific and should be customized based on NOR chip */
    volatile uint16_t *pAddr = (uint16_t *)address;

    /* Standard AMD/Spansion erase sequence */
    *(volatile uint16_t *)((address & 0xFFFF0000) | 0x0AAA) = 0xAA;
    *(volatile uint16_t *)((address & 0xFFFF0000) | 0x0554) = 0x55;
    *(volatile uint16_t *)((address & 0xFFFF0000) | 0x0AAA) = 0x80;
    *(volatile uint16_t *)((address & 0xFFFF0000) | 0x0AAA) = 0xAA;
    *(volatile uint16_t *)((address & 0xFFFF0000) | 0x0554) = 0x55;
    *pAddr = 0x30;

    /* Wait for erase completion */
    uint32_t timeout = FMC_NOR_TIMEOUT;
    while ((*pAddr != 0xFFFF) && (timeout-- > 0)) {
        HAL_Delay(1);
    }

    return (timeout > 0) ? HAL_OK : HAL_TIMEOUT;
}

/*=============================================================================
 * NAND Flash Functions
 *===========================================================================*/

/**
 * @brief Initialize FMC driver with NAND Flash configuration
 */
HAL_StatusTypeDef FMC_Driver_NAND_Init(FMC_Driver_Handle_t *handle, FMC_Driver_NAND_Config_t *config) {
    if (handle == NULL || config == NULL) {
        return HAL_ERROR;
    }

    /* Enable FMC clock */
    __HAL_RCC_FMC_CLK_ENABLE();

    /* Configure NAND Flash device */
    handle->hnand.Instance = FMC_NAND_DEVICE;
    handle->hnand.Init.NandBank = config->bank;
    handle->hnand.Init.Waitfeature = config->waitFeature;
    handle->hnand.Init.MemoryDataWidth = config->memoryDataWidth;
    handle->hnand.Init.EccComputation = config->eccComputation;
    handle->hnand.Init.ECCPageSize = config->eccPageSize;
    handle->hnand.Init.TCLRSetupTime = config->tadl;
    handle->hnand.Init.TARSetupTime = config->thold;

    /* Timing configuration */
    FMC_NAND_PCC_TimingTypeDef timing;
    timing.SetupTime = config->tset;
    timing.WaitSetupTime = config->twait;
    timing.HoldSetupTime = config->thold;
    timing.HiZSetupTime = config->tadl;

    /* Initialize NAND controller */
    if (HAL_NAND_Init(&handle->hnand, &timing, &timing) != HAL_OK) {
        handle->errorCode = FMC_DRIVER_ERROR_INIT;
        return HAL_ERROR;
    }

    /* Update handle state */
    handle->memoryType = FMC_DRIVER_MEMORY_NAND;
    handle->nandConfig = *config;
    handle->initialized = true;
    handle->errorCode = FMC_DRIVER_ERROR_NONE;

    return HAL_OK;
}

/**
 * @brief Write data to NAND Flash
 */
HAL_StatusTypeDef FMC_Driver_NAND_Write(FMC_Driver_Handle_t *handle, uint32_t address, const uint8_t *data, uint32_t size) {
    if (handle == NULL || data == NULL || !handle->initialized) {
        return HAL_ERROR;
    }

    /* Convert address to NAND address structure */
    NAND_AddressTypeDef nandAddr;
    nandAddr.Page = address & 0xFF;
    nandAddr.Plane = 0;
    nandAddr.Block = (address >> 8) & 0xFF;

    /* Write page */
    (void)size; /* Unused in page-based operation */
    return HAL_NAND_Write_Page_8b(&handle->hnand, &nandAddr, (uint8_t *)data, 1);
}

/**
 * @brief Read data from NAND Flash
 */
HAL_StatusTypeDef FMC_Driver_NAND_Read(FMC_Driver_Handle_t *handle, uint32_t address, uint8_t *data, uint32_t size) {
    if (handle == NULL || data == NULL || !handle->initialized) {
        return HAL_ERROR;
    }

    /* Convert address to NAND address structure */
    NAND_AddressTypeDef nandAddr;
    nandAddr.Page = address & 0xFF;
    nandAddr.Plane = 0;
    nandAddr.Block = (address >> 8) & 0xFF;

    /* Read page */
    (void)size; /* Unused in page-based operation */
    return HAL_NAND_Read_Page_8b(&handle->hnand, &nandAddr, data, 1);
}

/**
 * @brief Erase NAND Flash block
 */
HAL_StatusTypeDef FMC_Driver_NAND_EraseBlock(FMC_Driver_Handle_t *handle, uint32_t address) {
    if (handle == NULL || !handle->initialized) {
        return HAL_ERROR;
    }

    /* Convert address to NAND address structure */
    NAND_AddressTypeDef nandAddr;
    nandAddr.Page = 0;
    nandAddr.Plane = 0;
    nandAddr.Block = (address >> 8) & 0xFF;

    /* Erase block */
    return HAL_NAND_Erase_Block(&handle->hnand, &nandAddr);
}

/*=============================================================================
 * Common Functions
 *===========================================================================*/

/**
 * @brief Deinitialize FMC driver
 */
HAL_StatusTypeDef FMC_Driver_DeInit(FMC_Driver_Handle_t *handle) {
    if (handle == NULL || !handle->initialized) {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status = HAL_OK;

    /* Deinitialize based on memory type */
    if (handle->memoryType & FMC_DRIVER_MEMORY_SDRAM) {
        status = HAL_SDRAM_DeInit(&handle->hsdram);
    }
    if (handle->memoryType & FMC_DRIVER_MEMORY_NOR) {
        status = HAL_SRAM_DeInit(&handle->hsram);
    }
    if (handle->memoryType & FMC_DRIVER_MEMORY_NAND) {
        status = HAL_NAND_DeInit(&handle->hnand);
    }

    /* Clear handle */
    memset(handle, 0, sizeof(FMC_Driver_Handle_t));

    return status;
}

/**
 * @brief Get FMC driver error status
 */
uint32_t FMC_Driver_GetError(FMC_Driver_Handle_t *handle) {
    if (handle == NULL) {
        return FMC_DRIVER_ERROR_INIT;
    }
    return handle->errorCode;
}


