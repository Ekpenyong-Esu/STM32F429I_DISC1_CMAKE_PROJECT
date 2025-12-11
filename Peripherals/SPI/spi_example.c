/**
  ******************************************************************************
  * @file    spi_example.c
  * @brief   SPI module usage examples
  * @details This file provides practical examples demonstrating how to use
  *          the SPI driver functions for various communication scenarios.
  *          Includes examples for sensor communication, display control,
  *          and data transfer operations.
  * @version 1.0
  * @date    2025-04-15
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "spi.h"
#include <stdio.h>
#include <string.h>

/* Private defines -----------------------------------------------------------*/

/**
 * @brief Example device commands (common SPI devices)
 */
#define SPI_FLASH_READ_CMD       0x03    /**< SPI Flash read command */
#define SPI_FLASH_WRITE_CMD      0x02    /**< SPI Flash write command */
#define SPI_FLASH_STATUS_CMD     0x05    /**< SPI Flash status register read */
#define SPI_ACCEL_READ_CMD       0x80    /**< Accelerometer read command */
#define SPI_ACCEL_WRITE_CMD      0x00    /**< Accelerometer write command */
#define SPI_DISPLAY_CMD          0x40    /**< Display command prefix */

/**
 * @brief SPI Flash memory addresses for example
 */
#define SPI_FLASH_TEST_ADDR      0x00000000  /**< Test memory location */

/**
 * @brief Example data sizes and constants
 */
#define TEST_DATA_SIZE           16      /**< Size of test data buffer */
#define SPI_CLOCK_SPEED_HIGH     10000000U  /**< High speed: 10 MHz */
#define SPI_CLOCK_SPEED_MEDIUM   5000000U   /**< Medium speed: 5 MHz */
#define SPI_CLOCK_SPEED_LOW      1000000U   /**< Low speed: 1 MHz */

/**
 * @brief SPI Flash operation constants
 */
#define SPI_FLASH_ADDR_BYTE_MASK 0xFFU   /**< Byte mask for address extraction */
#define SPI_FLASH_ADDR_SHIFT_16  16U     /**< Shift for MSB address byte */
#define SPI_FLASH_ADDR_SHIFT_8   8U      /**< Shift for middle address byte */
#define SPI_FLASH_WRITE_DELAY_MS 10U     /**< Write operation delay */
#define SPI_CRC_POLYNOMIAL_DEFAULT 10U   /**< Default CRC polynomial value */

/**
 * @brief Accelerometer register constants
 */
#define SPI_ACCEL_CTRL_REG1      0x20U   /**< Control register 1 address */
#define SPI_ACCEL_CONFIG_ENABLE  0x47U   /**< Enable X, Y, Z axes, 50Hz */

/**
 * @brief Test data constants
 */
#define TEST_DATA_AA             0xAAU
#define TEST_DATA_BB             0xBBU
#define TEST_DATA_CC             0xCCU
#define TEST_DATA_DD             0xDDU

/**
 * @brief Test data pattern
 */
#define TEST_DATA_00             0x00
#define TEST_DATA_11             0x11
#define TEST_DATA_22             0x22
#define TEST_DATA_33             0x33
#define TEST_DATA_44             0x44
#define TEST_DATA_55             0x55
#define TEST_DATA_66             0x66
#define TEST_DATA_77             0x77
#define TEST_DATA_88             0x88
#define TEST_DATA_99             0x99
#define TEST_DATA_EE             0xEE
#define TEST_DATA_FF             0xFF

/* Private variables ---------------------------------------------------------*/
static uint8_t testData[TEST_DATA_SIZE] = {
    TEST_DATA_00, TEST_DATA_11, TEST_DATA_22, TEST_DATA_33,
    TEST_DATA_44, TEST_DATA_55, TEST_DATA_66, TEST_DATA_77,
    TEST_DATA_88, TEST_DATA_99, TEST_DATA_AA, TEST_DATA_BB,
    TEST_DATA_CC, TEST_DATA_DD, TEST_DATA_EE, TEST_DATA_FF
};

