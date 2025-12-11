/**
  ******************************************************************************
  * @file    accel.h
  * @brief   MMA8452Q Accelerometer driver interface
  * @details This file contains function prototypes and definitions for
  *          the MMA8452Q 3-axis digital accelerometer.
  * @version 1.0
  * @date    2025-09-01
  ******************************************************************************
  */

#ifndef __ACCEL_H__
#define __ACCEL_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Accelerometer status enumeration
 */
typedef enum {
    ACCEL_OK = 0,           /**< Operation completed successfully */
    ACCEL_ERROR,            /**< General error occurred */
    ACCEL_BUSY,             /**< Accelerometer is busy */
    ACCEL_TIMEOUT,          /**< Operation timed out */
    ACCEL_INVALID_PARAM,    /**< Invalid parameter provided */
    ACCEL_NOT_READY         /**< Device not ready */
} ACCEL_StatusTypeDef;

/**
 * @brief Accelerometer configuration structure
 */
typedef struct {
    uint8_t DataRate;       /**< Output data rate */
    uint8_t Range;          /**< Measurement range (±2g, ±4g, ±8g) */
    uint8_t Mode;           /**< Operating mode */
    bool HighPassFilter;    /**< High-pass filter enable */
    bool LowNoise;          /**< Low noise mode enable */
} ACCEL_ConfigTypeDef;

/**
 * @brief Accelerometer data structure
 */
typedef struct {
    int16_t X;              /**< X-axis acceleration (raw) */
    int16_t Y;              /**< Y-axis acceleration (raw) */
    int16_t Z;              /**< Z-axis acceleration (raw) */
    float X_g;              /**< X-axis acceleration in g */
    float Y_g;              /**< Y-axis acceleration in g */
    float Z_g;              /**< Z-axis acceleration in g */
} ACCEL_DataTypeDef;

/**
 * @brief Interrupt configuration structure
 */
typedef struct {
    bool DataReady;         /**< Data ready interrupt */
    bool Motion;            /**< Motion detection interrupt */
    bool Freefall;          /**< Freefall detection interrupt */
    bool Tap;               /**< Tap detection interrupt */
} ACCEL_IntConfigTypeDef;

/* Exported constants --------------------------------------------------------*/

/** @defgroup MMA8452Q_Registers MMA8452Q Register Addresses
 * @{
 */
#define ACCEL_REG_STATUS          0x00    /**< Data status register */
#define ACCEL_REG_OUT_X_MSB       0x01    /**< X-axis MSB */
#define ACCEL_REG_OUT_X_LSB       0x02    /**< X-axis LSB */
#define ACCEL_REG_OUT_Y_MSB       0x03    /**< Y-axis MSB */
#define ACCEL_REG_OUT_Y_LSB       0x04    /**< Y-axis LSB */
#define ACCEL_REG_OUT_Z_MSB       0x05    /**< Z-axis MSB */
#define ACCEL_REG_OUT_Z_LSB       0x06    /**< Z-axis LSB */
#define ACCEL_REG_F_SETUP         0x09    /**< FIFO setup register */
#define ACCEL_REG_TRIG_CFG        0x0A    /**< Trigger configuration */
#define ACCEL_REG_SYSMOD          0x0B    /**< System mode register */
#define ACCEL_REG_INT_SOURCE      0x0C    /**< Interrupt source register */
#define ACCEL_REG_WHO_AM_I        0x0D    /**< Device ID register */
#define ACCEL_REG_XYZ_DATA_CFG    0x0E    /**< Data configuration register */
#define ACCEL_REG_HP_FILTER_CUTOFF 0x0F   /**< High-pass filter register */
#define ACCEL_REG_PL_STATUS       0x10    /**< Portrait/Landscape status */
#define ACCEL_REG_PL_CFG          0x11    /**< Portrait/Landscape config */
#define ACCEL_REG_PL_COUNT        0x12    /**< Portrait/Landscape debounce */
#define ACCEL_REG_PL_BF_ZCOMP     0x13    /**< Back/front Z compensation */
#define ACCEL_REG_P_L_THS_REG     0x14    /**< Portrait/Landscape threshold */
#define ACCEL_REG_FF_MT_CFG       0x15    /**< Freefall/Motion configuration */
#define ACCEL_REG_FF_MT_SRC       0x16    /**< Freefall/Motion source */
#define ACCEL_REG_FF_MT_THS       0x17    /**< Freefall/Motion threshold */
#define ACCEL_REG_FF_MT_COUNT     0x18    /**< Freefall/Motion debounce */
#define ACCEL_REG_TRANSIENT_CFG   0x1D    /**< Transient configuration */
#define ACCEL_REG_TRANSIENT_SRC   0x1E    /**< Transient source */
#define ACCEL_REG_TRANSIENT_THS   0x1F    /**< Transient threshold */
#define ACCEL_REG_TRANSIENT_COUNT 0x20    /**< Transient debounce */
#define ACCEL_REG_PULSE_CFG       0x21    /**< Pulse configuration */
#define ACCEL_REG_PULSE_SRC       0x22    /**< Pulse source */
#define ACCEL_REG_PULSE_THSX      0x23    /**< Pulse X threshold */
#define ACCEL_REG_PULSE_THSY      0x24    /**< Pulse Y threshold */
#define ACCEL_REG_PULSE_THSZ      0x25    /**< Pulse Z threshold */
#define ACCEL_REG_PULSE_TMLT      0x26    /**< Pulse time limit */
#define ACCEL_REG_PULSE_LTCY      0x27    /**< Pulse latency */
#define ACCEL_REG_PULSE_WIND      0x28    /**< Pulse window */
#define ACCEL_REG_ASLP_COUNT      0x29    /**< Auto-sleep counter */
#define ACCEL_REG_CTRL_REG1       0x2A    /**< Control register 1 */
#define ACCEL_REG_CTRL_REG2       0x2B    /**< Control register 2 */
#define ACCEL_REG_CTRL_REG3       0x2C    /**< Control register 3 */
#define ACCEL_REG_CTRL_REG4       0x2D    /**< Control register 4 */
#define ACCEL_REG_CTRL_REG5       0x2E    /**< Control register 5 */
#define ACCEL_REG_OFF_X           0x2F    /**< X-axis offset */
#define ACCEL_REG_OFF_Y           0x30    /**< Y-axis offset */
#define ACCEL_REG_OFF_Z           0x31    /**< Z-axis offset */
/** @} */

