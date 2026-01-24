/**
  ******************************************************************************
  * @file    xpt2046.c
  * @brief   XPT2046 Resistive Touchscreen Controller Driver Implementation
  * @details This file provides the implementation of XPT2046 touchscreen functions
  *          using SPI interface on STM32F429I-DISC1.
  * @version 1.0
  * @date    2025-01-19
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "xpt2046.h"
#include "spi.h"
#include "gpio.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Private defines -----------------------------------------------------------*/

/** @defgroup XPT2046_Private_Defines Private Defines
 * @{
 */

/* XPT2046 Commands */
#define XPT2046_CMD_READ_X            0x90    /**< Read X coordinate */
#define XPT2046_CMD_READ_Y            0xD0    /**< Read Y coordinate */
#define XPT2046_CMD_READ_Z1           0xB0    /**< Read Z1 (pressure) */
#define XPT2046_CMD_READ_Z2           0xC0    /**< Read Z2 (pressure) */
#define XPT2046_CMD_START_BIT         0x80    /**< Start bit for commands */

/* XPT2046 Configuration */
#define XPT2046_POWER_MODE            0x00    /**< Power mode */
#define XPT2046_REF_MODE              0x04    /**< Reference voltage mode */
#define XPT2046_ADC_MODE              0x08    /**< ADC resolution mode */

/* Timing constants */
#define XPT2046_TOUCH_DELAY           1       /**< Delay between touch checks (ms) */
#define XPT2046_READ_DELAY            10      /**< Delay for ADC conversion (us) */
#define XPT2046_DEBOUNCE_COUNT        3       /**< Debounce sample count */

/* Calibration constants */
#define XPT2046_CAL_POINTS            5       /**< Number of calibration points */

/** @} */

/* Private types ------------------------------------------------------------*/

/**
 * @brief Calibration point structure
 */
typedef struct {
    uint16_t x_display;
    uint16_t y_display;
    uint16_t x_touch;
    uint16_t y_touch;
} XPT2046_CalPoint_t;

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static XPT2046_StatusTypeDef XPT2046_ReadADC(XPT2046_Handle_t *hxpt, uint8_t command, uint16_t *value);
static XPT2046_StatusTypeDef XPT2046_ReadCoordinates(XPT2046_Handle_t *hxpt, uint16_t *x, uint16_t *y, uint16_t *pressure);
static void XPT2046_MapCoordinates(XPT2046_Handle_t *hxpt, uint16_t *x, uint16_t *y);
static uint16_t XPT2046_CalculatePressure(uint16_t z1, uint16_t z2);
static void XPT2046_DelayUs(uint32_t delay);

/* Exported functions -------------------------------------------------------*/

/**
 * @brief   Initialize XPT2046 touchscreen controller
 * @details Configures SPI and initializes the touchscreen
 * @param   hxpt Pointer to XPT2046 handle
 * @param   cs_port Chip select port
 * @param   cs_pin Chip select pin
 * @param   irq_port Interrupt port
 * @param   irq_pin Interrupt pin
 * @param   width Display width
 * @param   height Display height
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

    /* Default calibration matrix (identity) */
    hxpt->config.calibration[0] = 1;   // x_scale
    hxpt->config.calibration[1] = 0;   // x_offset
    hxpt->config.calibration[2] = 0;   // y_x_coeff
    hxpt->config.calibration[3] = 0;   // y_scale
    hxpt->config.calibration[4] = 1;   // y_offset
    hxpt->config.calibration[5] = 0;   // x_y_coeff
    hxpt->config.calibration[6] = 1;   // divisor

    /* Configure GPIO pins */
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Chip select pin */
    GPIO_InitStruct.Pin = cs_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(cs_port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET); // Deselect

    /* Interrupt pin */
    GPIO_InitStruct.Pin = irq_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(irq_port, &GPIO_InitStruct);

    hxpt->initialized = true;

    return XPT2046_OK;
}

