/**
 ******************************************************************************
 * @file    rtc.c
 * @author  Mahonri
 * @brief   RTC peripheral driver implementation for STM32F429I Discovery board
 *          This file provides the implementation of RTC functions
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
#include "rtc.h"
#include <stdio.h>
#include <string.h>
#include "log.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define RTC_TIMEOUT_VALUE           1000U

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static RTC_HandleTypeDef hrtc;

/* Private function prototypes -----------------------------------------------*/
static void RTC_MspInit(void);
static void RTC_MspDeInit(void);
static uint32_t RTC_DateTimeToTimestamp(RTC_Date_t* sDate, RTC_Time_t* sTime);
static void RTC_TimestampToDateTime(uint32_t timestamp, RTC_Date_t* sDate, RTC_Time_t* sTime);

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  Initialize the RTC peripheral
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_Init(void)
{
    log_debug("RTC: Initializing RTC");

    /* Initialize RTC MSP */
    RTC_MspInit();

    /* Configure RTC prescaler and RTC data registers */
    hrtc.Instance = RTC;
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    /* LSI is ~32kHz, so prescaler values adjusted accordingly */
    hrtc.Init.AsynchPrediv = 124;  /* (32000/256) - 1 = 124 */
    hrtc.Init.SynchPrediv = 255;   /* 256 - 1 = 255 */
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

    if (HAL_RTC_Init(&hrtc) != HAL_OK)
    {
        return RTC_STATUS_ERROR;
    }

    log_debug("RTC: RTC initialized successfully");

    return RTC_STATUS_OK;
}

/**
 * @brief  Deinitialize the RTC peripheral
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_DeInit(void)
{
    if (HAL_RTC_DeInit(&hrtc) != HAL_OK)
    {
        return RTC_STATUS_ERROR;
    }

    RTC_MspDeInit();
    return RTC_STATUS_OK;
}

/**
 * @brief  Set the RTC current time
 * @param  sTime: Pointer to Time structure
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_SetTime(RTC_Time_t* sTime)
{
    RTC_TimeTypeDef hal_time;

    if (sTime == NULL)
    {
        return RTC_STATUS_ERROR;
    }

    /* Convert to HAL format */
    hal_time.Hours = sTime->Hours;
    hal_time.Minutes = sTime->Minutes;
    hal_time.Seconds = sTime->Seconds;
    hal_time.TimeFormat = sTime->TimeFormat;
    hal_time.DayLightSaving = sTime->DayLightSaving;
    hal_time.StoreOperation = sTime->StoreOperation;

    if (HAL_RTC_SetTime(&hrtc, &hal_time, RTC_FORMAT_BIN) != HAL_OK)
    {
        return RTC_STATUS_ERROR;
    }

    return RTC_STATUS_OK;
}

/**
 * @brief  Get the RTC current time
 * @param  sTime: Pointer to Time structure
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_GetTime(RTC_Time_t* sTime)
{
    RTC_TimeTypeDef hal_time;

    if (sTime == NULL)
    {
        return RTC_STATUS_ERROR;
    }

    if (HAL_RTC_GetTime(&hrtc, &hal_time, RTC_FORMAT_BIN) != HAL_OK)
    {
        return RTC_STATUS_ERROR;
    }

    /* Convert from HAL format */
    sTime->Hours = hal_time.Hours;
    sTime->Minutes = hal_time.Minutes;
    sTime->Seconds = hal_time.Seconds;
    sTime->TimeFormat = hal_time.TimeFormat;
    sTime->DayLightSaving = hal_time.DayLightSaving;
    sTime->StoreOperation = hal_time.StoreOperation;

    return RTC_STATUS_OK;
}

/**
 * @brief  Set the RTC current date
 * @param  sDate: Pointer to date structure
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_SetDate(RTC_Date_t* sDate)
{
    RTC_DateTypeDef hal_date;

    if (sDate == NULL)
    {
        return RTC_STATUS_ERROR;
    }

    /* Convert to HAL format */
    hal_date.WeekDay = sDate->WeekDay;
    hal_date.Month = sDate->Month;
    hal_date.Date = sDate->Date;
    hal_date.Year = sDate->Year;

    if (HAL_RTC_SetDate(&hrtc, &hal_date, RTC_FORMAT_BIN) != HAL_OK)
    {
        return RTC_STATUS_ERROR;
    }

    return RTC_STATUS_OK;
}

