/**
 * @file gyro_example.c
 * @brief GYRO driver usage examples for L3GD20 gyroscope
 * @details This file provides examples of how to use the GYRO driver
 *          for the L3GD20 gyroscope on STM32F429 Discovery board.
 * @version 1.0
 * @date 2025-09-03
 * @author STM32 Team
 */

/* Includes ------------------------------------------------------------------*/
#include "gyro_example.h"
#include "gyro.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>

/* Private variables ---------------------------------------------------------*/
static GYRO_Handle_t hgyro;
static SPI_HandleTypeDef hspi5; /* SPI5 is used for gyroscope on Discovery board */

/* Private function prototypes -----------------------------------------------*/
static void GYRO_Example_GPIO_Init(void);
static void GYRO_Example_SPI_Init(void);
static void GYRO_Example_PrintData(GYRO_Data_t *data);
static void GYRO_Example_PrintError(uint32_t error);

/**
 * @brief Basic GYRO initialization example
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_Example_Basic_Init(void) {
    printf("GYRO Basic Initialization Example\r\n");
    printf("==================================\r\n");

    /* Initialize GPIO and SPI */
    GYRO_Example_GPIO_Init();
    GYRO_Example_SPI_Init();

    /* Configure GYRO settings */
    GYRO_Config_t config = {
        .powerMode = GYRO_POWER_NORMAL,
        .outputDataRate = GYRO_ODR_95HZ_BW_12_5,
        .bandwidth = GYRO_BANDWIDTH_1,
        .fullScale = GYRO_FULLSCALE_250,
        .blockDataUpdate = GYRO_BDU_CONTINUOUS,
        .endianness = GYRO_BLE_MSB,
        .axes = GYRO_XYZ_ENABLE
    };

    /* Initialize GYRO */
    HAL_StatusTypeDef status = GYRO_Init(&hgyro, &hspi5, GPIOC, GPIO_PIN_1, &config);

    if (status == HAL_OK) {
        printf("GYRO initialized successfully!\r\n");

        /* Read device ID */
        uint8_t deviceId;
        if (GYRO_GetDeviceID(&hgyro, &deviceId) == HAL_OK) {
            printf("Device ID: 0x%02X\r\n", deviceId);
        }
    } else {
        printf("GYRO initialization failed!\r\n");
        GYRO_Example_PrintError(GYRO_GetError(&hgyro));
    }

    printf("\r\n");
    return status;
}

/**
 * @brief Single data read example
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_Example_Single_Read(void) {
    printf("GYRO Single Data Read Example\r\n");
    printf("=============================\r\n");

    GYRO_Data_t gyroData;
    HAL_StatusTypeDef status = GYRO_ReadData(&hgyro, &gyroData);

    if (status == HAL_OK) {
        printf("Gyroscope Data:\r\n");
        GYRO_Example_PrintData(&gyroData);
    } else {
        printf("Failed to read gyroscope data!\r\n");
        GYRO_Example_PrintError(GYRO_GetError(&hgyro));
    }

    printf("\r\n");
    return status;
}

/**
 * @brief Continuous data reading example
 * @param duration_ms: Duration in milliseconds
 * @param interval_ms: Reading interval in milliseconds
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_Example_Continuous_Read(uint32_t duration_ms, uint32_t interval_ms) {
    printf("GYRO Continuous Reading Example\r\n");
    printf("Duration: %u ms, Interval: %u ms\r\n", duration_ms, interval_ms);
    printf("===============================\r\n");

    uint32_t startTime = HAL_GetTick();
    uint32_t lastReadTime = 0;
    uint32_t readCount = 0;

    while ((HAL_GetTick() - startTime) < duration_ms) {
        uint32_t currentTime = HAL_GetTick();

        if ((currentTime - lastReadTime) >= interval_ms) {
            GYRO_Data_t gyroData;
            bool dataReady = false;

            /* Check if new data is available */
            if (GYRO_IsDataReady(&hgyro, &dataReady) == HAL_OK && dataReady) {
                if (GYRO_ReadData(&hgyro, &gyroData) == HAL_OK) {
                    printf("Read #%u: ", ++readCount);
                    GYRO_Example_PrintData(&gyroData);
                } else {
                    printf("Read error at #%u\r\n", readCount);
                }
            }

            lastReadTime = currentTime;
        }

        HAL_Delay(1); /* Small delay to prevent busy waiting */
    }

    printf("Continuous reading completed. Total reads: %u\r\n\r\n", readCount);
    return HAL_OK;
}

