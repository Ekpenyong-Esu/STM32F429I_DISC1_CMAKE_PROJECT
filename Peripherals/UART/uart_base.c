/**
  ******************************************************************************
  * @file    uart_base.c
  * @brief   UART initialization and configuration
  * @details Implements core UART initialization and hardware configuration
  * @version 1.0
  * @date    2025-04-19
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "uart.h"
#include "uart_internal.h"
#include "../SYS/sys.h"

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
uint8_t RxData;
volatile uint8_t RxReady = 0;

/**
  * @brief  UART Initialization Function
  * @details Configures the USART1 peripheral with the following settings:
  *          - Baud rate: 115200 bps
  *          - Word length: 8 bits
  *          - Stop bits: 1 bit
  *          - Parity: None
  *          - Mode: Both transmit and receive enabled
  *          - Hardware flow control: None
  *          - Oversampling: 16 samples per bit
  *
  * @note   USART1 is commonly used for debug console or communication
  *         with a host computer
  * @param  None
  * @retval None
  */
void UART_Init(void)
{
  /* UART1 initialization */
  huart1.Instance = USART1;                      /* Select USART1 peripheral */
  huart1.Init.BaudRate = 115200;                 /* Communication speed: 115200 bits/s */
  huart1.Init.WordLength = UART_WORDLENGTH_8B;   /* 8-bit data length */
  huart1.Init.StopBits = UART_STOPBITS_1;        /* 1 stop bit */
  huart1.Init.Parity = UART_PARITY_NONE;         /* No parity bit */
  huart1.Init.Mode = UART_MODE_TX_RX;            /* Enable both transmit and receive */
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;   /* No hardware flow control */
  huart1.Init.OverSampling = UART_OVERSAMPLING_16; /* 16 samples per bit */

  /* Initialize the UART peripheral with the specified parameters */
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();  /* Call error handler if initialization fails */
  }

  /* Start receiving data in interrupt mode */
  HAL_UART_Receive_IT(&huart1, &RxData, 1);
}

/**
  * @brief  UART MSP Initialization
  * @details This function configures:
  *          - GPIO pins for UART1:
  *            - TX: PA9 (AF7)
  *            - RX: PA10 (AF7)
  *          - Enables peripheral clocks
  *          - Configures NVIC for UART interrupts
  *
  * @note   This function is called by HAL_UART_Init()
  * @param  huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  if(huart->Instance == USART1)
  {
    /* Enable peripheral clocks */
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* USART1 GPIO Configuration: PA9=USART1_TX, PA10=USART1_RX */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* UART1 interrupt configuration */
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  }
}