/**
 * @brief  Get the RTC current date
 * @param  sDate: Pointer to Date structure
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_GetDate(RTC_Date_t* sDate)
{
    RTC_DateTypeDef hal_date;

    if (sDate == NULL)
    {
        return RTC_STATUS_ERROR;
    }

    if (HAL_RTC_GetDate(&hrtc, &hal_date, RTC_FORMAT_BIN) != HAL_OK)
    {
        return RTC_STATUS_ERROR;
    }

    /* Convert from HAL format */
    sDate->WeekDay = hal_date.WeekDay;
    sDate->Month = hal_date.Month;
    sDate->Date = hal_date.Date;
    sDate->Year = hal_date.Year;

    return RTC_STATUS_OK;
}

/**
 * @brief  Set the specified RTC Alarm
 * @param  sAlarm: Pointer to Alarm structure
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_SetAlarm(RTC_Alarm_t* sAlarm)
{
    RTC_AlarmTypeDef hal_alarm;

    if (sAlarm == NULL)
    {
        return RTC_STATUS_ERROR;
    }

    /* Convert to HAL format */
    hal_alarm.AlarmTime.Hours = sAlarm->AlarmTime.Hours;
    hal_alarm.AlarmTime.Minutes = sAlarm->AlarmTime.Minutes;
    hal_alarm.AlarmTime.Seconds = sAlarm->AlarmTime.Seconds;
    hal_alarm.AlarmTime.TimeFormat = sAlarm->AlarmTime.TimeFormat;
    hal_alarm.AlarmMask = sAlarm->AlarmMask;
    hal_alarm.AlarmSubSecondMask = sAlarm->AlarmSubSecondMask;
    hal_alarm.AlarmDateWeekDaySel = sAlarm->AlarmDateWeekDaySel;
    hal_alarm.AlarmDateWeekDay = sAlarm->AlarmDateWeekDay;
    hal_alarm.Alarm = sAlarm->Alarm;

    if (HAL_RTC_SetAlarm_IT(&hrtc, &hal_alarm, RTC_FORMAT_BIN) != HAL_OK)
    {
        return RTC_STATUS_ERROR;
    }

    return RTC_STATUS_OK;
}

/**
 * @brief  Get the specified RTC Alarm
 * @param  sAlarm: Pointer to Alarm structure
 * @param  Alarm: Specifies the Alarm. Can be RTC_ALARM_A or RTC_ALARM_B
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_GetAlarm(RTC_Alarm_t* sAlarm, uint32_t Alarm)
{
    RTC_AlarmTypeDef hal_alarm;

    if (sAlarm == NULL)
    {
        return RTC_STATUS_ERROR;
    }

    hal_alarm.Alarm = Alarm;

    if (HAL_RTC_GetAlarm(&hrtc, &hal_alarm, Alarm, RTC_FORMAT_BIN) != HAL_OK)
    {
        return RTC_STATUS_ERROR;
    }

    /* Convert from HAL format */
    sAlarm->AlarmTime.Hours = hal_alarm.AlarmTime.Hours;
    sAlarm->AlarmTime.Minutes = hal_alarm.AlarmTime.Minutes;
    sAlarm->AlarmTime.Seconds = hal_alarm.AlarmTime.Seconds;
    sAlarm->AlarmTime.TimeFormat = hal_alarm.AlarmTime.TimeFormat;
    sAlarm->AlarmMask = hal_alarm.AlarmMask;
    sAlarm->AlarmSubSecondMask = hal_alarm.AlarmSubSecondMask;
    sAlarm->AlarmDateWeekDaySel = hal_alarm.AlarmDateWeekDaySel;
    sAlarm->AlarmDateWeekDay = hal_alarm.AlarmDateWeekDay;
    sAlarm->Alarm = hal_alarm.Alarm;

    return RTC_STATUS_OK;
}

