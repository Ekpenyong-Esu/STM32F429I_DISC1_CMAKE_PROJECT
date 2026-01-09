/* sdram.h - Simple SDRAM driver adapted for project
 * Provides a small, straightforward API for SDRAM (IS42S16400J)
 */

#ifndef __SDRAM_H__
#define __SDRAM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"
#include <stdint.h>

/* Status */
typedef enum {
    SDRAM_OK = 0,
    SDRAM_ERROR
} SDRAM_StatusTypeDef;



/* Public API */
SDRAM_StatusTypeDef SDRAM_Init(void);
void SDRAM_Initialization_sequence(uint32_t RefreshCount);
SDRAM_StatusTypeDef SDRAM_Read(uint32_t StartAddress, uint32_t *pData, uint32_t Size);
SDRAM_StatusTypeDef SDRAM_Read_DMA(uint32_t StartAddress, uint32_t *pData, uint32_t Size);
SDRAM_StatusTypeDef SDRAM_Write(uint32_t StartAddress, uint32_t *pData, uint32_t Size);
SDRAM_StatusTypeDef SDRAM_Write_DMA(uint32_t StartAddress, uint32_t *pData, uint32_t Size);
SDRAM_StatusTypeDef SDRAM_SendCommand(FMC_SDRAM_CommandTypeDef *SdramCmd);
void SDRAM_DMA_IRQHandler(void);

/* Weak MSP hooks (override in application if needed) */
void SDRAM_MspInit(SDRAM_HandleTypeDef *hsdram, void *Params);
void SDRAM_MspDeInit(SDRAM_HandleTypeDef *hsdram, void *Params);

#ifdef __cplusplus
}
#endif

#endif /* __SDRAM_H__ */
