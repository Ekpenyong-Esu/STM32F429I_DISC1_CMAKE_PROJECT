/**
  ******************************************************************************
  * @file    i2c_example.c
  * @brief   I2C module usage examples
  * @details This file provides practical examples demonstrating how to use
  *          the I2C driver functions for various communication scenarios.
  *          Includes examples for sensor communication, EEPROM access,
  *          and device scanning.
  * @version 1.0
  * @date    2025-04-15
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "i2c.h"
#include <stdio.h>
#include <string.h>

/* Private defines -----------------------------------------------------------*/

/**
 * @brief Example device addresses (common I2C devices)
 */
#define EEPROM_ADDR         0x50    /**< 24LC256 EEPROM address */
#define TEMP_SENSOR_ADDR    0x48    /**< TMP117 temperature sensor address */
#define ACCEL_ADDR          0x1D    /**< MMA8452Q accelerometer address */
#define INVALID_DEVICE_ADDR 0xFF    /**< Invalid device address for testing */

/**
 * @brief EEPROM memory addresses for example
 */
#define EEPROM_TEST_ADDR    0x0000  /**< Test memory location */

/**
 * @brief Example data sizes and constants
 */
#define TEST_DATA_SIZE      16      /**< Size of test data buffer */
#define MAX_DEVICES         16      /**< Maximum devices to scan for */
#define EEPROM_WRITE_DELAY  10      /**< EEPROM write cycle delay in ms */
#define I2C_CLOCK_SPEED_FAST 400000U /**< Fast mode: 400 kHz */

/**
 * @brief Test data pattern
 */
#define TEST_DATA_08        0x08
#define TEST_DATA_09        0x09
#define TEST_DATA_0A        0x0A
#define TEST_DATA_0B        0x0B
#define TEST_DATA_0C        0x0C
#define TEST_DATA_0D        0x0D
#define TEST_DATA_0E        0x0E
#define TEST_DATA_0F        0x0F

/* Private variables ---------------------------------------------------------*/
static uint8_t testData[TEST_DATA_SIZE] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    TEST_DATA_08, TEST_DATA_09, TEST_DATA_0A, TEST_DATA_0B,
    TEST_DATA_0C, TEST_DATA_0D, TEST_DATA_0E, TEST_DATA_0F
};

/* Private function prototypes -----------------------------------------------*/
static void I2C_Example_BasicTransmitReceive(void);
static void I2C_Example_MemoryOperations(void);
static void I2C_Example_DeviceScanning(void);
static void I2C_Example_ErrorHandling(void);
static void I2C_Example_CustomConfig(void);

/* Public functions ----------------------------------------------------------*/

/**
 * @brief   Run all I2C examples
 * @details Demonstrates various I2C communication scenarios
 * @param   None
 * @retval  None
 */
void I2C_RunExamples(void)
{
    printf("=== I2C Driver Examples ===\n\r");

    /* Initialize I2C peripheral */
    I2C_Init();
    printf("I2C initialized successfully\n\r");

    /* Run individual examples */
    I2C_Example_BasicTransmitReceive();
    I2C_Example_MemoryOperations();
    I2C_Example_DeviceScanning();
    I2C_Example_ErrorHandling();
    I2C_Example_CustomConfig();

    printf("=== All I2C examples completed ===\n\r");
}

/**
 * @brief   Basic transmit and receive example
 * @details Demonstrates simple I2C master transmit and receive operations
 * @param   None
 * @retval  None
 */
