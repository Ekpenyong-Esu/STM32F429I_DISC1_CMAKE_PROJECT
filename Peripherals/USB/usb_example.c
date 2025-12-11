/**
 ******************************************************************************
 * @file    usb_example.c
 * @brief   USB example implementation file for STM32F429I-DISC1
 * @author  Your Name
 * @date    September 2025
 ******************************************************************************
 * @attention
 *
 * This file provides USB example implementations for the STM32F429I-Discovery board.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "usb_example.h"
#include <stdio.h>
#include <string.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static USB_ExampleModeTypeDef current_example_mode = USB_EXAMPLE_HOST_CDC;
static uint8_t usb_example_buffer[USB_EXAMPLE_BUFFER_SIZE];
static volatile uint8_t data_received_flag = 0;
static volatile uint8_t transmit_complete_flag = 0;
static uint16_t received_data_length = 0;

/* Private function prototypes -----------------------------------------------*/
static void USB_Example_InitializeCDC(void);
static void USB_Example_ProcessReceivedData(void);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Initialize CDC for examples
 * @retval None
 */
static void USB_Example_InitializeCDC(void)
{
    CDC_LineCodingTypeDef linecoding;

    /* Configure CDC line coding */
    linecoding.b.dwDTERate = 115200;
    linecoding.b.bCharFormat = 0;      /* 1 stop bit */
    linecoding.b.bParityType = 0;      /* No parity */
    linecoding.b.bDataBits = 8;        /* 8 data bits */

    USB_Host_CDC_SetLineCoding(&linecoding);
}

/**
 * @brief Process received data
 * @retval None
 */
static void USB_Example_ProcessReceivedData(void)
{
    if (data_received_flag)
    {
        data_received_flag = 0;

        /* Echo received data back */
        if (current_example_mode == USB_EXAMPLE_LOOPBACK)
        {
            USB_Host_CDC_Transmit(usb_example_buffer, received_data_length);
        }

        /* Print received data (example) */
        USB_Example_PrintStatus("Data received");
    }
}

/* Public functions ----------------------------------------------------------*/

/**
 * @brief Initialize USB Example
 * @param mode: Example mode to initialize
 * @retval USB_StatusTypeDef: Operation status
 */
USB_StatusTypeDef USB_Example_Init(USB_ExampleModeTypeDef mode)
{
    USB_ConfigTypeDef usb_config;
    USB_StatusTypeDef status;

    current_example_mode = mode;

    /* Initialize USB configuration */
    usb_config.mode = USB_MODE_HOST;
    usb_config.speed = USB_OTG_SPEED_HIGH;
    usb_config.phy_itface = USB_OTG_ULPI_PHY;
    usb_config.dma_enable = DISABLE;
    usb_config.low_power_enable = DISABLE;
    usb_config.lpm_enable = DISABLE;
    usb_config.battery_charging_enable = DISABLE;

    /* Initialize USB */
    status = USB_Init(&usb_config);
    if (status != USB_STATUS_OK)
    {
        USB_Example_PrintError("USB initialization failed");
        return status;
    }

    /* Start USB */
    status = USB_Start();
    if (status != USB_STATUS_OK)
    {
        USB_Example_PrintError("USB start failed");
        return status;
    }

    USB_Example_PrintStatus("USB Example initialized");

    return USB_STATUS_OK;
}

/**
 * @brief Deinitialize USB Example
 * @retval USB_StatusTypeDef: Operation status
 */
USB_StatusTypeDef USB_Example_DeInit(void)
{
    USB_StatusTypeDef status;

    status = USB_Stop();
    if (status != USB_STATUS_OK)
    {
        return status;
    }

    status = USB_DeInit();
    if (status != USB_STATUS_OK)
    {
        return status;
    }

    USB_Example_PrintStatus("USB Example deinitialized");

    return USB_STATUS_OK;
}

/**
 * @brief Process USB Example state machine
 * @retval None
 */
void USB_Example_Process(void)
{
    static uint32_t last_tick = 0;

    /* Process USB Host */
    USB_Host_Process();

    /* Process received data */
    USB_Example_ProcessReceivedData();

    /* Run example specific processing every 100ms */
    if (HAL_GetTick() - last_tick > 100)
    {
        last_tick = HAL_GetTick();

        switch (current_example_mode)
        {
            case USB_EXAMPLE_HOST_CDC:
                /* CDC example processing would go here */
                break;

            case USB_EXAMPLE_HOST_MSC:
                /* MSC example processing would go here */
                break;

            case USB_EXAMPLE_DEVICE_CDC:
                /* Device CDC example processing would go here */
                break;

            case USB_EXAMPLE_LOOPBACK:
                /* Loopback example processing would go here */
                break;

            default:
                break;
        }
    }
}

