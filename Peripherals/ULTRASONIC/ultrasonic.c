/**
  ******************************************************************************
  * @file    ultrasonic.c
  * @brief   Ultrasonic distance sensor driver implementation
  * @details This file provides the implementation of ultrasonic distance sensors
  *          using timer input capture for echo pulse measurement.
  * @version 1.0
  * @date    2025-01-13
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "ultrasonic.h"
#include "tim.h"
#include "log.h"
#include "gpio.h"
#include "stm32f4xx_hal.h"
#include <stdlib.h>
#include <string.h>

/* Private defines -----------------------------------------------------------*/

/** @defgroup ULTRASONIC_Private_Defines Private Defines
 * @{
 */

/* Speed of sound correction factor per degree Celsius */
#define ULTRASONIC_SPEED_CORRECTION    0.6f    /* m/s per °C */

/** @} */

/* Private variables ---------------------------------------------------------*/

/** @defgroup ULTRASONIC_Private_Variables Private Variables
 * @{
 */

/* Global ultrasonic handle for interrupt handling */
static ULTRASONIC_Handle_t *g_hultra = NULL;

/** @} */

/* Private function prototypes -----------------------------------------------*/
static void ULTRASONIC_MspInit(ULTRASONIC_Handle_t *hultra);
static void ULTRASONIC_MspDeInit(ULTRASONIC_Handle_t *hultra);
static ULTRASONIC_StatusTypeDef ULTRASONIC_ValidateConfig(ULTRASONIC_Config_t *config);
static uint16_t ULTRASONIC_CalculateDistance(uint32_t echoTime, uint8_t temperature);
static float ULTRASONIC_GetSpeedOfSound(uint8_t temperature);

/* Exported functions -------------------------------------------------------*/

/**
 * @brief   Initialize ultrasonic sensor
 * @details Configures GPIO pins and timer for distance measurement
 * @param   hultra Pointer to ultrasonic sensor handle
 * @param   htim Pointer to timer handle for input capture
 * @param   channel Timer input capture channel
 * @param   pins Pointer to GPIO pin configuration
 * @retval  ULTRASONIC_StatusTypeDef Operation status
 */
ULTRASONIC_StatusTypeDef ULTRASONIC_Init(ULTRASONIC_Handle_t *hultra,
                                        TIM_HandleTypeDef *htim,
                                        uint32_t channel,
                                        ULTRASONIC_Pins_t *pins)
{
    if (hultra == NULL || htim == NULL || pins == NULL) {
        log_error("ULTRASONIC: Invalid parameters provided to ULTRASONIC_Init");
        return ULTRASONIC_INVALID_PARAM;
    }

    log_debug("ULTRASONIC: Initializing ultrasonic sensor with channel %d", channel);

    /* Initialize structure */
    memset(hultra, 0, sizeof(ULTRASONIC_Handle_t));
    hultra->htim = htim;
    hultra->channel = channel;
    hultra->pins = *pins;
    g_hultra = hultra;

    /* Initialize MSP (GPIO pins) */
    ULTRASONIC_MspInit(hultra);
    log_debug("ULTRASONIC: GPIO pins initialized");

    /* Configure timer for input capture */
    if (TIM_IC_Init(htim, htim->Instance, 0, 0xFFFF) != HAL_OK) {
        log_error("ULTRASONIC: Failed to initialize timer for input capture");
        return ULTRASONIC_ERROR;
    }
    log_debug("ULTRASONIC: Timer configured for input capture");

    /* Configure input capture channel */
    if (TIM_IC_ConfigChannel(htim, channel, TIM_ICPOLARITY_RISING) != HAL_OK) {
        log_error("ULTRASONIC: Failed to configure input capture channel");
        return ULTRASONIC_ERROR;
    }
    log_debug("ULTRASONIC: Input capture channel configured");

    /* Set default configuration */
    ULTRASONIC_Config_t default_config = ULTRASONIC_GetDefaultConfig();
    ULTRASONIC_Config(hultra, &default_config);

    /* Start timer */
    TIM_Start(htim);

    hultra->isInitialized = true;

    log_info("ULTRASONIC: Ultrasonic sensor initialized successfully");
    return ULTRASONIC_OK;
}

/**
 * @brief   Deinitialize ultrasonic sensor
 * @details Stops timer and releases resources
 * @param   hultra Pointer to ultrasonic sensor handle
 * @retval  ULTRASONIC_StatusTypeDef Operation status
 */
