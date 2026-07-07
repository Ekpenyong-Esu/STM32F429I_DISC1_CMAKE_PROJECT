/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   GPIO module implementation for STM32F429
  * @details This file provides comprehensive code for the configuration
  *          and control of GPIO pins on the STM32F429 microcontroller.
  * @version 2.0
  * @date    2024-12-19
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"
#include <stdio.h>

/* Private defines -----------------------------------------------------------*/
#define GPIO_TIMEOUT 0x1000U

/* Private variables ---------------------------------------------------------*/
static volatile uint32_t button_press_count = 0;

/* Private function prototypes -----------------------------------------------*/
static HAL_StatusTypeDef GPIO_EnableClock(GPIO_TypeDef *GPIOx);
static uint8_t GPIO_GetPinNumber(uint16_t GPIO_Pin);
static IRQn_Type GPIO_GetIRQn(uint16_t GPIO_Pin);

/* Exported functions ------------------------------------------------------- */

/**
 * @brief Initialize GPIO driver handle
 * @param handle: Pointer to GPIO driver handle
 * @retval HAL_StatusTypeDef: Operation status
 */
HAL_StatusTypeDef GPIO_Driver_Init(GPIO_Driver_Handle_t *handle)
{
    if (handle == NULL) {
        return HAL_ERROR;
    }

    /* Enable GPIO clock */
    if (GPIO_EnableClock(handle->Port) != HAL_OK) {
        handle->errorCode = GPIO_DRIVER_ERROR_INIT;
        return HAL_ERROR;
    }

    /* Initialize GPIO pin */
    HAL_GPIO_Init(handle->Port, &handle->Init);

    handle->initialized = 1;
    handle->errorCode = GPIO_DRIVER_ERROR_NONE;

    return HAL_OK;
}

/**
 * @brief Deinitialize GPIO driver handle
 * @param handle: Pointer to GPIO driver handle
 * @retval HAL_StatusTypeDef: Operation status
 */
HAL_StatusTypeDef GPIO_Driver_DeInit(GPIO_Driver_Handle_t *handle)
{
    if (handle == NULL || !handle->initialized) {
        return HAL_ERROR;
    }

    /* Deinitialize GPIO pin */
    HAL_GPIO_DeInit(handle->Port, handle->Init.Pin);

    handle->initialized = 0;
    handle->errorCode = GPIO_DRIVER_ERROR_NONE;

    return HAL_OK;
}

/**
 * @brief Initialize GPIO pin with specified configuration
 * @param GPIOx: GPIO port
 * @param GPIO_Init: GPIO initialization structure
 * @retval HAL_StatusTypeDef: Operation status
 */
HAL_StatusTypeDef GPIO_Driver_Pin_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init)
{
    if (GPIOx == NULL || GPIO_Init == NULL) {
        return HAL_ERROR;
    }

    /* Enable GPIO clock */
    if (GPIO_EnableClock(GPIOx) != HAL_OK) {
        return HAL_ERROR;
    }

    /* Initialize GPIO pin */
    HAL_GPIO_Init(GPIOx, GPIO_Init);

    return HAL_OK;
}

/**
 * @brief Read GPIO pin state
 * @param GPIOx: GPIO port
 * @param GPIO_Pin: GPIO pin
 * @retval GPIO_PinState: Pin state (GPIO_PIN_SET or GPIO_PIN_RESET)
 */
GPIO_PinState GPIO_Driver_ReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    return HAL_GPIO_ReadPin(GPIOx, GPIO_Pin);
}

/**
 * @brief Write GPIO pin state
 * @param GPIOx: GPIO port
 * @param GPIO_Pin: GPIO pin
 * @param PinState: Pin state (GPIO_PIN_SET or GPIO_PIN_RESET)
 * @retval None
 */
void GPIO_Driver_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, PinState);
}

/**
 * @brief Toggle GPIO pin state
 * @param GPIOx: GPIO port
 * @param GPIO_Pin: GPIO pin
 * @retval None
 */
void GPIO_Driver_TogglePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    HAL_GPIO_TogglePin(GPIOx, GPIO_Pin);
}

/**
 * @brief Read GPIO port value
 * @param GPIOx: GPIO port
 * @retval uint16_t: Port value
 */