/**
 * @brief   Check if touchscreen is touched
 * @param   hxpt Pointer to XPT2046 handle
 * @retval  bool True if touched, false otherwise
 */
bool XPT2046_IsTouched(XPT2046_Handle_t *hxpt)
{
    if (hxpt == NULL || !hxpt->initialized) {
        return false;
    }

    /* Check interrupt pin - active low when touched */
    return (HAL_GPIO_ReadPin(hxpt->config.irq_port, hxpt->config.irq_pin) == GPIO_PIN_RESET);
}

/**
 * @brief   Read touch coordinates and pressure
 * @param   hxpt Pointer to XPT2046 handle
 * @param   touch Pointer to touch point structure
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_ReadTouch(XPT2046_Handle_t *hxpt, XPT2046_TouchPoint_t *touch)
{
    if (hxpt == NULL || touch == NULL || !hxpt->initialized) {
        return XPT2046_INVALID_PARAM;
    }

    if (!XPT2046_IsTouched(hxpt)) {
        touch->state = XPT2046_STATE_RELEASED;
        touch->pressure = 0;
        return XPT2046_NO_TOUCH;
    }

    uint16_t x = 0;
    uint16_t y = 0;
    uint16_t pressure = 0;
    XPT2046_StatusTypeDef status = XPT2046_ReadCoordinates(hxpt, &x, &y, &pressure);

    if (status != XPT2046_OK) {
        return status;
    }

    /* Map coordinates using calibration */
    XPT2046_MapCoordinates(hxpt, &x, &y);

    /* Apply flipping if configured */
    if (hxpt->config.flip_x) {
        x = hxpt->config.width - 1 - x;
    }
    if (hxpt->config.flip_y) {
        y = hxpt->config.height - 1 - y;
    }

    /* Update touch structure */
    touch->x = x;
    touch->y = y;
    touch->pressure = pressure;
    touch->state = XPT2046_STATE_PRESSED;

    return XPT2046_OK;
}

/**
 * @brief   Update touch state
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
        /* Touch detected */
        if (hxpt->touch.state == XPT2046_STATE_RELEASED) {
            /* New touch */
            hxpt->touch = new_touch;
            hxpt->touch.state = XPT2046_STATE_PRESSED;
        } else {
            /* Continued touch */
            hxpt->touch.x = new_touch.x;
            hxpt->touch.y = new_touch.y;
            hxpt->touch.pressure = new_touch.pressure;
            hxpt->touch.state = XPT2046_STATE_HELD;
        }
        hxpt->last_touch_time = HAL_GetTick();
    } else if (status == XPT2046_NO_TOUCH) {
        /* No touch */
        if (hxpt->touch.state != XPT2046_STATE_RELEASED) {
            /* Touch released */
            hxpt->touch.state = XPT2046_STATE_RELEASED;
            hxpt->touch.pressure = 0;
        }
    } else {
        return status;
    }

    return XPT2046_OK;
}

/**
 * @brief   Set calibration matrix
 * @param   hxpt Pointer to XPT2046 handle
 * @param   calibration Calibration matrix (7 values)
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_SetCalibration(XPT2046_Handle_t *hxpt, uint16_t *calibration)
{
    if (hxpt == NULL || calibration == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    memcpy(hxpt->config.calibration, calibration, sizeof(hxpt->config.calibration));

    return XPT2046_OK;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Read ADC value from XPT2046
 * @param   hxpt Pointer to XPT2046 handle
 * @param   command ADC command
 * @param   value Pointer to store ADC value
 * @retval  XPT2046_StatusTypeDef Operation status
 */
