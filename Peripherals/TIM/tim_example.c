#include "stm32f429xx.h"
#include "tim_base.h"
#include "tim_base_dma.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_tim.h"
#include "tim_extclock.h"
#include "tim_input_capture.h"
#include "tim_encoder.h"
#include "tim_sync.h"
#include "tim_pwm.h"
#include "tim_output_compare.h"

#include "tim_example.h"

uint32_t pwm_buffer[2] = {TIMER_PWM_25_PERCENT, TIMER_PWM_75_PERCENT}; // PWM buffer

// Example handles (should be global/static in real app)
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim10;
DMA_HandleTypeDef hdma_tim2_up;
uint32_t led_pattern[2] = {GPIO_PIN_13, GPIO_PIN_14};

/**
 * @brief Initializes TIM2 as a simple up-counter at 1 Hz. No interrupts or DMA.
 *        TIM2 is a 32-bit timer. No GPIO/AF required for basic counting.
 *        Example output: None (timer runs internally).
 *        Real-world: Simple timekeeping, delays, or periodic polling in embedded systems.
 */
void Timer_Basic_Example(void) {
    __HAL_RCC_GPIOG_CLK_ENABLE();           // Enable GPIOG peripheral clock
    GPIO_InitTypeDef gpio = {0};            // GPIO configuration structure
    gpio.Pin = GPIO_PIN_13 | GPIO_PIN_14;   // Select PG13 and PG14 (user LEDs)
    gpio.Mode = GPIO_MODE_OUTPUT_PP;        // Output, push-pull
    gpio.Pull = GPIO_NOPULL;                // No pull-up or pull-down
    gpio.Speed = GPIO_SPEED_FREQ_LOW;       // Low speed
    HAL_GPIO_Init(GPIOG, &gpio);            // Initialize GPIOG pins

    __HAL_RCC_TIM2_CLK_ENABLE();            // Enable TIM2 peripheral clock
    TIM_Base_Init(&htim2, TIM2, TIMER_PRESCALER_1HZ, TIMER_PERIOD_1HZ); // Init TIM2: 1 Hz
    TIM_Base_Start(&htim2);                 // Start TIM2 base timer
}

/**
 * @brief Initializes TIM3 to generate periodic interrupts at 1 Hz. Sets up NVIC and starts timer in interrupt mode.
 *        TIM3 is a 16-bit timer. No GPIO/AF required for basic interrupt mode.
 *        Example output: None (timer runs internally, toggles LEDs in callback).
 *        Real-world: Periodic tasks such as blinking LEDs, sensor polling, or real-time scheduling.
 */
void Timer_Interrupt_Example(void) {
    __HAL_RCC_GPIOG_CLK_ENABLE();           // Enable GPIOG clock
    GPIO_InitTypeDef gpio = {0};            // GPIO config struct
    gpio.Pin = GPIO_PIN_13 | GPIO_PIN_14;   // PG13, PG14 (LEDs)
    gpio.Mode = GPIO_MODE_OUTPUT_PP;        // Output, push-pull
    gpio.Pull = GPIO_NOPULL;                // No pull
    gpio.Speed = GPIO_SPEED_FREQ_LOW;       // Low speed
    HAL_GPIO_Init(GPIOG, &gpio);            // Init GPIOG

    HAL_NVIC_SetPriority(TIM3_IRQn, 5, 0);  // Set TIM3 IRQ priority
    HAL_NVIC_EnableIRQ(TIM3_IRQn);          // Enable TIM3 IRQ

    __HAL_RCC_TIM3_CLK_ENABLE();            // Enable TIM3 clock
    TIM_Base_Init(&htim3, TIM3, TIMER_PRESCALER_1HZ, TIMER_PERIOD_1HZ); // Init TIM3: 1 Hz
    TIM_Interrupt_Start_IT(&htim3);         // Start TIM3 in interrupt mod
}

/**
 * @brief Uses TIM2 and DMA to toggle GPIOG LEDs with a pattern.
 *        TIM2 (32-bit) triggers DMA1_Stream1/Channel3 on update event.
 *        DMA transfers led_pattern[] to GPIOG->ODR (PG13, PG14: user LEDs on STM32F429I-DISC1).
 *        No alternate function required for GPIOG (output mode).
 *        Real-world: Efficiently drives LED patterns or other parallel outputs without CPU intervention.
 */
void Timer_DMA_Blink_Example(void) {
    __HAL_RCC_GPIOG_CLK_ENABLE();           // Enable GPIOG clock
    GPIO_InitTypeDef gpio = {0};            // GPIO config struct
    gpio.Pin = GPIO_PIN_13 | GPIO_PIN_14;   // PG13, PG14 (LEDs)
    gpio.Mode = GPIO_MODE_OUTPUT_PP;        // Output, push-pull
    gpio.Pull = GPIO_NOPULL;                // No pull
    gpio.Speed = GPIO_SPEED_FREQ_LOW;       // Low speed
    HAL_GPIO_Init(GPIOG, &gpio);            // Init GPIOG

    __HAL_RCC_TIM2_CLK_ENABLE();            // Enable TIM2 clock
    TIM_Base_Init(&htim2, TIM2, TIMER_PRESCALER_1HZ, TIMER_PERIOD_1HZ); // Init TIM2: 1 Hz
    TIM_Base_DMA_Setup(&htim2);             // Setup DMA for TIM2 update event
    TIM_Base_LowLevel_DMA_Blink(&htim2, &hdma_tim2_up, led_pattern, (uint32_t*)&(GPIOG->ODR), 2); // DMA: pattern to ODR
    // Do NOT call TIM_Base_Start here; DMA function already enables timer as needed
}

