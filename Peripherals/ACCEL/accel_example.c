/**
  ******************************************************************************
  * @file    accel_example.c
  * @brief   MMA8452Q Accelerometer usage examples
  * @details This file provides practical examples demonstrating how to use
  *          the MMA8452Q accelerometer driver functions.
  * @version 1.0
  * @date    2025-09-01
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "accel.h"
#include "../SPI/spi.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <math.h>

/* Private defines -----------------------------------------------------------*/
#define EXAMPLE_DELAY_1S          1000U   /**< 1 second delay */
#define EXAMPLE_DELAY_500MS       500U    /**< 500ms delay */
#define EXAMPLE_DELAY_100MS       100U    /**< 100ms delay */
#define EXAMPLE_MANUAL_OFFSET_X   10      /**< Manual X offset for demo */
#define EXAMPLE_MANUAL_OFFSET_Y   -5      /**< Manual Y offset for demo */
#define EXAMPLE_MANUAL_OFFSET_Z   20      /**< Manual Z offset for demo */
#define EXAMPLE_MONITOR_COUNT     50U     /**< Number of monitoring iterations */

/* Private function prototypes -----------------------------------------------*/
static void ACCEL_Example_BasicRead(void);
static void ACCEL_Example_Configuration(void);
static void ACCEL_Example_Calibration(void);
static void ACCEL_Example_Interrupts(void);
static void ACCEL_Example_SelfTest(void);

/* Public functions ----------------------------------------------------------*/

/**
 * @brief   Run all accelerometer examples
 * @details Demonstrates various accelerometer operations
 * @param   None
 * @retval  None
 */
void ACCEL_RunExamples(void)
{
    printf("=== MMA8452Q Accelerometer Examples ===\n\r");

    /* Initialize SPI first (required for accelerometer communication) */
    SPI_Init();
    printf("SPI initialized for accelerometer communication\n\r");

    /* Run individual examples */
    ACCEL_Example_BasicRead();
    ACCEL_Example_Configuration();
    ACCEL_Example_Calibration();
    ACCEL_Example_Interrupts();
    ACCEL_Example_SelfTest();

    printf("=== All accelerometer examples completed ===\n\r");
}

/**
 * @brief   Basic accelerometer reading example
 * @details Demonstrates basic acceleration data reading
 * @param   None
 * @retval  None
 */
static void ACCEL_Example_BasicRead(void)
{
    printf("\n--- Basic Accelerometer Reading Example ---\n\r");

    ACCEL_StatusTypeDef status = ACCEL_OK;
    ACCEL_DataTypeDef accelData;

    /* Initialize accelerometer with default settings */
    status = ACCEL_Init();
    if (status != ACCEL_OK)
    {
        printf("Accelerometer initialization failed: %s\n\r", ACCEL_GetStatusString(status));
        return;
    }
    printf("Accelerometer initialized successfully\n\r");

    /* Read acceleration data multiple times */
    for (uint8_t i = 0; i < 5; i++)
    {
        status = ACCEL_ReadData(&accelData);
        if (status == ACCEL_OK)
        {
            printf("Sample %d:\n\r", i + 1);
            printf("  Raw: X=%d, Y=%d, Z=%d\n\r", accelData.X, accelData.Y, accelData.Z);
            printf("  G-force: X=%.3f, Y=%.3f, Z=%.3f\n\r",
                   accelData.X_g, accelData.Y_g, accelData.Z_g);

            /* Calculate total acceleration magnitude */
            float magnitude = sqrtf(accelData.X_g * accelData.X_g +
                                   accelData.Y_g * accelData.Y_g +
                                   accelData.Z_g * accelData.Z_g);
            printf("  Magnitude: %.3f g\n\r", magnitude);
        }
        else
        {
            printf("Failed to read accelerometer data: %s\n\r", ACCEL_GetStatusString(status));
        }

        /* Delay between readings */
        HAL_Delay(EXAMPLE_DELAY_1S);
    }
}

/**
 * @brief   Accelerometer configuration example
 * @details Demonstrates different configuration options
 * @param   None
 * @retval  None
 */
