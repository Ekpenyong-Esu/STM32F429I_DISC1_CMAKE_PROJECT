/**
 * @file ir.c
 * @brief IR (Infrared) driver implementation for STM32F429 Discovery board
 * @details This file provides the implementation for infrared communication
 *          including IR transmitter and receiver functionality using timers
 *          and GPIO pins. Supports common IR protocols like NEC, RC5, SIRC.
 * @version 1.0
 * @date 2025-09-03
 * @author STM32 Team
 */

/* Includes ------------------------------------------------------------------*/
#include "ir.h"
#include "main.h"

/* Private function prototypes -----------------------------------------------*/
static HAL_StatusTypeDef IR_ValidateHandle(IR_Handle_t *handle);
static HAL_StatusTypeDef IR_ConfigureTimers(IR_Handle_t *handle);
static HAL_StatusTypeDef IR_ProcessReceivedData(IR_Handle_t *handle);
static HAL_StatusTypeDef IR_DecodeNEC(IR_Handle_t *handle);
static HAL_StatusTypeDef IR_DecodeRC5(IR_Handle_t *handle);
static HAL_StatusTypeDef IR_DecodeSIRC(IR_Handle_t *handle);
static HAL_StatusTypeDef IR_PrepareNECTransmit(IR_Handle_t *handle, uint8_t address, uint8_t command);
static HAL_StatusTypeDef IR_PrepareRC5Transmit(IR_Handle_t *handle, uint8_t address, uint8_t command);
static HAL_StatusTypeDef IR_PrepareSIRCTransmit(IR_Handle_t *handle, uint8_t address, uint8_t command);
static HAL_StatusTypeDef IR_StartTransmission(IR_Handle_t *handle);
static void IR_ResetReceiveBuffer(IR_Handle_t *handle);
static void IR_AddPulseToTxBuffer(IR_Handle_t *handle, uint16_t mark, uint16_t space);

/**
 * @brief Initialize IR driver
 * @param handle: Pointer to IR handle structure
 * @param htimCarrier: Pointer to carrier timer handle
 * @param htimCapture: Pointer to capture timer handle
 * @param txPort: TX GPIO port
 * @param txPin: TX GPIO pin
 * @param rxPort: RX GPIO port
 * @param rxPin: RX GPIO pin
 * @param txChannel: PWM channel for carrier
 * @param rxChannel: Input capture channel
 * @param config: Pointer to configuration structure
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_Init(IR_Handle_t *handle, TIM_HandleTypeDef *htimCarrier,
                         TIM_HandleTypeDef *htimCapture, GPIO_TypeDef *txPort,
                         uint16_t txPin, GPIO_TypeDef *rxPort, uint16_t rxPin,
                         uint32_t txChannel, uint32_t rxChannel, IR_Config_t *config) {
    if (handle == NULL || htimCarrier == NULL || htimCapture == NULL ||
        txPort == NULL || rxPort == NULL || config == NULL) {
        return HAL_ERROR;
    }

    /* Initialize handle structure */
    handle->htimCarrier = htimCarrier;
    handle->htimCapture = htimCapture;
    handle->txPort = txPort;
    handle->txPin = txPin;
    handle->rxPort = rxPort;
    handle->rxPin = rxPin;
    handle->txChannel = txChannel;
    handle->rxChannel = rxChannel;
    handle->config = *config;

    /* Initialize state variables */
    handle->state = IR_STATE_IDLE;
    handle->lastEvent = IR_EVENT_NONE;
    handle->errorCode = IR_ERROR_NONE;
    handle->initialized = false;
    handle->rxIndex = 0;
    handle->lastCaptureTime = 0;
    handle->txIndex = 0;
    handle->txCount = 0;
    handle->txActive = false;
    handle->eventCallback = NULL;

    /* Initialize frame structure */
    handle->rxFrame.valid = false;
    handle->rxFrame.repeat = false;

    /* Configure timers */
    HAL_StatusTypeDef status = IR_ConfigureTimers(handle);
    if (status != HAL_OK) {
        handle->errorCode = IR_ERROR_TIMER;
        return status;
    }

    /* Configure carrier frequency */
    status = IR_ConfigureCarrier(handle, config->carrierFreq, config->dutyCycle);
    if (status != HAL_OK) {
        handle->errorCode = IR_ERROR_INIT;
        return status;
    }

    handle->initialized = true;
    return HAL_OK;
}

