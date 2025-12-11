/**
 * @file ir_example.c
 * @brief IR driver usage examples for infrared communication
 * @details This file provides examples of how to use the IR driver
 *          for infrared transmission and reception on STM32F429 Discovery board.
 * @version 1.0
 * @date 2025-09-03
 * @author STM32 Team
 */

/* Includes ------------------------------------------------------------------*/
#include "ir_example.h"
#include "ir.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>

/* Private variables ---------------------------------------------------------*/
static IR_Handle_t hir;
static TIM_HandleTypeDef htim3;  /* Timer for carrier generation */
static TIM_HandleTypeDef htim4;  /* Timer for input capture */

/* Private function prototypes -----------------------------------------------*/
static void IR_Example_GPIO_Init(void);
static void IR_Example_Timer_Init(void);
static void IR_Example_PrintFrame(IR_Frame_t *frame);
static void IR_Example_PrintError(uint32_t error);
static void IR_Example_EventCallback(IR_Event_t event, IR_Frame_t *frame);

/* Example constants */
#define IR_TX_PORT                GPIOB
#define IR_TX_PIN                 GPIO_PIN_5
#define IR_RX_PORT                GPIOB
#define IR_RX_PIN                 GPIO_PIN_6
#define IR_TX_CHANNEL             TIM_CHANNEL_2
#define IR_RX_CHANNEL             TIM_CHANNEL_1

/**
 * @brief Basic IR initialization example
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_Example_Basic_Init(void) {
    printf("IR Basic Initialization Example\r\n");
    printf("================================\r\n");

    /* Initialize GPIO and Timers */
    IR_Example_GPIO_Init();
    IR_Example_Timer_Init();

    /* Configure IR settings */
    IR_Config_t config = {
        .protocol = IR_PROTOCOL_NEC,
        .carrierFreq = IR_CARRIER_FREQUENCY,
        .dutyCycle = IR_CARRIER_DUTY_CYCLE,
        .tolerance = IR_NEC_TOLERANCE,
        .autoRepeat = true,
        .invertSignal = false
    };

    /* Initialize IR driver */
    HAL_StatusTypeDef status = IR_Init(&hir, &htim3, &htim4, IR_TX_PORT, IR_TX_PIN,
                                      IR_RX_PORT, IR_RX_PIN, IR_TX_CHANNEL,
                                      IR_RX_CHANNEL, &config);

    if (status == HAL_OK) {
        printf("IR driver initialized successfully!\r\n");

        /* Set event callback */
        IR_SetEventCallback(&hir, IR_Example_EventCallback);
        printf("Event callback registered\r\n");
    } else {
        printf("IR driver initialization failed!\r\n");
        IR_Example_PrintError(IR_GetError(&hir));
    }

    printf("\r\n");
    return status;
}

/**
 * @brief IR transmitter example (NEC protocol)
 * @param address: Device address
 * @param command: Command code
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_Example_Transmit_NEC(uint8_t address, uint8_t command) {
    printf("IR NEC Transmission Example\r\n");
    printf("Address: 0x%02X, Command: 0x%02X\r\n", address, command);
    printf("===========================\r\n");

    HAL_StatusTypeDef status = IR_TransmitNEC(&hir, address, command);

    if (status == HAL_OK) {
        printf("NEC frame transmitted successfully!\r\n");

        /* Wait for transmission to complete */
        uint32_t startTime = HAL_GetTick();
        while (IR_GetState(&hir) == IR_STATE_TRANSMITTING &&
               (HAL_GetTick() - startTime) < 1000) {
            HAL_Delay(1);
        }

        if (IR_GetState(&hir) == IR_STATE_IDLE) {
            printf("Transmission completed\r\n");
        } else {
            printf("Transmission timeout\r\n");
            status = HAL_TIMEOUT;
        }
    } else {
        printf("NEC transmission failed!\r\n");
        IR_Example_PrintError(IR_GetError(&hir));
    }

    printf("\r\n");
    return status;
}

