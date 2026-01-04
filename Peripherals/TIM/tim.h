/**
 * @file tim.h
 * @brief STM32F429I-DISC1 Unified Timer Driver
 *
 * Provides a simplified API for all timer features:
 * - Base timer (counting, delays)
 * - PWM output (LED dimming, motor control)
 * - Input capture (frequency/pulse measurement)
 * - Output compare (timing events)
 * - Encoder interface (rotary encoder reading)
 *
 * @note Enable timer clock before calling init functions:
 *       __HAL_RCC_TIMx_CLK_ENABLE()
 */

#ifndef TIM_H
#define TIM_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================== Common Defines ========================== */

/** @brief 1 Hz timer: 90 MHz / 9000 / 10000 = 1 Hz */
#define TIM_PRESCALER_1HZ       (8999U)
#define TIM_PERIOD_1HZ          (9999U)

/** @brief 1 kHz timer: 90 MHz / 900 / 100 = 1 kHz */
#define TIM_PRESCALER_1KHZ      (899U)
#define TIM_PERIOD_1KHZ         (99U)

/** @brief 10 kHz PWM: 90 MHz / 9 / 1000 = 10 kHz */
#define TIM_PRESCALER_10KHZ     (8U)
#define TIM_PERIOD_10KHZ        (999U)

/** @brief PWM duty cycle values (for period = 999) */
#define TIM_PWM_DUTY_0          (0U)
#define TIM_PWM_DUTY_25         (249U)
#define TIM_PWM_DUTY_50         (499U)
#define TIM_PWM_DUTY_75         (749U)
#define TIM_PWM_DUTY_100        (999U)

/* ========================== Base Timer ========================== */

/**
 * @brief  Initialize timer as basic up-counter
 * @param  htim      Timer handle
 * @param  instance  Timer peripheral (TIM1, TIM2, TIM3, etc.)
 * @param  prescaler Clock divider (timer_clk = APB_clk / (prescaler + 1))
 * @param  period    Auto-reload value (overflow at period + 1 counts)
 * @retval HAL_OK on success
 */
HAL_StatusTypeDef TIM_Init(TIM_HandleTypeDef *htim,
                           TIM_TypeDef *instance,
                           uint32_t prescaler,
                           uint32_t period);

/**
 * @brief  Start timer counting
 */
HAL_StatusTypeDef TIM_Start(TIM_HandleTypeDef *htim);

/**
 * @brief  Stop timer counting
 */
HAL_StatusTypeDef TIM_Stop(TIM_HandleTypeDef *htim);

/**
 * @brief  Start timer with update interrupt (calls HAL_TIM_PeriodElapsedCallback)
 */
HAL_StatusTypeDef TIM_Start_IT(TIM_HandleTypeDef *htim);

/**
 * @brief  Stop timer interrupt
 */
HAL_StatusTypeDef TIM_Stop_IT(TIM_HandleTypeDef *htim);

/**
 * @brief  Get current counter value
 */
uint32_t TIM_GetCounter(TIM_HandleTypeDef *htim);

/**
 * @brief  Set counter value
 */
void TIM_SetCounter(TIM_HandleTypeDef *htim, uint32_t value);

/* ========================== PWM Output ========================== */

/**
 * @brief  Initialize timer for PWM output
 * @param  htim      Timer handle
 * @param  instance  Timer peripheral
 * @param  prescaler Clock divider
 * @param  period    PWM period (frequency = timer_clk / (prescaler+1) / (period+1))
 * @retval HAL_OK on success
 */
HAL_StatusTypeDef TIM_PWM_Init(TIM_HandleTypeDef *htim,
                               TIM_TypeDef *instance,
                               uint32_t prescaler,
                               uint32_t period);

/**
 * @brief  Configure PWM channel duty cycle
 * @param  htim    Timer handle
 * @param  channel TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3, or TIM_CHANNEL_4
 * @param  pulse   Compare value (duty = pulse / period * 100%)
 */