/**
 * @brief Uses TIM2 and DMA to toggle GPIOG LEDs with a pattern.
 *        TIM2 (32-bit) triggers DMA1_Stream1/Channel3 on update event.
 *        DMA transfers led_pattern[] to GPIOG->ODR (PG13, PG14: user LEDs on STM32F429I-DISC1).
 *        No alternate function required for GPIOG (output mode).
 *        Real-world: Efficiently drives LED patterns or other parallel outputs without CPU intervention.
 */
void Timer_DMA_Blink_Example_IT(void) {
    __HAL_RCC_GPIOG_CLK_ENABLE();           // Enable GPIOG clock
     __HAL_RCC_TIM2_CLK_ENABLE();            // Enable TIM2 clock
    GPIO_InitTypeDef gpio = {0};            // GPIO config struct
    gpio.Pin = GPIO_PIN_13 | GPIO_PIN_14;   // PG13, PG14 (LEDs)
    gpio.Mode = GPIO_MODE_OUTPUT_PP;        // Output, push-pull
    gpio.Pull = GPIO_NOPULL;                // No pull
    gpio.Speed = GPIO_SPEED_FREQ_LOW;       // Low speed
    HAL_GPIO_Init(GPIOG, &gpio);            // Init GPIOG

    HAL_NVIC_SetPriority(TIM2_IRQn, 5, 0);  // Set TIM2 IRQ priority
    HAL_NVIC_EnableIRQ(TIM2_IRQn);          // Enable TIM2 IRQ


    TIM_Base_Init(&htim2, TIM2, TIMER_PRESCALER_1HZ, TIMER_PERIOD_1HZ); // Init TIM2: 1 Hz
    TIM_Base_DMA_Setup(&htim2);             // Setup DMA for TIM2 update event
    TIM_Base_LowLevel_DMA_Blink_IT(&htim2, &hdma_tim2_up, led_pattern, (uint32_t*)&(GPIOG->ODR), 2); // DMA: pattern to ODR
    // Do NOT call TIM_Base_Start here; DMA function already enables timer as needed
}

/**
 * @brief Configures TIM3 CH1 for PWM at 10 kHz, 50% duty cycle.
 *        TIM3_CH1 is mapped to PA6 (AF2) per STM32F429 datasheet.
 *        GPIO: PA6, Mode: Alternate Function Push-Pull, AF2 (TIM3), Low speed, No pull.
 *        Can drive an LED, buzzer, or motor.
 *        Real-world: Dimming LEDs, controlling motors, or generating audio signals.
 */
void Timer_PWM_Example(void) {
    __HAL_RCC_TIM3_CLK_ENABLE();             // Enable TIM3 clock
    __HAL_RCC_GPIOA_CLK_ENABLE();            // Enable GPIOA clock
    GPIO_InitTypeDef gpio = {0};             // GPIO config struct
    gpio.Pin = GPIO_PIN_6;                   // PA6 (TIM3_CH1)
    gpio.Mode = GPIO_MODE_AF_PP;             // Alternate function, push-pull
    gpio.Pull = GPIO_NOPULL;                 // No pull
    gpio.Speed = GPIO_SPEED_FREQ_LOW;        // Low speed
    gpio.Alternate = GPIO_AF2_TIM3;          // AF2 = TIM3
    HAL_GPIO_Init(GPIOA, &gpio);             // Init PA6

    TIM_PWM_Init(&htim3, TIM3, TIMER_PRESCALER_10KHZ, TIMER_PERIOD_10KHZ); // Init TIM3 for PWM
    TIM_PWM_ConfigChannel(&htim3, TIM_CHANNEL_1, TIMER_PWM_50_PERCENT);    // 50% duty cycle
    TIM_PWM_Start(&htim3, TIM_CHANNEL_1);    // Start PWM on TIM3_CH1
}


void Timer_PWM_IT_Example(void) {
    __HAL_RCC_TIM3_CLK_ENABLE();             // Enable TIM3 clock
    __HAL_RCC_GPIOA_CLK_ENABLE();            // Enable GPIOA clock
    GPIO_InitTypeDef gpio = {0};             // GPIO config struct
    gpio.Pin = GPIO_PIN_6;                   // PA6 (TIM3_CH1)
    gpio.Mode = GPIO_MODE_AF_PP;             // Alternate function, push-pull
    gpio.Pull = GPIO_NOPULL;                 // No pull
    gpio.Speed = GPIO_SPEED_FREQ_LOW;        // Low speed
    gpio.Alternate = GPIO_AF2_TIM3;          // AF2 = TIM3
    HAL_GPIO_Init(GPIOA, &gpio);             // Init PA6

    HAL_NVIC_SetPriority(TIM3_IRQn, 5, 0);  // Set TIM3 IRQ priority
    HAL_NVIC_EnableIRQ(TIM3_IRQn);          // Enable TIM3 IRQ

    TIM_PWM_Init(&htim3, TIM3, TIMER_PRESCALER_10KHZ, TIMER_PERIOD_10KHZ); // Init TIM3 for PWM
    TIM_PWM_ConfigChannel(&htim3, TIM_CHANNEL_1, TIMER_PWM_50_PERCENT);    // 50% duty cycle
    HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_1);    // Start PWM on TIM3_CH1
}

