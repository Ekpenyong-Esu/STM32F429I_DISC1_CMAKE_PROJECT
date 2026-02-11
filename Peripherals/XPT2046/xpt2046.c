/**
  ******************************************************************************
  * @file    xpt2046.c
  * @brief   XPT2046 Resistive Touchscreen Controller Driver Implementation
  * @details Fixed version with proper ADC reading, debouncing, and calibration
  * @version 1.1
  * @date    2025-02-08
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "xpt2046.h"
#include "log.h"
#include "spi.h"
#include "gpio.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Private defines -----------------------------------------------------------*/

/** @defgroup XPT2046_Private_Defines Private Defines
 * @{
 */

/* XPT2046 Commands (Control Byte Format: S A2 A1 A0 MODE SER/DFR PD1 PD0) */
#define XPT2046_CMD_X_POS             0xD0    /**< Read X position (12-bit) */
#define XPT2046_CMD_Y_POS             0x90    /**< Read Y position (12-bit) */
#define XPT2046_CMD_Z1_POS            0xB0    /**< Read Z1 (pressure) */
#define XPT2046_CMD_Z2_POS            0xC0    /**< Read Z2 (pressure) */

/* Command bit definitions */
#define XPT2046_START_BIT             0x80    /**< Start bit */
#define XPT2046_12BIT_MODE            0x00    /**< 12-bit conversion */
#define XPT2046_8BIT_MODE             0x08    /**< 8-bit conversion */
#define XPT2046_DIFF_MODE             0x00    /**< Differential reference */
#define XPT2046_SINGLE_MODE           0x04    /**< Single-ended reference */
#define XPT2046_POWER_DOWN            0x00    /**< Power down between conversions */
#define XPT2046_POWER_ALWAYS_ON       0x03    /**< Always powered on */

/* Timing constants */
#define XPT2046_SETTLE_DELAY_US       10      /**< ADC settling time (us) */
#define XPT2046_CONVERSION_DELAY_US   5       /**< Conversion delay (us) */
#define XPT2046_DEBOUNCE_SAMPLES      5       /**< Number of samples for debouncing */
#define XPT2046_TOUCH_THRESHOLD       200     /**< Minimum pressure to register touch */

/* ADC value limits */
#define XPT2046_ADC_MIN               100     /**< Minimum valid ADC value */
#define XPT2046_ADC_MAX               3900    /**< Maximum valid ADC value */

/** @} */

/* Default weak MSP implementations (board can override) */
__weak void XPT2046_MspInit(GPIO_TypeDef *cs_port, uint16_t cs_pin,
                            GPIO_TypeDef *irq_port, uint16_t irq_pin)
{
    (void)cs_port;
    (void)cs_pin;
    (void)irq_port;
    (void)irq_pin;
}

__weak void XPT2046_MspDeInit(GPIO_TypeDef *cs_port, uint16_t cs_pin,
                              GPIO_TypeDef *irq_port, uint16_t irq_pin)
{
    (void)cs_port;
    (void)cs_pin;
    (void)irq_port;
    (void)irq_pin;
}

/* Private function prototypes -----------------------------------------------*/
static XPT2046_StatusTypeDef XPT2046_ReadADC(XPT2046_Handle_t *hxpt, uint8_t command, uint16_t *value);
static XPT2046_StatusTypeDef XPT2046_ReadCoordinatesRaw(XPT2046_Handle_t *hxpt,
                                                        uint16_t *x_raw, uint16_t *y_raw,
                                                        uint16_t *pressure);
static void XPT2046_ApplyCalibration(XPT2046_Handle_t *hxpt, uint16_t x_raw, uint16_t y_raw,
                                    uint16_t *x_cal, uint16_t *y_cal);
static uint16_t XPT2046_CalculatePressure(uint16_t x, uint16_t z1, uint16_t z2);
static uint16_t XPT2046_MedianFilter(uint16_t *samples, uint8_t count);
static void XPT2046_DelayUs(uint32_t us);

/* Exported functions -------------------------------------------------------*/