/**
 * @brief  Disable the specified RTC Alarm
 * @param  Alarm: Specifies the Alarm. Can be RTC_ALARM_A or RTC_ALARM_B
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_DisableAlarm(uint32_t Alarm)
{
    if (HAL_RTC_DeactivateAlarm(&hrtc, Alarm) != HAL_OK)
    {
        return RTC_STATUS_ERROR;
    }

    return RTC_STATUS_OK;
}

/**
 * @brief  Get timestamp from RTC
 * @retval uint32_t: Unix timestamp (seconds since Jan 1, 1970)
 */
uint32_t RTC_GetTimestamp(void)
{
    RTC_Date_t sDate;
    RTC_Time_t sTime;

    /* Get current date and time */
    if (RTC_GetDate(&sDate) != RTC_STATUS_OK || RTC_GetTime(&sTime) != RTC_STATUS_OK)
    {
        return 0;
    }

    return RTC_DateTimeToTimestamp(&sDate, &sTime);
}

/**
 * @brief  Set timestamp to RTC
 * @param  timestamp: Unix timestamp (seconds since Jan 1, 1970)
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_SetTimestamp(uint32_t timestamp)
{
    RTC_Date_t sDate;
    RTC_Time_t sTime;

    /* Convert timestamp to date and time */
    RTC_TimestampToDateTime(timestamp, &sDate, &sTime);

    /* Set date and time */
    if (RTC_SetDate(&sDate) != RTC_STATUS_OK || RTC_SetTime(&sTime) != RTC_STATUS_OK)
    {
        return RTC_STATUS_ERROR;
    }

    return RTC_STATUS_OK;
}

/**
 * @brief  Format time as string
 * @param  sTime: Pointer to Time structure
 * @param  buffer: Buffer to store formatted time string
 * @param  size: Size of the buffer
 * @retval None
 */
void RTC_FormatTimeString(RTC_Time_t* sTime, char* buffer, size_t size)
{
    if (sTime == NULL || buffer == NULL)
    {
        return;
    }

    if (sTime->TimeFormat == RTC_HOURFORMAT12_PM || sTime->TimeFormat == RTC_HOURFORMAT12_AM)
    {
        snprintf(buffer, size, "%02d:%02d:%02d %s",
                 sTime->Hours, sTime->Minutes, sTime->Seconds,
                 (sTime->TimeFormat == RTC_HOURFORMAT12_PM) ? "PM" : "AM");
    }
    else
    {
        snprintf(buffer, size, "%02d:%02d:%02d",
                 sTime->Hours, sTime->Minutes, sTime->Seconds);
    }
}

/**
 * @brief  Format date as string
 * @param  sDate: Pointer to Date structure
 * @param  buffer: Buffer to store formatted date string
 * @param  size: Size of the buffer
 * @retval None
 */
void RTC_FormatDateString(RTC_Date_t* sDate, char* buffer, size_t size)
{
    const char* weekdays[] = {"", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

    if (sDate == NULL || buffer == NULL)
    {
        return;
    }

    snprintf(buffer, size, "%s %02d/%02d/20%02d",
             weekdays[sDate->WeekDay],
             sDate->Date, sDate->Month, sDate->Year);
}

/**
 * @brief  Callback function for RTC Alarm
 * @note   This function should be implemented by the user
 * @param  Alarm: Specifies the Alarm. Can be RTC_ALARM_A or RTC_ALARM_B
 * @retval None
 */
__weak void RTC_AlarmCallback(uint32_t Alarm)
{
    /* NOTE: This function should not be modified, when the callback is needed,
             the RTC_AlarmCallback could be implemented in the user file
    */
    UNUSED(Alarm);
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Initialize RTC MSP
 * @retval None
 */
static void RTC_MspInit(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    /* Configure LSI as RTC clock source (more reliable than LSE) */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        return; // Failed to configure LSI
    }

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        return; // Failed to configure RTC clock
    }

    /* Enable RTC peripheral clock */
    __HAL_RCC_RTC_ENABLE();
}

/**
 * @brief  Deinitialize RTC MSP
 * @retval None
 */
static void RTC_MspDeInit(void)
{
    /* Disable RTC peripheral clock */
    __HAL_RCC_RTC_DISABLE();
}

