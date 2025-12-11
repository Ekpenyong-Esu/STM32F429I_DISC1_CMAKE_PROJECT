/**
  ******************************************************************************
  * @file    pwr_example.c
  * @brief   Power Management usage examples
  * @details This file provides example code demonstrating how to use
  *          the PWR driver for power management and low-power modes.
  * @version 1.0
  * @date    2025-12-11
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "pwr.h"
#include <stdio.h>

/* Example 1: Basic Sleep mode */
void PWR_Example_SleepMode(void)
{
    printf("=== PWR Sleep Mode Example ===\r\n\r\n");

    printf("Entering Sleep mode for 2 seconds...\r\n");
    printf("(CPU sleeps, peripherals active, SysTick wakes)\r\n");

    /* Sleep using SysTick interrupt to wake */
    PWR_SleepFor(2000);

    printf("Woke up from Sleep mode!\r\n\r\n");
}

/* Example 2: Stop mode with wakeup */
void PWR_Example_StopMode(void)
{
    printf("=== PWR Stop Mode Example ===\r\n\r\n");

    printf("Stop mode is the lowest power mode that retains RAM.\r\n");
    printf("Wake sources: EXTI interrupt, RTC alarm\r\n\r\n");

    printf("NOTE: This example is demonstration code.\r\n");
    printf("To actually enter Stop mode, uncomment the code.\r\n\r\n");

#if 0  /* Uncomment to enable Stop mode */
    printf("Entering Stop mode (low-power regulator)...\r\n");
    printf("Press a button or wait for RTC alarm to wake.\r\n");

    /* Enter Stop mode with low-power regulator */
    PWR_EnterStopMode(PWR_REGULATOR_LOW_POWER, PWR_STOP_ENTRY_WFI);

    /* After wakeup, reconfigure clocks */
    printf("Reconfiguring clocks after Stop mode...\r\n");
    PWR_ConfigureAfterStop();

    printf("Woke up from Stop mode!\r\n");
#endif

    printf("Stop mode demonstration complete.\r\n");
}

/* Example 3: Standby mode */
void PWR_Example_StandbyMode(void)
{
    printf("=== PWR Standby Mode Example ===\r\n\r\n");

    printf("Standby is the lowest power mode (~2.5uA).\r\n");
    printf("RAM is NOT retained. Only backup registers survive.\r\n");
    printf("Wake sources: WKUP pin (PA0), RTC alarm, reset\r\n\r\n");

    /* Check if this was a standby wakeup */
    if (PWR_WasStandbyWakeup())
    {
        printf("*** This boot is from STANDBY wakeup! ***\r\n");
        PWR_ClearStandbyFlag();

        /* Read saved data from backup register */
        uint32_t savedValue = 0;
        PWR_ReadBackupRegister(0, &savedValue);
        printf("Recovered value from backup: 0x%08lX\r\n", savedValue);
    }
    else
    {
        printf("Normal boot (not from Standby)\r\n");
    }

    printf("\r\nNOTE: This example is demonstration code.\r\n");
    printf("To actually enter Standby mode, uncomment the code.\r\n\r\n");

#if 0  /* Uncomment to enable Standby mode */
    /* Save important data to backup register before standby */
    printf("Saving data to backup register...\r\n");
    PWR_WriteBackupRegister(0, 0xDEADBEEF);

    /* Enable wakeup pin */
    printf("Enabling wakeup pin (PA0)...\r\n");
    PWR_EnableWakeupPin(true);

    printf("Entering Standby mode...\r\n");
    printf("Press WKUP button (PA0) to wake.\r\n");

    /* Enter Standby - this won't return! System resets on wake */
    PWR_EnterStandbyMode();

    /* Never reached */
    printf("ERROR: Should not reach here!\r\n");
#endif

    printf("Standby mode demonstration complete.\r\n");
}

/* Example 4: PVD (Power Voltage Detector) */
void PWR_Example_PVD(void)
{
    printf("=== PWR PVD (Voltage Detector) Example ===\r\n\r\n");

    printf("PVD monitors VDD and can trigger interrupt when low.\r\n");
    printf("Useful for:\r\n");
    printf("  - Detecting brownout conditions\r\n");
    printf("  - Saving data before power loss\r\n");
    printf("  - Triggering safe shutdown\r\n\r\n");

    /* Enable PVD at 2.9V threshold */
    printf("Enabling PVD with 2.9V threshold...\r\n");
    PWR_EnablePVD(PWR_PVD_LEVEL_2V9);

    /* Check current status */
    bool powerLow = PWR_GetPVDStatus();
    if (powerLow)
    {
        printf("WARNING: VDD is below threshold!\r\n");
    }
    else
    {
        printf("VDD is above threshold (normal)\r\n");
    }

    /* To use interrupt mode, implement PVD_IRQHandler:
     * void PVD_IRQHandler(void)
     * {
     *     HAL_PWR_PVD_IRQHandler();
     * }
     *
     * void HAL_PWR_PVDCallback(void)
     * {
     *     // Save critical data
     *     // Trigger safe shutdown
     * }
     */

    printf("\r\nPVD demonstration complete.\r\n");
}