/**
 * @brief   Initialize XPT2046 touchscreen controller
 * @param   hxpt Pointer to XPT2046 handle
 * @param   cs_port Chip select port
 * @param   cs_pin Chip select pin
 * @param   irq_port Interrupt port (T_IRQ/PENIRQ pin)
 * @param   irq_pin Interrupt pin
 * @param   width Display width in pixels
 * @param   height Display height in pixels
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_Init(XPT2046_Handle_t *hxpt,
                                  GPIO_TypeDef *cs_port, uint16_t cs_pin,
                                  GPIO_TypeDef *irq_port, uint16_t irq_pin,
                                  uint16_t width, uint16_t height)
{
    if (hxpt == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    log_debug("XPT2046: Initializing touch controller");

    /* Board-specific MSP hook (GPIO clocks) */
    XPT2046_MspInit(cs_port, cs_pin, irq_port, irq_pin);

    /* Initialize structure */
    memset(hxpt, 0, sizeof(XPT2046_Handle_t));

    hxpt->config.cs_port = cs_port;
    hxpt->config.cs_pin = cs_pin;
    hxpt->config.irq_port = irq_port;
    hxpt->config.irq_pin = irq_pin;
    hxpt->config.width = width;
    hxpt->config.height = height;
    hxpt->config.flip_x = false;
    hxpt->config.flip_y = false;
    hxpt->config.swap_xy = false;

    /* Default calibration values (will need adjustment for your specific display) */
    hxpt->cal.x_min = 300;      // Adjust these based on your display
    hxpt->cal.x_max = 3700;
    hxpt->cal.y_min = 300;
    hxpt->cal.y_max = 3700;
    hxpt->cal.pressure_threshold = XPT2046_TOUCH_THRESHOLD;

    /* Test communication by reading a value */
    uint16_t test_val = 0;
    if (XPT2046_ReadADC(hxpt, XPT2046_CMD_X_POS, &test_val) != XPT2046_OK) {
        log_error("XPT2046: Communication test failed");
        return XPT2046_ERROR;
    }

    hxpt->initialized = true;
    log_debug("XPT2046: Initialized successfully (display: %dx%d)", width, height);

    return XPT2046_OK;
}

/**
 * @brief   Check if touchscreen is currently touched
 * @param   hxpt Pointer to XPT2046 handle
 * @retval  bool True if touched (IRQ line is low), false otherwise
 */
bool XPT2046_IsTouched(XPT2046_Handle_t *hxpt)
{
    if (hxpt == NULL || !hxpt->initialized) {
        return false;
    }

    /* IRQ pin is active low when screen is touched */
    return (HAL_GPIO_ReadPin(hxpt->config.irq_port, hxpt->config.irq_pin) == GPIO_PIN_RESET);
}