/**
 * @brief  Convert date and time to Unix timestamp
 * @param  sDate: Pointer to Date structure
 * @param  sTime: Pointer to Time structure
 * @retval uint32_t: Unix timestamp
 */
static uint32_t RTC_DateTimeToTimestamp(RTC_Date_t* sDate, RTC_Time_t* sTime)
{
    /* Simple timestamp calculation (not accounting for leap years accurately) */
    /* This is a basic implementation - for production use, consider a more robust solution */

    uint32_t year = 2000 + sDate->Year;
    uint32_t days = 0;

    /* Count days from 1970 to current year */
    for (uint32_t y = 1970; y < year; y++)
    {
        if ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0))
        {
            days += 366; /* Leap year */
        }
        else
        {
            days += 365; /* Normal year */
        }
    }

    /* Days in months for normal year */
    const uint32_t daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    /* Add days for completed months in current year */
    for (uint32_t m = 1; m < sDate->Month; m++)
    {
        days += daysInMonth[m - 1];
        /* Add extra day for February in leap year */
        if (m == 2 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)))
        {
            days += 1;
        }
    }

    /* Add days in current month */
    days += sDate->Date - 1;

    /* Convert to seconds and add time */
    return days * 86400 + sTime->Hours * 3600 + sTime->Minutes * 60 + sTime->Seconds;
}

/**
 * @brief  Convert Unix timestamp to date and time
 * @param  timestamp: Unix timestamp
 * @param  sDate: Pointer to Date structure
 * @param  sTime: Pointer to Time structure
 * @retval None
 */
static void RTC_TimestampToDateTime(uint32_t timestamp, RTC_Date_t* sDate, RTC_Time_t* sTime)
{
    /* Simple timestamp conversion (not accounting for leap years accurately) */
    /* This is a basic implementation - for production use, consider a more robust solution */

    uint32_t days = timestamp / 86400;
    uint32_t seconds = timestamp % 86400;

    /* Calculate time */
    sTime->Hours = seconds / 3600;
    sTime->Minutes = (seconds % 3600) / 60;
    sTime->Seconds = seconds % 60;
    sTime->TimeFormat = RTC_HOURFORMAT_24;
    sTime->DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime->StoreOperation = RTC_STOREOPERATION_RESET;

    /* Calculate date starting from 1970 */
    uint32_t year = 1970;
    while (days >= 365)
    {
        if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
        {
            if (days >= 366)
            {
                days -= 366;
                year++;
            }
            else
            {
                break;
            }
        }
        else
        {
            days -= 365;
            year++;
        }
    }

    /* Days in months for normal year */
    const uint32_t daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    /* Calculate month and day */
    uint32_t month = 1;
    while (month <= 12)
    {
        uint32_t daysThisMonth = daysInMonth[month - 1];
        /* Add extra day for February in leap year */
        if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)))
        {
            daysThisMonth = 29;
        }

        if (days < daysThisMonth)
        {
            break;
        }

        days -= daysThisMonth;
        month++;
    }

    /* Set date */
    sDate->Year = year - 2000;
    sDate->Month = month;
    sDate->Date = days + 1;

    /* Calculate day of week (simplified) */
    /* January 1, 1970 was a Thursday (day 4) */
    uint32_t totalDays = (timestamp / 86400) + 4;
    sDate->WeekDay = (totalDays % 7) + 1;
    if (sDate->WeekDay > 7)
    {
        sDate->WeekDay = 1;
    }
}

/**
 * @brief  RTC Alarm interrupt handler
 * @retval None
 */
void RTC_Alarm_IRQHandler(void)
{
    HAL_RTC_AlarmIRQHandler(&hrtc);
}

/**
 * @brief  RTC Alarm callback
 * @param  hrtc: RTC handle
 * @retval None
 */
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
    UNUSED(hrtc);
    RTC_AlarmCallback(RTC_ALARM_A);
}

/**
 * @brief  RTC Alarm B callback
 * @param  hrtc: RTC handle
 * @retval None
 */
void HAL_RTCEx_AlarmBEventCallback(RTC_HandleTypeDef *hrtc)
{
    UNUSED(hrtc);
    RTC_AlarmCallback(RTC_ALARM_B);
}
