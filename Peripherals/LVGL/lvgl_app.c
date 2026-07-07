/*******************************************************************************
 * lvgl_app.c  -  Clean Industrial GUI with Icons, Time, Date & Log
 *******************************************************************************
 * Target  : STM32F429I-DISC1   (320 x 480 px portrait)
 *
 * SCREENS
 * -------
 *  [1] HOME     - Header (time/date), 4 sensor tiles, activity log panel
 *  [2] SENSORS  - Live trend chart + 4 icon sensor cards
 *  [3] CONFIG   - Brightness/Volume sliders, WiFi/BT toggles
 *  [4] INFO     - MCU specifications card + uptime
 *  [5] PINS     - Scrollable GPIO pin list (sub-screen, back button)
 *  [6] PIN DET  - Individual pin detail with live HAL state poll
 *
 * NAVIGATION
 * ----------
 *  A 44 px bottom tab-bar is attached to the first four screens.
 *  Pins and Pin-Detail use a standard Back button.
 *
 * ICONS USED  (LVGL built-in symbols)
 * -----------
 *  LV_SYMBOL_CHARGE   (~clock / time)
 *  LV_SYMBOL_DRIVE    (~calendar / date)
 *  LV_SYMBOL_WARNING  (~temperature high)
 *  LV_SYMBOL_REFRESH  (~humidity / water)
 *  LV_SYMBOL_DOWNLOAD (~pressure)
 *  LV_SYMBOL_SHUFFLE  (~motion/accel)
 *  LV_SYMBOL_VOLUME_MAX, LV_SYMBOL_AUDIO
 *  LV_SYMBOL_WIFI, LV_SYMBOL_BLUETOOTH
 *  LV_SYMBOL_BELL     (log/alerts)
 *  LV_SYMBOL_EYE_OPEN (sensors screen)
 *  LV_SYMBOL_SETTINGS (config screen)
 *  LV_SYMBOL_CALL     (info screen)
 *  LV_SYMBOL_OK       (ACK)
 *  LV_SYMBOL_LEFT     (back navigation)
 *  LV_SYMBOL_HOME     (home tab)
 *  LV_SYMBOL_LIST     (sensors tab)
 *  LV_SYMBOL_EDIT     (pin detail)
 *
 * PUBLIC API  (unchanged - called by main.c):
 *   LVGL_App_Init()
 *   LVGL_App_Tick()
 *   LVGL_App_UpdateTemperature(int)
 *   LVGL_App_UpdateHumidity(int)
 *   LVGL_App_UpdateStatus(const char *)
 *   LVGL_App_AddChartData(int)
 ******************************************************************************/

#include <src/font/lv_font.h>
#include <stdint.h>
#include <string.h>
#include "lvgl.h"
#include "lvgl_app.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "main.h"
#include "stm32f4xx_hal.h"
#include "log.h"

/*=============================================================================
 * COLOUR PALETTE
 *===========================================================================*/
#define C_BG       0x0D1117   /* near-black background                */
#define C_PANEL    0x161B22   /* card / panel background              */
#define C_BORDER   0x30363D   /* subtle border                        */
#define C_ACCENT   0x58A6FF   /* blue accent (active tab, highlights) */
#define C_OK       0x3FB950   /* green  - normal                      */
#define C_WARN     0xD29922   /* amber  - warning                     */
#define C_FAULT    0xF85149   /* red    - fault                       */
#define C_TXT      0xE6EDF3   /* primary text                         */
#define C_DIM      0x8B949E   /* secondary / dim text                 */
#define C_HDR      0x0D1117   /* header bar background                */

/*=============================================================================
 * SCREEN HANDLES
 *===========================================================================*/
static lv_obj_t *scr_home;
static lv_obj_t *scr_sensors;
static lv_obj_t *scr_config;
static lv_obj_t *scr_info;
static lv_obj_t *scr_pins;
static lv_obj_t *scr_pin_detail;

/* Tab-bar target array — filled after all screens are created */
static lv_obj_t *tab_screens[4];

/*=============================================================================
 * WIDGET HANDLES UPDATED BY PUBLIC API
 *===========================================================================*/

/* HOME tiles */
static lv_obj_t *tile_temp_val;    /* temperature value label   */
static lv_obj_t *tile_hum_val;     /* humidity value label      */
static lv_obj_t *tile_pres_val;    /* pressure value label      */
static lv_obj_t *tile_acc_val;     /* accel value label         */
static lv_obj_t *tile_temp_dot;    /* health dot - temp tile    */
static lv_obj_t *tile_hum_dot;     /* health dot - humid tile   */

/* HOME log */
static lv_obj_t *log_list;         /* scrollable log panel      */
static lv_obj_t *status_label;     /* status strip label        */

/* HOME / global time+date */
static lv_obj_t *hdr_time_lbl;     /* HH:MM:SS on home screen   */
static lv_obj_t *hdr_date_lbl;     /* date string on home screen*/

/* SENSORS */
static lv_obj_t *chart_sensor;
static lv_chart_series_t *chart_series;

/* INFO */
static lv_obj_t *info_uptime_lbl;

/* PINS */
static lv_obj_t *pin_name_label;
static lv_obj_t *pin_port_label;
static lv_obj_t *pin_nature_label;
static lv_obj_t *pin_state_label;
static lv_timer_t *pin_detail_timer = NULL;

/* Navigation button handles */
static lv_obj_t *btn_home_sensors;
static lv_obj_t *btn_home_config;
static lv_obj_t *btn_sensors_back;
static lv_obj_t *btn_config_info;
static lv_obj_t *btn_config_back;
static lv_obj_t *btn_info_back;
static lv_obj_t *btn_home_pins;
static lv_obj_t *btn_pins_back;
static lv_obj_t *btn_pin_detail_back;

/*=============================================================================
 * PIN TABLE (kept from original code)
 *===========================================================================*/
typedef struct {
    const char   *name;
    GPIO_TypeDef *port;
    uint16_t      pin;
} PinInfo;

static const PinInfo *current_pin_info = NULL;

