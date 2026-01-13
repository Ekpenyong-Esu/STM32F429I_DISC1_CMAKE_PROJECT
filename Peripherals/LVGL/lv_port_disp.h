/*******************************************************************************
 * LVGL Display Port Header
 *******************************************************************************
 * This header declares the display driver initialization function.
 *
 * Typically you don't need to call this directly - use LVGL_App_Init() instead.
 *
 * For advanced users:
 * If you need custom display initialization, you can call lv_port_disp_init()
 * directly after lv_init().
 ******************************************************************************/

#ifndef LV_PORT_DISP_H
#define LV_PORT_DISP_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
 * Function: lv_port_disp_init
 * Description: Initialize LVGL display driver for STM32F429I LCD
 * Parameters: None
 * Returns: None
 * Notes: Called automatically by LVGL_App_Init()
 *---------------------------------------------------------------------------*/
void lv_port_disp_init(void);


#ifdef __cplusplus
}
#endif

#endif /* LV_PORT_DISP_H */