static void I2C_Example_BasicTransmitReceive(void)
{
    printf("\n--- Basic Transmit/Receive Example ---\n\r");

    uint8_t txData[4] = {0x01, 0x02, 0x03, 0x04};
    uint8_t rxData[4] = {0};
    I2C_StatusTypeDef status = I2C_OK;

    /* Transmit data to a device */
    status = I2C_Master_Transmit(TEMP_SENSOR_ADDR << 1, txData, sizeof(txData), I2C_TIMEOUT_DEFAULT);
    if (status == I2C_OK)
    {
        printf("Data transmitted successfully to device 0x%02X\n\r", TEMP_SENSOR_ADDR);
    }
    else
    {
        printf("Transmit failed: %s\n\r", I2C_GetStatusString(status));
    }

    /* Receive data from a device */
    status = I2C_Master_Receive(TEMP_SENSOR_ADDR << 1, rxData, sizeof(rxData), I2C_TIMEOUT_DEFAULT);
    if (status == I2C_OK)
    {
        printf("Data received from device 0x%02X: ", TEMP_SENSOR_ADDR);
        for (size_t i = 0; i < sizeof(rxData); i++)
        {
            printf("0x%02X ", rxData[i]);
        }
        printf("\n\r");
    }
    else
    {
        printf("Receive failed: %s\n\r", I2C_GetStatusString(status));
    }

    /* Transmit and receive in single transaction */
    uint8_t cmd = 0x00;  /* Command to send */
    uint8_t response[2] = {0};

    status = I2C_Master_TransmitReceive(TEMP_SENSOR_ADDR << 1, &cmd, 1, response, 2, I2C_TIMEOUT_DEFAULT);
    if (status == I2C_OK)
    {
        printf("TransmitReceive successful: CMD=0x%02X, Response=0x%02X%02X\n\r", cmd, response[0], response[1]);
    }
    else
    {
        printf("TransmitReceive failed: %s\n\r", I2C_GetStatusString(status));
    }
}

/**
 * @brief   Memory operations example
 * @details Demonstrates I2C memory read/write operations (EEPROM)
 * @param   None
 * @retval  None
 */
static void I2C_Example_MemoryOperations(void)
{
    printf("\n--- Memory Operations Example ---\n\r");

    uint8_t readData[TEST_DATA_SIZE] = {0};
    I2C_StatusTypeDef status = I2C_OK;

    /* Write data to EEPROM */
    status = I2C_Mem_Write(EEPROM_ADDR << 1, EEPROM_TEST_ADDR, I2C_MEMADD_SIZE_16BIT,
                          testData, sizeof(testData), I2C_TIMEOUT_LONG);
    if (status == I2C_OK)
    {
        printf("Data written to EEPROM at address 0x%04X\n\r", EEPROM_TEST_ADDR);
    }
    else
    {
        printf("EEPROM write failed: %s\n\r", I2C_GetStatusString(status));
        return;
    }

    /* Wait for write cycle to complete (EEPROM specific) */
    HAL_Delay(EEPROM_WRITE_DELAY);

    /* Read data from EEPROM */
    status = I2C_Mem_Read(EEPROM_ADDR << 1, EEPROM_TEST_ADDR, I2C_MEMADD_SIZE_16BIT,
                         readData, sizeof(readData), I2C_TIMEOUT_DEFAULT);
    if (status == I2C_OK)
    {
        printf("Data read from EEPROM: ");
        for (size_t i = 0; i < sizeof(readData); i++)
        {
            printf("0x%02X ", readData[i]);
        }
        printf("\n\r");

        /* Verify data integrity */
        if (memcmp(testData, readData, sizeof(testData)) == 0)
        {
            printf("Data verification successful!\n\r");
        }
        else
        {
            printf("Data verification failed!\n\r");
        }
    }
    else
    {
        printf("EEPROM read failed: %s\n\r", I2C_GetStatusString(status));
    }
}

/**
 * @brief   Device scanning example
 * @details Scans the I2C bus for connected devices
 * @param   None
 * @retval  None
 */
