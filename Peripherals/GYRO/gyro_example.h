/**
 * @file gyro_example.h
 * @brief GYRO driver usage examples header
 * @details This file provides the function prototypes for GYRO driver examples
 *          for the L3GD20 gyroscope on STM32F429 Discovery board.
 * @version 1.0
 * @date 2025-09-03
 * @author STM32 Team
 */

#ifndef GYRO_EXAMPLE_H
#define GYRO_EXAMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Exported function prototypes ---------------------------------------------*/

/**
 * @brief Basic GYRO initialization example
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_Example_Basic_Init(void);

/**
 * @brief Single data read example
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_Example_Single_Read(void);

/**
 * @brief Continuous data reading example
 * @param duration_ms: Duration in milliseconds
 * @param interval_ms: Reading interval in milliseconds
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_Example_Continuous_Read(uint32_t duration_ms, uint32_t interval_ms);

/**
 * @brief Individual axis reading example
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_Example_Individual_Axis(void);

/**
 * @brief Temperature reading example
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_Example_Temperature(void);

/**
 * @brief Full scale configuration example
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_Example_FullScale_Config(void);

/**
 * @brief Data rate configuration example
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_Example_DataRate_Config(void);

/**
 * @brief Comprehensive test example
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_Example_Comprehensive_Test(void);

/**
 * @brief Cleanup GYRO resources
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_Example_Cleanup(void);

#ifdef __cplusplus
}
#endif

#endif /* GYRO_EXAMPLE_H */