/**
 * @brief TIM3 CH1 outputs PWM, duty cycle updated by DMA from buffer.
 *        TIM3_CH1 is mapped to PB4 (AF2) per STM32F429 datasheet.
 *        GPIO: PB4, Mode: Alternate Function Push-Pull, AF2 (TIM3), Low speed, No pull.
 *        DMA1_Stream1/Channel3 can be used for TIM3_UP (if needed for update events).
 *        Real-world: Advanced motor control, digital audio, or complex LED effects.
 */
void Timer_PWM_DMA_Example(void) {
    __HAL_RCC_TIM3_CLK_ENABLE();              // Enable TIM3 clock
    __HAL_RCC_GPIOB_CLK_ENABLE();             // Enable GPIOB clock
    GPIO_InitTypeDef gpio = {0};              // GPIO config struct
    gpio.Pin = GPIO_PIN_4;                    // PB4 (TIM3_CH1)
    gpio.Mode = GPIO_MODE_AF_PP;              // Alternate function, push-pull
    gpio.Pull = GPIO_NOPULL;                  // No pull
    gpio.Speed = GPIO_SPEED_FREQ_LOW;         // Low speed
    gpio.Alternate = GPIO_AF2_TIM3;           // AF2 = TIM3
    HAL_GPIO_Init(GPIOB, &gpio);              // Init PB4

    HAL_NVIC_SetPriority(TIM3_IRQn, 5, 0);  // Set TIM3 IRQ priority
    HAL_NVIC_EnableIRQ(TIM3_IRQn);          // Enable TIM3 IRQ

    TIM_Base_DMA_Setup(&htim3);               // Setup DMA for TIM3
    TIM_PWM_Init(&htim3, TIM3, TIMER_PRESCALER_10KHZ, TIMER_PERIOD_10KHZ); // Init TIM3 for PWM
    TIM_PWM_ConfigChannel(&htim3, TIM_CHANNEL_1, pwm_buffer[0]);           // Set initial duty
    TIM_DMA_PWM_Start(&htim3, TIM_CHANNEL_1, pwm_buffer, 2);               // Start DMA-driven PWM
}

/**
 * @brief TIM3 CH1 outputs PWM with DMA and interrupts, duty cycle updated by DMA from buffer.
 *        TIM3_CH1 is mapped to PB4 (AF2) per STM32F429 datasheet.
 *        GPIO: PB4, Mode: Alternate Function Push-Pull, AF2 (TIM3), Low speed, No pull.
 *        Combines DMA transfer with interrupt handling for advanced PWM control.
 *        Real-world: Advanced motor control with feedback, audio generation, or complex LED patterns.
 */
void Timer_PWM_DMA_IT_Example(void) {
    __HAL_RCC_TIM3_CLK_ENABLE();              // Enable TIM3 clock
    __HAL_RCC_GPIOB_CLK_ENABLE();             // Enable GPIOB clock
    GPIO_InitTypeDef gpio = {0};              // GPIO config struct
    gpio.Pin = GPIO_PIN_4;                    // PB4 (TIM3_CH1)
    gpio.Mode = GPIO_MODE_AF_PP;              // Alternate function, push-pull
    gpio.Pull = GPIO_NOPULL;                  // No pull
    gpio.Speed = GPIO_SPEED_FREQ_LOW;         // Low speed
    gpio.Alternate = GPIO_AF2_TIM3;           // AF2 = TIM3
    HAL_GPIO_Init(GPIOB, &gpio);              // Init PB4

    HAL_NVIC_SetPriority(TIM3_IRQn, 5, 0);   // Set TIM3 IRQ priority
    HAL_NVIC_EnableIRQ(TIM3_IRQn);           // Enable TIM3 IRQ

    TIM_Base_DMA_Setup(&htim3);               // Setup DMA for TIM3
    TIM_PWM_Init(&htim3, TIM3, TIMER_PRESCALER_10KHZ, TIMER_PERIOD_10KHZ); // Init TIM3 for PWM
    TIM_PWM_ConfigChannel(&htim3, TIM_CHANNEL_1, pwm_buffer[0]);           // Set initial duty
    TIM_DMA_PWM_Start(&htim3, TIM_CHANNEL_1, pwm_buffer, 2);               // Start DMA-driven PWM with IT
}

/**
 * @brief Configures TIM3 to use TI1 pin as external clock source (external pulses on CH1).
 * @warning Pin conflict: PA6 is also used in Timer_PWM_Example(). Do not run both simultaneously.
 * @warning Pin conflict: PA6 is also used in Timer_PWM_Example(). Do not run both simultaneously.
 *        TIM3_CH1 is mapped to PA6 (AF2) per STM32F429 datasheet.
 *        GPIO: PA6, Mode: Alternate Function Push-Pull, AF2 (TIM3), Low speed, No pull.
 *        Real-world: Frequency counting, event counting, or external synchronization.
 */
