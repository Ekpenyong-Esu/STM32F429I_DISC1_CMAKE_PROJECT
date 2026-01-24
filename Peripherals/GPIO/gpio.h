/**
  ******************************************************************************
  * @file    gpio.h
  * @brief   GPIO module interface for STM32F429
  * @details This file contains all the function prototypes for
  *          the GPIO peripheral configuration and control.
  *          It provides comprehensive APIs to initialize and control GPIO pins
  *          on the STM32F429 microcontroller.
  * @version 2.0
  * @date    2024-12-19
  ******************************************************************************
  */

#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"


/* Exported types ------------------------------------------------------------*/

/**
 * @brief GPIO driver handle structure
 */
typedef struct {
    GPIO_TypeDef *Port;                    /**< GPIO port (GPIOA-GPIOI) */
    GPIO_InitTypeDef Init;                 /**< GPIO initialization structure */
    uint8_t initialized;                   /**< Initialization status flag */
    uint32_t errorCode;                    /**< Last error code */
} GPIO_Driver_Handle_t;

/**
 * @brief GPIO driver error codes
 */
typedef enum {
    GPIO_DRIVER_ERROR_NONE = 0x00U,       /**< Operation successful */
    GPIO_DRIVER_ERROR_INIT = 0x01U,       /**< Initialization failed */
    GPIO_DRIVER_ERROR_CONFIG = 0x02U,     /**< Configuration error */
    GPIO_DRIVER_ERROR_PIN = 0x04U         /**< Invalid pin specification */
} GPIO_Driver_Error_t;

/* Exported constants --------------------------------------------------------*/

/* GPIO pin definitions */
#define GPIO_PIN_0                 ((uint16_t)0x0001)
#define GPIO_PIN_1                 ((uint16_t)0x0002)
#define GPIO_PIN_2                 ((uint16_t)0x0004)
#define GPIO_PIN_3                 ((uint16_t)0x0008)
#define GPIO_PIN_4                 ((uint16_t)0x0010)
#define GPIO_PIN_5                 ((uint16_t)0x0020)
#define GPIO_PIN_6                 ((uint16_t)0x0040)
#define GPIO_PIN_7                 ((uint16_t)0x0080)
#define GPIO_PIN_8                 ((uint16_t)0x0100)
#define GPIO_PIN_9                 ((uint16_t)0x0200)
#define GPIO_PIN_10                ((uint16_t)0x0400)
#define GPIO_PIN_11                ((uint16_t)0x0800)
#define GPIO_PIN_12                ((uint16_t)0x1000)
#define GPIO_PIN_13                ((uint16_t)0x2000)
#define GPIO_PIN_14                ((uint16_t)0x4000)
#define GPIO_PIN_15                ((uint16_t)0x8000)
#define GPIO_PIN_ALL               ((uint16_t)0xFFFF)

/* Exported macro ------------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/**
 * @brief Initialize GPIO driver handle
 * @param handle: Pointer to GPIO driver handle
 * @retval HAL_StatusTypeDef: Operation status
 */
HAL_StatusTypeDef GPIO_Driver_Init(GPIO_Driver_Handle_t *handle);

/**
 * @brief Deinitialize GPIO driver handle
 * @param handle: Pointer to GPIO driver handle
 * @retval HAL_StatusTypeDef: Operation status
 */
HAL_StatusTypeDef GPIO_Driver_DeInit(GPIO_Driver_Handle_t *handle);

/**
 * @brief Read GPIO pin state
 * @param GPIOx: GPIO port
 * @param GPIO_Pin: GPIO pin
 * @retval GPIO_PinState: Pin state (SET or RESET)
 */
GPIO_PinState GPIO_Driver_ReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

/**
 * @brief Write GPIO pin state
 * @param GPIOx: GPIO port
 * @param GPIO_Pin: GPIO pin
 * @param PinState: Pin state to be written (SET or RESET)
 * @retval None
 */
void GPIO_Driver_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);

/**
 * @brief Toggle GPIO pin state
 * @param GPIOx: GPIO port
 * @param GPIO_Pin: GPIO pin
 * @retval None
 */
void GPIO_Driver_TogglePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

/**
 * @brief Read GPIO port value
 * @param GPIOx: GPIO port
 * @retval uint16_t: Port value
 */
uint16_t GPIO_Driver_ReadPort(GPIO_TypeDef *GPIOx);

/**
 * @brief Write GPIO port value
 * @param GPIOx: GPIO port
 * @param PortValue: Port value
 * @retval None
 */
void GPIO_Driver_WritePort(GPIO_TypeDef *GPIOx, uint16_t PortValue);

/**
 * @brief Enable GPIO interrupt
 * @param GPIOx: GPIO port
 * @param GPIO_Pin: GPIO pin
 * @param edge: Interrupt edge (EXTI_TRIGGER_RISING/FALLING/RISING_FALLING)
 * @retval HAL_StatusTypeDef: Operation status
 */
HAL_StatusTypeDef GPIO_Driver_EnableIT(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint32_t edge);

/**
 * @brief Disable GPIO interrupt
 * @param GPIOx: GPIO port
 * @param GPIO_Pin: GPIO pin
 * @retval HAL_StatusTypeDef: Operation status
 */
HAL_StatusTypeDef GPIO_Driver_DisableIT(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

/**
 * @brief Clear GPIO interrupt pending bit
 * @param GPIO_Pin: GPIO pin
 * @retval None
 */
void GPIO_Driver_ClearITPendingBit(uint16_t GPIO_Pin);

/**
 * @brief Lock GPIO pin configuration
 * @param GPIOx: GPIO port
 * @param GPIO_Pin: GPIO pin
 * @retval HAL_StatusTypeDef: Operation status
 */
HAL_StatusTypeDef GPIO_Driver_LockPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

/**
 * @brief Get GPIO driver error code
 * @param handle: Pointer to GPIO driver handle
 * @retval uint32_t: Error code
 */
uint32_t GPIO_Driver_GetError(GPIO_Driver_Handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* __GPIO_H__ */
