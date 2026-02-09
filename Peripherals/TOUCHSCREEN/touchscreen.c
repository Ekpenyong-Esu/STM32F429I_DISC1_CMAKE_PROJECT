/**
  ******************************************************************************
  * @file    touchscreen.c
  * @brief   STMPE811 Touchscreen driver implementation for STM32F429 Discovery Board
  * @details This file provides the implementation of touchscreen functions
  *          for the STMPE811 resistive touchscreen controller.
  * @version 1.0
  * @date    2025-09-04
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "touchscreen.h"
#include "i2c.h"
#include "stdbool.h"
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "log.h"
#include "app_low_power.h"  /* For touch activity handling */
#include "stm32f4xx_hal_gpio.h"

/* Private constants ---------------------------------------------------------*/
#define TS_DELAY_MS(x)                  HAL_Delay(x)
#define TS_GESTURE_THRESHOLD            20      /* Minimum movement for gesture */
#define TS_LONG_PRESS_TIME              1000    /* Minimum time for long press (ms) */
#define TS_PRESSURE_THRESHOLD           2      /* Default pressure threshold */
#define TS_DEFAULT_MIN_COORD            200     /* Default minimum coordinate */
#define TS_DEFAULT_MAX_COORD            3900    /* Default maximum coordinate */
#define TS_ADC_CTRL_12BIT               0x49    /* 12-bit ADC configuration (BSP value) */

/* Pressure scaling */
#define TS_PRESSURE_MAX_VALUE           0xFFU
#define TS_PRESSURE_SCALE               255U
#define TS_BYTE_MASK                    0xFFU

/* Measured raw bounds (replace with your actual measured values).
   NOTE: On this panel smaller raw X corresponds to the RIGHT side of the display,
   and smaller raw Y corresponds to the BOTTOM of the display (hence mappings are inverted). */
#define TS_RAW_X_MIN 301   // observed small raw X (right edge)
#define TS_RAW_X_MAX 3796  // observed large raw X (left edge)
#define TS_RAW_Y_MIN 151   // observed small raw Y (bottom edge)
#define TS_RAW_Y_MAX 3605  // observed large raw Y (top edge)


/* Private variables ---------------------------------------------------------*/
/* Global touchscreen handle */
TS_HandleTypeDef *g_hts = NULL;
/* Deferred EXTI handling flag to avoid I2C use in ISR */
static volatile bool s_ts_irq_pending = false;

/* Private function prototypes -----------------------------------------------*/

static TS_StatusTypeDef TS_InitI2C(TS_HandleTypeDef *hts);
static TS_StatusTypeDef TS_CheckDevice(TS_HandleTypeDef *hts);
static TS_StatusTypeDef TS_ConfigureController(TS_HandleTypeDef *hts);
static TS_StatusTypeDef TS_ReadRawCoordinates(uint16_t *raw_x,
                                              uint16_t *raw_y,
                                              uint16_t *pressure);

static TS_StatusTypeDef TS_ConvertCoordinates(uint16_t raw_x,
                                              uint16_t raw_y,
                                              uint16_t *disp_x,
                                              uint16_t *disp_y);
static void TS_FilterCoordinates(uint16_t *x,
                                 uint16_t *y);
static TS_GestureTypeDef TS_AnalyzeGesture(TS_HandleTypeDef *hts);

 static int32_t map(int32_t val,
                   int32_t in_min, int32_t in_max,
                   int32_t out_min, int32_t out_max);

/* Helper: integer-safe linear mapping with input clamping. Handles inverted output ranges. */
static int32_t map(int32_t val,
                   int32_t in_min, int32_t in_max,
                   int32_t out_min, int32_t out_max)
{
    if (in_max == in_min) return out_min; /* avoid divide by zero */

    /* Clamp input to expected raw range */
    if (val < in_min) val = in_min;
    if (val > in_max) val = in_max;

    int64_t in_range = (int64_t)(in_max - in_min);
    int64_t out_range = (int64_t)(out_max - out_min);

    int64_t scaled = (int64_t)(val - in_min) * out_range;
    int32_t result = (int32_t)(scaled / in_range + out_min);

    return result;
}

/* Exported functions --------------------------------------------------------*/