/**
 * @brief Deinitialize IR driver
 * @param handle: Pointer to IR handle structure
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_DeInit(IR_Handle_t *handle) {
    if (IR_ValidateHandle(handle) != HAL_OK) {
        return HAL_ERROR;
    }

    /* Stop any ongoing operations */
    IR_StopReceive(handle);

    /* Stop carrier timer */
    HAL_TIM_PWM_Stop(handle->htimCarrier, handle->txChannel);

    /* Reset state */
    handle->state = IR_STATE_IDLE;
    handle->initialized = false;

    return HAL_OK;
}

/**
 * @brief Start IR receiver
 * @param handle: Pointer to IR handle structure
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_StartReceive(IR_Handle_t *handle) {
    if (IR_ValidateHandle(handle) != HAL_OK) {
        return HAL_ERROR;
    }

    if (handle->state == IR_STATE_TRANSMITTING) {
        handle->errorCode = IR_ERROR_TX_BUSY;
        return HAL_ERROR;
    }

    /* Reset receive buffer */
    IR_ResetReceiveBuffer(handle);

    /* Start input capture */
    HAL_StatusTypeDef status = HAL_TIM_IC_Start_IT(handle->htimCapture, handle->rxChannel);
    if (status != HAL_OK) {
        handle->errorCode = IR_ERROR_TIMER;
        return status;
    }

    handle->state = IR_STATE_RECEIVING;
    return HAL_OK;
}

/**
 * @brief Stop IR receiver
 * @param handle: Pointer to IR handle structure
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_StopReceive(IR_Handle_t *handle) {
    if (IR_ValidateHandle(handle) != HAL_OK) {
        return HAL_ERROR;
    }

    /* Stop input capture */
    HAL_TIM_IC_Stop_IT(handle->htimCapture, handle->rxChannel);

    handle->state = IR_STATE_IDLE;
    return HAL_OK;
}

/**
 * @brief Transmit IR frame (NEC protocol)
 * @param handle: Pointer to IR handle structure
 * @param address: Device address
 * @param command: Command code
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_TransmitNEC(IR_Handle_t *handle, uint8_t address, uint8_t command) {
    if (IR_ValidateHandle(handle) != HAL_OK) {
        return HAL_ERROR;
    }

    if (handle->state == IR_STATE_TRANSMITTING) {
        handle->errorCode = IR_ERROR_TX_BUSY;
        return HAL_ERROR;
    }

    /* Prepare NEC transmission */
    HAL_StatusTypeDef status = IR_PrepareNECTransmit(handle, address, command);
    if (status != HAL_OK) {
        return status;
    }

    /* Start transmission */
    return IR_StartTransmission(handle);
}

/**
 * @brief Transmit IR frame (RC5 protocol)
 * @param handle: Pointer to IR handle structure
 * @param address: Device address (5 bits)
 * @param command: Command code (6 bits)
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_TransmitRC5(IR_Handle_t *handle, uint8_t address, uint8_t command) {
    if (IR_ValidateHandle(handle) != HAL_OK) {
        return HAL_ERROR;
    }

    if (handle->state == IR_STATE_TRANSMITTING) {
        handle->errorCode = IR_ERROR_TX_BUSY;
        return HAL_ERROR;
    }

    /* Check parameter ranges */
    if (address > 31 || command > 63) {
        handle->errorCode = IR_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    /* Prepare RC5 transmission */
    HAL_StatusTypeDef status = IR_PrepareRC5Transmit(handle, address, command);
    if (status != HAL_OK) {
        return status;
    }

    /* Start transmission */
    return IR_StartTransmission(handle);
}

/**
 * @brief Transmit IR frame (SIRC protocol)
 * @param handle: Pointer to IR handle structure
 * @param address: Device address (5 bits)
 * @param command: Command code (7 bits)
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_TransmitSIRC(IR_Handle_t *handle, uint8_t address, uint8_t command) {
    if (IR_ValidateHandle(handle) != HAL_OK) {
        return HAL_ERROR;
    }

    if (handle->state == IR_STATE_TRANSMITTING) {
        handle->errorCode = IR_ERROR_TX_BUSY;
        return HAL_ERROR;
    }

    /* Check parameter ranges */
    if (address > 31 || command > 127) {
        handle->errorCode = IR_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    /* Prepare SIRC transmission */
    HAL_StatusTypeDef status = IR_PrepareSIRCTransmit(handle, address, command);
    if (status != HAL_OK) {
        return status;
    }

    /* Start transmission */
    return IR_StartTransmission(handle);
}

