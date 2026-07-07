/**
 * @file    lv_port_disp.c
 * @brief   LVGL Display Port for STM32F429I-DISC1 (LVGL v9 API)
 * @details Connects LVGL to ILI9488 LCD via SPI.
 * @version 2.0
 * @date    2026-01-03
 */

#include "lv_port_disp.h"
#include "lvgl.h"
#include "ili9488.h"
#include "main.h"
#include <stdint.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include "log.h"

/*-----------------------------------------------------------------------------
 * Display Configuration
 *---------------------------------------------------------------------------*/

/** Display dimensions (ILI9488 4.0" SPI) */
#define DISP_HOR_RES    320
#define DISP_VER_RES    480

/** Display orientation: ILI9488_ORIENTATION_PORTRAIT or ILI9488_ORIENTATION_LANDSCAPE */
/* The ILI9488 SPI panel is physically 320 wide × 480 tall (portrait).
 * Using LANDSCAPE sets MADCTL MV=1 which swaps rows and columns in hardware,
 * causing every LVGL pixel flush to appear rotated 90° on the physical panel.
 * The background flood-fill still works (it covers all pixels) but all widget
 * content appears as thin rotated strips → "icons invisible". */
#define DISP_ORIENTATION ILI9488_ORIENTATION_PORTRAIT

/** Bytes per pixel (RGB565 = 16-bit = 2 bytes) */
#define DISP_BPP        2

/* ILI9488 control pins (adjust if wired differently) */
#define LCD_CS_PORT     CSX_GPIO_Port
#define LCD_CS_PIN      CSX_Pin
#define LCD_DC_PORT     WRX_DCX_GPIO_Port
#define LCD_DC_PIN      WRX_DCX_Pin
#define LCD_RST_PORT    RDX_GPIO_Port
#define LCD_RST_PIN     RDX_Pin

/*-----------------------------------------------------------------------------
 * Draw Buffer Configuration
 *---------------------------------------------------------------------------*/

/** Number of lines to buffer for partial rendering (saves internal RAM) */
#define DRAW_BUF_LINES  20

/** Draw buffer size */
#define DRAW_BUF_SIZE   (DISP_HOR_RES * DRAW_BUF_LINES)

/** Draw buffer in internal RAM (not SDRAM) */
static lv_color_t draw_buf[DRAW_BUF_SIZE] __attribute__((aligned(4)));

static ILI9488_Handle_t s_lcd;

/* Logical display dimensions (adjusted for orientation) */
static uint16_t lvgl_disp_width;
static uint16_t lvgl_disp_height;

/*-----------------------------------------------------------------------------
 * Private Variables
 *---------------------------------------------------------------------------*/

/**
 * @brief   LVGL flush callback - copy rendered pixels to SDRAM framebuffer
 * @param   disp    Display object
 * @param   area    Area to update (x1,y1 to x2,y2)
 * @param   px_map  Pixel data from LVGL
 */
static void disp_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    if (area->x2 < 0 || area->y2 < 0 || area->x1 > lvgl_disp_width - 1 || area->y1 > lvgl_disp_height - 1) {
        lv_display_flush_ready(disp);
        return;
    }

    int32_t act_x1 = area->x1 < 0 ? 0 : area->x1;
    int32_t act_y1 = area->y1 < 0 ? 0 : area->y1;
    int32_t act_x2 = area->x2 > lvgl_disp_width - 1 ? lvgl_disp_width - 1 : area->x2;
    int32_t act_y2 = area->y2 > lvgl_disp_height - 1 ? lvgl_disp_height - 1 : area->y2;

    int32_t w = act_x2 - act_x1 + 1;
    int32_t h = act_y2 - act_y1 + 1;
    uint32_t size = (uint32_t)(w * h);

    int32_t src_w = area->x2 - area->x1 + 1;
    int32_t skip_x = act_x1 - area->x1;
    int32_t skip_y = act_y1 - area->y1;
    uint16_t *src = (uint16_t *)px_map + (skip_y * src_w) + skip_x;

    ILI9488_WritePixels(&s_lcd,
                        (uint16_t)act_x1, (uint16_t)act_y1,
                        (uint16_t)act_x2, (uint16_t)act_y2,
                        src, size);

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
    log_debug("LVGL: Initializing display port");

    /* Initialize ILI9488 with display dimensions */
    if (ILI9488_Init(&s_lcd, LCD_CS_PORT, LCD_CS_PIN, LCD_DC_PORT, LCD_DC_PIN, LCD_RST_PORT, LCD_RST_PIN,
                     DISP_HOR_RES, DISP_VER_RES) != ILI9488_OK) {
        log_error("LVGL: ILI9488 init failed");
        while (1) { }
    }

    ILI9488_SetOrientation(&s_lcd, DISP_ORIENTATION);

    /* Set logical display dimensions based on orientation */
    if (DISP_ORIENTATION == ILI9488_ORIENTATION_PORTRAIT) {
        lvgl_disp_width = DISP_HOR_RES;
        lvgl_disp_height = DISP_VER_RES;
    } else if (DISP_ORIENTATION == ILI9488_ORIENTATION_LANDSCAPE) {
        lvgl_disp_width = DISP_VER_RES;
        lvgl_disp_height = DISP_HOR_RES;
    } else {
        // Default to portrait
        lvgl_disp_width = DISP_HOR_RES;
        lvgl_disp_height = DISP_VER_RES;
    }

    /* Read controller ID bytes (if supported) and log them */
    uint8_t id[3] = {0};
    if (ILI9488_ReadID(&s_lcd, id, sizeof(id)) == ILI9488_OK) {
        log_debug("LVGL: ILI9488 ID bytes: 0x%02X 0x%02X 0x%02X", id[0], id[1], id[2]);
    } else {
        log_debug("LVGL: ILI9488 ReadID failed or not supported");
    }

    // /* Quick sanity test: fill screen with magenta briefly then black so we can verify
    //  * the controller and SPI bus are functional at startup. If the screen stays white
    //  * after this test, the issue is likely hardware (backlight, wiring) or the controller
    //  * isn't accepting commands.
    //  */
    // log_debug("LVGL: Running LCD sanity clear test (magenta->black)");
    // ILI9488_Clear(&s_lcd, ILI9488_COLOR_MAGENTA);
    // HAL_Delay(100);
    // ILI9488_Clear(&s_lcd, ILI9488_COLOR_BLACK);


    /* Create LVGL display (v9 API) */
    lv_display_t *disp = lv_display_create(lvgl_disp_width, lvgl_disp_height);
    if (!disp) {
        log_error("LVGL: Failed to create display");
        while (1);
    }


    /* Set flush callback */
    lv_display_set_flush_cb(disp, disp_flush_cb);

    /* Set color format BEFORE registering buffers so LVGL uses the correct
     * bytes-per-pixel when computing how many lines fit in the buffer. */
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);

    /* Configure partial draw buffer */
    lv_display_set_buffers(disp,
    draw_buf,
    NULL,
    sizeof(draw_buf),
    LV_DISPLAY_RENDER_MODE_PARTIAL);
    log_debug("LVGL: Display port initialized");
}