/**
 * @brief Initialize touchscreen system
 * @param hts Pointer to touchscreen handle structure
 * @param hi2c Pointer to I2C handle
 * @retval TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_Init(TS_HandleTypeDef *hts, I2C_HandleTypeDef *hi2c)
{
    TS_StatusTypeDef status = TS_OK;

    /* Check parameters */
    if (hts == NULL || hi2c == NULL) {
        return TS_INVALID_PARAM;
    }

    /* Initialize structure */
    memset(hts, 0, sizeof(TS_HandleTypeDef));
    hts->hi2c = hi2c;
    g_hts = hts;

    /* Initialize I2C peripheral */
    status = TS_InitI2C(hts);
    if (status != TS_OK) {
        return status;
    }

    /* Check if device is present */
    status = TS_CheckDevice(hts);
    if (status != TS_OK) {
        return status;
    }

    /* Set default configuration */
    TS_ConfigTypeDef default_config = TS_GetDefaultConfig();
    status = TS_Configure(hts, &default_config);
    if (status != TS_OK) {
        return status;
    }



    /* Configure STMPE811 controller */
    status = TS_ConfigureController(hts);
    if (status != TS_OK) {
        return status;
    }

    /* Configure interrupts if enabled */
    if (hts->Config.InterruptEnable) {
        TS_EnableInterrupt(hts, true);
        TS_ITConfig(hts);

    }
    else {
        TS_EnableInterrupt(hts, false);
    }

    hts->IsInitialized = true;
    return TS_OK;
}

/**
 * @brief Deinitialize touchscreen system
 * @param hts Pointer to touchscreen handle structure
 * @retval TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_DeInit(TS_HandleTypeDef *hts)
{
    if (hts == NULL) {
        return TS_INVALID_PARAM;
    }

    /* Disable interrupts */
    TS_EnableInterrupt(hts, false);

    /* Disable touchscreen controller */
    TS_WriteRegister(hts, STMPE811_REG_SYS_CTRL2, STMPE811_SYS_CTRL2_TSC_OFF);

    /* Reset structure */
    hts->IsInitialized = false;
    g_hts = NULL;

    return TS_OK;
}

/**
 * @brief Configure touchscreen parameters
 * @param hts Pointer to touchscreen handle structure
 * @param config Pointer to configuration structure
 * @retval TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_Configure(TS_HandleTypeDef *hts, TS_ConfigTypeDef *config)
{
    if (hts == NULL || config == NULL) {
        return TS_INVALID_PARAM;
    }

    /* Store configuration */
    hts->Config = *config;

    return TS_OK;
}

/**
 * @brief Reset touchscreen controller
 * @param hts Pointer to touchscreen handle structure
 * @retval TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_Reset(TS_HandleTypeDef *hts)
{
    if (hts == NULL) {
        return TS_INVALID_PARAM;
    }

    /* Software reset */
    TS_WriteRegister(hts, STMPE811_REG_SYS_CTRL1, 0x02);
    TS_DELAY_MS(5);
    TS_WriteRegister(hts, STMPE811_REG_SYS_CTRL1, 0x00);
    TS_DELAY_MS(2);

    return TS_OK;
}

/**
 * @brief Get current touch data
 * @param hts Pointer to touchscreen handle structure
 * @param touch_data Pointer to store touch data
 * @retval TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_GetTouchData(TS_HandleTypeDef *hts, TS_TouchDataTypeDef *touch_data)
{
    if (hts == NULL || touch_data == NULL) {
        return TS_INVALID_PARAM;
    }

    *touch_data = hts->TouchData;

    return TS_OK;
}

/**
 * @brief Get single touch coordinates
 * @param hts Pointer to touchscreen handle structure
 * @param xPos Pointer to store X coordinate
 * @param yPos Pointer to store Y coordinate
 * @retval TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_GetSingleTouch(TS_HandleTypeDef *hts,
                                  uint16_t *xPos,
                                  uint16_t *yPos)
{
    uint16_t raw_x = 0;
    uint16_t raw_y = 0;
    uint16_t disp_x = 0;
    uint16_t disp_y = 0;

    if(hts == NULL || xPos == NULL || yPos == NULL) {
        return TS_INVALID_PARAM;
    }

    *xPos = 0;
    *yPos = 0;
    uint8_t fifo_size = 0;
    uint8_t status = 0;

    if(!hts->IsInitialized) {
        return TS_NOT_INITIALIZED;
    }

    if (TS_ReadRegister(hts, STMPE811_REG_TSC_CTRL, &status) != TS_OK ) {
        return TS_ERROR;
    }

    if (status & (uint8_t)STMPE811_TS_CTRL_STATUS)
    {
        if (TS_ReadRegister(hts, STMPE811_REG_FIFO_SIZE, &fifo_size) != TS_OK) return TS_ERROR;
    }
    else {

        /* Reset FIFO (mandatory after reading) */
        TS_WriteRegister(g_hts, STMPE811_REG_FIFO_STA, 0x01);
        TS_WriteRegister(g_hts, STMPE811_REG_FIFO_STA, 0x00);
        return TS_ERROR;
    }


    if(TS_ReadRawCoordinates(&raw_x, &raw_y, NULL) != TS_OK) {
        return TS_ERROR;
    }

    if(TS_ConvertCoordinates(raw_x, raw_y, &disp_x, &disp_y) != TS_OK) {
        return TS_ERROR;
    }

    TS_FilterCoordinates(&disp_x, &disp_y);

    *xPos = disp_x;
    *yPos = disp_y;

    return TS_OK;
}


