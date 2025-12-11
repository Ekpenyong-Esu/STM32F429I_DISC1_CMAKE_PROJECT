#ifndef TIM_EXAMPLE_H
#define TIM_EXAMPLE_H

#include "stdint.h"

#define TIMER_PERIOD_10KHZ       9U
// --- Timer and GPIO constants ---
#define TIMER_PRESCALER_1HZ      8399U
#define TIMER_PERIOD_1HZ         9999U
#define TIMER_PRESCALER_10KHZ    839U

#define TIMER_PWM_50_PERCENT     5U
#define TIMER_PWM_25_PERCENT     2U
#define TIMER_PWM_75_PERCENT     7U
#define TIMER_OC_PULSE           5000U
#define TIMER_INPUTCAP_PRESC     83U
#define TIMER_PERIOD_MAX         0xFFFFU
#define SYSTICK_DELAY_1S         1000U
#define LED_PATTERN_ON           0xFF00U
#define LED_PATTERN_OFF          0x00FFU

extern uint32_t pwm_buffer[2]; // PWM buffer for DMA example

#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes for TIM examples
void Timer_Basic_Example(void);
void Timer_Interrupt_Example(void);
void Timer_DMA_Blink_Example(void);
void Timer_PWM_Example(void);
void Timer_PWM_DMA_Example(void);
void Timer_ExtClock_Mode1_Example(void);
void Timer_ExtClock_Mode2_Example(void);
void Timer_InputCapture_Example(void);
void Timer_OutputCompare_Example(void);
void Timer_OnePulse_Example(void);
void Timer_Encoder_Example(void);
void Timer_MasterSlave_Sync_Example(void);
void Timer_SysTick_Example(void);
void DWT_Delay_us(uint32_t microseconds);
void Timer_InputCapture_IT_Example(void);
void Timer_DMA_Blink_Example_IT(void);
void Timer_ExtClock_Mode1_IT_Example(void);
void Timer_ExtClock_Mode2_IT_Example(void);
void Timer_PWM_IT_Example(void);
void Timer_PWM_DMA_IT_Example(void);
void Timer_OutputCompare_IT_Example(void);
void Timer_Encoder_Example_IT(void);
void Timer_OnePulse_IT_Example(void);
void Timer_MasterSlave_Sync_IT_Example(void);

#ifdef __cplusplus
}
#endif

#endif // TIM_EXAMPLE_H
