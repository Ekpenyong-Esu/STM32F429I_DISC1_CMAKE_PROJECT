/**
  ******************************************************************************
  * @file    servo.c
  * @brief   Servo motor driver implementation
  * @details This file provides the implementation of servo motor functions
  *          using PWM signals from timer peripherals on STM32F429 Discovery board.
  * @version 1.0
  * @date    2025-01-13
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "servo.h"
#include "tim.h"
#include "stm32f4xx_hal.h"
#include <stdlib.h>
#include <string.h>

/* Private defines -----------------------------------------------------------*/

/** @defgroup SERVO_Private_Defines Private Defines
 * @{
 */

/* PWM timer configuration for 50Hz (20ms period) */
#define SERVO_PWM_PRESCALER     85      /* 86MHz / 86 = 1MHz timer clock */
#define SERVO_PWM_PERIOD        19999   /* 1MHz / 20000 = 50Hz (20ms period) */

/* Pulse width calculation: pulse = (pulseWidth_us * timer_clock) / 1000000 */
#define SERVO_PULSE_CALC(pulseWidth)  ((pulseWidth * 100) / 100)  /* For 1MHz timer */

/** @} */

/* Private variables ---------------------------------------------------------*/

/** @defgroup SERVO_Private_Variables Private Variables
 * @{
 */

/* Global servo handle for interrupt handling */
static SERVO_Handle_t *g_hservo = NULL;

/** @} */

/* Private function prototypes -----------------------------------------------*/
static void SERVO_MspInit(SERVO_Handle_t *hservo);
static void SERVO_MspDeInit(SERVO_Handle_t *hservo);
static SERVO_StatusTypeDef SERVO_ValidateConfig(SERVO_Config_t *config);
static uint16_t SERVO_CalculatePulse(uint16_t pulseWidth);

/* Exported functions -------------------------------------------------------*/

/**
 * @brief   Initialize servo motor
 * @details Configures timer and GPIO for PWM servo control
 * @param   hservo Pointer to servo motor handle
 * @param   htim Pointer to timer handle for PWM
 * @param   channel Timer PWM channel (TIM_CHANNEL_1, etc.)
 * @param   gpioPort GPIO port for PWM pin
 * @param   gpioPin GPIO pin for PWM signal
 * @retval  SERVO_StatusTypeDef Operation status
 */
SERVO_StatusTypeDef SERVO_Init(SERVO_Handle_t *hservo,
                              TIM_HandleTypeDef *htim,
                              uint32_t channel,
                              GPIO_TypeDef *gpioPort,
                              uint16_t gpioPin)
{
    if (hservo == NULL || htim == NULL || gpioPort == NULL) {
        return SERVO_INVALID_PARAM;
    }

    /* Initialize structure */
    memset(hservo, 0, sizeof(SERVO_Handle_t));
    hservo->htim = htim;
    hservo->channel = channel;
    hservo->gpioPort = gpioPort;
    hservo->gpioPin = gpioPin;
    g_hservo = hservo;

    /* Initialize MSP (GPIO and PWM) */
    SERVO_MspInit(hservo);

    /* Configure timer for PWM */
    TIM_PWM_Init(htim, htim->Instance, SERVO_PWM_PRESCALER, SERVO_PWM_PERIOD);

    /* Set default configuration */
    SERVO_Config_t default_config = SERVO_GetDefaultConfig();
    SERVO_Config(hservo, &default_config);

    /* Start PWM */
    TIM_PWM_Start(htim, channel);

    /* Move to default position */
    SERVO_SetAngle(hservo, hservo->config.defaultAngle);

    hservo->isInitialized = true;

    return SERVO_OK;
}

/**
 * @brief   Deinitialize servo motor
 * @details Stops PWM and releases resources
 * @param   hservo Pointer to servo motor handle
 * @retval  SERVO_StatusTypeDef Operation status
 */
SERVO_StatusTypeDef SERVO_DeInit(SERVO_Handle_t *hservo)
{
    if (hservo == NULL) {
        return SERVO_INVALID_PARAM;
    }

    /* Stop PWM */
    TIM_PWM_Stop(hservo->htim, hservo->channel);

    /* Deinitialize MSP */
    SERVO_MspDeInit(hservo);

    hservo->isInitialized = false;
    g_hservo = NULL;

    return SERVO_OK;
}

/**
 * @brief   Configure servo motor parameters
 * @details Sets servo configuration parameters
 * @param   hservo Pointer to servo motor handle
 * @param   config Pointer to configuration structure
 * @retval  SERVO_StatusTypeDef Operation status
 */
