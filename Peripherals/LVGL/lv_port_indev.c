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
#include "stdbool.h"
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
#define DISP_ORIENTATION    ILI9488_ORIENTATION_LANDSCAPE
#define DISP_WIDTH          480
#define DISP_HEIGHT         320

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

    /* Service any pending IRQ (updates driver's cached TouchData) */
    XPT2046_ServiceIRQ();

    /* Get touch state (reads cached state populated by IRQ/service) */
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

        log_debug("Touch: screen=(%d,%d)",
                  logical_x, logical_y);
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

    /* Enable interrupt-driven operation and configure NVIC */
    XPT2046_EnableInterrupt(&hxpt, true);
    XPT2046_ITConfig(&hxpt);

    log_debug("LVGL: XPT2046 initialized (interrupt mode)");

    /*
     * Set calibration values based on orientation
     * These are approximate defaults - run calibration routine for accurate values
     * Touch corners of screen and note ADC values, then update these
     */
    /* Default raw ADC min/max (measured) */
    XPT2046_CalibrationTypeDef cal = {
        .MinX = 200,
        .MaxX = 3900,
        .MinY = 200,
        .MaxY = 3900,
        .ScaleX = 0.0f,   /* will be computed from LVGL display size */
        .ScaleY = 0.0f,   /* will be computed from LVGL display size */
        .OffsetX = 0,
        .OffsetY = 0,
        .SwapXY = false,
        .FlipX = false,
        .FlipY = false,
        .IsCalibrated = false
    };

    /* Adjust calibration flags based on display orientation (preserve your FlipX change) */
    switch (DISP_ORIENTATION) {
        case ILI9488_ORIENTATION_PORTRAIT:
            cal.FlipY = true;
            break;
        case ILI9488_ORIENTATION_LANDSCAPE:
            cal.SwapXY = true;
            cal.FlipY = true;
            /* preserve user-added FlipX for your panel wiring */
            cal.FlipX = true;
            break;
        case ILI9488_ORIENTATION_PORTRAIT_REV:
            cal.FlipX = true;
            cal.FlipY = true;
            break;
        case ILI9488_ORIENTATION_LANDSCAPE_REV:
            cal.SwapXY = true;
            cal.FlipX = true;
            break;
    }

    /* Compute Scale/Offset from the runtime LVGL display resolution so touch maps correctly */
    lv_display_t *lv_disp = lv_display_get_default();
    int32_t disp_w = lv_display_get_horizontal_resolution(lv_disp);
    int32_t disp_h = lv_display_get_vertical_resolution(lv_disp);

    /* Defensive: fall back to defaults if LVGL not yet initialized */
    if (disp_w <= 0) disp_w = DISP_WIDTH;
    if (disp_h <= 0) disp_h = DISP_HEIGHT;

    cal.ScaleX = (float)disp_w / (float)(cal.MaxX - cal.MinX);
    cal.ScaleY = (float)disp_h / (float)(cal.MaxY - cal.MinY);
    cal.OffsetX = (int16_t)(- (int32_t)cal.MinX);
    cal.OffsetY = (int16_t)(- (int32_t)cal.MinY);
    cal.IsCalibrated = true;

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
void lv_port_indev_calibrate(void)
{
    if (!hxpt.IsInitialized) {
        log_error("Touch not initialized - cannot calibrate");
        return;
    }

    /*
     * TODO: Implement interactive 3-point calibration routine.
     * 1. Display crosshair at known screen positions (e.g. corners)
     * 2. Read raw ADC values when user touches each crosshair
     * 3. Compute MinX/MaxX/MinY/MaxY from the raw readings
     * 4. Apply with XPT2046_SetCalibration()
     *
     * For now, use XPT2046_PrintRawCoordinates() to manually read
     * corner values and update the defaults in lv_port_indev_init().
     */
    log_warning("Calibration routine not yet implemented."
                " Use XPT2046_PrintRawCoordinates() to read corner ADC values"
                " and update cal.MinX/MaxX/MinY/MaxY in lv_port_indev_init().");
}

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