/**
 * @brief Transmit custom IR frame
 * @param handle: Pointer to IR handle structure
 * @param pulses: Array of pulse structures
 * @param count: Number of pulses
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_TransmitCustom(IR_Handle_t *handle, IR_Pulse_t *pulses, uint16_t count) {
    if (IR_ValidateHandle(handle) != HAL_OK || pulses == NULL || count == 0) {
        return HAL_ERROR;
    }

    if (handle->state == IR_STATE_TRANSMITTING) {
        handle->errorCode = IR_ERROR_TX_BUSY;
        return HAL_ERROR;
    }

    if (count > IR_TX_BUFFER_SIZE) {
        handle->errorCode = IR_ERROR_BUFFER_OVERFLOW;
        return HAL_ERROR;
    }

    /* Copy pulses to transmit buffer */
    for (uint16_t i = 0; i < count; i++) {
        handle->txBuffer[i] = pulses[i];
    }

    handle->txCount = count;
    handle->txIndex = 0;

    /* Start transmission */
    return IR_StartTransmission(handle);
}

/**
 * @brief Check if frame is available
 * @param handle: Pointer to IR handle structure
 * @param frame: Pointer to frame structure
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_GetFrame(IR_Handle_t *handle, IR_Frame_t *frame) {
    if (IR_ValidateHandle(handle) != HAL_OK || frame == NULL) {
        return HAL_ERROR;
    }

    if (handle->rxFrame.valid) {
        *frame = handle->rxFrame;
        handle->rxFrame.valid = false; /* Mark as consumed */
        return HAL_OK;
    }

    return HAL_ERROR; /* No frame available */
}

/**
 * @brief Set event callback function
 * @param handle: Pointer to IR handle structure
 * @param callback: Callback function pointer
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_SetEventCallback(IR_Handle_t *handle,
                                     void (*callback)(IR_Event_t event, IR_Frame_t *frame)) {
    if (IR_ValidateHandle(handle) != HAL_OK) {
        return HAL_ERROR;
    }

    handle->eventCallback = callback;
    return HAL_OK;
}

/**
 * @brief Configure carrier frequency
 * @param handle: Pointer to IR handle structure
 * @param frequency: Carrier frequency in Hz
 * @param dutyCycle: Duty cycle percentage (1-99)
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_ConfigureCarrier(IR_Handle_t *handle, uint32_t frequency, uint8_t dutyCycle) {
    if (IR_ValidateHandle(handle) != HAL_OK || dutyCycle == 0 || dutyCycle >= 100) {
        return HAL_ERROR;
    }

    /* Calculate timer parameters */
    uint32_t timerClock = HAL_RCC_GetPCLK1Freq() * 2; /* Assuming APB1 timer */
    uint32_t prescaler = 1;
    uint32_t period = (timerClock / (prescaler * frequency)) - 1;
    uint32_t pulse = (period * dutyCycle) / 100;

    /* Configure timer */
    handle->htimCarrier->Init.Prescaler = prescaler - 1;
    handle->htimCarrier->Init.Period = period;

    if (HAL_TIM_PWM_Init(handle->htimCarrier) != HAL_OK) {
        handle->errorCode = IR_ERROR_TIMER;
        return HAL_ERROR;
    }

    /* Configure PWM channel */
    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = pulse;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

    if (HAL_TIM_PWM_ConfigChannel(handle->htimCarrier, &sConfigOC, handle->txChannel) != HAL_OK) {
        handle->errorCode = IR_ERROR_TIMER;
        return HAL_ERROR;
    }

    /* Update configuration */
    handle->config.carrierFreq = frequency;
    handle->config.dutyCycle = dutyCycle;

    return HAL_OK;
}

