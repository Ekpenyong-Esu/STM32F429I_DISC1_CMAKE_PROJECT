/**
  ******************************************************************************
  * @file    mems.h
  * @brief   MEMS sensors driver interface for STM32F429 Discovery Board
  * @details This file contains function prototypes and definitions for
  *          MEMS (Micro-Electro-Mechanical Systems) sensors on the
  *          STM32F429 Discovery board, specifically the L3GD20 gyroscope.
  * @version 1.0
  * @date    2025-09-04
  ******************************************************************************
  */

#ifndef __MEMS_H__
#define __MEMS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/* Exported constants --------------------------------------------------------*/
#define MEMS_SPI_TIMEOUT                1000
#define MEMS_MAX_RETRIES                3

/* L3GD20 Gyroscope Constants */
#define L3GD20_WHO_AM_I_VALUE           0xD4
#define L3GD20_SENSITIVITY_250DPS       8.75f    /* mdps/LSB */
#define L3GD20_SENSITIVITY_500DPS       17.50f   /* mdps/LSB */
#define L3GD20_SENSITIVITY_2000DPS      70.0f    /* mdps/LSB */

/* GPIO Pin Definitions */
#define MEMS_CS_PIN                     GPIO_PIN_1
#define MEMS_CS_GPIO_PORT               GPIOC
#define MEMS_INT1_PIN                   GPIO_PIN_1
#define MEMS_INT1_GPIO_PORT             GPIOA
#define MEMS_INT2_PIN                   GPIO_PIN_2
#define MEMS_INT2_GPIO_PORT             GPIOA

/* SPI Definitions */
#define MEMS_SPI                        SPI5
#define MEMS_SPI_CLK_ENABLE()           __HAL_RCC_SPI5_CLK_ENABLE()
#define MEMS_SPI_SCK_GPIO_PORT          GPIOF
#define MEMS_SPI_SCK_PIN                GPIO_PIN_7
#define MEMS_SPI_MISO_GPIO_PORT         GPIOF
#define MEMS_SPI_MISO_PIN               GPIO_PIN_8
#define MEMS_SPI_MOSI_GPIO_PORT         GPIOF
#define MEMS_SPI_MOSI_PIN               GPIO_PIN_9

/* L3GD20 Register Addresses */
#define L3GD20_WHO_AM_I_ADDR            0x0F
#define L3GD20_CTRL_REG1_ADDR           0x20
#define L3GD20_CTRL_REG2_ADDR           0x21
#define L3GD20_CTRL_REG3_ADDR           0x22
#define L3GD20_CTRL_REG4_ADDR           0x23
#define L3GD20_CTRL_REG5_ADDR           0x24
#define L3GD20_REFERENCE_ADDR           0x25
#define L3GD20_OUT_TEMP_ADDR            0x26
#define L3GD20_STATUS_REG_ADDR          0x27
#define L3GD20_OUT_X_L_ADDR             0x28
#define L3GD20_OUT_X_H_ADDR             0x29
#define L3GD20_OUT_Y_L_ADDR             0x2A
#define L3GD20_OUT_Y_H_ADDR             0x2B
#define L3GD20_OUT_Z_L_ADDR             0x2C
#define L3GD20_OUT_Z_H_ADDR             0x2D
#define L3GD20_FIFO_CTRL_REG_ADDR       0x2E
#define L3GD20_FIFO_SRC_REG_ADDR        0x2F
#define L3GD20_INT1_CFG_ADDR            0x30
#define L3GD20_INT1_SRC_ADDR            0x31
#define L3GD20_INT1_THS_XH_ADDR         0x32
#define L3GD20_INT1_THS_XL_ADDR         0x33
#define L3GD20_INT1_THS_YH_ADDR         0x34
#define L3GD20_INT1_THS_YL_ADDR         0x35
#define L3GD20_INT1_THS_ZH_ADDR         0x36
#define L3GD20_INT1_THS_ZL_ADDR         0x37
#define L3GD20_INT1_DURATION_ADDR       0x38

