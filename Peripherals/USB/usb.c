/**
 ******************************************************************************
 * @file    usb.c
 * @brief   USB peripheral implementation file for STM32F429I-DISC1
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

/* Includes ------------------------------------------------------------------*/
#include "usb.h"
#include "main.h"
#include <string.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static USB_OperationModeTypeDef usb_current_mode = USB_MODE_HOST;
static uint8_t usb_tx_buffer[USB_HOST_TX_BUFFER_SIZE];
static uint16_t usb_last_received_size = 0;

/* External variables --------------------------------------------------------*/
/* These are defined in usb_host.c */

/* Private function prototypes -----------------------------------------------*/
/* None */

/* Private functions ---------------------------------------------------------*/

/* Public functions ----------------------------------------------------------*/

/**
 * @brief Initialize USB peripheral
 * @param config: USB configuration structure
 * @retval USB_StatusTypeDef: Operation status
 */
USB_StatusTypeDef USB_Init(USB_ConfigTypeDef *config)
{
    if (config == NULL)
    {
        return USB_STATUS_ERROR;
    }

    usb_current_mode = config->mode;

    switch (config->mode)
    {
        case USB_MODE_HOST:
            return USB_Host_Init();

        case USB_MODE_DEVICE:
            /* Device mode initialization would go here */
            return USB_STATUS_NOT_SUPPORTED;

        case USB_MODE_OTG:
            /* OTG mode initialization would go here */
            return USB_STATUS_NOT_SUPPORTED;

        default:
            return USB_STATUS_ERROR;
    }
}

/**
 * @brief Deinitialize USB peripheral
 * @retval USB_StatusTypeDef: Operation status
 */
USB_StatusTypeDef USB_DeInit(void)
{
    switch (usb_current_mode)
    {
        case USB_MODE_HOST:
            return USB_Host_DeInit();

        case USB_MODE_DEVICE:
            return USB_Device_DeInit();

        case USB_MODE_OTG:
            return USB_STATUS_NOT_SUPPORTED;

        default:
            return USB_STATUS_ERROR;
    }
}

/**
 * @brief Start USB peripheral
 * @retval USB_StatusTypeDef: Operation status
 */
USB_StatusTypeDef USB_Start(void)
{
    switch (usb_current_mode)
    {
        case USB_MODE_HOST:
            if (USBH_Start(&hUsbHostHS) == USBH_OK)
            {
                return USB_STATUS_OK;
            }
            break;

        case USB_MODE_DEVICE:
            return USB_Device_Start();

        case USB_MODE_OTG:
            return USB_STATUS_NOT_SUPPORTED;

        default:
            break;
    }

    return USB_STATUS_ERROR;
}

/**
 * @brief Stop USB peripheral
 * @retval USB_StatusTypeDef: Operation status
 */
USB_StatusTypeDef USB_Stop(void)
{
    switch (usb_current_mode)
    {
        case USB_MODE_HOST:
            if (USBH_Stop(&hUsbHostHS) == USBH_OK)
            {
                return USB_STATUS_OK;
            }
            break;

        case USB_MODE_DEVICE:
            return USB_Device_Stop();

        case USB_MODE_OTG:
            return USB_STATUS_NOT_SUPPORTED;

        default:
            break;
    }

    return USB_STATUS_ERROR;
}

/**
 * @brief Initialize USB Host
 * @retval USB_StatusTypeDef: Operation status
 */
USB_StatusTypeDef USB_Host_Init(void)
{
    /* Initialize USB Host */
    MX_USB_HOST_Init();

    return USB_STATUS_OK;
}

/**
 * @brief Deinitialize USB Host
 * @retval USB_StatusTypeDef: Operation status
 */
USB_StatusTypeDef USB_Host_DeInit(void)
{
    if (USBH_DeInit(&hUsbHostHS) == USBH_OK)
    {
        return USB_STATUS_OK;
    }

    return USB_STATUS_ERROR;
}

/**
 * @brief Process USB Host state machine
 * @retval USB_StatusTypeDef: Operation status
 */