/**
 * @brief Set protocol tolerance
 * @param handle: Pointer to IR handle structure
 * @param tolerance: Tolerance in microseconds
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_SetTolerance(IR_Handle_t *handle, uint16_t tolerance) {
    if (IR_ValidateHandle(handle) != HAL_OK) {
        return HAL_ERROR;
    }

    handle->config.tolerance = tolerance;
    return HAL_OK;
}

/**
 * @brief Get current state
 * @param handle: Pointer to IR handle structure
 * @return IR_State_t: Current state
 */
IR_State_t IR_GetState(IR_Handle_t *handle) {
    if (handle == NULL) {
        return IR_STATE_ERROR;
    }
    return handle->state;
}

/**
 * @brief Get last error
 * @param handle: Pointer to IR handle structure
 * @return uint32_t: Error code
 */
uint32_t IR_GetError(IR_Handle_t *handle) {
    if (handle == NULL) {
        return IR_ERROR_INIT;
    }
    return handle->errorCode;
}

/**
 * @brief Clear error and reset state
 * @param handle: Pointer to IR handle structure
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_ClearError(IR_Handle_t *handle) {
    if (handle == NULL) {
        return HAL_ERROR;
    }

    handle->errorCode = IR_ERROR_NONE;
    if (handle->state == IR_STATE_ERROR) {
        handle->state = IR_STATE_IDLE;
    }

    return HAL_OK;
}

/**
 * @brief Input capture callback (to be called from HAL interrupt)
 * @param handle: Pointer to IR handle structure
 * @param captureValue: Captured timer value
 * @return void
 */
void IR_InputCaptureCallback(IR_Handle_t *handle, uint32_t captureValue) {
    if (handle == NULL || handle->state != IR_STATE_RECEIVING) {
        return;
    }

    /* Calculate time difference */
    uint32_t timeDiff;
    if (captureValue >= handle->lastCaptureTime) {
        timeDiff = captureValue - handle->lastCaptureTime;
    } else {
        /* Timer overflow occurred */
        timeDiff = (0xFFFFFFFF - handle->lastCaptureTime) + captureValue + 1;
    }

    /* Store timing in buffer */
    if (handle->rxIndex < IR_RX_BUFFER_SIZE) {
        handle->rxBuffer[handle->rxIndex++] = timeDiff;
    } else {
        /* Buffer overflow */
        handle->errorCode = IR_ERROR_BUFFER_OVERFLOW;
        handle->lastEvent = IR_EVENT_ERROR_OVERFLOW;
        if (handle->eventCallback != NULL) {
            handle->eventCallback(IR_EVENT_ERROR_OVERFLOW, NULL);
        }
        IR_ResetReceiveBuffer(handle);
        return;
    }

    handle->lastCaptureTime = captureValue;

    /* Check for end of frame (long silence) */
    uint32_t silenceTime = IR_TicksToMicroseconds(timeDiff,
                                                  HAL_RCC_GetPCLK1Freq() * 2);
    if (silenceTime > 10000) { /* 10ms silence indicates end of frame */
        handle->state = IR_STATE_PROCESSING;
        IR_ProcessReceivedData(handle);
    }
}

/**
 * @brief Timer overflow callback (to be called from HAL interrupt)
 * @param handle: Pointer to IR handle structure
 * @return void
 */
void IR_TimerOverflowCallback(IR_Handle_t *handle) {
    if (handle == NULL) {
        return;
    }

    /* Handle timeout */
    if (handle->state == IR_STATE_RECEIVING && handle->rxIndex > 0) {
        handle->state = IR_STATE_PROCESSING;
        IR_ProcessReceivedData(handle);
    } else {
        handle->errorCode = IR_ERROR_TIMEOUT;
        handle->lastEvent = IR_EVENT_ERROR_TIMEOUT;
        if (handle->eventCallback != NULL) {
            handle->eventCallback(IR_EVENT_ERROR_TIMEOUT, NULL);
        }
    }
}

/**
 * @brief PWM pulse finished callback (to be called from HAL interrupt)
 * @param handle: Pointer to IR handle structure
 * @return void
 */
