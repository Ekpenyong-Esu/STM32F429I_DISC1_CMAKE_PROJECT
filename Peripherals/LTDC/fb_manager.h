/* fb_manager.h
 * Minimal framebuffer manager for LTDC (RGB only)
 * Provides double-buffer allocation and VSYNC-safe swap helpers.
 */
#ifndef FB_MANAGER_H
#define FB_MANAGER_H

#include <stdint.h>
#include "stm32f4xx_hal.h"

int fb_manager_init(uint32_t width, uint32_t height);
/* Return pointer (address) to the back buffer where the caller should render. */
uint32_t fb_manager_get_backbuffer_address(void);
/* Mark the backbuffer as ready to be displayed. This will schedule a VSYNC-safe swap. */
void fb_manager_mark_backbuffer_ready(void);
/* Try to perform swap at VSYNC, blocking up to timeout_ms. Returns HAL_OK or HAL_TIMEOUT/HAL_ERROR. */
HAL_StatusTypeDef fb_manager_swap_blocking(uint32_t timeout_ms);
/* Get the currently active framebuffer address used by LTDC */
uint32_t fb_manager_get_active_address(void);

#endif /* FB_MANAGER_H */
