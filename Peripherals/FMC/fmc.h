/**
  ******************************************************************************
  * @file    fmc.h
  * @brief   FMC module interface
  * @details This file contains all the function prototypes for
  *          the Flexible Memory Controller (FMC) configuration.
  *          It provides APIs to initialize and control external memory
  *          interfaces on the STM32F429 board, particularly SDRAM.
  * @version 1.0
  * @date    2025-04-15
  ******************************************************************************
  */

#ifndef __FMC_H__
#define __FMC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported functions prototypes ---------------------------------------------*/
/**
 * @brief   Initializes FMC peripheral used for external memory control
 * @details Configures the FMC controller for SDRAM operation with
 *          appropriate timing parameters, refresh rate, and access configuration
 * @param   None
 * @retval  None
 */
void FMC_Init(void);

/* Exported variables ---------------------------------------------------------*/
/**
 * @brief   SDRAM handle structure
 * @details Used by HAL functions to manage SDRAM operations
 *          typically used for graphics framebuffer or large data storage
 */
extern SDRAM_HandleTypeDef hsdram1;

#ifdef __cplusplus
}
#endif

#endif /* __FMC_H__ */