static void ACCEL_Example_Configuration(void)
{
    printf("\n--- Accelerometer Configuration Example ---\n\r");

    ACCEL_StatusTypeDef status = ACCEL_OK;
    ACCEL_ConfigTypeDef config;

    /* Configure for high-speed, 4g range operation */
    config.DataRate = ACCEL_ODR_400HZ;
    config.Range = ACCEL_RANGE_4G;
    config.Mode = ACCEL_MODE_ACTIVE;
    config.HighPassFilter = true;
    config.LowNoise = true;

    status = ACCEL_Init_Custom(&config);
    if (status != ACCEL_OK)
    {
        printf("Custom configuration failed: %s\n\r", ACCEL_GetStatusString(status));
        return;
    }
    printf("Configured for 400Hz, ±4g range, high-pass filter enabled\n\r");

    /* Test different data rates */
    uint8_t dataRates[] = {ACCEL_ODR_800HZ, ACCEL_ODR_200HZ, ACCEL_ODR_50HZ};
    const char* rateNames[] = {"800Hz", "200Hz", "50Hz"};

    for (uint8_t i = 0; i < 3; i++)
    {
        status = ACCEL_SetDataRate(dataRates[i]);
        if (status == ACCEL_OK)
        {
            printf("Data rate set to %s\n\r", rateNames[i]);
            HAL_Delay(EXAMPLE_DELAY_500MS);  /* Allow time to see the rate change */
        }
        else
        {
            printf("Failed to set data rate: %s\n\r", ACCEL_GetStatusString(status));
        }
    }

    /* Test different measurement ranges */
    uint8_t ranges[] = {ACCEL_RANGE_2G, ACCEL_RANGE_8G, ACCEL_RANGE_4G};
    const char* rangeNames[] = {"±2g", "±8g", "±4g"};

    for (uint8_t i = 0; i < 3; i++)
    {
        status = ACCEL_SetRange(ranges[i]);
        if (status == ACCEL_OK)
        {
            printf("Range set to %s\n\r", rangeNames[i]);

            /* Read a sample to show the range effect */
            ACCEL_DataTypeDef data;
            status = ACCEL_ReadData(&data);
            if (status == ACCEL_OK)
            {
                printf("  Sample at %s: X=%.3f, Y=%.3f, Z=%.3f g\n\r",
                       rangeNames[i], data.X_g, data.Y_g, data.Z_g);
            }
        }
        else
        {
            printf("Failed to set range: %s\n\r", ACCEL_GetStatusString(status));
        }

        HAL_Delay(EXAMPLE_DELAY_500MS);
    }
}

/**
 * @brief   Accelerometer calibration example
 * @details Demonstrates offset calibration
 * @param   None
 * @retval  None
 */
static void ACCEL_Example_Calibration(void)
{
    printf("\n--- Accelerometer Calibration Example ---\n\r");

    ACCEL_StatusTypeDef status = ACCEL_OK;
    ACCEL_DataTypeDef data;
    int8_t xOffset = 0;
    int8_t yOffset = 0;
    int8_t zOffset = 0;

    /* Read data before calibration */
    printf("Reading data before calibration...\n\r");
    status = ACCEL_ReadData(&data);
    if (status == ACCEL_OK)
    {
        printf("Before calibration: X=%.3f, Y=%.3f, Z=%.3f g\n\r",
               data.X_g, data.Y_g, data.Z_g);
    }

    /* Get current offset values */
    status = ACCEL_GetOffset(&xOffset, &yOffset, &zOffset);
    if (status == ACCEL_OK)
    {
        printf("Current offsets: X=%d, Y=%d, Z=%d\n\r", xOffset, yOffset, zOffset);
    }

    /* Perform automatic calibration */
    printf("Performing automatic calibration (keep sensor level)...\n\r");
    status = ACCEL_Calibrate();
    if (status == ACCEL_OK)
    {
        printf("Calibration completed successfully\n\r");

        /* Get new offset values */
        status = ACCEL_GetOffset(&xOffset, &yOffset, &zOffset);
        if (status == ACCEL_OK)
        {
            printf("New offsets: X=%d, Y=%d, Z=%d\n\r", xOffset, yOffset, zOffset);
        }

        /* Read data after calibration */
        HAL_Delay(100);
        status = ACCEL_ReadData(&data);
        if (status == ACCEL_OK)
        {
            printf("After calibration: X=%.3f, Y=%.3f, Z=%.3f g\n\r",
                   data.X_g, data.Y_g, data.Z_g);
        }
    }
    else
    {
        printf("Calibration failed: %s\n\r", ACCEL_GetStatusString(status));
    }

    /* Demonstrate manual offset setting */
    printf("Setting manual offsets...\n\r");
    status = ACCEL_SetOffset(EXAMPLE_MANUAL_OFFSET_X, EXAMPLE_MANUAL_OFFSET_Y, EXAMPLE_MANUAL_OFFSET_Z);
    if (status == ACCEL_OK)
    {
        printf("Manual offsets set successfully\n\r");
    }
    else
    {
        printf("Failed to set manual offsets: %s\n\r", ACCEL_GetStatusString(status));
    }
}

