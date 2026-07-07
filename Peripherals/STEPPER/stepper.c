/**
  ******************************************************************************
  * @file    stepper.c
  * @brief   Stepper motor driver implementation
  * @details This file provides the implementation of stepper motor functions
  *          using GPIO pins for coil control and timer for precise timing.
  * @version 1.0
  * @date    2025-01-13
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stepper.h"
#include "gpio.h"
#include "tim.h"
#include "stm32f4xx_hal.h"
#include <stdlib.h>
#include <string.h>

/* Private defines -----------------------------------------------------------*/

/** @defgroup STEPPER_Private_Defines Private Defines
 * @{
 */

/* Step sequences for different stepping modes */
/* Full step sequence (4 steps) */
static const uint8_t fullStepSequence[4][4] = {
    {1, 1, 0, 0},  /* Step 0 */
    {0, 1, 1, 0},  /* Step 1 */
    {0, 0, 1, 1},  /* Step 2 */
    {1, 0, 0, 1}   /* Step 3 */
};

/* Half step sequence (8 steps) */
static const uint8_t halfStepSequence[8][4] = {
    {1, 0, 0, 0},  /* Step 0 */
    {1, 1, 0, 0},  /* Step 1 */
    {0, 1, 0, 0},  /* Step 2 */
    {0, 1, 1, 0},  /* Step 3 */
    {0, 0, 1, 0},  /* Step 4 */
    {0, 0, 1, 1},  /* Step 5 */
    {0, 0, 0, 1},  /* Step 6 */
    {1, 0, 0, 1}   /* Step 7 */
};

/* Wave drive sequence (4 steps) */
static const uint8_t waveDriveSequence[4][4] = {
    {1, 0, 0, 0},  /* Step 0 */
    {0, 1, 0, 0},  /* Step 1 */
    {0, 0, 1, 0},  /* Step 2 */
    {0, 0, 0, 1}   /* Step 3 */
};

/* Default timing values */
#define STEPPER_DEFAULT_DELAY_US   1000U   /* 1ms default delay */
#define STEPPER_MIN_DELAY_US       100U    /* Minimum delay 100us */
#define STEPPER_MAX_DELAY_US       100000U /* Maximum delay 100ms */

/** @} */

/* Private variables ---------------------------------------------------------*/

/** @defgroup STEPPER_Private_Variables Private Variables
 * @{
 */

/* Global stepper handle for interrupt handling */
static STEPPER_Handle_t *g_hstep = NULL;

/** @} */

/* Private function prototypes -----------------------------------------------*/
static void STEPPER_MspInit(STEPPER_Handle_t *hstep);
static void STEPPER_MspDeInit(STEPPER_Handle_t *hstep);
static void STEPPER_SetStep(STEPPER_Handle_t *hstep, uint8_t step);
static uint8_t STEPPER_GetMaxSteps(STEPPER_StepMode_t mode);
static uint32_t STEPPER_CalculateDelay(uint16_t rpm, uint16_t stepsPerRev);
static STEPPER_StatusTypeDef STEPPER_ValidateConfig(STEPPER_Config_t *config);
static STEPPER_StatusTypeDef STEPPER_MoveStep(STEPPER_Handle_t *hstep);

/* Exported functions -------------------------------------------------------*/

/**
 * @brief   Initialize stepper motor
 * @details Configures GPIO pins and timer for stepper motor control
 * @param   hstep Pointer to stepper motor handle
 * @param   htim Pointer to timer handle for timing
 * @param   pins Pointer to GPIO pin configuration
 * @retval  STEPPER_StatusTypeDef Operation status
 */
STEPPER_StatusTypeDef STEPPER_Init(STEPPER_Handle_t *hstep,
                                  TIM_HandleTypeDef *htim,
                                  STEPPER_Pins_t *pins)
{
    if (hstep == NULL || htim == NULL || pins == NULL) {
        return STEPPER_INVALID_PARAM;
    }

    /* Initialize structure */
    memset(hstep, 0, sizeof(STEPPER_Handle_t));
    hstep->htim = htim;
    hstep->pins = *pins;
    g_hstep = hstep;

    /* Initialize MSP (GPIO pins) */
    STEPPER_MspInit(hstep);

    /* Set default configuration */
    STEPPER_Config_t default_config = STEPPER_GetDefaultConfig();
    STEPPER_Config(hstep, &default_config);

    /* Initialize timer for microsecond delays */
    /* Note: Timer should be configured externally or use TIM functions */

    hstep->isInitialized = true;

    return STEPPER_OK;
}

