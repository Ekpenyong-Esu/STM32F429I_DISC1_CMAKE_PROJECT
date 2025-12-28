/**
 * @file gyro.h
 * @brief GYRO driver interface for L3GD20 gyroscope
 * @details This file contains all the function prototypes for
 *          the L3GD20 gyroscope sensor configuration and control.
 *          It provides APIs to initialize and read gyroscope data
 *          on the STM32F429 Discovery board.
 * @version 1.0
 * @date 2025-09-03
 * @author STM32 Team
 */

#ifndef GYRO_H
#define GYRO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported constants --------------------------------------------------------*/
#define GYRO_SPI_TIMEOUT              1000

/* L3GD20 Register Addresses */
#define L3GD20_WHO_AM_I_ADDR          0x0F
#define L3GD20_CTRL_REG1_ADDR         0x20
#define L3GD20_CTRL_REG2_ADDR         0x21
#define L3GD20_CTRL_REG3_ADDR         0x22
#define L3GD20_CTRL_REG4_ADDR         0x23
#define L3GD20_CTRL_REG5_ADDR         0x24
#define L3GD20_REFERENCE_ADDR         0x25
#define L3GD20_OUT_TEMP_ADDR          0x26
#define L3GD20_STATUS_REG_ADDR        0x27
#define L3GD20_OUT_X_L_ADDR           0x28
#define L3GD20_OUT_X_H_ADDR           0x29
#define L3GD20_OUT_Y_L_ADDR           0x2A
#define L3GD20_OUT_Y_H_ADDR           0x2B
#define L3GD20_OUT_Z_L_ADDR           0x2C
#define L3GD20_OUT_Z_H_ADDR           0x2D
#define L3GD20_FIFO_CTRL_REG_ADDR     0x2E
#define L3GD20_FIFO_SRC_REG_ADDR      0x2F
#define L3GD20_INT1_CFG_ADDR          0x30
#define L3GD20_INT1_SRC_ADDR          0x31
#define L3GD20_INT1_THS_XH_ADDR       0x32
#define L3GD20_INT1_THS_XL_ADDR       0x33
#define L3GD20_INT1_THS_YH_ADDR       0x34
#define L3GD20_INT1_THS_YL_ADDR       0x35
#define L3GD20_INT1_THS_ZH_ADDR       0x36
#define L3GD20_INT1_THS_ZL_ADDR       0x37
#define L3GD20_INT1_DURATION_ADDR     0x38

/* L3GD20 Device IDs */
#define L3GD20_DEVICE_ID              0xD4    /* Original L3GD20 */
#define L3GD20H_DEVICE_ID             0xD7    /* L3GD20H variant */
#define L3G4200D_DEVICE_ID            0xD3    /* L3G4200D variant */

/* SPI Read/Write Commands */
#define L3GD20_SPI_READ               0x80
#define L3GD20_SPI_WRITE              0x00
#define L3GD20_SPI_MULTIPLEBYTE       0x40

/* Exported types ------------------------------------------------------------*/

/**
 * @brief GYRO Configuration Structure
 */
typedef struct {
    uint8_t powerMode;          /* Power mode (Normal/Sleep/Power-down) */
    uint8_t outputDataRate;     /* Output data rate */
    uint8_t axes;               /* X, Y, Z axes enable */
    uint8_t bandwidth;          /* Bandwidth selection */
    uint8_t fullScale;          /* Full scale selection */
    uint8_t blockDataUpdate;    /* Block data update */
    uint8_t endianness;         /* Big/Little endian data selection */
} GYRO_Config_t;

/**
 * @brief GYRO Data Structure
 */
typedef struct {
    int16_t x;                  /* X-axis angular velocity */
    int16_t y;                  /* Y-axis angular velocity */
    int16_t z;                  /* Z-axis angular velocity */
    int8_t temperature;         /* Temperature reading */
} GYRO_Data_t;

/**
 * @brief GYRO Handle Structure
 */
typedef struct {
    SPI_HandleTypeDef *hspi;    /* SPI handle */
    GPIO_TypeDef *csPort;       /* Chip select port */
    uint16_t csPin;             /* Chip select pin */
    GYRO_Config_t config;       /* Configuration */
    bool initialized;           /* Initialization status */
    uint32_t errorCode;         /* Last error code */
} GYRO_Handle_t;

/* GYRO Configuration Constants */
/* Power Mode */
#define GYRO_POWER_DOWN           0x00
#define GYRO_SLEEP                0x08
#define GYRO_NORMAL               0x0F
#define GYRO_POWER_NORMAL         0x0F  /* Alias for compatibility */

/* Output Data Rate */
#define GYRO_ODR_95HZ             0x00
#define GYRO_ODR_190HZ            0x40
#define GYRO_ODR_380HZ            0x80
#define GYRO_ODR_760HZ            0xC0
#define GYRO_ODR_95HZ_BW_12_5     0x00  /* 95Hz with 12.5Hz bandwidth */
#define GYRO_ODR_190HZ_BW_25      0x40  /* 190Hz with 25Hz bandwidth */
#define GYRO_ODR_380HZ_BW_50      0x80  /* 380Hz with 50Hz bandwidth */
#define GYRO_ODR_760HZ_BW_100     0xC0  /* 760Hz with 100Hz bandwidth */