/**
 * @brief   Accelerometer interrupts example
 * @details Demonstrates interrupt configuration and handling
 * @param   None
 * @retval  None
 */
static void ACCEL_Example_Interrupts(void)
{
    printf("\n--- Accelerometer Interrupts Example ---\n\r");

    ACCEL_StatusTypeDef status = ACCEL_OK;
    ACCEL_IntConfigTypeDef intConfig = {
        .DataReady = true,
        .Motion = false,
        .Freefall = false,
        .Tap = true
    };

    /* Configure interrupts */
    status = ACCEL_ConfigInterrupts(&intConfig);
    if (status == ACCEL_OK)
    {
        printf("Interrupts configured: Data Ready and Tap detection enabled\n\r");
    }
    else
    {
        printf("Failed to configure interrupts: %s\n\r", ACCEL_GetStatusString(status));
        return;
    }

    /* Monitor interrupt sources for a short period */
    printf("Monitoring interrupt sources for 5 seconds...\n\r");
    for (uint8_t i = 0; i < EXAMPLE_MONITOR_COUNT; i++)
    {
        uint8_t intSource = 0;
        status = ACCEL_GetInterruptSource(&intSource);
        if (status == ACCEL_OK && intSource != 0)
        {
            printf("Interrupt detected: 0x%02X", intSource);
            if (intSource & 0x01) {
                printf(" (Data Ready)");
            }
            if (intSource & 0x08) {
                printf(" (Tap)");
            }
            printf("\n\r");
        }

        HAL_Delay(EXAMPLE_DELAY_100MS);
    }

    /* Disable interrupts */
    ACCEL_IntConfigTypeDef disableConfig = {0};
    status = ACCEL_ConfigInterrupts(&disableConfig);
    if (status == ACCEL_OK)
    {
        printf("Interrupts disabled\n\r");
    }
}

/**
 * @brief   Accelerometer self-test example
 * @details Demonstrates the built-in self-test functionality
 * @param   None
 * @retval  None
 */
static void ACCEL_Example_SelfTest(void)
{
    printf("\n--- Accelerometer Self-Test Example ---\n\r");

    ACCEL_StatusTypeDef status = ACCEL_OK;

    /* Perform self-test */
    printf("Performing accelerometer self-test...\n\r");
    status = ACCEL_SelfTest();

    if (status == ACCEL_OK)
    {
        printf("Self-test PASSED - accelerometer is functioning correctly\n\r");
    }
    else
    {
        printf("Self-test FAILED: %s\n\r", ACCEL_GetStatusString(status));
        printf("Possible issues:\n\r");
        printf("  - Faulty accelerometer\n\r");
        printf("  - Incorrect power supply\n\r");
        printf("  - Communication problems\n\r");
    }

    /* Get device information */
    uint8_t deviceId = 0;
    status = ACCEL_GetDeviceID(&deviceId);
    if (status == ACCEL_OK)
    {
        printf("Device ID: 0x%02X (expected: 0x%02X)\n\r", deviceId, ACCEL_DEVICE_ID);
    }

    /* Get current configuration */
    uint8_t currentRange = 0;
    uint8_t currentMode = 0;
    status = ACCEL_GetRange(&currentRange);
    if (status == ACCEL_OK)
    {
        const char* rangeStr = "Unknown";
        switch (currentRange)
        {
            case ACCEL_RANGE_2G: rangeStr = "±2g"; break;
            case ACCEL_RANGE_4G: rangeStr = "±4g"; break;
            case ACCEL_RANGE_8G: rangeStr = "±8g"; break;
            default: break;
        }
        printf("Current range: %s\n\r", rangeStr);
    }

    status = ACCEL_GetMode(&currentMode);
    if (status == ACCEL_OK)
    {
        const char* modeStr = "Unknown";
        switch (currentMode)
        {
            case ACCEL_MODE_STANDBY: modeStr = "Standby"; break;
            case ACCEL_MODE_ACTIVE: modeStr = "Active"; break;
            case ACCEL_MODE_SLEEP: modeStr = "Sleep"; break;
            default: break;
        }
        printf("Current mode: %s\n\r", modeStr);
    }
}
