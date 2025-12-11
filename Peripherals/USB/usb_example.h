/**
 ******************************************************************************
 * @file    usb_example.h
 * @brief   USB example header file for STM32F429I-DISC1
 * @author  Your Name
 * @date    September 2025
 ******************************************************************************
 * @attention
 *
 * This file provides USB example usage for the STM32F429I-Discovery board.
 *
 ******************************************************************************
 */

#ifndef __USB_EXAMPLE_H
#define __USB_EXAMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usb.h"

/* Exported types ------------------------------------------------------------*/

/**
 * @brief USB Example Mode enumeration
 */
typedef enum {
    USB_EXAMPLE_HOST_CDC = 0,
    USB_EXAMPLE_HOST_MSC,
    USB_EXAMPLE_DEVICE_CDC,
    USB_EXAMPLE_LOOPBACK
} USB_ExampleModeTypeDef;

/* Exported constants --------------------------------------------------------*/

#define USB_EXAMPLE_BUFFER_SIZE         256U
#define USB_EXAMPLE_TIMEOUT             5000U

/* Test strings for demonstration */
#define USB_TEST_STRING                 "Hello from STM32F429I-Discovery USB Host!\r\n"
#define USB_WELCOME_MESSAGE             "USB Host CDC Example - STM32F429I-DISC1\r\n"

/* Exported functions prototypes ---------------------------------------------*/

/* USB Example initialization and control */
USB_StatusTypeDef USB_Example_Init(USB_ExampleModeTypeDef mode);
USB_StatusTypeDef USB_Example_DeInit(void);
void USB_Example_Process(void);

/* USB Host CDC Examples */
void USB_Example_Host_CDC_Demo(void);
void USB_Example_Host_CDC_Echo(void);
void USB_Example_Host_CDC_SendTestData(void);
USB_StatusTypeDef USB_Example_Host_CDC_Configure(uint32_t baudrate);

/* USB Host Mass Storage Examples */
void USB_Example_Host_MSC_Demo(void);
void USB_Example_Host_MSC_ListFiles(void);
USB_StatusTypeDef USB_Example_Host_MSC_ReadFile(const char *filename, uint8_t *buffer, uint32_t size);
USB_StatusTypeDef USB_Example_Host_MSC_WriteFile(const char *filename, const uint8_t *data, uint32_t size);

/* USB Device Examples */
void USB_Example_Device_CDC_Demo(void);
void USB_Example_Device_CDC_Loopback(void);

/* USB Status and Information Functions */
void USB_Example_PrintDeviceInfo(void);
void USB_Example_PrintConnectionStatus(void);
USB_StatusTypeDef USB_Example_WaitForDeviceConnection(uint32_t timeout);

/* USB Example Callback Functions */
void USB_Example_ConnectCallback(void);
void USB_Example_DisconnectCallback(void);
void USB_Example_DataReceivedCallback(const uint8_t *data, uint16_t length);
void USB_Example_TransmitCompleteCallback(void);

/* Utility Functions */
void USB_Example_Delay(uint32_t delay_ms);
void USB_Example_PrintStatus(const char *message);
void USB_Example_PrintError(const char *error_message);

#ifdef __cplusplus
}
#endif

#endif /* __USB_EXAMPLE_H */

/**
 * @}
 */

/**
 * @}
 */