SERVO_StatusTypeDef SERVO_Config(SERVO_Handle_t *hservo, SERVO_Config_t *config)
{
    if (hservo == NULL || config == NULL) {
        return SERVO_INVALID_PARAM;
    }

    /* Validate configuration */
    if (SERVO_ValidateConfig(config) != SERVO_OK) {
        return SERVO_INVALID_PARAM;
    }

    /* Store configuration */
    hservo->config = *config;

    return SERVO_OK;
}

/**
 * @brief   Set servo angle
 * @details Moves servo to specified angle
 * @param   hservo Pointer to servo motor handle
 * @param   angle Angle in degrees (0-180)
 * @retval  SERVO_StatusTypeDef Operation status
 */
SERVO_StatusTypeDef SERVO_SetAngle(SERVO_Handle_t *hservo, uint16_t angle)
{
    if (hservo == NULL || !hservo->isInitialized) {
        return SERVO_NOT_INITIALIZED;
    }

    /* Validate angle */
    if (!SERVO_IsValidAngle(angle, &hservo->config)) {
        return SERVO_OUT_OF_RANGE;
    }

    /* Calculate pulse width for angle */
    uint16_t pulseWidth = SERVO_AngleToPulseWidth(angle, &hservo->config);

    /* Set pulse width */
    SERVO_StatusTypeDef status = SERVO_SetPulseWidth(hservo, pulseWidth);
    if (status == SERVO_OK) {
        hservo->currentAngle = angle;
    }

    return status;
}

/**
 * @brief   Set servo pulse width
 * @details Sets PWM pulse width directly
 * @param   hservo Pointer to servo motor handle
 * @param   pulseWidth Pulse width in microseconds
 * @retval  SERVO_StatusTypeDef Operation status
 */
SERVO_StatusTypeDef SERVO_SetPulseWidth(SERVO_Handle_t *hservo, uint16_t pulseWidth)
{
    if (hservo == NULL || !hservo->isInitialized) {
        return SERVO_NOT_INITIALIZED;
    }

    /* Validate pulse width */
    if (!SERVO_IsValidPulseWidth(pulseWidth, &hservo->config)) {
        return SERVO_INVALID_PARAM;
    }

    /* Calculate PWM compare value */
    uint16_t pulse = SERVO_CalculatePulse(pulseWidth);

    /* Set PWM duty cycle */
    TIM_PWM_SetDuty(hservo->htim, hservo->channel, pulse);

    return SERVO_OK;
}

/**
 * @brief   Get current servo angle
 * @details Returns current servo angle
 * @param   hservo Pointer to servo motor handle
 * @retval  uint16_t Current angle in degrees
 */
uint16_t SERVO_GetAngle(SERVO_Handle_t *hservo)
{
    if (hservo == NULL) {
        return 0;
    }

    return hservo->currentAngle;
}

/**
 * @brief   Move servo to minimum angle
 * @details Moves servo to configured minimum angle
 * @param   hservo Pointer to servo motor handle
 * @retval  SERVO_StatusTypeDef Operation status
 */
SERVO_StatusTypeDef SERVO_MoveToMin(SERVO_Handle_t *hservo)
{
    if (hservo == NULL) {
        return SERVO_INVALID_PARAM;
    }

    return SERVO_SetAngle(hservo, hservo->config.minAngle);
}

/**
 * @brief   Move servo to maximum angle
 * @details Moves servo to configured maximum angle
 * @param   hservo Pointer to servo motor handle
 * @retval  SERVO_StatusTypeDef Operation status
 */
SERVO_StatusTypeDef SERVO_MoveToMax(SERVO_Handle_t *hservo)
{
    if (hservo == NULL) {
        return SERVO_INVALID_PARAM;
    }

    return SERVO_SetAngle(hservo, hservo->config.maxAngle);
}

/**
 * @brief   Move servo to center position
 * @details Moves servo to 90 degrees (center)
 * @param   hservo Pointer to servo motor handle
 * @retval  SERVO_StatusTypeDef Operation status
 */
SERVO_StatusTypeDef SERVO_MoveToCenter(SERVO_Handle_t *hservo)
{
    if (hservo == NULL) {
        return SERVO_INVALID_PARAM;
    }

    return SERVO_SetAngle(hservo, 90);
}

/**
 * @brief   Sweep servo across range
 * @details Moves servo from min to max angle and back
 * @param   hservo Pointer to servo motor handle
 * @param   speed Delay between angle steps in milliseconds
 * @retval  SERVO_StatusTypeDef Operation status
 */
