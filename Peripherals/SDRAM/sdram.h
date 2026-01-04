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

/* Memory layout */
#define SDRAM_DEVICE_ADDR         ((uint32_t)0xD0000000u)
#define SDRAM_DEVICE_SIZE         ((uint32_t)0x800000u)  /* 8 MBytes */

/* SDRAM configuration defaults */
#define SDRAM_TIMEOUT            ((uint32_t)0xFFFFu)
#define SDRAM_REFRESH_COUNT      ((uint32_t)1386u)

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