ULTRASONIC_StatusTypeDef ULTRASONIC_DeInit(ULTRASONIC_Handle_t *hultra)
{
    if (hultra == NULL) {
        return ULTRASONIC_INVALID_PARAM;
    }

    /* Stop timer */
    TIM_Stop(hultra->htim);

    /* Deinitialize MSP */
    ULTRASONIC_MspDeInit(hultra);

    hultra->isInitialized = false;
    g_hultra = NULL;

    return ULTRASONIC_OK;
}

/**
 * @brief   Configure ultrasonic sensor parameters
 * @details Sets sensor configuration parameters
 * @param   hultra Pointer to ultrasonic sensor handle
 * @param   config Pointer to configuration structure
 * @retval  ULTRASONIC_StatusTypeDef Operation status
 */
ULTRASONIC_StatusTypeDef ULTRASONIC_Config(ULTRASONIC_Handle_t *hultra,
                                          ULTRASONIC_Config_t *config)
{
    if (hultra == NULL || config == NULL) {
        return ULTRASONIC_INVALID_PARAM;
    }

    /* Validate configuration */
    if (ULTRASONIC_ValidateConfig(config) != ULTRASONIC_OK) {
        return ULTRASONIC_INVALID_PARAM;
    }

    /* Store configuration */
    hultra->config = *config;

    return ULTRASONIC_OK;
}

/**
 * @brief   Start distance measurement
 * @details Triggers ultrasonic pulse and starts measurement
 * @param   hultra Pointer to ultrasonic sensor handle
 * @retval  ULTRASONIC_StatusTypeDef Operation status
 */
ULTRASONIC_StatusTypeDef ULTRASONIC_StartMeasurement(ULTRASONIC_Handle_t *hultra)
{
    if (hultra == NULL || !hultra->isInitialized) {
        log_error("ULTRASONIC: Sensor not initialized");
        return ULTRASONIC_NOT_INITIALIZED;
    }

    if (!hultra->measurementDone) {
        log_warning("ULTRASONIC: Sensor busy, previous measurement not complete");
        return ULTRASONIC_BUSY;
    }

    log_debug("ULTRASONIC: Starting distance measurement");

    /* Reset measurement flags */
    hultra->measurementDone = false;
    hultra->echoStart = 0;
    hultra->echoEnd = 0;

    /* Generate trigger pulse */
    HAL_GPIO_WritePin(hultra->pins.triggerPort, hultra->pins.triggerPin, GPIO_PIN_SET);

    /* Wait for trigger pulse width (10us) */
    for (volatile uint32_t i = 0; i < (SystemCoreClock / 1000000) * ULTRASONIC_TRIGGER_PULSE_WIDTH; i++) {
        /* Busy wait for approximately 10us */
    }

    HAL_GPIO_WritePin(hultra->pins.triggerPort, hultra->pins.triggerPin, GPIO_PIN_RESET);

    /* Enable input capture interrupt for echo detection */
    TIM_IC_Start_IT(hultra->htim, hultra->channel);

    return ULTRASONIC_OK;
}

/**
 * @brief   Get measured distance
 * @details Returns the last measured distance in millimeters
 * @param   hultra Pointer to ultrasonic sensor handle
 * @retval  uint16_t Distance in millimeters (0 if no valid measurement)
 */
uint16_t ULTRASONIC_GetDistance(ULTRASONIC_Handle_t *hultra)
{
    if (hultra == NULL) {
        return 0;
    }

    return hultra->lastDistance;
}

/**
 * @brief   Check if measurement is complete
 * @details Returns true if a measurement cycle has finished
 * @param   hultra Pointer to ultrasonic sensor handle
 * @retval  bool True if measurement is complete
 */
bool ULTRASONIC_IsMeasurementComplete(ULTRASONIC_Handle_t *hultra)
{
    if (hultra == NULL) {
        return false;
    }

    return hultra->measurementDone;
}

/**
 * @brief   Wait for measurement completion
 * @details Blocks until measurement is complete or timeout occurs
 * @param   hultra Pointer to ultrasonic sensor handle
 * @param   timeout Timeout in milliseconds
 * @retval  ULTRASONIC_StatusTypeDef Operation status
 */