/**
 * @brief   Deinitialize stepper motor
 * @details Stops motor and releases resources
 * @param   hstep Pointer to stepper motor handle
 * @retval  STEPPER_StatusTypeDef Operation status
 */
STEPPER_StatusTypeDef STEPPER_DeInit(STEPPER_Handle_t *hstep)
{
    if (hstep == NULL) {
        return STEPPER_INVALID_PARAM;
    }

    /* Stop motor if running */
    STEPPER_Stop(hstep);

    /* Deinitialize MSP */
    STEPPER_MspDeInit(hstep);

    hstep->isInitialized = false;
    g_hstep = NULL;

    return STEPPER_OK;
}

/**
 * @brief   Configure stepper motor parameters
 * @details Sets motor configuration parameters
 * @param   hstep Pointer to stepper motor handle
 * @param   config Pointer to configuration structure
 * @retval  STEPPER_StatusTypeDef Operation status
 */
STEPPER_StatusTypeDef STEPPER_Config(STEPPER_Handle_t *hstep, STEPPER_Config_t *config)
{
    if (hstep == NULL || config == NULL) {
        return STEPPER_INVALID_PARAM;
    }

    /* Validate configuration */
    if (STEPPER_ValidateConfig(config) != STEPPER_OK) {
        return STEPPER_INVALID_PARAM;
    }

    /* Store configuration */
    hstep->config = *config;

    /* Calculate step delay for default speed */
    hstep->stepDelay = STEPPER_CalculateDelay(60, config->stepsPerRevolution); /* 60 RPM default */

    return STEPPER_OK;
}

/**
 * @brief   Move stepper motor by specified steps
 * @details Moves motor relative to current position
 * @param   hstep Pointer to stepper motor handle
 * @param   steps Number of steps to move
 * @param   direction Direction of movement
 * @param   speed Speed in RPM
 * @retval  STEPPER_StatusTypeDef Operation status
 */
STEPPER_StatusTypeDef STEPPER_MoveSteps(STEPPER_Handle_t *hstep,
                                       uint32_t steps,
                                       STEPPER_Direction_t direction,
                                       uint16_t speed)
{
    if (hstep == NULL || !hstep->isInitialized) {
        return STEPPER_NOT_INITIALIZED;
    }

    if (hstep->isRunning) {
        return STEPPER_BUSY;
    }

    if (speed < STEPPER_MIN_SPEED_RPM || speed > STEPPER_MAX_SPEED_RPM) {
        return STEPPER_INVALID_PARAM;
    }

    /* Set direction and speed */
    hstep->direction = direction;
    hstep->currentSpeed = speed;
    hstep->stepDelay = STEPPER_CalculateDelay(speed, hstep->config.stepsPerRevolution);
    hstep->targetPosition = hstep->currentPosition +
                           (direction == STEPPER_DIR_CW ? steps : -(int32_t)steps);

    /* Start movement */
    hstep->isRunning = true;

    /* Move steps synchronously */
    for (uint32_t i = 0; i < steps; i++) {
        if (STEPPER_MoveStep(hstep) != STEPPER_OK) {
            hstep->isRunning = false;
            return STEPPER_ERROR;
        }

        /* Delay between steps */
        for (volatile uint32_t delay = 0; delay < hstep->stepDelay * 10; delay++) {
            /* Busy wait - in real implementation, use timer interrupts */
        }
    }

    hstep->isRunning = false;
    return STEPPER_OK;
}

/**
 * @brief   Move stepper motor to absolute position
 * @details Moves motor to specified absolute position
 * @param   hstep Pointer to stepper motor handle
 * @param   position Target position in steps
 * @param   speed Speed in RPM
 * @retval  STEPPER_StatusTypeDef Operation status
 */
