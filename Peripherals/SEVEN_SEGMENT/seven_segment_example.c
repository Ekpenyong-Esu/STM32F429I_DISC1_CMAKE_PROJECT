/**
 * @file    seven_segment_example.c
 * @brief   Seven-Segment Display Driver Usage Examples
 * @details Demonstrates how to use the seven-segment display driver
 *          with both GPIO and HT1621 modes
 * @version 1.0
 * @date    2026-01-03
 */

/* Includes ------------------------------------------------------------------*/
#include "seven_segment.h"
#include "stm32f4xx_hal.h"

/* Private variables ---------------------------------------------------------*/
static SegDisplayHandle_t displayHandle;
static SegGpioPin_t digitPins[4];  /* For 4-digit display */

/* Example 1: GPIO Mode - Common Cathode 4-Digit Display
 * ======================================================
 * Wiring:
 *   Segments A-G, DP connect to MCU GPIOs via current-limiting resistors
 *   Digit commons connect to MCU GPIOs via transistors (NPN for cathode)
 */
void SevenSeg_Example_GPIO_CommonCathode(void)
{
    SegDisplayConfig_t config = {0};

    /* Enable GPIO clocks (adjust based on your pins) */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* Set driver type */
    config.driverType = SEG_DRIVER_GPIO;

    /* Configure segment pins (A-G + DP)
     * Adjust ports and pins based on your hardware */
    config.config.gpio.segments[SEG_A].port  = GPIOA;
    config.config.gpio.segments[SEG_A].pin   = GPIO_PIN_0;
    config.config.gpio.segments[SEG_B].port  = GPIOA;
    config.config.gpio.segments[SEG_B].pin   = GPIO_PIN_1;
    config.config.gpio.segments[SEG_C].port  = GPIOA;
    config.config.gpio.segments[SEG_C].pin   = GPIO_PIN_2;
    config.config.gpio.segments[SEG_D].port  = GPIOA;
    config.config.gpio.segments[SEG_D].pin   = GPIO_PIN_3;
    config.config.gpio.segments[SEG_E].port  = GPIOA;
    config.config.gpio.segments[SEG_E].pin   = GPIO_PIN_4;
    config.config.gpio.segments[SEG_F].port  = GPIOA;
    config.config.gpio.segments[SEG_F].pin   = GPIO_PIN_5;
    config.config.gpio.segments[SEG_G].port  = GPIOA;
    config.config.gpio.segments[SEG_G].pin   = GPIO_PIN_6;
    config.config.gpio.segments[SEG_DP].port = GPIOA;
    config.config.gpio.segments[SEG_DP].pin  = GPIO_PIN_7;

    /* Configure digit select pins */
    digitPins[0].port = GPIOB;
    digitPins[0].pin  = GPIO_PIN_0;
    digitPins[1].port = GPIOB;
    digitPins[1].pin  = GPIO_PIN_1;
    digitPins[2].port = GPIOB;
    digitPins[2].pin  = GPIO_PIN_2;
    digitPins[3].port = GPIOB;
    digitPins[3].pin  = GPIO_PIN_3;

    config.config.gpio.digits = digitPins;
    config.config.gpio.digitCount = 4;

    /* Common cathode: segments are active HIGH */
    config.config.gpio.polarity = SEG_COMMON_CATHODE;

    /* Digit select is active HIGH (using NPN transistors) */
    config.config.gpio.digitActiveHigh = true;

    config.multiplexDelayUs = 2000;  /* 2ms per digit */
    config.leadingZeros = false;

    /* Initialize display */
    if (Seg_Init(&displayHandle, &config) == SEG_OK) {
        /* Display test pattern */
        Seg_Test(&displayHandle);
        HAL_Delay(1000);

        /* Display a number */
        Seg_DisplayInt(&displayHandle, 1234);
    }
}

/* Example 2: GPIO Mode - Common Anode 4-Digit Display
 * ====================================================
 * Wiring:
 *   Segments A-G, DP connect to MCU GPIOs via current-limiting resistors
 *   Digit commons connect to MCU GPIOs via transistors (PNP for anode)
 */