ULTRASONIC_StatusTypeDef ULTRASONIC_WaitForMeasurement(ULTRASONIC_Handle_t *hultra,
                                                      uint32_t timeout)
{
    if (hultra == NULL || !hultra->isInitialized) {
        return ULTRASONIC_NOT_INITIALIZED;
    }

    uint32_t startTime = HAL_GetTick();

    while (!hultra->measurementDone) {
        if ((HAL_GetTick() - startTime) > timeout) {
            /* Disable input capture interrupt */
            TIM_IC_Stop(hultra->htim, hultra->channel);
            return ULTRASONIC_TIMEOUT;
        }
        HAL_Delay(1);
    }

    return ULTRASONIC_OK;
}

/**
 * @brief   Perform single distance measurement
 * @details Triggers measurement and waits for completion
 * @param   hultra Pointer to ultrasonic sensor handle
 * @retval  uint16_t Distance in millimeters (0 if measurement failed)
 */
uint16_t ULTRASONIC_MeasureDistance(ULTRASONIC_Handle_t *hultra)
{
    if (hultra == NULL || !hultra->isInitialized) {
        return 0;
    }

    /* Start measurement */
    if (ULTRASONIC_StartMeasurement(hultra) != ULTRASONIC_OK) {
        return 0;
    }

    /* Wait for completion */
    if (ULTRASONIC_WaitForMeasurement(hultra, hultra->config.measurementTimeout / 1000) != ULTRASONIC_OK) {
        return 0;
    }

    return hultra->lastDistance;
}

/**
 * @brief   Set ambient temperature for speed correction
 * @details Updates temperature for accurate distance calculation
 * @param   hultra Pointer to ultrasonic sensor handle
 * @param   temperature Temperature in Celsius
 * @retval  ULTRASONIC_StatusTypeDef Operation status
 */
ULTRASONIC_StatusTypeDef ULTRASONIC_SetTemperature(ULTRASONIC_Handle_t *hultra,
                                                  uint8_t temperature)
{
    if (hultra == NULL) {
        return ULTRASONIC_INVALID_PARAM;
    }

    hultra->config.temperature = temperature;

    return ULTRASONIC_OK;
}

/**
 * @brief   Get default configuration
 * @details Returns a default configuration structure
 * @param   None
 * @retval  ULTRASONIC_Config_t Default configuration
 */
ULTRASONIC_Config_t ULTRASONIC_GetDefaultConfig(void)
{
    ULTRASONIC_Config_t config = {
        .triggerTimeout = ULTRASONIC_DEFAULT_TRIGGER_TIMEOUT,
        .measurementTimeout = ULTRASONIC_DEFAULT_MEASUREMENT_TIMEOUT,
        .minDistance = ULTRASONIC_DEFAULT_MIN_DISTANCE,
        .maxDistance = ULTRASONIC_DEFAULT_MAX_DISTANCE,
        .temperature = ULTRASONIC_DEFAULT_TEMPERATURE
    };

    return config;
}

/**
 * @brief   Timer input capture callback
 * @details Should be called from timer interrupt handler
 * @param   hultra Pointer to ultrasonic sensor handle
 * @retval  None
 */
