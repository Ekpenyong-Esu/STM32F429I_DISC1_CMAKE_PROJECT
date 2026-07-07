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
#include "log.h"

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

/* Safe SPI prescaler used by XPT2046 when sharing bus with faster devices (e.g. ILI9488).
 * Driver will temporarily switch SPI to this prescaler during touch ADC reads and restore
 * the previous prescaler afterwards. */
#define XPT2046_SAFE_BAUD_PRESCALER     SPI_BAUDRATEPRESCALER_32

/* Private variables ---------------------------------------------------------*/
/* Global touchscreen handle used for deferred IRQ handling */
static XPT2046_HandleTypeDef *g_hxpt = NULL;
/* Deferred EXTI handling flag (set in ISR) */
static volatile bool s_xpt_irq_pending = false;

/* Private function prototypes -----------------------------------------------*/
static XPT2046_StatusTypeDef XPT2046_ReadRawCoordinates(XPT2046_HandleTypeDef *hxpt,
                                                        uint16_t *raw_x,
                                                        uint16_t *raw_y,
                                                        uint16_t *pressure);
static XPT2046_StatusTypeDef XPT2046_ReadTouchData(XPT2046_HandleTypeDef *hxpt);
static XPT2046_StatusTypeDef XPT2046_ConvertCoordinates(XPT2046_HandleTypeDef *hxpt,
                                                        uint16_t raw_x,
                                                        uint16_t raw_y,
                                                        uint16_t *disp_x,
                                                        uint16_t *disp_y);
static void XPT2046_FilterCoordinates(uint16_t *x, uint16_t *y);
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
    /* Save global handle for deferred IRQ processing */
    g_hxpt = hxpt;

    /* SPI handling must be pre-initialized by application (via SPI_Init() in main) */
   if (hxpt->hspi->Instance == NULL) {
        return XPT2046_ERROR;  /* SPI not initialized */
    }

    /* Initialize MSP (GPIO, clocks) */
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
 * @brief   Print raw ADC coordinates (helper for calibration)
 * @param   hxpt Pointer to XPT2046 handle
 * @note    Call from main loop to log raw ADC readings for calibration
 */
