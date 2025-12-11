/**
 ******************************************************************************
 * @file    rtc_example.c
 * @author  Mahonri
 * @brief   RTC example implementation for STM32F429I Discovery board
 *          This file contains example functions demonstrating RTC usage
 ******************************************************************************
 * @attention
 *
 * This software is provided as-is, without any express or implied warranties.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "rtc_example.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void PrintStatus(RTC_StatusTypeDef status);

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  RTC basic example - Initialize and display current time
 * @retval None
 */
void RTC_BasicExample(void)
{
    RTC_StatusTypeDef status;
    RTC_Time_t sTime;
    RTC_Date_t sDate;
    char timeStr[20], dateStr[30];

    printf("=== RTC Basic Example ===\n");

    /* Initialize RTC */
    status = RTC_Init();
    printf("RTC Initialization: ");
    PrintStatus(status);

    if (status != RTC_STATUS_OK)
    {
        printf("Failed to initialize RTC\n");
        return;
    }

    /* Get current time and date */
    status = RTC_GetTime(&sTime);
    if (status == RTC_STATUS_OK)
    {
        status = RTC_GetDate(&sDate);
    }

    if (status == RTC_STATUS_OK)
    {
        /* Format and display time and date */
        RTC_FormatTimeString(&sTime, timeStr, sizeof(timeStr));
        RTC_FormatDateString(&sDate, dateStr, sizeof(dateStr));

        printf("Current Date: %s\n", dateStr);
        printf("Current Time: %s\n", timeStr);
    }
    else
    {
        printf("Failed to read RTC\n");
    }

    printf("=== End Basic Example ===\n\n");
}

/**
 * @brief  RTC set time example
 * @retval None
 */
void RTC_SetTimeExample(void)
{
    RTC_StatusTypeDef status;
    RTC_Time_t sTime;
    RTC_Date_t sDate;
    char timeStr[20], dateStr[30];

    printf("=== RTC Set Time Example ===\n");

    /* Initialize RTC */
    status = RTC_Init();
    if (status != RTC_STATUS_OK)
    {
        printf("Failed to initialize RTC\n");
        return;
    }

    /* Set time to 14:30:00 (2:30 PM) */
    sTime.Hours = 14;
    sTime.Minutes = 30;
    sTime.Seconds = 0;
    sTime.TimeFormat = RTC_HOURFORMAT_24;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;

    /* Set date to September 4, 2025 (Thursday) */
    sDate.WeekDay = RTC_WEEKDAY_THURSDAY;
    sDate.Month = RTC_MONTH_SEPTEMBER;
    sDate.Date = 4;
    sDate.Year = 25; /* Year 2025 */

    /* Set the date and time */
    status = RTC_SetDate(&sDate);
    printf("Set Date: ");
    PrintStatus(status);

    status = RTC_SetTime(&sTime);
    printf("Set Time: ");
    PrintStatus(status);

    if (status == RTC_STATUS_OK)
    {
        /* Verify by reading back */
        printf("Verifying set values...\n");

        status = RTC_GetTime(&sTime);
        if (status == RTC_STATUS_OK)
        {
            status = RTC_GetDate(&sDate);
        }

        if (status == RTC_STATUS_OK)
        {
            RTC_FormatTimeString(&sTime, timeStr, sizeof(timeStr));
            RTC_FormatDateString(&sDate, dateStr, sizeof(dateStr));

            printf("Read back Date: %s\n", dateStr);
            printf("Read back Time: %s\n", timeStr);
        }
    }

    printf("=== End Set Time Example ===\n\n");
}

/**
 * @brief  RTC alarm example
 * @retval None
 */
void RTC_AlarmExample(void)
{
    RTC_StatusTypeDef status;
    RTC_Alarm_t sAlarm;

    printf("=== RTC Alarm Example ===\n");

    /* Initialize RTC */
    status = RTC_Init();
    if (status != RTC_STATUS_OK)
    {
        printf("Failed to initialize RTC\n");
        return;
    }

    /* Configure Alarm A for 14:31:00 (1 minute after the set time) */
    sAlarm.AlarmTime.Hours = 14;
    sAlarm.AlarmTime.Minutes = 31;
    sAlarm.AlarmTime.Seconds = 0;
    sAlarm.AlarmTime.TimeFormat = RTC_HOURFORMAT_24;
    sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY;
    sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
    sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
    sAlarm.AlarmDateWeekDay = 4; /* 4th of the month */
    sAlarm.Alarm = RTC_ALARM_A;

    /* Set the alarm */
    status = RTC_SetAlarm(&sAlarm);
    printf("Set Alarm A: ");
    PrintStatus(status);

    if (status == RTC_STATUS_OK)
    {
        printf("Alarm A set for 14:31:00 on the 4th\n");
        printf("Alarm will trigger when time matches!\n");
        printf("Note: Implement RTC_AlarmCallback() to handle alarm events\n");
    }

    printf("=== End Alarm Example ===\n\n");
}