STEPPER_StatusTypeDef STEPPER_MoveToPosition(STEPPER_Handle_t *hstep,
                                            uint32_t position,
                                            uint16_t speed)
{
    if (hstep == NULL || !hstep->isInitialized) {
        return STEPPER_NOT_INITIALIZED;
    }

    /* Calculate steps and direction */
    int32_t steps = (int32_t)position - (int32_t)hstep->currentPosition;
    STEPPER_Direction_t direction = (steps >= 0) ? STEPPER_DIR_CW : STEPPER_DIR_CCW;
    uint32_t abs_steps = abs(steps);

    return STEPPER_MoveSteps(hstep, abs_steps, direction, speed);
}

/**
 * @brief   Rotate stepper motor continuously
 * @details Starts continuous rotation at specified speed
 * @param   hstep Pointer to stepper motor handle
 * @param   direction Direction of rotation
 * @param   speed Speed in RPM
 * @retval  STEPPER_StatusTypeDef Operation status
 */
STEPPER_StatusTypeDef STEPPER_RotateContinuous(STEPPER_Handle_t *hstep,
                                              STEPPER_Direction_t direction,
                                              uint16_t speed)
{
    if (hstep == NULL || !hstep->isInitialized) {
        return STEPPER_NOT_INITIALIZED;
    }

    if (hstep->isRunning) {
        return STEPPER_BUSY;
    }

    /* Set parameters */
    hstep->direction = direction;
    hstep->currentSpeed = speed;
    hstep->stepDelay = STEPPER_CalculateDelay(speed, hstep->config.stepsPerRevolution);
    hstep->isRunning = true;

    /* In a real implementation, this would start a timer interrupt
       For now, we'll simulate continuous rotation with a loop */
    while (hstep->isRunning) {
        STEPPER_MoveStep(hstep);

        /* Delay between steps */
        for (volatile uint32_t delay = 0; delay < hstep->stepDelay * 10; delay++) {
            /* Busy wait */
        }
    }

    return STEPPER_OK;
}

/**
 * @brief   Stop stepper motor
 * @details Stops motor movement immediately
 * @param   hstep Pointer to stepper motor handle
 * @retval  STEPPER_StatusTypeDef Operation status
 */
