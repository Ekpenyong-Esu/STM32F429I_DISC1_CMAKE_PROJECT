/**
 * @file fmc_example.c
 * @brief FMC Driver Example Implementation
 * @author Generated for STM32F429
 * @date 2025
 *
 * This file provides examples of how to use the FMC driver for different memory types.
 * Updated to use corrected API following HAL best practices.
 */

#include "fmc.h"
#include "fmc_example.h"
#include <stdio.h>

/* Private defines */
#define SDRAM_BASE_ADDRESS    0xC0000000U
#define NOR_BASE_ADDRESS      0x60000000U
#define NAND_BASE_ADDRESS     0x70000000U
#define TEST_DATA_SIZE        32
#define TEST_SIZE_1KB         1024

/* SDRAM Configuration */
static FMC_Driver_SDRAM_Config_t sdramConfig = {
    .bank = FMC_SDRAM_BANK1,
    .columnBits = FMC_SDRAM_COLUMN_BITS_NUM_8,
    .rowBits = FMC_SDRAM_ROW_BITS_NUM_11,
    .dataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16,
    .internalBanks = FMC_SDRAM_INTERN_BANKS_NUM_4,
    .casLatency = FMC_SDRAM_CAS_LATENCY_3,
    .clockPeriod = FMC_SDRAM_CLOCK_PERIOD_2,
    .readBurst = FMC_SDRAM_RBURST_ENABLE,
    .readPipeDelay = FMC_SDRAM_RPIPE_DELAY_0,
    .writeProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE,
    .loadToActiveDelay = 2,
    .exitSelfRefreshDelay = 7,
    .selfRefreshTime = 4,
    .rowCycleDelay = 7,
    .writeRecoveryTime = 2,
    .rpDelay = 2,
    .rcdDelay = 2
};

/* NOR Flash Configuration */
static FMC_Driver_NOR_Config_t norConfig = {
    .bank = FMC_NORSRAM_BANK1,
    .dataAddressMux = FMC_DATA_ADDRESS_MUX_DISABLE,
    .memoryType = FMC_MEMORY_TYPE_NOR,
    .memoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_16,
    .burstAccessMode = FMC_BURST_ACCESS_MODE_DISABLE,
    .waitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW,
    .waitSignalActive = FMC_WAIT_TIMING_BEFORE_WS,
    .writeOperation = FMC_WRITE_OPERATION_ENABLE,
    .waitSignal = FMC_WAIT_SIGNAL_DISABLE,
    .extendedMode = FMC_EXTENDED_MODE_DISABLE,
    .asynchronousWait = FMC_ASYNCHRONOUS_WAIT_DISABLE,
    .writeBurst = FMC_WRITE_BURST_DISABLE,
    .continuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ONLY,
    .writeFifo = 0,  /* Write FIFO disabled */
    .pageSize = FMC_PAGE_SIZE_NONE,
    .addressSetupTime = 2,
    .addressHoldTime = 1,
    .dataSetupTime = 3,
    .busTurnAroundDuration = 1,
    .clkDivision = 2,
    .dataLatency = 2,
    .accessMode = FMC_ACCESS_MODE_A
};

/* NAND Flash Configuration */
static FMC_Driver_NAND_Config_t nandConfig = {
    .bank = FMC_NAND_BANK2,
    .waitFeature = FMC_NAND_PCC_WAIT_FEATURE_DISABLE,
    .memoryDataWidth = FMC_NAND_PCC_MEM_BUS_WIDTH_8,
    .eccComputation = FMC_NAND_ECC_DISABLE,
    .eccPageSize = FMC_NAND_ECC_PAGE_SIZE_256BYTE,
    .tadl = 0,
    .thold = 1,
    .twait = 2,
    .tset = 1
};



/**
 * @brief SDRAM example function
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef FMC_Driver_SDRAM_Example(void) {
    FMC_Driver_Handle_t handle;
    HAL_StatusTypeDef status = HAL_OK;

    /* Initialize SDRAM */
    status = FMC_Driver_SDRAM_Init(&handle, &sdramConfig);
    if (status != HAL_OK) {
        printf("SDRAM initialization failed!\n");
        return status;
    }

    /* Test SDRAM */
    uint32_t testAddress = SDRAM_BASE_ADDRESS;
    uint32_t testSize = TEST_SIZE_1KB;

    if (FMC_Driver_SDRAM_Test(&handle, testAddress, testSize)) {
        printf("SDRAM test passed!\n");
    } else {
        printf("SDRAM test failed!\n");
        return HAL_ERROR;
    }

    /* Write and read data */
    uint8_t writeData[TEST_DATA_SIZE] = "Hello SDRAM!";
    uint8_t readData[TEST_DATA_SIZE];

    status = FMC_Driver_SDRAM_Write(&handle, testAddress, writeData, sizeof(writeData));
    if (status != HAL_OK) {
        printf("SDRAM write failed!\n");
        return status;
    }

    status = FMC_Driver_SDRAM_Read(&handle, testAddress, readData, sizeof(readData));
    if (status != HAL_OK) {
        printf("SDRAM read failed!\n");
        return status;
    }

    printf("SDRAM data: %s\n", readData);

    /* Deinitialize */
    FMC_Driver_DeInit(&handle);

    return HAL_OK;
}