SERVO_StatusTypeDef SERVO_Sweep(SERVO_Handle_t *hservo, uint16_t speed)
{
    if (hservo == NULL || !hservo->isInitialized) {
        return SERVO_NOT_INITIALIZED;
    }

    /* Sweep from min to max */
    for (uint16_t angle = hservo->config.minAngle; angle <= hservo->config.maxAngle; angle++) {
        SERVO_SetAngle(hservo, angle);
        HAL_Delay(speed);
    }

    /* Sweep from max to min */
    for (uint16_t angle = hservo->config.maxAngle; angle >= hservo->config.minAngle; angle--) {
        SERVO_SetAngle(hservo, angle);
        HAL_Delay(speed);
    }

    return SERVO_OK;
}

/**
 * @brief   Calibrate servo minimum angle
 * @details Sets current position as minimum angle
 * @param   hservo Pointer to servo motor handle
 * @param   angle Actual angle in degrees
 * @retval  SERVO_StatusTypeDef Operation status
 */
SERVO_StatusTypeDef SERVO_CalibrateMin(SERVO_Handle_t *hservo, uint16_t angle)
{
    if (hservo == NULL) {
        return SERVO_INVALID_PARAM;
    }

    hservo->config.minAngle = angle;

    return SERVO_OK;
}

/**
 * @brief   Calibrate servo maximum angle
 * @details Sets current position as maximum angle
 * @param   hservo Pointer to servo motor handle
 * @param   angle Actual angle in degrees
 * @retval  SERVO_StatusTypeDef Operation status
 */
SERVO_StatusTypeDef SERVO_CalibrateMax(SERVO_Handle_t *hservo, uint16_t angle)
{
    if (hservo == NULL) {
        return SERVO_INVALID_PARAM;
    }

    hservo->config.maxAngle = angle;

    return SERVO_OK;
}

/**
 * @brief   Reset servo calibration
 * @details Resets calibration to default values
 * @param   hservo Pointer to servo motor handle
 * @retval  SERVO_StatusTypeDef Operation status
 */
SERVO_StatusTypeDef SERVO_ResetCalibration(SERVO_Handle_t *hservo)
{
    if (hservo == NULL) {
        return SERVO_INVALID_PARAM;
    }

    SERVO_Config_t default_config = SERVO_GetDefaultConfig();
    hservo->config.minAngle = default_config.minAngle;
    hservo->config.maxAngle = default_config.maxAngle;
    hservo->config.minPulseWidth = default_config.minPulseWidth;
    hservo->config.maxPulseWidth = default_config.maxPulseWidth;

    return SERVO_OK;
}

/**
 * @brief   Convert angle to pulse width
 * @details Calculates PWM pulse width for given angle
 * @param   angle Angle in degrees
 * @param   config Pointer to servo configuration
 * @retval  uint16_t Pulse width in microseconds
 */
uint16_t SERVO_AngleToPulseWidth(uint16_t angle, SERVO_Config_t *config)
{
    if (config == NULL) {
        return SERVO_DEFAULT_PULSE_WIDTH_US;
    }

    /* Linear interpolation: pulse = min_pulse + (angle * (max_pulse - min_pulse)) / (max_angle - min_angle) */
    uint16_t pulseRange = config->maxPulseWidth - config->minPulseWidth;
    uint16_t angleRange = config->maxAngle - config->minAngle;

    if (angleRange == 0) {
        return config->minPulseWidth;
    }

    uint16_t pulse = config->minPulseWidth + ((angle - config->minAngle) * pulseRange) / angleRange;

    return pulse;
}

/**
 * @brief   Convert pulse width to angle
 * @details Calculates angle for given PWM pulse width
 * @param   pulseWidth Pulse width in microseconds
 * @param   config Pointer to servo configuration
 * @retval  uint16_t Angle in degrees
 */
uint16_t SERVO_PulseWidthToAngle(uint16_t pulseWidth, SERVO_Config_t *config)
{
    if (config == NULL) {
        return SERVO_DEFAULT_ANGLE;
    }

    /* Linear interpolation: angle = min_angle + (pulse * (max_angle - min_angle)) / (max_pulse - min_pulse) */
    uint16_t pulseRange = config->maxPulseWidth - config->minPulseWidth;
    uint16_t angleRange = config->maxAngle - config->minAngle;

    if (pulseRange == 0) {
        return config->minAngle;
    }

    uint16_t angle = config->minAngle + ((pulseWidth - config->minPulseWidth) * angleRange) / pulseRange;

    return angle;
}

