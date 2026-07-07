/**
  ******************************************************************************
  * @file    lv_port_indev.h
  * @brief   LVGL Input Device Port Header
  * @version 1.1
  * @date    2025-02-08
  ******************************************************************************
  */

#ifndef LV_PORT_INDEV_H
#define LV_PORT_INDEV_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "lvgl.h"
#include "xpt2046.h"
#include <stdint.h>

/* Exported functions --------------------------------------------------------*/

/**
 * @brief   Initialize LVGL input device (touchscreen)
 * @note    Call after lv_init() and lv_port_disp_init()
 * @retval  0 on success, -1 on error
 */
int lv_port_indev_init(void);

/**
 * @brief   Get XPT2046 touch handle
 * @retval  Pointer to touch handle
 */
XPT2046_HandleTypeDef *lv_port_indev_get_xpt2046_handle(void);

/**
 * @brief   Get LVGL input device object
 * @retval  Pointer to input device
 */
lv_indev_t *lv_port_indev_get_indev(void);

/**
 * @brief   Test touch by printing coordinates to log
 * @note    Useful for debugging touch issues
 */
void lv_port_indev_test_touch(void);

/**
 * @brief   Run touch calibration routine
 * @note    Interactive calibration for accurate touch mapping
 */
void lv_port_indev_calibrate(void);

#ifdef __cplusplus
}
#endif

#endif /* LV_PORT_INDEV_H */

