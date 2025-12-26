/**
 * @file ir.h
 * @brief IR (Infrared) driver header for STM32F429 Discovery board
 * @details This file provides the interface for infrared communication
 *          including IR transmitter and receiver functionality using timers
 *          and GPIO pins. Supports common IR protocols like NEC, RC5, SIRC.
 * @version 1.0
 * @date 2025-09-03
 * @author STM32 Team
 */

#ifndef IR_H
#define IR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdbool.h>
#include <stdint.h>

/* Exported constants --------------------------------------------------------*/

/* IR Timing Constants (in microseconds) */
#define IR_CARRIER_FREQUENCY          38000U    /* 38 kHz carrier frequency */
#define IR_CARRIER_DUTY_CYCLE         33U       /* 33% duty cycle */
#define IR_TIMEOUT_MS                 1000U     /* 1 second timeout */

/* NEC Protocol Constants */
#define IR_NEC_HEADER_MARK            9000U     /* 9ms header mark */
#define IR_NEC_HEADER_SPACE           4500U     /* 4.5ms header space */
#define IR_NEC_BIT_MARK               562U      /* 562us bit mark */
#define IR_NEC_BIT_0_SPACE            562U      /* 562us for bit 0 */
#define IR_NEC_BIT_1_SPACE            1687U     /* 1.687ms for bit 1 */
#define IR_NEC_STOP_BIT               562U      /* 562us stop bit */
#define IR_NEC_REPEAT_SPACE           2250U     /* 2.25ms repeat space */
#define IR_NEC_TOLERANCE              200U      /* 200us tolerance */

/* RC5 Protocol Constants */
#define IR_RC5_BIT_TIME               1778U     /* 1.778ms bit time */
#define IR_RC5_HALF_BIT               889U      /* Half bit time */
#define IR_RC5_TOLERANCE              300U      /* 300us tolerance */

/* SIRC Protocol Constants */
#define IR_SIRC_HEADER_MARK           2400U     /* 2.4ms header */
#define IR_SIRC_BIT_MARK              600U      /* 600us bit mark */
#define IR_SIRC_BIT_0_SPACE           600U      /* 600us for bit 0 */
#define IR_SIRC_BIT_1_SPACE           1200U     /* 1.2ms for bit 1 */
#define IR_SIRC_TOLERANCE             200U      /* 200us tolerance */

/* IR Buffer Sizes */
#define IR_RX_BUFFER_SIZE             256U      /* Receive buffer size */
#define IR_TX_BUFFER_SIZE             128U      /* Transmit buffer size */
#define IR_MAX_PULSE_COUNT            100U      /* Maximum pulses per frame */

/* IR Protocols */
typedef enum {
    IR_PROTOCOL_NEC = 0,
    IR_PROTOCOL_RC5,
    IR_PROTOCOL_SIRC,
    IR_PROTOCOL_CUSTOM,
    IR_PROTOCOL_COUNT
} IR_Protocol_t;

/* IR States */
typedef enum {
    IR_STATE_IDLE = 0,
    IR_STATE_RECEIVING,
    IR_STATE_TRANSMITTING,
    IR_STATE_PROCESSING,
    IR_STATE_ERROR
} IR_State_t;

/* IR Events */
typedef enum {
    IR_EVENT_NONE = 0,
    IR_EVENT_FRAME_RECEIVED,
    IR_EVENT_FRAME_TRANSMITTED,
    IR_EVENT_REPEAT_RECEIVED,
    IR_EVENT_ERROR_TIMEOUT,
    IR_EVENT_ERROR_PROTOCOL,
    IR_EVENT_ERROR_OVERFLOW
} IR_Event_t;

/* Error Codes */
#define IR_ERROR_NONE                 0x00U
#define IR_ERROR_INIT                 0x01U
#define IR_ERROR_INVALID_PARAM        0x02U
#define IR_ERROR_TIMEOUT              0x03U
#define IR_ERROR_PROTOCOL             0x04U
#define IR_ERROR_BUFFER_OVERFLOW      0x05U
#define IR_ERROR_TX_BUSY              0x06U
#define IR_ERROR_RX_BUSY              0x07U
#define IR_ERROR_TIMER                0x08U