#define PIN_ENTRY(n, p, pi) {n, p, pi}
static const PinInfo pin_table[] = {
    PIN_ENTRY("PC14_OSC32_IN",  PC14_OSC32_IN_GPIO_Port,  PC14_OSC32_IN_Pin),
    PIN_ENTRY("PC15_OSC32_OUT", PC15_OSC32_OUT_GPIO_Port, PC15_OSC32_OUT_Pin),
    PIN_ENTRY("A0",  A0_GPIO_Port,  A0_Pin),
    PIN_ENTRY("A1",  A1_GPIO_Port,  A1_Pin),
    PIN_ENTRY("A2",  A2_GPIO_Port,  A2_Pin),
    PIN_ENTRY("A3",  A3_GPIO_Port,  A3_Pin),
    PIN_ENTRY("A4",  A4_GPIO_Port,  A4_Pin),
    PIN_ENTRY("A5",  A5_GPIO_Port,  A5_Pin),
    PIN_ENTRY("SPI4_SCK",  SPI4_SCK_GPIO_Port,  SPI4_SCK_Pin),
    PIN_ENTRY("SPI4_MISO", SPI4_MISO_GPIO_Port, SPI4_MISO_Pin),
    PIN_ENTRY("SPI4_MOSI", SPI4_MOSI_GPIO_Port, SPI4_MOSI_Pin),
    PIN_ENTRY("ENABLE",  ENABLE_GPIO_Port,  ENABLE_Pin),
    PIN_ENTRY("PH0_OSC_IN",  PH0_OSC_IN_GPIO_Port,  PH0_OSC_IN_Pin),
    PIN_ENTRY("PH1_OSC_OUT", PH1_OSC_OUT_GPIO_Port, PH1_OSC_OUT_Pin),
    PIN_ENTRY("SDNWE",       SDNWE_GPIO_Port,       SDNWE_Pin),
    PIN_ENTRY("NCS_MEMS_SPI",NCS_MEMS_SPI_GPIO_Port,NCS_MEMS_SPI_Pin),
    PIN_ENTRY("CSX",         CSX_GPIO_Port,          CSX_Pin),
    PIN_ENTRY("B1",          B1_GPIO_Port,            B1_Pin),
    PIN_ENTRY("MEMS_INT1",   MEMS_INT1_GPIO_Port,    MEMS_INT1_Pin),
    PIN_ENTRY("MEMS_INT2",   MEMS_INT2_GPIO_Port,    MEMS_INT2_Pin),
    PIN_ENTRY("B5",  B5_GPIO_Port,  B5_Pin),
    PIN_ENTRY("VSYNC",VSYNC_GPIO_Port,VSYNC_Pin),
    PIN_ENTRY("G2",  G2_GPIO_Port,  G2_Pin),
    PIN_ENTRY("ACP_RST",   ACP_RST_GPIO_Port,   ACP_RST_Pin),
    PIN_ENTRY("OTG_FS_PSO",OTG_FS_PSO_GPIO_Port,OTG_FS_PSO_Pin),
    PIN_ENTRY("OTG_FS_OC", OTG_FS_OC_GPIO_Port, OTG_FS_OC_Pin),
    PIN_ENTRY("R3",R3_GPIO_Port,R3_Pin),
    PIN_ENTRY("R6",R6_GPIO_Port,R6_Pin),
    PIN_ENTRY("BOOT1",BOOT1_GPIO_Port,BOOT1_Pin),
    PIN_ENTRY("SDNRAS",SDNRAS_GPIO_Port,SDNRAS_Pin),
    PIN_ENTRY("A6", A6_GPIO_Port, A6_Pin),
    PIN_ENTRY("A7", A7_GPIO_Port, A7_Pin),
    PIN_ENTRY("A8", A8_GPIO_Port, A8_Pin),
    PIN_ENTRY("A9", A9_GPIO_Port, A9_Pin),
    PIN_ENTRY("A10",A10_GPIO_Port,A10_Pin),
    PIN_ENTRY("A11",A11_GPIO_Port,A11_Pin),
    PIN_ENTRY("D4", D4_GPIO_Port, D4_Pin),
    PIN_ENTRY("D5", D5_GPIO_Port, D5_Pin),
    PIN_ENTRY("D6", D6_GPIO_Port, D6_Pin),
    PIN_ENTRY("D7", D7_GPIO_Port, D7_Pin),
    PIN_ENTRY("D8", D8_GPIO_Port, D8_Pin),
    PIN_ENTRY("D9", D9_GPIO_Port, D9_Pin),
    PIN_ENTRY("D10",D10_GPIO_Port,D10_Pin),
    PIN_ENTRY("D11",D11_GPIO_Port,D11_Pin),
    PIN_ENTRY("D12",D12_GPIO_Port,D12_Pin),
    PIN_ENTRY("G4",G4_GPIO_Port,G4_Pin),
    PIN_ENTRY("G5",G5_GPIO_Port,G5_Pin),
    PIN_ENTRY("OTG_HS_ID",OTG_HS_ID_GPIO_Port,OTG_HS_ID_Pin),
    PIN_ENTRY("VBUS_HS",  VBUS_HS_GPIO_Port,  VBUS_HS_Pin),
    PIN_ENTRY("OTG_HS_DM",OTG_HS_DM_GPIO_Port,OTG_HS_DM_Pin),
    PIN_ENTRY("OTG_HS_DP",OTG_HS_DP_GPIO_Port,OTG_HS_DP_Pin),
    PIN_ENTRY("D13",D13_GPIO_Port,D13_Pin),
    PIN_ENTRY("D14",D14_GPIO_Port,D14_Pin),
    PIN_ENTRY("D15",D15_GPIO_Port,D15_Pin),
    PIN_ENTRY("TE",    TE_GPIO_Port,    TE_Pin),
    PIN_ENTRY("RDX",   RDX_GPIO_Port,   RDX_Pin),
    PIN_ENTRY("WRX_DCX",WRX_DCX_GPIO_Port,WRX_DCX_Pin),
    PIN_ENTRY("D0",D0_GPIO_Port,D0_Pin),
    PIN_ENTRY("D1",D1_GPIO_Port,D1_Pin),
    PIN_ENTRY("BA0",BA0_GPIO_Port,BA0_Pin),
    PIN_ENTRY("BA1",BA1_GPIO_Port,BA1_Pin),
    PIN_ENTRY("R7",R7_GPIO_Port,R7_Pin),
    PIN_ENTRY("DOTCLK",DOTCLK_GPIO_Port,DOTCLK_Pin),
    PIN_ENTRY("SDCLK", SDCLK_GPIO_Port, SDCLK_Pin),
    PIN_ENTRY("HSYNC", HSYNC_GPIO_Port, HSYNC_Pin),
    PIN_ENTRY("G6",G6_GPIO_Port,G6_Pin),
    PIN_ENTRY("I2C3_SDA",I2C3_SDA_GPIO_Port,I2C3_SDA_Pin),
    PIN_ENTRY("I2C3_SCL",I2C3_SCL_GPIO_Port,I2C3_SCL_Pin),
    PIN_ENTRY("STLINK_RX",STLINK_RX_GPIO_Port,STLINK_RX_Pin),
    PIN_ENTRY("STLINK_TX",STLINK_TX_GPIO_Port,STLINK_TX_Pin),
    PIN_ENTRY("R4",R4_GPIO_Port,R4_Pin),
    PIN_ENTRY("R5",R5_GPIO_Port,R5_Pin),
    PIN_ENTRY("SWDIO",SWDIO_GPIO_Port,SWDIO_Pin),
    PIN_ENTRY("SWCLK",SWCLK_GPIO_Port,SWCLK_Pin),
    PIN_ENTRY("TP_INT1",TP_INT1_GPIO_Port,TP_INT1_Pin),
    PIN_ENTRY("R2",R2_GPIO_Port,R2_Pin),
    PIN_ENTRY("D2",D2_GPIO_Port,D2_Pin),
    PIN_ENTRY("D3",D3_GPIO_Port,D3_Pin),
    PIN_ENTRY("G7",G7_GPIO_Port,G7_Pin),
    PIN_ENTRY("B2",B2_GPIO_Port,B2_Pin),
    PIN_ENTRY("G3",G3_GPIO_Port,G3_Pin),
    PIN_ENTRY("B3",B3_GPIO_Port,B3_Pin),
    PIN_ENTRY("B4",B4_GPIO_Port,B4_Pin),
    PIN_ENTRY("LD3",LD3_GPIO_Port,LD3_Pin),
    PIN_ENTRY("LD4",LD4_GPIO_Port,LD4_Pin),
    PIN_ENTRY("SDNCAS",SDNCAS_GPIO_Port,SDNCAS_Pin),
    PIN_ENTRY("SDCKE1",SDCKE1_GPIO_Port,SDCKE1_Pin),
    PIN_ENTRY("SDNE1", SDNE1_GPIO_Port, SDNE1_Pin),
    PIN_ENTRY("B6",B6_GPIO_Port,B6_Pin),
    PIN_ENTRY("B7",B7_GPIO_Port,B7_Pin),
    PIN_ENTRY("NBL0",NBL0_GPIO_Port,NBL0_Pin),
    PIN_ENTRY("NBL1",NBL1_GPIO_Port,NBL1_Pin),
};

/*=============================================================================
 * FORWARD DECLARATIONS
 *===========================================================================*/
static void create_home_screen(void);
static void create_sensors_screen(void);
static void create_config_screen(void);
static void create_info_screen(void);
static void create_pins_screen(void);
static void create_pin_detail_screen(void);

static void attach_tab_bar(lv_obj_t *screen, int active_idx);
static lv_obj_t *make_header(lv_obj_t *screen, const char *title_icon,
                              const char *title_text, bool show_clock);