/** @defgroup MMA8452Q_Device_Constants Device Constants
 * @{
 */
#define ACCEL_DEVICE_ID          0x2A    /**< MMA8452Q device ID */
#define ACCEL_I2C_ADDRESS        0x1D    /**< 7-bit I2C address */
#define ACCEL_SPI_READ_CMD       0x80    /**< SPI read command prefix */
#define ACCEL_SPI_WRITE_CMD      0x00    /**< SPI write command prefix */
/** @} */

/** @defgroup MMA8452Q_Data_Rates Output Data Rates
 * @{
 */
#define ACCEL_ODR_800HZ          0x00    /**< 800 Hz */
#define ACCEL_ODR_400HZ          0x01    /**< 400 Hz */
#define ACCEL_ODR_200HZ          0x02    /**< 200 Hz */
#define ACCEL_ODR_100HZ          0x03    /**< 100 Hz */
#define ACCEL_ODR_50HZ           0x04    /**< 50 Hz */
#define ACCEL_ODR_12_5HZ         0x05    /**< 12.5 Hz */
#define ACCEL_ODR_6_25HZ         0x06    /**< 6.25 Hz */
#define ACCEL_ODR_1_56HZ         0x07    /**< 1.56 Hz */
/** @} */

/** @defgroup MMA8452Q_Ranges Measurement Ranges
 * @{
 */
#define ACCEL_RANGE_2G           0x00    /**< ±2g range */
#define ACCEL_RANGE_4G           0x01    /**< ±4g range */
#define ACCEL_RANGE_8G           0x02    /**< ±8g range */
/** @} */

/** @defgroup MMA8452Q_Modes Operating Modes
 * @{
 */
#define ACCEL_MODE_STANDBY       0x00    /**< Standby mode */
#define ACCEL_MODE_ACTIVE        0x01    /**< Active mode */
#define ACCEL_MODE_SLEEP         0x02    /**< Sleep mode */
/** @} */

/** @defgroup MMA8452Q_Timeouts Timeout Values
 * @{
 */
#define ACCEL_TIMEOUT_DEFAULT    1000U   /**< Default timeout in milliseconds */
#define ACCEL_TIMEOUT_INIT       5000U   /**< Initialization timeout */
/** @} */

/* Exported functions prototypes ---------------------------------------------*/

/** @defgroup ACCEL_Init_Config Initialization and Configuration
 * @{
 */

/**
 * @brief   Initialize the MMA8452Q accelerometer
 * @details Configures the accelerometer with default settings
 * @param   None
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_Init(void);

/**
 * @brief   Initialize accelerometer with custom configuration
 * @details Allows custom configuration of accelerometer parameters
 * @param   config Pointer to accelerometer configuration structure
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_Init_Custom(const ACCEL_ConfigTypeDef* config);

/**
 * @brief   Deinitialize the accelerometer
 * @details Puts the accelerometer in standby mode
 * @param   None
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_DeInit(void);

/** @} */

/** @defgroup ACCEL_Data_Operations Data Operations
 * @{
 */

