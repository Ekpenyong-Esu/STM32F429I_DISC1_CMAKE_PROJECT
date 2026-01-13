/**
  ******************************************************************************
  * @file    stepper.h
  * @brief   Stepper motor driver interface
  * @details This file contains function prototypes and definitions for
  *          stepper motor control using GPIO pins and timer for timing.
  * @version 1.0
  * @date    2025-01-13
  ******************************************************************************
  */

#ifndef __STEPPER_H__
#define __STEPPER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported constants --------------------------------------------------------*/

/** @defgroup STEPPER_Motor_Specifications Motor Specifications
 * @{
 */
#define STEPPER_MAX_SPEED_RPM     1000    /*!< Maximum speed in RPM */
#define STEPPER_MIN_SPEED_RPM     1       /*!< Minimum speed in RPM */
#define STEPPER_DEFAULT_STEPS_PER_REV 200 /*!< Default steps per revolution */
/** @} */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Stepper motor status enumeration
 */
typedef enum {
    STEPPER_OK = 0,              /*!< Operation completed successfully */
    STEPPER_ERROR,               /*!< General error occurred */
    STEPPER_BUSY,                /*!< Motor is busy */
    STEPPER_INVALID_PARAM,       /*!< Invalid parameter provided */
    STEPPER_NOT_INITIALIZED,     /*!< Motor not initialized */
    STEPPER_TIMEOUT,             /*!< Operation timed out */
    STEPPER_LIMIT_REACHED        /*!< Movement limit reached */
} STEPPER_StatusTypeDef;

/**
 * @brief Stepper motor direction enumeration
 */
typedef enum {
    STEPPER_DIR_CW = 0,          /*!< Clockwise direction */
    STEPPER_DIR_CCW              /*!< Counter-clockwise direction */
} STEPPER_Direction_t;

/**
 * @brief Stepper motor step mode enumeration
 */
typedef enum {
    STEPPER_MODE_FULL_STEP = 0,  /*!< Full step mode */
    STEPPER_MODE_HALF_STEP,      /*!< Half step mode */
    STEPPER_MODE_WAVE_DRIVE,     /*!< Wave drive mode */
    STEPPER_MODE_MICROSTEP_4,    /*!< 1/4 microstepping */
    STEPPER_MODE_MICROSTEP_8,    /*!< 1/8 microstepping */
    STEPPER_MODE_MICROSTEP_16    /*!< 1/16 microstepping */
} STEPPER_StepMode_t;

/**
 * @brief Stepper motor configuration structure
 */
typedef struct {
    uint16_t stepsPerRevolution;  /*!< Steps per revolution */
    uint16_t maxSpeedRPM;        /*!< Maximum speed in RPM */
    uint16_t acceleration;       /*!< Acceleration in steps/sec² */
    uint16_t deceleration;       /*!< Deceleration in steps/sec² */
    STEPPER_StepMode_t stepMode; /*!< Step mode */
    bool enableLimitSwitches;    /*!< Enable limit switch checking */
} STEPPER_Config_t;

/**
 * @brief Stepper motor GPIO pin configuration
 */
typedef struct {
    GPIO_TypeDef *port1;         /*!< GPIO port for coil 1 */
    uint16_t pin1;               /*!< GPIO pin for coil 1 */
    GPIO_TypeDef *port2;         /*!< GPIO port for coil 2 */
    uint16_t pin2;               /*!< GPIO pin for coil 2 */
    GPIO_TypeDef *port3;         /*!< GPIO port for coil 3 */
    uint16_t pin3;               /*!< GPIO pin for coil 3 */
    GPIO_TypeDef *port4;         /*!< GPIO port for coil 4 */
    uint16_t pin4;               /*!< GPIO pin for coil 4 */
} STEPPER_Pins_t;

/**
 * @brief Stepper motor handle structure
 */
typedef struct {
    TIM_HandleTypeDef *htim;     /*!< Timer handle for timing */
    STEPPER_Pins_t pins;         /*!< GPIO pin configuration */
    STEPPER_Config_t config;     /*!< Motor configuration */
    STEPPER_Direction_t direction; /*!< Current direction */
    uint32_t currentPosition;    /*!< Current position in steps */
    uint32_t targetPosition;     /*!< Target position in steps */
    uint16_t currentSpeed;       /*!< Current speed in RPM */
    bool isRunning;              /*!< Motor running status */
    bool isInitialized;          /*!< Initialization status */
    uint32_t stepDelay;          /*!< Delay between steps in microseconds */
} STEPPER_Handle_t;

/* Exported functions -------------------------------------------------------*/

/** @defgroup STEPPER_Init Initialization and Configuration
 * @{
 */

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
                                  STEPPER_Pins_t *pins);

