/**
 * @file fmc.h
 * @brief Flexible Memory Controller (FMC) Driver Header
 * @author Generated for STM32F429
 * @date 2025
 *
 * This FMC driver provides a comprehensive interface for external memory operations
 * on STM32F4 series, supporting SDRAM, NOR Flash, and NAND Flash memories.
 * Built on top of STM32 HAL library following best practices.
 */

#ifndef FMC_H
#define FMC_H

#include "stm32f4xx.h"
#include "stm32f4xx_hal_sdram.h"
#include "stm32f4xx_hal_sram.h"
#include "stm32f4xx_hal_nand.h"
#include <stdbool.h>
#include <stdint.h>

/* FMC Memory Types */
#define FMC_DRIVER_MEMORY_SDRAM     0x01U
#define FMC_DRIVER_MEMORY_NOR       0x02U
#define FMC_DRIVER_MEMORY_NAND      0x04U

/* FMC Driver Error Codes */
#define FMC_DRIVER_ERROR_NONE       0x00U
#define FMC_DRIVER_ERROR_INIT       0x01U
#define FMC_DRIVER_ERROR_CONFIG     0x02U
#define FMC_DRIVER_ERROR_OPERATION  0x04U

#define FMC_SDRAM_TIMEOUT       0x1000
#define FMC_NOR_TIMEOUT         1000U
#define FMC_NAND_TIMEOUT        1000U


/* Memory layout */
#define SDRAM_DEVICE_ADDR         ((uint32_t)0xD0000000)
#define SDRAM_DEVICE_SIZE         ((uint32_t)0x800000)  /* 8 MBytes */

/* SDRAM configuration defaults */
#define SDRAM_TIMEOUT            ((uint32_t)0xFFFF)
#define SDRAM_REFRESH_COUNT      ((uint32_t)1292)

/* FMC SDRAM Mode definition register defines (minimal subset used by driver) */
#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)

/* SDRAM Configuration Structure - follows HAL best practices */
typedef struct {
    uint32_t bank;                    /* FMC_SDRAM_BANK1 or FMC_SDRAM_BANK2 */
    uint32_t columnBits;              /* Column address bits */
    uint32_t rowBits;                 /* Row address bits */
    uint32_t dataWidth;               /* Memory data width */
    uint32_t internalBanks;           /* Number of internal banks */
    uint32_t casLatency;              /* CAS latency */
    uint32_t clockPeriod;             /* SDRAM clock period */
    uint32_t readBurst;               /* Read burst enable/disable */
    uint32_t readPipeDelay;           /* Read pipe delay */
    uint32_t writeProtection;         /* Write protection */
    /* Timing parameters */
    uint32_t loadToActiveDelay;       /* tMRD */
    uint32_t exitSelfRefreshDelay;    /* tXSR */
    uint32_t selfRefreshTime;         /* tRAS */
    uint32_t rowCycleDelay;           /* tRC */
    uint32_t writeRecoveryTime;       /* tWR */
    uint32_t rpDelay;                 /* tRP */
    uint32_t rcdDelay;                /* tRCD */
} FMC_Driver_SDRAM_Config_t;

/* NOR Flash Configuration Structure - follows HAL best practices */
typedef struct {
    uint32_t bank;                    /* FMC_NORSRAM_BANK1 to FMC_NORSRAM_BANK4 */
    uint32_t dataAddressMux;          /* Data/Address multiplexing */
    uint32_t memoryType;              /* Memory type */
    uint32_t memoryDataWidth;         /* Data width */
    uint32_t burstAccessMode;         /* Burst access mode */
    uint32_t waitSignalPolarity;      /* Wait signal polarity */
    uint32_t waitSignalActive;        /* Wait signal active edge */
    uint32_t writeOperation;          /* Write operation enable */
    uint32_t waitSignal;              /* Wait signal enable */
    uint32_t extendedMode;            /* Extended mode enable */
    uint32_t asynchronousWait;        /* Asynchronous wait */
    uint32_t writeBurst;              /* Write burst enable */
    uint32_t continuousClock;         /* Continuous clock */
    uint32_t writeFifo;               /* Write FIFO */
    uint32_t pageSize;                /* Page size */
    /* Timing parameters */
    uint32_t addressSetupTime;        /* Address setup time */
    uint32_t addressHoldTime;         /* Address hold time */
    uint32_t dataSetupTime;           /* Data setup time */
    uint32_t busTurnAroundDuration;   /* Bus turn around duration */
    uint32_t clkDivision;             /* Clock division */
    uint32_t dataLatency;             /* Data latency */
    uint32_t accessMode;              /* Access mode */
} FMC_Driver_NOR_Config_t;

/* NAND Flash Configuration Structure - follows HAL best practices */
typedef struct {
    uint32_t bank;                    /* FMC_NAND_BANK2 or FMC_NAND_BANK3 */
    uint32_t waitFeature;             /* Wait feature enable */
    uint32_t memoryDataWidth;         /* Data width */
    uint32_t eccComputation;          /* ECC computation */
    uint32_t eccPageSize;             /* ECC page size */
    uint32_t tadl;                    /* ALE to data start time */
    uint32_t thold;                   /* ALE to RE/WE delay */
    uint32_t twait;                   /* Ready to RE delay */
    uint32_t tset;                    /* WE to RE delay */
} FMC_Driver_NAND_Config_t;