TS_StatusTypeDef TS_GetTouchState(TS_HandleTypeDef *hts,
                                 uint16_t *x,
                                 uint16_t *y,
                                 uint8_t *pressed)
{
    uint16_t raw_x = 0;
    uint16_t raw_y;
    uint16_t disp_x;
    uint16_t disp_y;

    if(hts == NULL || x == NULL || y == NULL || pressed == NULL) {
        return TS_INVALID_PARAM;
    }

    *pressed = 0;
    *x = 0;
    *y = 0;

    if(!hts->IsInitialized) {
        return TS_NOT_INITIALIZED;
    }

    if(TS_ReadRawCoordinates(&raw_x, &raw_y, NULL) != TS_OK) {
        return TS_OK;
    }

    if(TS_ConvertCoordinates(raw_x, raw_y, &disp_x, &disp_y) != TS_OK) {
        return TS_OK;
    }

    TS_FilterCoordinates(&disp_x, &disp_y);

    *x = disp_x;
    *y = disp_y;
    *pressed = 1;

    return TS_OK;
}



/**
 * @brief Check if touchscreen is currently touched
 * @param hts Pointer to touchscreen handle structure
 * @retval bool True if touched (FIFO has data)
 */
bool TS_IsTouched(TS_HandleTypeDef *hts)
{
    if (hts == NULL || !hts->IsInitialized) {
        return false;
    }

    uint8_t fifo_size = 0;
    if (TS_ReadRegister(hts, STMPE811_REG_FIFO_SIZE, &fifo_size) != TS_OK) {
        return false;
    }

    return (fifo_size > 0);
}

/**
 * @brief Get number of active touches
 * @param hts Pointer to touchscreen handle structure
 * @retval uint8_t Number of touches
 */
uint8_t TS_GetTouchCount(TS_HandleTypeDef *hts)
{
    if (hts == NULL) {
        return 0;
    }

    return hts->TouchData.TouchCount;
}

/**
 * @brief Calibrate touchscreen
 * @param hts Pointer to touchscreen handle structure
 * @retval TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_Calibrate(TS_HandleTypeDef *hts)
{
    if (hts == NULL) {
        return TS_INVALID_PARAM;
    }

    /* Basic calibration - in a real implementation, this would
       involve displaying calibration points and collecting user input */

    hts->Calibration.MinX = TS_DEFAULT_MIN_COORD;
    hts->Calibration.MaxX = TS_DEFAULT_MAX_COORD;
    hts->Calibration.MinY = TS_DEFAULT_MIN_COORD;
    hts->Calibration.MaxY = TS_DEFAULT_MAX_COORD;
    hts->Calibration.ScaleX = (float)TS_DISPLAY_WIDTH / (float)(hts->Calibration.MaxX - hts->Calibration.MinX);
    hts->Calibration.ScaleY = (float)TS_DISPLAY_HEIGHT / (float)(hts->Calibration.MaxY - hts->Calibration.MinY);
    hts->Calibration.OffsetX = (int16_t)(-hts->Calibration.MinX);
    hts->Calibration.OffsetY = (int16_t)(-hts->Calibration.MinY);
    hts->Calibration.IsCalibrated = true;

    return TS_OK;
}