/**
 * @brief IR transmitter example (RC5 protocol)
 * @param address: Device address
 * @param command: Command code
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_Example_Transmit_RC5(uint8_t address, uint8_t command) {
    printf("IR RC5 Transmission Example\r\n");
    printf("Address: 0x%02X, Command: 0x%02X\r\n", address, command);
    printf("===========================\r\n");

    HAL_StatusTypeDef status = IR_TransmitRC5(&hir, address, command);

    if (status == HAL_OK) {
        printf("RC5 frame transmitted successfully!\r\n");

        /* Wait for transmission to complete */
        uint32_t startTime = HAL_GetTick();
        while (IR_GetState(&hir) == IR_STATE_TRANSMITTING &&
               (HAL_GetTick() - startTime) < 1000) {
            HAL_Delay(1);
        }

        printf("Transmission completed\r\n");
    } else {
        printf("RC5 transmission failed!\r\n");
        IR_Example_PrintError(IR_GetError(&hir));
    }

    printf("\r\n");
    return status;
}

/**
 * @brief IR transmitter example (SIRC protocol)
 * @param address: Device address
 * @param command: Command code
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_Example_Transmit_SIRC(uint8_t address, uint8_t command) {
    printf("IR SIRC Transmission Example\r\n");
    printf("Address: 0x%02X, Command: 0x%02X\r\n", address, command);
    printf("============================\r\n");

    HAL_StatusTypeDef status = IR_TransmitSIRC(&hir, address, command);

    if (status == HAL_OK) {
        printf("SIRC frame transmitted successfully!\r\n");

        /* Wait for transmission to complete */
        uint32_t startTime = HAL_GetTick();
        while (IR_GetState(&hir) == IR_STATE_TRANSMITTING &&
               (HAL_GetTick() - startTime) < 1000) {
            HAL_Delay(1);
        }

        printf("Transmission completed\r\n");
    } else {
        printf("SIRC transmission failed!\r\n");
        IR_Example_PrintError(IR_GetError(&hir));
    }

    printf("\r\n");
    return status;
}

/**
 * @brief IR receiver example
 * @param timeout_ms: Timeout in milliseconds
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_Example_Receive(uint32_t timeout_ms) {
    printf("IR Receiver Example\r\n");
    printf("Timeout: %u ms\r\n", timeout_ms);
    printf("===================\r\n");

    /* Start receiver */
    HAL_StatusTypeDef status = IR_StartReceive(&hir);
    if (status != HAL_OK) {
        printf("Failed to start receiver!\r\n");
        IR_Example_PrintError(IR_GetError(&hir));
        return status;
    }

    printf("Waiting for IR signal...\r\n");

    /* Wait for frame or timeout */
    uint32_t startTime = HAL_GetTick();
    IR_Frame_t frame;
    bool frameReceived = false;

    while ((HAL_GetTick() - startTime) < timeout_ms && !frameReceived) {
        if (IR_GetFrame(&hir, &frame) == HAL_OK) {
            printf("Frame received!\r\n");
            IR_Example_PrintFrame(&frame);
            frameReceived = true;
        }
        HAL_Delay(10);
    }

    /* Stop receiver */
    IR_StopReceive(&hir);

    if (!frameReceived) {
        printf("No frame received within timeout\r\n");
        status = HAL_TIMEOUT;
    }

    printf("\r\n");
    return status;
}

/**
 * @brief Continuous IR receiver example
 * @param duration_ms: Duration in milliseconds
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_Example_Continuous_Receive(uint32_t duration_ms) {
    printf("IR Continuous Receiver Example\r\n");
    printf("Duration: %u ms\r\n", duration_ms);
    printf("==============================\r\n");

    /* Start receiver */
    HAL_StatusTypeDef status = IR_StartReceive(&hir);
    if (status != HAL_OK) {
        printf("Failed to start receiver!\r\n");
        return status;
    }

    printf("Listening for IR signals...\r\n");

    uint32_t startTime = HAL_GetTick();
    uint32_t frameCount = 0;
    IR_Frame_t frame;

    while ((HAL_GetTick() - startTime) < duration_ms) {
        if (IR_GetFrame(&hir, &frame) == HAL_OK) {
            frameCount++;
            printf("Frame #%u: ", frameCount);
            IR_Example_PrintFrame(&frame);
        }
        HAL_Delay(10);
    }

    /* Stop receiver */
    IR_StopReceive(&hir);

    printf("Continuous reception completed. Total frames: %u\r\n", frameCount);
    printf("\r\n");
    return HAL_OK;
}

