/**
 ******************************************************************************
 * @file    rtc.h
 * @author  Mahonri
 * @brief   RTC peripheral driver for STM32F429I Discovery board
 *          This file provides a clean architecture interface for RTC operations
 ******************************************************************************
 * @attention
 *
 * This software is provided as-is, without any express or implied warranties.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RTC_H
#define __RTC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief RTC Time structure definition
 */
typedef struct {
    uint8_t Hours;      /*!< Specifies the RTC Time Hour.
                             This parameter must be a number between Min_Data = 0 and Max_Data = 12 if the RTC_HourFormat_12 is selected.
                             This parameter must be a number between Min_Data = 0 and Max_Data = 23 if the RTC_HourFormat_24 is selected */

    uint8_t Minutes;    /*!< Specifies the RTC Time Minutes.
                             This parameter must be a number between Min_Data = 0 and Max_Data = 59 */

    uint8_t Seconds;    /*!< Specifies the RTC Time Seconds.
                             This parameter must be a number between Min_Data = 0 and Max_Data = 59 */

    uint8_t TimeFormat; /*!< Specifies the RTC AM/PM Time.
                             This parameter can be a value of @ref RTC_AM_PM_Definitions */

    uint32_t DayLightSaving; /*!< Specifies DayLight Save Operation.
                                  This parameter can be a value of @ref RTC_DayLightSaving_Definitions */

    uint32_t StoreOperation; /*!< Specifies RTC_StoreOperation value to be written in the BCK bit
                                  in CR register to store the operation.
                                  This parameter can be a value of @ref RTC_StoreOperation_Definitions */
} RTC_Time_t;

/**
 * @brief RTC Date structure definition
 */
typedef struct {
    uint8_t WeekDay; /*!< Specifies the RTC Date WeekDay.
                          This parameter can be a value of @ref RTC_WeekDay_Definitions */

    uint8_t Month;   /*!< Specifies the RTC Date Month (in BCD format).
                          This parameter can be a value of @ref RTC_Month_Date_Definitions */

    uint8_t Date;    /*!< Specifies the RTC Date.
                          This parameter must be a number between Min_Data = 1 and Max_Data = 31 */

    uint8_t Year;    /*!< Specifies the RTC Date Year.
                          This parameter must be a number between Min_Data = 0 and Max_Data = 99 */
} RTC_Date_t;

/**
 * @brief RTC Alarm structure definition
 */
typedef struct {
    RTC_Time_t AlarmTime;          /*!< Specifies the RTC Alarm Time members */

    uint32_t AlarmMask;            /*!< Specifies the RTC Alarm Masks.
                                        This parameter can be a value of @ref RTC_AlarmMask_Definitions */

    uint32_t AlarmSubSecondMask;   /*!< Specifies the RTC Alarm SubSeconds Masks.
                                        This parameter can be a value of @ref RTC_Alarm_Sub_Seconds_Masks_Definitions */

    uint32_t AlarmDateWeekDaySel;  /*!< Specifies the RTC Alarm is on Date or WeekDay.
                                        This parameter can be a value of @ref RTC_AlarmDateWeekDay_Definitions */

    uint8_t AlarmDateWeekDay;      /*!< Specifies the RTC Alarm Date/WeekDay.
                                        If the Alarm Date is selected, this parameter
                                        must be set to a value in the 1-31 range.
                                        If the Alarm WeekDay is selected, this
                                        parameter can be a value of @ref RTC_WeekDay_Definitions */

    uint32_t Alarm;                /*!< Specifies the alarm .
                                        This parameter can be a value of @ref RTC_Alarms_Definitions */
} RTC_Alarm_t;

/**
 * @brief RTC configuration status enumeration
 */
typedef enum {
    RTC_STATUS_OK       = 0x00U,
    RTC_STATUS_ERROR    = 0x01U,
    RTC_STATUS_BUSY     = 0x02U,
    RTC_STATUS_TIMEOUT  = 0x03U
} RTC_StatusTypeDef;

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/**
 * @brief  Convert a 2 digit decimal to BCD format.
 * @param  VALUE: Byte to be converted
 * @retval Converted byte
 */
#define RTC_ByteToBcd2(VALUE) \
    ((uint8_t)(((VALUE) / 10) << 4) | ((VALUE) % 10))

/**
 * @brief  Convert from 2 digit BCD to Binary.
 * @param  VALUE: BCD value to be converted
 * @retval Converted word
 */
#define RTC_Bcd2ToByte(VALUE) \
    (uint8_t)(((uint8_t)((VALUE) & 0xF0U) >> 4) * 10 + ((VALUE) & 0x0FU))

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  Initialize the RTC peripheral
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_Init(void);

/**
 * @brief  Deinitialize the RTC peripheral
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_DeInit(void);

/**
 * @brief  Set the RTC current time
 * @param  sTime: Pointer to Time structure
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_SetTime(RTC_Time_t* sTime);

/**
 * @brief  Get the RTC current time
 * @param  sTime: Pointer to Time structure
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_GetTime(RTC_Time_t* sTime);

/**
 * @brief  Set the RTC current date
 * @param  sDate: Pointer to date structure
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_SetDate(RTC_Date_t* sDate);

/**
 * @brief  Get the RTC current date
 * @param  sDate: Pointer to Date structure
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_GetDate(RTC_Date_t* sDate);

/**
 * @brief  Set the specified RTC Alarm
 * @param  sAlarm: Pointer to Alarm structure
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_SetAlarm(RTC_Alarm_t* sAlarm);

/**
 * @brief  Get the specified RTC Alarm
 * @param  sAlarm: Pointer to Alarm structure
 * @param  Alarm: Specifies the Alarm. Can be RTC_ALARM_A or RTC_ALARM_B
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_GetAlarm(RTC_Alarm_t* sAlarm, uint32_t Alarm);

/**
 * @brief  Disable the specified RTC Alarm
 * @param  Alarm: Specifies the Alarm. Can be RTC_ALARM_A or RTC_ALARM_B
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_DisableAlarm(uint32_t Alarm);

/**
 * @brief  Get timestamp from RTC
 * @retval uint32_t: Unix timestamp (seconds since Jan 1, 1970)
 */
uint32_t RTC_GetTimestamp(void);

/**
 * @brief  Set timestamp to RTC
 * @param  timestamp: Unix timestamp (seconds since Jan 1, 1970)
 * @retval RTC_StatusTypeDef: Status of the operation
 */
RTC_StatusTypeDef RTC_SetTimestamp(uint32_t timestamp);

/**
 * @brief  Format time as string
 * @param  sTime: Pointer to Time structure
 * @param  buffer: Buffer to store formatted time string
 * @param  size: Size of the buffer
 * @retval None
 */
void RTC_FormatTimeString(RTC_Time_t* sTime, char* buffer, size_t size);

/**
 * @brief  Format date as string
 * @param  sDate: Pointer to Date structure
 * @param  buffer: Buffer to store formatted date string
 * @param  size: Size of the buffer
 * @retval None
 */
void RTC_FormatDateString(RTC_Date_t* sDate, char* buffer, size_t size);

/**
 * @brief  Callback function for RTC Alarm
 * @note   This function should be implemented by the user
 * @param  Alarm: Specifies the Alarm. Can be RTC_ALARM_A or RTC_ALARM_B
 * @retval None
 */
void RTC_AlarmCallback(uint32_t Alarm);

#ifdef __cplusplus
}
#endif

#endif /* __RTC_H */