/* Axes Enable */
#define GYRO_X_ENABLE             0x01
#define GYRO_Y_ENABLE             0x02
#define GYRO_Z_ENABLE             0x04
#define GYRO_XYZ_ENABLE           0x07

/* Bandwidth */
#define GYRO_BANDWIDTH_0          0x00
#define GYRO_BANDWIDTH_1          0x10
#define GYRO_BANDWIDTH_2          0x20
#define GYRO_BANDWIDTH_3          0x30

/* Full Scale */
#define GYRO_FULLSCALE_250        0x00
#define GYRO_FULLSCALE_500        0x10
#define GYRO_FULLSCALE_2000       0x20

/* Block Data Update */
#define GYRO_BDU_CONTINUOUS       0x00
#define GYRO_BDU_MSBLSB           0x80

/* Endianness */
#define GYRO_LITTLE_ENDIAN        0x00
#define GYRO_BIG_ENDIAN           0x40
#define GYRO_BLE_MSB              0x40  /* Alias for big endian */

/* Error Codes */
#define GYRO_ERROR_NONE           0x00
#define GYRO_ERROR_INIT           0x01
#define GYRO_ERROR_SPI            0x02
#define GYRO_ERROR_DEVICE_ID      0x03
#define GYRO_ERROR_CONFIG         0x04
#define GYRO_ERROR_READ           0x05
#define GYRO_ERROR_WRITE          0x06

/* Function Prototypes -------------------------------------------------------*/

/**
 * @brief Initialize GYRO sensor
 * @param handle: Pointer to GYRO handle structure
 * @param hspi: Pointer to SPI handle
 * @param csPort: Chip select GPIO port
 * @param csPin: Chip select GPIO pin
 * @param config: Pointer to configuration structure
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_Init(GYRO_Handle_t *handle, SPI_HandleTypeDef *hspi,
                           GPIO_TypeDef *csPort, uint16_t csPin,
                           GYRO_Config_t *config);

/**
 * @brief Deinitialize GYRO sensor
 * @param handle: Pointer to GYRO handle structure
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_DeInit(GYRO_Handle_t *handle);

/**
 * @brief Read gyroscope data
 * @param handle: Pointer to GYRO handle structure
 * @param data: Pointer to data structure
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_ReadData(GYRO_Handle_t *handle, GYRO_Data_t *data);

/**
 * @brief Read single axis data
 * @param handle: Pointer to GYRO handle structure
 * @param axis: Axis to read (0=X, 1=Y, 2=Z)
 * @param value: Pointer to store the value
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_ReadAxis(GYRO_Handle_t *handle, uint8_t axis, int16_t *value);

/**
 * @brief Read temperature
 * @param handle: Pointer to GYRO handle structure
 * @param temperature: Pointer to store temperature value
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_ReadTemperature(GYRO_Handle_t *handle, int8_t *temperature);

/**
 * @brief Check if new data is available
 * @param handle: Pointer to GYRO handle structure
 * @param dataReady: Pointer to store data ready status
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_IsDataReady(GYRO_Handle_t *handle, bool *dataReady);

/**
 * @brief Get device ID
 * @param handle: Pointer to GYRO handle structure
 * @param deviceId: Pointer to store device ID
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_GetDeviceID(GYRO_Handle_t *handle, uint8_t *deviceId);

/**
 * @brief Configure interrupt
 * @param handle: Pointer to GYRO handle structure
 * @param config: Interrupt configuration
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_ConfigInterrupt(GYRO_Handle_t *handle, uint8_t config);

/**
 * @brief Set full scale range
 * @param handle: Pointer to GYRO handle structure
 * @param fullScale: Full scale value
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_SetFullScale(GYRO_Handle_t *handle, uint8_t fullScale);

/**
 * @brief Set output data rate
 * @param handle: Pointer to GYRO handle structure
 * @param dataRate: Output data rate value
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_SetOutputDataRate(GYRO_Handle_t *handle, uint8_t dataRate);

/**
 * @brief Enable/disable axes
 * @param handle: Pointer to GYRO handle structure
 * @param axes: Axes to enable/disable
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GYRO_SetAxes(GYRO_Handle_t *handle, uint8_t axes);

/**
 * @brief Convert raw data to angular velocity (dps)
 * @param rawValue: Raw sensor value
 * @param fullScale: Current full scale setting
 * @return float: Angular velocity in degrees per second
 */
float GYRO_ConvertToDPS(int16_t rawValue, uint8_t fullScale);

/**
 * @brief Get last error code
 * @param handle: Pointer to GYRO handle structure
 * @return uint32_t: Error code
 */
uint32_t GYRO_GetError(GYRO_Handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* GYRO_H */
