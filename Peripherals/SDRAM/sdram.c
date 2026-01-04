/* sdram.c - Simple SDRAM driver (adapted, minimal)
 * Uses HAL FMC SDRAM functions. Designed to be small and straightforward.
 */

#include "sdram.h"
#include "stm32f4xx_hal.h"
#include "../FMC/fmc.h"

/* Internal static handles - delegate to FMC driver */
static FMC_Driver_Handle_t fmc_handle;
static FMC_Driver_SDRAM_Config_t fmc_cfg;
static FMC_SDRAM_CommandTypeDef sdram_cmd;

/* Simple init: sets up handle + timing, calls MSP init, then HAL init + sequence */
SDRAM_StatusTypeDef SDRAM_Init(void)
{
    /* Populate FMC SDRAM config */
    fmc_cfg.bank = FMC_SDRAM_BANK2;
    fmc_cfg.columnBits = FMC_SDRAM_COLUMN_BITS_NUM_8;
    fmc_cfg.rowBits = FMC_SDRAM_ROW_BITS_NUM_12;
    fmc_cfg.dataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16;
    fmc_cfg.internalBanks = FMC_SDRAM_INTERN_BANKS_NUM_4;
    fmc_cfg.casLatency = FMC_SDRAM_CAS_LATENCY_3;
    fmc_cfg.writeProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
    fmc_cfg.clockPeriod = FMC_SDRAM_CLOCK_PERIOD_2;
    fmc_cfg.readBurst = FMC_SDRAM_RBURST_DISABLE;
    fmc_cfg.readPipeDelay = FMC_SDRAM_RPIPE_DELAY_1;

    /* Timing values are tuned for 90 MHz SD clock (example values) */
    fmc_cfg.loadToActiveDelay = 2;
    fmc_cfg.exitSelfRefreshDelay = 7;
    fmc_cfg.selfRefreshTime = 4;
    fmc_cfg.rowCycleDelay = 7;
    fmc_cfg.writeRecoveryTime = 2;
    fmc_cfg.rpDelay = 2;
    fmc_cfg.rcdDelay = 2;

    /* Allow application to configure clocks/GPIO/DMA before HAL init */
    SDRAM_MspInit(&fmc_handle.hsdram, NULL);

    if (FMC_Driver_SDRAM_Init(&fmc_handle, &fmc_cfg) != HAL_OK) {
        return SDRAM_ERROR;
    }

    /* Driver handled initialization sequence internally; still provide hook */
    return SDRAM_OK;
}

void SDRAM_Initialization_sequence(uint32_t RefreshCount)
{
    __IO uint32_t tmpmrd = 0;

    /* Step 1: Configure a clock configuration enable command */
sdram_cmd.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
sdram_cmd.CommandTarget = (fmc_cfg.bank == FMC_SDRAM_BANK1) ? FMC_SDRAM_CMD_TARGET_BANK1 : FMC_SDRAM_CMD_TARGET_BANK2;
sdram_cmd.AutoRefreshNumber = 1;
sdram_cmd.ModeRegisterDefinition = 0;

HAL_SDRAM_SendCommand(&fmc_handle.hsdram, &sdram_cmd, SDRAM_TIMEOUT);

    /* Step 2: Insert delay (min 100us) */
    HAL_Delay(1);

    /* Step 3: Precharge all */
sdram_cmd.CommandMode = FMC_SDRAM_CMD_PALL;
sdram_cmd.CommandTarget = (fmc_cfg.bank == FMC_SDRAM_BANK1) ? FMC_SDRAM_CMD_TARGET_BANK1 : FMC_SDRAM_CMD_TARGET_BANK2;
sdram_cmd.AutoRefreshNumber = 1;
sdram_cmd.ModeRegisterDefinition = 0;
HAL_SDRAM_SendCommand(&fmc_handle.hsdram, &sdram_cmd, SDRAM_TIMEOUT);

    /* Step 4: Auto refresh (ST BSP uses 8 cycles for IS42S16400J) */
sdram_cmd.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
sdram_cmd.CommandTarget = (fmc_cfg.bank == FMC_SDRAM_BANK1) ? FMC_SDRAM_CMD_TARGET_BANK1 : FMC_SDRAM_CMD_TARGET_BANK2;
sdram_cmd.AutoRefreshNumber = 8;
sdram_cmd.ModeRegisterDefinition = 0;
HAL_SDRAM_SendCommand(&fmc_handle.hsdram, &sdram_cmd, SDRAM_TIMEOUT);

    /* Step 5: Program the external memory mode register */
    tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1 |
             SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL |
             SDRAM_MODEREG_CAS_LATENCY_3 |
             SDRAM_MODEREG_OPERATING_MODE_STANDARD |
             SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

    sdram_cmd.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
    sdram_cmd.CommandTarget = (fmc_cfg.bank == FMC_SDRAM_BANK1) ? FMC_SDRAM_CMD_TARGET_BANK1 : FMC_SDRAM_CMD_TARGET_BANK2;
    sdram_cmd.AutoRefreshNumber = 1;
    sdram_cmd.ModeRegisterDefinition = tmpmrd;
    HAL_SDRAM_SendCommand(&fmc_handle.hsdram, &sdram_cmd, SDRAM_TIMEOUT);

    /* Step 6: Set the refresh rate counter */
    HAL_SDRAM_ProgramRefreshRate(&fmc_handle.hsdram, RefreshCount);
}