/**
 * @brief Set calibration data
 * @param hts Pointer to touchscreen handle structure
 * @param calibration Pointer to calibration data
 * @retval TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_SetCalibration(TS_HandleTypeDef *hts, TS_CalibrationTypeDef *calibration)
{
    if (hts == NULL || calibration == NULL) {
        return TS_INVALID_PARAM;
    }

    hts->Calibration = *calibration;

    return TS_OK;
}

/**
 * @brief Get calibration data
 * @param hts Pointer to touchscreen handle structure
 * @param calibration Pointer to store calibration data
 * @retval TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_GetCalibration(TS_HandleTypeDef *hts, TS_CalibrationTypeDef *calibration)
{
    if (hts == NULL || calibration == NULL) {
        return TS_INVALID_PARAM;
    }

    *calibration = hts->Calibration;

    return TS_OK;
}

/**
 * @brief Enable/disable interrupt
 * @param hts Pointer to touchscreen handle structure
 * @param enable Enable/disable flag
 * @retval TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_EnableInterrupt(TS_HandleTypeDef *hts, bool enable)
{
    if (hts == NULL) {
        return TS_INVALID_PARAM;
    }

    if (enable) {
        log_debug("TS: Enabling touchscreen interrupt");
        /* Enable touch detection interrupt */
        TS_WriteRegister(hts, STMPE811_REG_INT_EN, STMPE811_INT_EN_TOUCH_DET);
        TS_WriteRegister(hts, STMPE811_REG_INT_CTRL, STMPE811_INT_CTRL_POL_LOW | STMPE811_INT_CTRL_ENABLE);
    } else {
        /* Disable interrupts */
        TS_WriteRegister(hts, STMPE811_REG_INT_EN, 0x00);
        TS_WriteRegister(hts, STMPE811_REG_INT_CTRL, STMPE811_INT_CTRL_DISABLE);
    }

    hts->InterruptMode = enable;

    return TS_OK;
}

/**
 * @brief Configure interrupt
 * @param hts Pointer to touchscreen handle structure
 * @retval TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_ITConfig(TS_HandleTypeDef *hts)
{
    if (hts == NULL) {
        return TS_INVALID_PARAM;
    }

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Configure interrupt pin */
    GPIO_InitStruct.Pin = TS_INT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(TS_INT_GPIO_PORT, &GPIO_InitStruct);

    /* Enable EXTI interrupt */
    HAL_NVIC_SetPriority(TS_INT_EXTI_IRQn, TS_INT_NVIC_PRIORITY, 0x00);
    HAL_NVIC_EnableIRQ(TS_INT_EXTI_IRQn);

    log_debug("TS_ITConfig Initialized");
    return TS_OK;
}

/**
 * @brief Interrupt handler
 * @param hts Pointer to touchscreen handle structure
 */
void TS_IRQHandler(TS_HandleTypeDef *hts)
{
    if (hts == NULL) {
        return;
    }

    uint8_t int_status = 0;

    /* Read interrupt status */
    TS_ReadRegister(hts, STMPE811_REG_INT_STA, &int_status);

    if (int_status & STMPE811_INT_EN_TOUCH_DET) {
        /* Touch detected - callbacks can be handled here if needed */
        /* Note: TS_ProcessTouchData removed as TS_GetSingleTouch handles everything */

        /* Call callbacks if registered (for other purposes) */
        if (hts->TouchCallback != NULL) {
            hts->TouchCallback();
        }

        /* Detect gestures if needed */
        TS_GestureTypeDef gesture = TS_AnalyzeGesture(hts);
        if (gesture != TS_GESTURE_NONE && hts->GestureCallback != NULL) {
            hts->GestureCallback(gesture);
        }
    }

    /* Clear interrupt */
    TS_WriteRegister(hts, STMPE811_REG_INT_STA, int_status);
}

/**
 * @brief Service pending touchscreen IRQ outside ISR context
 * @details Clears STMPE811 interrupt and invokes callbacks safely from thread context
 */
void TS_ServiceIRQ(void)
{
    if (s_ts_irq_pending && g_hts != NULL && g_hts->IsInitialized)
    {
        s_ts_irq_pending = false;
        TS_IRQHandler(g_hts);
    }
}

/**
 * @brief Register callback functions
 * @param hts Pointer to touchscreen handle structure
 * @param touch_callback Touch detected callback
 * @param release_callback Touch released callback
 * @param gesture_callback Gesture detected callback
 * @retval TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_RegisterCallbacks(TS_HandleTypeDef *hts,
                                     void (*touch_callback)(void),
                                     void (*release_callback)(void),
                                     void (*gesture_callback)(TS_GestureTypeDef))
{
    if (hts == NULL) {
        return TS_INVALID_PARAM;
    }

    hts->TouchCallback = touch_callback;
    hts->ReleaseCallback = release_callback;
    hts->GestureCallback = gesture_callback;

    return TS_OK;
}

/**
 * @brief Convert raw touchscreen coordinates to display coordinates
 * @param raw_x Raw X coordinate
 * @param raw_y Raw Y coordinate
 * @param disp_x Pointer to store display X coordinate
 * @param disp_y Pointer to store display Y coordinate
 * @retval TS_StatusTypeDef Status of the operation
 */