void SevenSeg_Example_GPIO_CommonAnode(void)
{
    SegDisplayConfig_t config = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    config.driverType = SEG_DRIVER_GPIO;

    /* Configure segment pins (same as above) */
    config.config.gpio.segments[SEG_A].port  = GPIOA;
    config.config.gpio.segments[SEG_A].pin   = GPIO_PIN_0;
    config.config.gpio.segments[SEG_B].port  = GPIOA;
    config.config.gpio.segments[SEG_B].pin   = GPIO_PIN_1;
    config.config.gpio.segments[SEG_C].port  = GPIOA;
    config.config.gpio.segments[SEG_C].pin   = GPIO_PIN_2;
    config.config.gpio.segments[SEG_D].port  = GPIOA;
    config.config.gpio.segments[SEG_D].pin   = GPIO_PIN_3;
    config.config.gpio.segments[SEG_E].port  = GPIOA;
    config.config.gpio.segments[SEG_E].pin   = GPIO_PIN_4;
    config.config.gpio.segments[SEG_F].port  = GPIOA;
    config.config.gpio.segments[SEG_F].pin   = GPIO_PIN_5;
    config.config.gpio.segments[SEG_G].port  = GPIOA;
    config.config.gpio.segments[SEG_G].pin   = GPIO_PIN_6;
    config.config.gpio.segments[SEG_DP].port = GPIOA;
    config.config.gpio.segments[SEG_DP].pin  = GPIO_PIN_7;

    digitPins[0].port = GPIOB;
    digitPins[0].pin  = GPIO_PIN_0;
    digitPins[1].port = GPIOB;
    digitPins[1].pin  = GPIO_PIN_1;
    digitPins[2].port = GPIOB;
    digitPins[2].pin  = GPIO_PIN_2;
    digitPins[3].port = GPIOB;
    digitPins[3].pin  = GPIO_PIN_3;

    config.config.gpio.digits = digitPins;
    config.config.gpio.digitCount = 4;

    /* Common anode: segments are active LOW (inverted internally) */
    config.config.gpio.polarity = SEG_COMMON_ANODE;

    /* Digit select is active LOW (using PNP transistors) */
    config.config.gpio.digitActiveHigh = false;

    config.multiplexDelayUs = 2000;
    config.leadingZeros = false;

    if (Seg_Init(&displayHandle, &config) == SEG_OK) {
        Seg_DisplayInt(&displayHandle, 5678);
    }
}

/* Example 3: HT1621 LCD Driver Mode
 * ==================================
 * Wiring:
 *   HT1621 CS   -> MCU GPIO
 *   HT1621 WR   -> MCU GPIO
 *   HT1621 DATA -> MCU GPIO
 *   HT1621 VDD  -> 3.3V or 5V
 *   HT1621 VSS  -> GND
 */
void SevenSeg_Example_HT1621(void)
{
    SegDisplayConfig_t config = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();

    config.driverType = SEG_DRIVER_HT1621;

    /* Configure HT1621 pins */
    config.config.ht1621.pins.csPort   = GPIOC;
    config.config.ht1621.pins.csPin    = GPIO_PIN_0;
    config.config.ht1621.pins.wrPort   = GPIOC;
    config.config.ht1621.pins.wrPin    = GPIO_PIN_1;
    config.config.ht1621.pins.dataPort = GPIOC;
    config.config.ht1621.pins.dataPin  = GPIO_PIN_2;

    config.config.ht1621.digitCount = 6;  /* 6-digit LCD */
    config.config.ht1621.bias = 3;        /* 1/3 bias */
    config.config.ht1621.commons = 4;     /* 4 commons */
    config.config.ht1621.segmentMap = NULL;  /* Use default mapping */

    config.leadingZeros = false;

    if (Seg_Init(&displayHandle, &config) == SEG_OK) {
        /* Display test */
        Seg_Test(&displayHandle);
        HAL_Delay(1000);

        /* Display temperature with decimal */
        Seg_DisplayFloat(&displayHandle, 25.6f, 1);
    }
}

/* Example 4: Timer-based Multiplexing for GPIO Mode
 * ==================================================
 * Use a hardware timer to call Seg_Update() for flicker-free display
 */

/* Timer interrupt handler - add this to your stm32f4xx_it.c */
void SevenSeg_TimerCallback(void)
{
    /* Call Seg_Update() at ~400Hz for 4-digit display (100Hz per digit) */
    Seg_Update(&displayHandle);
}

/* Timer initialization example */
void SevenSeg_Example_TimerInit(void)
{
    /* Example: Use TIM6 for display multiplexing
     * Configure timer to generate interrupt at 400Hz
     *
     * Timer clock = 84MHz (APB1)
     * Prescaler = 8399 -> 10kHz
     * Period = 24 -> ~400Hz
     */

    /* Enable TIM6 clock */
    __HAL_RCC_TIM6_CLK_ENABLE();

    TIM_HandleTypeDef htim6 = {0};
    htim6.Instance = TIM6;
    htim6.Init.Prescaler = 8399;
    htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim6.Init.Period = 24;
    htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    HAL_TIM_Base_Init(&htim6);

    /* Enable update interrupt */
    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);

    /* Start timer with interrupt */
    HAL_TIM_Base_Start_IT(&htim6);
}