/**
 * @brief  RTC timestamp example
 * @retval None
 */
void RTC_TimestampExample(void)
{
    RTC_StatusTypeDef status;
    uint32_t timestamp;
    uint32_t testTimestamp = 1725465600; /* Sept 4, 2024 14:00:00 UTC */

    printf("=== RTC Timestamp Example ===\n");

    /* Initialize RTC */
    status = RTC_Init();
    if (status != RTC_STATUS_OK)
    {
        printf("Failed to initialize RTC\n");
        return;
    }

    /* Set using timestamp */
    printf("Setting timestamp: %"PRIu32" (Sept 4, 2024 14:00:00 UTC)\n", testTimestamp);
    status = RTC_SetTimestamp(testTimestamp);
    printf("Set Timestamp: ");
    PrintStatus(status);

    if (status == RTC_STATUS_OK)
    {
        /* Read back timestamp */
        timestamp = RTC_GetTimestamp();
        printf("Read back timestamp: %"PRIu32"\n", timestamp);
        printf("Difference: %"PRId32" seconds\n", (int32_t)(timestamp - testTimestamp));
    }    printf("=== End Timestamp Example ===\n\n");
}

/**
 * @brief  RTC format example - Display formatted date and time
 * @retval None
 */
void RTC_FormatExample(void)
{
    RTC_StatusTypeDef status;
    RTC_Time_t sTime;
    RTC_Date_t sDate;
    char timeStr[30], dateStr[50];

    printf("=== RTC Format Example ===\n");

    /* Initialize RTC */
    status = RTC_Init();
    if (status != RTC_STATUS_OK)
    {
        printf("Failed to initialize RTC\n");
        return;
    }

    /* Set specific time for demonstration */
    sTime.Hours = 15;
    sTime.Minutes = 45;
    sTime.Seconds = 30;
    sTime.TimeFormat = RTC_HOURFORMAT_24;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;

    sDate.WeekDay = RTC_WEEKDAY_WEDNESDAY;
    sDate.Month = RTC_MONTH_SEPTEMBER;
    sDate.Date = 4;
    sDate.Year = 25;

    RTC_SetDate(&sDate);
    RTC_SetTime(&sTime);

    /* Demonstrate different formatting */
    printf("24-hour format:\n");
    RTC_FormatTimeString(&sTime, timeStr, sizeof(timeStr));
    RTC_FormatDateString(&sDate, dateStr, sizeof(dateStr));
    printf("  Date: %s\n", dateStr);
    printf("  Time: %s\n", timeStr);

    /* Change to 12-hour format for demonstration */
    sTime.TimeFormat = RTC_HOURFORMAT12_PM;
    sTime.Hours = 3; /* 3 PM */

    printf("\n12-hour format:\n");
    RTC_FormatTimeString(&sTime, timeStr, sizeof(timeStr));
    printf("  Time: %s\n", timeStr);

    printf("=== End Format Example ===\n\n");
}

/**
 * @brief  RTC complete example - Demonstrates all features
 * @retval None
 */
void RTC_CompleteExample(void)
{
    printf("=== RTC Complete Example ===\n");
    printf("Running all RTC examples...\n\n");

    /* Run all examples */
    RTC_BasicExample();
    RTC_SetTimeExample();
    RTC_AlarmExample();
    RTC_TimestampExample();
    RTC_FormatExample();

    printf("=== All Examples Complete ===\n");
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Print RTC status
 * @param  status: RTC status to print
 * @retval None
 */
static void PrintStatus(RTC_StatusTypeDef status)
{
    switch (status)
    {
        case RTC_STATUS_OK:
            printf("OK\n");
            break;
        case RTC_STATUS_ERROR:
            printf("ERROR\n");
            break;
        case RTC_STATUS_BUSY:
            printf("BUSY\n");
            break;
        case RTC_STATUS_TIMEOUT:
            printf("TIMEOUT\n");
            break;
        default:
            printf("UNKNOWN\n");
            break;
    }
}

/**
 * @brief  RTC Alarm callback implementation
 * @note   This is a user implementation of the alarm callback
 * @param  Alarm: Specifies the Alarm. Can be RTC_ALARM_A or RTC_ALARM_B
 * @retval None
 */
void RTC_AlarmCallback(uint32_t Alarm)
{
    if (Alarm == RTC_ALARM_A)
    {
        printf("*** ALARM A TRIGGERED! ***\n");
    }
    else if (Alarm == RTC_ALARM_B)
    {
        printf("*** ALARM B TRIGGERED! ***\n");
    }
}