/**
 * @brief   Read current touch position and pressure
 * @param   hxpt Pointer to XPT2046 handle
 * @param   touch Pointer to touch point structure to fill
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_ReadTouch(XPT2046_Handle_t *hxpt, XPT2046_TouchPoint_t *touch)
{
    if (hxpt == NULL || touch == NULL || !hxpt->initialized) {
        return XPT2046_INVALID_PARAM;
    }

    /* Initialize touch structure */
    touch->state = XPT2046_STATE_RELEASED;
    touch->x = 0;
    touch->y = 0;
    touch->pressure = 0;

    /* Quick check - if not touched, return early */
    // Commented out to allow touch detection even if IRQ pin is not working
    // if (!XPT2046_IsTouched(hxpt)) {
    //     log_debug("XPT2046: No touch detected");
    //     return XPT2046_NO_TOUCH;
    // }

    /* Read raw coordinates with averaging */
    uint16_t x_samples[XPT2046_DEBOUNCE_SAMPLES];
    uint16_t y_samples[XPT2046_DEBOUNCE_SAMPLES];
    uint16_t pressure_samples[XPT2046_DEBOUNCE_SAMPLES];

    for (uint8_t i = 0; i < XPT2046_DEBOUNCE_SAMPLES; i++) {
        uint16_t x_raw;
        uint16_t y_raw;
        uint16_t pressure;

        XPT2046_StatusTypeDef status = XPT2046_ReadCoordinatesRaw(hxpt, &x_raw, &y_raw, &pressure);
        if (status != XPT2046_OK) {
            return status;
        }

        x_samples[i] = x_raw;
        y_samples[i] = y_raw;
        pressure_samples[i] = pressure;

        /* Small delay between samples */
        HAL_Delay(1);
    }

    /* Use median filtering to reject noise */
    uint16_t x_raw = XPT2046_MedianFilter(x_samples, XPT2046_DEBOUNCE_SAMPLES);
    uint16_t y_raw = XPT2046_MedianFilter(y_samples, XPT2046_DEBOUNCE_SAMPLES);
    uint16_t pressure = XPT2046_MedianFilter(pressure_samples, XPT2046_DEBOUNCE_SAMPLES);

    /* Check if pressure is sufficient */
    if (pressure < hxpt->cal.pressure_threshold) {
        return XPT2046_NO_TOUCH;
    }

    /* Validate raw coordinates are in reasonable range */
    if (x_raw < XPT2046_ADC_MIN || x_raw > XPT2046_ADC_MAX ||
        y_raw < XPT2046_ADC_MIN || y_raw > XPT2046_ADC_MAX) {
        log_debug("XPT2046: Invalid coordinates (x=%u, y=%u)", x_raw, y_raw);
        return XPT2046_NO_TOUCH;
    }

    /* Apply calibration to get screen coordinates */
    uint16_t x_cal;
    uint16_t y_cal;
    XPT2046_ApplyCalibration(hxpt, x_raw, y_raw, &x_cal, &y_cal);

    /* Apply transformations (swap/flip) if configured */
    uint16_t x_final = x_cal;
    uint16_t y_final = y_cal;

    if (hxpt->config.swap_xy) {
        uint16_t temp = x_final;
        x_final = y_final;
        y_final = temp;
    }

    if (hxpt->config.flip_x) {
        x_final = hxpt->config.width - 1 - x_final;
    }

    if (hxpt->config.flip_y) {
        y_final = hxpt->config.height - 1 - y_final;
    }

    /* Clamp to display bounds */
    if (x_final >= hxpt->config.width) x_final = hxpt->config.width - 1;
    if (y_final >= hxpt->config.height) y_final = hxpt->config.height - 1;

    /* Update touch structure */
    touch->x = x_final;
    touch->y = y_final;
    touch->pressure = pressure;
    touch->state = XPT2046_STATE_PRESSED;

    log_debug("XPT2046: Touch at (%u,%u) pressure=%u [raw: %u,%u]",
              x_final, y_final, pressure, x_raw, y_raw);

    return XPT2046_OK;
}

/**
 * @brief   Update touch state (call periodically in main loop or timer)
 * @param   hxpt Pointer to XPT2046 handle
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_Update(XPT2046_Handle_t *hxpt)
{
    if (hxpt == NULL || !hxpt->initialized) {
        return XPT2046_NOT_INITIALIZED;
    }

    XPT2046_TouchPoint_t new_touch;
    XPT2046_StatusTypeDef status = XPT2046_ReadTouch(hxpt, &new_touch);

    if (status == XPT2046_OK) {
        /* Valid touch detected */
        if (hxpt->touch.state == XPT2046_STATE_RELEASED) {
            /* New touch event */
            hxpt->touch = new_touch;
            hxpt->touch.state = XPT2046_STATE_PRESSED;
            log_debug("XPT2046: Touch pressed");
        } else {
            /* Continued touch (drag) */
            hxpt->touch.x = new_touch.x;
            hxpt->touch.y = new_touch.y;
            hxpt->touch.pressure = new_touch.pressure;
            hxpt->touch.state = XPT2046_STATE_HELD;
        }
        hxpt->last_touch_time = HAL_GetTick();
    } else if (status == XPT2046_NO_TOUCH) {
        /* No touch detected */
        if (hxpt->touch.state != XPT2046_STATE_RELEASED) {
            /* Touch release event */
            log_debug("XPT2046: Touch released");
            hxpt->touch.state = XPT2046_STATE_RELEASED;
            hxpt->touch.pressure = 0;
        }
    } else {
        /* Error occurred */
        return status;
    }

    return XPT2046_OK;
}