void IR_PWMPulseFinishedCallback(IR_Handle_t *handle) {
    if (handle == NULL || !handle->txActive) {
        return;
    }

    /* Move to next pulse */
    handle->txIndex++;

    if (handle->txIndex >= handle->txCount) {
        /* Transmission complete */
        handle->txActive = false;
        handle->state = IR_STATE_IDLE;
        handle->lastEvent = IR_EVENT_FRAME_TRANSMITTED;

        if (handle->eventCallback != NULL) {
            handle->eventCallback(IR_EVENT_FRAME_TRANSMITTED, NULL);
        }
    } else {
        /* Continue with next pulse */
        IR_Pulse_t *pulse = &handle->txBuffer[handle->txIndex];

        /* Configure next pulse timing */
        /* Implementation depends on specific timer configuration */
        /* This is a simplified version */
        HAL_TIM_PWM_Start(handle->htimCarrier, handle->txChannel);
        HAL_Delay(pulse->mark / 1000); /* Convert to milliseconds */
        HAL_TIM_PWM_Stop(handle->htimCarrier, handle->txChannel);
        HAL_Delay(pulse->space / 1000);
    }
}

/* Private Functions ---------------------------------------------------------*/

/**
 * @brief Validate IR handle
 * @param handle: Pointer to IR handle structure
 * @return HAL_StatusTypeDef: HAL status
 */
static HAL_StatusTypeDef IR_ValidateHandle(IR_Handle_t *handle) {
    if (handle == NULL || !handle->initialized) {
        return HAL_ERROR;
    }
    return HAL_OK;
}

/**
 * @brief Configure timers for IR operation
 * @param handle: Pointer to IR handle structure
 * @return HAL_StatusTypeDef: HAL status
 */
static HAL_StatusTypeDef IR_ConfigureTimers(IR_Handle_t *handle) {
    /* Configure capture timer for microsecond resolution */
    uint32_t timerClock = HAL_RCC_GetPCLK1Freq() * 2;
    uint32_t prescaler = timerClock / 1000000; /* 1MHz for microsecond resolution */

    handle->htimCapture->Init.Prescaler = prescaler - 1;
    handle->htimCapture->Init.Period = 0xFFFFFFFF; /* Maximum period */
    handle->htimCapture->Init.CounterMode = TIM_COUNTERMODE_UP;

    return HAL_TIM_IC_Init(handle->htimCapture);
}

/**
 * @brief Process received data and decode protocol
 * @param handle: Pointer to IR handle structure
 * @return HAL_StatusTypeDef: HAL status
 */
static HAL_StatusTypeDef IR_ProcessReceivedData(IR_Handle_t *handle) {
    HAL_StatusTypeDef status = HAL_ERROR;

    /* Try to decode based on configured protocol */
    switch (handle->config.protocol) {
        case IR_PROTOCOL_NEC:
            status = IR_DecodeNEC(handle);
            break;
        case IR_PROTOCOL_RC5:
            status = IR_DecodeRC5(handle);
            break;
        case IR_PROTOCOL_SIRC:
            status = IR_DecodeSIRC(handle);
            break;
        default:
            /* Try all protocols */
            if (IR_DecodeNEC(handle) == HAL_OK) {
                handle->rxFrame.protocol = IR_PROTOCOL_NEC;
                status = HAL_OK;
            } else if (IR_DecodeRC5(handle) == HAL_OK) {
                handle->rxFrame.protocol = IR_PROTOCOL_RC5;
                status = HAL_OK;
            } else if (IR_DecodeSIRC(handle) == HAL_OK) {
                handle->rxFrame.protocol = IR_PROTOCOL_SIRC;
                status = HAL_OK;
            }
            break;
    }

    if (status == HAL_OK) {
        handle->rxFrame.valid = true;
        handle->lastEvent = IR_EVENT_FRAME_RECEIVED;
        if (handle->eventCallback != NULL) {
            handle->eventCallback(IR_EVENT_FRAME_RECEIVED, &handle->rxFrame);
        }
    } else {
        handle->errorCode = IR_ERROR_PROTOCOL;
        handle->lastEvent = IR_EVENT_ERROR_PROTOCOL;
        if (handle->eventCallback != NULL) {
            handle->eventCallback(IR_EVENT_ERROR_PROTOCOL, NULL);
        }
    }

    /* Reset for next frame */
    IR_ResetReceiveBuffer(handle);
    handle->state = IR_STATE_RECEIVING;

    return status;
}

/**
 * @brief Decode NEC protocol
 * @param handle: Pointer to IR handle structure
 * @return HAL_StatusTypeDef: HAL status
 */