/**
 * @brief Custom IR transmission example
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_Example_Custom_Transmit(void) {
    printf("IR Custom Transmission Example\r\n");
    printf("===============================\r\n");

    /* Create custom pulse sequence */
    IR_Pulse_t customPulses[] = {
        {9000, 4500},   /* Header */
        {562, 1687},    /* Bit 1 */
        {562, 562},     /* Bit 0 */
        {562, 1687},    /* Bit 1 */
        {562, 562},     /* Bit 0 */
        {562, 0}        /* Stop bit */
    };

    HAL_StatusTypeDef status = IR_TransmitCustom(&hir, customPulses,
                                                sizeof(customPulses) / sizeof(customPulses[0]));

    if (status == HAL_OK) {
        printf("Custom frame transmitted successfully!\r\n");
        printf("Pulse sequence: Header + 1010 + Stop\r\n");
    } else {
        printf("Custom transmission failed!\r\n");
        IR_Example_PrintError(IR_GetError(&hir));
    }

    printf("\r\n");
    return status;
}

/**
 * @brief IR carrier frequency configuration example
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_Example_Configure_Carrier(void) {
    printf("IR Carrier Configuration Example\r\n");
    printf("================================\r\n");

    uint32_t frequencies[] = {36000, 38000, 40000, 56000};
    uint8_t dutyCycles[] = {25, 33, 50};

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            printf("Testing %u Hz, %u%% duty cycle...\r\n",
                   frequencies[i], dutyCycles[j]);

            HAL_StatusTypeDef status = IR_ConfigureCarrier(&hir, frequencies[i], dutyCycles[j]);
            if (status == HAL_OK) {
                printf("  Configuration successful\r\n");

                /* Test transmission */
                status = IR_TransmitNEC(&hir, 0x00, 0x01);
                if (status == HAL_OK) {
                    printf("  Test transmission OK\r\n");
                } else {
                    printf("  Test transmission failed\r\n");
                }
            } else {
                printf("  Configuration failed\r\n");
            }

            HAL_Delay(500);
        }
    }

    /* Reset to default */
    IR_ConfigureCarrier(&hir, IR_CARRIER_FREQUENCY, IR_CARRIER_DUTY_CYCLE);
    printf("Reset to default configuration\r\n");

    printf("\r\n");
    return HAL_OK;
}

/**
 * @brief IR protocol comparison example
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_Example_Protocol_Comparison(void) {
    printf("IR Protocol Comparison Example\r\n");
    printf("==============================\r\n");

    uint8_t address = 0x01;
    uint8_t command = 0x02;

    /* Test NEC protocol */
    printf("NEC Protocol:\r\n");
    printf("  32 bits total (address + ~address + command + ~command)\r\n");
    printf("  9ms header, 562us bit timing\r\n");
    IR_Example_Transmit_NEC(address, command);

    /* Test RC5 protocol */
    printf("RC5 Protocol:\r\n");
    printf("  14 bits total (start + toggle + address + command)\r\n");
    printf("  Manchester encoding, 1.778ms bit time\r\n");
    IR_Example_Transmit_RC5(address & 0x1F, command & 0x3F);

    /* Test SIRC protocol */
    printf("SIRC Protocol:\r\n");
    printf("  12 bits total (command + address)\r\n");
    printf("  2.4ms header, 600us bit timing\r\n");
    IR_Example_Transmit_SIRC(address & 0x1F, command & 0x7F);

    printf("Protocol comparison completed\r\n");
    printf("\r\n");
    return HAL_OK;
}

/**
 * @brief Comprehensive IR test example
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_Example_Comprehensive_Test(void) {
    printf("IR Comprehensive Test\r\n");
    printf("=====================\r\n");

    /* Initialize */
    if (IR_Example_Basic_Init() != HAL_OK) {
        return HAL_ERROR;
    }

    /* Test transmissions */
    IR_Example_Transmit_NEC(0x12, 0x34);
    IR_Example_Transmit_RC5(0x05, 0x15);
    IR_Example_Transmit_SIRC(0x07, 0x25);

    /* Test custom transmission */
    IR_Example_Custom_Transmit();

    /* Test carrier configuration */
    IR_Example_Configure_Carrier();

    /* Test protocol comparison */
    IR_Example_Protocol_Comparison();

    /* Test receiver for 5 seconds */
    printf("Testing receiver for 5 seconds...\r\n");
    IR_Example_Continuous_Receive(5000);

    printf("Comprehensive test completed!\r\n");
    return HAL_OK;
}

/**
 * @brief Cleanup IR resources
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_Example_Cleanup(void) {
    printf("IR Cleanup\r\n");
    printf("==========\r\n");

    HAL_StatusTypeDef status = IR_DeInit(&hir);

    if (status == HAL_OK) {
        printf("IR driver deinitialized successfully!\r\n");
    } else {
        printf("IR driver deinitialization failed!\r\n");
    }

    printf("\r\n");
    return status;
}

/* Private Functions ---------------------------------------------------------*/

