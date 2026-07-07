/**
  ******************************************************************************
  * @file    servo.h
  * @brief   Servo motor driver interface
  * @details This file contains function prototypes and definitions for
  *          servo motor control using PWM signals from timer peripherals.
  * @version 1.0
  * @date    2025-01-13
  ******************************************************************************
  */

#ifndef __SERVO_H__
#define __SERVO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported constants --------------------------------------------------------*/

/** @defgroup SERVO_Motor_Specifications Motor Specifications
 * @{
 */
#define SERVO_MIN_ANGLE              0       /*!< Minimum angle in degrees */
#define SERVO_MAX_ANGLE              180     /*!< Maximum angle in degrees */
#define SERVO_DEFAULT_ANGLE          90      /*!< Default angle in degrees */

#define SERVO_MIN_PULSE_WIDTH_US     500     /*!< Minimum pulse width in microseconds */
#define SERVO_MAX_PULSE_WIDTH_US     2500    /*!< Maximum pulse width in microseconds */
#define SERVO_DEFAULT_PULSE_WIDTH_US 1500    /*!< Default pulse width in microseconds */

#define SERVO_PWM_FREQUENCY_HZ       50      /*!< Standard servo PWM frequency (20ms period) */
#define SERVO_PWM_PERIOD_US          20000   /*!< PWM period in microseconds (20ms) */

/** @} */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Servo motor status enumeration
 */
typedef enum {
    SERVO_OK = 0,              /*!< Operation completed successfully */
    SERVO_ERROR,               /*!< General error occurred */
    SERVO_BUSY,                /*!< Servo is busy */
    SERVO_TIMEOUT,             /*!< Operation timed out */
    SERVO_INVALID_PARAM,       /*!< Invalid parameter provided */
    SERVO_NOT_INITIALIZED,     /*!< Servo not initialized */
    SERVO_OUT_OF_RANGE         /*!< Angle out of valid range */
} SERVO_StatusTypeDef;

/**
 * @brief Servo motor configuration structure
 */
typedef struct {
    uint16_t minAngle;         /*!< Minimum angle in degrees */
    uint16_t maxAngle;         /*!< Maximum angle in degrees */
    uint16_t minPulseWidth;    /*!< Minimum pulse width in microseconds */
    uint16_t maxPulseWidth;    /*!< Maximum pulse width in microseconds */
    uint16_t defaultAngle;     /*!< Default angle in degrees */
} SERVO_Config_t;

/**
 * @brief Servo motor handle structure
 */
typedef struct {
    TIM_HandleTypeDef *htim;   /*!< Timer handle for PWM generation */
    uint32_t channel;          /*!< Timer PWM channel */
    GPIO_TypeDef *gpioPort;    /*!< GPIO port for PWM pin */
    uint16_t gpioPin;          /*!< GPIO pin for PWM signal */
    SERVO_Config_t config;     /*!< Servo configuration */
    uint16_t currentAngle;     /*!< Current angle in degrees */
    bool isInitialized;        /*!< Initialization status */
} SERVO_Handle_t;

/* Exported functions -------------------------------------------------------*/

/** @defgroup SERVO_Init Initialization and Configuration
 * @{
 */

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
                              uint16_t gpioPin);

/**
 * @brief   Deinitialize servo motor
 * @details Stops PWM and releases resources
 * @param   hservo Pointer to servo motor handle
 * @retval  SERVO_StatusTypeDef Operation status
 */
SERVO_StatusTypeDef SERVO_DeInit(SERVO_Handle_t *hservo);

/**
 * @brief   Configure servo motor parameters
 * @details Sets servo configuration parameters
 * @param   hservo Pointer to servo motor handle
 * @param   config Pointer to configuration structure
 * @retval  SERVO_StatusTypeDef Operation status
 */
SERVO_StatusTypeDef SERVO_Config(SERVO_Handle_t *hservo, SERVO_Config_t *config);

/** @} */

/** @defgroup SERVO_Control Servo Control
 * @{
 */

/**
 * @brief   Set servo angle
 * @details Moves servo to specified angle
 * @param   hservo Pointer to servo motor handle
 * @param   angle Angle in degrees (0-180)
 * @retval  SERVO_StatusTypeDef Operation status
 */
SERVO_StatusTypeDef SERVO_SetAngle(SERVO_Handle_t *hservo, uint16_t angle);

/**
 * @brief   Set servo pulse width
 * @details Sets PWM pulse width directly
 * @param   hservo Pointer to servo motor handle
 * @param   pulseWidth Pulse width in microseconds
 * @retval  SERVO_StatusTypeDef Operation status
 */