/**
 * @brief USB Host CDC Demo
 * @retval None
 */
void USB_Example_Host_CDC_Demo(void)
{
    if (!USB_IsDeviceConnected())
    {
        USB_Example_PrintStatus("Waiting for USB device connection...");
        return;
    }

    USB_Example_PrintStatus("Running USB Host CDC Demo");

    /* Initialize CDC */
    USB_Example_InitializeCDC();

    /* Send welcome message */
    USB_Host_CDC_Transmit((uint8_t *)USB_WELCOME_MESSAGE, strlen(USB_WELCOME_MESSAGE));

    /* Start receiving data */
    USB_Host_CDC_Receive(usb_example_buffer, USB_EXAMPLE_BUFFER_SIZE);
}

/**
 * @brief USB Host CDC Echo Demo
 * @retval None
 */
void USB_Example_Host_CDC_Echo(void)
{
    if (!USB_IsDeviceConnected())
    {
        return;
    }

    /* Set mode to loopback for echo functionality */
    current_example_mode = USB_EXAMPLE_LOOPBACK;

    /* Start receiving data for echo */
    USB_Host_CDC_Receive(usb_example_buffer, USB_EXAMPLE_BUFFER_SIZE);
}

/**
 * @brief Send test data via USB Host CDC
 * @retval None
 */
void USB_Example_Host_CDC_SendTestData(void)
{
    if (!USB_IsDeviceConnected())
    {
        USB_Example_PrintError("No USB device connected");
        return;
    }

    USB_Example_PrintStatus("Sending test data...");

    /* Send test string */
    USB_Host_CDC_Transmit((uint8_t *)USB_TEST_STRING, strlen(USB_TEST_STRING));
}

/**
 * @brief Configure USB Host CDC
 * @param baudrate: Baudrate to set
 * @retval USB_StatusTypeDef: Operation status
 */
USB_StatusTypeDef USB_Example_Host_CDC_Configure(uint32_t baudrate)
{
    CDC_LineCodingTypeDef linecoding;

    if (!USB_IsDeviceConnected())
    {
        return USB_STATUS_ERROR;
    }

    /* Set line coding */
    linecoding.b.dwDTERate = baudrate;
    linecoding.b.bCharFormat = 0;      /* 1 stop bit */
    linecoding.b.bParityType = 0;      /* No parity */
    linecoding.b.bDataBits = 8;        /* 8 data bits */

    return USB_Host_CDC_SetLineCoding(&linecoding);
}

/**
 * @brief USB Host Mass Storage Demo (placeholder)
 * @retval None
 */
void USB_Example_Host_MSC_Demo(void)
{
    USB_Example_PrintStatus("USB Host MSC Demo - Not implemented");
}

/**
 * @brief List files on USB Mass Storage device (placeholder)
 * @retval None
 */
void USB_Example_Host_MSC_ListFiles(void)
{
    USB_Example_PrintStatus("USB Host MSC List Files - Not implemented");
}

/**
 * @brief Read file from USB Mass Storage device (placeholder)
 * @param filename: File name to read
 * @param buffer: Buffer to store data
 * @param size: Buffer size
 * @retval USB_StatusTypeDef: Operation status
 */
USB_StatusTypeDef USB_Example_Host_MSC_ReadFile(const char *filename, uint8_t *buffer, uint32_t size)
{
    UNUSED(filename);
    UNUSED(buffer);
    UNUSED(size);

    return USB_STATUS_NOT_SUPPORTED;
}

/**
 * @brief Write file to USB Mass Storage device (placeholder)
 * @param filename: File name to write
 * @param data: Data to write
 * @param size: Data size
 * @retval USB_StatusTypeDef: Operation status
 */
USB_StatusTypeDef USB_Example_Host_MSC_WriteFile(const char *filename, const uint8_t *data, uint32_t size)
{
    UNUSED(filename);
    UNUSED(data);
    UNUSED(size);

    return USB_STATUS_NOT_SUPPORTED;
}

/**
 * @brief USB Device CDC Demo (placeholder)
 * @retval None
 */
