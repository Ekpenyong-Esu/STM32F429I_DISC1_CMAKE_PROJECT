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
static XPT2046_Handle_t hxpt;

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
    if (!hxpt.initialized) {
        return;
    }

    /* Try to read touch */
    XPT2046_TouchPoint_t touch;
    XPT2046_StatusTypeDef status = XPT2046_ReadTouch(&hxpt, &touch);

    if (status == XPT2046_OK && touch.state == XPT2046_STATE_PRESSED) {
        /* Valid touch detected */
        uint16_t x = touch.x;
        uint16_t y = touch.y;

        /*
         * Coordinate transformation based on display orientation
         * XPT2046 raw coordinates need to be mapped to match display orientation
         */
        uint16_t logical_x = 0;
        uint16_t logical_y = 0;

        switch (DISP_ORIENTATION) {
            case ILI9488_ORIENTATION_PORTRAIT:
                /* Portrait: 320x480 */
                logical_x = x;
                logical_y = y;
                break;

            case ILI9488_ORIENTATION_LANDSCAPE:
                /* Landscape: 480x320
                 * Typically need to swap and flip for proper mapping
                 * Adjust these based on your specific hardware
                 */
                logical_x = y;
                logical_y = (DISP_WIDTH - 1) - x;
                break;

            case ILI9488_ORIENTATION_PORTRAIT_REV:
                /* Portrait reversed: 320x480 */
                logical_x = (DISP_WIDTH - 1) - x;
                logical_y = (DISP_HEIGHT - 1) - y;
                break;

            case ILI9488_ORIENTATION_LANDSCAPE_REV:
                /* Landscape reversed: 480x320 */
                logical_x = (DISP_HEIGHT - 1) - y;
                logical_y = x;
                break;

            default:
                logical_x = x;
                logical_y = y;
                break;
        }

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

        log_debug("Touch: (%d,%d) [raw: %u,%u] pressure=%u",
                  logical_x, logical_y, x, y, touch.pressure);
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
        TP_CS_PORT, TP_CS_PIN,
        TP_IRQ_PORT, TP_IRQ_PIN,
        DISP_WIDTH, DISP_HEIGHT
    );

    if (status != XPT2046_OK) {
        log_error("LVGL: XPT2046 init failed (status=%d)", status);
        return -1;
    }

    log_debug("LVGL: XPT2046 initialized");

    /*
     * Set touch transformations based on orientation
     * These values may need adjustment for your specific display/touch combo
     */
    switch (DISP_ORIENTATION) {
        case ILI9488_ORIENTATION_PORTRAIT:
            XPT2046_SetTransform(&hxpt, false, false, false);
            break;

        case ILI9488_ORIENTATION_LANDSCAPE:
            /* Landscape typically needs swap_xy=true, adjust flip as needed */
            XPT2046_SetTransform(&hxpt, true, false, true);
            break;

        case ILI9488_ORIENTATION_PORTRAIT_REV:
            XPT2046_SetTransform(&hxpt, false, true, true);
            break;

        case ILI9488_ORIENTATION_LANDSCAPE_REV:
            XPT2046_SetTransform(&hxpt, true, true, false);
            break;
    }

    /*
     * Set calibration values
     * These are approximate defaults - run calibration routine for accurate values
     * Touch corners of screen and note ADC values, then update these
     */
    XPT2046_SetCalibration(&hxpt,
                          300,   // x_min (touch left edge)
                          3700,  // x_max (touch right edge)
                          300,   // y_min (touch top edge)
                          3700); // y_max (touch bottom edge)

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
XPT2046_Handle_t *lv_port_indev_get_xpt2046_handle(void)
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
    if (!hxpt.initialized) {
        log_error("Touch not initialized");
        return;
    }

    XPT2046_TouchPoint_t touch;
    if (XPT2046_ReadTouch(&hxpt, &touch) == XPT2046_OK) {
        log_info("Touch detected at (%u,%u) pressure=%u",
                 touch.x, touch.y, touch.pressure);
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