SERVO_StatusTypeDef SERVO_SetPulseWidth(SERVO_Handle_t *hservo, uint16_t pulseWidth);

/**
 * @brief   Get current servo angle
 * @details Returns current servo angle
 * @param   hservo Pointer to servo motor handle
 * @retval  uint16_t Current angle in degrees
 */
uint16_t SERVO_GetAngle(SERVO_Handle_t *hservo);

/**
 * @brief   Move servo to minimum angle
 * @details Moves servo to configured minimum angle
 * @param   hservo Pointer to servo motor handle
 * @retval  SERVO_StatusTypeDef Operation status
 */
SERVO_StatusTypeDef SERVO_MoveToMin(SERVO_Handle_t *hservo);

/**
 * @brief   Move servo to maximum angle
 * @details Moves servo to configured maximum angle
 * @param   hservo Pointer to servo motor handle
 * @retval  SERVO_StatusTypeDef Operation status
 */
SERVO_StatusTypeDef SERVO_MoveToMax(SERVO_Handle_t *hservo);

/**
 * @brief   Move servo to center position
 * @details Moves servo to 90 degrees (center)
 * @param   hservo Pointer to servo motor handle
 * @retval  SERVO_StatusTypeDef Operation status
 */
SERVO_StatusTypeDef SERVO_MoveToCenter(SERVO_Handle_t *hservo);

/**
 * @brief   Sweep servo across range
 * @details Moves servo from min to max angle and back
 * @param   hservo Pointer to servo motor handle
 * @param   speed Delay between angle steps in milliseconds
 * @retval  SERVO_StatusTypeDef Operation status
 */
SERVO_StatusTypeDef SERVO_Sweep(SERVO_Handle_t *hservo, uint16_t speed);

/** @} */

/** @defgroup SERVO_Calibration Servo Calibration
 * @{
 */

/**
 * @brief   Calibrate servo minimum angle
 * @details Sets current position as minimum angle
 * @param   hservo Pointer to servo motor handle
 * @param   angle Actual angle in degrees
 * @retval  SERVO_StatusTypeDef Operation status
 */
SERVO_StatusTypeDef SERVO_CalibrateMin(SERVO_Handle_t *hservo, uint16_t angle);

/**
 * @brief   Calibrate servo maximum angle
 * @details Sets current position as maximum angle
 * @param   hservo Pointer to servo motor handle
 * @param   angle Actual angle in degrees
 * @retval  SERVO_StatusTypeDef Operation status
 */
SERVO_StatusTypeDef SERVO_CalibrateMax(SERVO_Handle_t *hservo, uint16_t angle);

/**
 * @brief   Reset servo calibration
 * @details Resets calibration to default values
 * @param   hservo Pointer to servo motor handle
 * @retval  SERVO_StatusTypeDef Operation status
 */
SERVO_StatusTypeDef SERVO_ResetCalibration(SERVO_Handle_t *hservo);

/** @} */

/** @defgroup SERVO_Utility Utility Functions
 * @{
 */

/**
 * @brief   Convert angle to pulse width
 * @details Calculates PWM pulse width for given angle
 * @param   angle Angle in degrees
 * @param   config Pointer to servo configuration
 * @retval  uint16_t Pulse width in microseconds
 */
uint16_t SERVO_AngleToPulseWidth(uint16_t angle, SERVO_Config_t *config);

/**
 * @brief   Convert pulse width to angle
 * @details Calculates angle for given PWM pulse width
 * @param   pulseWidth Pulse width in microseconds
 * @param   config Pointer to servo configuration
 * @retval  uint16_t Angle in degrees
 */
uint16_t SERVO_PulseWidthToAngle(uint16_t pulseWidth, SERVO_Config_t *config);

/**
 * @brief   Get default configuration
 * @details Returns a default configuration structure
 * @param   None
 * @retval  SERVO_Config_t Default configuration
 */
SERVO_Config_t SERVO_GetDefaultConfig(void);

/**
 * @brief   Validate servo angle
 * @details Checks if angle is within valid range
 * @param   angle Angle to validate
 * @param   config Pointer to servo configuration
 * @retval  bool True if angle is valid
 */
bool SERVO_IsValidAngle(uint16_t angle, SERVO_Config_t *config);

/**
 * @brief   Validate pulse width
 * @details Checks if pulse width is within valid range
 * @param   pulseWidth Pulse width to validate
 * @param   config Pointer to servo configuration
 * @retval  bool True if pulse width is valid
 */
bool SERVO_IsValidPulseWidth(uint16_t pulseWidth, SERVO_Config_t *config);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __SERVO_H__ */