/**
 * @brief Individual axis reading example
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_Example_Individual_Axis(void) {
    printf("GYRO Individual Axis Reading Example\r\n");
    printf("====================================\r\n");

    int16_t x, y, z;

    /* Read X axis */
    if (GYRO_ReadAxis(&hgyro, 0, &x) == HAL_OK) {
        printf("X-axis: %d (%.2f dps)\r\n", x, GYRO_ConvertToDPS(x, GYRO_FULLSCALE_250));
    } else {
        printf("Failed to read X-axis\r\n");
    }

    /* Read Y axis */
    if (GYRO_ReadAxis(&hgyro, 1, &y) == HAL_OK) {
        printf("Y-axis: %d (%.2f dps)\r\n", y, GYRO_ConvertToDPS(y, GYRO_FULLSCALE_250));
    } else {
        printf("Failed to read Y-axis\r\n");
    }

    /* Read Z axis */
    if (GYRO_ReadAxis(&hgyro, 2, &z) == HAL_OK) {
        printf("Z-axis: %d (%.2f dps)\r\n", z, GYRO_ConvertToDPS(z, GYRO_FULLSCALE_250));
    } else {
        printf("Failed to read Z-axis\r\n");
    }

    printf("\r\n");
    return HAL_OK;
}

/**
 * @brief Temperature reading example
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_Example_Temperature(void) {
    printf("GYRO Temperature Reading Example\r\n");
    printf("================================\r\n");

    int8_t temperature;
    HAL_StatusTypeDef status = GYRO_ReadTemperature(&hgyro, &temperature);

    if (status == HAL_OK) {
        printf("Temperature: %d°C\r\n", temperature);
    } else {
        printf("Failed to read temperature!\r\n");
        GYRO_Example_PrintError(GYRO_GetError(&hgyro));
    }

    printf("\r\n");
    return status;
}

/**
 * @brief Full scale configuration example
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_Example_FullScale_Config(void) {
    printf("GYRO Full Scale Configuration Example\r\n");
    printf("=====================================\r\n");

    uint8_t fullScales[] = {GYRO_FULLSCALE_250, GYRO_FULLSCALE_500, GYRO_FULLSCALE_2000};
    const char* fullScaleNames[] = {"250 dps", "500 dps", "2000 dps"};

    for (int i = 0; i < 3; i++) {
        printf("Setting full scale to %s...\r\n", fullScaleNames[i]);

        if (GYRO_SetFullScale(&hgyro, fullScales[i]) == HAL_OK) {
            HAL_Delay(100); /* Allow sensor to settle */

            GYRO_Data_t gyroData;
            if (GYRO_ReadData(&hgyro, &gyroData) == HAL_OK) {
                printf("  Raw data: X=%d, Y=%d, Z=%d\r\n",
                       gyroData.x, gyroData.y, gyroData.z);
                printf("  Converted: X=%.2f, Y=%.2f, Z=%.2f dps\r\n",
                       GYRO_ConvertToDPS(gyroData.x, fullScales[i]),
                       GYRO_ConvertToDPS(gyroData.y, fullScales[i]),
                       GYRO_ConvertToDPS(gyroData.z, fullScales[i]));
            }
        } else {
            printf("  Failed to set full scale!\r\n");
        }
        printf("\r\n");
    }

    /* Reset to default */
    GYRO_SetFullScale(&hgyro, GYRO_FULLSCALE_250);

    return HAL_OK;
}

/**
 * @brief Data rate configuration example
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_Example_DataRate_Config(void) {
    printf("GYRO Data Rate Configuration Example\r\n");
    printf("====================================\r\n");

    uint8_t dataRates[] = {GYRO_ODR_95HZ_BW_12_5, GYRO_ODR_190HZ_BW_25,
                           GYRO_ODR_380HZ_BW_50, GYRO_ODR_760HZ_BW_100};
    const char* dataRateNames[] = {"95 Hz", "190 Hz", "380 Hz", "760 Hz"};

    for (int i = 0; i < 4; i++) {
        printf("Setting data rate to %s...\r\n", dataRateNames[i]);

        if (GYRO_SetOutputDataRate(&hgyro, dataRates[i]) == HAL_OK) {
            printf("  Data rate set successfully\r\n");

            /* Test data ready frequency */
            uint32_t startTime = HAL_GetTick();
            uint32_t readyCount = 0;

            while ((HAL_GetTick() - startTime) < 1000) { /* Test for 1 second */
                bool dataReady = false;
                if (GYRO_IsDataReady(&hgyro, &dataReady) == HAL_OK && dataReady) {
                    readyCount++;
                    HAL_Delay(1); /* Small delay to avoid multiple reads of same data */
                }
            }

            printf("  Measured rate: ~%u Hz\r\n", readyCount);
        } else {
            printf("  Failed to set data rate!\r\n");
        }
        printf("\r\n");
    }

    /* Reset to default */
    GYRO_SetOutputDataRate(&hgyro, GYRO_ODR_95HZ_BW_12_5);

    return HAL_OK;
}