/* L3GD20 Control Register Values */
#define L3GD20_POWER_DOWN               0x00
#define L3GD20_NORMAL_MODE              0x08
#define L3GD20_ODR_95Hz                 0x00
#define L3GD20_ODR_190Hz                0x40
#define L3GD20_ODR_380Hz                0x80
#define L3GD20_ODR_760Hz                0xC0
#define L3GD20_AXES_ENABLE              0x07
#define L3GD20_BANDWIDTH_1              0x00
#define L3GD20_BANDWIDTH_2              0x10
#define L3GD20_BANDWIDTH_3              0x20
#define L3GD20_BANDWIDTH_4              0x30

#define L3GD20_FULLSCALE_250            0x00
#define L3GD20_FULLSCALE_500            0x10
#define L3GD20_FULLSCALE_2000           0x20
#define L3GD20_BLE_LSB                  0x00
#define L3GD20_BLE_MSB                  0x40

/* Temperature offset used when converting raw temp to degrees C */
#define MEMS_TEMPERATURE_OFFSET        25.0f

/* SPI Communication Constants */
#define L3GD20_READ_CMD                 0x80
#define L3GD20_MULTIPLEBYTE_CMD         0x40

/* Exported types ------------------------------------------------------------*/

/**
 * @brief MEMS status enumeration
 */
typedef enum {
    MEMS_OK = 0,                /**< Operation completed successfully */
    MEMS_ERROR,                 /**< General error occurred */
    MEMS_BUSY,                  /**< MEMS device is busy */
    MEMS_TIMEOUT,               /**< Operation timed out */
    MEMS_INVALID_PARAM,         /**< Invalid parameter provided */
    MEMS_NOT_INITIALIZED,       /**< Device not initialized */
    MEMS_COMMUNICATION_ERROR,   /**< SPI communication error */
    MEMS_DEVICE_NOT_FOUND      /**< Device not detected */
} MEMS_StatusTypeDef;

/**
 * @brief MEMS sensor types
 */
typedef enum {
    MEMS_SENSOR_GYROSCOPE = 0,  /**< Gyroscope sensor */
    MEMS_SENSOR_ACCELEROMETER,  /**< Accelerometer sensor */
    MEMS_SENSOR_MAGNETOMETER,   /**< Magnetometer sensor */
    MEMS_SENSOR_ALL            /**< All sensors */
} MEMS_SensorTypeDef;

/**
 * @brief Gyroscope output data rate
 */
typedef enum {
    MEMS_GYRO_ODR_95Hz = 0,     /**< 95 Hz output data rate */
    MEMS_GYRO_ODR_190Hz,        /**< 190 Hz output data rate */
    MEMS_GYRO_ODR_380Hz,        /**< 380 Hz output data rate */
    MEMS_GYRO_ODR_760Hz         /**< 760 Hz output data rate */
} MEMS_GyroODRTypeDef;

/**
 * @brief Gyroscope full scale range
 */
typedef enum {
    MEMS_GYRO_FULLSCALE_250 = 0, /**< ±250 dps full scale */
    MEMS_GYRO_FULLSCALE_500,      /**< ±500 dps full scale */
    MEMS_GYRO_FULLSCALE_2000      /**< ±2000 dps full scale */
} MEMS_GyroFullScaleTypeDef;

/**
 * @brief Gyroscope bandwidth
 */
typedef enum {
    MEMS_GYRO_BANDWIDTH_1 = 0,  /**< Bandwidth mode 1 */
    MEMS_GYRO_BANDWIDTH_2,      /**< Bandwidth mode 2 */
    MEMS_GYRO_BANDWIDTH_3,      /**< Bandwidth mode 3 */
    MEMS_GYRO_BANDWIDTH_4       /**< Bandwidth mode 4 */
} MEMS_GyroBandwidthTypeDef;

/**
 * @brief MEMS gyroscope configuration structure
 */
typedef struct {
    MEMS_GyroODRTypeDef OutputDataRate;        /**< Output data rate */
    MEMS_GyroFullScaleTypeDef FullScale;       /**< Full scale range */
    MEMS_GyroBandwidthTypeDef Bandwidth;       /**< Bandwidth selection */
    bool XAxisEnable;                          /**< X-axis enable */
    bool YAxisEnable;                          /**< Y-axis enable */
    bool ZAxisEnable;                          /**< Z-axis enable */
    bool PowerDownMode;                        /**< Power down mode */
} MEMS_GyroConfigTypeDef;