void Timer_ExtClock_Mode1_Example(void) {
    __HAL_RCC_TIM3_CLK_ENABLE();              // Enable TIM3 clock
    __HAL_RCC_GPIOA_CLK_ENABLE();             // Enable GPIOA clock
    GPIO_InitTypeDef gpio = {0};              // GPIO config struct
    gpio.Pin = GPIO_PIN_6;                    // PA6 (TIM3_CH1)
    gpio.Mode = GPIO_MODE_AF_PP;              // Alternate function, push-pull
    gpio.Pull = GPIO_NOPULL;                  // No pull
    gpio.Speed = GPIO_SPEED_FREQ_LOW;         // Low speed
    gpio.Alternate = GPIO_AF2_TIM3;           // AF2 = TIM3
    HAL_GPIO_Init(GPIOA, &gpio);              // Init PA6

    TIM_Base_Init(&htim3, TIM3, 0, TIMER_PERIOD_MAX); // Init TIM3, no prescaler
    TIM_ExtClock_Config(&htim3, TIM_CLOCKSOURCE_TI1, 0, 0, 0); // Ext clock on TI1
    TIM_Base_Start(&htim3);                   // Start TIM3
}

void Timer_ExtClock_Mode1_IT_Example(void) {
    __HAL_RCC_TIM3_CLK_ENABLE();              // Enable TIM3 clock
    __HAL_RCC_GPIOA_CLK_ENABLE();             // Enable GPIOA clock
    GPIO_InitTypeDef gpio = {0};              // GPIO config struct
    gpio.Pin = GPIO_PIN_6;                    // PA6 (TIM3_CH1)
    gpio.Mode = GPIO_MODE_AF_PP;              // Alternate function, push-pull
    gpio.Pull = GPIO_NOPULL;                  // No pull
    gpio.Speed = GPIO_SPEED_FREQ_LOW;         // Low speed
    gpio.Alternate = GPIO_AF2_TIM3;           // AF2 = TIM3
    HAL_GPIO_Init(GPIOA, &gpio);              // Init PA6

    HAL_NVIC_SetPriority(TIM3_IRQn, 5, 0);  // Set TIM3 IRQ priority
    HAL_NVIC_EnableIRQ(TIM3_IRQn);          // Enable TIM3 IRQ

    TIM_Base_Init(&htim3, TIM3, 0, TIMER_PERIOD_MAX); // Init TIM3, no prescaler
    TIM_ExtClock_Config(&htim3, TIM_CLOCKSOURCE_TI1, 0, 0, 0); // Ext clock on TI1
    TIM_Interrupt_Start_IT(&htim3);                   // Start TIM3
}

/**
 * @brief Configures TIM2 to use ETR pin as external clock source (external events on ETR).
 * @warning Pin conflict: PA0 is also used in Timer_InputCapture_Example(). Do not run both simultaneously.
 *        TIM2_ETR is mapped to PA0 (AF1) per STM32F429 datasheet.
 *        GPIO: PA0, Mode: Alternate Function Push-Pull, AF1 (TIM2), Low speed, No pull.
 *        Real-world: Industrial automation, rotary encoders, or external event timing.
 */
void Timer_ExtClock_Mode2_Example(void) {
    __HAL_RCC_TIM2_CLK_ENABLE();               // Enable TIM2 clock
    __HAL_RCC_GPIOA_CLK_ENABLE();              // Enable GPIOA clock
    GPIO_InitTypeDef gpio = {0};               // GPIO config struct
    gpio.Pin = GPIO_PIN_0;                     // PA0 (TIM2_ETR)
    gpio.Mode = GPIO_MODE_AF_PP;               // Alternate function, push-pull
    gpio.Pull = GPIO_NOPULL;                   // No pull
    gpio.Speed = GPIO_SPEED_FREQ_LOW;          // Low speed
    gpio.Alternate = GPIO_AF1_TIM2;            // AF1 = TIM2
    HAL_GPIO_Init(GPIOA, &gpio);               // Init PA0
    TIM_Base_Init(&htim2, TIM2, 0, TIMER_PERIOD_MAX); // Init TIM2, no prescaler
    TIM_ExtClock_Config(&htim2, TIM_CLOCKSOURCE_ETRMODE2, TIM_ETRPOLARITY_NONINVERTED, TIM_ETRPRESCALER_DIV1, 0); // Ext clock on ETR
    TIM_Base_Start(&htim2);                    // Start TIM2
}

