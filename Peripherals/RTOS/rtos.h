/**
  ******************************************************************************
  * @file    rtos.h
  * @brief   RTOS module interface
  * @details This file contains all the function prototypes for
  *          Real-Time Operating System (RTOS) configuration and use.
  *          It provides APIs to initialize and control RTOS tasks
  *          on the STM32F429 board.
  * @version 1.0
  * @date    2025-04-15
  ******************************************************************************
  */

#ifndef __RTOS_H__
#define __RTOS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Exported functions prototypes ---------------------------------------------*/
/**
 * @brief   Initializes RTOS components before kernel start
 * @details Sets up task handles, semaphores, queues, and other RTOS objects
 * @param   None
 * @retval  None
 */
void RTOS_Init(void);

/**
 * @brief   Starts the RTOS kernel
 * @details Begins task scheduling and transfers control to the RTOS kernel
 * @note    This function should never return in normal operation
 * @param   None
 * @retval  None
 */
void RTOS_Start(void);

/**
 * @brief   Default task function
 * @details Entry function for the default task created by the system
 * @param   argument  Pointer to task arguments (if provided)
 * @retval  None
 */
void StartDefaultTask(void *argument);

/* Exported variables ---------------------------------------------------------*/
/**
 * @brief   Handle for the default task
 * @details Used to reference the default task for control operations
 */
extern osThreadId_t defaultTaskHandle;

/**
 * @brief   Default task attributes
 * @details Contains configuration settings for the default task
 *          including stack size, priority, and name
 */
extern const osThreadAttr_t defaultTask_attributes;

#ifdef __cplusplus
}
#endif

#endif /* __RTOS_H__ */