static TS_StatusTypeDef TS_ConvertCoordinates(uint16_t raw_x,
                                              uint16_t raw_y,
                                              uint16_t *disp_x,
                                              uint16_t *disp_y)
{
    if(disp_x == NULL || disp_y == NULL) {
        return TS_INVALID_PARAM;
    }

    /* Use linear mapping between measured raw ranges and display coords.
       Small raw_x -> RIGHT side of display (inverted X).
       Small raw_y -> BOTTOM of display (inverted Y).
       The 'map' helper clamps inputs and performs integer-safe scaling. */
    int32_t xr = map((int32_t)raw_x, (int32_t)TS_RAW_X_MIN, (int32_t)TS_RAW_X_MAX,
                     (int32_t)(TS_DISPLAY_WIDTH - 1), 0);

    int32_t yr = map((int32_t)raw_y, (int32_t)TS_RAW_Y_MIN, (int32_t)TS_RAW_Y_MAX,
                     (int32_t)(TS_DISPLAY_HEIGHT - 1), 0);

    /* Clamp to display bounds */
    if (xr < 0) xr = 0;
    else if (xr >= TS_DISPLAY_WIDTH) xr = TS_DISPLAY_WIDTH - 1;

    if (yr < 0) yr = 0;
    else if (yr >= TS_DISPLAY_HEIGHT) yr = TS_DISPLAY_HEIGHT - 1;

    *disp_x = (uint16_t)xr;
    *disp_y = (uint16_t)yr;

    return TS_OK;
}


/**
 * @brief Read register from STMPE811
 * @param hts Pointer to touchscreen handle structure
 * @param reg Register address
 * @param data Pointer to store data
 * @retval TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_ReadRegister(TS_HandleTypeDef *hts, uint8_t reg, uint8_t *data)
{
    if (hts == NULL || data == NULL) {
        return TS_INVALID_PARAM;
    }

    if (I2C_Mem_Read(STMPE811_I2C_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, data, 1, TS_TIMEOUT) != I2C_OK) {
        return TS_COMMUNICATION_ERROR;
    }

    return TS_OK;
}

/**
 * @brief Write register to STMPE811
 * @param hts Pointer to touchscreen handle structure
 * @param reg Register address
 * @param data Data to write
 * @retval TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_WriteRegister(TS_HandleTypeDef *hts, uint8_t reg, uint8_t data)
{
    if (hts == NULL) {
        return TS_INVALID_PARAM;
    }

    if (I2C_Mem_Write(STMPE811_I2C_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, TS_TIMEOUT) != I2C_OK) {
        return TS_COMMUNICATION_ERROR;
    }

    return TS_OK;
}


/**
 * @brief Read 16-bit register from STMPE811
 * @param hts Pointer to touchscreen handle structure
 * @param reg Register address
 * @param data Pointer to store data
 * @retval TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_ReadRegister16(TS_HandleTypeDef *hts, uint8_t reg, uint16_t *data)
{
    if (hts == NULL || data == NULL) {
        return TS_INVALID_PARAM;
    }

    uint8_t buffer[2];
    if (I2C_Mem_Read(STMPE811_I2C_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, buffer, 2, TS_TIMEOUT) != I2C_OK) {
        return TS_COMMUNICATION_ERROR;
    }

    *data = (buffer[0] << 8) | buffer[1];

    return TS_OK;
}

/**
 * @brief Write 16-bit data to touchscreen register
 * @param hts Pointer to touchscreen handle
 * @param reg Register address
 * @param data Data to write
 * @retval TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_WriteRegister16(TS_HandleTypeDef *hts, uint8_t reg, uint16_t data)
{
    if (hts == NULL) {
        return TS_INVALID_PARAM;
    }

    uint8_t buffer[2];
    buffer[0] = (uint8_t)(data >> 8);   // MSB first
    buffer[1] = (uint8_t)(data & TS_BYTE_MASK); // LSB

    if (I2C_Mem_Write(STMPE811_I2C_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, buffer, 2, TS_TIMEOUT) != I2C_OK) {
        return TS_COMMUNICATION_ERROR;
    }

    return TS_OK;
}

/**
 * @brief Read multiple bytes from touchscreen register with error recovery
 * @details Reads a block of data from STMPE811 registers using I2C with automatic
 *          bus recovery on communication failures for improved reliability.
 * @param hts Pointer to touchscreen handle
 * @param reg Starting register address
 * @param data Pointer to data buffer to store read bytes
 * @param size Number of bytes to read
 * @retval TS_StatusTypeDef TS_OK on success, TS_INVALID_PARAM for bad inputs,
 *                         TS_COMMUNICATION_ERROR on I2C failure
 */