/**
 * @brief   Set calibration parameters
 * @param   hxpt Pointer to XPT2046 handle
 * @param   x_min Minimum X ADC value
 * @param   x_max Maximum X ADC value
 * @param   y_min Minimum Y ADC value
 * @param   y_max Maximum Y ADC value
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_SetCalibration(XPT2046_Handle_t *hxpt,
                                            uint16_t x_min, uint16_t x_max,
                                            uint16_t y_min, uint16_t y_max)
{
    if (hxpt == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    hxpt->cal.x_min = x_min;
    hxpt->cal.x_max = x_max;
    hxpt->cal.y_min = y_min;
    hxpt->cal.y_max = y_max;

    log_info("XPT2046: Calibration set - X[%u:%u] Y[%u:%u]",
             x_min, x_max, y_min, y_max);

    return XPT2046_OK;
}

/**
 * @brief   Configure touch transformations
 * @param   hxpt Pointer to XPT2046 handle
 * @param   swap_xy Swap X and Y coordinates
 * @param   flip_x Flip X coordinate
 * @param   flip_y Flip Y coordinate
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_SetTransform(XPT2046_Handle_t *hxpt,
                                          bool swap_xy, bool flip_x, bool flip_y)
{
    if (hxpt == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    hxpt->config.swap_xy = swap_xy;
    hxpt->config.flip_x = flip_x;
    hxpt->config.flip_y = flip_y;

    log_info("XPT2046: Transform set - swap:%d flip_x:%d flip_y:%d",
             swap_xy, flip_x, flip_y);

    return XPT2046_OK;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Read single ADC channel from XPT2046
 * @param   hxpt Pointer to XPT2046 handle
 * @param   command ADC command byte
 * @param   value Pointer to store 12-bit ADC value
 * @retval  XPT2046_StatusTypeDef Operation status
 */
static XPT2046_StatusTypeDef XPT2046_ReadADC(XPT2046_Handle_t *hxpt,
                                            uint8_t command,
                                            uint16_t *value)
{
    uint8_t tx_data[3] = {command, 0x00, 0x00};
    uint8_t rx_data[3] = {0x00, 0x00, 0x00};

    /* Select chip */
    HAL_GPIO_WritePin(hxpt->config.cs_port, hxpt->config.cs_pin, GPIO_PIN_RESET);

    /* Wait for ADC to settle */
    XPT2046_DelayUs(XPT2046_SETTLE_DELAY_US);

    /* Perform SPI transaction using the SPI module API */
    SPI_StatusTypeDef spi_status = SPI_TransmitReceive(tx_data, rx_data, 3, SPI_TIMEOUT_SHORT);
    
    /* Deselect chip */
    HAL_GPIO_WritePin(hxpt->config.cs_port, hxpt->config.cs_pin, GPIO_PIN_SET);

    if (spi_status != SPI_OK) {
        log_error("XPT2046: SPI transfer failed (status=%d)", spi_status);
        return XPT2046_ERROR;
    }

    /*
     * XPT2046 returns 12-bit value in bits [14:3] of the 16-bit response
     * Byte 0: Command echo (ignored)
     * Byte 1: Bits [11:5] of result
     * Byte 2: Bits [4:0] of result in upper 5 bits, plus 3 trailing zeros
     */
    *value = ((uint16_t)(rx_data[1] & 0x7F) << 5) | ((uint16_t)(rx_data[2] & 0xF8) >> 3);

    return XPT2046_OK;
}

/**
 * @brief   Read raw touch coordinates and pressure
 * @param   hxpt Pointer to XPT2046 handle
 * @param   x_raw Pointer to store raw X coordinate
 * @param   y_raw Pointer to store raw Y coordinate
 * @param   pressure Pointer to store pressure value
 * @retval  XPT2046_StatusTypeDef Operation status
 */
static XPT2046_StatusTypeDef XPT2046_ReadCoordinatesRaw(XPT2046_Handle_t *hxpt,
                                                        uint16_t *x_raw,
                                                        uint16_t *y_raw,
                                                        uint16_t *pressure)
{
    uint16_t x;
    uint16_t y;
    uint16_t z1;
    uint16_t z2;

    /* Read X position */
    if (XPT2046_ReadADC(hxpt, XPT2046_CMD_X_POS, &x) != XPT2046_OK) {
        return XPT2046_ERROR;
    }

    /* Read Y position */
    if (XPT2046_ReadADC(hxpt, XPT2046_CMD_Y_POS, &y) != XPT2046_OK) {
        return XPT2046_ERROR;
    }

    /* Read pressure measurements (Z1 and Z2) */
    if (XPT2046_ReadADC(hxpt, XPT2046_CMD_Z1_POS, &z1) != XPT2046_OK) {
        return XPT2046_ERROR;
    }

    if (XPT2046_ReadADC(hxpt, XPT2046_CMD_Z2_POS, &z2) != XPT2046_OK) {
        return XPT2046_ERROR;
    }

    /* Calculate pressure from Z1 and Z2 */
    *pressure = XPT2046_CalculatePressure(x, z1, z2);

    *x_raw = x;
    *y_raw = y;

    return XPT2046_OK;
}

