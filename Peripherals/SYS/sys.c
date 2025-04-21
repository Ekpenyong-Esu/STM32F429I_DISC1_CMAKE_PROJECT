/**
  ******************************************************************************
  * @file    sys.c
  * @brief   System module implementation
  * @details This file provides code for core system configuration
  *          including clock setup and error handling.
  * @version 1.0
  * @date    2025-04-15
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "sys.h"

/**
  * @brief  System Initialization Function
  * @details Performs fundamental system initialization by:
  *          1. Initializing the HAL library
  *          2. Configuring the Flash interface and SysTick
  *          3. Setting up the system clock
  *
  * @note   This function should be called at the beginning of the
  *         application before any other initialization
  * @param  None
  * @retval None
  */
void SYS_Init(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();
}

/**
  * @brief  System Clock Configuration
  * @details Configures the STM32F429 clock system with the following settings:
  *          - HSE used as PLL source (8 MHz external crystal)
  *          - PLL configured for 168 MHz system clock
  *          - Flash latency set to 5 wait states for 168 MHz
  *          - AHB bus running at system clock frequency (168 MHz)
  *          - APB1 peripheral clock at 42 MHz (HCLK/4)
  *          - APB2 peripheral clock at 84 MHz (HCLK/2)
  *          - Power regulator output voltage scale 1 (highest performance)
  *
  * @note   This configuration provides maximum CPU performance
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};  /* RCC oscillator configuration structure */
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};  /* RCC clock configuration structure */

  /* Configure the main internal regulator output voltage */
  __HAL_RCC_PWR_CLK_ENABLE();                  /* Enable power controller clock */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1); /* Set voltage scale for max performance */

  /* Initialize the RCC Oscillators according to the specified parameters
     in the RCC_OscInitTypeDef structure */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE; /* Use High-Speed External oscillator */
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;                   /* Enable HSE oscillator */
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;               /* Enable PLL */
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;       /* Use HSE as PLL source */
  RCC_OscInitStruct.PLL.PLLM = 4;                            /* HSE divider for PLL entry: 8MHz/4 = 2MHz */
  RCC_OscInitStruct.PLL.PLLN = 168;                          /* PLL multiplication factor: 2MHz*168 = 336MHz */
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;                /* PLL division for system clock: 336MHz/2 = 168MHz */
  RCC_OscInitStruct.PLL.PLLQ = 7;                            /* PLL division for USB OTG FS: 336MHz/7 = 48MHz */

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();  /* Call error handler if configuration fails */
  }

  /* Initialize the CPU, AHB and APB buses clocks */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2; /* Configure all clocks */
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; /* Use PLL as system clock source */
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;        /* AHB clock = SYSCLK (168MHz) */
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;         /* APB1 clock = HCLK/4 (42MHz) */
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;         /* APB2 clock = HCLK/2 (84MHz) */

  /* Set the appropriate flash latency for 168MHz operation */
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();  /* Call error handler if configuration fails */
  }
}

/**
  * @brief  Error Handler Function
  * @details This function is executed in case of error occurrence.
  *          It implements a simple infinite loop with interrupts disabled
  *          to prevent further execution when a critical error occurs.
  *
  * @note   This is a basic implementation. Consider adding:
  *         - LED blinking for visual error indication
  *         - Error code storage in a non-volatile memory
  *         - Watchdog reset after a timeout
  *         - Debug message output if debug interface is available
  *
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();               /* Disable all interrupts */
  while (1)                      /* Enter infinite loop to prevent further execution */
  {
    /* Could add LED blinking code here for visual error indication */
  }
}