/**
 * @brief NOR Flash example function
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef FMC_Driver_NOR_Example(void) {
    FMC_Driver_Handle_t handle;
    HAL_StatusTypeDef status = HAL_OK;

    /* Initialize NOR Flash */
    status = FMC_Driver_NOR_Init(&handle, &norConfig);
    if (status != HAL_OK) {
        printf("NOR Flash initialization failed!\n");
        return status;
    }

    /* Write and read data */
    uint32_t testAddress = NOR_BASE_ADDRESS;
    uint8_t writeData[TEST_DATA_SIZE] = "Hello NOR!";
    uint8_t readData[TEST_DATA_SIZE];

    status = FMC_Driver_NOR_Write(&handle, testAddress, writeData, sizeof(writeData));
    if (status != HAL_OK) {
        printf("NOR Flash write failed!\n");
        return status;
    }

    status = FMC_Driver_NOR_Read(&handle, testAddress, readData, sizeof(readData));
    if (status != HAL_OK) {
        printf("NOR Flash read failed!\n");
        return status;
    }

    printf("NOR Flash data: %s\n", readData);

    /* Erase sector */
    status = FMC_Driver_NOR_EraseSector(&handle, testAddress);
    if (status != HAL_OK) {
        printf("NOR Flash erase failed!\n");
        return status;
    }

    /* Deinitialize */
    FMC_Driver_DeInit(&handle);

    return HAL_OK;
}

/**
 * @brief NAND Flash example function
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef FMC_Driver_NAND_Example(void) {
    FMC_Driver_Handle_t handle;
    HAL_StatusTypeDef status = HAL_OK;

    /* Initialize NAND Flash */
    status = FMC_Driver_NAND_Init(&handle, &nandConfig);
    if (status != HAL_OK) {
        printf("NAND Flash initialization failed!\n");
        return status;
    }

    /* Write and read data */
    uint32_t testAddress = NAND_BASE_ADDRESS;
    uint8_t writeData[TEST_DATA_SIZE] = "Hello NAND!";
    uint8_t readData[TEST_DATA_SIZE];

    status = FMC_Driver_NAND_Write(&handle, testAddress, writeData, sizeof(writeData));
    if (status != HAL_OK) {
        printf("NAND Flash write failed!\n");
        return status;
    }

    status = FMC_Driver_NAND_Read(&handle, testAddress, readData, sizeof(readData));
    if (status != HAL_OK) {
        printf("NAND Flash read failed!\n");
        return status;
    }

    printf("NAND Flash data: %s\n", readData);

    /* Erase block */
    status = FMC_Driver_NAND_EraseBlock(&handle, testAddress);
    if (status != HAL_OK) {
        printf("NAND Flash erase failed!\n");
        return status;
    }

    /* Deinitialize */
    FMC_Driver_DeInit(&handle);

    return HAL_OK;
}

/**
 * @brief Main FMC example function
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef FMC_Driver_Example(void) {
    HAL_StatusTypeDef status = HAL_OK;

    printf("Starting FMC Driver Examples...\n");

    /* SDRAM Example */
    printf("\n--- SDRAM Example ---\n");
    status = FMC_Driver_SDRAM_Example();
    if (status != HAL_OK) {
        printf("SDRAM example failed!\n");
        return status;
    }

    /* NOR Flash Example */
    printf("\n--- NOR Flash Example ---\n");
    status = FMC_Driver_NOR_Example();
    if (status != HAL_OK) {
        printf("NOR Flash example failed!\n");
        return status;
    }

    /* NAND Flash Example */
    printf("\n--- NAND Flash Example ---\n");
    status = FMC_Driver_NAND_Example();
    if (status != HAL_OK) {
        printf("NAND Flash example failed!\n");
        return status;
    }

    printf("\nAll FMC examples completed successfully!\n");
    return HAL_OK;
}