static HAL_StatusTypeDef IR_DecodeNEC(IR_Handle_t *handle) {
    if (handle->rxIndex < 34) { /* Minimum for NEC: header + 32 bits + stop */
        return HAL_ERROR;
    }

    /* Convert first timing to microseconds */
    uint32_t timerFreq = HAL_RCC_GetPCLK1Freq() * 2;
    uint32_t headerMark = IR_TicksToMicroseconds(handle->rxBuffer[0], timerFreq);
    uint32_t headerSpace = IR_TicksToMicroseconds(handle->rxBuffer[1], timerFreq);

    /* Check NEC header */
    if (!IR_IsWithinTolerance(headerMark, IR_NEC_HEADER_MARK, handle->config.tolerance) ||
        !IR_IsWithinTolerance(headerSpace, IR_NEC_HEADER_SPACE, handle->config.tolerance)) {
        return HAL_ERROR;
    }

    /* Decode 32 data bits */
    uint32_t data = 0;
    for (int i = 0; i < 32; i++) {
        uint32_t bitMark = IR_TicksToMicroseconds(handle->rxBuffer[2 + i * 2], timerFreq);
        uint32_t bitSpace = IR_TicksToMicroseconds(handle->rxBuffer[3 + i * 2], timerFreq);

        if (!IR_IsWithinTolerance(bitMark, IR_NEC_BIT_MARK, handle->config.tolerance)) {
            return HAL_ERROR;
        }

        if (IR_IsWithinTolerance(bitSpace, IR_NEC_BIT_1_SPACE, handle->config.tolerance)) {
            data |= (1UL << (31 - i)); /* MSB first */
        } else if (!IR_IsWithinTolerance(bitSpace, IR_NEC_BIT_0_SPACE, handle->config.tolerance)) {
            return HAL_ERROR;
        }
    }

    /* Extract address and command */
    uint8_t address = (data >> 24) & 0xFF;
    uint8_t addressInv = (data >> 16) & 0xFF;
    uint8_t command = (data >> 8) & 0xFF;
    uint8_t commandInv = data & 0xFF;

    /* Verify inverse bytes */
    if ((address ^ addressInv) != 0xFF || (command ^ commandInv) != 0xFF) {
        return HAL_ERROR;
    }

    /* Fill frame structure */
    handle->rxFrame.protocol = IR_PROTOCOL_NEC;
    handle->rxFrame.address = address;
    handle->rxFrame.command = command;
    handle->rxFrame.data = data;
    handle->rxFrame.bits = 32;
    handle->rxFrame.repeat = false;

    return HAL_OK;
}

/**
 * @brief Decode RC5 protocol
 * @param handle: Pointer to IR handle structure
 * @return HAL_StatusTypeDef: HAL status
 */
static HAL_StatusTypeDef IR_DecodeRC5(IR_Handle_t *handle) {
    /* RC5 decoding implementation */
    /* This is a simplified version - full implementation would be more complex */
    if (handle->rxIndex < 28) { /* RC5 has 14 bits, each with 2 transitions */
        return HAL_ERROR;
    }

    /* Simplified RC5 decoding */
    uint16_t data = 0;
    uint32_t timerFreq = HAL_RCC_GetPCLK1Freq() * 2;

    for (int i = 0; i < 14; i++) {
        uint32_t timing = IR_TicksToMicroseconds(handle->rxBuffer[i * 2], timerFreq);
        if (IR_IsWithinTolerance(timing, IR_RC5_BIT_TIME, handle->config.tolerance)) {
            data |= (1 << (13 - i));
        }
    }

    /* Extract fields */
    uint8_t startBits = (data >> 12) & 0x03;
    uint8_t toggle = (data >> 11) & 0x01;
    uint8_t address = (data >> 6) & 0x1F;
    uint8_t command = data & 0x3F;

    if (startBits != 0x03) { /* RC5 start bits should be 11 */
        return HAL_ERROR;
    }

    /* Fill frame structure */
    handle->rxFrame.protocol = IR_PROTOCOL_RC5;
    handle->rxFrame.address = address;
    handle->rxFrame.command = command;
    handle->rxFrame.data = data;
    handle->rxFrame.bits = 14;
    handle->rxFrame.repeat = false;

    return HAL_OK;
}

/**
 * @brief Decode SIRC protocol
 * @param handle: Pointer to IR handle structure
 * @return HAL_StatusTypeDef: HAL status
 */