static lv_obj_t *make_card(lv_obj_t *parent, int x, int y, int w, int h);
static lv_obj_t *make_label(lv_obj_t *parent, const char *text,
                              const lv_font_t *font, uint32_t col);

static void tab_event_cb(lv_event_t *e);
static void nav_event_handler(lv_event_t *e);
static void pin_item_event_handler(lv_event_t *e);
static void pin_detail_timer_cb(lv_timer_t *t);
static void pin_detail_back_cb(lv_event_t *e);
static void clock_timer_cb(lv_timer_t *t);
static void uptime_timer_cb(lv_timer_t *t);
static void log_append(const char *icon, const char *msg);

static const char *pin_port_name(GPIO_TypeDef *port);
static uint8_t     pin_index(uint16_t pin);
static const char *pin_nature_from_name(const char *name);

/*=============================================================================
 * HELPERS
 *===========================================================================*/

static lv_obj_t *make_card(lv_obj_t *parent, int x, int y, int w, int h)
{
    lv_obj_t *c = lv_obj_create(parent);
    lv_obj_set_size(c, w, h);
    lv_obj_set_pos(c, x, y);
    lv_obj_set_style_bg_color(c, lv_color_hex(C_PANEL), 0);
    lv_obj_set_style_border_color(c, lv_color_hex(C_BORDER), 0);
    lv_obj_set_style_border_width(c, 1, 0);
    lv_obj_set_style_radius(c, 8, 0);
    lv_obj_set_style_pad_all(c, 6, 0);
    lv_obj_clear_flag(c, LV_OBJ_FLAG_SCROLLABLE);
    return c;
}

static lv_obj_t *make_label(lv_obj_t *parent, const char *text,
                              const lv_font_t *font, uint32_t col)
{
    lv_obj_t *l = lv_label_create(parent);
    lv_label_set_text(l, text);
    lv_obj_set_style_text_color(l, lv_color_hex(col), 0);
    lv_obj_set_style_text_font(l, font, 0);
    return l;
}

/*-----------------------------------------------------------------------------
 * make_header
 *
 * Creates a 320x44 dark header bar at the top of every screen.
 * Contains:
 *   Left  - icon + title text
 *   Right - LV_SYMBOL_CHARGE time label  |  LV_SYMBOL_DRIVE date label
 *           (only on home screen; other screens show icon + title only)
 *
 * Returns the header container so callers can add extra widgets if needed.
 *---------------------------------------------------------------------------*/
static lv_obj_t *make_header(lv_obj_t *screen, const char *title_icon,
                               const char *title_text, bool show_clock)
{
    lv_obj_t *hdr = lv_obj_create(screen);
    lv_obj_set_size(hdr, lv_pct(100), 44);
    lv_obj_align(hdr, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(hdr, lv_color_hex(C_HDR), 0);
    lv_obj_set_style_border_width(hdr, 0, 0);
    lv_obj_set_style_border_side(hdr, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_color(hdr, lv_color_hex(C_BORDER), LV_PART_MAIN);
    lv_obj_set_style_border_width(hdr, 1, LV_PART_MAIN);
    lv_obj_set_style_pad_all(hdr, 0, 0);
    lv_obj_clear_flag(hdr, LV_OBJ_FLAG_SCROLLABLE);

    /* Icon + title (left side) */
    char full_title[48];
    lv_snprintf(full_title, sizeof(full_title), "%s %s", title_icon, title_text);
    lv_obj_t *title_lbl = make_label(hdr, full_title, &lv_font_montserrat_14, C_TXT);
    lv_obj_align(title_lbl, LV_ALIGN_LEFT_MID, 10, 0);

    if (show_clock) {
        /* Time (right side, above date)  —  updated by clock_timer_cb */
        hdr_time_lbl = lv_label_create(hdr);
        lv_label_set_text(hdr_time_lbl, LV_SYMBOL_CHARGE " 00:00:00");
        lv_obj_set_style_text_color(hdr_time_lbl, lv_color_hex(C_ACCENT), 0);
        lv_obj_set_style_text_font(hdr_time_lbl, &lv_font_montserrat_14, 0);
        lv_obj_align(hdr_time_lbl, LV_ALIGN_RIGHT_MID, -8, -7);

        /* Date (right side, below time) */
        hdr_date_lbl = lv_label_create(hdr);
        lv_label_set_text(hdr_date_lbl, LV_SYMBOL_DRIVE " Day 1");
        lv_obj_set_style_text_color(hdr_date_lbl, lv_color_hex(C_DIM), 0);
        lv_obj_set_style_text_font(hdr_date_lbl, &lv_font_montserrat_14, 0);
        lv_obj_align(hdr_date_lbl, LV_ALIGN_RIGHT_MID, -8, 8);
    }

    return hdr;
}

/*-----------------------------------------------------------------------------
 * BOTTOM TAB BAR
 *
 * Four equal 80 px buttons spanning the full display width.
 * Each displays a symbol + short label.
 * Active tab: accent fill + white top-line.
 *---------------------------------------------------------------------------*/
static const char *tab_icons[4] = {
    LV_SYMBOL_HOME,
    LV_SYMBOL_EYE_OPEN,
    LV_SYMBOL_SETTINGS,
    LV_SYMBOL_CALL
};
static const char *tab_names[4] = { "Home", "Sensors", "Config", "Info" };

static void tab_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    int idx = (int)(intptr_t)lv_event_get_user_data(e);
    if (idx < 0 || idx > 3 || !tab_screens[idx]) return;
    lv_screen_load_anim(tab_screens[idx], LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, false);
}

static void attach_tab_bar(lv_obj_t *screen, int active_idx)
{
    lv_obj_t *bar = lv_obj_create(screen);
    lv_obj_set_size(bar, lv_pct(100), 44);
    lv_obj_align(bar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x010409), 0);
    lv_obj_set_style_border_width(bar, 0, 0);
    lv_obj_set_style_border_side(bar, LV_BORDER_SIDE_TOP, 0);
    lv_obj_set_style_border_color(bar, lv_color_hex(C_BORDER), LV_PART_MAIN);
    lv_obj_set_style_border_width(bar, 1, LV_PART_MAIN);
    lv_obj_set_style_pad_all(bar, 0, 0);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);
    /* Flex row makes each button share width equally regardless of display
     * orientation or resolution — no hardcoded pixel widths needed. */
    lv_obj_set_layout(bar, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bar, LV_FLEX_ALIGN_SPACE_EVENLY,
                           LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    for (int i = 0; i < 4; i++) {
        bool active = (i == active_idx);

        lv_obj_t *btn = lv_btn_create(bar);
        lv_obj_set_flex_grow(btn, 1);
        lv_obj_set_height(btn, 44);
        lv_obj_set_style_radius(btn, 0, 0);
        lv_obj_set_style_border_width(btn, 0, 0);
        lv_obj_set_style_pad_all(btn, 0, 0);          /* override theme PAD_DEF so icon+label fit */
        lv_obj_set_style_bg_color(btn,
            lv_color_hex(active ? 0x1A2332 : 0x010409), 0);
        lv_obj_set_style_shadow_width(btn, 0, 0);

        /* White top accent on active tab */
        if (active) {
            lv_obj_set_style_border_side(btn, LV_BORDER_SIDE_TOP, 0);
            lv_obj_set_style_border_width(btn, 2, 0);
            lv_obj_set_style_border_color(btn, lv_color_hex(C_ACCENT), 0);
        }

        /* Icon */
        lv_obj_t *icon_lbl = lv_label_create(btn);
        lv_label_set_text(icon_lbl, tab_icons[i]);
        lv_obj_set_style_text_color(icon_lbl,
            lv_color_hex(active ? C_ACCENT : C_DIM), 0);
        lv_obj_set_style_text_font(icon_lbl, &lv_font_montserrat_16, 0);
        lv_obj_align(icon_lbl, LV_ALIGN_TOP_MID, 0, 4);

        /* Label */
        lv_obj_t *name_lbl = lv_label_create(btn);
        lv_label_set_text(name_lbl, tab_names[i]);
        lv_obj_set_style_text_color(name_lbl,
            lv_color_hex(active ? C_ACCENT : C_DIM), 0);
        lv_obj_set_style_text_font(name_lbl, &lv_font_montserrat_14, 0);
        lv_obj_align(name_lbl, LV_ALIGN_BOTTOM_MID, 0, -2);

        lv_obj_add_event_cb(btn, tab_event_cb, LV_EVENT_CLICKED,
                             (void *)(intptr_t)i);
    }
}

