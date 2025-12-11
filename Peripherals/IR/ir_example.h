/**
 * @file ir_example.h
 * @brief IR driver usage examples header
 * @details This file provides the function prototypes for IR driver examples
 *          for infrared communication on STM32F429 Discovery board.
 * @version 1.0
 * @date 2025-09-03
 * @author STM32 Team
 */

#ifndef IR_EXAMPLE_H
#define IR_EXAMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Exported function prototypes ---------------------------------------------*/

/**
 * @brief Basic IR initialization example
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_Example_Basic_Init(void);

/**
 * @brief IR transmitter example (NEC protocol)
 * @param address: Device address
 * @param command: Command code
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_Example_Transmit_NEC(uint8_t address, uint8_t command);

/**
 * @brief IR transmitter example (RC5 protocol)
 * @param address: Device address
 * @param command: Command code
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_Example_Transmit_RC5(uint8_t address, uint8_t command);

/**
 * @brief IR transmitter example (SIRC protocol)
 * @param address: Device address
 * @param command: Command code
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_Example_Transmit_SIRC(uint8_t address, uint8_t command);

/**
 * @brief IR receiver example
 * @param timeout_ms: Timeout in milliseconds
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_Example_Receive(uint32_t timeout_ms);

/**
 * @brief Continuous IR receiver example
 * @param duration_ms: Duration in milliseconds
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_Example_Continuous_Receive(uint32_t duration_ms);

/**
 * @brief Custom IR transmission example
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_Example_Custom_Transmit(void);

/**
 * @brief IR carrier frequency configuration example
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_Example_Configure_Carrier(void);

/**
 * @brief IR protocol comparison example
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_Example_Protocol_Comparison(void);

/**
 * @brief Comprehensive IR test example
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_Example_Comprehensive_Test(void);

/**
 * @brief Cleanup IR resources
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_Example_Cleanup(void);

#ifdef __cplusplus
}
#endif

#endif /* IR_EXAMPLE_H */
