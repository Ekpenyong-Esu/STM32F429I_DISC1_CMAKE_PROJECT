/**
 ******************************************************************************
 * @file    usb.h
 * @brief   USB peripheral header file for STM32F429I-DISC1
 * @author  Your Name
 * @date    September 2025
 ******************************************************************************
 * @attention
 *
 * This file provides USB Host and Device functionality using the STM32 HAL
 * library for the STM32F429I-Discovery board.
 *
 ******************************************************************************
 */

#ifndef __USB_H
#define __USB_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usb_host.h"
#include "usbh_cdc.h"

/* Exported types ------------------------------------------------------------*/

/**
 * @brief USB Mode enumeration
 */
typedef enum {
    USB_MODE_HOST = 0,
    USB_MODE_DEVICE,
    USB_MODE_OTG
} USB_OperationModeTypeDef;

/**
 * @brief USB Status enumeration
 */
typedef enum {
    USB_STATUS_OK = 0,
    USB_STATUS_ERROR,
    USB_STATUS_BUSY,
    USB_STATUS_TIMEOUT,
    USB_STATUS_NOT_SUPPORTED
} USB_StatusTypeDef;

/**
 * @brief USB Device Class enumeration
 */
typedef enum {
    USB_DEVICE_CLASS_CDC = 0,
    USB_DEVICE_CLASS_MSC,
    USB_DEVICE_CLASS_HID,
    USB_DEVICE_CLASS_AUDIO
} USB_DeviceClassTypeDef;

/**
 * @brief USB Configuration structure
 */
typedef struct {
    USB_OperationModeTypeDef mode;
    uint32_t speed;                    /* USB_OTG_SPEED_HIGH or USB_OTG_SPEED_FULL */
    uint32_t phy_itface;              /* USB_OTG_ULPI_PHY or USB_OTG_EMBEDDED_PHY */
    uint32_t dma_enable;              /* Enable or disable DMA */
    uint32_t low_power_enable;        /* Enable or disable low power mode */
    uint32_t lpm_enable;              /* Enable or disable Link Power Management */
    uint32_t battery_charging_enable; /* Enable or disable Battery charging */
} USB_ConfigTypeDef;

/**
 * @brief USB Host Application State
 */
typedef enum {
    USB_HOST_IDLE = 0,
    USB_HOST_WAIT_FOR_ATTACHMENT,
    USB_HOST_DEVICE_ATTACHED,
    USB_HOST_ENUMERATION_DONE,
    USB_HOST_USER_INPUT,
    USB_HOST_CLASS_REQUEST,
    USB_HOST_INPUT_PROCESS,
    USB_HOST_CLASS_ACTIVE,
    USB_HOST_CTRL_XFER,
    USB_HOST_SUSPENDED,
    USB_HOST_ERROR_STATE
} USB_HostStateTypeDef;

/* Exported constants --------------------------------------------------------*/

/* USB OTG HS configuration */
#define USB_OTG_HS_CORE                     USB_OTG_HS
#define USB_OTG_HS_DATA_FIFO_OFFSET         0x1000U
#define USB_OTG_HS_DATA_FIFO_SIZE           4096U

/* USB Host CDC receive buffer size */
#define USB_HOST_RX_BUFFER_SIZE             512U
#define USB_HOST_TX_BUFFER_SIZE             512U

/* USB timeout definitions */
#define USB_TIMEOUT_DEFAULT                 5000U
#define USB_TIMEOUT_CONNECT                 3000U

/* Exported macro ------------------------------------------------------------*/

/**
 * @brief Check if USB Host is ready
 */
#define USB_HOST_IS_READY()                 (Appli_state == APPLICATION_READY)

/**
 * @brief Check if USB device is connected
 */
#define USB_HOST_IS_CONNECTED()             (Appli_state != APPLICATION_DISCONNECT)

/* Exported functions prototypes ---------------------------------------------*/

/* USB Initialization and Configuration */
USB_StatusTypeDef USB_Init(USB_ConfigTypeDef *config);
USB_StatusTypeDef USB_DeInit(void);
USB_StatusTypeDef USB_Start(void);
USB_StatusTypeDef USB_Stop(void);

/* USB Host Functions */
USB_StatusTypeDef USB_Host_Init(void);
USB_StatusTypeDef USB_Host_DeInit(void);
USB_StatusTypeDef USB_Host_Process(void);
USB_HostStateTypeDef USB_Host_GetState(void);

/* USB Host CDC Functions */
USB_StatusTypeDef USB_Host_CDC_Transmit(uint8_t *data, uint16_t length);
USB_StatusTypeDef USB_Host_CDC_Receive(uint8_t *data, uint16_t length);
USB_StatusTypeDef USB_Host_CDC_SetLineCoding(CDC_LineCodingTypeDef *linecoding);
USB_StatusTypeDef USB_Host_CDC_GetLineCoding(CDC_LineCodingTypeDef *linecoding);
uint16_t USB_Host_CDC_GetLastReceivedDataSize(void);

/* USB Device Functions (placeholder for future implementation) */
USB_StatusTypeDef USB_Device_Init(USB_DeviceClassTypeDef device_class);
USB_StatusTypeDef USB_Device_DeInit(void);
USB_StatusTypeDef USB_Device_Start(void);
USB_StatusTypeDef USB_Device_Stop(void);

/* USB Utility Functions */
USB_StatusTypeDef USB_SetOperationMode(USB_OperationModeTypeDef mode);
USB_OperationModeTypeDef USB_GetOperationMode(void);
uint8_t USB_IsDeviceConnected(void);
uint32_t USB_GetConnectedDeviceVID(void);
uint32_t USB_GetConnectedDevicePID(void);

/* USB Interrupt and Callback Functions */
void USB_IRQHandler(void);
void USB_ConnectCallback(void);
void USB_DisconnectCallback(void);
void USB_DataReceivedCallback(const uint8_t *data, uint16_t length);
void USB_TransmitCompleteCallback(void);

/* USB Error Handling */
void USB_ErrorHandler(uint32_t error_code);

/* Exported variables --------------------------------------------------------*/
extern USBH_HandleTypeDef hUsbHostHS;
extern ApplicationTypeDef Appli_state;

#ifdef __cplusplus
}
#endif

#endif /* __USB_H */

/**
 * @}
 */

/**
 * @}
 */
