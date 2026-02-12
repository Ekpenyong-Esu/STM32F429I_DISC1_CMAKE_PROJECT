/**
  ******************************************************************************
  * @file    xpt2046.c
  * @brief   XPT2046 Resistive Touchscreen driver implementation for STM32F429 Discovery Board
  * @details This file provides the implementation of XPT2046 touchscreen functions
  *          using SPI interface. Based on touchscreen.c pattern for STMPE811.
  * @version 1.0
  * @date    2025-02-11
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "xpt2046.h"
#include "spi.h"
#include "stdbool.h"
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

/* Private constants ---------------------------------------------------------*/
#define XPT2046_DELAY_MS(x)             HAL_Delay(x)
#define XPT2046_DELAY_US(x)             do { \
    uint32_t start = DWT->CYCCNT; \
    uint32_t ticks = (SystemCoreClock / 1000000U) * (x); \
    while ((DWT->CYCCNT - start) < ticks); \
} while(0)

/* SPI communication timing */
#define XPT2046_CS_DELAY_US             1       /* Delay after CS assertion */
#define XPT2046_CONVERSION_DELAY_US     10      /* ADC conversion delay */
#define XPT2046_SETTLING_DELAY_US       50      /* Settling time between reads */

/* Private variables ---------------------------------------------------------*/
/* Global touchscreen handle */
XPT2046_HandleTypeDef *g_hxpt = NULL;
/* Deferred EXTI handling flag */
static volatile bool s_xpt_irq_pending = false;

/* Private function prototypes -----------------------------------------------*/
static XPT2046_StatusTypeDef XPT2046_ReadRawCoordinates(XPT2046_HandleTypeDef *hxpt,
                                                        uint16_t *raw_x,
                                                        uint16_t *raw_y,
                                                        uint16_t *pressure);
static XPT2046_StatusTypeDef XPT2046_ConvertCoordinates(XPT2046_HandleTypeDef *hxpt,
                                                        uint16_t raw_x,
                                                        uint16_t raw_y,
                                                        uint16_t *disp_x,
                                                        uint16_t *disp_y);
static void XPT2046_FilterCoordinates(uint16_t *x, uint16_t *y);
static XPT2046_GestureTypeDef XPT2046_AnalyzeGesture(XPT2046_HandleTypeDef *hxpt);
static uint16_t XPT2046_ReadChannel(XPT2046_HandleTypeDef *hxpt, uint8_t channel);
static uint16_t XPT2046_ReadChannelFiltered(XPT2046_HandleTypeDef *hxpt,
                                            uint8_t channel,
                                            uint8_t samples);
static int32_t map(int32_t val, int32_t in_min, int32_t in_max,
                  int32_t out_min, int32_t out_max);
static void XPT2046_CS_Low(XPT2046_HandleTypeDef *hxpt);
static void XPT2046_CS_High(XPT2046_HandleTypeDef *hxpt);

/* Private helper functions implementation -----------------------------------*/

/**
 * @brief   Assert chip select (active low)
 * @param   hxpt Pointer to XPT2046 handle
 */
static void XPT2046_CS_Low(XPT2046_HandleTypeDef *hxpt)
{
    HAL_GPIO_WritePin(hxpt->CS_Port, hxpt->CS_Pin, GPIO_PIN_RESET);
    XPT2046_DELAY_US(XPT2046_CS_DELAY_US);
}

/**
 * @brief   Deassert chip select
 * @param   hxpt Pointer to XPT2046 handle
 */
static void XPT2046_CS_High(XPT2046_HandleTypeDef *hxpt)
{
    HAL_GPIO_WritePin(hxpt->CS_Port, hxpt->CS_Pin, GPIO_PIN_SET);
}

/**
 * @brief   Linear mapping with input clamping
 * @param   val Input value
 * @param   in_min Input minimum
 * @param   in_max Input maximum
 * @param   out_min Output minimum
 * @param   out_max Output maximum
 * @retval  Mapped value
 */
