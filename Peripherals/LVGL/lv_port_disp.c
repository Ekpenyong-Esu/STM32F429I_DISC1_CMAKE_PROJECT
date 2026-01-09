/**
 * @file    lv_port_disp.c
 * @brief   LVGL Display Port for STM32F429I-DISC1 (LVGL v9 API)
 * @details Connects LVGL to ILI9341 LCD via LTDC with SDRAM framebuffer.
 *          Based on ST BSP approach - direct SDRAM access, simple and reliable.
 * @version 2.0
 * @date    2026-01-03
 */

#include "lv_port_disp.h"
#include "fmc.h"
#include "lvgl.h"
#include "ltdc.h"
#include <stdint.h>
#include <string.h>
#include "stm32f4xx_hal.h"  /* Add for DMA */

/*-----------------------------------------------------------------------------
 * Display Configuration
 *---------------------------------------------------------------------------*/

/** Display dimensions (ILI9341 on STM32F429I-DISC1) */
#define DISP_HOR_RES    240
#define DISP_VER_RES    320

/** Bytes per pixel (RGB565 = 16-bit = 2 bytes) */
#define DISP_BPP        2

/** SDRAM framebuffer base address (Bank 2) */
#define FB_BASE_ADDR    SDRAM_DEVICE_ADDR

/** Single framebuffer size in bytes */
#define FB_SIZE         LTDC_FB_SIZE_RGB565

/** Framebuffer addresses for double-buffering */
#define FB0_ADDR        FB_BASE_ADDR
#define FB1_ADDR        (FB_BASE_ADDR + FB_SIZE)

/*-----------------------------------------------------------------------------
 * DMA Configuration for Display Flush
 *---------------------------------------------------------------------------*/

/** DMA Stream parameters for display flush */
#define DMA_STREAM               DMA2_Stream0
#define DMA_CHANNEL              DMA_CHANNEL_0
#define DMA_STREAM_IRQ           DMA2_Stream0_IRQn
#define DMA_STREAM_IRQHANDLER    DMA2_Stream0_IRQHandler

/*-----------------------------------------------------------------------------
 * Draw Buffer Configuration
 *---------------------------------------------------------------------------*/

/** Number of lines to buffer for partial rendering (saves internal RAM) */
#define DRAW_BUF_LINES  40

/** Draw buffer size */
#define DRAW_BUF_SIZE   (DISP_HOR_RES * DRAW_BUF_LINES)

/** Draw buffer in internal RAM (not SDRAM) */
static lv_color_t draw_buf[DRAW_BUF_SIZE] __attribute__((aligned(4)));

/*-----------------------------------------------------------------------------
 * Private Variables
 *---------------------------------------------------------------------------*/

/** Current active framebuffer (0 or 1) */
static uint8_t active_fb = 0;

/** DMA handle for display flush */
static DMA_HandleTypeDef dmaHandle;

/** LTDC handle reference (defined in ltdc.c) */
extern LTDC_HandleTypeDef hltdc;

/** Flush state variables */
static volatile uint8_t dma_transfer_complete = 0;
static lv_display_t *current_display = NULL;


/*-----------------------------------------------------------------------------
 * Private Functions
 *---------------------------------------------------------------------------*/

/**
 * @brief   Get current backbuffer address (for rendering)
 * @retval  Backbuffer address in SDRAM
 */
static inline uint32_t get_backbuffer_addr(void)
{
    return (active_fb == 0) ? FB1_ADDR : FB0_ADDR;
}

/**
 * @brief   Swap framebuffers (update LTDC to show backbuffer)
 */
static void swap_buffers(void)
{
    /* Toggle active buffer */
    active_fb = (active_fb == 0) ? 1 : 0;

    /* Update LTDC layer 0 address to new frontbuffer */
    uint32_t fb_addr = (active_fb == 0) ? FB0_ADDR : FB1_ADDR;
    HAL_LTDC_SetAddress(&hltdc, fb_addr, 0);

    /* Request vertical blanking reload for tear-free update */
    __HAL_LTDC_VERTICAL_BLANKING_RELOAD_CONFIG(&hltdc);
}

/**
 * @brief   DMA transfer complete callback
 */
static void dma_transfer_complete_callback(DMA_HandleTypeDef *hdma)
{
    dma_transfer_complete = 1;
    if (current_display) {
        lv_display_flush_ready(current_display);
    }
}

/**
 * @brief   Configure DMA for display flush operations
 */