/*-----------------------------------------------------------------------------
 * Navigation handler (for pins / detail back buttons)
 *---------------------------------------------------------------------------*/
static void nav_event_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    lv_obj_t *target = (lv_obj_t *)lv_event_get_user_data(e);
    if (!target) return;
    lv_screen_load_anim(target, LV_SCR_LOAD_ANIM_FADE_ON, 250, 0, false);
}

/*-----------------------------------------------------------------------------
 * LOG PANEL HELPER
 *
 * Prepends a new entry (icon + timestamp + message) at the top of log_list.
 * A running second counter is used as the timestamp source.
 *---------------------------------------------------------------------------*/
static uint32_t log_seconds = 0;   /* maintained by uptime_timer_cb */

static void log_append(const char *icon, const char *msg)
{
    if (!log_list) return;
    char buf[80];
    uint32_t h = log_seconds / 3600;
    uint32_t m = (log_seconds % 3600) / 60;
    uint32_t s = log_seconds % 60;
    lv_snprintf(buf, sizeof(buf), "%s [%02lu:%02lu:%02lu]  %s",
                icon,
                (unsigned long)h,
                (unsigned long)m,
                (unsigned long)s,
                msg);
    lv_list_add_btn(log_list, NULL, buf);
}

/*=============================================================================
 * SCREEN 1 — HOME
 *
 * Layout (320 x 480 portrait):
 *    0 – 44    Header bar: icon + "STM32 Monitor", time, date
 *   46 – 56    Status strip: system status text + bell icon placeholder
 *   58 – 210   Sensor tile grid: 2 x 2 (152 x 72 px each)
 *  214 – 430   Activity log panel (scrollable lv_list)
 *  436 – 480   Bottom tab bar
 *===========================================================================*/
static void create_home_screen(void)
{
    scr_home = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_home, lv_color_hex(C_BG), 0);
    lv_obj_clear_flag(scr_home, LV_OBJ_FLAG_SCROLLABLE);

    /* Header with clock */
    make_header(scr_home, LV_SYMBOL_HOME, "STM32 Monitor", true);

    /* Status strip */
    lv_obj_t *status_strip = lv_obj_create(scr_home);
    lv_obj_set_size(status_strip, 320, 18);
    lv_obj_set_pos(status_strip, 0, 45);
    lv_obj_set_style_bg_color(status_strip, lv_color_hex(0x0A1628), 0);
    lv_obj_set_style_border_width(status_strip, 0, 0);
    lv_obj_set_style_pad_all(status_strip, 0, 0);
    lv_obj_clear_flag(status_strip, LV_OBJ_FLAG_SCROLLABLE);

    status_label = make_label(status_strip,
        LV_SYMBOL_OK "  System Ready",
        &lv_font_montserrat_14, C_OK);
    lv_obj_align(status_label, LV_ALIGN_LEFT_MID, 8, 0);

    lv_obj_t *bell = make_label(status_strip,
        LV_SYMBOL_BELL,
        &lv_font_montserrat_14, C_WARN);
    lv_obj_align(bell, LV_ALIGN_RIGHT_MID, -8, 0);

    /*-----------------------------------------------------------------
     * Sensor tiles  (2 x 2 grid)
     *
     * Each tile has:
     *   - coloured left accent stripe
     *   - LVGL symbol icon (top-left)
     *   - sensor name (dim, small)
     *   - large live value (updated by API)
     *   - health dot (top-right, green / amber / red)
     *-----------------------------------------------------------------*/
    typedef struct { const char *icon; const char *name; const char *unit;
                     uint32_t stripe; lv_obj_t **val_ptr; lv_obj_t **dot_ptr;
    } TileDef;

    static TileDef tiles[4] = {
        { LV_SYMBOL_WARNING, "TEMPERATURE", "\xC2\xB0""C",
          0xF85149, &tile_temp_val, &tile_temp_dot },
        { LV_SYMBOL_REFRESH, "HUMIDITY",    "% RH",
          0x58A6FF, &tile_hum_val,  &tile_hum_dot  },
        { LV_SYMBOL_DOWNLOAD,"PRESSURE",   "hPa",
          0xD29922, &tile_pres_val, NULL           },
        { LV_SYMBOL_SHUFFLE, "ACCEL",      "mg",
          0x3FB950, &tile_acc_val,  NULL           },
    };

    for (int i = 0; i < 4; i++) {
        int col = i % 2;
        int row = i / 2;
        int tx  = 4   + col * 158;
        int ty  = 64  + row * 76;

        lv_obj_t *tile = make_card(scr_home, tx, ty, 152, 70);

        /* Accent stripe */
        lv_obj_t *stripe = lv_obj_create(tile);
        lv_obj_set_size(stripe, 4, 40);
        lv_obj_set_pos(stripe, 0, 12);
        lv_obj_set_style_bg_color(stripe, lv_color_hex(tiles[i].stripe), 0);
        lv_obj_set_style_border_width(stripe, 0, 0);
        lv_obj_set_style_radius(stripe, 2, 0);

        /* Icon */
        lv_obj_t *icon_lbl = make_label(tile, tiles[i].icon,
                                          &lv_font_montserrat_16,
                                          tiles[i].stripe);
        lv_obj_set_pos(icon_lbl, 10, 4);

        /* Sensor name */
        lv_obj_t *name_lbl = make_label(tile, tiles[i].name,
                                          &lv_font_montserrat_14, C_DIM);
        lv_obj_set_pos(name_lbl, 32, 6);

        /* Live value */
        *tiles[i].val_ptr = make_label(tile, "--",
                                        &lv_font_montserrat_16, C_TXT);
        lv_obj_set_pos(*tiles[i].val_ptr, 10, 36);

        /* Unit */
        lv_obj_t *unit_lbl = make_label(tile, tiles[i].unit,
                                          &lv_font_montserrat_14, C_DIM);
        lv_obj_align_to(unit_lbl, *tiles[i].val_ptr,
                         LV_ALIGN_OUT_RIGHT_BOTTOM, 4, 0);

        /* Health dot */
        if (tiles[i].dot_ptr) {
            *tiles[i].dot_ptr = lv_obj_create(tile);
            lv_obj_set_size(*tiles[i].dot_ptr, 10, 10);
            lv_obj_set_style_radius(*tiles[i].dot_ptr, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_color(*tiles[i].dot_ptr,
                                       lv_color_hex(C_OK), 0);
            lv_obj_set_style_border_width(*tiles[i].dot_ptr, 0, 0);
            lv_obj_align(*tiles[i].dot_ptr, LV_ALIGN_TOP_RIGHT, -4, 4);
        }
    }

    /*-----------------------------------------------------------------
     * Activity Log Panel
     *
     * A scrollable lv_list positioned below the sensor tiles.
     * New entries are added via log_append() which is called from
     * LVGL_App_UpdateStatus() and LVGL_App_UpdateTemperature().
     *-----------------------------------------------------------------*/
    lv_obj_t *log_card = make_card(scr_home, 4, 218, 312, 148);

    /* Log header row */
    lv_obj_t *log_icon = make_label(log_card,
        LV_SYMBOL_BELL "  ACTIVITY LOG",
        &lv_font_montserrat_14, C_ACCENT);
    lv_obj_set_pos(log_icon, 4, 0);

    /* Divider line under log header */
    lv_obj_t *div = lv_obj_create(log_card);
    lv_obj_set_size(div, 296, 1);
    lv_obj_set_pos(div, 4, 18);
    lv_obj_set_style_bg_color(div, lv_color_hex(C_BORDER), 0);
    lv_obj_set_style_border_width(div, 0, 0);

    /* Scrollable log list */
    log_list = lv_list_create(log_card);
    lv_obj_set_size(log_list, 298, 116);
    lv_obj_set_pos(log_list, 2, 22);
    lv_obj_set_style_bg_color(log_list, lv_color_hex(C_PANEL), 0);
    lv_obj_set_style_border_width(log_list, 0, 0);
    lv_obj_set_style_text_font(log_list, &lv_font_montserrat_14, LV_PART_MAIN);

    /* Seed a few initial log entries */
    log_append(LV_SYMBOL_OK,      "System boot complete");
    log_append(LV_SYMBOL_CHARGE,  "LVGL initialised");
    log_append(LV_SYMBOL_REFRESH, "Sensor loop started");

    /* Bottom nav */
    attach_tab_bar(scr_home, 0);

    /* Save nav button handles */
    (void)btn_home_sensors;
    (void)btn_home_config;
    (void)btn_home_pins;
}