/**
 * @brief   Deinitialize stepper motor
 * @details Stops motor and releases resources
 * @param   hstep Pointer to stepper motor handle
 * @retval  STEPPER_StatusTypeDef Operation status
 */
STEPPER_StatusTypeDef STEPPER_DeInit(STEPPER_Handle_t *hstep);

/**
 * @brief   Configure stepper motor parameters
 * @details Sets motor configuration parameters
 * @param   hstep Pointer to stepper motor handle
 * @param   config Pointer to configuration structure
 * @retval  STEPPER_StatusTypeDef Operation status
 */
STEPPER_StatusTypeDef STEPPER_Config(STEPPER_Handle_t *hstep, STEPPER_Config_t *config);

/** @} */

/** @defgroup STEPPER_Motion_Control Motion Control
 * @{
 */

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
                                       uint16_t speed);

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
                                            uint16_t speed);

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
                                              uint16_t speed);

/**
 * @brief   Stop stepper motor
 * @details Stops motor movement immediately
 * @param   hstep Pointer to stepper motor handle
 * @retval  STEPPER_StatusTypeDef Operation status
 */
STEPPER_StatusTypeDef STEPPER_Stop(STEPPER_Handle_t *hstep);

/**
 * @brief   Emergency stop stepper motor
 * @details Stops motor immediately without deceleration
 * @param   hstep Pointer to stepper motor handle
 * @retval  STEPPER_StatusTypeDef Operation status
 */
STEPPER_StatusTypeDef STEPPER_EmergencyStop(STEPPER_Handle_t *hstep);

/** @} */

/** @defgroup STEPPER_Status Status and Information
 * @{
 */

/**
 * @brief   Check if stepper motor is running
 * @details Returns motor running status
 * @param   hstep Pointer to stepper motor handle
 * @retval  bool True if motor is running
 */
bool STEPPER_IsRunning(STEPPER_Handle_t *hstep);

/**
 * @brief   Get current position
 * @details Returns current motor position in steps
 * @param   hstep Pointer to stepper motor handle
 * @retval  uint32_t Current position
 */
uint32_t STEPPER_GetPosition(STEPPER_Handle_t *hstep);

/**
 * @brief   Set current position
 * @details Sets current motor position (for homing/calibration)
 * @param   hstep Pointer to stepper motor handle
 * @param   position New position value
 * @retval  STEPPER_StatusTypeDef Operation status
 */
STEPPER_StatusTypeDef STEPPER_SetPosition(STEPPER_Handle_t *hstep, uint32_t position);

/**
 * @brief   Get motor status
 * @details Returns detailed motor status information
 * @param   hstep Pointer to stepper motor handle
 * @retval  STEPPER_StatusTypeDef Motor status
 */
STEPPER_StatusTypeDef STEPPER_GetStatus(STEPPER_Handle_t *hstep);

/** @} */

/** @defgroup STEPPER_Advanced Advanced Features
 * @{
 */

/**
 * @brief   Enable/disable limit switches
 * @details Configures limit switch functionality
 * @param   hstep Pointer to stepper motor handle
 * @param   enable Enable/disable limit switches
 * @retval  STEPPER_StatusTypeDef Operation status
 */
STEPPER_StatusTypeDef STEPPER_EnableLimitSwitches(STEPPER_Handle_t *hstep, bool enable);

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
                                  uint16_t speed);

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
                                             uint16_t deceleration);

/** @} */

/** @defgroup STEPPER_Utility Utility Functions
 * @{
 */

/**
 * @brief   Convert RPM to step delay
 * @details Calculates delay between steps for given RPM
 * @param   rpm Speed in RPM
 * @param   stepsPerRev Steps per revolution
 * @retval  uint32_t Delay in microseconds
 */
uint32_t STEPPER_RPMToDelay(uint16_t rpm, uint16_t stepsPerRev);

/**
 * @brief   Convert step delay to RPM
 * @details Calculates RPM for given step delay
 * @param   delay Delay between steps in microseconds
 * @param   stepsPerRev Steps per revolution
 * @retval  uint16_t Speed in RPM
 */
uint16_t STEPPER_DelayToRPM(uint32_t delay, uint16_t stepsPerRev);

/**
 * @brief   Get default configuration
 * @details Returns a default configuration structure
 * @param   None
 * @retval  STEPPER_Config_t Default configuration
 */
STEPPER_Config_t STEPPER_GetDefaultConfig(void);

/**
 * @brief   Get default pin configuration
 * @details Returns a default pin configuration for common stepper motors
 * @param   None
 * @retval  STEPPER_Pins_t Default pin configuration
 */
STEPPER_Pins_t STEPPER_GetDefaultPins(void);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __STEPPER_H__ */