TS_StatusTypeDef TS_ReadRegisterMulti(TS_HandleTypeDef *hts, uint8_t reg, uint8_t *data, uint16_t size)
{
    if (hts == NULL || data == NULL || size == 0) {
        return TS_INVALID_PARAM;
    }

    if (I2C_Mem_Read_Multi(STMPE811_I2C_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, data, size, TS_TIMEOUT) != I2C_OK) {
        return TS_COMMUNICATION_ERROR;
    }

    return TS_OK;
}

/**
 * @brief Get pressure value from touchscreen
 * @param hts Pointer to touchscreen handle
 * @param pressure Pointer to store pressure value
 * @retval TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_GetPressure(TS_HandleTypeDef *hts, uint16_t *pressure)
{
    if (hts == NULL || pressure == NULL) {
        return TS_INVALID_PARAM;
    }

    uint8_t data = 0;

    /* Read pressure data from Z register */
    if (TS_ReadRegister(hts, STMPE811_REG_TSC_DATA_Z, &data) != TS_OK) {
        return TS_COMMUNICATION_ERROR;
    }

    /* Convert to 16-bit pressure value */
    *pressure = (uint16_t)data;

    /* Apply pressure scaling if needed */
    if (*pressure > 0) {
        *pressure = (uint16_t)((*pressure * TS_PRESSURE_SCALE) / TS_PRESSURE_MAX_VALUE);
    }

    return TS_OK;
}

/**
 * @brief Get default configuration
 * @retval TS_ConfigTypeDef Default configuration
 */
TS_ConfigTypeDef TS_GetDefaultConfig(void)
{
    TS_ConfigTypeDef config = {
        .SampleTime = STMPE811_TSC_CFG_4_SAMPLE,
        .AverageControl = STMPE811_TSC_CFG_DELAY_1MS,
        .TouchDetectDelay = STMPE811_TSC_CFG_SETTLE_1MS,
        .PanelDriverSettlingTime = STMPE811_TSC_CFG_SETTLE_1MS,
        .PressureThreshold = TS_PRESSURE_THRESHOLD,
        .InterruptEnable = true,
        .FIFOEnable = true,
        .FIFOThreshold = 1
    };

    return config;
}

/* Private functions ---------------------------------------------------------*/


/**
 * @brief Initialize I2C peripheral
 * @param hts Pointer to touchscreen handle structure
 * @retval TS_StatusTypeDef Status of the operation
 */
static TS_StatusTypeDef TS_InitI2C(TS_HandleTypeDef *hts)
{
    /* If application already provided an I2C handle, assume it's configured and ready.
       Otherwise use the central I2C driver (I2C_Init which configures hi2c3) */
    if (hts->hi2c->Instance == NULL) {
        I2C_Init(); /* initializes hi2c3 */
        hts->hi2c = &hi2c3; /* use central handle */
    }

    return TS_OK;
}

/**
 * @brief Check if STMPE811 device is present
 * @param hts Pointer to touchscreen handle structure
 * @retval TS_StatusTypeDef Status of the operation
 */
static TS_StatusTypeDef TS_CheckDevice(TS_HandleTypeDef *hts)
{
    uint16_t device_id = 0;

    /* Check that device responds on I2C bus before reading registers */
    if (I2C_IsDeviceReady(STMPE811_I2C_ADDRESS, 3, 100) != I2C_OK) {
        log_debug("TS: Device not found on I2C bus");
        return TS_DEVICE_NOT_FOUND;
    }

    if (TS_ReadRegister16(hts, STMPE811_REG_CHIP_ID, &device_id) != TS_OK) {
        log_debug("TS: Failed to read device ID");
        return TS_DEVICE_NOT_FOUND;
    }

    if (device_id != STMPE811_CHIP_ID) {
        log_debug("TS: Device ID mismatch");
        return TS_DEVICE_NOT_FOUND;
    }

    hts->DeviceID = device_id;
    log_debug("TS: Device found, ID: 0x%04X", device_id);
    return TS_OK;
}