USB_StatusTypeDef USB_Host_Process(void)
{
    USBH_Process(&hUsbHostHS);
    return USB_STATUS_OK;
}

/**
 * @brief Get USB Host state
 * @retval USB_HostStateTypeDef: Current host state
 */
USB_HostStateTypeDef USB_Host_GetState(void)
{
    switch (Appli_state)
    {
        case APPLICATION_IDLE:
            return USB_HOST_IDLE;

        case APPLICATION_START:
            return USB_HOST_DEVICE_ATTACHED;

        case APPLICATION_READY:
            return USB_HOST_CLASS_ACTIVE;

        case APPLICATION_DISCONNECT:
            return USB_HOST_WAIT_FOR_ATTACHMENT;

        default:
            return USB_HOST_ERROR_STATE;
    }
}

/**
 * @brief Transmit data via USB Host CDC
 * @param data: Pointer to data buffer
 * @param length: Data length
 * @retval USB_StatusTypeDef: Operation status
 */
USB_StatusTypeDef USB_Host_CDC_Transmit(uint8_t *data, uint16_t length)
{
    if (data == NULL || length == 0 || length > USB_HOST_TX_BUFFER_SIZE)
    {
        return USB_STATUS_ERROR;
    }

    if (Appli_state != APPLICATION_READY)
    {
        return USB_STATUS_ERROR;
    }

    /* Copy data to transmit buffer */
    memcpy(usb_tx_buffer, data, length);

    if (USBH_CDC_Transmit(&hUsbHostHS, usb_tx_buffer, length) == USBH_OK)
    {
        return USB_STATUS_OK;
    }

    return USB_STATUS_ERROR;
}

/**
 * @brief Receive data via USB Host CDC
 * @param data: Pointer to receive buffer
 * @param length: Maximum data length to receive
 * @retval USB_StatusTypeDef: Operation status
 */
USB_StatusTypeDef USB_Host_CDC_Receive(uint8_t *data, uint16_t length)
{
    if (data == NULL || length == 0)
    {
        return USB_STATUS_ERROR;
    }

    if (Appli_state != APPLICATION_READY)
    {
        return USB_STATUS_ERROR;
    }

    if (USBH_CDC_Receive(&hUsbHostHS, data,
                        (length > USB_HOST_RX_BUFFER_SIZE) ? USB_HOST_RX_BUFFER_SIZE : length) == USBH_OK)
    {
        return USB_STATUS_OK;
    }

    return USB_STATUS_ERROR;
}

/**
 * @brief Set CDC Line Coding
 * @param linecoding: Pointer to line coding structure
 * @retval USB_StatusTypeDef: Operation status
 */
USB_StatusTypeDef USB_Host_CDC_SetLineCoding(CDC_LineCodingTypeDef *linecoding)
{
    if (linecoding == NULL)
    {
        return USB_STATUS_ERROR;
    }

    if (Appli_state != APPLICATION_READY)
    {
        return USB_STATUS_ERROR;
    }

    if (USBH_CDC_SetLineCoding(&hUsbHostHS, linecoding) == USBH_OK)
    {
        return USB_STATUS_OK;
    }

    return USB_STATUS_ERROR;
}

/**
 * @brief Get CDC Line Coding
 * @param linecoding: Pointer to line coding structure
 * @retval USB_StatusTypeDef: Operation status
 */
USB_StatusTypeDef USB_Host_CDC_GetLineCoding(CDC_LineCodingTypeDef *linecoding)
{
    if (linecoding == NULL)
    {
        return USB_STATUS_ERROR;
    }

    if (Appli_state != APPLICATION_READY)
    {
        return USB_STATUS_ERROR;
    }

    if (USBH_CDC_GetLineCoding(&hUsbHostHS, linecoding) == USBH_OK)
    {
        return USB_STATUS_OK;
    }

    return USB_STATUS_ERROR;
}

/**
 * @brief Get last received data size
 * @retval uint16_t: Last received data size
 */
uint16_t USB_Host_CDC_GetLastReceivedDataSize(void)
{
    return usb_last_received_size;
}

