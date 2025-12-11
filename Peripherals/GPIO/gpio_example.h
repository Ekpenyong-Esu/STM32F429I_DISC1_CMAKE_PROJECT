/**
 * @file gpio_example.h
 * @brief Header file for GPIO driver examples
 * @version 1.0
 * @date 2025-09-03
 * @author STM32 Team
 *
 * This file contains function prototypes for GPIO usage examples.
 * Follows STM32 HAL library best practices.
 */

#ifndef GPIO_EXAMPLE_H
#define GPIO_EXAMPLE_H

#include "stm32f4xx_hal.h"

/* Function Prototypes */

/**
 * @brief GPIO LED example function
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GPIO_LED_Example(void);

/**
 * @brief GPIO Button example function
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GPIO_Button_Example(void);

/**
 * @brief GPIO Interrupt example function
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GPIO_Interrupt_Example(void);

/**
 * @brief GPIO Polling example function
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GPIO_Polling_Example(void);

/**
 * @brief Main GPIO example function
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GPIO_Example(void);

#endif /* GPIO_EXAMPLE_H */
