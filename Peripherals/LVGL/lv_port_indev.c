/*******************************************************************************
 * LVGL Input Device Port - Touchscreen Integration
 *******************************************************************************
 * This file connects LVGL to the touchscreen hardware on STM32F429I-Discovery.
 *
 * Supported Touch Controllers:
 * - XPT2046: Resistive touch (SPI)
 *
 * What this does:
 * - Reads touch coordinates from hardware via SPI
 * - Reports touch state (pressed/released) to LVGL
 * - Handles coordinate translation and calibration
 *
 * For Beginners:
 * 1. This is currently a placeholder (returns "not touched")
 * 2. Implement touch_read() with your I2C touch driver
 * 3. See TOUCHSCREEN peripheral for hardware driver code
 ******************************************************************************/

#include <stdint.h>
#include "lvgl.h"
#include "lv_port_indev.h"
#include "xpt2046.h"

/* Touchscreen handle */
static XPT2046_Handle_t hxpt;

/* Last known touch position */
static int16_t last_x = 0;
static int16_t last_y = 0;

/* Touch controller pins (adjust if wired differently) */
#define TP_CS_PORT   GPIOC
#define TP_CS_PIN    GPIO_PIN_3
#define TP_IRQ_PORT  GPIOA
#define TP_IRQ_PIN   GPIO_PIN_15
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

    if (!hxpt.initialized) {
        data->state = LV_INDEV_STATE_RELEASED;
        data->point.x = last_x;
        data->point.y = last_y;
        return;
    }

    XPT2046_TouchPoint_t touch;

    if (XPT2046_ReadTouch(&hxpt, &touch) == XPT2046_OK) {
        x = touch.x;
        y = touch.y;

        if (x >= 320) x = 319;
        if (y >= 480) y = 479;

        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PRESSED;

        last_x = data->point.x;
        last_y = data->point.y;
    } else {
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
    if (XPT2046_Init(&hxpt, TP_CS_PORT, TP_CS_PIN, TP_IRQ_PORT, TP_IRQ_PIN, 320, 480) != XPT2046_OK) {
        return;
    }

    /* Create LVGL input device driver */
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touch_read);

}