/*=============================================================================
 * SCREEN 2 — SENSORS
 *
 * Layout:
 *    0 – 44   Header
 *   48 – 238  Trend chart card (full-width)
 *  240 – 432  4 sensor detail cards in a row
 *  436 – 480  Bottom tab bar
 *===========================================================================*/
static void create_sensors_screen(void)
{
    scr_sensors = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_sensors, lv_color_hex(C_BG), 0);
    lv_obj_clear_flag(scr_sensors, LV_OBJ_FLAG_SCROLLABLE);

    make_header(scr_sensors, LV_SYMBOL_EYE_OPEN, "Sensor Monitor", false);

    /* Chart card */
    lv_obj_t *chart_card = make_card(scr_sensors, 4, 50, 312, 192);

    make_label(chart_card,
        LV_SYMBOL_WARNING "  TEMPERATURE TREND  (20 samples)",
        &lv_font_montserrat_14, C_DIM);

    chart_sensor = lv_chart_create(chart_card);
    lv_obj_set_size(chart_sensor, 294, 158);
    lv_obj_set_pos(chart_sensor, 4, 22);
    lv_chart_set_type(chart_sensor, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(chart_sensor, 20);
    lv_chart_set_range(chart_sensor, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_obj_set_style_bg_color(chart_sensor, lv_color_hex(C_BG), 0);
    lv_obj_set_style_border_color(chart_sensor, lv_color_hex(C_BORDER), 0);
    lv_obj_set_style_border_width(chart_sensor, 1, 0);
    lv_obj_set_style_size(chart_sensor, 0, 0, LV_PART_INDICATOR);

    chart_series = lv_chart_add_series(chart_sensor,
                                        lv_color_hex(C_ACCENT),
                                        LV_CHART_AXIS_PRIMARY_Y);
    uint16_t seed[] = {25,27,26,28,30,32,29,31,33,35,34,32,30,28,26,27,29,31,30,28};
    for (int i = 0; i < 20; i++)
        lv_chart_set_next_value(chart_sensor, chart_series, seed[i]);

    /* 4 sensor value cards in a single row */
    typedef struct { const char *icon; const char *name; const char *val;
                     uint32_t col; } SCard;
    static const SCard scards[4] = {
        { LV_SYMBOL_SHUFFLE,  "ACCEL",  "1.2 g",   0xD29922 },
        { LV_SYMBOL_LOOP,     "GYRO",   "45°/s",   0x3FB950 },
        { LV_SYMBOL_WARNING,  "TEMP",   "25°C",    0xF85149 },
        { LV_SYMBOL_DOWNLOAD, "PRES",   "1013",    0x58A6FF },
    };

    for (int i = 0; i < 4; i++) {
        lv_obj_t *sc = make_card(scr_sensors, 4 + i * 78, 248, 74, 82);

        /* Coloured top-border accent */
        lv_obj_set_style_border_color(sc, lv_color_hex(scards[i].col), 0);
        lv_obj_set_style_border_width(sc, 2, 0);
        lv_obj_set_style_border_side(sc, LV_BORDER_SIDE_TOP, 0);

        /* Icon */
        lv_obj_t *icon = make_label(sc, scards[i].icon,
                                     &lv_font_montserrat_16, scards[i].col);
        lv_obj_align(icon, LV_ALIGN_TOP_MID, 0, 2);

        /* Name */
        lv_obj_t *nm = make_label(sc, scards[i].name,
                                   &lv_font_montserrat_14, C_DIM);
        lv_obj_align(nm, LV_ALIGN_TOP_MID, 0, 24);

        /* Value */
        lv_obj_t *vl = make_label(sc, scards[i].val,
                                   &lv_font_montserrat_14, C_TXT);
        lv_obj_align(vl, LV_ALIGN_BOTTOM_MID, 0, -4);
    }

    attach_tab_bar(scr_sensors, 1);

    /* Keep btn_sensors_back = NULL (tab bar used for navigation) */
    btn_sensors_back = NULL;
}

/*=============================================================================
 * SCREEN 3 — CONFIG
 *
 * Layout:
 *    0 – 44   Header
 *   50 – 130  Brightness row (icon + label + slider)
 *  136 – 216  Volume row    (icon + label + slider)
 *  222 – 262  WiFi toggle row
 *  268 – 308  Bluetooth toggle row
 *  314 – 354  System Info button
 *  436 – 480  Bottom tab bar
 *===========================================================================*/
static void create_config_screen(void)
{
    scr_config = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_config, lv_color_hex(C_BG), 0);
    lv_obj_clear_flag(scr_config, LV_OBJ_FLAG_SCROLLABLE);

    make_header(scr_config, LV_SYMBOL_SETTINGS, "Configuration", false);

    /* Helper macro for a row separator line */
    #define ROW_DIV(parent, yy)                                             \
    do {                                                                    \
        lv_obj_t *_d = lv_obj_create(parent);                              \
        lv_obj_set_size(_d, 300, 1);                                        \
        lv_obj_set_pos(_d, 10, yy);                                         \
        lv_obj_set_style_bg_color(_d, lv_color_hex(C_BORDER), 0);          \
        lv_obj_set_style_border_width(_d, 0, 0);                           \
    } while (0)

    /* --- Brightness -------------------------------------------------------- */
    lv_obj_t *bri_icon = make_label(scr_config,
        LV_SYMBOL_IMAGE "  Brightness",
        &lv_font_montserrat_14, C_TXT);
    lv_obj_set_pos(bri_icon, 14, 56);

    lv_obj_t *bri_val = make_label(scr_config, "70%",
                                    &lv_font_montserrat_14, C_ACCENT);
    lv_obj_align(bri_val, LV_ALIGN_TOP_RIGHT, -14, 56);

    lv_obj_t *slider_bri = lv_slider_create(scr_config);
    lv_obj_set_size(slider_bri, 290, 12);
    lv_obj_set_pos(slider_bri, 14, 80);
    lv_slider_set_value(slider_bri, 70, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider_bri, lv_color_hex(C_ACCENT), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider_bri, lv_color_hex(C_ACCENT), LV_PART_KNOB);

    ROW_DIV(scr_config, 102);

    /* --- Volume ------------------------------------------------------------ */
    lv_obj_t *vol_icon = make_label(scr_config,
        LV_SYMBOL_VOLUME_MAX "  Volume",
        &lv_font_montserrat_14, C_TXT);
    lv_obj_set_pos(vol_icon, 14, 112);

    lv_obj_t *vol_val = make_label(scr_config, "50%",
                                    &lv_font_montserrat_14, C_ACCENT);
    lv_obj_align(vol_val, LV_ALIGN_TOP_RIGHT, -14, 112);

    lv_obj_t *slider_vol = lv_slider_create(scr_config);
    lv_obj_set_size(slider_vol, 290, 12);
    lv_obj_set_pos(slider_vol, 14, 136);
    lv_slider_set_value(slider_vol, 50, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider_vol, lv_color_hex(C_OK), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider_vol, lv_color_hex(C_OK), LV_PART_KNOB);

    ROW_DIV(scr_config, 160);

    /* --- WiFi toggle ------------------------------------------------------- */
    lv_obj_t *wifi_icon = make_label(scr_config,
        LV_SYMBOL_WIFI "  WiFi",
        &lv_font_montserrat_14, C_TXT);
    lv_obj_set_pos(wifi_icon, 14, 172);

    lv_obj_t *sw_wifi = lv_switch_create(scr_config);
    lv_obj_align(sw_wifi, LV_ALIGN_TOP_RIGHT, -14, 168);
    lv_obj_set_style_bg_color(sw_wifi, lv_color_hex(C_OK), LV_PART_INDICATOR);

    ROW_DIV(scr_config, 204);

    /* --- Bluetooth toggle -------------------------------------------------- */
    lv_obj_t *bt_icon = make_label(scr_config,
        LV_SYMBOL_BLUETOOTH "  Bluetooth",
        &lv_font_montserrat_14, C_TXT);
    lv_obj_set_pos(bt_icon, 14, 216);

    lv_obj_t *sw_bt = lv_switch_create(scr_config);
    lv_obj_align(sw_bt, LV_ALIGN_TOP_RIGHT, -14, 212);
    lv_obj_add_state(sw_bt, LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(sw_bt, lv_color_hex(C_ACCENT), LV_PART_INDICATOR);

    ROW_DIV(scr_config, 248);

    /* --- System info button ------------------------------------------------ */
    lv_obj_t *btn_info = lv_btn_create(scr_config);
    lv_obj_set_size(btn_info, 290, 40);
    lv_obj_set_pos(btn_info, 14, 260);
    lv_obj_set_style_bg_color(btn_info, lv_color_hex(0x21262D), 0);
    lv_obj_set_style_border_color(btn_info, lv_color_hex(C_BORDER), 0);
    lv_obj_set_style_border_width(btn_info, 1, 0);
    lv_obj_set_style_radius(btn_info, 8, 0);
    lv_obj_set_style_shadow_width(btn_info, 0, 0);
    lv_obj_set_style_pad_all(btn_info, 0, 0);
    btn_config_info = btn_info;

    lv_obj_t *lbl_info = lv_label_create(btn_info);
    lv_label_set_text(lbl_info, LV_SYMBOL_CALL "  System Information");
    lv_obj_set_style_text_color(lbl_info, lv_color_hex(C_TXT), 0);
    lv_obj_center(lbl_info);

    /* --- Pins utility button ----------------------------------------------- */
    lv_obj_t *btn_pins = lv_btn_create(scr_config);
    lv_obj_set_size(btn_pins, 290, 40);
    lv_obj_set_pos(btn_pins, 14, 310);
    lv_obj_set_style_bg_color(btn_pins, lv_color_hex(0x21262D), 0);
    lv_obj_set_style_border_color(btn_pins, lv_color_hex(C_BORDER), 0);
    lv_obj_set_style_border_width(btn_pins, 1, 0);
    lv_obj_set_style_radius(btn_pins, 8, 0);
    lv_obj_set_style_shadow_width(btn_pins, 0, 0);
    lv_obj_set_style_pad_all(btn_pins, 0, 0);
    btn_home_pins = btn_pins;

    lv_obj_t *lbl_pins = lv_label_create(btn_pins);
    lv_label_set_text(lbl_pins, LV_SYMBOL_LIST "  GPIO Pin Inspector");
    lv_obj_set_style_text_color(lbl_pins, lv_color_hex(C_TXT), 0);
    lv_obj_center(lbl_pins);

    #undef ROW_DIV

    /* Suppress unused-variable warnings for sliders with no callbacks yet */
    (void)bri_val; (void)vol_val;
    (void)slider_bri; (void)slider_vol;
    (void)sw_wifi; (void)sw_bt;

    btn_config_back = NULL;
    attach_tab_bar(scr_config, 2);
}

/*=============================================================================
 * SCREEN 4 — INFO
 *
 * Layout:
 *    0 – 44   Header
 *   50 – 280  MCU info card
 *  286 – 320  Uptime label
 *  436 – 480  Bottom tab bar
 *===========================================================================*/
static void create_info_screen(void)
{
    scr_info = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_info, lv_color_hex(C_BG), 0);
    lv_obj_clear_flag(scr_info, LV_OBJ_FLAG_SCROLLABLE);

    make_header(scr_info, LV_SYMBOL_CALL, "System Info", false);

    /* Info card */
    lv_obj_t *card = make_card(scr_info, 10, 52, 300, 240);

    lv_obj_t *info_txt = make_label(card,
        LV_SYMBOL_CHARGE "  MCU\n"
        "  Device   :  STM32F429ZI\n"
        "  Core     :  ARM Cortex-M4F\n"
        "  Clock    :  180 MHz\n"
        "  Flash    :  2 048 KB\n"
        "  SRAM     :  256 KB\n"
        "\n"
        LV_SYMBOL_IMAGE "  DISPLAY\n"
        "  Panel    :  ILI9341  (LTDC)\n"
        "  Size     :  320 x 480 px\n"
        "\n"
        LV_SYMBOL_DRIVE "  FIRMWARE\n"
        "  LVGL     :  v9.x\n"
        "  Build    :  " __DATE__,
        &lv_font_montserrat_14, C_TXT);
    lv_obj_set_pos(info_txt, 8, 6);

    /* Uptime label */
    info_uptime_lbl = make_label(scr_info,
        LV_SYMBOL_REFRESH "  Uptime: 00:00:00",
        &lv_font_montserrat_14, C_DIM);
    lv_obj_set_pos(info_uptime_lbl, 14, 300);

    btn_info_back = NULL;
    attach_tab_bar(scr_info, 3);
}