uint16_t GPIO_Driver_ReadPort(GPIO_TypeDef *GPIOx)
{
    if (GPIOx == NULL) {
        return 0;
    }
    return (uint16_t)(GPIOx->IDR);
}

/**
 * @brief Write GPIO port value
 * @param GPIOx: GPIO port
 * @param PortValue: Port value
 * @retval None
 */
void GPIO_Driver_WritePort(GPIO_TypeDef *GPIOx, uint16_t PortValue)
{
    if (GPIOx != NULL) {
        GPIOx->ODR = PortValue;
    }
}

/**
 * @brief Enable GPIO interrupt
 * @param GPIOx: GPIO port
 * @param GPIO_Pin: GPIO pin
 * @param edge: Interrupt edge (EXTI_TRIGGER_RISING/FALLING/RISING_FALLING)
 * @retval HAL_StatusTypeDef: Operation status
 */
HAL_StatusTypeDef GPIO_Driver_EnableIT(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint32_t edge)
{
    if (GPIOx == NULL) {
        return HAL_ERROR;
    }

    uint8_t pin_num = GPIO_GetPinNumber(GPIO_Pin);
    if (pin_num > 15) {
        return HAL_ERROR;
    }

    /* Configure SYSCFG for EXTI */
    uint32_t port_num = ((uint32_t)GPIOx - GPIOA_BASE) / 0x400;
    uint32_t exti_cr_reg = pin_num / 4;
    uint32_t exti_cr_shift = (pin_num % 4) * 4;

    SYSCFG->EXTICR[exti_cr_reg] &= ~(0xF << exti_cr_shift);
    SYSCFG->EXTICR[exti_cr_reg] |= (port_num << exti_cr_shift);

    /* Configure EXTI */
    EXTI->IMR |= GPIO_Pin;  // Unmask interrupt

    switch (edge) {
        case EXTI_TRIGGER_RISING:
            EXTI->RTSR |= GPIO_Pin;
            EXTI->FTSR &= ~GPIO_Pin;
            break;
        case EXTI_TRIGGER_FALLING:
            EXTI->FTSR |= GPIO_Pin;
            EXTI->RTSR &= ~GPIO_Pin;
            break;
        case EXTI_TRIGGER_RISING_FALLING:
            EXTI->RTSR |= GPIO_Pin;
            EXTI->FTSR |= GPIO_Pin;
            break;
        default:
            return HAL_ERROR;
    }

    /* Enable NVIC interrupt */
    IRQn_Type irqn = GPIO_GetIRQn(GPIO_Pin);
    HAL_NVIC_SetPriority(irqn, 0, 0);
    HAL_NVIC_EnableIRQ(irqn);

    return HAL_OK;
}

/**
 * @brief Disable GPIO interrupt
 * @param GPIOx: GPIO port
 * @param GPIO_Pin: GPIO pin
 * @retval HAL_StatusTypeDef: Operation status
 */
HAL_StatusTypeDef GPIO_Driver_DisableIT(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    if (GPIOx == NULL) {
        return HAL_ERROR;
    }

    /* Mask interrupt */
    EXTI->IMR &= ~GPIO_Pin;

    /* Disable NVIC interrupt */
    IRQn_Type irqn = GPIO_GetIRQn(GPIO_Pin);
    HAL_NVIC_DisableIRQ(irqn);

    return HAL_OK;
}

/**
 * @brief Clear GPIO interrupt pending bit
 * @param GPIO_Pin: GPIO pin
 * @retval None
 */
void GPIO_Driver_ClearITPendingBit(uint16_t GPIO_Pin)
{
    EXTI->PR = GPIO_Pin;
}

/**
 * @brief Lock GPIO pin configuration
 * @param GPIOx: GPIO port
 * @param GPIO_Pin: GPIO pin
 * @retval HAL_StatusTypeDef: Operation status
 */
