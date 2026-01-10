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

/* Touchscreen handle */
static TS_HandleTypeDef hts;
static I2C_HandleTypeDef *hi2c_touch = NULL;

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
static void touch_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    (void)indev;  /* Unused parameter */

    /* Check if touchscreen is initialized */
    if (!hts.IsInitialized) {
        data->state = LV_INDEV_STATE_RELEASED;
        printf("Touchscreen not initialized\n");
        return;
    }


    TS_ReadTouchData(&hts);

    /* Read touch data from STMPE811 */
    uint16_t x;
    uint16_t y;
    if (TS_GetSingleTouch(&hts, &x, &y) == TS_OK) {
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = x;
        data->point.y = y;
        printf("Touch detected: x=%d, y=%d\n", x, y);
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
        printf("Touch released\n");
    }
}

/*-----------------------------------------------------------------------------
 * Initialize Input Device Port
 *---------------------------------------------------------------------------*/
/* Call this function once during startup (after lv_init()).
 * It registers the touchscreen with LVGL.
 */
void lv_port_indev_init(void)
{
    /* Initialize touchscreen with I2C3 */
    if (TS_Init(&hts, &hi2c3) != TS_OK) {
        /* Touchscreen initialization failed */
        printf("ERROR: Touchscreen init failed\n");
        return;
    }
    printf("Touchscreen initialized successfully\n");

    // /* Configure touchscreen interrupts for better responsiveness */
    // TS_ITConfig(&hts);
    // TS_EnableInterrupt(&hts, true);

    /* Create and configure the input device driver (v9 API) */
    lv_indev_t * indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);  /* Pointer = touchscreen or mouse */
    lv_indev_set_read_cb(indev, touch_read);          /* Our touch reading function */

    /* Note: Touch hardware initialization is done above */
}