static HAL_StatusTypeDef IR_DecodeSIRC(IR_Handle_t *handle) {
    if (handle->rxIndex < 25) { /* SIRC: header + 12 bits minimum */
        return HAL_ERROR;
    }

    uint32_t timerFreq = HAL_RCC_GetPCLK1Freq() * 2;
    uint32_t headerMark = IR_TicksToMicroseconds(handle->rxBuffer[0], timerFreq);

    /* Check SIRC header */
    if (!IR_IsWithinTolerance(headerMark, IR_SIRC_HEADER_MARK, handle->config.tolerance)) {
        return HAL_ERROR;
    }

    /* Decode data bits */
    uint16_t data = 0;
    for (int i = 0; i < 12; i++) {
        uint32_t bitMark = IR_TicksToMicroseconds(handle->rxBuffer[1 + i * 2], timerFreq);
        uint32_t bitSpace = IR_TicksToMicroseconds(handle->rxBuffer[2 + i * 2], timerFreq);

        if (!IR_IsWithinTolerance(bitMark, IR_SIRC_BIT_MARK, handle->config.tolerance)) {
            return HAL_ERROR;
        }

        if (IR_IsWithinTolerance(bitSpace, IR_SIRC_BIT_1_SPACE, handle->config.tolerance)) {
            data |= (1 << i); /* LSB first */
        } else if (!IR_IsWithinTolerance(bitSpace, IR_SIRC_BIT_0_SPACE, handle->config.tolerance)) {
            return HAL_ERROR;
        }
    }

    /* Extract command and address */
    uint8_t command = data & 0x7F;
    uint8_t address = (data >> 7) & 0x1F;

    /* Fill frame structure */
    handle->rxFrame.protocol = IR_PROTOCOL_SIRC;
    handle->rxFrame.address = address;
    handle->rxFrame.command = command;
    handle->rxFrame.data = data;
    handle->rxFrame.bits = 12;
    handle->rxFrame.repeat = false;

    return HAL_OK;
}

/**
 * @brief Prepare NEC transmission
 * @param handle: Pointer to IR handle structure
 * @param address: Device address
 * @param command: Command code
 * @return HAL_StatusTypeDef: HAL status
 */
static HAL_StatusTypeDef IR_PrepareNECTransmit(IR_Handle_t *handle, uint8_t address, uint8_t command) {
    handle->txCount = 0;

    /* Add header */
    IR_AddPulseToTxBuffer(handle, IR_NEC_HEADER_MARK, IR_NEC_HEADER_SPACE);

    /* Create 32-bit data word */
    uint32_t data = ((uint32_t)address << 24) |
                    ((uint32_t)(~address) << 16) |
                    ((uint32_t)command << 8) |
                    (uint32_t)(~command);

    /* Add data bits (MSB first) */
    for (int i = 31; i >= 0; i--) {
        uint16_t space = (data & (1UL << i)) ? IR_NEC_BIT_1_SPACE : IR_NEC_BIT_0_SPACE;
        IR_AddPulseToTxBuffer(handle, IR_NEC_BIT_MARK, space);
    }

    /* Add stop bit */
    IR_AddPulseToTxBuffer(handle, IR_NEC_STOP_BIT, 0);

    handle->txIndex = 0;
    return HAL_OK;
}

/**
 * @brief Prepare RC5 transmission
 * @param handle: Pointer to IR handle structure
 * @param address: Device address
 * @param command: Command code
 * @return HAL_StatusTypeDef: HAL status
 */
static HAL_StatusTypeDef IR_PrepareRC5Transmit(IR_Handle_t *handle, uint8_t address, uint8_t command) {
    handle->txCount = 0;

    /* Create RC5 data word */
    uint16_t data = 0x3000 | /* Start bits */
                    ((uint16_t)address << 6) |
                    (uint16_t)command;

    /* Add RC5 bits using Manchester encoding */
    for (int i = 13; i >= 0; i--) {
        if (data & (1 << i)) {
            /* Bit 1: space-mark */
            IR_AddPulseToTxBuffer(handle, 0, IR_RC5_HALF_BIT);
            IR_AddPulseToTxBuffer(handle, IR_RC5_HALF_BIT, 0);
        } else {
            /* Bit 0: mark-space */
            IR_AddPulseToTxBuffer(handle, IR_RC5_HALF_BIT, 0);
            IR_AddPulseToTxBuffer(handle, 0, IR_RC5_HALF_BIT);
        }
    }

    handle->txIndex = 0;
    return HAL_OK;
}