HAL_StatusTypeDef GPIO_Driver_LockPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    if (GPIOx == NULL) {
        return HAL_ERROR;
    }

    /* Set lock sequence */
    GPIOx->LCKR = GPIO_LCKR_LCKK | GPIO_Pin;
    GPIOx->LCKR = GPIO_Pin;
    GPIOx->LCKR = GPIO_LCKR_LCKK | GPIO_Pin;
    GPIOx->LCKR = GPIO_Pin;
    GPIOx->LCKR = GPIO_LCKR_LCKK | GPIO_Pin;

    /* Read lock status */
    uint32_t lock_status = GPIOx->LCKR;

    if (lock_status & GPIO_LCKR_LCKK) {
        return HAL_OK;  // Successfully locked
    } else {
        return HAL_ERROR;  // Lock failed
    }
}

/**
 * @brief Get GPIO driver error code
 * @param handle: Pointer to GPIO driver handle
 * @retval uint32_t: Error code
 */
uint32_t GPIO_Driver_GetError(GPIO_Driver_Handle_t *handle)
{
    if (handle == NULL) {
        return GPIO_DRIVER_ERROR_INIT;
    }
    return handle->errorCode;
}

/* Private functions ------------------------------------------------------- */

/**
 * @brief Enable GPIO port clock
 * @param GPIOx: GPIO port
 * @retval HAL_StatusTypeDef: Operation status
 */
static HAL_StatusTypeDef GPIO_EnableClock(GPIO_TypeDef *GPIOx)
{
    if (GPIOx == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    } else if (GPIOx == GPIOB) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    } else if (GPIOx == GPIOC) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    } else if (GPIOx == GPIOD) {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    } else if (GPIOx == GPIOE) {
        __HAL_RCC_GPIOE_CLK_ENABLE();
    } else if (GPIOx == GPIOF) {
        __HAL_RCC_GPIOF_CLK_ENABLE();
    } else if (GPIOx == GPIOG) {
        __HAL_RCC_GPIOG_CLK_ENABLE();
    } else if (GPIOx == GPIOH) {
        __HAL_RCC_GPIOH_CLK_ENABLE();
    } else if (GPIOx == GPIOI) {
        __HAL_RCC_GPIOI_CLK_ENABLE();
    } else {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
 * @brief Get pin number from pin mask
 * @param GPIO_Pin: GPIO pin mask
 * @retval uint8_t: Pin number (0-15) or 0xFF if invalid
 */
static uint8_t GPIO_GetPinNumber(uint16_t GPIO_Pin)
{
    uint8_t pin_num = 0;
    while ((GPIO_Pin & (1 << pin_num)) == 0 && pin_num < 16) {
        pin_num++;
    }
    return (pin_num < 16) ? pin_num : 0xFF;
}

/**
 * @brief Get IRQ number for GPIO pin
 * @param GPIO_Pin: GPIO pin mask
 * @retval IRQn_Type: IRQ number
 */
static IRQn_Type GPIO_GetIRQn(uint16_t GPIO_Pin)
{
    uint8_t pin_num = GPIO_GetPinNumber(GPIO_Pin);

    if (pin_num < 5) {
        return (IRQn_Type)(EXTI0_IRQn + pin_num);
    } else if (pin_num < 10) {
        return EXTI9_5_IRQn;
    } else {
        return EXTI15_10_IRQn;
    }
}

/* Interrupt handlers ------------------------------------------------------ */

/**
 * @brief EXTI line 0 interrupt handler
 */
// void EXTI0_IRQHandler(void)
// {
//     if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_0) != RESET) {
//         __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
//         // Add custom handler here
//     }
// }

/**
 * @brief EXTI line 1 interrupt handler
 */
void EXTI1_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_1) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1);
        // Add custom handler here
    }
}

/**
 * @brief EXTI lines 2-3 interrupt handler
 */
void EXTI2_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_2) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_2);
        // Add custom handler here
    }
}

void EXTI3_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_3) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_3);
        // Add custom handler here
    }
}

/**
 * @brief EXTI lines 4-9 interrupt handler
 */
void EXTI9_5_IRQHandler(void)
{
    for (uint8_t pin = 4; pin <= 9; pin++) {
        uint16_t pin_mask = 1 << pin;
        if (__HAL_GPIO_EXTI_GET_IT(pin_mask) != RESET) {
            __HAL_GPIO_EXTI_CLEAR_IT(pin_mask);
            // Add custom handler here based on pin
        }
    }
}