void Timer_ExtClock_Mode2_IT_Example(void) {
    __HAL_RCC_TIM2_CLK_ENABLE();               // Enable TIM2 clock
    __HAL_RCC_GPIOA_CLK_ENABLE();              // Enable GPIOA clock
    GPIO_InitTypeDef gpio = {0};               // GPIO config struct
    gpio.Pin = GPIO_PIN_0;                     // PA0 (TIM2_ETR)
    gpio.Mode = GPIO_MODE_AF_PP;               // Alternate function, push-pull
    gpio.Pull = GPIO_NOPULL;                   // No pull
    gpio.Speed = GPIO_SPEED_FREQ_LOW;          // Low speed
    gpio.Alternate = GPIO_AF1_TIM2;            // AF1 = TIM2
    HAL_GPIO_Init(GPIOA, &gpio);               // Init PA0

    HAL_NVIC_SetPriority(TIM2_IRQn, 5, 0);  // Set TIM2 IRQ priority
    HAL_NVIC_EnableIRQ(TIM2_IRQn);          // Enable TIM2 IRQ

    TIM_Base_Init(&htim2, TIM2, 0, TIMER_PERIOD_MAX); // Init TIM2, no prescaler
    TIM_ExtClock_Config(&htim2, TIM_CLOCKSOURCE_ETRMODE2, TIM_ETRPOLARITY_NONINVERTED, TIM_ETRPRESCALER_DIV1, 0); // Ext clock on ETR
    TIM_Interrupt_Start_IT(&htim2);                    // Start TIM2
}

/**
 * @brief Configures TIM2 CH1 to measure pulse width or frequency on input pin.
 * @warning Pin conflict: PA0 is also used in Timer_ExtClock_Mode2_Example(). Do not run both simultaneously.
 *        TIM2_CH1 is mapped to PA0 (AF1) per STM32F429 datasheet.
 *        GPIO: PA0, Mode: Alternate Function Push-Pull, AF1 (TIM2), Low speed, No pull.
 *        Captures rising edges and triggers interrupt.
 *        Real-world: Pulse width measurement, frequency measurement, or signal analysis.
 */
void Timer_InputCapture_Example(void) {
    __HAL_RCC_TIM2_CLK_ENABLE();                // Enable TIM2 clock
    __HAL_RCC_GPIOA_CLK_ENABLE();               // Enable GPIOA clock
    GPIO_InitTypeDef gpio = {0};                // GPIO config struct
    gpio.Pin = GPIO_PIN_0;                      // PA0 (TIM2_CH1)
    gpio.Mode = GPIO_MODE_AF_PP;                // Alternate function, push-pull
    gpio.Pull = GPIO_NOPULL;                    // No pull
    gpio.Speed = GPIO_SPEED_FREQ_LOW;           // Low speed
    gpio.Alternate = GPIO_AF1_TIM2;             // AF1 = TIM2
    HAL_GPIO_Init(GPIOA, &gpio);                // Init PA0

    TIM_IC_Init(&htim2, TIM2, TIMER_INPUTCAP_PRESC, TIMER_PERIOD_MAX); // Init TIM2 for input capture
    TIM_IC_ConfigChannel(&htim2, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING); // Capture rising edges
    TIM_IC_Start(&htim2, TIM_CHANNEL_1);        // Start input capture
}



void Timer_InputCapture_IT_Example(void) {
    __HAL_RCC_TIM2_CLK_ENABLE();                // Enable TIM2 clock
    __HAL_RCC_GPIOA_CLK_ENABLE();               // Enable GPIOA clock
    GPIO_InitTypeDef gpio = {0};                // GPIO config struct
    gpio.Pin = GPIO_PIN_0;                      // PA0 (TIM2_CH1)
    gpio.Mode = GPIO_MODE_AF_PP;                // Alternate function, push-pull
    gpio.Pull = GPIO_NOPULL;                    // No pull
    gpio.Speed = GPIO_SPEED_FREQ_LOW;           // Low speed
    gpio.Alternate = GPIO_AF1_TIM2;             // AF1 = TIM2
    HAL_GPIO_Init(GPIOA, &gpio);                // Init PA0

    HAL_NVIC_SetPriority(TIM2_IRQn, 5, 0);      // Set TIM2 IRQ priority
    HAL_NVIC_EnableIRQ(TIM2_IRQn);              // Enable TIM2 IRQ

    TIM_IC_Init(&htim2, TIM2, TIMER_INPUTCAP_PRESC, TIMER_PERIOD_MAX); // Init TIM2 for input capture
    TIM_IC_ConfigChannel(&htim2, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING); // Capture rising edges
    TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);        // Start input capture
}

/**
 * @brief Configures TIM3 CH1 to toggle output pin on match event.
 *        TIM3_CH1 is mapped to PC6 (AF2) per STM32F429 datasheet.
 *        GPIO: PC6, Mode: Alternate Function Push-Pull, AF2 (TIM3), Low speed, No pull.
 *        Generates a square wave or timed output.
 *        Real-world: Square wave generation, timed control signals, or periodic toggling.
 */
void Timer_OutputCompare_Example(void) {
    __HAL_RCC_TIM3_CLK_ENABLE();                 // Enable TIM3 clock
    __HAL_RCC_GPIOC_CLK_ENABLE();                // Enable GPIOC clock
    GPIO_InitTypeDef gpio = {0};                 // GPIO config struct
    gpio.Pin = GPIO_PIN_6;                       // PC6 (TIM3_CH1)
    gpio.Mode = GPIO_MODE_AF_PP;                 // Alternate function, push-pull
    gpio.Pull = GPIO_NOPULL;                     // No pull
    gpio.Speed = GPIO_SPEED_FREQ_LOW;            // Low speed
    gpio.Alternate = GPIO_AF2_TIM3;              // AF2 = TIM3
    HAL_GPIO_Init(GPIOC, &gpio);                 // Init PC6

    TIM_OC_Init(&htim3, TIM3, TIMER_PRESCALER_1HZ, TIMER_PERIOD_1HZ); // Init TIM3 for output compare
    TIM_OC_ConfigChannel(&htim3, TIM_CHANNEL_1, TIMER_OC_PULSE);      // Set compare value
    TIM_OC_Start(&htim3, TIM_CHANNEL_1);         // Start output compare
}

