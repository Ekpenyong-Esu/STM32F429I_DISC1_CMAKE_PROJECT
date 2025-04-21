/**
  ******************************************************************************
  * @file    rtos.c
  * @brief   RTOS module implementation
  * @details This file provides code for the configuration
  *          and initialization of the RTOS kernel and tasks.
  *          It implements task creation and system timing.
  * @version 1.0
  * @date    2025-04-15
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "rtos.h"
#include "usb_host.h"

/* Private variables ---------------------------------------------------------*/
/**
 * @brief   Handle for the default task
 * @details Used to control and reference the default task
 */
osThreadId_t defaultTaskHandle;

/**
 * @brief   Default task configuration attributes
 * @details Defines the task name, stack size, and priority
 * @note    Stack size is set to 16 KB (4096 * 4 bytes) which is
 *          adequate for most general-purpose tasks
 */
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",         /* Task name for debugging and analysis */
  .stack_size = 4096 * 4,        /* 16 KB stack allocation */
  .priority = (osPriority_t) osPriorityNormal, /* Normal priority level */
};

/**
  * @brief  RTOS Initialization Function
  * @details Performs initialization of the RTOS kernel and creates
  *          the default system tasks. The sequence is:
  *          1. Initialize the OS kernel
  *          2. Create the default task
  *          3. Set up any additional RTOS resources
  *
  * @note   This should be called before any peripheral initialization
  *         that requires RTOS services
  * @param  None
  * @retval None
  */
void RTOS_Init(void)
{
  /* Init scheduler - prepare the RTOS kernel */
  osKernelInitialize();

  /* Create the thread(s) */
  /* Creation of defaultTask - the main application task */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* Add additional RTOS resources here (mutexes, semaphores, queues, etc.) */
}

/**
  * @brief  RTOS Start Function
  * @details Starts the RTOS kernel scheduler, which begins
  *          task execution and time management.
  *
  * @note   This function transfers control to the RTOS kernel and
  *         should never return in normal operation. It should be
  *         called after all initialization is complete.
  * @param  None
  * @retval None
  */
void RTOS_Start(void)
{
  /* Start scheduler - begin task execution */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* If we do reach here, it indicates a system failure */
}

/**
  * @brief  Function implementing the defaultTask thread
  * @details This is the entry function for the default task. It:
  *          1. Initializes the USB Host interface
  *          2. Enters an infinite loop for continuous operation
  *          3. Includes a delay to prevent CPU hogging
  *
  * @note   The default task is typically used for:
  *         - Non-time-critical operations
  *         - Background processing
  *         - System maintenance functions
  *
  * @param  argument: Task input argument pointer (unused in this implementation)
  * @retval None
  */
void StartDefaultTask(void *argument)
{
  /* Initialize code for USB Host */
  MX_USB_HOST_Init();

  /* Infinite loop - the task body that runs repeatedly */
  for(;;)
  {
    /* Small delay to prevent task from consuming all CPU time */
    osDelay(1);  /* Yield for 1 tick (typically 1ms) */
  }
}

/**
  * @brief  Period elapsed callback in non-blocking mode
  * @details This function is automatically called by the HAL when
  *          TIM6 timer interrupt occurs. It updates the system tick
  *          counter used by the RTOS for timing and task switching.
  *
  * @note   This function is critical for RTOS timing. TIM6 is configured
  *         as the time base source for the RTOS kernel.
  * @param  htim : TIM handle pointer to identify which timer triggered the callback
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM6)  /* Verify that TIM6 triggered this callback */
  {
    HAL_IncTick();  /* Increment the HAL tick counter used by the RTOS */
  }
}