HAL_StatusTypeDef TIM_PWM_ConfigChannel(TIM_HandleTypeDef *htim,
                                        uint32_t channel,
                                        uint32_t pulse);

/**
 * @brief  Start PWM output on channel
 */
HAL_StatusTypeDef TIM_PWM_Start(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief  Stop PWM output on channel
 */
HAL_StatusTypeDef TIM_PWM_Stop(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief  Set PWM duty cycle while running
 * @param  htim    Timer handle
 * @param  channel Timer channel
 * @param  pulse   New compare value
 */
void TIM_PWM_SetDuty(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t pulse);

/* ========================== Input Capture ========================== */

/**
 * @brief  Initialize timer for input capture
 */
HAL_StatusTypeDef TIM_IC_Init(TIM_HandleTypeDef *htim,
                              TIM_TypeDef *instance,
                              uint32_t prescaler,
                              uint32_t period);

/**
 * @brief  Configure input capture channel
 * @param  htim     Timer handle
 * @param  channel  Timer channel
 * @param  polarity TIM_ICPOLARITY_RISING, TIM_ICPOLARITY_FALLING, or TIM_ICPOLARITY_BOTHEDGE
 */
HAL_StatusTypeDef TIM_IC_ConfigChannel(TIM_HandleTypeDef *htim,
                                       uint32_t channel,
                                       uint32_t polarity);

/**
 * @brief  Start input capture
 */
HAL_StatusTypeDef TIM_IC_Start(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief  Start input capture with interrupt
 */
HAL_StatusTypeDef TIM_IC_Start_IT(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief  Stop input capture
 */
HAL_StatusTypeDef TIM_IC_Stop(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief  Get captured value
 */
uint32_t TIM_IC_GetCapture(TIM_HandleTypeDef *htim, uint32_t channel);

/* ========================== Output Compare ========================== */

/**
 * @brief  Initialize timer for output compare
 */
HAL_StatusTypeDef TIM_OC_Init(TIM_HandleTypeDef *htim,
                              TIM_TypeDef *instance,
                              uint32_t prescaler,
                              uint32_t period);

/**
 * @brief  Configure output compare channel
 * @param  htim    Timer handle
 * @param  channel Timer channel
 * @param  pulse   Compare value (when counter matches, output toggles)
 */
HAL_StatusTypeDef TIM_OC_ConfigChannel(TIM_HandleTypeDef *htim,
                                       uint32_t channel,
                                       uint32_t pulse);

/**
 * @brief  Start output compare
 */
HAL_StatusTypeDef TIM_OC_Start(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief  Stop output compare
 */
HAL_StatusTypeDef TIM_OC_Stop(TIM_HandleTypeDef *htim, uint32_t channel);

/* ========================== Encoder Interface ========================== */

/**
 * @brief  Initialize timer for quadrature encoder interface
 * @param  htim      Timer handle
 * @param  instance  Timer peripheral (must support encoder: TIM1-5, TIM8)
 * @param  period    Max count before rollover (use 0xFFFF for 16-bit)
 */
HAL_StatusTypeDef TIM_Encoder_Init(TIM_HandleTypeDef *htim,
                                   TIM_TypeDef *instance,
                                   uint32_t period);

/**
 * @brief  Start encoder counting
 */
HAL_StatusTypeDef TIM_Encoder_Start(TIM_HandleTypeDef *htim);

/**
 * @brief  Stop encoder counting
 */
HAL_StatusTypeDef TIM_Encoder_Stop(TIM_HandleTypeDef *htim);

/**
 * @brief  Get encoder count (position)
 */
int32_t TIM_Encoder_GetCount(TIM_HandleTypeDef *htim);

/**
 * @brief  Reset encoder count to zero
 */
void TIM_Encoder_Reset(TIM_HandleTypeDef *htim);

#ifdef __cplusplus
}
#endif

#endif /* TIM_H */