void Timer_OutputCompare_IT_Example(void) {
    __HAL_RCC_TIM3_CLK_ENABLE();                 // Enable TIM3 clock
    __HAL_RCC_GPIOC_CLK_ENABLE();                // Enable GPIOC clock
    GPIO_InitTypeDef gpio = {0};                 // GPIO config struct
    gpio.Pin = GPIO_PIN_6;                       // PC6 (TIM3_CH1)
    gpio.Mode = GPIO_MODE_AF_PP;                 // Alternate function, push-pull
    gpio.Pull = GPIO_NOPULL;                     // No pull
    gpio.Speed = GPIO_SPEED_FREQ_LOW;            // Low speed
    gpio.Alternate = GPIO_AF2_TIM3;              // AF2 = TIM3
    HAL_GPIO_Init(GPIOC, &gpio);                 // Init PC6

    HAL_NVIC_SetPriority(TIM3_IRQn, 5, 0);      // Set TIM3 IRQ priority
    HAL_NVIC_EnableIRQ(TIM3_IRQn);              // Enable TIM3 IRQ

    TIM_OC_Init(&htim3, TIM3, TIMER_PRESCALER_1HZ, TIMER_PERIOD_1HZ); // Init TIM3 for output compare
    TIM_OC_ConfigChannel(&htim3, TIM_CHANNEL_1, TIMER_OC_PULSE);      // Set compare value
    TIM_OC_Start_IT(&htim3, TIM_CHANNEL_1);         // Start output compare
}

/**
 * @brief Configures TIM10 to generate a single output pulse.
 *        TIM10_CH1 is mapped to PB8 (AF3) per STM32F429 datasheet.
 *        GPIO: PB8, Mode: Alternate Function Push-Pull, AF3 (TIM10), Low speed, No pull.
 *        Useful for precise pulse generation.
 *        Real-world: Ultrasonic distance measurement, trigger signals, or camera strobe.
 */
void Timer_OnePulse_Example(void) {
    __HAL_RCC_TIM10_CLK_ENABLE();                // Enable TIM10 clock
    __HAL_RCC_GPIOB_CLK_ENABLE();                // Enable GPIOB clock
    GPIO_InitTypeDef gpio = {0};                 // GPIO config struct
    gpio.Pin = GPIO_PIN_8;                       // PB8 (TIM10_CH1)
    gpio.Mode = GPIO_MODE_AF_PP;                 // Alternate function, push-pull
    gpio.Pull = GPIO_NOPULL;                     // No pull
    gpio.Speed = GPIO_SPEED_FREQ_LOW;            // Low speed
    gpio.Alternate = GPIO_AF3_TIM10;             // AF3 = TIM10
    HAL_GPIO_Init(GPIOB, &gpio);                 // Init PB8

    TIM_OnePulse_InitTypeDef sConfigOP = {0};    // One-pulse config struct
    TIM_Base_Init(&htim10, TIM10, TIMER_PRESCALER_1HZ, TIMER_PERIOD_1HZ); // Init TIM10
    HAL_TIM_OnePulse_Init(&htim10, TIM_OPMODE_SINGLE); // One-pulse mode
    sConfigOP.OCMode = TIM_OCMODE_PWM2;          // Output compare mode
    sConfigOP.Pulse = TIMER_OC_PULSE;            // Pulse width
    sConfigOP.OCPolarity = TIM_OCPOLARITY_HIGH;  // Output polarity
    sConfigOP.ICPolarity = TIM_ICPOLARITY_RISING;// Input polarity
    sConfigOP.ICSelection = TIM_ICSELECTION_DIRECTTI; // Direct input
    HAL_TIM_OnePulse_ConfigChannel(&htim10, &sConfigOP, TIM_CHANNEL_1, TIM_CHANNEL_2); // Config channel
    HAL_TIM_OnePulse_Start(&htim10, TIM_CHANNEL_1); // Start one-pulse
}