static int32_t map(int32_t val, int32_t in_min, int32_t in_max,
                  int32_t out_min, int32_t out_max)
{
    if (in_max == in_min) return out_min;

    /* Clamp input to expected range */
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
 * @brief   Initialize XPT2046 touchscreen system
 * @param   hxpt Pointer to XPT2046 handle structure
 * @param   hspi Pointer to SPI handle
 * @param   cs_port Chip select GPIO port
 * @param   cs_pin Chip select GPIO pin
 * @param   irq_port Interrupt GPIO port
 * @param   irq_pin Interrupt GPIO pin
 * @retval  XPT2046_StatusTypeDef Status of the operation
 */
XPT2046_StatusTypeDef XPT2046_Init(XPT2046_HandleTypeDef *hxpt,
                                  SPI_HandleTypeDef *hspi,
                                  GPIO_TypeDef *cs_port, uint16_t cs_pin,
                                  GPIO_TypeDef *irq_port, uint16_t irq_pin)
{
    XPT2046_StatusTypeDef status = XPT2046_OK;

    /* Check parameters */
    if (hxpt == NULL || hspi == NULL || cs_port == NULL || irq_port == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    /* Initialize structure */
    memset(hxpt, 0, sizeof(XPT2046_HandleTypeDef));
    hxpt->hspi = hspi;
    hxpt->CS_Port = cs_port;
    hxpt->CS_Pin = cs_pin;
    hxpt->IRQ_Port = irq_port;
    hxpt->IRQ_Pin = irq_pin;
    g_hxpt = hxpt;

    /* SPI handingle must be pre-initialized by application (via SPI_Init() in main) */
   if (hxpt->hspi->Instance == NULL) {
        return XPT2046_ERROR;  /* SPI not initialized */
    }

    /* Initialize MSP (GPIO, ingclocks) */
    XPT2046_MspInit(cs_port, cs_pin, irq_port, irq_pin);

    /* Set CS high (inactive) */
    XPT2046_CS_High(hxpt);

    /* Set default configuration */
    XPT2046_ConfigTypeDef default_config = XPT2046_GetDefaultConfig();
    status = XPT2046_Configure(hxpt, &default_config);
    if (status != XPT2046_OK) {
        return status;
    }

    /* Set default calibration */
    XPT2046_CalibrationTypeDef default_cal = {
        .MinX = XPT2046_RAW_X_MIN,
        .MaxX = XPT2046_RAW_X_MAX,
        .MinY = XPT2046_RAW_Y_MIN,
        .MaxY = XPT2046_RAW_Y_MAX,
        .ScaleX = (float)XPT2046_DISPLAY_WIDTH / (float)(XPT2046_RAW_X_MAX - XPT2046_RAW_X_MIN),
        .ScaleY = (float)XPT2046_DISPLAY_HEIGHT / (float)(XPT2046_RAW_Y_MAX - XPT2046_RAW_Y_MIN),
        .OffsetX = -XPT2046_RAW_X_MIN,
        .OffsetY = -XPT2046_RAW_Y_MIN,
        .SwapXY = false,
        .FlipX = false,
        .FlipY = false,
        .IsCalibrated = false
    };
    XPT2046_SetCalibration(hxpt, &default_cal);

    /* Configure interrupts if enabled */
    if (hxpt->Config.InterruptEnable) {
        XPT2046_EnableInterrupt(hxpt, true);
        XPT2046_ITConfig(hxpt);
    }

    hxpt->IsInitialized = true;
    return XPT2046_OK;
}

/**
 * @brief   Deinitialize XPT2046 touchscreen system
 * @param   hxpt Pointer to XPT2046 handle structure
 * @retval  XPT2046_StatusTypeDef Status of the operation
 */
XPT2046_StatusTypeDef XPT2046_DeInit(XPT2046_HandleTypeDef *hxpt)
{
    if (hxpt == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    /* Disable interrupts */
    XPT2046_EnableInterrupt(hxpt, false);

    /* Deinitialize MSP */
    XPT2046_MspDeInit(hxpt->CS_Port, hxpt->CS_Pin,
                     hxpt->IRQ_Port, hxpt->IRQ_Pin);

    /* Reset structure */
    hxpt->IsInitialized = false;
    g_hxpt = NULL;

    return XPT2046_OK;
}

/**
 * @brief   Configure XPT2046 touchscreen parameters
 * @param   hxpt Pointer to XPT2046 handle structure
 * @param   config Pointer to configuration structure
 * @retval  XPT2046_StatusTypeDef Status of the operation
 */
XPT2046_StatusTypeDef XPT2046_Configure(XPT2046_HandleTypeDef *hxpt,
                                       XPT2046_ConfigTypeDef *config)
{
    if (hxpt == NULL || config == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    /* Store configuration */
    hxpt->Config = *config;

    return XPT2046_OK;
}

/**
 * @brief   Reset XPT2046 touchscreen controller
 * @param   hxpt Pointer to XPT2046 handle structure
 * @retval  XPT2046_StatusTypeDef Status of the operation
 * @note    XPT2046 doesn't have software reset, this clears state
 */
XPT2046_StatusTypeDef XPT2046_Reset(XPT2046_HandleTypeDef *hxpt)
{
    if (hxpt == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    /* Clear touch data */
    memset(&hxpt->TouchData, 0, sizeof(XPT2046_TouchDataTypeDef));
    memset(&hxpt->PrevTouchData, 0, sizeof(XPT2046_TouchDataTypeDef));
    hxpt->LastTouchTime = 0;

    /* Power cycle by reading with power-down mode */
    XPT2046_CS_Low(hxpt);
    uint8_t cmd = XPT2046_CMD_READ_X | XPT2046_CMD_POWERDOWN_DISABLE;
    HAL_SPI_Transmit(hxpt->hspi, &cmd, 1, XPT2046_TIMEOUT);
    XPT2046_CS_High(hxpt);

    XPT2046_DELAY_MS(10);

    return XPT2046_OK;
}

/**
 * @brief   Get current touch data
 * @param   hxpt Pointer to XPT2046 handle structure
 * @param   touch_data Pointer to store touch data
 * @retval  XPT2046_StatusTypeDef Status of the operation
 */
XPT2046_StatusTypeDef XPT2046_GetTouchData(XPT2046_HandleTypeDef *hxpt,
                                          XPT2046_TouchDataTypeDef *touch_data)
{
    if (hxpt == NULL || touch_data == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    *touch_data = hxpt->TouchData;

    return XPT2046_OK;
}

/**
 * @brief   Get single touch coordinates
 * @param   hxpt Pointer to XPT2046 handle structure
 * @param   xPos Pointer to store X coordinate
 * @param   yPos Pointer to store Y coordinate
 * @retval  XPT2046_StatusTypeDef Status of the operation
 */
XPT2046_StatusTypeDef XPT2046_GetSingleTouch(XPT2046_HandleTypeDef *hxpt,
                                            uint16_t *xPos,
                                            uint16_t *yPos)
{
    uint16_t raw_x = 0;
    uint16_t raw_y = 0;
    uint16_t disp_x = 0;
    uint16_t disp_y = 0;

    if (hxpt == NULL || xPos == NULL || yPos == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    *xPos = 0;
    *yPos = 0;

    if (!hxpt->IsInitialized) {
        return XPT2046_NOT_INITIALIZED;
    }

    /* Check if touched */
    if (!XPT2046_IsTouched(hxpt)) {
        return XPT2046_NO_TOUCH;
    }

    /* Read raw coordinates */
    if (XPT2046_ReadRawCoordinates(hxpt, &raw_x, &raw_y, NULL) != XPT2046_OK) {
        return XPT2046_ERROR;
    }

    /* Convert to display coordinates */
    if (XPT2046_ConvertCoordinates(hxpt, raw_x, raw_y, &disp_x, &disp_y) != XPT2046_OK) {
        return XPT2046_ERROR;
    }

    /* Apply filtering */
    XPT2046_FilterCoordinates(&disp_x, &disp_y);

    *xPos = disp_x;
    *yPos = disp_y;

    return XPT2046_OK;
}

/**
 * @brief   Get touch state with coordinates and pressed status
 * @param   hxpt Pointer to XPT2046 handle structure
 * @param   x Pointer to store X coordinate
 * @param   y Pointer to store Y coordinate
 * @param   pressed Pointer to store pressed status
 * @retval  XPT2046_StatusTypeDef Status of the operation
 */
XPT2046_StatusTypeDef XPT2046_GetTouchState(XPT2046_HandleTypeDef *hxpt,
                                           uint16_t *x,
                                           uint16_t *y,
                                           uint8_t *pressed)
{
    uint16_t raw_x = 0;
    uint16_t raw_y = 0;
    uint16_t disp_x = 0;
    uint16_t disp_y = 0;

    if (hxpt == NULL || x == NULL || y == NULL || pressed == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    *pressed = 0;
    *x = 0;
    *y = 0;

    if (!hxpt->IsInitialized) {
        return XPT2046_NOT_INITIALIZED;
    }

    /* Check if touched */
    if (!XPT2046_IsTouched(hxpt)) {
        return XPT2046_OK;  /* Not an error, just no touch */
    }

    /* Read raw coordinates */
    if (XPT2046_ReadRawCoordinates(hxpt, &raw_x, &raw_y, NULL) != XPT2046_OK) {
        return XPT2046_OK;  /* Return OK with no touch on read error */
    }

    /* Convert to display coordinates */
    if (XPT2046_ConvertCoordinates(hxpt, raw_x, raw_y, &disp_x, &disp_y) != XPT2046_OK) {
        return XPT2046_OK;
    }

    /* Apply filtering */
    XPT2046_FilterCoordinates(&disp_x, &disp_y);

    *x = disp_x;
    *y = disp_y;
    *pressed = 1;

    return XPT2046_OK;
}

/**
 * @brief   Check if touchscreen is currently touched
 * @param   hxpt Pointer to XPT2046 handle structure
 * @retval  bool True if touched, false otherwise
 */
bool XPT2046_IsTouched(XPT2046_HandleTypeDef *hxpt)
{
    if (hxpt == NULL || !hxpt->IsInitialized) {
        return false;
    }

    /* Check interrupt pin - active low when touched */
    GPIO_PinState irq_state = HAL_GPIO_ReadPin(hxpt->IRQ_Port, hxpt->IRQ_Pin);
    return (irq_state == GPIO_PIN_RESET);
}

/**
 * @brief   Get number of active touches
 * @param   hxpt Pointer to XPT2046 handle structure
 * @retval  uint8_t Number of touches (0 or 1)
 */
uint8_t XPT2046_GetTouchCount(XPT2046_HandleTypeDef *hxpt)
{
    if (hxpt == NULL) {
        return 0;
    }

    return hxpt->TouchData.TouchCount;
}

/**
 * @brief   Read touch data from XPT2046
 * @param   hxpt Pointer to XPT2046 handle structure
 * @retval  XPT2046_StatusTypeDef Status of the operation
 */
XPT2046_StatusTypeDef XPT2046_ReadTouchData(XPT2046_HandleTypeDef *hxpt)
{
    if (hxpt == NULL || !hxpt->IsInitialized) {
        return XPT2046_NOT_INITIALIZED;
    }

    /* Save previous touch data */
    hxpt->PrevTouchData = hxpt->TouchData;

    /* Check if touched */
    if (!XPT2046_IsTouched(hxpt)) {
        hxpt->TouchData.TouchCount = 0;
        hxpt->TouchData.Points[0].State = XPT2046_TOUCH_RELEASED;
        return XPT2046_NO_TOUCH;
    }

    uint16_t raw_x = 0;
    uint16_t raw_y = 0;
    uint16_t pressure = 0;
    uint16_t disp_x = 0;
    uint16_t disp_y = 0;

    /* Read raw coordinates and pressure */
    if (XPT2046_ReadRawCoordinates(hxpt, &raw_x, &raw_y, &pressure) != XPT2046_OK) {
        return XPT2046_ERROR;
    }

    /* Check pressure threshold */
    if (pressure < hxpt->Config.PressureThreshold) {
        hxpt->TouchData.TouchCount = 0;
        hxpt->TouchData.Points[0].State = XPT2046_TOUCH_RELEASED;
        return XPT2046_NO_TOUCH;
    }

    /* Convert to display coordinates */
    if (XPT2046_ConvertCoordinates(hxpt, raw_x, raw_y, &disp_x, &disp_y) != XPT2046_OK) {
        return XPT2046_ERROR;
    }

    /* Apply filtering */
    XPT2046_FilterCoordinates(&disp_x, &disp_y);

    /* Update touch data */
    hxpt->TouchData.TouchCount = 1;
    hxpt->TouchData.Points[0].X = disp_x;
    hxpt->TouchData.Points[0].Y = disp_y;
    hxpt->TouchData.Points[0].Z = pressure;
    hxpt->TouchData.Points[0].RawX = raw_x;
    hxpt->TouchData.Points[0].RawY = raw_y;
    hxpt->TouchData.Points[0].Timestamp = HAL_GetTick();

    /* Determine touch state */
    if (hxpt->PrevTouchData.TouchCount == 0) {
        hxpt->TouchData.Points[0].State = XPT2046_TOUCH_PRESSED;
    } else {
        hxpt->TouchData.Points[0].State = XPT2046_TOUCH_MOVING;
    }

    hxpt->LastTouchTime = HAL_GetTick();

    return XPT2046_OK;
}

/**
 * @brief   Calibrate touchscreen
 * @param   hxpt Pointer to XPT2046 handle structure
 * @retval  XPT2046_StatusTypeDef Status of the operation
 */
XPT2046_StatusTypeDef XPT2046_Calibrate(XPT2046_HandleTypeDef *hxpt)
{
    if (hxpt == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    /* Basic calibration - in a real implementation, this would
       involve displaying calibration points and collecting user input */

    hxpt->Calibration.MinX = XPT2046_RAW_X_MIN;
    hxpt->Calibration.MaxX = XPT2046_RAW_X_MAX;
    hxpt->Calibration.MinY = XPT2046_RAW_Y_MIN;
    hxpt->Calibration.MaxY = XPT2046_RAW_Y_MAX;
    hxpt->Calibration.ScaleX = (float)XPT2046_DISPLAY_WIDTH /
                              (float)(hxpt->Calibration.MaxX - hxpt->Calibration.MinX);
    hxpt->Calibration.ScaleY = (float)XPT2046_DISPLAY_HEIGHT /
                              (float)(hxpt->Calibration.MaxY - hxpt->Calibration.MinY);
    hxpt->Calibration.OffsetX = (int16_t)(-hxpt->Calibration.MinX);
    hxpt->Calibration.OffsetY = (int16_t)(-hxpt->Calibration.MinY);
    hxpt->Calibration.IsCalibrated = true;

    return XPT2046_OK;
}

/**
 * @brief   Set calibration data
 * @param   hxpt Pointer to XPT2046 handle structure
 * @param   calibration Pointer to calibration data
 * @retval  XPT2046_StatusTypeDef Status of the operation
 */
XPT2046_StatusTypeDef XPT2046_SetCalibration(XPT2046_HandleTypeDef *hxpt,
                                            XPT2046_CalibrationTypeDef *calibration)
{
    if (hxpt == NULL || calibration == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    hxpt->Calibration = *calibration;

    return XPT2046_OK;
}

/**
 * @brief   Get calibration data
 * @param   hxpt Pointer to XPT2046 handle structure
 * @param   calibration Pointer to store calibration data
 * @retval  XPT2046_StatusTypeDef Status of the operation
 */
XPT2046_StatusTypeDef XPT2046_GetCalibration(XPT2046_HandleTypeDef *hxpt,
                                            XPT2046_CalibrationTypeDef *calibration)
{
    if (hxpt == NULL || calibration == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    *calibration = hxpt->Calibration;

    return XPT2046_OK;
}

/**
 * @brief   Enable/disable interrupt
 * @param   hxpt Pointer to XPT2046 handle structure
 * @param   enable Enable/disable flag
 * @retval  XPT2046_StatusTypeDef Status of the operation
 */
XPT2046_StatusTypeDef XPT2046_EnableInterrupt(XPT2046_HandleTypeDef *hxpt,
                                             bool enable)
{
    if (hxpt == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    hxpt->InterruptMode = enable;

    /* Interrupt is configured in MSP init */
    /* Enable/disable NVIC if needed */
    if (enable) {
        HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
    } else {
        HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
    }

    return XPT2046_OK;
}

/**
 * @brief   Configure interrupt
 * @param   hxpt Pointer to XPT2046 handle structure
 * @retval  XPT2046_StatusTypeDef Status of the operation
 * @note    GPIO configuration is done in XPT2046_MspInit
 */
XPT2046_StatusTypeDef XPT2046_ITConfig(XPT2046_HandleTypeDef *hxpt)
{
    if (hxpt == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    /* NVIC configuration - already done in MspInit, but ensure enabled */
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

    return XPT2046_OK;
}

/**
 * @brief   Interrupt handler
 * @param   hxpt Pointer to XPT2046 handle structure
 */
void XPT2046_IRQHandler(XPT2046_HandleTypeDef *hxpt)
{
    if (hxpt == NULL) {
        return;
    }

    /* Read touch data */
    XPT2046_ReadTouchData(hxpt);

    /* Call callbacks if registered */
    if (hxpt->TouchData.TouchCount > 0) {
        if (hxpt->TouchData.Points[0].State == XPT2046_TOUCH_PRESSED &&
            hxpt->TouchCallback != NULL) {
            hxpt->TouchCallback();
        }
    } else {
        if (hxpt->PrevTouchData.TouchCount > 0 && hxpt->ReleaseCallback != NULL) {
            hxpt->ReleaseCallback();
        }
    }

    /* Detect gestures if needed */
    XPT2046_GestureTypeDef gesture = XPT2046_AnalyzeGesture(hxpt);
    if (gesture != XPT2046_GESTURE_NONE && hxpt->GestureCallback != NULL) {
        hxpt->GestureCallback(gesture);
    }
}

/**
 * @brief   Service pending touchscreen IRQ outside ISR context
 * @details Handles deferred interrupt processing
 */
void XPT2046_ServiceIRQ(void)
{
    if (s_xpt_irq_pending && g_hxpt != NULL && g_hxpt->IsInitialized)
    {
        s_xpt_irq_pending = false;
        XPT2046_IRQHandler(g_hxpt);
    }
}

/**
 * @brief   Register callback functions
 * @param   hxpt Pointer to XPT2046 handle structure
 * @param   touch_callback Touch detected callback
 * @param   release_callback Touch released callback
 * @param   gesture_callback Gesture detected callback
 * @retval  XPT2046_StatusTypeDef Status of the operation
 */
XPT2046_StatusTypeDef XPT2046_RegisterCallbacks(XPT2046_HandleTypeDef *hxpt,
                                               void (*touch_callback)(void),
                                               void (*release_callback)(void),
                                               void (*gesture_callback)(XPT2046_GestureTypeDef))
{
    if (hxpt == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    hxpt->TouchCallback = touch_callback;
    hxpt->ReleaseCallback = release_callback;
    hxpt->GestureCallback = gesture_callback;

    return XPT2046_OK;
}

/**
 * @brief   Get pressure value
 * @param   hxpt Pointer to XPT2046 handle structure
 * @param   pressure Pointer to store pressure value
 * @retval  XPT2046_StatusTypeDef Status of the operation
 */
XPT2046_StatusTypeDef XPT2046_GetPressure(XPT2046_HandleTypeDef *hxpt,
                                         uint16_t *pressure)
{
    if (hxpt == NULL || pressure == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    if (!hxpt->IsInitialized) {
        return XPT2046_NOT_INITIALIZED;
    }

    /* Read Z1 and Z2 for pressure calculation */
    uint16_t z1 = XPT2046_ReadChannelFiltered(hxpt, XPT2046_CMD_READ_Z1, 3);
    uint16_t z2 = XPT2046_ReadChannelFiltered(hxpt, XPT2046_CMD_READ_Z2, 3);

    /* Calculate pressure: pressure is proportional to Z1/(Z2-Z1) */
    if (z2 > z1 && z1 > 0) {
        *pressure = (4095 * z1) / (z2 - z1);
    } else {
        *pressure = 0;
    }

    return XPT2046_OK;
}

/**
 * @brief   Set pressure threshold
 * @param   hxpt Pointer to XPT2046 handle structure
 * @param   threshold Pressure threshold value
 * @retval  XPT2046_StatusTypeDef Status of the operation
 */
XPT2046_StatusTypeDef XPT2046_SetThreshold(XPT2046_HandleTypeDef *hxpt,
                                          uint16_t threshold)
{
    if (hxpt == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    hxpt->Config.PressureThreshold = threshold;

    return XPT2046_OK;
}

/**
 * @brief   Detect gesture from touch data
 * @param   hxpt Pointer to XPT2046 handle structure
 * @retval  XPT2046_StatusTypeDef Status of the operation
 */
XPT2046_StatusTypeDef XPT2046_DetectGesture(XPT2046_HandleTypeDef *hxpt)
{
    if (hxpt == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    hxpt->TouchData.Gesture = XPT2046_AnalyzeGesture(hxpt);
    hxpt->TouchData.GestureTimestamp = HAL_GetTick();

    return XPT2046_OK;
}

/**
 * @brief   Get last detected gesture
 * @param   hxpt Pointer to XPT2046 handle structure
 * @retval  XPT2046_GestureTypeDef Last detected gesture
 */
XPT2046_GestureTypeDef XPT2046_GetLastGesture(XPT2046_HandleTypeDef *hxpt)
{
    if (hxpt == NULL) {
        return XPT2046_GESTURE_NONE;
    }

    return hxpt->TouchData.Gesture;
}

/**
 * @brief   Enable/disable gesture detection
 * @param   hxpt Pointer to XPT2046 handle structure
 * @param   enable Enable/disable flag
 * @retval  XPT2046_StatusTypeDef Status of the operation
 */
XPT2046_StatusTypeDef XPT2046_EnableGestureDetection(XPT2046_HandleTypeDef *hxpt,
                                                     bool enable)
{
    if (hxpt == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    /* Gesture detection state could be stored in Config if needed */
    /* For now, gestures are always analyzed when touch data is read */

    return XPT2046_OK;
}

/**
 * @brief   Get default configuration
 * @retval  XPT2046_ConfigTypeDef Default configuration structure
 */
XPT2046_ConfigTypeDef XPT2046_GetDefaultConfig(void)
{
    XPT2046_ConfigTypeDef config = {
        .Samples = XPT2046_SAMPLES,
        .PressureThreshold = XPT2046_MIN_PRESSURE,
        .InterruptEnable = true,
        .DebounceCount = XPT2046_DEBOUNCE_COUNT,
        .Use12Bit = true
    };

    return config;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Read single channel from XPT2046
 * @param   hxpt Pointer to XPT2046 handle
 * @param   channel Channel command byte
 * @retval  12-bit ADC value
 */
static uint16_t XPT2046_ReadChannel(XPT2046_HandleTypeDef *hxpt, uint8_t channel)
{
    uint8_t tx_data[3] = {channel, 0x00, 0x00};
    uint8_t rx_data[3] = {0};

    XPT2046_CS_Low(hxpt);

    /* Transmit command and receive response */
    HAL_SPI_TransmitReceive(hxpt->hspi, tx_data, rx_data, 3, XPT2046_TIMEOUT);

    XPT2046_CS_High(hxpt);

    /* Extract 12-bit value from response (bits are in rx_data[1] and rx_data[2]) */
    uint16_t value = ((uint16_t)rx_data[1] << 8) | (uint16_t)rx_data[2];
    value = value >> 3;  /* Right-shift to get 12-bit value */

    XPT2046_DELAY_US(XPT2046_SETTLING_DELAY_US);

    return value;
}

/**
 * @brief   Read channel with averaging
 * @param   hxpt Pointer to XPT2046 handle
 * @param   channel Channel command byte
 * @param   samples Number of samples to average
 * @retval  Averaged 12-bit ADC value
 */
static uint16_t XPT2046_ReadChannelFiltered(XPT2046_HandleTypeDef *hxpt,
                                            uint8_t channel,
                                            uint8_t samples)
{
    uint32_t sum = 0;
    uint16_t values[16];  /* Maximum 16 samples */

    if (samples > 16) samples = 16;
    if (samples < 1) samples = 1;

    /* Read multiple samples */
    for (uint8_t i = 0; i < samples; i++) {
        values[i] = XPT2046_ReadChannel(hxpt, channel);
        sum += values[i];
    }

    /* Return average */
    return (uint16_t)(sum / samples);
}

/**
 * @brief   Read raw coordinates from XPT2046
 * @param   hxpt Pointer to XPT2046 handle
 * @param   raw_x Pointer to store raw X coordinate
 * @param   raw_y Pointer to store raw Y coordinate
 * @param   pressure Pointer to store pressure value (can be NULL)
 * @retval  XPT2046_StatusTypeDef Status of the operation
 */
static XPT2046_StatusTypeDef XPT2046_ReadRawCoordinates(XPT2046_HandleTypeDef *hxpt,
                                                        uint16_t *raw_x,
                                                        uint16_t *raw_y,
                                                        uint16_t *pressure)
{
    if (raw_x == NULL || raw_y == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    /* Read X and Y with filtering */
    *raw_x = XPT2046_ReadChannelFiltered(hxpt, XPT2046_CMD_READ_X,
                                        hxpt->Config.Samples);
    *raw_y = XPT2046_ReadChannelFiltered(hxpt, XPT2046_CMD_READ_Y,
                                        hxpt->Config.Samples);

    /* Read pressure if requested */
    if (pressure != NULL) {
        uint16_t z1 = XPT2046_ReadChannelFiltered(hxpt, XPT2046_CMD_READ_Z1, 3);
        uint16_t z2 = XPT2046_ReadChannelFiltered(hxpt, XPT2046_CMD_READ_Z2, 3);

        /* Calculate pressure */
        if (z2 > z1 && z1 > 0) {
            *pressure = (4095 * z1) / (z2 - z1);
        } else {
            *pressure = 0;
        }
    }

    return XPT2046_OK;
}

/**
 * @brief   Convert raw touchscreen coordinates to display coordinates
 * @param   hxpt Pointer to XPT2046 handle
 * @param   raw_x Raw X coordinate
 * @param   raw_y Raw Y coordinate
 * @param   disp_x Pointer to store display X coordinate
 * @param   disp_y Pointer to store display Y coordinate
 * @retval  XPT2046_StatusTypeDef Status of the operation
 */
static XPT2046_StatusTypeDef XPT2046_ConvertCoordinates(XPT2046_HandleTypeDef *hxpt,
                                                        uint16_t raw_x,
                                                        uint16_t raw_y,
                                                        uint16_t *disp_x,
                                                        uint16_t *disp_y)
{
    if (disp_x == NULL || disp_y == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    int32_t x, y;
    int32_t width = XPT2046_DISPLAY_WIDTH;
    int32_t height = XPT2046_DISPLAY_HEIGHT;

    /* Determine effective display dimensions based on SwapXY */
    if (hxpt->Calibration.SwapXY) {
        width = XPT2046_DISPLAY_HEIGHT;
        height = XPT2046_DISPLAY_WIDTH;
    }

    /* Apply calibration mapping - Map raw to logical display limits */
    x = map((int32_t)raw_x,
            (int32_t)hxpt->Calibration.MinX,
            (int32_t)hxpt->Calibration.MaxX,
            0,
            (int32_t)(width - 1));

    y = map((int32_t)raw_y,
            (int32_t)hxpt->Calibration.MinY,
            (int32_t)hxpt->Calibration.MaxY,
            0,
            (int32_t)(height - 1));

    /* Apply swap if configured */
    if (hxpt->Calibration.SwapXY) {
        int32_t temp = x;
        x = y;
        y = temp;
    }

    /* Apply flip if configured - Use effective dimensions */
    if (hxpt->Calibration.FlipX) {
        x = (width - 1) - x;
    }
    if (hxpt->Calibration.FlipY) {
        y = (height - 1) - y;
    }

    /* Clamp to display bounds - Use effective dimensions */
    if (x < 0) x = 0;
    else if (x >= width) x = width - 1;

    if (y < 0) y = 0;
    else if (y >= height) y = height - 1;

    *disp_x = (uint16_t)x;
    *disp_y = (uint16_t)y;

    return XPT2046_OK;
}

/**
 * @brief   Filter coordinates using threshold-based update
 * @param   x Pointer to X coordinate
 * @param   y Pointer to Y coordinate
 */
static void XPT2046_FilterCoordinates(uint16_t *x, uint16_t *y)
{
    static uint16_t _x = 0;
    static uint16_t _y = 0;

    int32_t x_raw = (int32_t)*x;
    int32_t y_raw = (int32_t)*y;

    int32_t xDiff = x_raw > _x ? (x_raw - _x) : (_x - x_raw);
    int32_t yDiff = y_raw > _y ? (y_raw - _y) : (_y - y_raw);

    /* Threshold-based smoothing */
    if ((xDiff + yDiff) > XPT2046_SMOOTHING_THRESHOLD) {
        _x = (uint16_t)x_raw;
        _y = (uint16_t)y_raw;
    }

    *x = _x;
    *y = _y;
}

/**
 * @brief   Analyze gesture from touch data
 * @param   hxpt Pointer to XPT2046 handle structure
 * @retval  XPT2046_GestureTypeDef Detected gesture
 */
static XPT2046_GestureTypeDef XPT2046_AnalyzeGesture(XPT2046_HandleTypeDef *hxpt)
{
    /* Basic gesture recognition */

    if (hxpt->TouchData.TouchCount == 0 && hxpt->PrevTouchData.TouchCount > 0) {
        /* Touch released */
        uint32_t touch_duration = HAL_GetTick() -
                                 hxpt->PrevTouchData.Points[0].Timestamp;

        if (touch_duration > XPT2046_LONG_PRESS_TIME) {
            return XPT2046_GESTURE_LONG_PRESS;
        }
        return XPT2046_GESTURE_TAP;
    }

    if (hxpt->TouchData.TouchCount > 0 && hxpt->PrevTouchData.TouchCount > 0) {
        /* Calculate movement */
        int16_t deltaX = (int16_t)(hxpt->TouchData.Points[0].X -
                                   hxpt->PrevTouchData.Points[0].X);
        int16_t deltaY = (int16_t)(hxpt->TouchData.Points[0].Y -
                                   hxpt->PrevTouchData.Points[0].Y);
        uint16_t distance = (uint16_t)sqrt((double)(deltaX * deltaX +
                                                    deltaY * deltaY));

        if (distance > XPT2046_GESTURE_THRESHOLD) {
            if (abs(deltaX) > abs(deltaY)) {
                return (deltaX > 0) ? XPT2046_GESTURE_SWIPE_RIGHT :
                                     XPT2046_GESTURE_SWIPE_LEFT;
            }
            return (deltaY > 0) ? XPT2046_GESTURE_SWIPE_DOWN :
                                 XPT2046_GESTURE_SWIPE_UP;
        }
    }

    return XPT2046_GESTURE_NONE;
}
