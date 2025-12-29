/*******************************************************************************
 * LVGL Display Port - Connects LVGL to STM32F429I LCD Hardware (v9 API)
 *******************************************************************************
 * This file implements the display driver that LVGL uses to render graphics
 * to the physical LCD screen using LVGL v9 API.
 *
 * What this does:
 * - Allocates frame buffers for LVGL rendering
 * - Implements the flush callback that writes pixels to the display
 * - Creates and configures the display object
 *
 * For Beginners:
 * 1. The main thing you need to configure is the framebuffer address
 * 2. Once LTDC/display is initialized, set LVGL_FB_ADDR
 * 3. The flush function copies rendered graphics to the display memory
 ******************************************************************************/

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "lvgl.h" // Ensure all LVGL types/macros are available
#include "lv_port_disp.h"
#include "ltdc.h" // For LTDC_HandleTypeDef
#include "fb_manager.h"
//#include "dma2d.h" // For DMA2D graphics accelerator - disabled to prevent hangs

/*-----------------------------------------------------------------------------
 * Framebuffer Configuration
 *---------------------------------------------------------------------------*/
#define LV_HOR_RES_MAX      240    /* Display width in pixels (landscape) */
#define LV_VER_RES_MAX      320     /* Display height in pixels (landscape) */

/* Framebuffer address in SDRAM (configured in LTDC) */
/* Framebuffer base in SDRAM (Bank 2). We will use double buffering: two full-screen
 * framebuffers located at LVGL_FB_ADDR and LVGL_FB_ADDR + FB_BYTES.
 */
#define LVGL_FB_ADDR        0xD0000000

#ifndef LVGL_BGR_MODE
/* Try no conversion first. If colors are wrong, set to:
 * 1 = swap R/B fields (RGB565 <-> BGR565)
 * 2 = byte-swap each 16-bit word (endianness correction)
 */
#define LVGL_BGR_MODE 0
#endif

/*-----------------------------------------------------------------------------
 * Draw Buffer Configuration
 *---------------------------------------------------------------------------*/
/* LVGL v9 needs memory buffers to render graphics before sending to display.
 *
 * We use partial rendering with smaller buffers to save RAM.
 * Buffer size = screen width × number of lines × bytes per pixel
 */
#ifndef LVGL_DRAW_BUF_LINES
#define LVGL_DRAW_BUF_LINES 40  /* Number of screen lines to buffer (increase to reduce partial-update artifacts) */
#endif

/* Calculate buffer size for partial rendering */
#define LVGL_BUF_SIZE (LV_HOR_RES_MAX * LVGL_DRAW_BUF_LINES)

/* Allocate draw buffer in RAM (not SDRAM). Align to 32-bit for DMA friendliness. */
static lv_color_t lvgl_draw_buf[LVGL_BUF_SIZE] __attribute__((aligned(4)));

/* Global display object for callback access */
static lv_display_t *g_display = NULL;

/* Double-buffering state (file scope so timer and flush can access) */
/* Framebuffer state managed by fb_manager.c */

/*-----------------------------------------------------------------------------
 * DMA2D Transfer Complete Callback - DISABLED
 *---------------------------------------------------------------------------*/
#if 0 // Disabled to prevent hangs
static void dma2d_transfer_complete_callback(DMA2D_HandleTypeDef *hdma2d)
{
    (void)hdma2d;  /* Mark parameter as unused */
    /* Notify LVGL that the flush operation is complete */
    if (g_display != NULL) {
        lv_display_flush_ready(g_display);
    }
}
#endif

/*-----------------------------------------------------------------------------
 * Display Flush Callback - Core Rendering Function (v9 API)
 *---------------------------------------------------------------------------*/
/* This function is called by LVGL when it has rendered some pixels.
 * Our job: copy those pixels to the SDRAM framebuffer using DMA2D.
 *
 * Parameters:
 * - disp:    Display object (v9 API)
 * - area:    Rectangle defining the region to update (x1,y1 to x2,y2)
 * - px_map:  Pointer to pixel data in LVGL format
 */