/**
 * @brief Configure STMPE811 controller
 * @param hts Pointer to touchscreen handle structure
 * @retval TS_StatusTypeDef Status of the operation
 * @note Configuration sequence based on ST BSP STMPE811_TS_Init()
 */
static TS_StatusTypeDef TS_ConfigureController(TS_HandleTypeDef *hts)
{
    uint8_t tmp = 0;

    /* Reset the device */
    TS_Reset(hts);

    TS_ReadRegister(hts, STMPE811_REG_SYS_CTRL2, &tmp);

    /* Enable GPIO/IO functionality by clearing GPIO_OFF bit */
    tmp &= ~(STMPE811_SYS_CTRL2_GPIO_OFF);

    /* Write back - this enables GPIO/IO functionality */
    TS_WriteRegister(hts, STMPE811_REG_SYS_CTRL2, tmp);

    /* Enable Alternate Function for touchscreen pins (GPIO 4-7) */
    TS_ReadRegister(hts, STMPE811_REG_IO_AF, &tmp);
    /* Enable AF for pins 4-7 (TSC pins): clear bits defined by STMPE811_TOUCH_IO_ALL */
    tmp &= ~(STMPE811_PIN_4 | STMPE811_PIN_5 | STMPE811_PIN_6 | STMPE811_PIN_7);
    TS_WriteRegister(hts, STMPE811_REG_IO_AF, tmp);

    /* Now enable TSC and ADC functions in SYS_CTRL2 by clearing their OFF bits */
    //TS_ReadRegister(hts, STMPE811_REG_SYS_CTRL2, &tmp);
    tmp &= ~(STMPE811_SYS_CTRL2_TSC_OFF | STMPE811_SYS_CTRL2_ADC_OFF);
    TS_WriteRegister(hts, STMPE811_REG_SYS_CTRL2, tmp);

     /* Configure ADC - Sample Time, bit number and Reference
         0x49 = BSP setting: 64 sample time, 12-bit, internal reference */
    TS_WriteRegister(hts, STMPE811_REG_ADC_CTRL1, TS_ADC_CTRL_12BIT);

    /* Wait for ADC to stabilize */
    TS_DELAY_MS(2);

    /* Configure ADC clock speed: 3.25 MHz */
    TS_WriteRegister(hts, STMPE811_REG_ADC_CTRL2, 0x01);

    /* Configure touchscreen:
       - 4 samples averaging (0x80)
       - 500us touch detect delay (0x18)
       - 500us panel driver settling time (0x02)
       = 0x9A */
    TS_WriteRegister(hts, STMPE811_REG_TSC_CFG, 0x9A);

    /* Configure FIFO threshold: single point reading */
    TS_WriteRegister(hts, STMPE811_REG_FIFO_TH, 0x01);

    /* Clear the FIFO memory content */
    TS_WriteRegister(hts, STMPE811_REG_FIFO_STA, 0x01U);

    /* Put the FIFO back into operation mode */
    TS_WriteRegister(hts, STMPE811_REG_FIFO_STA, 0x00U);

    /* Set the fractional part and whole part for Z pressure:
       - Fractional part: 7
       - Whole part: 1 */
    TS_WriteRegister(hts, STMPE811_REG_TSC_FRACT_XYZ, 0x01U);

    /* Set the driving capability for TSC pins: 50mA */
    TS_WriteRegister(hts, STMPE811_REG_TSC_I_DRIVE, 0x01U);

     /* Enable touchscreen controller: use BSP default (enable TSC in XYZ mode) */
     TS_WriteRegister(hts, STMPE811_REG_TSC_CTRL, STMPE811_TSC_CTRL_EN);

    /* Clear all pending interrupts */
    TS_WriteRegister(hts, STMPE811_REG_INT_STA, 0xFFU);

    /* Wait for configuration to settle */
    TS_DELAY_MS(2);



    return TS_OK;
}



/**
 * @brief Read raw coordinates from controller

 * @param raw_x Pointer to store raw X coordinate
 * @param raw_y Pointer to store raw Y coordinate
 * @param pressure Pointer to store pressure value
 * @retval TS_StatusTypeDef Status of the operation
 */