/**
 * @brief Comprehensive test example
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_Example_Comprehensive_Test(void) {
    printf("GYRO Comprehensive Test\r\n");
    printf("=======================\r\n");

    /* Initialize */
    if (GYRO_Example_Basic_Init() != HAL_OK) {
        return HAL_ERROR;
    }

    /* Individual tests */
    GYRO_Example_Single_Read();
    GYRO_Example_Individual_Axis();
    GYRO_Example_Temperature();
    GYRO_Example_FullScale_Config();
    GYRO_Example_DataRate_Config();

    /* Continuous reading test */
    GYRO_Example_Continuous_Read(5000, 100); /* 5 seconds, 100ms interval */

    printf("Comprehensive test completed!\r\n");
    return HAL_OK;
}

/**
 * @brief Cleanup GYRO resources
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_Example_Cleanup(void) {
    printf("GYRO Cleanup\r\n");
    printf("============\r\n");

    HAL_StatusTypeDef status = GYRO_DeInit(&hgyro);

    if (status == HAL_OK) {
        printf("GYRO deinitialized successfully!\r\n");
    } else {
        printf("GYRO deinitialization failed!\r\n");
    }

    printf("\r\n");
    return status;
}

/* Private Functions ---------------------------------------------------------*/

/**
 * @brief Initialize GPIO for GYRO
 */
static void GYRO_Example_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIO clocks */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();

    /* Configure CS pin (PC1) */
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* Set CS high initially */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);

    /* Configure SPI pins */
    /* PF7: SPI5_SCK, PF8: SPI5_MISO, PF9: SPI5_MOSI */
    GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI5;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
}

/**
 * @brief Initialize SPI for GYRO
 */
static void GYRO_Example_SPI_Init(void) {
    /* Enable SPI5 clock */
    __HAL_RCC_SPI5_CLK_ENABLE();

    /* Configure SPI5 */
    hspi5.Instance = SPI5;
    hspi5.Init.Mode = SPI_MODE_MASTER;
    hspi5.Init.Direction = SPI_DIRECTION_2LINES;
    hspi5.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi5.Init.CLKPolarity = SPI_POLARITY_HIGH;
    hspi5.Init.CLKPhase = SPI_PHASE_2EDGE;
    hspi5.Init.NSS = SPI_NSS_SOFT;
    hspi5.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
    hspi5.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi5.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi5.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi5.Init.CRCPolynomial = 10;

    HAL_SPI_Init(&hspi5);
}

/**
 * @brief Print gyroscope data
 * @param data: Pointer to gyroscope data
 */
static void GYRO_Example_PrintData(GYRO_Data_t *data) {
    printf("X=%6d Y=%6d Z=%6d (raw) | ", data->x, data->y, data->z);
    printf("X=%7.2f Y=%7.2f Z=%7.2f (dps)\r\n",
           GYRO_ConvertToDPS(data->x, GYRO_FULLSCALE_250),
           GYRO_ConvertToDPS(data->y, GYRO_FULLSCALE_250),
           GYRO_ConvertToDPS(data->z, GYRO_FULLSCALE_250));
}

/**
 * @brief Print error information
 * @param error: Error code
 */
static void GYRO_Example_PrintError(uint32_t error) {
    printf("Error code: 0x%08X - ", error);

    switch (error) {
        case GYRO_ERROR_NONE:
            printf("No error\r\n");
            break;
        case GYRO_ERROR_INIT:
            printf("Initialization error\r\n");
            break;
        case GYRO_ERROR_SPI:
            printf("SPI communication error\r\n");
            break;
        case GYRO_ERROR_DEVICE_ID:
            printf("Device ID mismatch\r\n");
            break;
        case GYRO_ERROR_CONFIG:
            printf("Configuration error\r\n");
            break;
        case GYRO_ERROR_READ:
            printf("Read operation error\r\n");
            break;
        case GYRO_ERROR_WRITE:
            printf("Write operation error\r\n");
            break;
        default:
            printf("Unknown error\r\n");
            break;
    }
}