static void dma_config(void)
{
    /* Enable DMA2 clock */
    __HAL_RCC_DMA2_CLK_ENABLE();

    /* Configure DMA handle */
    dmaHandle.Instance = DMA_STREAM;
    dmaHandle.Init.Channel = DMA_CHANNEL;
    dmaHandle.Init.Direction = DMA_MEMORY_TO_MEMORY;
    dmaHandle.Init.PeriphInc = DMA_PINC_ENABLE;
    dmaHandle.Init.MemInc = DMA_MINC_ENABLE;
    dmaHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;  /* 16-bit */
    dmaHandle.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;     /* 16-bit */
    dmaHandle.Init.Mode = DMA_NORMAL;
    dmaHandle.Init.Priority = DMA_PRIORITY_HIGH;
    dmaHandle.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    dmaHandle.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_1QUARTERFULL;
    dmaHandle.Init.MemBurst = DMA_MBURST_SINGLE;
    dmaHandle.Init.PeriphBurst = DMA_PBURST_SINGLE;

    /* Initialize DMA */
    if (HAL_DMA_Init(&dmaHandle) != HAL_OK) {
        /* Error handling */
        while(1);
    }

    /* Register completion callback */
    HAL_DMA_RegisterCallback(&dmaHandle, HAL_DMA_XFER_CPLT_CB_ID, dma_transfer_complete_callback);

    /* Configure NVIC */
    HAL_NVIC_SetPriority(DMA_STREAM_IRQ, 0, 0);
    HAL_NVIC_EnableIRQ(DMA_STREAM_IRQ);
}

/**
 * @brief   LVGL flush callback - copy rendered pixels to SDRAM framebuffer
 * @param   disp    Display object
 * @param   area    Area to update (x1,y1 to x2,y2)
 * @param   px_map  Pixel data from LVGL
 */
static void disp_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    /* Get area dimensions */
    int32_t width = lv_area_get_width(area);
    int32_t height = lv_area_get_height(area);

    /* Calculate destination address in SDRAM backbuffer */
    uint32_t backbuf = get_backbuffer_addr();
    uint16_t *dst = (uint16_t *)(backbuf + (area->y1 * DISP_HOR_RES + area->x1) * DISP_BPP);
    uint16_t *src = (uint16_t *)px_map;

    /* For partial rendering, we need to handle line-by-line copy */
    /* Start DMA transfer for first line */
    dma_transfer_complete = 0;
    current_display = disp;

    /* Copy line by line using DMA */
    for (int32_t y = 0; y < height; y++) {
        /* Wait for previous transfer to complete */
        while (!dma_transfer_complete && y > 0);

        dma_transfer_complete = 0;

        /* Start DMA transfer for this line */
        if (HAL_DMA_Start_IT(&dmaHandle, (uint32_t)src, (uint32_t)dst, width) != HAL_OK) {
            /* Fallback to memcpy if DMA fails */
            memcpy(dst, src, width * DISP_BPP);
            dma_transfer_complete = 1;
        }

        dst += DISP_HOR_RES;  /* Next line in framebuffer */
        src += width;         /* Next line in source */
    }

    /* Wait for final transfer to complete */
    while (!dma_transfer_complete);

    /* Swap buffers only on last flush of frame */
    if (lv_display_flush_is_last(disp)) {
        swap_buffers();
    }

    /* Notify LVGL flush is complete */
    lv_display_flush_ready(disp);
}

/*-----------------------------------------------------------------------------
 * Public Functions
 *---------------------------------------------------------------------------*/

/**
 * @brief   Initialize LVGL display driver
 * @note    Call after lv_init(), SDRAM init, ILI9341 init, and LTDC init
 */
void lv_port_disp_init(void)
{
    /* Configure DMA for display flush */
    dma_config();

    /* Clear both framebuffers to black */
    memset((void *)FB0_ADDR, 0x00, FB_SIZE);
    memset((void *)FB1_ADDR, 0x00, FB_SIZE);

    /* Set LTDC to show framebuffer 0 initially */
    active_fb = 0;
    HAL_LTDC_SetAddress(&hltdc, FB0_ADDR, 0);

    /* Create LVGL display (v9 API) */
    lv_display_t *disp = lv_display_create(DISP_HOR_RES, DISP_VER_RES);
    if (disp == NULL) {
        return;  /* Display creation failed */
    }

    /* Set flush callback */
    lv_display_set_flush_cb(disp, disp_flush_cb);

    /* Configure draw buffers for partial rendering */
    lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);

    /* Set color format to RGB565 */
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
}

/**
 * @brief   DMA Stream IRQ Handler
 */
void DMA_STREAM_IRQHANDLER(void)
{
    HAL_DMA_IRQHandler(&dmaHandle);
}