/**
 * @brief   Read acceleration data from all axes
 * @details Reads raw and converted acceleration data
 * @param   data Pointer to data structure to store results
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_ReadData(ACCEL_DataTypeDef* data);

/**
 * @brief   Read raw acceleration data
 * @details Reads 14-bit raw data from accelerometer
 * @param   xAxis Pointer to store X-axis data
 * @param   yAxis Pointer to store Y-axis data
 * @param   zAxis Pointer to store Z-axis data
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_ReadRawData(int16_t* xAxis, int16_t* yAxis, int16_t* zAxis);

/**
 * @brief   Convert raw data to g-force
 * @details Converts 14-bit raw data to acceleration in g
 * @param   raw Raw 14-bit acceleration data
 * @param   range Measurement range (2, 4, or 8g)
 * @retval  float Acceleration in g
 */
float ACCEL_ConvertToG(int16_t raw, uint8_t range);

/** @} */

/** @defgroup ACCEL_Device_Management Device Management
 * @{
 */

/**
 * @brief   Check if accelerometer is ready
 * @details Verifies device communication and ID
 * @param   None
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_IsReady(void);

/**
 * @brief   Get device ID
 * @details Reads the WHO_AM_I register
 * @param   deviceId Pointer to store device ID
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_GetDeviceID(uint8_t* deviceId);

/**
 * @brief   Set operating mode
 * @details Changes between standby, active, and sleep modes
 * @param   mode Operating mode
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_SetMode(uint8_t mode);

/**
 * @brief   Get current operating mode
 * @details Reads the current system mode
 * @param   mode Pointer to store current mode
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_GetMode(uint8_t* mode);

/** @} */

/** @defgroup ACCEL_Configuration Configuration Functions
 * @{
 */

/**
 * @brief   Set output data rate
 * @details Configures the accelerometer sampling rate
 * @param   odr Output data rate
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_SetDataRate(uint8_t odr);

/**
 * @brief   Set measurement range
 * @details Configures the accelerometer measurement range
 * @param   range Measurement range (±2g, ±4g, ±8g)
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_SetRange(uint8_t range);

/**
 * @brief   Get current measurement range
 * @details Reads the current range setting
 * @param   range Pointer to store current range
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_GetRange(uint8_t* range);

/**
 * @brief   Enable/disable high-pass filter
 * @details Controls the high-pass filter for DC offset removal
 * @param   enable Enable (true) or disable (false)
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_EnableHighPassFilter(bool enable);

/**
 * @brief   Enable/disable low noise mode
 * @details Controls low noise mode for improved accuracy
 * @param   enable Enable (true) or disable (false)
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_EnableLowNoise(bool enable);

/** @} */

/** @defgroup ACCEL_Interrupt_Functions Interrupt Functions
 * @{
 */

/**
 * @brief   Configure interrupts
 * @details Sets up interrupt sources and routing
 * @param   config Pointer to interrupt configuration
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_ConfigInterrupts(const ACCEL_IntConfigTypeDef* config);

/**
 * @brief   Get interrupt source
 * @details Reads which interrupt sources are active
 * @param   intSource Pointer to store interrupt source flags
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_GetInterruptSource(uint8_t* intSource);

/** @} */

/** @defgroup ACCEL_Calibration Calibration Functions
 * @{
 */

/**
 * @brief   Calibrate accelerometer
 * @details Performs offset calibration for all axes
 * @param   None
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_Calibrate(void);

/**
 * @brief   Set manual offset
 * @details Sets manual offset values for calibration
 * @param   xOffset X-axis offset
 * @param   yOffset Y-axis offset
 * @param   zOffset Z-axis offset
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_SetOffset(int8_t xOffset, int8_t yOffset, int8_t zOffset);

/**
 * @brief   Get current offset values
 * @details Reads current offset calibration values
 * @param   xOffset Pointer to store X-axis offset
 * @param   yOffset Pointer to store Y-axis offset
 * @param   zOffset Pointer to store Z-axis offset
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_GetOffset(int8_t* xOffset, int8_t* yOffset, int8_t* zOffset);

/** @} */

/** @defgroup ACCEL_Utility_Functions Utility Functions
 * @{
 */

/**
 * @brief   Perform self-test
 * @details Runs built-in self-test function
 * @param   None
 * @retval  ACCEL_StatusTypeDef Operation status
 */
ACCEL_StatusTypeDef ACCEL_SelfTest(void);

/**
 * @brief   Get accelerometer status string
 * @details Converts status code to human-readable string
 * @param   status Accelerometer status code
 * @retval  const char* Status description string
 */
const char* ACCEL_GetStatusString(ACCEL_StatusTypeDef status);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __ACCEL_H__ */