/* Private function prototypes -----------------------------------------------*/
static void SPI_Example_BasicTransmitReceive(void);
static void SPI_Example_FlashMemoryOperations(void);
static void SPI_Example_SensorCommunication(void);
static void SPI_Example_ErrorHandling(void);
static void SPI_Example_CustomConfig(void);

/* Public functions ----------------------------------------------------------*/

/**
 * @brief   Run all SPI examples
 * @details Demonstrates various SPI communication scenarios
 * @param   None
 * @retval  None
 */
void SPI_RunExamples(void)
{
    printf("=== SPI Driver Examples ===\n\r");

    /* Initialize SPI peripheral */
    SPI_Init();
    printf("SPI initialized successfully\n\r");

    /* Run individual examples */
    SPI_Example_BasicTransmitReceive();
    SPI_Example_FlashMemoryOperations();
    SPI_Example_SensorCommunication();
    SPI_Example_ErrorHandling();
    SPI_Example_CustomConfig();

    printf("=== All SPI examples completed ===\n\r");
}

/**
 * @brief   Basic transmit and receive example
 * @details Demonstrates simple SPI master transmit and receive operations
 * @param   None
 * @retval  None
 */
static void SPI_Example_BasicTransmitReceive(void)
{
    printf("\n--- Basic Transmit/Receive Example ---\n\r");

    uint8_t txData[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    uint8_t rxData[8] = {0};
    SPI_StatusTypeDef status = SPI_OK;

    /* Transmit data */
    status = SPI_Transmit(txData, sizeof(txData), SPI_TIMEOUT_DEFAULT);
    if (status == SPI_OK)
    {
        printf("Data transmitted successfully: ");
        for (size_t i = 0; i < sizeof(txData); i++)
        {
            printf("0x%02X ", txData[i]);
        }
        printf("\n\r");
    }
    else
    {
        printf("Transmit failed: %s\n\r", SPI_GetStatusString(status));
    }

    /* Receive data */
    status = SPI_Receive(rxData, sizeof(rxData), SPI_TIMEOUT_DEFAULT);
    if (status == SPI_OK)
    {
        printf("Data received: ");
        for (size_t i = 0; i < sizeof(rxData); i++)
        {
            printf("0x%02X ", rxData[i]);
        }
        printf("\n\r");
    }
    else
    {
        printf("Receive failed: %s\n\r", SPI_GetStatusString(status));
    }

    /* Transmit and receive simultaneously */
    uint8_t cmd = SPI_FLASH_STATUS_CMD;  /* Status register read command */
    uint8_t response = 0;

    status = SPI_TransmitReceive(&cmd, &response, 1, SPI_TIMEOUT_DEFAULT);
    if (status == SPI_OK)
    {
        printf("TransmitReceive successful: CMD=0x%02X, Response=0x%02X\n\r", cmd, response);
    }
    else
    {
        printf("TransmitReceive failed: %s\n\r", SPI_GetStatusString(status));
    }
}

/**
 * @brief   Flash memory operations example
 * @details Demonstrates SPI flash memory read/write operations
 * @param   None
 * @retval  None
 */
static void SPI_Example_FlashMemoryOperations(void)
{
    printf("\n--- Flash Memory Operations Example ---\n\r");

    uint8_t readData[TEST_DATA_SIZE] = {0};
    SPI_StatusTypeDef status = SPI_OK;

    /* Write enable command */
    uint8_t writeEnableCmd = 0x06;
    status = SPI_Transmit(&writeEnableCmd, 1, SPI_TIMEOUT_DEFAULT);
    if (status != SPI_OK)
    {
        printf("Write enable failed: %s\n\r", SPI_GetStatusString(status));
        return;
    }

    /* Write data to flash */
    uint8_t writeCmd[4 + TEST_DATA_SIZE];
    writeCmd[0] = SPI_FLASH_WRITE_CMD;
    writeCmd[1] = (SPI_FLASH_TEST_ADDR >> SPI_FLASH_ADDR_SHIFT_16) & SPI_FLASH_ADDR_BYTE_MASK;
    writeCmd[2] = (SPI_FLASH_TEST_ADDR >> SPI_FLASH_ADDR_SHIFT_8) & SPI_FLASH_ADDR_BYTE_MASK;
    writeCmd[3] = SPI_FLASH_TEST_ADDR & SPI_FLASH_ADDR_BYTE_MASK;
    memcpy(&writeCmd[4], testData, TEST_DATA_SIZE);

    status = SPI_Transmit(writeCmd, sizeof(writeCmd), SPI_TIMEOUT_LONG);
    if (status == SPI_OK)
    {
        printf("Data written to flash at address 0x%06X\n\r", SPI_FLASH_TEST_ADDR);
    }
    else
    {
        printf("Flash write failed: %s\n\r", SPI_GetStatusString(status));
        return;
    }

    /* Wait for write completion (simplified) */
    HAL_Delay(SPI_FLASH_WRITE_DELAY_MS);

    /* Read data from flash */
    uint8_t readCmd[4] = {
        SPI_FLASH_READ_CMD,
        (SPI_FLASH_TEST_ADDR >> SPI_FLASH_ADDR_SHIFT_16) & SPI_FLASH_ADDR_BYTE_MASK,
        (SPI_FLASH_TEST_ADDR >> SPI_FLASH_ADDR_SHIFT_8) & SPI_FLASH_ADDR_BYTE_MASK,
        SPI_FLASH_TEST_ADDR & SPI_FLASH_ADDR_BYTE_MASK
    };

    status = SPI_Transmit(readCmd, sizeof(readCmd), SPI_TIMEOUT_DEFAULT);
    if (status != SPI_OK)
    {
        printf("Read command failed: %s\n\r", SPI_GetStatusString(status));
        return;
    }

    status = SPI_Receive(readData, sizeof(readData), SPI_TIMEOUT_DEFAULT);
    if (status == SPI_OK)
    {
        printf("Data read from flash: ");
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
        printf("Flash read failed: %s\n\r", SPI_GetStatusString(status));
    }
}

/**
 * @brief   Sensor communication example
 * @details Demonstrates SPI communication with sensors
 * @param   None
 * @retval  None
 */
static void SPI_Example_SensorCommunication(void)
{
    printf("\n--- Sensor Communication Example ---\n\r");

    uint8_t txBuffer[2];
    uint8_t rxBuffer[2];
    SPI_StatusTypeDef status = SPI_OK;

    /* Read accelerometer X-axis data */
    txBuffer[0] = SPI_ACCEL_READ_CMD | 0x01;  /* Read X-axis register */
    txBuffer[1] = 0x00;  /* Dummy byte for read */

    status = SPI_TransmitReceive(txBuffer, rxBuffer, 2, SPI_TIMEOUT_DEFAULT);
    if (status == SPI_OK)
    {
        printf("Accelerometer X-axis data: 0x%02X%02X\n\r", rxBuffer[0], rxBuffer[1]);
    }
    else
    {
        printf("Sensor read failed: %s\n\r", SPI_GetStatusString(status));
    }

    /* Write configuration to sensor */
    txBuffer[0] = SPI_ACCEL_WRITE_CMD | SPI_ACCEL_CTRL_REG1;  /* Write to CTRL_REG1 */
    txBuffer[1] = SPI_ACCEL_CONFIG_ENABLE;  /* Enable X, Y, Z axes, 50Hz */

    status = SPI_Transmit(txBuffer, 2, SPI_TIMEOUT_DEFAULT);
    if (status == SPI_OK)
    {
        printf("Sensor configuration written successfully\n\r");
    }
    else
    {
        printf("Sensor write failed: %s\n\r", SPI_GetStatusString(status));
    }
}

/**
 * @brief   Error handling example
 * @details Demonstrates proper error handling and recovery
 * @param   None
 * @retval  None
 */
static void SPI_Example_ErrorHandling(void)
{
    printf("\n--- Error Handling Example ---\n\r");

    uint8_t data[4] = {0};
    SPI_StatusTypeDef status = SPI_OK;

    /* Test with invalid parameters */
    printf("Testing with invalid parameters...\n\r");
    status = SPI_Transmit(NULL, 4, SPI_TIMEOUT_DEFAULT);
    if (status == SPI_INVALID_PARAM)
    {
        printf("Invalid parameter correctly detected: %s\n\r", SPI_GetStatusString(status));
    }

    /* Test timeout scenario (if device not responding) */
    printf("Testing timeout scenario...\n\r");
    status = SPI_Receive(data, sizeof(data), SPI_TIMEOUT_SHORT);
    if (status == SPI_TIMEOUT)
    {
        printf("Timeout correctly detected: %s\n\r", SPI_GetStatusString(status));

        /* Get detailed error information */
        uint32_t errorCode = SPI_GetError();
        printf("HAL Error Code: 0x%08X\n\r", (unsigned int)errorCode);
    }
    else if (status == SPI_OK)
    {
        printf("Receive completed (device responded)\n\r");
    }
    else
    {
        printf("Unexpected error: %s\n\r", SPI_GetStatusString(status));
    }
}

/**
 * @brief   Custom configuration example
 * @details Demonstrates custom SPI configuration
 * @param   None
 * @retval  None
 */
static void SPI_Example_CustomConfig(void)
{
    printf("\n--- Custom Configuration Example ---\n\r");

    /* Define custom SPI configuration for high-speed operation */
    SPI_ConfigTypeDef customConfig = {
        .Mode = SPI_MODE_MASTER,                          /* Master mode */
        .Direction = SPI_DIRECTION_2LINES,                /* Full-duplex */
        .DataSize = SPI_DATASIZE_8BIT,                    /* 8-bit data */
        .CLKPolarity = SPI_POLARITY_LOW,                  /* CPOL = 0 */
        .CLKPhase = SPI_PHASE_1EDGE,                      /* CPHA = 0 */
        .NSS = SPI_NSS_SOFT,                              /* Software NSS */
        .BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4,     /* High speed */
        .FirstBit = SPI_FIRSTBIT_MSB,                     /* MSB first */
        .TIMode = SPI_TIMODE_DISABLE,                     /* TI mode disabled */
        .CRCCalculation = SPI_CRCCALCULATION_DISABLE,     /* CRC disabled */
        .CRCPolynomial = SPI_CRC_POLYNOMIAL_DEFAULT                               /* CRC polynomial */
    };

    /* Apply custom configuration */
    SPI_StatusTypeDef status = SPI_Init_Custom(&customConfig);
    if (status == SPI_OK)
    {
        printf("Custom SPI configuration applied successfully\n\r");
        printf("  - Baud Rate Prescaler: %d\n\r", customConfig.BaudRatePrescaler);
        printf("  - Clock Polarity: %s\n\r",
               (customConfig.CLKPolarity == SPI_POLARITY_LOW) ? "Low" : "High");
        printf("  - Clock Phase: %s\n\r",
               (customConfig.CLKPhase == SPI_PHASE_1EDGE) ? "1 Edge" : "2 Edge");
    }
    else
    {
        printf("Custom configuration failed: %s\n\r", SPI_GetStatusString(status));
    }

    /* Test with custom configuration */
    uint8_t testData[4] = {TEST_DATA_AA, TEST_DATA_BB, TEST_DATA_CC, TEST_DATA_DD};
    status = SPI_Transmit(testData, sizeof(testData), SPI_TIMEOUT_DEFAULT);
    if (status == SPI_OK)
    {
        printf("High-speed transmission successful\n\r");
    }
    else
    {
        printf("High-speed transmission failed: %s\n\r", SPI_GetStatusString(status));
    }

    /* Reinitialize with default settings */
    SPI_Init();
    printf("Reverted to default SPI configuration\n\r");
}
