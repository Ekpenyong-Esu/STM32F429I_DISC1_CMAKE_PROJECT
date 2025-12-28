/* fb_manager.c
 * Simple framebuffer manager for LTDC double-buffering.
 * - Allocates two full-screen buffers at a fixed SDRAM base
 * - Exposes helper APIs to get backbuffer address and perform VSYNC-safe swaps
 *
 * Note: This implementation uses blocking HAL calls for simplicity. It can be
 * extended to use LTDC RR interrupt and DMA2D-accelerated copying.
 */

#include "fb_manager.h"
#include "ltdc.h"
#include <string.h>

/* Framebuffer base address in SDRAM (Bank 2). Update if your board differs. */
#ifndef FB_SDRAM_BASE
#define FB_SDRAM_BASE ((uintptr_t)0xD0000000U)
#endif

static uint32_t fb_width = 0;
static uint32_t fb_height = 0;
static uint32_t fb_pixels = 0;
static uint32_t fb_bytes = 0;
static uintptr_t fb_base = FB_SDRAM_BASE;

/* 0 or 1 active index */
static int active_idx = 0;
/* -1 when no pending buffer otherwise 0/1 */
static int pending_idx = -1;

/* Initialize manager with width/height in pixels. Returns 0 on success. */
int fb_manager_init(uint32_t width, uint32_t height)
{
    if (width == 0 || height == 0) return -1;
    fb_width = width;
    fb_height = height;
    fb_pixels = fb_width * fb_height;
    fb_bytes = fb_pixels * sizeof(uint16_t); /* assumes LVGL RGB565 */

    /* Basic sanity: ensure two buffers fit in SDRAM region (best effort) */
    /* Note: user must ensure SDRAM mapping is correct for their board */
    /* No malloc; fixed addresses used */

    /* Initialize LTDC to point at buffer 0 as the active framebuffer (best-effort) */
    extern LTDC_HandleTypeDef hltdc;
    HAL_StatusTypeDef st = HAL_LTDC_SetAddress(&hltdc, (uint32_t)fb_base, 0);
    if (st != HAL_OK) return -2;
    __HAL_LTDC_VERTICAL_BLANKING_RELOAD_CONFIG(&hltdc);
    /* attempt to wait briefly for RR flag */
    uint32_t timeout = 1000;
    while((__HAL_LTDC_GET_FLAG(&hltdc, LTDC_FLAG_RR) == RESET) && timeout--) {}
    if (__HAL_LTDC_GET_FLAG(&hltdc, LTDC_FLAG_RR) != RESET) {
        __HAL_LTDC_CLEAR_FLAG(&hltdc, LTDC_FLAG_RR);
    }

    active_idx = 0;
    pending_idx = -1;
    return 0;
}

uint32_t fb_manager_get_backbuffer_address(void)
{
    uintptr_t addr = fb_base + ((active_idx == 0) ? fb_bytes : 0);
    return (uint32_t)addr;
}

void fb_manager_mark_backbuffer_ready(void)
{
    pending_idx = (active_idx == 0) ? 1 : 0;
}

HAL_StatusTypeDef fb_manager_swap_blocking(uint32_t timeout_ms)
{
    if (pending_idx < 0) return HAL_OK; /* nothing to do */
    extern LTDC_HandleTypeDef hltdc;

    uintptr_t new_addr = fb_base + ((pending_idx == 0) ? 0 : fb_bytes);
    HAL_StatusTypeDef st = HAL_LTDC_SetAddress(&hltdc, (uint32_t)new_addr, 0);
    if (st != HAL_OK) return st;
    __HAL_LTDC_VERTICAL_BLANKING_RELOAD_CONFIG(&hltdc);

    uint32_t start = HAL_GetTick();
    while((__HAL_LTDC_GET_FLAG(&hltdc, LTDC_FLAG_RR) == RESET)) {
        if (HAL_GetTick() - start > timeout_ms) return HAL_TIMEOUT;
    }
    __HAL_LTDC_CLEAR_FLAG(&hltdc, LTDC_FLAG_RR);

    active_idx = pending_idx;
    pending_idx = -1;
    return HAL_OK;
}

uint32_t fb_manager_get_active_address(void)
{
    return (uint32_t)(fb_base + ((active_idx == 0) ? 0 : fb_bytes));
}