/**
 * @brief MEMS interrupt configuration structure
 */
typedef struct {
    bool InterruptEnable;                      /**< Enable interrupt */
    bool BootStatusEnable;                     /**< Boot status interrupt */
    bool ActiveLevel;                          /**< Active level (0=low, 1=high) */
    bool OutputType;                           /**< Output type (0=push-pull, 1=open-drain) */
    bool DataReadyEnable;                      /**< Data ready interrupt enable */
    bool WatermarkEnable;                      /**< FIFO watermark interrupt */
    bool OverrunEnable;                        /**< FIFO overrun interrupt */
    bool EmptyEnable;                          /**< FIFO empty interrupt */
} MEMS_InterruptConfigTypeDef;

/**
 * @brief 3-axis data structure
 */
typedef struct {
    int16_t X;                                 /**< X-axis data */
    int16_t Y;                                 /**< Y-axis data */
    int16_t Z;                                 /**< Z-axis data */
} MEMS_AxesRawTypeDef;

/**
 * @brief 3-axis float data structure
 */
typedef struct {
    float X;                                   /**< X-axis data in engineering units */
    float Y;                                   /**< Y-axis data in engineering units */
    float Z;                                   /**< Z-axis data in engineering units */
} MEMS_AxesTypeDef;

/**
 * @brief MEMS driver structure
 */
typedef struct {
    SPI_HandleTypeDef *hspi;                   /**< SPI handle */
    GPIO_TypeDef *CS_Port;                      /**< Optional chip-select port (external device) */
    uint16_t CS_Pin;                            /**< Optional chip-select pin (external device) */
    MEMS_GyroConfigTypeDef GyroConfig;         /**< Gyroscope configuration */
    MEMS_InterruptConfigTypeDef IntConfig;     /**< Interrupt configuration */
    bool IsInitialized;                        /**< Initialization status */
    bool IsCalibrated;                        /**< Calibration status */
    MEMS_AxesTypeDef CalibrationOffset;        /**< Calibration offset values */
    float Temperature;                         /**< Current temperature */
} MEMS_HandleTypeDef;

/**
 * @brief Set chip-select pin to use for this MEMS handle.
 * @note If not set, the board default `MEMS_CS_GPIO_PORT`/`MEMS_CS_PIN` will be used.
 */
void MEMS_SetCS(MEMS_HandleTypeDef *hmems, GPIO_TypeDef *csPort, uint16_t csPin);

/**
 * @brief MEMS device information structure
 */
typedef struct {
    uint8_t WhoAmI;                           /**< Device ID */
    const char *DeviceName;                   /**< Device name string */
    bool IsPresent;                           /**< Device presence flag */
    uint8_t Version;                          /**< Device version */
} MEMS_DeviceInfoTypeDef;

/**
 * @brief MEMS calibration structure
 */
typedef struct {
    MEMS_AxesTypeDef Offset;                  /**< Offset values */
    MEMS_AxesTypeDef Scale;                   /**< Scale factors */
    bool IsValid;                             /**< Calibration validity */
    uint32_t CalibrationTime;                 /**< Calibration timestamp */
} MEMS_CalibrationTypeDef;

/* Exported function prototypes ---------------------------------------------*/

