/*******************************************************************************
 * LVGL Input Device Port Header
 *******************************************************************************
 * This header declares the input device (touchscreen) initialization function.
 *
 * Typically you don't need to call this directly - use LVGL_App_Init() instead.
 *
 * For advanced users:
 * If you need custom input initialization, you can call lv_port_indev_init()
 * directly after lv_init().
 ******************************************************************************/

#ifndef LV_PORT_INDEV_H
#define LV_PORT_INDEV_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
 * Function: lv_port_indev_init
 * Description: Initialize LVGL touchscreen driver
 * Parameters: None
 * Returns: None
 * Notes: Called automatically by LVGL_App_Init()
 *---------------------------------------------------------------------------*/
void lv_port_indev_init(void);

#ifdef __cplusplus
}
#endif

#endif /* LV_PORT_INDEV_H */
