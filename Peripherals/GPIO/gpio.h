/**
  ******************************************************************************
  * @file    gpio.h
  * @brief   GPIO module interface
  * @details This file contains all the function prototypes for
  *          the GPIO peripheral configuration and control.
  *          It provides APIs to initialize and control different GPIO pins
  *          on the STM32F429 board.
  * @version 1.0
  * @date    2025-09-03
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
typedef enum {
    INTERRUPT_MODE,
    POLLING_MODE
} GPIO_Mode_t;
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
 * @param   mode: GPIO_Mode_t (INTERRUPT_MODE or POLLING_MODE)
 * @retval  None
 */
void GPIO_PA0_Button_Init(GPIO_Mode_t mode);

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
void GPIO_LED_LD3_Set(GPIO_PinState state);

/**
 * @brief   Set the state of the LD4 LED
 * @details Controls the LD4 LED on the STM32F429 board
 * @param   state: Desired state (GPIO_PIN_SET or GPIO_PIN_RESET)
 * @retval  None
 */
void GPIO_LED_LD4_Set(GPIO_PinState state);

/**
 * @brief   Toggle the LD3 LED
 * @details Toggles the current state of LD3 LED
 * @param   None
 * @retval  None
 */
void GPIO_LED_LD3_Toggle(void);

/**
 * @brief   Toggle the LD4 LED
 * @details Toggles the current state of LD4 LED
 * @param   None
 * @retval  None
 */
void GPIO_LED_LD4_Toggle(void);

/**
 * @brief   Get the state of the user button (B1/PA0)
 * @details Reads the current state of the user button
 * @param   None
 * @retval  GPIO_PinState: Current button state
 */
GPIO_PinState GPIO_Button_B1_GetState(void);

/**
 * @brief   Check if user button is pressed (with debouncing)
 * @details Reads button state with software debouncing
 * @param   debounce_ms: Debounce time in milliseconds
 * @retval  uint8_t: 1 if button is pressed, 0 otherwise
 */
uint8_t GPIO_Button_B1_IsPressed(uint32_t debounce_ms);

/**
 * @brief   Get button press count
 * @details Returns the total number of button presses since last reset
 * @param   None
 * @retval  uint32_t: Button press count
 */
uint32_t GPIO_Button_GetPressCount(void);

/**
 * @brief   Reset button press count
 * @details Resets the button press counter to zero
 * @param   None
 * @retval  None
 */
void GPIO_Button_ResetPressCount(void);

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