/* Exported types ------------------------------------------------------------*/

/**
 * @brief IR Pulse Structure
 */
typedef struct {
    uint16_t mark;                  /* Mark duration in microseconds */
    uint16_t space;                 /* Space duration in microseconds */
} IR_Pulse_t;

/**
 * @brief IR Frame Structure
 */
typedef struct {
    IR_Protocol_t protocol;         /* IR protocol type */
    uint32_t address;               /* Device address */
    uint32_t command;               /* Command code */
    uint32_t data;                  /* Raw data */
    uint8_t bits;                   /* Number of bits */
    bool repeat;                    /* Repeat flag */
    bool valid;                     /* Valid frame flag */
} IR_Frame_t;

/**
 * @brief IR Configuration Structure
 */
typedef struct {
    IR_Protocol_t protocol;         /* Default protocol */
    uint32_t carrierFreq;           /* Carrier frequency in Hz */
    uint8_t dutyCycle;              /* Duty cycle percentage */
    uint16_t tolerance;             /* Timing tolerance in microseconds */
    bool autoRepeat;                /* Auto repeat detection */
    bool invertSignal;              /* Invert output signal */
} IR_Config_t;

/**
 * @brief IR Handle Structure
 */
typedef struct {
    /* Hardware handles */
    TIM_HandleTypeDef *htimCarrier;     /* Timer for carrier generation */
    TIM_HandleTypeDef *htimCapture;     /* Timer for input capture */
    GPIO_TypeDef *txPort;               /* TX GPIO port */
    uint16_t txPin;                     /* TX GPIO pin */
    GPIO_TypeDef *rxPort;               /* RX GPIO port */
    uint16_t rxPin;                     /* RX GPIO pin */
    uint32_t txChannel;                 /* PWM channel for TX */
    uint32_t rxChannel;                 /* Input capture channel for RX */

    /* Configuration */
    IR_Config_t config;                 /* IR configuration */

    /* State variables */
    IR_State_t state;                   /* Current state */
    IR_Event_t lastEvent;               /* Last event */
    uint32_t errorCode;                 /* Last error code */
    bool initialized;                   /* Initialization flag */

    /* Receive data */
    uint32_t rxBuffer[IR_RX_BUFFER_SIZE];   /* Raw timing buffer */
    uint16_t rxIndex;                       /* Current buffer index */
    uint32_t lastCaptureTime;               /* Last capture timestamp */
    IR_Frame_t rxFrame;                     /* Received frame */

    /* Transmit data */
    IR_Pulse_t txBuffer[IR_TX_BUFFER_SIZE]; /* Transmit pulse buffer */
    uint16_t txIndex;                       /* Current transmit index */
    uint16_t txCount;                       /* Total pulses to transmit */
    bool txActive;                          /* Transmission active flag */

    /* Callback function */
    void (*eventCallback)(IR_Event_t event, IR_Frame_t *frame);
} IR_Handle_t;

/* Exported function prototypes ---------------------------------------------*/

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
                         uint32_t txChannel, uint32_t rxChannel, IR_Config_t *config);