void XPT2046_PrintRawCoordinates(XPT2046_HandleTypeDef *hxpt)
{
    if (hxpt == NULL || !hxpt->IsInitialized) {
        log_warning("XPT2046: not initialized - cannot print raw coordinates");
        return;
    }

    uint16_t rawx = 0;
    uint16_t rawy = 0;
    uint16_t pressure = 0;
    for (int i = 0; i < 5; ++i) {
        XPT2046_ReadRawCoordinates(hxpt, &rawx, &rawy, &pressure);
        log_info("XPT2046 Raw: X=%u Y=%u Pressure=%u", rawx, rawy, pressure);
        HAL_Delay(100);
    }
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
    if (hxpt == NULL || x == NULL || y == NULL || pressed == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    *pressed = 0;
    *x = 0;
    *y = 0;

    if (!hxpt->IsInitialized) {
        return XPT2046_NOT_INITIALIZED;
    }

    /* Service any pending IRQ so the cached TouchData is fresh */
    XPT2046_ServiceIRQ();

    /* Return cached touch state updated by IRQ handler / ServiceIRQ */
    if (hxpt->TouchData.TouchCount == 0) {
        return XPT2046_OK; /* no touch */
    }

    *x = hxpt->TouchData.Points[0].X;
    *y = hxpt->TouchData.Points[0].Y;
    *pressed = 1;

    return XPT2046_OK;
}

/*
 * Internal: read touch and update driver's cached TouchData
 * (kept internal — used by IRQ service path)
 */
static XPT2046_StatusTypeDef XPT2046_ReadTouchData(XPT2046_HandleTypeDef *hxpt)
{
    if (hxpt == NULL || !hxpt->IsInitialized) {
        return XPT2046_NOT_INITIALIZED;
    }

    /* Save previous touch data */
    hxpt->PrevTouchData = hxpt->TouchData;

    /* If IRQ line reports no touch, clear cache */
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

    if (XPT2046_ReadRawCoordinates(hxpt, &raw_x, &raw_y, &pressure) != XPT2046_OK) {
        return XPT2046_ERROR;
    }

    if (pressure < hxpt->Config.PressureThreshold) {
        hxpt->TouchData.TouchCount = 0;
        hxpt->TouchData.Points[0].State = XPT2046_TOUCH_RELEASED;
        return XPT2046_NO_TOUCH;
    }

    if (XPT2046_ConvertCoordinates(hxpt, raw_x, raw_y, &disp_x, &disp_y) != XPT2046_OK) {
        return XPT2046_ERROR;
    }

    XPT2046_FilterCoordinates(&disp_x, &disp_y);

    hxpt->TouchData.TouchCount = 1;
    hxpt->TouchData.Points[0].X = disp_x;
    hxpt->TouchData.Points[0].Y = disp_y;
    hxpt->TouchData.Points[0].Z = pressure;
    hxpt->TouchData.Points[0].RawX = raw_x;
    hxpt->TouchData.Points[0].RawY = raw_y;
    hxpt->TouchData.Points[0].Timestamp = HAL_GetTick();

    hxpt->TouchData.Points[0].State = (hxpt->PrevTouchData.TouchCount == 0)
                                      ? XPT2046_TOUCH_PRESSED
                                      : XPT2046_TOUCH_MOVING;

    hxpt->LastTouchTime = HAL_GetTick();

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
 * @brief   Enable/disable NVIC for touch IRQ and set driver flag
 */
XPT2046_StatusTypeDef XPT2046_EnableInterrupt(XPT2046_HandleTypeDef *hxpt, bool enable)
{
    if (hxpt == NULL) return XPT2046_INVALID_PARAM;

    hxpt->InterruptMode = enable;

    if (enable) {
        HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
    } else {
        HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
    }

    return XPT2046_OK;
}

/**
 * @brief   Ensure EXTI/NVIC configured for touch IRQ (no GPIO setup here)
 */
XPT2046_StatusTypeDef XPT2046_ITConfig(XPT2046_HandleTypeDef *hxpt)
{
    if (hxpt == NULL) return XPT2046_INVALID_PARAM;

    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

    return XPT2046_OK;
}

/**
 * @brief   HAL EXTI callback (called from ISR) — mark IRQ pending
 * @note    Keep this minimal: set flag only, actual SPI reads are deferred.
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (g_hxpt != NULL && GPIO_Pin == g_hxpt->IRQ_Pin) {
        s_xpt_irq_pending = true;
    }
}

/**
 * @brief   Service pending touchscreen IRQ outside ISR context
 * @note    Call from main loop or from LVGL input read to update cached state
 */
void XPT2046_ServiceIRQ(void)
{
    if (s_xpt_irq_pending && g_hxpt != NULL && g_hxpt->IsInitialized)
    {
        s_xpt_irq_pending = false;
        /* Read/update cached touch data (performs SPI reads) */
        (void)XPT2046_ReadTouchData(g_hxpt);
    }
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
 * @brief   Interrupt handler
 * @param   hxpt Pointer to XPT2046 handle structure
 */

/**
 * @brief   Service pending touchscreen IRQ outside ISR context
 * @details Handles deferred interrupt processing
 */




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
 * @brief   Get default configuration
 * @retval  XPT2046_ConfigTypeDef Default configuration structure
 */
XPT2046_ConfigTypeDef XPT2046_GetDefaultConfig(void)
{
    XPT2046_ConfigTypeDef config = {
        .Samples = XPT2046_SAMPLES,
        .PressureThreshold = XPT2046_MIN_PRESSURE,
        .InterruptEnable = false,
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

    /*
     * First (dummy) transfer: some panels / first-conversion reads can be noisy or
     * return the previous conversion result. Perform a dummy transfer and discard it,
     * then perform the real measurement.
     */
    HAL_SPI_TransmitReceive(hxpt->hspi, tx_data, rx_data, 3, XPT2046_TIMEOUT);
    XPT2046_DELAY_US(XPT2046_CONVERSION_DELAY_US);

    /* Actual measurement */
    memset(rx_data, 0, sizeof(rx_data));
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

    /*
     * Try to ensure SPI is running at a safe (low) speed for the XPT2046 ADC reads.
     * Prefer using the public SPI helper when hxpt->hspi points to SPI4 (project default).
     * If speed change fails, we log a warning and continue with the current speed.
     */
    uint32_t prev_prescaler = 0;
    bool switched = false;

    if (hxpt != NULL && hxpt->hspi != NULL) {
        prev_prescaler = hxpt->hspi->Init.BaudRatePrescaler;

        if (prev_prescaler != XPT2046_SAFE_BAUD_PRESCALER) {
            if (hxpt->hspi->Instance == SPI4) {
                if (SPI_SetBaudRatePrescaler(XPT2046_SAFE_BAUD_PRESCALER) == SPI_OK) {
                    switched = true;
                } else {
                    log_warning("XPT2046: failed to set SPI prescaler via SPI_SetBaudRatePrescaler()");
                }
            } else {
                /* Fallback: reconfigure the provided SPI handle directly */
                if (HAL_SPI_DeInit(hxpt->hspi) == HAL_OK) {
                    hxpt->hspi->Init.BaudRatePrescaler = XPT2046_SAFE_BAUD_PRESCALER;
                    if (HAL_SPI_Init(hxpt->hspi) == HAL_OK) {
                        switched = true;
                    } else {
                        log_warning("XPT2046: HAL_SPI_Init() failed when changing prescaler");
                    }
                } else {
                    log_warning("XPT2046: HAL_SPI_DeInit() failed when changing prescaler");
                }
            }
        }
    }

    /* Read multiple samples */
    for (uint8_t i = 0; i < samples; i++) {
        values[i] = XPT2046_ReadChannel(hxpt, channel);
        sum += values[i];
    }

    /* Restore previous SPI prescaler if it was changed */
    if (switched && hxpt != NULL && hxpt->hspi != NULL) {
        if (hxpt->hspi->Instance == SPI4) {
            if (SPI_SetBaudRatePrescaler(prev_prescaler) != SPI_OK) {
                log_error("XPT2046: failed to restore SPI prescaler via SPI_SetBaudRatePrescaler()");
            }
        } else {
            if (HAL_SPI_DeInit(hxpt->hspi) == HAL_OK) {
                hxpt->hspi->Init.BaudRatePrescaler = prev_prescaler;
                if (HAL_SPI_Init(hxpt->hspi) != HAL_OK) {
                    log_error("XPT2046: HAL_SPI_Init() failed when restoring prescaler");
                }
            } else {
                log_error("XPT2046: HAL_SPI_DeInit() failed when restoring prescaler");
            }
        }
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

    log_debug("Raw coordinates: X=%u, Y=%u", *raw_x, *raw_y);

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
    int32_t eff_w = XPT2046_DISPLAY_WIDTH;
    int32_t eff_h = XPT2046_DISPLAY_HEIGHT;

    /* Map raw values to native (portrait) display coordinates */
    x = map((int32_t)raw_x,
            (int32_t)hxpt->Calibration.MinX,
            (int32_t)hxpt->Calibration.MaxX,
            0,
            (int32_t)XPT2046_DISPLAY_WIDTH - 1);

    y = map((int32_t)raw_y,
            (int32_t)hxpt->Calibration.MinY,
            (int32_t)hxpt->Calibration.MaxY,
            0,
            (int32_t)XPT2046_DISPLAY_HEIGHT - 1);

    /* Apply swap FIRST (if needed for landscape) */
    if (hxpt->Calibration.SwapXY) {
        int32_t temp = x;
        x = y;
        y = temp;
        /* After swap, effective dimensions are also swapped */
        eff_w = XPT2046_DISPLAY_HEIGHT;
        eff_h = XPT2046_DISPLAY_WIDTH;
    }

    /* Then apply flips using effective (post-swap) dimensions */
    if (hxpt->Calibration.FlipX) {
        x = (eff_w - 1) - x;
    }
    if (hxpt->Calibration.FlipY) {
        y = (eff_h - 1) - y;
    }

    /* Clamp to effective display bounds */
    if (x < 0) x = 0;
    if (x >= eff_w) x = eff_w - 1;
    if (y < 0) y = 0;
    if (y >= eff_h) y = eff_h - 1;

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
    static uint16_t _x = 0xFFFF;  // Use invalid value to detect first touch
    static uint16_t _y = 0xFFFF;

    uint16_t x_new = *x;
    uint16_t y_new = *y;

    /* First touch - no filter, just store */
    if (_x == 0xFFFF || _y == 0xFFFF) {
        _x = x_new;
        _y = y_new;
        *x = _x;
        *y = _y;
        return;
    }

    /* Calculate difference */
    int32_t xDiff = abs((int32_t)x_new - (int32_t)_x);
    int32_t yDiff = abs((int32_t)y_new - (int32_t)_y);

    /* Large movement - update immediately */
    if (xDiff > XPT2046_SMOOTHING_THRESHOLD || yDiff > XPT2046_SMOOTHING_THRESHOLD) {
        _x = x_new;
        _y = y_new;
    }
    /* Small movement - apply smoothing */
    else {
        _x = (_x * 3 + x_new) / 4;  // 75% old, 25% new
        _y = (_y * 3 + y_new) / 4;
    }

    *x = _x;
    *y = _y;
}

/**
 * @brief   Analyze gesture from touch data
 * @param   hxpt Pointer to XPT2046 handle structure
 * @retval  XPT2046_GestureTypeDef Detected gesture
 */
