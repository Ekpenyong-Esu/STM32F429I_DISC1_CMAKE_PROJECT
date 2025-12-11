/**
 * @file fmc_example.h
 * @brief Header file for FMC driver examples
 * @version 1.0
 * @date 2024
 * @author STM32 Team
 *
 * This file contains function prototypes for FMC driver usage examples.
 * Follows STM32 HAL library best practices.
 */

#ifndef FMC_EXAMPLE_H
#define FMC_EXAMPLE_H

#include "fmc.h"

/* Function Prototypes */

/**
 * @brief SDRAM example function
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef FMC_Driver_SDRAM_Example(void);

/**
 * @brief NOR Flash example function
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef FMC_Driver_NOR_Example(void);

/**
 * @brief NAND Flash example function
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef FMC_Driver_NAND_Example(void);

/**
 * @brief Main FMC example function
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef FMC_Driver_Example(void);

#endif /* FMC_EXAMPLE_H */
