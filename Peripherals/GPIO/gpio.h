/**
  ******************************************************************************
  * @file    gpio.h
  * @brief   GPIO module interface
  * @details This file contains all the function prototypes for
  *          the GPIO peripheral configuration and control.
  *          It provides APIs to initialize and control different GPIO pins
  *          on the STM32F429 board.
  * @version 1.0
  * @date    2025-04-15
  ******************************************************************************
  */

#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported functions prototypes ---------------------------------------------*/


typedef enum {
    INTERRUPT_MODE,
    POLLING_MODE
}Status;
/**
 * @brief   Initializes all GPIO pins used in the application
 * @details Configures the GPIO pin modes, pull-up/down resistors,
 *          speeds, and initial output levels
 * @param   None
 * @retval  None
 */
void GPIO_Init(void);

/**
 * @brief   Initialize PA0 button interrupt
 * @details Configures PA0 (B1) button for interrupt operation
 * @param   None
 * @retval  None
 */
void GPIO_PA0_Button_Init(Status status);

/**
 * @brief   Button interrupt callback handler
 * @details Handles PA0 (B1 button) interrupts
 * @param   None
 * @retval  None
 */
void GPIO_Button_Callback(void);


/**
 * @brief   Set the state of the LD3 LED
 * @details Controls the LD3 LED on the STM32F429 board
 * @param   state: Desired state (GPIO_PIN_SET or GPIO_PIN_RESET)
 * @retval  None
 */
//void GPIO_LED_Control(uint8_t state);

/**
 * @brief   Initialize GPIO interrupts
 * @details Initializes GPIO and configures PA0 button for interrupt operation
 * @param   None
 * @retval  None
 */
void initialize_gpio_interrupts(void);

#ifdef __cplusplus
}
#endif

#endif /* __GPIO_H__ */