/**
 * @brief   Get default configuration
 * @details Returns a default configuration structure
 * @param   None
 * @retval  SERVO_Config_t Default configuration
 */
SERVO_Config_t SERVO_GetDefaultConfig(void)
{
    SERVO_Config_t config = {
        .minAngle = SERVO_MIN_ANGLE,
        .maxAngle = SERVO_MAX_ANGLE,
        .minPulseWidth = SERVO_MIN_PULSE_WIDTH_US,
        .maxPulseWidth = SERVO_MAX_PULSE_WIDTH_US,
        .defaultAngle = SERVO_DEFAULT_ANGLE
    };

    return config;
}

/**
 * @brief   Validate servo angle
 * @details Checks if angle is within valid range
 * @param   angle Angle to validate
 * @param   config Pointer to servo configuration
 * @retval  bool True if angle is valid
 */
bool SERVO_IsValidAngle(uint16_t angle, SERVO_Config_t *config)
{
    if (config == NULL) {
        return (angle >= SERVO_MIN_ANGLE && angle <= SERVO_MAX_ANGLE);
    }

    return (angle >= config->minAngle && angle <= config->maxAngle);
}

/**
 * @brief   Validate pulse width
 * @details Checks if pulse width is within valid range
 * @param   pulseWidth Pulse width to validate
 * @param   config Pointer to servo configuration
 * @retval  bool True if pulse width is valid
 */
bool SERVO_IsValidPulseWidth(uint16_t pulseWidth, SERVO_Config_t *config)
{
    if (config == NULL) {
        return (pulseWidth >= SERVO_MIN_PULSE_WIDTH_US && pulseWidth <= SERVO_MAX_PULSE_WIDTH_US);
    }

    return (pulseWidth >= config->minPulseWidth && pulseWidth <= config->maxPulseWidth);
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Initialize MSP (GPIO and PWM)
 * @details Configures GPIO pin for PWM output
 * @param   hservo Pointer to servo motor handle
 * @retval  None
 */
static void SERVO_MspInit(SERVO_Handle_t *hservo)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIO clock */
    if (hservo->gpioPort == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    } else if (hservo->gpioPort == GPIOB) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    } else if (hservo->gpioPort == GPIOC) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    } else if (hservo->gpioPort == GPIOD) {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    }

    /* Configure GPIO pin for PWM output */
    GPIO_InitStruct.Pin = hservo->gpioPin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    /* Set alternate function based on timer and channel */
    if (hservo->htim->Instance == TIM1) {
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
    } else if (hservo->htim->Instance == TIM2) {
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    } else if (hservo->htim->Instance == TIM3) {
        GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    } else if (hservo->htim->Instance == TIM4) {
        GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
    } else if (hservo->htim->Instance == TIM5) {
        GPIO_InitStruct.Alternate = GPIO_AF2_TIM5;
    }

    HAL_GPIO_Init(hservo->gpioPort, &GPIO_InitStruct);
}

/**
 * @brief   Deinitialize MSP
 * @details Resets GPIO pin to default state
 * @param   hservo Pointer to servo motor handle
 * @retval  None
 */
static void SERVO_MspDeInit(SERVO_Handle_t *hservo)
{
    /* Reset pin to input mode */
    HAL_GPIO_DeInit(hservo->gpioPort, hservo->gpioPin);
}

/**
 * @brief   Validate configuration
 * @details Checks if configuration parameters are valid
 * @param   config Pointer to configuration structure
 * @retval  SERVO_StatusTypeDef Validation status
 */
static SERVO_StatusTypeDef SERVO_ValidateConfig(SERVO_Config_t *config)
{
    if (config->minAngle >= config->maxAngle) {
        return SERVO_INVALID_PARAM;
    }

    if (config->minPulseWidth >= config->maxPulseWidth) {
        return SERVO_INVALID_PARAM;
    }

    if (config->defaultAngle < config->minAngle || config->defaultAngle > config->maxAngle) {
        return SERVO_INVALID_PARAM;
    }

    return SERVO_OK;
}

/**
 * @brief   Calculate PWM pulse value
 * @details Converts microseconds to timer compare value
 * @param   pulseWidth Pulse width in microseconds
 * @retval  uint16_t Timer compare value
 */
static uint16_t SERVO_CalculatePulse(uint16_t pulseWidth)
{
    /* For 1MHz timer clock and 20ms period:
     * pulse_value = (pulseWidth_us * 100) / 100 = pulseWidth_us
     * Since 1MHz = 1 tick per microsecond
     */
    return pulseWidth;
}
