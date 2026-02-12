/**
  ******************************************************************************
  * @file    lv_port_indev.c
  * @brief   LVGL Input Device Port - XPT2046 Touch Integration (LVGL v9)
  * @details Fixed version with proper coordinate handling and orientation support
  * @version 1.1
  * @date    2025-02-08
  ******************************************************************************
  */

#include "lv_port_indev.h"
#include "lvgl.h"
#include "xpt2046.h"
#include "ili9488.h"
#include "spi.h"
#include "log.h"
#include <stdint.h>

/*-----------------------------------------------------------------------------
 * Touch Configuration
 *---------------------------------------------------------------------------*/

/* Touch controller pins - adjust to match your hardware */
#define TP_CS_PORT      GPIOC
#define TP_CS_PIN       GPIO_PIN_3
#define TP_IRQ_PORT     GPIOA
#define TP_IRQ_PIN      GPIO_PIN_15

/* Display orientation - MUST match lv_port_disp.c */
#define DISP_ORIENTATION    ILI9488_ORIENTATION_PORTRAIT
#define DISP_WIDTH          320
#define DISP_HEIGHT         480

/*-----------------------------------------------------------------------------
 * Private Variables
 *---------------------------------------------------------------------------*/

/** XPT2046 touch controller handle */
static XPT2046_HandleTypeDef hxpt;

/** LVGL input device object */
static lv_indev_t *s_indev = NULL;


/** Last valid touch position (for reporting when released) */
static int16_t last_x = 0;
static int16_t last_y = 0;

/*-----------------------------------------------------------------------------
 * Private Function Prototypes
 *---------------------------------------------------------------------------*/
static void touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data);

/*-----------------------------------------------------------------------------
 * Private Functions
 *---------------------------------------------------------------------------*/

/**
 * @brief   LVGL touch read callback
 * @param   indev Input device object
 * @param   data Pointer to input data structure to fill
 * @note    Called periodically by LVGL to get current touch state
 */
static void touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    (void)indev;  /* Unused parameter */

    /* Default to released state */
    data->state = LV_INDEV_STATE_RELEASED;
    data->point.x = last_x;
    data->point.y = last_y;

    /* Check if touch controller is initialized */
    if (!hxpt.IsInitialized) {
        return;
    }

    /* Get touch state */
    uint16_t x = 0;
    uint16_t y = 0;
    uint8_t pressed = 0;

    XPT2046_StatusTypeDef status = XPT2046_GetTouchState(&hxpt, &x, &y, &pressed);

    if (status == XPT2046_OK && pressed) {
        /* Valid touch detected */

        /*
         * XPT2046 driver already handles calibration and orientation mapping
         * based on the configuration set in lv_port_indev_init
         */
        uint16_t logical_x = x;
        uint16_t logical_y = y;

        /* Clamp coordinates to screen bounds */
        if (logical_x >= DISP_WIDTH) logical_x = DISP_WIDTH - 1;
        if (logical_y >= DISP_HEIGHT) logical_y = DISP_HEIGHT - 1;

        /* Update LVGL data structure */
        data->point.x = logical_x;
        data->point.y = logical_y;
        data->state = LV_INDEV_STATE_PRESSED;

        /* Store for next iteration (when finger lifts) */
        last_x = logical_x;
        last_y = logical_y;

        log_debug("Touch: (%d,%d) [raw: %u,%u]",
                  logical_x, logical_y, x, y);
    }
}

/*-----------------------------------------------------------------------------
 * Public Functions
 *---------------------------------------------------------------------------*/

/**
 * @brief   Initialize LVGL input device (touchscreen)
 * @note    Call after lv_init() and lv_port_disp_init()
 * @retval  0 on success, -1 on error
 */