/**
 * @brief Initialize MEMS sensors
 * @param hmems Pointer to MEMS handle structure
 * @param hspi Pointer to SPI handle
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_Init(MEMS_HandleTypeDef *hmems, SPI_HandleTypeDef *hspi);

/**
 * @brief Deinitialize MEMS sensors
 * @param hmems Pointer to MEMS handle structure
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_DeInit(MEMS_HandleTypeDef *hmems);

/**
 * @brief Configure gyroscope sensor
 * @param hmems Pointer to MEMS handle structure
 * @param config Pointer to gyroscope configuration structure
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_GyroConfig(MEMS_HandleTypeDef *hmems, MEMS_GyroConfigTypeDef *config);

/**
 * @brief Read gyroscope raw data
 * @param hmems Pointer to MEMS handle structure
 * @param axes Pointer to store raw axes data
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_GyroReadRaw(MEMS_HandleTypeDef *hmems, MEMS_AxesRawTypeDef *axes);

/**
 * @brief Read gyroscope data in engineering units (dps)
 * @param hmems Pointer to MEMS handle structure
 * @param axes Pointer to store axes data in dps
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_GyroRead(MEMS_HandleTypeDef *hmems, MEMS_AxesTypeDef *axes);

/**
 * @brief Read device temperature
 * @param hmems Pointer to MEMS handle structure
 * @param temperature Pointer to store temperature value
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_ReadTemperature(MEMS_HandleTypeDef *hmems, float *temperature);

/**
 * @brief Configure interrupt settings
 * @param hmems Pointer to MEMS handle structure
 * @param config Pointer to interrupt configuration structure
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_ConfigureInterrupt(MEMS_HandleTypeDef *hmems, MEMS_InterruptConfigTypeDef *config);

/**
 * @brief Calibrate gyroscope sensor
 * @param hmems Pointer to MEMS handle structure
 * @param samples Number of samples for calibration
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_CalibrateGyroscope(MEMS_HandleTypeDef *hmems, uint16_t samples);

/**
 * @brief Get device information
 * @param hmems Pointer to MEMS handle structure
 * @param info Pointer to store device information
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_GetDeviceInfo(MEMS_HandleTypeDef *hmems, MEMS_DeviceInfoTypeDef *info);

/**
 * @brief Read device status register
 * @param hmems Pointer to MEMS handle structure
 * @param status Pointer to store status value
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_ReadStatus(MEMS_HandleTypeDef *hmems, uint8_t *status);

/**
 * @brief Set gyroscope power mode
 * @param hmems Pointer to MEMS handle structure
 * @param power_down Power down mode (true = power down, false = normal)
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_SetPowerMode(MEMS_HandleTypeDef *hmems, bool power_down);

/**
 * @brief Reset device to default settings
 * @param hmems Pointer to MEMS handle structure
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_Reset(MEMS_HandleTypeDef *hmems);

/**
 * @brief Perform self-test
 * @param hmems Pointer to MEMS handle structure
 * @param result Pointer to store self-test result
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_SelfTest(MEMS_HandleTypeDef *hmems, bool *result);

/**
 * @brief Enable/disable specific axis
 * @param hmems Pointer to MEMS handle structure
 * @param axis_mask Axis mask (bit 0=X, bit 1=Y, bit 2=Z)
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_SetAxisEnable(MEMS_HandleTypeDef *hmems, uint8_t axis_mask);

/**
 * @brief Read multiple registers
 * @param hmems Pointer to MEMS handle structure
 * @param start_addr Starting register address
 * @param data Pointer to store read data
 * @param length Number of bytes to read
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_ReadRegisters(MEMS_HandleTypeDef *hmems, uint8_t start_addr,
                                     uint8_t *data, uint8_t length);

/**
 * @brief Write multiple registers
 * @param hmems Pointer to MEMS handle structure
 * @param start_addr Starting register address
 * @param data Pointer to data to write
 * @param length Number of bytes to write
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_WriteRegisters(MEMS_HandleTypeDef *hmems, uint8_t start_addr,
                                      uint8_t *data, uint8_t length);

/**
 * @brief Get current full scale setting
 * @param hmems Pointer to MEMS handle structure
 * @param full_scale Pointer to store current full scale
 * @retval MEMS_StatusTypeDef Status of the operation
 */
MEMS_StatusTypeDef MEMS_GetFullScale(MEMS_HandleTypeDef *hmems, MEMS_GyroFullScaleTypeDef *full_scale);

/**
 * @brief Convert raw data to engineering units
 * @param raw_data Raw sensor data
 * @param full_scale Current full scale setting
 * @retval float Converted value in dps
 */
float MEMS_ConvertToDPS(int16_t raw_data, MEMS_GyroFullScaleTypeDef full_scale);

#ifdef __cplusplus
}
#endif

#endif /* __MEMS_H__ */