void Timer_OnePulse_IT_Example(void) {
    __HAL_RCC_TIM10_CLK_ENABLE();                // Enable TIM10 clock
    __HAL_RCC_GPIOB_CLK_ENABLE();                // Enable GPIOB clock
    GPIO_InitTypeDef gpio = {0};                 // GPIO config struct
    gpio.Pin = GPIO_PIN_8;                       // PB8 (TIM10_CH1)
    gpio.Mode = GPIO_MODE_AF_PP;                 // Alternate function, push-pull
    gpio.Pull = GPIO_NOPULL;                     // No pull
    gpio.Speed = GPIO_SPEED_FREQ_LOW;            // Low speed
    gpio.Alternate = GPIO_AF3_TIM10;             // AF3 = TIM10
    HAL_GPIO_Init(GPIOB, &gpio);                 // Init PB8

    HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 5, 0);      // Set TIM10 IRQ priority
    HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);              // Enable TIM10 IRQ

    TIM_OnePulse_InitTypeDef sConfigOP = {0};    // One-pulse config struct
    TIM_Base_Init(&htim10, TIM10, TIMER_PRESCALER_1HZ, TIMER_PERIOD_1HZ); // Init TIM10
    HAL_TIM_OnePulse_Init(&htim10, TIM_OPMODE_SINGLE); // One-pulse mode
    sConfigOP.OCMode = TIM_OCMODE_PWM2;          // Output compare mode
    sConfigOP.Pulse = TIMER_OC_PULSE;            // Pulse width
    sConfigOP.OCPolarity = TIM_OCPOLARITY_HIGH;  // Output polarity
    sConfigOP.ICPolarity = TIM_ICPOLARITY_RISING;// Input polarity
    sConfigOP.ICSelection = TIM_ICSELECTION_DIRECTTI; // Direct input
    HAL_TIM_OnePulse_ConfigChannel(&htim10, &sConfigOP, TIM_CHANNEL_1, TIM_CHANNEL_2); // Config channel
    HAL_TIM_OnePulse_Start_IT(&htim10, TIM_CHANNEL_1); // Start one-pulse
}

/**
 * @brief Configures TIM1 to read a quadrature encoder (CH1 and CH2).
 *        TIM1_CH1 is mapped to PA8 (AF1), TIM1_CH2 to PA9 (AF1) per STM32F429 datasheet.
 *        GPIO: PA8|PA9, Mode: Alternate Function Push-Pull, AF1 (TIM1), Low speed, No pull.
 *        Decodes position and direction from encoder signals.
 *        Real-world: Robotics, motor control, or position sensing.
 */
void Timer_Encoder_Example(void) {
    __HAL_RCC_TIM1_CLK_ENABLE();                 // Enable TIM1 clock
    __HAL_RCC_GPIOA_CLK_ENABLE();                // Enable GPIOA clock
    GPIO_InitTypeDef gpio = {0};                 // GPIO config struct
    gpio.Pin = GPIO_PIN_8 | GPIO_PIN_9;          // PA8 (CH1), PA9 (CH2)
    gpio.Mode = GPIO_MODE_AF_PP;                 // Alternate function, push-pull
    gpio.Pull = GPIO_NOPULL;                     // No pull
    gpio.Speed = GPIO_SPEED_FREQ_LOW;            // Low speed
    gpio.Alternate = GPIO_AF1_TIM1;              // AF1 = TIM1
    HAL_GPIO_Init(GPIOA, &gpio);                 // Init PA8, PA9

    TIM_Encoder_Init(&htim1, TIM1, 0, TIMER_PERIOD_MAX); // Init TIM1 for encoder
    TIM_Encoder_Start(&htim1);                   // Start encoder mode
}

void Timer_Encoder_Example_IT(void) {
    __HAL_RCC_TIM1_CLK_ENABLE();                 // Enable TIM1 clock
    __HAL_RCC_GPIOA_CLK_ENABLE();                // Enable GPIOA clock
    GPIO_InitTypeDef gpio = {0};                 // GPIO config struct
    gpio.Pin = GPIO_PIN_8 | GPIO_PIN_9;          // PA8 (CH1), PA9 (CH2)
    gpio.Mode = GPIO_MODE_AF_PP;                 // Alternate function, push-pull
    gpio.Pull = GPIO_NOPULL;                     // No pull
    gpio.Speed = GPIO_SPEED_FREQ_LOW;            // Low speed
    gpio.Alternate = GPIO_AF1_TIM1;              // AF1 = TIM1
    HAL_GPIO_Init(GPIOA, &gpio);                 // Init PA8, PA9

    HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 5, 0);      // Set TIM10 IRQ priority
    HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);              // Enable TIM10 IRQ

    TIM_Encoder_Init(&htim1, TIM1, 0, TIMER_PERIOD_MAX); // Init TIM1 for encoder
    TIM_Encoder_Start_IT(&htim1);                   // Start encoder mode
}

/**
 * @brief Uses SysTick for 1ms tick, demonstrates delay and tick count. Relies on HAL_Init default setup.
 * @note  Real-world: System timekeeping, task scheduling, or timeouts.
 */
void Timer_SysTick_Example(void) {
    // HAL_Delay and HAL_GetTick use SysTick (1ms tick)
    HAL_Delay(SYSTICK_DELAY_1S); // Delay 1 second
    (void)HAL_GetTick(); // Get current tick in ms (suppress unused warning)
}

/**
 * @brief Uses DWT cycle counter for precise microsecond measurement. Measures code execution time in cycles and converts to microseconds.
 * @note  Real-world: Performance profiling, benchmarking, or precise timing analysis.
 */
 #define DWT_MICROSECONDS_PER_SECOND 1000000U
void DWT_Delay_us(uint32_t microseconds) {
    // Enable DWT if not already enabled
    if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    }
    if (!(DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk)) {
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
    uint32_t cycles = microseconds * (SystemCoreClock / DWT_MICROSECONDS_PER_SECOND);
    uint32_t start = DWT->CYCCNT;
    while ((DWT->CYCCNT - start) < cycles) {
        // Wait
    }
}