STEPPER_StatusTypeDef STEPPER_Stop(STEPPER_Handle_t *hstep)
{
    if (hstep == NULL) {
        return STEPPER_INVALID_PARAM;
    }

    hstep->isRunning = false;

    /* Turn off all coils */
    HAL_GPIO_WritePin(hstep->pins.port1, hstep->pins.pin1, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(hstep->pins.port2, hstep->pins.pin2, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(hstep->pins.port3, hstep->pins.pin3, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(hstep->pins.port4, hstep->pins.pin4, GPIO_PIN_RESET);

    return STEPPER_OK;
}

/**
 * @brief   Emergency stop stepper motor
 * @details Stops motor immediately without deceleration
 * @param   hstep Pointer to stepper motor handle
 * @retval  STEPPER_StatusTypeDef Operation status
 */
STEPPER_StatusTypeDef STEPPER_EmergencyStop(STEPPER_Handle_t *hstep)
{
    return STEPPER_Stop(hstep);
}

/**
 * @brief   Check if stepper motor is running
 * @details Returns motor running status
 * @param   hstep Pointer to stepper motor handle
 * @retval  bool True if motor is running
 */
bool STEPPER_IsRunning(STEPPER_Handle_t *hstep)
{
    if (hstep == NULL) {
        return false;
    }

    return hstep->isRunning;
}

/**
 * @brief   Get current position
 * @details Returns current motor position in steps
 * @param   hstep Pointer to stepper motor handle
 * @retval  uint32_t Current position
 */
uint32_t STEPPER_GetPosition(STEPPER_Handle_t *hstep)
{
    if (hstep == NULL) {
        return 0;
    }

    return hstep->currentPosition;
}

/**
 * @brief   Set current position
 * @details Sets current motor position (for homing/calibration)
 * @param   hstep Pointer to stepper motor handle
 * @param   position New position value
 * @retval  STEPPER_StatusTypeDef Operation status
 */
STEPPER_StatusTypeDef STEPPER_SetPosition(STEPPER_Handle_t *hstep, uint32_t position)
{
    if (hstep == NULL) {
        return STEPPER_INVALID_PARAM;
    }

    hstep->currentPosition = position;

    return STEPPER_OK;
}

/**
 * @brief   Get motor status
 * @details Returns detailed motor status information
 * @param   hstep Pointer to stepper motor handle
 * @retval  STEPPER_StatusTypeDef Motor status
 */
STEPPER_StatusTypeDef STEPPER_GetStatus(STEPPER_Handle_t *hstep)
{
    if (hstep == NULL) {
        return STEPPER_INVALID_PARAM;
    }

    if (!hstep->isInitialized) {
        return STEPPER_NOT_INITIALIZED;
    }

    if (hstep->isRunning) {
        return STEPPER_BUSY;
    }

    return STEPPER_OK;
}

/**
 * @brief   Enable/disable limit switches
 * @details Configures limit switch functionality
 * @param   hstep Pointer to stepper motor handle
 * @param   enable Enable/disable limit switches
 * @retval  STEPPER_StatusTypeDef Operation status
 */
STEPPER_StatusTypeDef STEPPER_EnableLimitSwitches(STEPPER_Handle_t *hstep, bool enable)
{
    if (hstep == NULL) {
        return STEPPER_INVALID_PARAM;
    }

    hstep->config.enableLimitSwitches = enable;

    return STEPPER_OK;
}

/**
 * @brief   Home stepper motor
 * @details Moves motor to home position using limit switches
 * @param   hstep Pointer to stepper motor handle
 * @param   direction Direction to move for homing
 * @param   speed Homing speed in RPM
 * @retval  STEPPER_StatusTypeDef Operation status
 */
STEPPER_StatusTypeDef STEPPER_Home(STEPPER_Handle_t *hstep,
                                  STEPPER_Direction_t direction,
                                  uint16_t speed)
{
    /* Basic homing implementation - in real application, would check limit switches */
    /* For now, just move to position 0 */
    return STEPPER_MoveToPosition(hstep, 0, speed);
}

/**
 * @brief   Set acceleration/deceleration
 * @details Configures acceleration and deceleration parameters
 * @param   hstep Pointer to stepper motor handle
 * @param   acceleration Acceleration in steps/sec²
 * @param   deceleration Deceleration in steps/sec²
 * @retval  STEPPER_StatusTypeDef Operation status
 */
STEPPER_StatusTypeDef STEPPER_SetAcceleration(STEPPER_Handle_t *hstep,
                                             uint16_t acceleration,
                                             uint16_t deceleration)
{
    if (hstep == NULL) {
        return STEPPER_INVALID_PARAM;
    }

    hstep->config.acceleration = acceleration;
    hstep->config.deceleration = deceleration;

    return STEPPER_OK;
}

/**
 * @brief   Convert RPM to step delay
 * @details Calculates delay between steps for given RPM
 * @param   rpm Speed in RPM
 * @param   stepsPerRev Steps per revolution
 * @retval  uint32_t Delay in microseconds
 */
uint32_t STEPPER_RPMToDelay(uint16_t rpm, uint16_t stepsPerRev)
{
    if (rpm == 0 || stepsPerRev == 0) {
        return STEPPER_MAX_DELAY_US;
    }

    /* Calculate delay: (60 * 1000000) / (rpm * stepsPerRev) microseconds */
    uint32_t delay = (60UL * 1000000UL) / ((uint32_t)rpm * (uint32_t)stepsPerRev);

    /* Clamp to valid range */
    if (delay < STEPPER_MIN_DELAY_US) {
        delay = STEPPER_MIN_DELAY_US;
    } else if (delay > STEPPER_MAX_DELAY_US) {
        delay = STEPPER_MAX_DELAY_US;
    }

    return delay;
}

/**
 * @brief   Convert step delay to RPM
 * @details Calculates RPM for given step delay
 * @param   delay Delay between steps in microseconds
 * @param   stepsPerRev Steps per revolution
 * @retval  uint16_t Speed in RPM
 */
uint16_t STEPPER_DelayToRPM(uint32_t delay, uint16_t stepsPerRev)
{
    if (delay == 0 || stepsPerRev == 0) {
        return STEPPER_MIN_SPEED_RPM;
    }

    /* Calculate RPM: (60 * 1000000) / (delay * stepsPerRev) */
    uint32_t rpm = (60UL * 1000000UL) / ((uint32_t)delay * (uint32_t)stepsPerRev);

    /* Clamp to valid range */
    if (rpm < STEPPER_MIN_SPEED_RPM) {
        rpm = STEPPER_MIN_SPEED_RPM;
    } else if (rpm > STEPPER_MAX_SPEED_RPM) {
        rpm = STEPPER_MAX_SPEED_RPM;
    }

    return (uint16_t)rpm;
}

/**
 * @brief   Get default configuration
 * @details Returns a default configuration structure
 * @param   None
 * @retval  STEPPER_Config_t Default configuration
 */
STEPPER_Config_t STEPPER_GetDefaultConfig(void)
{
    STEPPER_Config_t config = {
        .stepsPerRevolution = STEPPER_DEFAULT_STEPS_PER_REV,
        .maxSpeedRPM = 500,
        .acceleration = 1000,
        .deceleration = 1000,
        .stepMode = STEPPER_MODE_FULL_STEP,
        .enableLimitSwitches = false
    };

    return config;
}

/**
 * @brief   Get default pin configuration
 * @details Returns a default pin configuration for common stepper motors
 * @param   None
 * @retval  STEPPER_Pins_t Default pin configuration
 */
STEPPER_Pins_t STEPPER_GetDefaultPins(void)
{
    STEPPER_Pins_t pins = {
        .port1 = GPIOE, .pin1 = GPIO_PIN_4,   /* PE4 */
        .port2 = GPIOE, .pin2 = GPIO_PIN_5,   /* PE5 */
        .port3 = GPIOE, .pin3 = GPIO_PIN_6,   /* PE6 */
        .port4 = GPIOB, .pin4 = GPIO_PIN_6    /* PB6 */
    };

    return pins;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Initialize MSP (GPIO pins)
 * @details Configures GPIO pins for stepper motor control
 * @param   hstep Pointer to stepper motor handle
 * @retval  None
 */
static void STEPPER_MspInit(STEPPER_Handle_t *hstep)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIO clocks for all used ports */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

    /* Configure coil pins */
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    /* Coil 1 */
    GPIO_InitStruct.Pin = hstep->pins.pin1;
    HAL_GPIO_Init(hstep->pins.port1, &GPIO_InitStruct);

    /* Coil 2 */
    GPIO_InitStruct.Pin = hstep->pins.pin2;
    HAL_GPIO_Init(hstep->pins.port2, &GPIO_InitStruct);

    /* Coil 3 */
    GPIO_InitStruct.Pin = hstep->pins.pin3;
    HAL_GPIO_Init(hstep->pins.port3, &GPIO_InitStruct);

    /* Coil 4 */
    GPIO_InitStruct.Pin = hstep->pins.pin4;
    HAL_GPIO_Init(hstep->pins.port4, &GPIO_InitStruct);

    /* Set all coils to low initially */
    HAL_GPIO_WritePin(hstep->pins.port1, hstep->pins.pin1, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(hstep->pins.port2, hstep->pins.pin2, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(hstep->pins.port3, hstep->pins.pin3, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(hstep->pins.port4, hstep->pins.pin4, GPIO_PIN_RESET);
}

/**
 * @brief   Deinitialize MSP
 * @details Resets GPIO pins to default state
 * @param   hstep Pointer to stepper motor handle
 * @retval  None
 */
static void STEPPER_MspDeInit(STEPPER_Handle_t *hstep)
{
    /* Reset pins to input mode */
    HAL_GPIO_DeInit(hstep->pins.port1, hstep->pins.pin1);
    HAL_GPIO_DeInit(hstep->pins.port2, hstep->pins.pin2);
    HAL_GPIO_DeInit(hstep->pins.port3, hstep->pins.pin3);
    HAL_GPIO_DeInit(hstep->pins.port4, hstep->pins.pin4);
}

/**
 * @brief   Set step pattern
 * @details Sets the GPIO pins according to the step sequence
 * @param   hstep Pointer to stepper motor handle
 * @param   step Step number in sequence
 * @retval  None
 */
static void STEPPER_SetStep(STEPPER_Handle_t *hstep, uint8_t step)
{
    const uint8_t *stepPattern = NULL;
    uint8_t maxSteps = STEPPER_GetMaxSteps(hstep->config.stepMode);

    /* Get the appropriate step sequence */
    switch (hstep->config.stepMode) {
        case STEPPER_MODE_FULL_STEP:
            stepPattern = fullStepSequence[step % 4];
            break;
        case STEPPER_MODE_HALF_STEP:
            stepPattern = halfStepSequence[step % 8];
            break;
        case STEPPER_MODE_WAVE_DRIVE:
            stepPattern = waveDriveSequence[step % 4];
            break;
        default:
            stepPattern = fullStepSequence[step % 4];
            break;
    }

    /* Set coil states */
    HAL_GPIO_WritePin(hstep->pins.port1, hstep->pins.pin1,
                     stepPattern[0] ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(hstep->pins.port2, hstep->pins.pin2,
                     stepPattern[1] ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(hstep->pins.port3, hstep->pins.pin3,
                     stepPattern[2] ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(hstep->pins.port4, hstep->pins.pin4,
                     stepPattern[3] ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**
 * @brief   Get maximum steps for step mode
 * @details Returns the number of steps in the sequence for the given mode
 * @param   mode Step mode
 * @retval  uint8_t Maximum steps
 */
static uint8_t STEPPER_GetMaxSteps(STEPPER_StepMode_t mode)
{
    switch (mode) {
        case STEPPER_MODE_FULL_STEP:
        case STEPPER_MODE_WAVE_DRIVE:
            return 4;
        case STEPPER_MODE_HALF_STEP:
            return 8;
        case STEPPER_MODE_MICROSTEP_4:
            return 16;
        case STEPPER_MODE_MICROSTEP_8:
            return 32;
        case STEPPER_MODE_MICROSTEP_16:
            return 64;
        default:
            return 4;
    }
}

/**
 * @brief   Calculate step delay from RPM
 * @details Converts RPM to microseconds delay between steps
 * @param   rpm Speed in RPM
 * @param   stepsPerRev Steps per revolution
 * @retval  uint32_t Delay in microseconds
 */
static uint32_t STEPPER_CalculateDelay(uint16_t rpm, uint16_t stepsPerRev)
{
    return STEPPER_RPMToDelay(rpm, stepsPerRev);
}

/**
 * @brief   Validate configuration
 * @details Checks if configuration parameters are valid
 * @param   config Pointer to configuration structure
 * @retval  STEPPER_StatusTypeDef Validation status
 */
static STEPPER_StatusTypeDef STEPPER_ValidateConfig(STEPPER_Config_t *config)
{
    if (config->stepsPerRevolution == 0) {
        return STEPPER_INVALID_PARAM;
    }

    if (config->maxSpeedRPM < STEPPER_MIN_SPEED_RPM ||
        config->maxSpeedRPM > STEPPER_MAX_SPEED_RPM) {
        return STEPPER_INVALID_PARAM;
    }

    return STEPPER_OK;
}

/**
 * @brief   Move one step
 * @details Executes a single step in the current direction
 * @param   hstep Pointer to stepper motor handle
 * @retval  STEPPER_StatusTypeDef Operation status
 */
static STEPPER_StatusTypeDef STEPPER_MoveStep(STEPPER_Handle_t *hstep)
{
    static uint8_t currentStep = 0;

    /* Set step pattern */
    STEPPER_SetStep(hstep, currentStep);

    /* Update position */
    if (hstep->direction == STEPPER_DIR_CW) {
        hstep->currentPosition++;
        currentStep++;
    } else {
        hstep->currentPosition--;
        currentStep--;
    }

    /* Wrap step counter */
    uint8_t maxSteps = STEPPER_GetMaxSteps(hstep->config.stepMode);
    currentStep %= maxSteps;

    return STEPPER_OK;
}