static XPT2046_StatusTypeDef XPT2046_ReadADC(XPT2046_Handle_t *hxpt, uint8_t command, uint16_t *value)
{
    uint8_t tx_data[3] = {command, 0x00, 0x00};
    uint8_t rx_data[3] = {0};

    HAL_GPIO_WritePin(hxpt->config.cs_port, hxpt->config.cs_pin, GPIO_PIN_RESET);
    XPT2046_DelayUs(XPT2046_READ_DELAY);

    if (SPI_TransmitReceive(tx_data, rx_data, 3, SPI_TIMEOUT_SHORT) != SPI_OK) {
        HAL_GPIO_WritePin(hxpt->config.cs_port, hxpt->config.cs_pin, GPIO_PIN_SET);
        return XPT2046_ERROR;
    }

    HAL_GPIO_WritePin(hxpt->config.cs_port, hxpt->config.cs_pin, GPIO_PIN_SET);

    /* Convert received data to 12-bit value */
    *value = ((rx_data[1] & 0x7F) << 5) | (rx_data[2] >> 3);

    return XPT2046_OK;
}

/**
 * @brief   Read coordinates and pressure
 * @param   hxpt Pointer to XPT2046 handle
 * @param   x Pointer to X coordinate
 * @param   y Pointer to Y coordinate
 * @param   pressure Pointer to pressure value
 * @retval  XPT2046_StatusTypeDef Operation status
 */
static XPT2046_StatusTypeDef XPT2046_ReadCoordinates(XPT2046_Handle_t *hxpt, uint16_t *x, uint16_t *y, uint16_t *pressure)
{
    uint16_t z1 = 0;
    uint16_t z2 = 0;

    /* Read X coordinate */
    if (XPT2046_ReadADC(hxpt, XPT2046_CMD_READ_X, x) != XPT2046_OK) {
        return XPT2046_ERROR;
    }

    /* Read Y coordinate */
    if (XPT2046_ReadADC(hxpt, XPT2046_CMD_READ_Y, y) != XPT2046_OK) {
        return XPT2046_ERROR;
    }

    /* Read pressure values */
    if (XPT2046_ReadADC(hxpt, XPT2046_CMD_READ_Z1, &z1) != XPT2046_OK) {
        return XPT2046_ERROR;
    }

    if (XPT2046_ReadADC(hxpt, XPT2046_CMD_READ_Z2, &z2) != XPT2046_OK) {
        return XPT2046_ERROR;
    }

    *pressure = XPT2046_CalculatePressure(z1, z2);

    return XPT2046_OK;
}

/**
 * @brief   Map coordinates using calibration matrix
 * @param   hxpt Pointer to XPT2046 handle
 * @param   x Pointer to X coordinate
 * @param   y Pointer to Y coordinate
 */
static void XPT2046_MapCoordinates(XPT2046_Handle_t *hxpt, uint16_t *x, uint16_t *y)
{
    /* Simple linear mapping for now - can be enhanced with full calibration matrix */
    uint16_t raw_x = *x;
    uint16_t raw_y = *y;

    /* Map to display coordinates */
    *x = (raw_x * hxpt->config.width) / XPT2046_MAX_X;
    *y = (raw_y * hxpt->config.height) / XPT2046_MAX_Y;

    /* Clamp to display bounds */
    if (*x >= hxpt->config.width) *x = hxpt->config.width - 1;
    if (*y >= hxpt->config.height) *y = hxpt->config.height - 1;
}

/**
 * @brief   Calculate touch pressure
 * @param   z1 Z1 ADC value
 * @param   z2 Z2 ADC value
 * @retval  uint16_t Pressure value
 */
static uint16_t XPT2046_CalculatePressure(uint16_t z1, uint16_t z2)
{
    if (z2 == 0) return 0;

    /* Pressure calculation: P = (Z1 / Z2 - 1) * scale */
    /* Simplified pressure calculation */
    uint16_t pressure = (z1 * 1000) / z2;

    if (pressure > XPT2046_MAX_PRESSURE) pressure = XPT2046_MAX_PRESSURE;

    return pressure;
}

/**
 * @brief   Microsecond delay
 * @param   delay Delay in microseconds
 */
static void XPT2046_DelayUs(uint32_t delay)
{
    volatile uint32_t count = delay * (SystemCoreClock / 1000000) / 4;
    while (count--) {
        __NOP();
    }
}