/**
 * @brief Synchronizes TIM2 (master) and TIM3 (slave) using internal trigger.
 *        TIM2_CH1 is mapped to PA0 (AF1), TIM3_CH1 to PA6 (AF2) per STM32F429 datasheet.
 *        GPIO: PA0 (AF1, TIM2), PA6 (AF2, TIM3), Mode: Alternate Function Push-Pull, Low speed, No pull.
 *        Demonstrates timer chaining for complex timing schemes.
 *        Real-world: Motor control, multi-phase PWM, or synchronized sampling.
 */
void Timer_MasterSlave_Sync_Example(void) {
    __HAL_RCC_TIM2_CLK_ENABLE();                 // Enable TIM2 clock
    __HAL_RCC_TIM3_CLK_ENABLE();                 // Enable TIM3 clock
    __HAL_RCC_GPIOA_CLK_ENABLE();                // Enable GPIOA clock
    GPIO_InitTypeDef gpio = {0};                 // GPIO config struct
    gpio.Pin = GPIO_PIN_0;                       // PA0 (TIM2_CH1)
    gpio.Mode = GPIO_MODE_AF_PP;                 // Alternate function, push-pull
    gpio.Pull = GPIO_NOPULL;                     // No pull
    gpio.Speed = GPIO_SPEED_FREQ_LOW;            // Low speed
    gpio.Alternate = GPIO_AF1_TIM2;              // AF1 = TIM2
    HAL_GPIO_Init(GPIOA, &gpio);                 // Init PA0
    gpio.Pin = GPIO_PIN_6;                       // PA6 (TIM3_CH1)
    gpio.Alternate = GPIO_AF2_TIM3;              // AF2 = TIM3
    HAL_GPIO_Init(GPIOA, &gpio);                 // Init PA6

    TIM_Base_Init(&htim2, TIM2, TIMER_PRESCALER_1HZ, TIMER_PERIOD_1HZ); // Init TIM2
    TIM_Base_Init(&htim3, TIM3, TIMER_PRESCALER_1HZ, TIMER_PERIOD_1HZ); // Init TIM3
    TIM_SyncMasterSlave(&htim2, TIM_TRGO_UPDATE, &htim3, TIM_SLAVEMODE_EXTERNAL1, TIM_TS_ITR1); // Sync TIM2->TIM3
    TIM_Base_Start(&htim2);                      // Start TIM2
    TIM_Base_Start(&htim3);                      // Start TIM3
}

void Timer_MasterSlave_Sync_IT_Example(void) {
    __HAL_RCC_TIM2_CLK_ENABLE();                 // Enable TIM2 clock
    __HAL_RCC_TIM3_CLK_ENABLE();                 // Enable TIM3 clock
    __HAL_RCC_GPIOA_CLK_ENABLE();                // Enable GPIOA clock
    GPIO_InitTypeDef gpio = {0};                 // GPIO config struct
    gpio.Pin = GPIO_PIN_0;                       // PA0 (TIM2_CH1)
    gpio.Mode = GPIO_MODE_AF_PP;                 // Alternate function, push-pull
    gpio.Pull = GPIO_NOPULL;                     // No pull
    gpio.Speed = GPIO_SPEED_FREQ_LOW;            // Low speed
    gpio.Alternate = GPIO_AF1_TIM2;              // AF1 = TIM2
    HAL_GPIO_Init(GPIOA, &gpio);                 // Init PA0
    gpio.Pin = GPIO_PIN_6;                       // PA6 (TIM3_CH1)
    gpio.Alternate = GPIO_AF2_TIM3;              // AF2 = TIM3
    HAL_GPIO_Init(GPIOA, &gpio);                 // Init PA6

    HAL_NVIC_SetPriority(TIM2_IRQn, 5, 0);      // Set TIM2 IRQ priority
    HAL_NVIC_EnableIRQ(TIM2_IRQn);              // Enable TIM2 IRQ

    HAL_NVIC_SetPriority(TIM3_IRQn, 5, 0);      // Set TIM3 IRQ priority
    HAL_NVIC_EnableIRQ(TIM3_IRQn);              // Enable TIM3 IRQ

    TIM_Base_Init(&htim2, TIM2, TIMER_PRESCALER_1HZ, TIMER_PERIOD_1HZ); // Init TIM2
    TIM_Base_Init(&htim3, TIM3, TIMER_PRESCALER_1HZ, TIMER_PERIOD_1HZ); // Init TIM3
    TIM_SyncMasterSlave(&htim2, TIM_TRGO_UPDATE, &htim3, TIM_SLAVEMODE_EXTERNAL1, TIM_TS_ITR1); // Sync TIM2->TIM3
    TIM_Interrupt_Start_IT(&htim2);                      // Start TIM2
    TIM_Interrupt_Start_IT(&htim3);                      // Start TIM3
}

void TIM1_UP_TIM10_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim10);
}

void TIM2_IRQHandler(void) { HAL_TIM_IRQHandler(&htim2); }
void TIM3_IRQHandler(void) { HAL_TIM_IRQHandler(&htim3); }
