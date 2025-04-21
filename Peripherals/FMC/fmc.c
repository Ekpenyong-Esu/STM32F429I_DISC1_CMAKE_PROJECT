/**
  ******************************************************************************
  * @file    fmc.c
  * @brief   FMC module implementation
  * @details This file provides code for the configuration
  *          and initialization of the Flexible Memory Controller
  *          for external SDRAM operation.
  * @version 1.0
  * @date    2025-04-15
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "fmc.h"
#include "../SYS/sys.h"

/* Private variables ---------------------------------------------------------*/
/**
 * @brief   SDRAM handle structure
 * @details Used by HAL functions for SDRAM peripheral operations
 */
SDRAM_HandleTypeDef hsdram1;

/**
 * @brief   SDRAM timing parameters structure
 * @details Contains timing values for SDRAM operations
 */
FMC_SDRAM_TimingTypeDef SdramTiming;

/**
  * @brief  FMC Initialization Function
  * @details Configures the FMC controller for SDRAM operation with the following settings:
  *          - SDRAM bank: Bank 1
  *          - Column address bits: 8
  *          - Row address bits: 12
  *          - Memory data width: 16 bits
  *          - Internal bank number: 4
  *          - CAS latency: 3 cycles
  *          - Write protection: Disabled
  *          - SDRAM clock period: 2 HCLK cycles
  *          - Read burst: Disabled
  *          - Read pipe delay: 1 cycle
  *
  *          And the following timing parameters:
  *          - Load to active delay: 2 cycles
  *          - Exit self-refresh delay: 7 cycles
  *          - Self refresh time: 4 cycles
  *          - Row cycle delay: 7 cycles
  *          - Write recovery time: 3 cycles
  *          - RP delay: 2 cycles
  *          - RCD delay: 2 cycles
  *
  * @note   The SDRAM is typically used for LCD framebuffer and for
  *         large data buffers in applications requiring significant RAM
  * @param  None
  * @retval None
  */
void FMC_Init(void)
{
  /* FMC initialization function */
  FMC_SDRAM_TimingTypeDef SdramTiming = {0};

  /** Perform the SDRAM1 memory initialization sequence */
  hsdram1.Instance = FMC_SDRAM_DEVICE;                         /* Select FMC SDRAM device */

  /* Configure SDRAM basic parameters */
  hsdram1.Init.SDBank = FMC_SDRAM_BANK1;                       /* Use SDRAM bank 1 */
  hsdram1.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_8; /* 8-bit column addressing */
  hsdram1.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_12;      /* 12-bit row addressing */
  hsdram1.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16;   /* 16-bit data bus width */
  hsdram1.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4; /* 4 internal SDRAM banks */
  hsdram1.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_3;           /* CAS latency of 3 cycles */
  hsdram1.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE; /* Allow write access */
  hsdram1.Init.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2;       /* SDRAM clock = HCLK/2 */
  hsdram1.Init.ReadBurst = FMC_SDRAM_RBURST_DISABLE;           /* Read burst disabled */
  hsdram1.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_1;        /* 1 cycle delay after CAS latency */

  /* Configure SDRAM timing parameters */
  SdramTiming.LoadToActiveDelay = 2;     /* tMRD: 2 cycles from mode register to active */
  SdramTiming.ExitSelfRefreshDelay = 7;  /* tXSR: 7 cycles exit self-refresh to active */
  SdramTiming.SelfRefreshTime = 4;       /* tRAS: 4 cycles minimum self-refresh period */
  SdramTiming.RowCycleDelay = 7;         /* tRC: 7 cycles row cycle time */
  SdramTiming.WriteRecoveryTime = 3;     /* tWR: 3 cycles write recovery time */
  SdramTiming.RPDelay = 2;               /* tRP: 2 cycles precharge to active delay */
  SdramTiming.RCDDelay = 2;              /* tRCD: 2 cycles RAS to CAS delay */

  /* Initialize the SDRAM with the specified parameters */
  if (HAL_SDRAM_Init(&hsdram1, &SdramTiming) != HAL_OK)
  {
    Error_Handler();  /* Call error handler if initialization fails */
  }
}