int lv_port_indev_init(void)
{
    log_info("LVGL: Initializing input device (XPT2046 touch)");

    /* Initialize XPT2046 touch controller */
    XPT2046_StatusTypeDef status = XPT2046_Init(
        &hxpt,
        &hspi4,
        TP_CS_PORT, TP_CS_PIN,
        TP_IRQ_PORT, TP_IRQ_PIN
    );

    if (status != XPT2046_OK) {
        log_error("LVGL: XPT2046 init failed (status=%d)", status);
        return -1;
    }

    log_debug("LVGL: XPT2046 initialized");

    /*
     * Set calibration values based on orientation
     * These are approximate defaults - run calibration routine for accurate values
     * Touch corners of screen and note ADC values, then update these
     */
    XPT2046_CalibrationTypeDef cal = {
        .MinX = 200,
        .MaxX = 3900,
        .MinY = 200,
        .MaxY = 3900,
        .ScaleX = (float)DISP_WIDTH / (float)(3900 - 200),
        .ScaleY = (float)DISP_HEIGHT / (float)(3900 - 200),
        .OffsetX = -200,
        .OffsetY = -200,
        .SwapXY = false,
        .FlipX = false,
        .FlipY = false,
        .IsCalibrated = true
    };

    /* Adjust calibration based on display orientation */
    switch (DISP_ORIENTATION) {
        case ILI9488_ORIENTATION_PORTRAIT:
            /* Portrait: Y axis typically needs flipping for bottom-connector mounting */
            cal.FlipY = true;
            break;

        case ILI9488_ORIENTATION_LANDSCAPE:
            /* Landscape typically needs swap_xy=true, adjust flip as needed */
            cal.SwapXY = true;
            cal.FlipY = true;
            break;

        case ILI9488_ORIENTATION_PORTRAIT_REV:
            /* Portrait reversed */
            cal.FlipX = true;
            cal.FlipY = true;
            break;

        case ILI9488_ORIENTATION_LANDSCAPE_REV:
            /* Landscape reversed */
            cal.SwapXY = true;
            cal.FlipX = true;
            break;
    }

    XPT2046_SetCalibration(&hxpt, &cal);
    log_debug("LVGL: Touch calibration set (using defaults - may need adjustment)");

    /* Create LVGL input device (v9 API) */
    s_indev = lv_indev_create();
    if (!s_indev) {
        log_error("LVGL: Failed to create input device");
        return -1;
    }

    /* Configure input device */
    lv_indev_set_type(s_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(s_indev, touch_read_cb);

    log_info("LVGL: Input device initialized successfully");

    return 0;
}

/**
 * @brief   Get XPT2046 touch handle
 * @retval  Pointer to touch handle
 */
XPT2046_HandleTypeDef *lv_port_indev_get_xpt2046_handle(void)
{
    return &hxpt;
}

/**
 * @brief   Get LVGL input device object
 * @retval  Pointer to input device
 */
lv_indev_t *lv_port_indev_get_indev(void)
{
    return s_indev;
}

/**
 * @brief   Test touch by printing coordinates to log
 * @note    Call in main loop to debug touch functionality
 */
void lv_port_indev_test_touch(void)
{
    if (!hxpt.IsInitialized) {
        log_error("Touch not initialized");
        return;
    }

    uint16_t x = 0;
    uint16_t y = 0;
    uint8_t pressed = 0;

    if (XPT2046_GetTouchState(&hxpt, &x, &y, &pressed) == XPT2046_OK && pressed) {
        uint16_t pressure = 0;
        XPT2046_GetPressure(&hxpt, &pressure);
        log_info("Touch detected at (%u,%u) pressure=%u", x, y, pressure);
    } else if (XPT2046_IsTouched(&hxpt)) {
        log_warning("Touch IRQ active but read failed");
    }
}

/**
 * @brief   Run touch calibration routine
 * @note    Touch corners in sequence to calibrate
 * @todo    Implement interactive calibration UI
 */
void lv_port_indev_calibrate(void)
{
    log_info("Touch calibration routine");
    log_info("Touch top-left corner and hold for 2 seconds...");

    /* TODO: Implement full calibration routine
     * 1. Show calibration screen with target points
     * 2. Read raw ADC values at each corner
     * 3. Calculate and store calibration parameters
     * 4. Save to flash/EEPROM for persistence
     */

    log_warning("Calibration not fully implemented - using defaults");
}
