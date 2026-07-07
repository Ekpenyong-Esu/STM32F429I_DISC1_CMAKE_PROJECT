#include "SEGGER_SYSVIEW.h"
#include "SEGGER_RTT.h"
#include "SEGGER_SYSVIEW_Conf.h"
#include "stm32f4xx.h"

static void cbSendSystemDesc(void) {
  SEGGER_SYSVIEW_SendSysDesc("N=NoRTOS App,D=Cortex-M4,O=None");
  SEGGER_SYSVIEW_SendSysDesc("I#15=SysTick");
}

U32 SEGGER_SYSVIEW_X_GetTimestamp(void) {
  return DWT->CYCCNT;
}

void SEGGER_SYSVIEW_Conf(void) {
  __disable_irq();
  // Enable DWT cycle counter
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  // Initialize RTT
  SEGGER_RTT_Init();

  // Use SystemCoreClock for both CPU and timestamp frequency
  SEGGER_SYSVIEW_Init(SystemCoreClock, SystemCoreClock, NULL, cbSendSystemDesc);
  // STM32F429 SRAM base address
  SEGGER_SYSVIEW_SetRAMBase(0x20000000u);

  __enable_irq();
}