/* Example 5: Backup registers */
void PWR_Example_BackupRegisters(void)
{
    printf("=== PWR Backup Registers Example ===\r\n\r\n");

    printf("Backup registers retain data during:\r\n");
    printf("  - Sleep mode\r\n");
    printf("  - Stop mode\r\n");
    printf("  - Standby mode\r\n");
    printf("  - VDD power loss (if VBAT is connected)\r\n\r\n");

    printf("STM32F429 has 20 backup registers (32-bit each).\r\n\r\n");

    /* Enable backup access */
    PWR_EnableBackupAccess();

    /* Write test values */
    printf("Writing test values to backup registers...\r\n");
    PWR_WriteBackupRegister(0, 0x12345678);
    PWR_WriteBackupRegister(1, 0xDEADBEEF);
    PWR_WriteBackupRegister(2, 0xCAFEBABE);

    /* Read back */
    printf("Reading values back:\r\n");
    uint32_t values[3];
    PWR_ReadBackupRegister(0, &values[0]);
    PWR_ReadBackupRegister(1, &values[1]);
    PWR_ReadBackupRegister(2, &values[2]);

    printf("  BKP0: 0x%08lX (expected 0x12345678)\r\n", values[0]);
    printf("  BKP1: 0x%08lX (expected 0xDEADBEEF)\r\n", values[1]);
    printf("  BKP2: 0x%08lX (expected 0xCAFEBABE)\r\n", values[2]);

    /* Verify */
    if (values[0] == 0x12345678 && values[1] == 0xDEADBEEF && values[2] == 0xCAFEBABE)
    {
        printf("\r\n✓ Backup register test PASSED\r\n");
    }
    else
    {
        printf("\r\n✗ Backup register test FAILED\r\n");
    }
}

/* Example 6: Power modes comparison */
void PWR_Example_PowerModes(void)
{
    printf("=== PWR Power Modes Comparison ===\r\n\r\n");

    printf("Mode        | Current   | Wake Time | RAM  | Wake Sources\r\n");
    printf("------------|-----------|-----------|------|--------------------\r\n");
    printf("Run         | ~100 mA   | N/A       | Yes  | N/A\r\n");
    printf("Sleep       | ~10 mA    | <1 us     | Yes  | Any interrupt\r\n");
    printf("Stop        | ~100 uA   | ~4 us     | Yes  | EXTI, RTC\r\n");
    printf("Standby     | ~2.5 uA   | ~50 us    | No*  | WKUP, RTC, Reset\r\n");
    printf("\r\n");
    printf("* Backup registers retained if VBAT connected\r\n\r\n");

    printf("Current estimated power consumption:\r\n");
    uint32_t current = PWR_GetEstimatedCurrent();
    printf("  ~%lu uA (~%.1f mA)\r\n", current, (float)current / 1000.0f);
}

/* Example 7: Voltage scaling */
void PWR_Example_VoltageScaling(void)
{
    printf("=== PWR Voltage Scaling Example ===\r\n\r\n");

    printf("STM32F429 voltage scaling modes:\r\n");
    printf("  Scale 1: High performance (up to 180 MHz)\r\n");
    printf("  Scale 2: Medium performance (up to 168 MHz)\r\n");
    printf("  Scale 3: Low power (up to 120 MHz)\r\n\r\n");

    printf("Current mode: Scale 1 (default)\r\n\r\n");

    printf("NOTE: Changing voltage scale requires reducing clock first.\r\n");
    printf("This example demonstrates API usage only.\r\n\r\n");

#if 0  /* Uncomment to test - requires clock reconfiguration */
    printf("Switching to low-power mode...\r\n");
    PWR_EnableLowPowerMode();

    printf("Switching back to high-performance mode...\r\n");
    PWR_EnableHighPerformance();
#endif

    printf("Voltage scaling demonstration complete.\r\n");
}

/**
 * @brief   Run all PWR examples
 */
void PWR_RunExamples(void)
{
    printf("\r\n========================================\r\n");
    printf("      Power Management Examples\r\n");
    printf("========================================\r\n\r\n");

    /* Initialize PWR with default config */
    PWR_InitDefault();

    PWR_Example_PowerModes();
    printf("\r\n");

    PWR_Example_BackupRegisters();
    printf("\r\n");

    PWR_Example_PVD();
    printf("\r\n");

    PWR_Example_SleepMode();
    printf("\r\n");

    PWR_Example_VoltageScaling();
    printf("\r\n");

    PWR_Example_StopMode();
    printf("\r\n");

    PWR_Example_StandbyMode();

    printf("\r\n========================================\r\n");
    printf("      All PWR Examples Complete\r\n");
    printf("========================================\r\n");
}