/**
 * @brief Prepare SIRC transmission
 * @param handle: Pointer to IR handle structure
 * @param address: Device address
 * @param command: Command code
 * @return HAL_StatusTypeDef: HAL status
 */
static HAL_StatusTypeDef IR_PrepareSIRCTransmit(IR_Handle_t *handle, uint8_t address, uint8_t command) {
    handle->txCount = 0;

    /* Add header */
    IR_AddPulseToTxBuffer(handle, IR_SIRC_HEADER_MARK, IR_SIRC_BIT_0_SPACE);

    /* Create 12-bit data word */
    uint16_t data = ((uint16_t)address << 7) | (uint16_t)command;

    /* Add data bits (LSB first) */
    for (int i = 0; i < 12; i++) {
        uint16_t space = (data & (1 << i)) ? IR_SIRC_BIT_1_SPACE : IR_SIRC_BIT_0_SPACE;
        IR_AddPulseToTxBuffer(handle, IR_SIRC_BIT_MARK, space);
    }

    handle->txIndex = 0;
    return HAL_OK;
}

/**
 * @brief Start transmission
 * @param handle: Pointer to IR handle structure
 * @return HAL_StatusTypeDef: HAL status
 */
static HAL_StatusTypeDef IR_StartTransmission(IR_Handle_t *handle) {
    if (handle->txCount == 0) {
        return HAL_ERROR;
    }

    handle->state = IR_STATE_TRANSMITTING;
    handle->txActive = true;
    handle->txIndex = 0;

    /* Start with first pulse */
    IR_Pulse_t *pulse = &handle->txBuffer[0];

    /* Start PWM for mark time */
    HAL_TIM_PWM_Start(handle->htimCarrier, handle->txChannel);

    /* Use a timer interrupt or delay for timing control */
    /* This is a simplified implementation */
    HAL_Delay(pulse->mark / 1000);
    HAL_TIM_PWM_Stop(handle->htimCarrier, handle->txChannel);
    HAL_Delay(pulse->space / 1000);

    return HAL_OK;
}

/**
 * @brief Reset receive buffer
 * @param handle: Pointer to IR handle structure
 * @return void
 */
static void IR_ResetReceiveBuffer(IR_Handle_t *handle) {
    handle->rxIndex = 0;
    handle->lastCaptureTime = 0;
    handle->rxFrame.valid = false;
}

/**
 * @brief Add pulse to transmit buffer
 * @param handle: Pointer to IR handle structure
 * @param mark: Mark time in microseconds
 * @param space: Space time in microseconds
 * @return void
 */
static void IR_AddPulseToTxBuffer(IR_Handle_t *handle, uint16_t mark, uint16_t space) {
    if (handle->txCount < IR_TX_BUFFER_SIZE) {
        handle->txBuffer[handle->txCount].mark = mark;
        handle->txBuffer[handle->txCount].space = space;
        handle->txCount++;
    }
}

/* Utility Functions ---------------------------------------------------------*/

/**
 * @brief Convert microseconds to timer ticks
 * @param microseconds: Time in microseconds
 * @param timerFreq: Timer frequency in Hz
 * @return uint32_t: Timer ticks
 */
uint32_t IR_MicrosecondsToTicks(uint32_t microseconds, uint32_t timerFreq) {
    return (microseconds * timerFreq) / 1000000;
}

/**
 * @brief Convert timer ticks to microseconds
 * @param ticks: Timer ticks
 * @param timerFreq: Timer frequency in Hz
 * @return uint32_t: Time in microseconds
 */
uint32_t IR_TicksToMicroseconds(uint32_t ticks, uint32_t timerFreq) {
    return (ticks * 1000000) / timerFreq;
}

/**
 * @brief Check if timing is within tolerance
 * @param measured: Measured time
 * @param expected: Expected time
 * @param tolerance: Tolerance value
 * @return bool: True if within tolerance
 */
bool IR_IsWithinTolerance(uint32_t measured, uint32_t expected, uint32_t tolerance) {
    return (measured >= (expected - tolerance)) && (measured <= (expected + tolerance));
}