/*=============================================================================
 * SCREEN 5 — PINS LIST
 *===========================================================================*/
static void create_pins_screen(void)
{
    scr_pins = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_pins, lv_color_hex(C_BG), 0);
    lv_obj_clear_flag(scr_pins, LV_OBJ_FLAG_SCROLLABLE);

    make_header(scr_pins, LV_SYMBOL_LIST, "GPIO Pins", false);

    lv_obj_t *list = lv_list_create(scr_pins);
    lv_obj_set_size(list, 312, 388);
    lv_obj_set_pos(list, 4, 50);
    lv_obj_set_style_bg_color(list, lv_color_hex(C_PANEL), 0);
    lv_obj_set_style_border_color(list, lv_color_hex(C_BORDER), 0);
    lv_obj_set_style_border_width(list, 1, 0);
    lv_obj_set_style_text_font(list, &lv_font_montserrat_14, LV_PART_MAIN);

    for (uint32_t i = 0; i < (sizeof(pin_table) / sizeof(pin_table[0])); i++) {
        lv_obj_t *btn = lv_list_add_btn(list, LV_SYMBOL_EDIT,
                                          pin_table[i].name);
        lv_obj_set_style_text_color(btn, lv_color_hex(C_TXT), LV_PART_MAIN);
        lv_obj_set_style_bg_color(btn, lv_color_hex(C_PANEL), 0);
        lv_obj_add_event_cb(btn, pin_item_event_handler,
                             LV_EVENT_CLICKED, (void *)&pin_table[i]);
    }

    /* Back button */
    lv_obj_t *btn_back = lv_btn_create(scr_pins);
    lv_obj_set_size(btn_back, 80, 34);
    lv_obj_align(btn_back, LV_ALIGN_BOTTOM_MID, 0, -6);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x21262D), 0);
    lv_obj_set_style_border_color(btn_back, lv_color_hex(C_BORDER), 0);
    lv_obj_set_style_border_width(btn_back, 1, 0);
    lv_obj_set_style_radius(btn_back, 8, 0);
    lv_obj_set_style_shadow_width(btn_back, 0, 0);
    lv_obj_set_style_pad_all(btn_back, 0, 0);
    btn_pins_back = btn_back;

    lv_obj_t *lbl = lv_label_create(btn_back);
    lv_label_set_text(lbl, LV_SYMBOL_LEFT "  Back");
    lv_obj_set_style_text_color(lbl, lv_color_hex(C_TXT), 0);
    lv_obj_center(lbl);
}