/**
 * @brief Deinitialize IR driver
 * @param handle: Pointer to IR handle structure
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_DeInit(IR_Handle_t *handle);

/**
 * @brief Start IR receiver
 * @param handle: Pointer to IR handle structure
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_StartReceive(IR_Handle_t *handle);

/**
 * @brief Stop IR receiver
 * @param handle: Pointer to IR handle structure
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_StopReceive(IR_Handle_t *handle);

/**
 * @brief Transmit IR frame (NEC protocol)
 * @param handle: Pointer to IR handle structure
 * @param address: Device address
 * @param command: Command code
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_TransmitNEC(IR_Handle_t *handle, uint8_t address, uint8_t command);

/**
 * @brief Transmit IR frame (RC5 protocol)
 * @param handle: Pointer to IR handle structure
 * @param address: Device address (5 bits)
 * @param command: Command code (6 bits)
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_TransmitRC5(IR_Handle_t *handle, uint8_t address, uint8_t command);

/**
 * @brief Transmit IR frame (SIRC protocol)
 * @param handle: Pointer to IR handle structure
 * @param address: Device address (5 bits)
 * @param command: Command code (7 bits)
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_TransmitSIRC(IR_Handle_t *handle, uint8_t address, uint8_t command);

/**
 * @brief Transmit custom IR frame
 * @param handle: Pointer to IR handle structure
 * @param pulses: Array of pulse structures
 * @param count: Number of pulses
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_TransmitCustom(IR_Handle_t *handle, IR_Pulse_t *pulses, uint16_t count);

/**
 * @brief Check if frame is available
 * @param handle: Pointer to IR handle structure
 * @param frame: Pointer to frame structure
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_GetFrame(IR_Handle_t *handle, IR_Frame_t *frame);

/**
 * @brief Set event callback function
 * @param handle: Pointer to IR handle structure
 * @param callback: Callback function pointer
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_SetEventCallback(IR_Handle_t *handle,
                                     void (*callback)(IR_Event_t event, IR_Frame_t *frame));

/**
 * @brief Configure carrier frequency
 * @param handle: Pointer to IR handle structure
 * @param frequency: Carrier frequency in Hz
 * @param dutyCycle: Duty cycle percentage (1-99)
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_ConfigureCarrier(IR_Handle_t *handle, uint32_t frequency, uint8_t dutyCycle);

/**
 * @brief Set protocol tolerance
 * @param handle: Pointer to IR handle structure
 * @param tolerance: Tolerance in microseconds
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_SetTolerance(IR_Handle_t *handle, uint16_t tolerance);

/**
 * @brief Get current state
 * @param handle: Pointer to IR handle structure
 * @return IR_State_t: Current state
 */
IR_State_t IR_GetState(IR_Handle_t *handle);

/**
 * @brief Get last error
 * @param handle: Pointer to IR handle structure
 * @return uint32_t: Error code
 */
uint32_t IR_GetError(IR_Handle_t *handle);

/**
 * @brief Clear error and reset state
 * @param handle: Pointer to IR handle structure
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_ClearError(IR_Handle_t *handle);

/**
 * @brief Input capture callback (to be called from HAL interrupt)
 * @param handle: Pointer to IR handle structure
 * @param captureValue: Captured timer value
 * @return void
 */
void IR_InputCaptureCallback(IR_Handle_t *handle, uint32_t captureValue);

/**
 * @brief Timer overflow callback (to be called from HAL interrupt)
 * @param handle: Pointer to IR handle structure
 * @return void
 */
void IR_TimerOverflowCallback(IR_Handle_t *handle);

/**
 * @brief PWM pulse finished callback (to be called from HAL interrupt)
 * @param handle: Pointer to IR handle structure
 * @return void
 */
void IR_PWMPulseFinishedCallback(IR_Handle_t *handle);

/* Utility Functions ---------------------------------------------------------*/

/**
 * @brief Convert microseconds to timer ticks
 * @param microseconds: Time in microseconds
 * @param timerFreq: Timer frequency in Hz
 * @return uint32_t: Timer ticks
 */
uint32_t IR_MicrosecondsToTicks(uint32_t microseconds, uint32_t timerFreq);

/**
 * @brief Convert timer ticks to microseconds
 * @param ticks: Timer ticks
 * @param timerFreq: Timer frequency in Hz
 * @return uint32_t: Time in microseconds
 */
uint32_t IR_TicksToMicroseconds(uint32_t ticks, uint32_t timerFreq);

/**
 * @brief Check if timing is within tolerance
 * @param measured: Measured time
 * @param expected: Expected time
 * @param tolerance: Tolerance value
 * @return bool: True if within tolerance
 */
bool IR_IsWithinTolerance(uint32_t measured, uint32_t expected, uint32_t tolerance);

#ifdef __cplusplus
}
#endif

#endif /* IR_H */