void ULTRASONIC_TIM_IC_CaptureCallback(ULTRASONIC_Handle_t *hultra)
{
    if (hultra == NULL) {
        return;
    }

    uint32_t currentCapture = TIM_IC_GetCapture(hultra->htim, hultra->channel);

    if (hultra->echoStart == 0) {
        /* Rising edge - echo start */
        hultra->echoStart = currentCapture;

        /* Switch to falling edge detection */
        TIM_IC_ConfigChannel(hultra->htim, hultra->channel, TIM_ICPOLARITY_FALLING);
    } else {
        /* Falling edge - echo end */
        hultra->echoEnd = currentCapture;

        /* Calculate echo time */
        uint32_t echoTime;
        if (hultra->echoEnd >= hultra->echoStart) {
            echoTime = hultra->echoEnd - hultra->echoStart;
        } else {
            /* Timer overflow */
            echoTime = (hultra->htim->Instance->ARR - hultra->echoStart) + hultra->echoEnd;
        }

        /* Calculate distance */
        hultra->lastDistance = ULTRASONIC_CalculateDistance(echoTime, hultra->config.temperature);

        /* Reset for next measurement */
        hultra->echoStart = 0;
        hultra->echoEnd = 0;
        hultra->measurementDone = true;

        log_debug("ULTRASONIC: Measurement complete, distance: %d mm", hultra->lastDistance);

        /* Switch back to rising edge detection */
        TIM_IC_ConfigChannel(hultra->htim, hultra->channel, TIM_ICPOLARITY_RISING);

        /* Disable input capture interrupt */
        TIM_IC_Stop(hultra->htim, hultra->channel);
    }
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Initialize MSP (GPIO pins)
 * @details Configures GPIO pins for trigger and echo
 * @param   hultra Pointer to ultrasonic sensor handle
 * @retval  None
 */
static void ULTRASONIC_MspInit(ULTRASONIC_Handle_t *hultra)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIO clocks */
    if (hultra->pins.triggerPort == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    } else if (hultra->pins.triggerPort == GPIOB) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    } else if (hultra->pins.triggerPort == GPIOC) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }

    if (hultra->pins.echoPort == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    } else if (hultra->pins.echoPort == GPIOB) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    } else if (hultra->pins.echoPort == GPIOC) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }

    /* Configure trigger pin as output */
    GPIO_InitStruct.Pin = hultra->pins.triggerPin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(hultra->pins.triggerPort, &GPIO_InitStruct);

    /* Configure echo pin as input with interrupt */
    GPIO_InitStruct.Pin = hultra->pins.echoPin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    /* Set alternate function based on timer */
    if (hultra->htim->Instance == TIM1) {
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
    } else if (hultra->htim->Instance == TIM2) {
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    } else if (hultra->htim->Instance == TIM3) {
        GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    } else if (hultra->htim->Instance == TIM4) {
        GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
    } else if (hultra->htim->Instance == TIM5) {
        GPIO_InitStruct.Alternate = GPIO_AF2_TIM5;
    }

    HAL_GPIO_Init(hultra->pins.echoPort, &GPIO_InitStruct);
}

/**
 * @brief   Deinitialize MSP
 * @details Resets GPIO pins to default state
 * @param   hultra Pointer to ultrasonic sensor handle
 * @retval  None
 */
static void ULTRASONIC_MspDeInit(ULTRASONIC_Handle_t *hultra)
{
    /* Reset pins to input mode */
    HAL_GPIO_DeInit(hultra->pins.triggerPort, hultra->pins.triggerPin);
    HAL_GPIO_DeInit(hultra->pins.echoPort, hultra->pins.echoPin);
}

/**
 * @brief   Validate configuration
 * @details Checks if configuration parameters are valid
 * @param   config Pointer to configuration structure
 * @retval  ULTRASONIC_StatusTypeDef Validation status
 */
static ULTRASONIC_StatusTypeDef ULTRASONIC_ValidateConfig(ULTRASONIC_Config_t *config)
{
    if (config->minDistance >= config->maxDistance) {
        return ULTRASONIC_INVALID_PARAM;
    }

    if (config->temperature > 50) {
        return ULTRASONIC_INVALID_PARAM;
    }

    return ULTRASONIC_OK;
}

/**
 * @brief   Calculate distance from echo time
 * @details Converts echo pulse duration to distance in millimeters
 * @param   echoTime Echo pulse duration in timer ticks
 * @param   temperature Ambient temperature in Celsius
 * @retval  uint16_t Distance in millimeters
 */
static uint16_t ULTRASONIC_CalculateDistance(uint32_t echoTime, uint8_t temperature)
{
    /* Convert timer ticks to microseconds */
    /* Assuming 1MHz timer frequency (1us per tick) */
    uint32_t echoTimeUs = echoTime;

    /* Calculate speed of sound at given temperature */
    float speedOfSound = ULTRASONIC_GetSpeedOfSound(temperature);

    /* Distance = (time * speed) / 2 (round trip) */
    /* Convert to mm: (time_us * speed_m/s * 1000) / (2 * 1000000) */
    float distance = (echoTimeUs * speedOfSound * 1000.0f) / (2.0f * 1000000.0f);

    return (uint16_t)distance;
}

/**
 * @brief   Get speed of sound at given temperature
 * @details Calculates speed of sound in air based on temperature
 * @param   temperature Temperature in Celsius
 * @retval  float Speed of sound in m/s
 */
static float ULTRASONIC_GetSpeedOfSound(uint8_t temperature)
{
    /* Speed of sound = 331.3 + (0.6 * temperature) m/s */
    return 331.3f + (ULTRASONIC_SPEED_CORRECTION * temperature);
}