/**
 * @brief Initialize GPIO for IR
 */
static void IR_Example_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIO clocks */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* Configure TX pin (PB5) for PWM output */
    GPIO_InitStruct.Pin = IR_TX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(IR_TX_PORT, &GPIO_InitStruct);

    /* Configure RX pin (PB6) for input capture */
    GPIO_InitStruct.Pin = IR_RX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
    HAL_GPIO_Init(IR_RX_PORT, &GPIO_InitStruct);
}

/**
 * @brief Initialize timers for IR
 */
static void IR_Example_Timer_Init(void) {
    /* Enable timer clocks */
    __HAL_RCC_TIM3_CLK_ENABLE();
    __HAL_RCC_TIM4_CLK_ENABLE();

    /* Configure TIM3 for carrier generation (TX) */
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 0;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 1000;  /* Will be configured by driver */
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_PWM_Init(&htim3);

    /* Configure TIM4 for input capture (RX) */
    htim4.Instance = TIM4;
    htim4.Init.Prescaler = 83;  /* 1MHz clock for microsecond resolution */
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = 0xFFFFFFFF;
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_IC_Init(&htim4);

    /* Configure input capture channel */
    TIM_IC_InitTypeDef sConfigIC = {0};
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_BOTHEDGE;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0;
    HAL_TIM_IC_ConfigChannel(&htim4, &sConfigIC, IR_RX_CHANNEL);
}

/**
 * @brief Print IR frame information
 * @param frame: Pointer to IR frame
 */
static void IR_Example_PrintFrame(IR_Frame_t *frame) {
    const char* protocolNames[] = {"NEC", "RC5", "SIRC", "CUSTOM"};

    printf("Protocol: %s, Address: 0x%02X, Command: 0x%02X, Data: 0x%08X, Bits: %u",
           protocolNames[frame->protocol],
           (unsigned int)frame->address,
           (unsigned int)frame->command,
           (unsigned int)frame->data,
           frame->bits);

    if (frame->repeat) {
        printf(" [REPEAT]");
    }
    printf("\r\n");
}

/**
 * @brief Print error information
 * @param error: Error code
 */
static void IR_Example_PrintError(uint32_t error) {
    printf("Error code: 0x%08X - ", error);

    switch (error) {
        case IR_ERROR_NONE:
            printf("No error\r\n");
            break;
        case IR_ERROR_INIT:
            printf("Initialization error\r\n");
            break;
        case IR_ERROR_INVALID_PARAM:
            printf("Invalid parameter\r\n");
            break;
        case IR_ERROR_TIMEOUT:
            printf("Timeout error\r\n");
            break;
        case IR_ERROR_PROTOCOL:
            printf("Protocol error\r\n");
            break;
        case IR_ERROR_BUFFER_OVERFLOW:
            printf("Buffer overflow\r\n");
            break;
        case IR_ERROR_TX_BUSY:
            printf("Transmitter busy\r\n");
            break;
        case IR_ERROR_RX_BUSY:
            printf("Receiver busy\r\n");
            break;
        case IR_ERROR_TIMER:
            printf("Timer error\r\n");
            break;
        default:
            printf("Unknown error\r\n");
            break;
    }
}

/**
 * @brief IR event callback function
 * @param event: IR event
 * @param frame: Pointer to IR frame (if applicable)
 */
static void IR_Example_EventCallback(IR_Event_t event, IR_Frame_t *frame) {
    switch (event) {
        case IR_EVENT_FRAME_RECEIVED:
            printf("Event: Frame received - ");
            if (frame != NULL) {
                IR_Example_PrintFrame(frame);
            }
            break;
        case IR_EVENT_FRAME_TRANSMITTED:
            printf("Event: Frame transmitted\r\n");
            break;
        case IR_EVENT_REPEAT_RECEIVED:
            printf("Event: Repeat received\r\n");
            break;
        case IR_EVENT_ERROR_TIMEOUT:
            printf("Event: Timeout error\r\n");
            break;
        case IR_EVENT_ERROR_PROTOCOL:
            printf("Event: Protocol error\r\n");
            break;
        case IR_EVENT_ERROR_OVERFLOW:
            printf("Event: Buffer overflow\r\n");
            break;
        default:
            printf("Event: Unknown (%d)\r\n", event);
            break;
    }
}
