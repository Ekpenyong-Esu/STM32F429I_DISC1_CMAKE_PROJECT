/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f429xx.h"

#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* Include modular peripheral headers */
#include "../../Peripherals/SYS/sys.h"
#include "../../Peripherals/RTOS/rtos.h"
#include "../../Peripherals/GPIO/gpio.h"
#include "../../Peripherals/CRC/crc.h"
#include "../../Peripherals/DMA2D/dma2d.h"
#include "../../Peripherals/FMC/fmc.h"
#include "../../Peripherals/I2C/i2c.h"
#include "../../Peripherals/LTDC/ltdc.h"
#include "../../Peripherals/SPI/spi.h"
#include "../../Peripherals/TIM/tim.h"
#include "../../Peripherals/UART/uart.h"

/* USER CODE END Includes */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* Initialize system components */
  SYS_Init();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all peripherals using the modular architecture */
  GPIO_Init();
  CRC_Init();
  DMA2D_Init();
  FMC_Init();
  I2C_Init();
  LTDC_Init();
  SPI_Init();
  TIM_Init();
  UART_Init();

  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Initialize and start RTOS */
  RTOS_Init();
  //RTOS_Start();



  /* Infinite loop - should never get here */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    printf("Hello from main.c!\n");
    /* USER CODE END WHILE */
    HAL_GPIO_TogglePin(GPIOG, LD3_Pin|LD4_Pin);
    HAL_Delay(100);
    HAL_GPIO_LockPin(GPIOG, LD3_Pin|LD4_Pin);

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,*/
  printf("Wrong parameters value: file %s on line %d\r\n", file, line)
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