static TS_StatusTypeDef TS_ReadRawCoordinates(uint16_t *raw_x,
                                              uint16_t *raw_y,
                                              uint16_t *pressure)
{
    uint8_t data[4] = {0};
    uint32_t xyz = 0;

    if (raw_x == NULL || raw_y == NULL) {
        return TS_INVALID_PARAM;
    }

    if (TS_ReadRegisterMulti(g_hts, STMPE811_REG_TSC_DATA_NON_INC, data, 4) != TS_OK) {
        return TS_COMMUNICATION_ERROR;
    }


    xyz  = ((uint32_t)data[0] << 24);
    xyz |= ((uint32_t)data[1] << 16);
    xyz |= ((uint32_t)data[2] << 8);
    xyz |=  (uint32_t)data[3] << 0;

    /* Extract 12-bit coordinates (STMPE811 datasheet format) */
    *raw_x = (xyz >> 20) & 0x0FFF;
    *raw_y = (xyz >>  8) & 0x0FFF;

    if (pressure != NULL) {
        *pressure = xyz & 0xFF;
    }

    log_debug("Raw X: %u, Raw Y: %u, Pressure: %u", *raw_x, *raw_y, pressure ? *pressure : 0);

    /* Reset FIFO (mandatory after reading) */
    TS_WriteRegister(g_hts, STMPE811_REG_FIFO_STA, 0x01);
    TS_WriteRegister(g_hts, STMPE811_REG_FIFO_STA, 0x00);

    return TS_OK;
}


/**
 * @brief Filter coordinates using threshold-based update (matches working project)
 * @param hts Pointer to touchscreen handle structure
 * @param xPos Pointer to X coordinate
 * @param yPos Pointer to Y coordinate
 */
//
static void TS_FilterCoordinates(uint16_t *x,
                                 uint16_t *y)
{
    static uint16_t _x = 0;
    static uint16_t _y = 0;

    int32_t x_raw = (int32_t)*x;
    int32_t y_raw = (int32_t)*y;

    int32_t xDiff = x_raw > _x ? (x_raw - _x) : (_x - x_raw);
    int32_t yDiff = y_raw > _y ? (y_raw - _y) : (_y - y_raw);

    /* Threshold-based smoothing (matches working project) */
    if ((xDiff + yDiff) > 5) {
        _x = (uint16_t)x_raw;
        _y = (uint16_t)y_raw;
    }

    *x = _x;
    *y = _y;

    log_debug("Filter X = %u, Filter Y = %u", *x, *y);
}

/**
 * @brief Analyze gesture from touch data
 * @param hts Pointer to touchscreen handle structure
 * @retval TS_GestureTypeDef Detected gesture
 */
static TS_GestureTypeDef TS_AnalyzeGesture(TS_HandleTypeDef *hts)
{
    /* Basic gesture recognition - can be enhanced */

    if (hts->TouchData.TouchCount == 0 && hts->PrevTouchData.TouchCount > 0) {
        /* Touch released */
        uint32_t touch_duration = HAL_GetTick() - hts->PrevTouchData.Points[0].Timestamp;

        if (touch_duration > TS_LONG_PRESS_TIME) {
            return TS_GESTURE_LONG_PRESS;
        }
        return TS_GESTURE_TAP;
    }

    if (hts->TouchData.TouchCount > 0 && hts->PrevTouchData.TouchCount > 0) {
        /* Calculate movement */
        int16_t deltaX = (int16_t)(hts->TouchData.Points[0].X - hts->PrevTouchData.Points[0].X);
        int16_t deltaY = (int16_t)(hts->TouchData.Points[0].Y - hts->PrevTouchData.Points[0].Y);
        uint16_t distance = (uint16_t)sqrt((double)(deltaX*deltaX + deltaY*deltaY));

        if (distance > TS_GESTURE_THRESHOLD) {
            if (abs(deltaX) > abs(deltaY)) {
                return (deltaX > 0) ? TS_GESTURE_SWIPE_RIGHT : TS_GESTURE_SWIPE_LEFT;
            }
            return (deltaY > 0) ? TS_GESTURE_SWIPE_DOWN : TS_GESTURE_SWIPE_UP;
        }
    }

    return TS_GESTURE_NONE;
}

/**
 * @brief Check if touch is valid based on pressure
 * @param pressure Pressure value
 * @retval bool True if valid touch
 */

/* IRQ Handlers -------------------------------------------------------------*/

/**
 * @brief External interrupt handler

 */

//  void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
// {
// //    if(GPIO_Pin == TS_INT_PIN)
// //     {
// //          /* Clear Wakeup flag */
// //         __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

// //         /* Always update touch activity for low power management */
// //         APP_TouchActivity();

// //         /* Defer I2C-based interrupt handling to thread context */
// //         s_ts_irq_pending = true;
// //     }
// }