static void disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    /* Calculate area dimensions */
    uint32_t width = lv_area_get_width(area);
    uint32_t height = lv_area_get_height(area);

    /* Clamp area to display bounds (defensive) */
    if (area->x1 < 0) return; /* simple guard; LVGL should not pass invalid areas */

    /* Use fb_manager to get the backbuffer address */
    uintptr_t back_fb_addr = (uintptr_t)fb_manager_get_backbuffer_address();
    uintptr_t dst_addr = back_fb_addr + ((area->y1 * LV_HOR_RES_MAX) + area->x1) * sizeof(lv_color_t);

    /* Use memcpy for reliable operation - DMA2D interrupt mode can cause hangs */
    /* Conversion mode:
     * 0 = no conversion (copy raw RGB565)
     * 1 = swap R/B fields (RGB565 -> BGR565)
     * 2 = byte-swap each 16-bit word (endian correction)
     */


    uint16_t *dst = (uint16_t *)dst_addr;
    uint16_t *src = (uint16_t *)px_map;

    if (LVGL_BGR_MODE == 0) {
        for (uint32_t y = 0; y < height; y++) {
            memcpy(dst, src, width * sizeof(uint16_t));
            dst += LV_HOR_RES_MAX;
            src += width;
        }
    } else if (LVGL_BGR_MODE == 1) {
        /* Field swap: R and B fields swapped */
        for (uint32_t y = 0; y < height; y++) {
            for (uint32_t x = 0; x < width; x++) {
                uint16_t v = src[x];
                uint16_t r = (v >> 11) & 0x1F;
                uint16_t g = (v >> 5) & 0x3F;
                uint16_t b = v & 0x1F;
                dst[x] = (uint16_t)((b << 11) | (g << 5) | r);
            }
            dst += LV_HOR_RES_MAX;
            src += width;
        }
    } else {
        /* Byte swap mode: swap high/low bytes of each 16-bit pixel */
        for (uint32_t y = 0; y < height; y++) {
            for (uint32_t x = 0; x < width; x++) {
                uint16_t v = src[x];
                dst[x] = (uint16_t)((v << 8) | (v >> 8));
            }
            dst += LV_HOR_RES_MAX;
            src += width;
        }
    }

    /* Only swap buffers at the end of the frame. In partial render mode LVGL may
     * call the flush callback multiple times per frame for different areas —
     * swapping the framebuffer on every flush results in mid-frame swaps which
     * produce flicker and incorrect colors. Use LVGL's API to detect the last
     * flush of the current refresh and do the VSYNC-safe swap only then. */
    if(lv_display_flush_is_last(disp)) {
        fb_manager_mark_backbuffer_ready();
        /* Use shorter timeout or non-blocking swap */
        (void)fb_manager_swap_blocking(5);  // ✅ Reduced from 50ms to 5ms
    }

    /* Notify LVGL that this flush region has been handled */
    lv_display_flush_ready(disp);
}
/* timer-based swap logic removed in favor of fb_manager APIs */

/*-----------------------------------------------------------------------------
 * Initialize Display Port (v9 API)
 *---------------------------------------------------------------------------*/
/* Call this function once during startup (after lv_init()).
 * It sets up LVGL's display driver with our hardware-specific settings.
 */
void lv_port_disp_init(void)
{
    /* Initialize framebuffer manager (double-buffering) */
    (void)fb_manager_init(LV_HOR_RES_MAX, LV_VER_RES_MAX);

    /* Step 1: Create a display object (v9 API)
     * This replaces lv_disp_drv_register from v8 */
    lv_display_t *disp = lv_display_create(LV_HOR_RES_MAX, LV_VER_RES_MAX);

    if(disp == NULL) {
        /* Display creation failed - should never happen */
        return;
    }

    /* Step 2: Set the flush callback */
    lv_display_set_flush_cb(disp, disp_flush);

    /* Step 3: Configure the draw buffers (v9 API)
     * Use partial rendering mode with single buffer for memory efficiency */
    lv_display_set_buffers(disp, lvgl_draw_buf, NULL, LVGL_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);

    /* Step 4: Set color format to RGB565 (16-bit) */
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);

    /* Store display object for callback access */
    g_display = disp;

    /* That's it! LVGL can now render to the display */
}