void USB_Example_Device_CDC_Demo(void)
{
    USB_Example_PrintStatus("USB Device CDC Demo - Not implemented");
}

/**
 * @brief USB Device CDC Loopback (placeholder)
 * @retval None
 */
void USB_Example_Device_CDC_Loopback(void)
{
    USB_Example_PrintStatus("USB Device CDC Loopback - Not implemented");
}

/**
 * @brief Print connected device information
 * @retval None
 */
void USB_Example_PrintDeviceInfo(void)
{
    if (USB_IsDeviceConnected())
    {
        uint32_t vid = USB_GetConnectedDeviceVID();
        uint32_t pid = USB_GetConnectedDevicePID();

        char info_buffer[128];
        snprintf(info_buffer, sizeof(info_buffer),
                "Connected Device - VID: 0x%04X, PID: 0x%04X", vid, pid);
        USB_Example_PrintStatus(info_buffer);
    }
    else
    {
        USB_Example_PrintStatus("No USB device connected");
    }
}

/**
 * @brief Print USB connection status
 * @retval None
 */
void USB_Example_PrintConnectionStatus(void)
{
    USB_HostStateTypeDef state = USB_Host_GetState();
    const char *state_names[] = {
        "IDLE", "WAIT_FOR_ATTACHMENT", "DEVICE_ATTACHED",
        "ENUMERATION_DONE", "USER_INPUT", "CLASS_REQUEST",
        "INPUT_PROCESS", "CLASS_ACTIVE", "CTRL_XFER",
        "SUSPENDED", "ERROR_STATE"
    };

    char status_buffer[64];
    snprintf(status_buffer, sizeof(status_buffer),
            "USB Host State: %s", state_names[state]);
    USB_Example_PrintStatus(status_buffer);
}

/**
 * @brief Wait for USB device connection
 * @param timeout: Timeout in milliseconds
 * @retval USB_StatusTypeDef: Operation status
 */
USB_StatusTypeDef USB_Example_WaitForDeviceConnection(uint32_t timeout)
{
    uint32_t start_time = HAL_GetTick();

    USB_Example_PrintStatus("Waiting for USB device connection...");

    while ((HAL_GetTick() - start_time) < timeout)
    {
        USB_Host_Process();

        if (USB_IsDeviceConnected())
        {
            USB_Example_PrintStatus("USB device connected");
            return USB_STATUS_OK;
        }

        HAL_Delay(10);
    }

    USB_Example_PrintError("USB device connection timeout");
    return USB_STATUS_TIMEOUT;
}

/**
 * @brief USB Connect Callback
 * @retval None
 */
void USB_Example_ConnectCallback(void)
{
    USB_Example_PrintStatus("USB device connected");
    USB_Example_PrintDeviceInfo();
}

/**
 * @brief USB Disconnect Callback
 * @retval None
 */
void USB_Example_DisconnectCallback(void)
{
    USB_Example_PrintStatus("USB device disconnected");
}

/**
 * @brief USB Data Received Callback
 * @param data: Pointer to received data
 * @param length: Data length
 * @retval None
 */
void USB_Example_DataReceivedCallback(const uint8_t *data, uint16_t length)
{
    if (length > 0 && length <= USB_EXAMPLE_BUFFER_SIZE)
    {
        memcpy(usb_example_buffer, data, length);
        received_data_length = length;
        data_received_flag = 1;
    }
}

/**
 * @brief USB Transmit Complete Callback
 * @retval None
 */
void USB_Example_TransmitCompleteCallback(void)
{
    transmit_complete_flag = 1;
}

/**
 * @brief Delay function
 * @param delay_ms: Delay in milliseconds
 * @retval None
 */
void USB_Example_Delay(uint32_t delay_ms)
{
    HAL_Delay(delay_ms);
}

/**
 * @brief Print status message
 * @param message: Status message to print
 * @retval None
 */
void USB_Example_PrintStatus(const char *message)
{
    /* This would typically print to UART or LCD */
    /* For now, it's a placeholder */
    UNUSED(message);

    /* Example: printf("[USB] %s\r\n", message); */
}

/**
 * @brief Print error message
 * @param error_message: Error message to print
 * @retval None
 */
void USB_Example_PrintError(const char *error_message)
{
    /* This would typically print to UART or LCD */
    /* For now, it's a placeholder */
    UNUSED(error_message);

    /* Example: printf("[USB ERROR] %s\r\n", error_message); */
}

/**
 * @}
 */

/**
 * @}
 */