/*=============================================================================
 * SCREEN 6 — PIN DETAIL
 *===========================================================================*/
static void create_pin_detail_screen(void)
{
    scr_pin_detail = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_pin_detail, lv_color_hex(C_BG), 0);
    lv_obj_clear_flag(scr_pin_detail, LV_OBJ_FLAG_SCROLLABLE);

    make_header(scr_pin_detail, LV_SYMBOL_EDIT, "Pin Detail", false);

    /* Detail card */
    lv_obj_t *card = make_card(scr_pin_detail, 10, 56, 300, 170);

    pin_name_label   = make_label(card, "Pin:     --",
                                   &lv_font_montserrat_14, C_TXT);
    lv_obj_set_pos(pin_name_label, 10, 8);

    pin_port_label   = make_label(card, "Port:    --",
                                   &lv_font_montserrat_14, C_TXT);
    lv_obj_set_pos(pin_port_label, 10, 36);

    pin_nature_label = make_label(card, "Function:  --",
                                   &lv_font_montserrat_14, C_TXT);
    lv_obj_set_pos(pin_nature_label, 10, 64);

    pin_state_label  = make_label(card, LV_SYMBOL_CHARGE "  State:   --",
                                   &lv_font_montserrat_14, C_ACCENT);
    lv_obj_set_pos(pin_state_label, 10, 100);

    lv_obj_t *poll_note = make_label(card, "(polled every 500 ms)",
                                      &lv_font_montserrat_14, C_DIM);
    lv_obj_set_pos(poll_note, 10, 130);

    /* Back button */
    lv_obj_t *btn_back = lv_btn_create(scr_pin_detail);
    lv_obj_set_size(btn_back, 110, 36);
    lv_obj_align(btn_back, LV_ALIGN_BOTTOM_MID, 0, -52);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x21262D), 0);
    lv_obj_set_style_border_color(btn_back, lv_color_hex(C_BORDER), 0);
    lv_obj_set_style_border_width(btn_back, 1, 0);
    lv_obj_set_style_radius(btn_back, 8, 0);
    lv_obj_set_style_shadow_width(btn_back, 0, 0);
    lv_obj_set_style_pad_all(btn_back, 0, 0);
    btn_pin_detail_back = btn_back;

    lv_obj_t *lbl = lv_label_create(btn_back);
    lv_label_set_text(lbl, LV_SYMBOL_LEFT "  Pin List");
    lv_obj_set_style_text_color(lbl, lv_color_hex(C_TXT), 0);
    lv_obj_center(lbl);

    /* Pin-state poll timer (paused until a pin is selected) */
    pin_detail_timer = lv_timer_create(pin_detail_timer_cb, 500, NULL);
    lv_timer_pause(pin_detail_timer);

    lv_obj_add_event_cb(btn_back, pin_detail_back_cb, LV_EVENT_CLICKED, NULL);
}

/*=============================================================================
 * EVENT CALLBACKS
 *===========================================================================*/

/* Clock timer: called every 1 s, updates time/date labels and uptime */
static uint32_t app_seconds = 0;

static void clock_timer_cb(lv_timer_t *t)
{
    (void)t;
    app_seconds++;
    log_seconds = app_seconds;

    /* Time label */
    if (hdr_time_lbl) {
        char buf[32];
        uint32_t h = app_seconds / 3600;
        uint32_t m = (app_seconds % 3600) / 60;
        uint32_t s = app_seconds % 60;
        lv_snprintf(buf, sizeof(buf), LV_SYMBOL_CHARGE " %02lu:%02lu:%02lu",
                    (unsigned long)h, (unsigned long)m, (unsigned long)s);
        lv_label_set_text(hdr_time_lbl, buf);
    }

    /* Date label — derive a simple day counter from elapsed seconds */
    if (hdr_date_lbl) {
        char buf[32];
        static const char *months[] = {
            "Jan","Feb","Mar","Apr","May","Jun",
            "Jul","Aug","Sep","Oct","Nov","Dec"
        };
        /* Approximate calendar from build date offset.
           For real date use the STM32 RTC peripheral. */
        uint32_t day = 22 + (app_seconds / 86400);  /* starts March 22 2026 */
        lv_snprintf(buf, sizeof(buf), LV_SYMBOL_DRIVE " %s %02lu 2026",
                    months[2],           /* March */
                    (unsigned long)day);
        lv_label_set_text(hdr_date_lbl, buf);
    }

    /* Info screen uptime */
    if (info_uptime_lbl) {
        char buf[40];
        uint32_t h = app_seconds / 3600;
        uint32_t m = (app_seconds % 3600) / 60;
        uint32_t s = app_seconds % 60;
        lv_snprintf(buf, sizeof(buf),
                    LV_SYMBOL_REFRESH "  Uptime: %02lu:%02lu:%02lu",
                    (unsigned long)h, (unsigned long)m, (unsigned long)s);
        lv_label_set_text(info_uptime_lbl, buf);
    }
}

/* Uptime dummy — clock_timer_cb does everything */
static void uptime_timer_cb(lv_timer_t *t) { (void)t; }

/* Pin-detail poll timer */
static void pin_detail_timer_cb(lv_timer_t *t)
{
    (void)t;
    const PinInfo *info = (const PinInfo *)lv_timer_get_user_data(pin_detail_timer);
    if (!info) return;
    GPIO_PinState st = HAL_GPIO_ReadPin(info->port, info->pin);
    char buf[40];
    lv_snprintf(buf, sizeof(buf), "%s  State:   %s",
                st == GPIO_PIN_SET ? LV_SYMBOL_OK : LV_SYMBOL_CLOSE,
                st == GPIO_PIN_SET ? "HIGH" : "LOW");
    lv_label_set_text(pin_state_label, buf);
    lv_obj_set_style_text_color(pin_state_label,
        lv_color_hex(st == GPIO_PIN_SET ? C_OK : C_FAULT), 0);
}

static void pin_detail_back_cb(lv_event_t *e)
{
    (void)e;
    if (pin_detail_timer) {
        lv_timer_pause(pin_detail_timer);
        lv_timer_set_user_data(pin_detail_timer, NULL);
    }
    current_pin_info = NULL;
}

static void pin_item_event_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    const PinInfo *info = (const PinInfo *)lv_event_get_user_data(e);
    if (!info) return;

    uint8_t     idx    = pin_index(info->pin);
    const char *pname  = pin_port_name(info->port);
    const char *nature = pin_nature_from_name(info->name);
    GPIO_PinState st   = HAL_GPIO_ReadPin(info->port, info->pin);

    char buf[64];
    lv_snprintf(buf, sizeof(buf), "Pin:     %s", info->name);
    lv_label_set_text(pin_name_label, buf);

    lv_snprintf(buf, sizeof(buf), "Port:    %s / Pin %u", pname, idx);
    lv_label_set_text(pin_port_label, buf);

    lv_snprintf(buf, sizeof(buf), "Function:  %s", nature);
    lv_label_set_text(pin_nature_label, buf);

    lv_snprintf(buf, sizeof(buf), "%s  State:   %s",
                st == GPIO_PIN_SET ? LV_SYMBOL_OK : LV_SYMBOL_CLOSE,
                st == GPIO_PIN_SET ? "HIGH" : "LOW");
    lv_label_set_text(pin_state_label, buf);
    lv_obj_set_style_text_color(pin_state_label,
        lv_color_hex(st == GPIO_PIN_SET ? C_OK : C_FAULT), 0);

    current_pin_info = info;
    if (pin_detail_timer) {
        lv_timer_set_user_data(pin_detail_timer, (void *)info);
        lv_timer_resume(pin_detail_timer);
    }
    lv_screen_load_anim(scr_pin_detail, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, false);
}