/**
 * @brief   Apply calibration to convert raw ADC values to screen coordinates
 * @param   hxpt Pointer to XPT2046 handle
 * @param   x_raw Raw X ADC value
 * @param   y_raw Raw Y ADC value
 * @param   x_cal Pointer to store calibrated X coordinate
 * @param   y_cal Pointer to store calibrated Y coordinate
 */
static void XPT2046_ApplyCalibration(XPT2046_Handle_t *hxpt,
                                    uint16_t x_raw, uint16_t y_raw,
                                    uint16_t *x_cal, uint16_t *y_cal)
{
    /* Linear mapping from ADC range to display coordinates */
    int32_t x_range = hxpt->cal.x_max - hxpt->cal.x_min;
    int32_t y_range = hxpt->cal.y_max - hxpt->cal.y_min;

    if (x_range <= 0) x_range = 1; // Prevent division by zero
    if (y_range <= 0) y_range = 1;

    /* Map X */
    int32_t x_temp = ((int32_t)x_raw - hxpt->cal.x_min) * hxpt->config.width / x_range;
    if (x_temp < 0) x_temp = 0;
    if (x_temp >= hxpt->config.width) x_temp = hxpt->config.width - 1;

    /* Map Y */
    int32_t y_temp = ((int32_t)y_raw - hxpt->cal.y_min) * hxpt->config.height / y_range;
    if (y_temp < 0) y_temp = 0;
    if (y_temp >= hxpt->config.height) y_temp = hxpt->config.height - 1;

    *x_cal = (uint16_t)x_temp;
    *y_cal = (uint16_t)y_temp;
}

/**
 * @brief   Calculate touch pressure from Z measurements
 * @param   x X position (needed for pressure calculation)
 * @param   z1 Z1 ADC value
 * @param   z2 Z2 ADC value
 * @retval  uint16_t Calculated pressure value
 */
static uint16_t XPT2046_CalculatePressure(uint16_t x, uint16_t z1, uint16_t z2)
{
    if (z1 == 0 || z2 == 0) {
        return 0;
    }

    /*
     * Pressure formula from XPT2046 datasheet:
     * P = (X_position / 4096) * ((Z2 / Z1) - 1)
     * We simplify and scale appropriately
     */
    uint32_t pressure = (uint32_t)x * (z2 - z1) / z1;

    /* Clamp to maximum */
    if (pressure > XPT2046_MAX_PRESSURE) {
        pressure = XPT2046_MAX_PRESSURE;
    }

    return (uint16_t)pressure;
}

/**
 * @brief   Median filter for noise rejection
 * @param   samples Array of samples
 * @param   count Number of samples
 * @retval  uint16_t Median value
 */
static uint16_t XPT2046_MedianFilter(uint16_t *samples, uint8_t count)
{
    /* Simple bubble sort */
    for (uint8_t i = 0; i < count - 1; i++) {
        for (uint8_t j = 0; j < count - i - 1; j++) {
            if (samples[j] > samples[j + 1]) {
                uint16_t temp = samples[j];
                samples[j] = samples[j + 1];
                samples[j + 1] = temp;
            }
        }
    }

    /* Return middle value */
    return samples[count / 2];
}

/**
 * @brief   Microsecond delay
 * @param   us Delay in microseconds
 */
static void XPT2046_DelayUs(uint32_t us)
{
    /* Approximate delay based on CPU clock */
    volatile uint32_t count = us * (SystemCoreClock / 1000000) / 4;
    while (count--) {
        __NOP();
    }
}

/**
 * @brief   GPIO EXTI callback (optional - for interrupt-driven touch)
 * @param   GPIO_Pin Pin that triggered interrupt
 * @note    Implement this if using interrupt mode instead of polling
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_15) {
        /* Touch interrupt on PA15 */
        log_debug("Touch interrupt detected");
        /* In interrupt-driven mode, you could set a flag here to read touch in main loop */
        /* For now, just log the event */
    }
}