/* FMC Driver Handle Structure - follows HAL best practices */
typedef struct {
    uint32_t memoryType;                      /* Type of memory (SDRAM/NOR/NAND) */
    SDRAM_HandleTypeDef hsdram;               /* SDRAM handle */
    SRAM_HandleTypeDef hsram;                 /* SRAM handle (for NOR Flash) */
    NAND_HandleTypeDef hnand;                 /* NAND handle */
    FMC_Driver_SDRAM_Config_t sdramConfig;    /* SDRAM configuration */
    FMC_Driver_NOR_Config_t norConfig;        /* NOR configuration */
    FMC_Driver_NAND_Config_t nandConfig;      /* NAND configuration */
    bool initialized;                         /* Initialization status */
    uint32_t errorCode;                       /* Last error code */
} FMC_Driver_Handle_t;

/* Function Prototypes - renamed to avoid HAL conflicts */

/**
 * @brief Initialize FMC driver with SDRAM configuration
 * @param handle: Pointer to FMC driver handle
 * @param config: Pointer to SDRAM configuration
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef FMC_Driver_SDRAM_Init(FMC_Driver_Handle_t *handle, FMC_Driver_SDRAM_Config_t *config);

/**
 * @brief Initialize FMC driver with NOR Flash configuration
 * @param handle: Pointer to FMC driver handle
 * @param config: Pointer to NOR configuration
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef FMC_Driver_NOR_Init(FMC_Driver_Handle_t *handle, FMC_Driver_NOR_Config_t *config);

/**
 * @brief Initialize FMC driver with NAND Flash configuration
 * @param handle: Pointer to FMC driver handle
 * @param config: Pointer to NAND configuration
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef FMC_Driver_NAND_Init(FMC_Driver_Handle_t *handle, FMC_Driver_NAND_Config_t *config);

/**
 * @brief Deinitialize FMC driver
 * @param handle: Pointer to FMC driver handle
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef FMC_Driver_DeInit(FMC_Driver_Handle_t *handle);

/**
 * @brief SDRAM memory test
 * @param handle: Pointer to FMC driver handle
 * @param startAddr: Start address for test
 * @param size: Size of memory to test
 * @return bool: True if test passed, false otherwise
 */
bool FMC_Driver_SDRAM_Test(FMC_Driver_Handle_t *handle, uint32_t startAddr, uint32_t size);

/**
 * @brief Write data to SDRAM
 * @param handle: Pointer to FMC driver handle
 * @param address: Memory address
 * @param data: Pointer to data buffer
 * @param size: Size of data in bytes
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef FMC_Driver_SDRAM_Write(FMC_Driver_Handle_t *handle, uint32_t address, const uint8_t *data, uint32_t size);

/**
 * @brief Read data from SDRAM
 * @param handle: Pointer to FMC driver handle
 * @param address: Memory address
 * @param data: Pointer to data buffer
 * @param size: Size of data in bytes
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef FMC_Driver_SDRAM_Read(FMC_Driver_Handle_t *handle, uint32_t address, uint8_t *data, uint32_t size);

/**
 * @brief Write data to NOR Flash
 * @param handle: Pointer to FMC driver handle
 * @param address: Memory address
 * @param data: Pointer to data buffer
 * @param size: Size of data in bytes
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef FMC_Driver_NOR_Write(FMC_Driver_Handle_t *handle, uint32_t address, const uint8_t *data, uint32_t size);

/**
 * @brief Read data from NOR Flash
 * @param handle: Pointer to FMC driver handle
 * @param address: Memory address
 * @param data: Pointer to data buffer
 * @param size: Size of data in bytes
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef FMC_Driver_NOR_Read(FMC_Driver_Handle_t *handle, uint32_t address, uint8_t *data, uint32_t size);

/**
 * @brief Erase NOR Flash sector
 * @param handle: Pointer to FMC driver handle
 * @param address: Sector address
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef FMC_Driver_NOR_EraseSector(FMC_Driver_Handle_t *handle, uint32_t address);

/**
 * @brief Write data to NAND Flash
 * @param handle: Pointer to FMC driver handle
 * @param address: Memory address
 * @param data: Pointer to data buffer
 * @param size: Size of data in bytes
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef FMC_Driver_NAND_Write(FMC_Driver_Handle_t *handle, uint32_t address, const uint8_t *data, uint32_t size);

/**
 * @brief Read data from NAND Flash
 * @param handle: Pointer to FMC driver handle
 * @param address: Memory address
 * @param data: Pointer to data buffer
 * @param size: Size of data in bytes
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef FMC_Driver_NAND_Read(FMC_Driver_Handle_t *handle, uint32_t address, uint8_t *data, uint32_t size);

/**
 * @brief Erase NAND Flash block
 * @param handle: Pointer to FMC driver handle
 * @param address: Block address
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef FMC_Driver_NAND_EraseBlock(FMC_Driver_Handle_t *handle, uint32_t address);

/**
 * @brief Get FMC driver error status
 * @param handle: Pointer to FMC driver handle
 * @return uint32_t: Error code
 */
uint32_t FMC_Driver_GetError(FMC_Driver_Handle_t *handle);

/* Legacy function for backward compatibility */
void FMC_Init(void);

#endif /* FMC_H */