SDRAM_StatusTypeDef SDRAM_Read(uint32_t StartAddress, uint32_t *pData, uint32_t Size)
{
    uint32_t byteSize = Size * 4ul;
    if (FMC_Driver_SDRAM_Read(&fmc_handle, StartAddress, (uint8_t *)pData, byteSize) != HAL_OK) {
        return SDRAM_ERROR;
    }
    return SDRAM_OK;
}

SDRAM_StatusTypeDef SDRAM_Read_DMA(uint32_t StartAddress, uint32_t *pData, uint32_t Size)
{
    /* FMC driver choose DMA or direct read depending on size */
    uint32_t byteSize = Size * 4ul;
    if (FMC_Driver_SDRAM_Read(&fmc_handle, StartAddress, (uint8_t *)pData, byteSize) != HAL_OK) {
        return SDRAM_ERROR;
    }
    return SDRAM_OK;
}

SDRAM_StatusTypeDef SDRAM_Write(uint32_t StartAddress, uint32_t *pData, uint32_t Size)
{
    uint32_t byteSize = Size * 4ul;
    if (FMC_Driver_SDRAM_Write(&fmc_handle, StartAddress, (const uint8_t *)pData, byteSize) != HAL_OK) {
        return SDRAM_ERROR;
    }
    return SDRAM_OK;
}

SDRAM_StatusTypeDef SDRAM_Write_DMA(uint32_t StartAddress, uint32_t *pData, uint32_t Size)
{
    uint32_t byteSize = Size * 4ul;
    if (FMC_Driver_SDRAM_Write(&fmc_handle, StartAddress, (const uint8_t *)pData, byteSize) != HAL_OK) {
        return SDRAM_ERROR;
    }
    return SDRAM_OK;
}

SDRAM_StatusTypeDef SDRAM_SendCommand(FMC_SDRAM_CommandTypeDef *SdramCmd)
{
    if (HAL_SDRAM_SendCommand(&fmc_handle.hsdram, SdramCmd, SDRAM_TIMEOUT) != HAL_OK) {
        return SDRAM_ERROR;
    }
    return SDRAM_OK;
}

void SDRAM_DMA_IRQHandler(void)
{
    HAL_DMA_IRQHandler(fmc_handle.hsdram.hdma);
}

/* Weak MSP implementations - can be overridden by application to set pin mux, DMA, NVIC */
__weak void SDRAM_MspInit(SDRAM_HandleTypeDef *hsdram, void *Params)
{
    /* Silence unused parameter warnings when application does not override this weak hook */
    (void)hsdram;
    (void)Params;

    /* Default: enable FMC clock and DMA clock; full pin config is application-specific */
    __HAL_RCC_FMC_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();
}

__weak void SDRAM_MspDeInit(SDRAM_HandleTypeDef *hsdram, void *Params)
{
    (void)hsdram;
    (void)Params;

    /* Default: disable clocks (if desired) */
    /* Actual GPIO and DMA deinit should be provided by application if needed */
}
