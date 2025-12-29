/* fb_manager.c - CORRECTED VERSION */
#include "fb_manager.h"
#include "ltdc.h"
#include <string.h>

#ifndef FB_SDRAM_BASE
#define FB_SDRAM_BASE ((uintptr_t)0xD0000000U)
#endif

static uint32_t fb_width = 0;
static uint32_t fb_height = 0;
static uint32_t fb_pixels = 0;
static uint32_t fb_bytes = 0;
static uintptr_t fb_base = FB_SDRAM_BASE;

/* Which buffer LTDC is currently displaying (0 or 1) */
static int display_idx = 0;
/* Which buffer LVGL is currently drawing to (0 or 1) */
static int draw_idx = 1;
/* Swap pending flag */
static volatile int swap_pending = 0;

int fb_manager_init(uint32_t width, uint32_t height)
{
    if (width == 0 || height == 0) return -1;

    fb_width = width;
    fb_height = height;
    fb_pixels = fb_width * fb_height;
    fb_bytes = fb_pixels * sizeof(uint16_t); /* RGB565 */

    /* Clear both buffers */
    uint16_t *buffer0 = (uint16_t *)fb_base;
    uint16_t *buffer1 = (uint16_t *)(fb_base + fb_bytes);

    for (uint32_t i = 0; i < fb_pixels; i++) {
        buffer0[i] = 0x0000;
        buffer1[i] = 0x0000;
    }

    /* Set LTDC to display buffer 0 */
    extern LTDC_HandleTypeDef hltdc;
    HAL_StatusTypeDef st = HAL_LTDC_SetAddress(&hltdc, (uint32_t)fb_base, 0);
    if (st != HAL_OK) return -2;

    /* Use VSYNC reload */
    __HAL_LTDC_RELOAD_CONFIG(&hltdc);

    display_idx = 0;  /* LTDC displays buffer 0 */
    draw_idx = 1;     /* LVGL draws to buffer 1 */
    swap_pending = 0;

    return 0;
}

uint32_t fb_manager_get_backbuffer_address(void)
{
    /* Return the buffer LVGL should draw to (not being displayed) */
    return (uint32_t)(fb_base + (draw_idx * fb_bytes));
}

void fb_manager_mark_backbuffer_ready(void)
{
    swap_pending = 1;
}

HAL_StatusTypeDef fb_manager_swap_blocking(uint32_t timeout_ms)
{
    if (!swap_pending) return HAL_OK;

    extern LTDC_HandleTypeDef hltdc;

    /* Point LTDC to the buffer LVGL just finished drawing */
    uint32_t new_display_addr = (uint32_t)(fb_base + (draw_idx * fb_bytes));

    HAL_StatusTypeDef st = HAL_LTDC_SetAddress(&hltdc, new_display_addr, 0);
    if (st != HAL_OK) return st;

    /* Request reload at VSYNC */
    __HAL_LTDC_RELOAD_CONFIG(&hltdc);

    /* Wait for reload with timeout */
    uint32_t start = HAL_GetTick();
    while (__HAL_LTDC_GET_FLAG(&hltdc, LTDC_FLAG_RR) == RESET) {
        if ((HAL_GetTick() - start) > timeout_ms) {
            return HAL_TIMEOUT;
        }
    }
    __HAL_LTDC_CLEAR_FLAG(&hltdc, LTDC_FLAG_RR);

    /* Swap buffer indices */
    int temp = display_idx;
    display_idx = draw_idx;
    draw_idx = temp;

    swap_pending = 0;
    return HAL_OK;
}

uint32_t fb_manager_get_active_address(void)
{
    return (uint32_t)(fb_base + (display_idx * fb_bytes));
}