/*=============================================================================
 * PIN TABLE HELPERS (kept from original)
 *===========================================================================*/
static const char *pin_port_name(GPIO_TypeDef *port)
{
    if (port == GPIOA) return "GPIOA";
    if (port == GPIOB) return "GPIOB";
    if (port == GPIOC) return "GPIOC";
    if (port == GPIOD) return "GPIOD";
    if (port == GPIOE) return "GPIOE";
    if (port == GPIOF) return "GPIOF";
    if (port == GPIOG) return "GPIOG";
    if (port == GPIOH) return "GPIOH";
    return "GPIO?";
}

static uint8_t pin_index(uint16_t pin)
{
    for (uint8_t i = 0; i < 16; i++)
        if (pin == (uint16_t)(1U << i)) return i;
    return 0xFF;
}

static const char *pin_nature_from_name(const char *name)
{
    if (strstr(name, "OSC"))   return "Oscillator";
    if (strstr(name, "SPI4"))  return "SPI4 (AF)";
    if (strstr(name, "I2C3"))  return "I2C3 (AF)";
    if (strstr(name, "STLINK"))return "ST-LINK UART";
    if (strstr(name, "SWD"))   return "SWD";
    if (strstr(name, "OTG") || strstr(name, "VBUS")) return "USB OTG";
    if (strstr(name, "LD"))    return "LED";
    if (strstr(name, "MEMS"))  return "MEMS Sensor";
    if (strstr(name, "TP_INT"))return "Touch Interrupt";
    if (strstr(name, "CSX") || strstr(name, "WRX") ||
        strstr(name, "RDX") || strstr(name, "TE"))   return "LCD Control";
    if (!strcmp(name,"VSYNC") || !strcmp(name,"HSYNC") ||
        !strcmp(name,"DOTCLK") ||
        name[0]=='R' || name[0]=='G' || name[0]=='B') return "LTDC RGB";
    if (strstr(name,"SDN") || strstr(name,"SDC") || strstr(name,"NBL") ||
        name[0]=='A' || name[0]=='D' || strstr(name,"BA"))
        return "FMC / SDRAM";
    if (!strcmp(name,"BOOT1"))  return "Boot Select";
    if (!strcmp(name,"B1"))     return "User Button";
    return "GPIO";
}

/*=============================================================================
 * PUBLIC API
 *===========================================================================*/

/*
 * LVGL_App_Init
 * Called once at startup from main.c, after HAL_Init() / SystemClock_Config().
 */
void LVGL_App_Init(void)
{
    log_debug("LVGL: initialising GUI");

    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();

    /* Build all screens */
    create_sensors_screen();   /* must exist before tab_screens[] is filled */
    create_config_screen();
    create_info_screen();
    create_pins_screen();
    create_pin_detail_screen();
    create_home_screen();      /* home last so its tab-bar sees the others  */

    /* Wire tab-bar navigation */
    tab_screens[0] = scr_home;
    tab_screens[1] = scr_sensors;
    tab_screens[2] = scr_config;
    tab_screens[3] = scr_info;

    /* Wire remaining navigation callbacks */
    if (btn_config_info)
        lv_obj_add_event_cb(btn_config_info,    nav_event_handler, LV_EVENT_CLICKED, scr_info);
    if (btn_home_pins)
        lv_obj_add_event_cb(btn_home_pins,      nav_event_handler, LV_EVENT_CLICKED, scr_pins);
    if (btn_pins_back)
        lv_obj_add_event_cb(btn_pins_back,      nav_event_handler, LV_EVENT_CLICKED, scr_config);
    if (btn_pin_detail_back)
        lv_obj_add_event_cb(btn_pin_detail_back,nav_event_handler, LV_EVENT_CLICKED, scr_pins);

    /* Background timers */
    lv_timer_create(clock_timer_cb,  1000, NULL);   /* 1 s clock + uptime  */
    lv_timer_create(uptime_timer_cb, 1000, NULL);   /* placeholder         */

    /* Show home screen */
    lv_screen_load(scr_home);
    lv_refr_now(NULL);

    log_debug("LVGL: GUI ready");
}

/*
 * LVGL_App_Tick
 * Called every 1-5 ms from the main loop.
 */
void LVGL_App_Tick(void)
{
    lv_timer_handler();
}

/*
 * LVGL_App_UpdateTemperature
 * Pushes a new temperature reading (degrees C).
 * Updates tile value, health dot colour, and appends a log entry if high.
 */
void LVGL_App_UpdateTemperature(int temp_celsius)
{
    if (tile_temp_val) {
        char buf[8];
        lv_snprintf(buf, sizeof(buf), "%d", temp_celsius);
        lv_label_set_text(tile_temp_val, buf);
    }
    if (tile_temp_dot) {
        uint32_t c = (temp_celsius >= 38) ? C_FAULT :
                     (temp_celsius >= 32) ? C_WARN  : C_OK;
        lv_obj_set_style_bg_color(tile_temp_dot, lv_color_hex(c), 0);
        if (temp_celsius >= 38) log_append(LV_SYMBOL_WARNING, "Temp FAULT");
        else if (temp_celsius >= 32) log_append(LV_SYMBOL_WARNING, "Temp WARN");
    }
    /* Push to chart on sensors screen */
    if (chart_sensor && chart_series)
        lv_chart_set_next_value(chart_sensor, chart_series,
                                (lv_coord_t)temp_celsius);
}

/*
 * LVGL_App_UpdateHumidity
 * Pushes a new humidity reading (% RH).
 */
void LVGL_App_UpdateHumidity(int humidity_percent)
{
    if (tile_hum_val) {
        char buf[8];
        lv_snprintf(buf, sizeof(buf), "%d", humidity_percent);
        lv_label_set_text(tile_hum_val, buf);
    }
    if (tile_hum_dot) {
        uint32_t c = (humidity_percent >= 85) ? C_FAULT :
                     (humidity_percent >= 70) ? C_WARN  : C_OK;
        lv_obj_set_style_bg_color(tile_hum_dot, lv_color_hex(c), 0);
    }
}

/*
 * LVGL_App_UpdateStatus
 * Updates the status strip on the Home screen and appends a log entry.
 */
void LVGL_App_UpdateStatus(const char *status)
{
    if (status_label && status) {
        char buf[64];
        lv_snprintf(buf, sizeof(buf), LV_SYMBOL_OK "  %s", status);
        lv_label_set_text(status_label, buf);
        lv_obj_invalidate(status_label);
    }
    if (status) log_append(LV_SYMBOL_OK, status);
}

/*
 * LVGL_App_AddChartData
 * Feeds the sensors-screen trend chart and derives values for
 * the PRESSURE and ACCEL tiles.
 */
void LVGL_App_AddChartData(int value)
{
    if (chart_sensor && chart_series)
        lv_chart_set_next_value(chart_sensor, chart_series, (lv_coord_t)value);

    if (tile_pres_val) {
        char buf[8];
        lv_snprintf(buf, sizeof(buf), "%d", 1000 + (value % 60));
        lv_label_set_text(tile_pres_val, buf);
    }
    if (tile_acc_val) {
        char buf[8];
        lv_snprintf(buf, sizeof(buf), "%.2f", (float)value * 0.04f);
        lv_label_set_text(tile_acc_val, buf);
    }
}