/**
 * @brief Initialize USB Device (placeholder)
 * @param device_class: Device class to initialize
 * @retval USB_StatusTypeDef: Operation status
 */
USB_StatusTypeDef USB_Device_Init(USB_DeviceClassTypeDef device_class)
{
    UNUSED(device_class);
    /* This would contain USB Device initialization code */
    return USB_STATUS_NOT_SUPPORTED;
}

/**
 * @brief Deinitialize USB Device (placeholder)
 * @retval USB_StatusTypeDef: Operation status
 */
USB_StatusTypeDef USB_Device_DeInit(void)
{
    /* This would contain USB Device deinitialization code */
    return USB_STATUS_NOT_SUPPORTED;
}

/**
 * @brief Start USB Device (placeholder)
 * @retval USB_StatusTypeDef: Operation status
 */
USB_StatusTypeDef USB_Device_Start(void)
{
    /* This would contain USB Device start code */
    return USB_STATUS_NOT_SUPPORTED;
}

/**
 * @brief Stop USB Device (placeholder)
 * @retval USB_StatusTypeDef: Operation status
 */
USB_StatusTypeDef USB_Device_Stop(void)
{
    /* This would contain USB Device stop code */
    return USB_STATUS_NOT_SUPPORTED;
}

/**
 * @brief Set USB operation mode
 * @param mode: Operation mode to set
 * @retval USB_StatusTypeDef: Operation status
 */
USB_StatusTypeDef USB_SetOperationMode(USB_OperationModeTypeDef mode)
{
    usb_current_mode = mode;
    return USB_STATUS_OK;
}

/**
 * @brief Get current USB operation mode
 * @retval USB_OperationModeTypeDef: Current operation mode
 */
USB_OperationModeTypeDef USB_GetOperationMode(void)
{
    return usb_current_mode;
}

/**
 * @brief Check if USB device is connected
 * @retval uint8_t: 1 if connected, 0 if not connected
 */
uint8_t USB_IsDeviceConnected(void)
{
    return (Appli_state == APPLICATION_READY) ? 1 : 0;
}

/**
 * @brief Get connected device VID
 * @retval uint32_t: Device VID
 */
uint32_t USB_GetConnectedDeviceVID(void)
{
    if (Appli_state == APPLICATION_READY)
    {
        return hUsbHostHS.device.DevDesc.idVendor;
    }

    return 0;
}

/**
 * @brief Get connected device PID
 * @retval uint32_t: Device PID
 */
uint32_t USB_GetConnectedDevicePID(void)
{
    if (Appli_state == APPLICATION_READY)
    {
        return hUsbHostHS.device.DevDesc.idProduct;
    }

    return 0;
}

/**
 * @brief USB interrupt handler
 * @retval None
 */
void USB_IRQHandler(void)
{
    HAL_HCD_IRQHandler(hUsbHostHS.pData);
}

/**
 * @brief USB Error Handler
 * @param error_code: Error code
 * @retval None
 */
void USB_ErrorHandler(uint32_t error_code)
{
    UNUSED(error_code);
    /* Handle USB specific errors */
    /* This is a weak function that can be overridden by user */
    Error_Handler();
}

/* Weak callback functions that can be overridden by user ----------------*/

/**
 * @brief USB Connect Callback (weak)
 * @retval None
 */
__weak void USB_ConnectCallback(void)
{
    /* User can override this function */
}

/**
 * @brief USB Disconnect Callback (weak)
 * @retval None
 */
__weak void USB_DisconnectCallback(void)
{
    /* User can override this function */
}

/**
 * @brief USB Data Received Callback (weak)
 * @param data: Pointer to received data
 * @param length: Data length
 * @retval None
 */
__weak void USB_DataReceivedCallback(const uint8_t *data, uint16_t length)
{
    /* User can override this function */
    UNUSED(data);
    UNUSED(length);
}

/**
 * @brief USB Transmit Complete Callback (weak)
 * @retval None
 */
__weak void USB_TransmitCompleteCallback(void)
{
    /* User can override this function */
}

/**
 * @}
 */

/**
 * @}
 */