/* Example 5: Display Various Data Types
 * ======================================
 */
void SevenSeg_Example_Display(void)
{
    /* Assume display is already initialized */

    /* Display integer */
    Seg_DisplayInt(&displayHandle, 1234);
    HAL_Delay(2000);

    /* Display negative number */
    Seg_DisplayInt(&displayHandle, -99);
    HAL_Delay(2000);

    /* Display float with 2 decimal places */
    Seg_DisplayFloat(&displayHandle, 12.34f, 2);
    HAL_Delay(2000);

    /* Display hexadecimal */
    Seg_DisplayHex(&displayHandle, 0xABCD);
    HAL_Delay(2000);

    /* Display string */
    Seg_DisplayString(&displayHandle, "HELP");
    HAL_Delay(2000);

    /* Display individual characters */
    Seg_SetChar(&displayHandle, 0, 'H');
    Seg_SetChar(&displayHandle, 1, 'i');
    Seg_SetChar(&displayHandle, 2, ' ');
    Seg_SetChar(&displayHandle, 3, '!');
    HAL_Delay(2000);

    /* Display custom pattern */
    Seg_SetPattern(&displayHandle, 0, 0x49);  /* Custom pattern */
    HAL_Delay(2000);

    /* Clear display */
    Seg_Clear(&displayHandle);
}

/* Example 6: Simple Counter
 * ==========================
 */
void SevenSeg_Example_Counter(void)
{
    /* Initialize as common cathode GPIO display first */
    SevenSeg_Example_GPIO_CommonCathode();

    int32_t count = 0;

    while (1) {
        Seg_DisplayInt(&displayHandle, count);

        /* For GPIO mode, call Seg_Update() in a loop or timer */
        for (int i = 0; i < 100; i++) {
            Seg_Update(&displayHandle);
            HAL_Delay(1);  /* ~1ms per update, 25 updates per digit */
        }

        count++;
        if (count > 9999) {
            count = 0;
        }
    }
}

/* Example 7: Single Digit Display
 * ================================
 */
void SevenSeg_Example_SingleDigit(void)
{
    SegDisplayConfig_t config = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    config.driverType = SEG_DRIVER_GPIO;

    /* Configure segment pins only (no digit select needed) */
    config.config.gpio.segments[SEG_A].port  = GPIOA;
    config.config.gpio.segments[SEG_A].pin   = GPIO_PIN_0;
    config.config.gpio.segments[SEG_B].port  = GPIOA;
    config.config.gpio.segments[SEG_B].pin   = GPIO_PIN_1;
    config.config.gpio.segments[SEG_C].port  = GPIOA;
    config.config.gpio.segments[SEG_C].pin   = GPIO_PIN_2;
    config.config.gpio.segments[SEG_D].port  = GPIOA;
    config.config.gpio.segments[SEG_D].pin   = GPIO_PIN_3;
    config.config.gpio.segments[SEG_E].port  = GPIOA;
    config.config.gpio.segments[SEG_E].pin   = GPIO_PIN_4;
    config.config.gpio.segments[SEG_F].port  = GPIOA;
    config.config.gpio.segments[SEG_F].pin   = GPIO_PIN_5;
    config.config.gpio.segments[SEG_G].port  = GPIOA;
    config.config.gpio.segments[SEG_G].pin   = GPIO_PIN_6;
    config.config.gpio.segments[SEG_DP].port = GPIOA;
    config.config.gpio.segments[SEG_DP].pin  = GPIO_PIN_7;

    /* Single digit - no multiplexing needed */
    digitPins[0].port = NULL;  /* No digit select */
    digitPins[0].pin  = 0;

    config.config.gpio.digits = digitPins;
    config.config.gpio.digitCount = 1;
    config.config.gpio.polarity = SEG_COMMON_CATHODE;
    config.config.gpio.digitActiveHigh = true;

    if (Seg_Init(&displayHandle, &config) == SEG_OK) {
        /* Count from 0-9 */
        for (uint8_t i = 0; i <= 9; i++) {
            Seg_SetDigit(&displayHandle, 0, i, false);
            Seg_Update(&displayHandle);  /* For single digit, writes directly */
            HAL_Delay(500);
        }
    }
}