static void I2C_Example_DeviceScanning(void)
{
    printf("\n--- Device Scanning Example ---\n\r");

    uint8_t devices[MAX_DEVICES] = {0};  /* Array to store found device addresses */
    uint8_t deviceCount = 0;

    printf("Scanning I2C bus for devices...\n\r");

    /* Scan for devices */
    deviceCount = I2C_ScanBus(devices, sizeof(devices), I2C_TIMEOUT_SHORT);

    if (deviceCount > 0)
    {
        printf("Found %d device(s):\n\r", deviceCount);
        for (uint8_t i = 0; i < deviceCount; i++)
        {
            printf("  - Device at address 0x%02X\n\r", devices[i]);
        }
    }
    else
    {
        printf("No devices found on I2C bus\n\r");
    }

    /* Test specific device readiness */
    I2C_StatusTypeDef status = I2C_IsDeviceReady(TEMP_SENSOR_ADDR << 1, 3, I2C_TIMEOUT_DEFAULT);
    if (status == I2C_OK)
    {
        printf("Temperature sensor (0x%02X) is ready\n\r", TEMP_SENSOR_ADDR);
    }
    else
    {
        printf("Temperature sensor (0x%02X) not responding: %s\n\r",
               TEMP_SENSOR_ADDR, I2C_GetStatusString(status));
    }
}

/**
 * @brief   Error handling example
 * @details Demonstrates proper error handling and recovery
 * @param   None
 * @retval  None
 */
static void I2C_Example_ErrorHandling(void)
{
    printf("\n--- Error Handling Example ---\n\r");

    uint8_t data[4] = {0};
    I2C_StatusTypeDef status = I2C_OK;

    /* Try to communicate with non-existent device */
    printf("Testing communication with non-existent device...\n\r");
    status = I2C_Master_Receive(INVALID_DEVICE_ADDR, data, sizeof(data), I2C_TIMEOUT_SHORT);

    if (status != I2C_OK)
    {
        printf("Expected error occurred: %s\n\r", I2C_GetStatusString(status));

        /* Get detailed error information */
        uint32_t errorCode = I2C_GetError();
        printf("HAL Error Code: 0x%08X\n\r", (unsigned int)errorCode);
    }

    /* Test with invalid parameters */
    printf("Testing with invalid parameters...\n\r");
    status = I2C_Master_Transmit(EEPROM_ADDR << 1, NULL, 4, I2C_TIMEOUT_DEFAULT);
    if (status == I2C_INVALID_PARAM)
    {
        printf("Invalid parameter correctly detected: %s\n\r", I2C_GetStatusString(status));
    }
}

/**
 * @brief   Custom configuration example
 * @details Demonstrates custom I2C configuration
 * @param   None
 * @retval  None
 */
static void I2C_Example_CustomConfig(void)
{
    printf("\n--- Custom Configuration Example ---\n\r");

    /* Define custom I2C configuration */
    I2C_ConfigTypeDef customConfig = {
        .ClockSpeed = I2C_CLOCK_SPEED_FAST,            /* 400 kHz (fast mode) */
        .DutyCycle = I2C_DUTYCYCLE_16_9,               /* 16:9 duty cycle for fast mode */
        .AddressingMode = I2C_ADDRESSINGMODE_7BIT,     /* 7-bit addressing */
        .OwnAddress1 = 0x00,                           /* Master mode */
        .DualAddressMode = I2C_DUALADDRESS_DISABLE,    /* Dual addressing disabled */
        .OwnAddress2 = 0x00,                           /* Not used */
        .GeneralCallMode = I2C_GENERALCALL_DISABLE,    /* General call disabled */
        .NoStretchMode = I2C_NOSTRETCH_DISABLE         /* Clock stretching enabled */
    };

    /* Apply custom configuration */
    I2C_StatusTypeDef status = I2C_Init_Custom(&customConfig);
    if (status == I2C_OK)
    {
        printf("Custom I2C configuration applied successfully\n\r");
        printf("  - Clock Speed: %d Hz\n\r", (int)customConfig.ClockSpeed);
        printf("  - Duty Cycle: %s\n\r",
               (customConfig.DutyCycle == I2C_DUTYCYCLE_2) ? "2:1" : "16:9");
    }
    else
    {
        printf("Custom configuration failed: %s\n\r", I2C_GetStatusString(status));
    }

    /* Reinitialize with default settings */
    I2C_Init();
    printf("Reverted to default I2C configuration\n\r");
}
