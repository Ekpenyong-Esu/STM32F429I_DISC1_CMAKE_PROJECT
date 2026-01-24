/*******************************************************************************
 * LVGL Input Device Port - Touchscreen Integration
 *******************************************************************************
 * This file connects LVGL to the touchscreen hardware on STM32F429I-Discovery.
 *
 * Supported Touch Controllers:
 * - FT6206: Capacitive touch (I2C address 0x38)
 * - STMPE811: Resistive touch (I2C address 0x41/0x82)
 *
 * What this does:
 * - Reads touch coordinates from hardware via I2C
 * - Reports touch state (pressed/released) to LVGL
 * - Handles coordinate translation and calibration
 *
 * For Beginners:
 * 1. This is currently a placeholder (returns "not touched")
 * 2. Implement touch_read() with your I2C touch driver
 * 3. See TOUCHSCREEN peripheral for hardware driver code
 ******************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include "lvgl.h"
#include "lv_port_indev.h"
#include "touchscreen.h"
#include "i2c.h"
#include "app_low_power.h"  /* Application low power management */

/* Touchscreen handle */
static TS_HandleTypeDef hts;

/* Last known touch position */
static int16_t last_x = 0;
static int16_t last_y = 0;
/*-----------------------------------------------------------------------------
 * Touch Read Callback - Get Current Touch State
 *---------------------------------------------------------------------------*/
/* This function is called by LVGL periodically to check for touch input.
 *
 * Parameters:
 * - indev: Input device (v9 API)
 * - data:  Structure to fill with touch information
 *
 * What to implement:
 * 1. Read touch controller via I2C
 * 2. If touched: set data->state = LV_INDEV_STATE_PRESSED
 * 3. Set data->point.x and data->point.y to touch coordinates
 * 4. If not touched: set data->state = LV_INDEV_STATE_RELEASED
 *
 * Example implementation:
 *   uint16_t x, y;
 *   if(TouchScreen_GetTouch(&x, &y)) {
 *     data->state = LV_INDEV_STATE_PRESSED;
 *     data->point.x = x;
 *     data->point.y = y;
 *   } else {
 *     data->state = LV_INDEV_STATE_RELEASED;
 *   }
 */
/* LVGL input device read callback */
static void touch_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    (void)indev;  /* Unused */

    uint16_t x = 0;
    uint16_t y = 0;

    if (!hts.IsInitialized) {
        data->state = LV_INDEV_STATE_RELEASED;
        data->point.x = last_x;
        data->point.y = last_y;
        return;
    }

    /* Read touchscreen data */
    /* Debounce touch: require two consecutive touch reads to confirm a real touch */
    static uint8_t touch_confirm_count = 0;

    if (TS_GetSingleTouch(&hts, &x, &y) == TS_OK) {

        /* Clamp coordinates to LVGL display bounds */
        if (x >= TS_DISPLAY_WIDTH)  x = TS_DISPLAY_WIDTH  - 1;
        if (y >= TS_DISPLAY_HEIGHT) y = TS_DISPLAY_HEIGHT - 1;

        /* Apply coordinate transformation if needed */
        /* For STM32F429I-DISC1, the touchscreen may need X/Y swap and/or rotation */

        /* Option 1: No transformation (current) */
        data->point.x = x;
        data->point.y = y;

        touch_confirm_count++;
        if (touch_confirm_count >= 2) {
            /* Confirmed touch - report pressed and update activity */
            data->state   = LV_INDEV_STATE_PRESSED;

            /* Update activity timestamp only when touch is confirmed */
            APP_TouchActivity();

            last_x = data->point.x;
            last_y = data->point.y;
        } else {
            /* Not yet confirmed - report previous state to avoid false presses */
            data->state   = LV_INDEV_STATE_RELEASED;
            data->point.x = last_x;
            data->point.y = last_y;
        }
    } else {
        touch_confirm_count = 0;
        data->state   = LV_INDEV_STATE_RELEASED;
        data->point.x = last_x;
        data->point.y = last_y;
    }


}

/*-----------------------------------------------------------------------------
 * Initialize Input Device Port
 *---------------------------------------------------------------------------*/
/* Call this function once during startup (after lv_init()).
 * It registers the touchscreen with LVGL.
 */
/* LVGL input device initialization */
void lv_port_indev_init(void)
{
    /* Initialize touchscreen with I2C3 handle */
    if (TS_Init(&hts, &hi2c3) != TS_OK) {
        return;
    }

    /* Optional: configure touchscreen interrupts */
    // TS_ITConfig(&hts);
    // TS_EnableInterrupt(&hts, true);

    /* Create LVGL input device driver */
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touch_read);

}
