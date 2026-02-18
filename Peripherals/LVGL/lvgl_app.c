/*******************************************************************************
 * LVGL Application - Professional Multi-Screen GUI
 *******************************************************************************
 * This file creates a complete 4-screen GUI interface for STM32F429I-DISC1.
 *
 * 📱 Screens Included:
 * 1. Home Dashboard - Temperature gauge, humidity bar, status
 * 2. Sensor Monitor - Live chart with 4 sensor cards
 * 3. Settings - Sliders and toggle switches
 * 4. System Info - Hardware specifications
 *
 * 🎯 For Beginners:
 * - See BEGINNER_GUIDE.md for step-by-step learning
 * - The code follows a simple pattern: Create → Configure → Position
 * - Start by reading create_home_screen() function
 *
 * 💡 Simple Example (What a minimal version looks like):
 *
 *   void LVGL_App_Init(void) {
 *       lv_init();                              // Start LVGL
 *       lv_port_disp_init();                    // Connect display
 *
 *       lv_obj_t *label = lv_label_create(lv_screen_active());   // Create label
 *       lv_label_set_text(label, "Hello!");                // Set text
 *       lv_obj_center(label);                              // Center it
 *   }
 *
 * That's it! This code just does that concept 4 times (once per screen).
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
#include "../LOG/log.h"

/*-----------------------------------------------------------------------------
 * Global Variables & Forward Declarations
 *---------------------------------------------------------------------------*/
static lv_obj_t *scr_home;      /* Home dashboard screen */
static lv_obj_t *scr_sensors;   /* Sensor monitoring screen */
static lv_obj_t *scr_settings;  /* Settings screen */
static lv_obj_t *scr_info;      /* System info screen */
static lv_obj_t *scr_pins;      /* Pins list screen */
static lv_obj_t *scr_pin_detail;/* Pin detail screen */

/* UI Elements */
static lv_obj_t *status_label;
static lv_obj_t *temp_arc;       /* Temperature gauge */
static lv_obj_t *temp_label;     /* Temperature label */
static lv_obj_t *humidity_bar;   /* Humidity bar */
static lv_obj_t *humidity_label; /* Humidity label */
static lv_obj_t *chart_sensor;   /* Sensor data chart */
static lv_chart_series_t *chart_series; /* Chart data series */

/* Navigation button handles (registered after screens created) */
static lv_obj_t *btn_home_sensors;
static lv_obj_t *btn_home_settings;
static lv_obj_t *btn_sensors_back;
static lv_obj_t *btn_settings_info;
static lv_obj_t *btn_settings_back;
static lv_obj_t *btn_info_back;
static lv_obj_t *btn_home_pins;
static lv_obj_t *btn_pins_back;
static lv_obj_t *btn_pin_detail_back;

static lv_obj_t *pin_name_label;
static lv_obj_t *pin_port_label;
static lv_obj_t *pin_nature_label;
static lv_obj_t *pin_state_label;

/* Timer callback and back-button helper */
static void pin_detail_timer_cb(lv_timer_t *timer);
static void pin_detail_back_cb(lv_event_t *e);

/* Forward declarations */
static void create_home_screen(void);
static void create_sensor_screen(void);
static void create_settings_screen(void);
static void create_info_screen(void);
static void create_pins_screen(void);
static void create_pin_detail_screen(void);
static void nav_event_handler(lv_event_t *e);
static void pin_item_event_handler(lv_event_t *e);

typedef struct {
    const char *name;
    GPIO_TypeDef *port;
    uint16_t pin;
} PinInfo;

/* Auto-refresh timer for the Pin Detail screen (declared after PinInfo so type is known) */
static const PinInfo *current_pin_info = NULL; /* pointer to currently displayed pin */
static lv_timer_t *pin_detail_timer = NULL;

#define PIN_ENTRY(name, port, pin) {name, port, pin}

static const PinInfo pin_table[] = {
    PIN_ENTRY("PC14_OSC32_IN", PC14_OSC32_IN_GPIO_Port, PC14_OSC32_IN_Pin),
    PIN_ENTRY("PC15_OSC32_OUT", PC15_OSC32_OUT_GPIO_Port, PC15_OSC32_OUT_Pin),
    PIN_ENTRY("A0", A0_GPIO_Port, A0_Pin),
    PIN_ENTRY("A1", A1_GPIO_Port, A1_Pin),
    PIN_ENTRY("A2", A2_GPIO_Port, A2_Pin),
    PIN_ENTRY("A3", A3_GPIO_Port, A3_Pin),
    PIN_ENTRY("A4", A4_GPIO_Port, A4_Pin),
    PIN_ENTRY("A5", A5_GPIO_Port, A5_Pin),
    PIN_ENTRY("SPI4_SCK", SPI4_SCK_GPIO_Port, SPI4_SCK_Pin),
    PIN_ENTRY("SPI4_MISO", SPI4_MISO_GPIO_Port, SPI4_MISO_Pin),
    PIN_ENTRY("SPI4_MOSI", SPI4_MOSI_GPIO_Port, SPI4_MOSI_Pin),
    PIN_ENTRY("ENABLE", ENABLE_GPIO_Port, ENABLE_Pin),
    PIN_ENTRY("PH0_OSC_IN", PH0_OSC_IN_GPIO_Port, PH0_OSC_IN_Pin),
    PIN_ENTRY("PH1_OSC_OUT", PH1_OSC_OUT_GPIO_Port, PH1_OSC_OUT_Pin),
    PIN_ENTRY("SDNWE", SDNWE_GPIO_Port, SDNWE_Pin),
    PIN_ENTRY("NCS_MEMS_SPI", NCS_MEMS_SPI_GPIO_Port, NCS_MEMS_SPI_Pin),
    PIN_ENTRY("CSX", CSX_GPIO_Port, CSX_Pin),
    PIN_ENTRY("B1", B1_GPIO_Port, B1_Pin),
    PIN_ENTRY("MEMS_INT1", MEMS_INT1_GPIO_Port, MEMS_INT1_Pin),
    PIN_ENTRY("MEMS_INT2", MEMS_INT2_GPIO_Port, MEMS_INT2_Pin),
    PIN_ENTRY("B5", B5_GPIO_Port, B5_Pin),
    PIN_ENTRY("VSYNC", VSYNC_GPIO_Port, VSYNC_Pin),
    PIN_ENTRY("G2", G2_GPIO_Port, G2_Pin),
    PIN_ENTRY("ACP_RST", ACP_RST_GPIO_Port, ACP_RST_Pin),
    PIN_ENTRY("OTG_FS_PSO", OTG_FS_PSO_GPIO_Port, OTG_FS_PSO_Pin),
    PIN_ENTRY("OTG_FS_OC", OTG_FS_OC_GPIO_Port, OTG_FS_OC_Pin),
    PIN_ENTRY("R3", R3_GPIO_Port, R3_Pin),
    PIN_ENTRY("R6", R6_GPIO_Port, R6_Pin),
    PIN_ENTRY("BOOT1", BOOT1_GPIO_Port, BOOT1_Pin),
    PIN_ENTRY("SDNRAS", SDNRAS_GPIO_Port, SDNRAS_Pin),
    PIN_ENTRY("A6", A6_GPIO_Port, A6_Pin),
    PIN_ENTRY("A7", A7_GPIO_Port, A7_Pin),
    PIN_ENTRY("A8", A8_GPIO_Port, A8_Pin),
    PIN_ENTRY("A9", A9_GPIO_Port, A9_Pin),
    PIN_ENTRY("A10", A10_GPIO_Port, A10_Pin),
    PIN_ENTRY("A11", A11_GPIO_Port, A11_Pin),
    PIN_ENTRY("D4", D4_GPIO_Port, D4_Pin),
    PIN_ENTRY("D5", D5_GPIO_Port, D5_Pin),
    PIN_ENTRY("D6", D6_GPIO_Port, D6_Pin),
    PIN_ENTRY("D7", D7_GPIO_Port, D7_Pin),
    PIN_ENTRY("D8", D8_GPIO_Port, D8_Pin),
    PIN_ENTRY("D9", D9_GPIO_Port, D9_Pin),
    PIN_ENTRY("D10", D10_GPIO_Port, D10_Pin),
    PIN_ENTRY("D11", D11_GPIO_Port, D11_Pin),
    PIN_ENTRY("D12", D12_GPIO_Port, D12_Pin),
    PIN_ENTRY("G4", G4_GPIO_Port, G4_Pin),
    PIN_ENTRY("G5", G5_GPIO_Port, G5_Pin),
    PIN_ENTRY("OTG_HS_ID", OTG_HS_ID_GPIO_Port, OTG_HS_ID_Pin),
    PIN_ENTRY("VBUS_HS", VBUS_HS_GPIO_Port, VBUS_HS_Pin),
    PIN_ENTRY("OTG_HS_DM", OTG_HS_DM_GPIO_Port, OTG_HS_DM_Pin),
    PIN_ENTRY("OTG_HS_DP", OTG_HS_DP_GPIO_Port, OTG_HS_DP_Pin),
    PIN_ENTRY("D13", D13_GPIO_Port, D13_Pin),
    PIN_ENTRY("D14", D14_GPIO_Port, D14_Pin),
    PIN_ENTRY("D15", D15_GPIO_Port, D15_Pin),
    PIN_ENTRY("TE", TE_GPIO_Port, TE_Pin),
    PIN_ENTRY("RDX", RDX_GPIO_Port, RDX_Pin),
    PIN_ENTRY("WRX_DCX", WRX_DCX_GPIO_Port, WRX_DCX_Pin),
    PIN_ENTRY("D0", D0_GPIO_Port, D0_Pin),
    PIN_ENTRY("D1", D1_GPIO_Port, D1_Pin),
    PIN_ENTRY("BA0", BA0_GPIO_Port, BA0_Pin),
    PIN_ENTRY("BA1", BA1_GPIO_Port, BA1_Pin),
    PIN_ENTRY("R7", R7_GPIO_Port, R7_Pin),
    PIN_ENTRY("DOTCLK", DOTCLK_GPIO_Port, DOTCLK_Pin),
    PIN_ENTRY("SDCLK", SDCLK_GPIO_Port, SDCLK_Pin),
    PIN_ENTRY("HSYNC", HSYNC_GPIO_Port, HSYNC_Pin),
    PIN_ENTRY("G6", G6_GPIO_Port, G6_Pin),
    PIN_ENTRY("I2C3_SDA", I2C3_SDA_GPIO_Port, I2C3_SDA_Pin),
    PIN_ENTRY("I2C3_SCL", I2C3_SCL_GPIO_Port, I2C3_SCL_Pin),
    PIN_ENTRY("STLINK_RX", STLINK_RX_GPIO_Port, STLINK_RX_Pin),
    PIN_ENTRY("STLINK_TX", STLINK_TX_GPIO_Port, STLINK_TX_Pin),
    PIN_ENTRY("R4", R4_GPIO_Port, R4_Pin),
    PIN_ENTRY("R5", R5_GPIO_Port, R5_Pin),
    PIN_ENTRY("SWDIO", SWDIO_GPIO_Port, SWDIO_Pin),
    PIN_ENTRY("SWCLK", SWCLK_GPIO_Port, SWCLK_Pin),
    PIN_ENTRY("TP_INT1", TP_INT1_GPIO_Port, TP_INT1_Pin),
    PIN_ENTRY("R2", R2_GPIO_Port, R2_Pin),
    PIN_ENTRY("D2", D2_GPIO_Port, D2_Pin),
    PIN_ENTRY("D3", D3_GPIO_Port, D3_Pin),
    PIN_ENTRY("G7", G7_GPIO_Port, G7_Pin),
    PIN_ENTRY("B2", B2_GPIO_Port, B2_Pin),
    PIN_ENTRY("G3", G3_GPIO_Port, G3_Pin),
    PIN_ENTRY("B3", B3_GPIO_Port, B3_Pin),
    PIN_ENTRY("B4", B4_GPIO_Port, B4_Pin),
    PIN_ENTRY("LD3", LD3_GPIO_Port, LD3_Pin),
    PIN_ENTRY("LD4", LD4_GPIO_Port, LD4_Pin),
    PIN_ENTRY("SDNCAS", SDNCAS_GPIO_Port, SDNCAS_Pin),
    PIN_ENTRY("SDCKE1", SDCKE1_GPIO_Port, SDCKE1_Pin),
    PIN_ENTRY("SDNE1", SDNE1_GPIO_Port, SDNE1_Pin),
    PIN_ENTRY("B6", B6_GPIO_Port, B6_Pin),
    PIN_ENTRY("B7", B7_GPIO_Port, B7_Pin),
    PIN_ENTRY("NBL0", NBL0_GPIO_Port, NBL0_Pin),
    PIN_ENTRY("NBL1", NBL1_GPIO_Port, NBL1_Pin)
};

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
    for (uint8_t i = 0; i < 16; i++) {
        if (pin == (uint16_t)(1U << i)) return i;
    }
    return 0xFF;
}

static const char *pin_nature_from_name(const char *name)
{
    if (strstr(name, "OSC")) return "OSC";
    if (strstr(name, "SPI4")) return "SPI4 (AF)";
    if (strstr(name, "I2C3")) return "I2C3 (AF)";
    if (strstr(name, "STLINK")) return "ST-LINK UART";
    if (strstr(name, "SWD")) return "SWD";
    if (strstr(name, "OTG") || strstr(name, "VBUS")) return "USB OTG";
    if (strstr(name, "LD")) return "LED";
    if (strstr(name, "MEMS")) return "MEMS";
    if (strstr(name, "TP_INT")) return "Touch INT";
    if (strstr(name, "CSX") || strstr(name, "WRX") || strstr(name, "RDX") || strstr(name, "TE")) return "LCD CTRL";
    if (!strcmp(name, "VSYNC") || !strcmp(name, "HSYNC") || !strcmp(name, "DOTCLK") ||
        !strcmp(name, "R2") || !strcmp(name, "R3") || !strcmp(name, "R4") || !strcmp(name, "R5") || !strcmp(name, "R6") || !strcmp(name, "R7") ||
        !strcmp(name, "G2") || !strcmp(name, "G3") || !strcmp(name, "G4") || !strcmp(name, "G5") || !strcmp(name, "G6") || !strcmp(name, "G7") ||
        !strcmp(name, "B2") || !strcmp(name, "B3") || !strcmp(name, "B4") || !strcmp(name, "B6") || !strcmp(name, "B7")) {
        return "LTDC";
    }
    if (strstr(name, "SDN") || strstr(name, "SDC") || strstr(name, "SDNE") || strstr(name, "SDCK") || strstr(name, "NBL") || name[0] == 'A' || name[0] == 'D' || strstr(name, "BA")) return "FMC/SDRAM";
    if (!strcmp(name, "BOOT1")) return "BOOT";
    if (!strcmp(name, "B1")) return "User Button";
    return "GPIO";
}

/*-----------------------------------------------------------------------------
 * Navigation Event Handler
 *---------------------------------------------------------------------------*/
static void nav_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        void *user_data = lv_event_get_user_data(e);
        lv_obj_t *target_screen = (lv_obj_t *)user_data;
        /* Safety: ignore clicks with NULL target to avoid passing NULL to LVGL */
        if (target_screen == NULL) return;
        lv_screen_load_anim(target_screen,
                    LV_SCR_LOAD_ANIM_FADE_ON,
                    300,
                    0,
                    false);

    }
}

/*-----------------------------------------------------------------------------
 * Pin-detail helpers
 *---------------------------------------------------------------------------*/
static void pin_detail_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    const PinInfo *info = (const PinInfo *)lv_timer_get_user_data(pin_detail_timer);
    if (info == NULL) return;

    GPIO_PinState state = HAL_GPIO_ReadPin(info->port, info->pin);
    char buf[64];
    lv_snprintf(buf, sizeof(buf), "State: %s", state == GPIO_PIN_SET ? "HIGH" : "LOW");
    lv_label_set_text(pin_state_label, buf);
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
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_CLICKED) return;

    const PinInfo *info = (const PinInfo *)lv_event_get_user_data(e);
    if (info == NULL) return;

    uint8_t idx = pin_index(info->pin);
    const char *port_name = pin_port_name(info->port);
    const char *nature = pin_nature_from_name(info->name);
    GPIO_PinState state = HAL_GPIO_ReadPin(info->port, info->pin);

    char buf[64];
    lv_snprintf(buf, sizeof(buf), "Pin: %s", info->name);
    lv_label_set_text(pin_name_label, buf);

    lv_snprintf(buf, sizeof(buf), "Port: %s / %u", port_name, idx);
    lv_label_set_text(pin_port_label, buf);

    lv_snprintf(buf, sizeof(buf), "Nature: %s", nature);
    lv_label_set_text(pin_nature_label, buf);

    lv_snprintf(buf, sizeof(buf), "State: %s", state == GPIO_PIN_SET ? "HIGH" : "LOW");
    lv_label_set_text(pin_state_label, buf);

    /* Start auto-refresh for this pin detail (timer user_data set to the PinInfo) */
    current_pin_info = info;
    if (pin_detail_timer) {
        lv_timer_set_user_data(pin_detail_timer, (void *)info);
        lv_timer_resume(pin_detail_timer);
    }

    lv_screen_load_anim(scr_pin_detail, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, false);
}

/*=============================================================================
 * SCREEN CREATION FUNCTIONS
 *===========================================================================*/
/* Each function below creates one complete screen with all its widgets.
 *
 * Pattern Used (same for every widget):
 * 1. CREATE:    lv_widget_create(parent)
 * 2. CONFIGURE: lv_obj_set_size(), lv_obj_set_style_xxx()
 * 3. POSITION:  lv_obj_align()
 *
 * Read create_home_screen() first - it's the simplest!
 */

/*-----------------------------------------------------------------------------
 * Create Home Dashboard Screen
 *---------------------------------------------------------------------------*/
/* This is the main screen users see first.
 * Contains: Title, Status card, Temperature gauge, Humidity bar, 2 buttons
 */
static void create_home_screen(void)
{
    scr_home = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_home, lv_color_hex(0x1a1a2e), 0);

    /* Title Bar */
    lv_obj_t *title = lv_label_create(scr_home);
    lv_label_set_text(title, "STM32F429 Dashboard");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    /* Status Card - Moved lower to avoid overlap with title */
    lv_obj_t *status_card = lv_obj_create(scr_home);
    lv_obj_set_size(status_card, 150, 40);  /* Smaller size for testing */
    lv_obj_align(status_card, LV_ALIGN_TOP_MID, 0, 45);
    lv_obj_set_style_bg_color(status_card, lv_color_hex(0x00ff00), 0);  /* Green background for debugging */
    lv_obj_set_style_border_color(status_card, lv_color_hex(0x0f4c75), 0);
    lv_obj_set_style_border_width(status_card, 2, 0);

    status_label = lv_label_create(status_card);
    lv_label_set_text(status_label, "System Ready");
    lv_obj_set_style_text_color(status_label, lv_color_white(), 0);  /* White text for debugging */
    lv_obj_set_style_text_font(status_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_align(status_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(status_label, 10, 10);  /* Position at 10,10 within the card */

    /* Temperature Arc Gauge - Repositioned lower */
    temp_arc = lv_arc_create(scr_home);
    lv_obj_set_size(temp_arc, 80, 80);
    lv_obj_align(temp_arc, LV_ALIGN_TOP_LEFT, 20, 140);
    lv_arc_set_rotation(temp_arc, 135);
    lv_arc_set_bg_angles(temp_arc, 0, 270);
    lv_arc_set_value(temp_arc, 25);
    lv_obj_set_style_arc_color(temp_arc, lv_color_hex(0xff6b6b), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(temp_arc, 8, LV_PART_INDICATOR);

    temp_label = lv_label_create(scr_home);
    lv_label_set_text(temp_label, "25°C\nTemp");
    lv_obj_set_style_text_color(temp_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(temp_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_align(temp_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align_to(temp_label, temp_arc, LV_ALIGN_CENTER, 0, 0);

    /* Humidity Bar - Repositioned to avoid overlap */
    lv_obj_t *hum_container = lv_obj_create(scr_home);
    lv_obj_set_size(hum_container, 80, 80);
    lv_obj_align(hum_container, LV_ALIGN_TOP_RIGHT, -20, 130);
    lv_obj_set_style_bg_color(hum_container, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_border_width(hum_container, 0, 0);

    humidity_bar = lv_bar_create(hum_container);
    lv_obj_set_size(humidity_bar, 15, 60);
    lv_obj_center(humidity_bar);
    lv_bar_set_value(humidity_bar, 60, LV_ANIM_ON);
    lv_obj_set_style_bg_color(humidity_bar, lv_color_hex(0x4ecdc4), LV_PART_INDICATOR);

    humidity_label = lv_label_create(hum_container);
    lv_label_set_text(humidity_label, "60%\nHumid");
    lv_obj_set_style_text_color(humidity_label, lv_color_white(), 0);
    lv_obj_set_style_text_align(humidity_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(humidity_label, LV_ALIGN_BOTTOM_MID, 0, -5);

    /* Navigation Buttons - Repositioned lower */
    lv_obj_t *btn_sensors = lv_btn_create(scr_home);
    lv_obj_set_size(btn_sensors, 100, 40);
    lv_obj_align(btn_sensors, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    lv_obj_set_style_bg_color(btn_sensors, lv_color_hex(0x3be477), 0);
    /* Register callback after all screens are created */
    btn_home_sensors = btn_sensors;

    lv_obj_t *lbl_sensors = lv_label_create(btn_sensors);
    lv_label_set_text(lbl_sensors, LV_SYMBOL_LIST " Sensors");
    lv_obj_center(lbl_sensors);

    lv_obj_t *btn_settings = lv_btn_create(scr_home);
    lv_obj_set_size(btn_settings, 100, 40);
    lv_obj_align(btn_settings, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
    lv_obj_set_style_bg_color(btn_settings, lv_color_hex(0xf7b731), 0);
    /* Register callback after all screens are created */
    btn_home_settings = btn_settings;

    lv_obj_t *lbl_settings = lv_label_create(btn_settings);
    lv_label_set_text(lbl_settings, LV_SYMBOL_SETTINGS " Config");
    lv_obj_center(lbl_settings);

    lv_obj_t *btn_pins = lv_btn_create(scr_home);
    lv_obj_set_size(btn_pins, 90, 36);
    lv_obj_align(btn_pins, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_color(btn_pins, lv_color_hex(0x4ecdc4), 0);
    btn_home_pins = btn_pins;

    lv_obj_t *lbl_pins = lv_label_create(btn_pins);
    lv_label_set_text(lbl_pins, LV_SYMBOL_LIST " Pins");
    lv_obj_center(lbl_pins);
}

/* This screen shows live sensor data visualization.
 * Contains: Title, Line chart, 4 sensor value cards, Back button
 */
static void create_sensor_screen(void)
{
    scr_sensors = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_sensors, lv_color_hex(0x1a1a2e), 0);

    /* Title */
    lv_obj_t *title = lv_label_create(scr_sensors);
    lv_label_set_text(title, LV_SYMBOL_EYE_OPEN " Sensor Monitor");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    /* Chart */
    chart_sensor = lv_chart_create(scr_sensors);
    lv_obj_set_size(chart_sensor, 200, 120);
    lv_obj_align(chart_sensor, LV_ALIGN_CENTER, 0, -20);
    lv_chart_set_type(chart_sensor, LV_CHART_TYPE_LINE);
    lv_chart_set_range(chart_sensor, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_obj_set_style_bg_color(chart_sensor, lv_color_hex(0x16213e), 0);

    /* Add data series */
    chart_series = lv_chart_add_series(chart_sensor, lv_color_hex(0x3be477), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_point_count(chart_sensor, 10);
    uint16_t sample_data[] = {10, 20, 35, 45, 50, 55, 60, 70, 65, 50};
    for(int i = 0; i < 10; i++) {
        lv_chart_set_next_value(chart_sensor, chart_series, sample_data[i]);
    }

    /* Sensor Value Cards */
    const char *sensor_names[] = {"Accel", "Gyro", "Temp", "Press"};
    const char *sensor_values[] = {"1.2g", "45°/s", "25°C", "1013"};
    uint32_t colors[] = {0xff6b6b, 0x4ecdc4, 0xf7b731, 0xa29bfe};

    for(int i = 0; i < 4; i++) {
        lv_obj_t *card = lv_obj_create(scr_sensors);
        lv_obj_set_size(card, 50, 50);
        lv_obj_align(card, LV_ALIGN_BOTTOM_LEFT, 10 + (i * 55), -50);
        lv_obj_set_style_bg_color(card, lv_color_hex(colors[i]), 0);
        lv_obj_set_style_border_width(card, 0, 0);

        lv_obj_t *name = lv_label_create(card);
        lv_label_set_text(name, sensor_names[i]);
        lv_obj_set_style_text_font(name, &lv_font_montserrat_14, 0);
        lv_obj_align(name, LV_ALIGN_TOP_MID, 0, -6);

        lv_obj_t *value = lv_label_create(card);
        lv_label_set_text(value, sensor_values[i]);
        lv_obj_set_style_text_font(value, &lv_font_montserrat_14, 0);
        lv_obj_align(value, LV_ALIGN_BOTTOM_MID, 0, -0.5);
    }

    /* Back Button */
    lv_obj_t *btn_back = lv_btn_create(scr_sensors);
    lv_obj_set_size(btn_back, 60, 35);
    lv_obj_align(btn_back, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x5f6368), 0);
    /* Register callback after all screens are created */
    btn_sensors_back = btn_back;

    lv_obj_t *lbl_back = lv_label_create(btn_back);
    lv_label_set_text(lbl_back, LV_SYMBOL_LEFT " Back");
    lv_obj_center(lbl_back);
}

/*-----------------------------------------------------------------------------
 * Create Settings Screen
 *---------------------------------------------------------------------------*/
/* This screen allows user configuration.
 * Contains: Title, 2 sliders (brightness/volume), 2 switches (WiFi/BT),
 *           System Info button, Back button
 */
static void create_settings_screen(void)
{
    scr_settings = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_settings, lv_color_hex(0x1a1a2e), 0);

    /* Title */
    lv_obj_t *title = lv_label_create(scr_settings);
    lv_label_set_text(title, LV_SYMBOL_SETTINGS " Settings");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    /* Brightness Slider */
    lv_obj_t *bright_label = lv_label_create(scr_settings);
    lv_label_set_text(bright_label, "Brightness");
    lv_obj_set_style_text_color(bright_label, lv_color_white(), 0);
    lv_obj_align(bright_label, LV_ALIGN_TOP_LEFT, 20, 50);

    lv_obj_t *slider_bright = lv_slider_create(scr_settings);
    lv_obj_set_size(slider_bright, 180, 10);
    lv_obj_align(slider_bright, LV_ALIGN_TOP_LEFT, 20, 75);
    lv_slider_set_value(slider_bright, 70, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider_bright, lv_color_hex(0xf7b731), LV_PART_INDICATOR);

    /* Volume Slider */
    lv_obj_t *vol_label = lv_label_create(scr_settings);
    lv_label_set_text(vol_label, "Volume");
    lv_obj_set_style_text_color(vol_label, lv_color_white(), 0);
    lv_obj_align(vol_label, LV_ALIGN_TOP_LEFT, 20, 110);

    lv_obj_t *slider_vol = lv_slider_create(scr_settings);
    lv_obj_set_size(slider_vol, 180, 10);
    lv_obj_align(slider_vol, LV_ALIGN_TOP_LEFT, 20, 135);
    lv_slider_set_value(slider_vol, 50, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider_vol, lv_color_hex(0x4ecdc4), LV_PART_INDICATOR);

    /* Toggle Switches */
    lv_obj_t *wifi_label = lv_label_create(scr_settings);
    lv_label_set_text(wifi_label, "WiFi");
    lv_obj_set_style_text_color(wifi_label, lv_color_white(), 0);
    lv_obj_align(wifi_label, LV_ALIGN_TOP_LEFT, 20, 170);

    lv_obj_t *sw_wifi = lv_switch_create(scr_settings);
    lv_obj_align(sw_wifi, LV_ALIGN_TOP_RIGHT, -20, 165);
    lv_obj_set_style_bg_color(sw_wifi, lv_color_hex(0x3be477), LV_PART_INDICATOR);

    lv_obj_t *bt_label = lv_label_create(scr_settings);
    lv_label_set_text(bt_label, "Bluetooth");
    lv_obj_set_style_text_color(bt_label, lv_color_white(), 0);
    lv_obj_align(bt_label, LV_ALIGN_TOP_LEFT, 20, 210);

    lv_obj_t *sw_bt = lv_switch_create(scr_settings);
    lv_obj_align(sw_bt, LV_ALIGN_TOP_RIGHT, -20, 205);
    lv_obj_add_state(sw_bt, LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(sw_bt, lv_color_hex(0x3be477), LV_PART_INDICATOR);

    /* Info Button */
    lv_obj_t *btn_info = lv_btn_create(scr_settings);
    lv_obj_set_size(btn_info, 200, 40);
    lv_obj_align(btn_info, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_bg_color(btn_info, lv_color_hex(0xa29bfe), 0);
    /* Register callback after all screens are created */
    btn_settings_info = btn_info;

    lv_obj_t *lbl_info = lv_label_create(btn_info);
    lv_label_set_text(lbl_info, LV_SYMBOL_CALL " System Info");
    lv_obj_center(lbl_info);

    /* Back Button */
    lv_obj_t *btn_back = lv_btn_create(scr_settings);
    lv_obj_set_size(btn_back, 60, 35);
    lv_obj_align(btn_back, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x5f6368), 0);
    /* Register callback after all screens are created */
    btn_settings_back = btn_back;

    lv_obj_t *lbl_back = lv_label_create(btn_back);
    lv_label_set_text(lbl_back, LV_SYMBOL_LEFT " Back");
    lv_obj_center(lbl_back);
}

/*-----------------------------------------------------------------------------
 * Create System Info Screen
 *---------------------------------------------------------------------------*/
/* This screen displays hardware and firmware information.
 * Contains: Title, Info card with specifications, Back button
 */
static void create_info_screen(void)
{
    scr_info = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_info, lv_color_hex(0x1a1a2e), 0);

    /* Title */
    lv_obj_t *title = lv_label_create(scr_info);
    lv_label_set_text(title, LV_SYMBOL_CALL " System Information");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    /* Info Card */
    lv_obj_t *info_card = lv_obj_create(scr_info);
    lv_obj_set_size(info_card, 200, 200);
    lv_obj_center(info_card);
    lv_obj_set_style_bg_color(info_card, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_border_color(info_card, lv_color_hex(0x0f4c75), 0);
    lv_obj_set_style_border_width(info_card, 2, 0);

    lv_obj_t *info_text = lv_label_create(info_card);
    lv_label_set_text(info_text,
        "MCU: STM32F429ZI\n"
        "Core: ARM Cortex-M4\n"
        "Freq: 180 MHz\n"
        "Flash: 2 MB\n"
        "RAM: 256 KB\n"
        "Display: 320x480\n"
        "LVGL: v9.4.x\n"
        "\n"
        "Status: Running");
    lv_obj_set_style_text_color(info_text, lv_color_hex(0x3be477), 0);
    lv_obj_set_style_text_font(info_text, &lv_font_montserrat_14, 0);
    lv_obj_center(info_text);

    /* Back Button */
    lv_obj_t *btn_back = lv_btn_create(scr_info);
    lv_obj_set_size(btn_back, 60, 35);
    lv_obj_align(btn_back, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x5f6368), 0);
    /* Register callback after all screens are created */
    btn_info_back = btn_back;

    lv_obj_t *lbl_back = lv_label_create(btn_back);
    lv_label_set_text(lbl_back, LV_SYMBOL_LEFT " Back");
    lv_obj_center(lbl_back);
}

static void create_pins_screen(void)
{
    scr_pins = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_pins, lv_color_hex(0x1a1a2e), 0);

    lv_obj_t *title = lv_label_create(scr_pins);
    lv_label_set_text(title, LV_SYMBOL_LIST " MCU Pins");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t *list = lv_list_create(scr_pins);
    lv_obj_set_size(list, 300, 380);
    lv_obj_align(list, LV_ALIGN_BOTTOM_MID, 0, -10);

    for (uint32_t i = 0; i < (sizeof(pin_table) / sizeof(pin_table[0])); i++) {
        lv_obj_t *btn = lv_list_add_btn(list, NULL, pin_table[i].name);
        lv_obj_add_event_cb(btn, pin_item_event_handler, LV_EVENT_CLICKED, (void *)&pin_table[i]);
    }

    lv_obj_t *btn_back = lv_btn_create(scr_pins);
    lv_obj_set_size(btn_back, 60, 35);
    lv_obj_align(btn_back, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x5f6368), 0);
    btn_pins_back = btn_back;

    lv_obj_t *lbl_back = lv_label_create(btn_back);
    lv_label_set_text(lbl_back, LV_SYMBOL_LEFT " Back");
    lv_obj_center(lbl_back);
}

static void create_pin_detail_screen(void)
{
    scr_pin_detail = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_pin_detail, lv_color_hex(0x1a1a2e), 0);

    lv_obj_t *title = lv_label_create(scr_pin_detail);
    lv_label_set_text(title, LV_SYMBOL_EDIT " Pin Detail");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    pin_name_label = lv_label_create(scr_pin_detail);
    lv_label_set_text(pin_name_label, "Pin: --");
    lv_obj_set_style_text_color(pin_name_label, lv_color_white(), 0);
    lv_obj_align(pin_name_label, LV_ALIGN_TOP_LEFT, 20, 60);

    pin_port_label = lv_label_create(scr_pin_detail);
    lv_label_set_text(pin_port_label, "Port: --");
    lv_obj_set_style_text_color(pin_port_label, lv_color_white(), 0);
    lv_obj_align(pin_port_label, LV_ALIGN_TOP_LEFT, 20, 90);

    pin_nature_label = lv_label_create(scr_pin_detail);
    lv_label_set_text(pin_nature_label, "Nature: --");
    lv_obj_set_style_text_color(pin_nature_label, lv_color_white(), 0);
    lv_obj_align(pin_nature_label, LV_ALIGN_TOP_LEFT, 20, 120);

    pin_state_label = lv_label_create(scr_pin_detail);
    lv_label_set_text(pin_state_label, "State: --");
    lv_obj_set_style_text_color(pin_state_label, lv_color_white(), 0);
    lv_obj_align(pin_state_label, LV_ALIGN_TOP_LEFT, 20, 150);

    /* Back button */
    lv_obj_t *btn_back = lv_btn_create(scr_pin_detail);
    lv_obj_set_size(btn_back, 80, 36);
    lv_obj_align(btn_back, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x4ecdc4), 0);
    btn_pin_detail_back = btn_back;

    lv_obj_t *lbl_back = lv_label_create(btn_back);
    lv_label_set_text(lbl_back, LV_SYMBOL_LEFT " Back");
    lv_obj_center(lbl_back);

    /* Create (paused) timer used to refresh the pin state while Pin Detail screen is visible */
    pin_detail_timer = lv_timer_create(pin_detail_timer_cb, 500, NULL);
    lv_timer_pause(pin_detail_timer);

    /* Pause timer when backing out of the detail screen */
    lv_obj_add_event_cb(btn_back, pin_detail_back_cb, LV_EVENT_CLICKED, NULL);
}

/*=============================================================================
 * PUBLIC API FUNCTIONS (Called from main.c)
 *===========================================================================*/

/*-----------------------------------------------------------------------------
 * Initialize LVGL Application
 *---------------------------------------------------------------------------*/
/* This is called ONCE at startup from main.c
 *
 * What it does:
 * 1. Initializes LVGL library
 * 2. Connects to display hardware
 * 3. Connects to touchscreen
 * 4. Creates all 4 screens
 * 5. Shows the home screen
 *
 * After this runs, call LVGL_App_Tick() repeatedly in your main loop.
 */
void LVGL_App_Init(void)
{
    log_debug("LVGL: Initializing application");
    /* Step 1: Initialize LVGL core */
    lv_init();

    /* Step 2: Initialize hardware ports */
    lv_port_disp_init();   /* Connect to display (LCD) */
    lv_port_indev_init();  /* Connect to input (touchscreen) */

    /* Step 3: Create all screens (order doesn't matter since callbacks are registered afterwards) */
    create_sensor_screen();
    create_settings_screen();
    create_info_screen();
    create_pins_screen();
    create_pin_detail_screen();
    create_home_screen();

    /* Register navigation callbacks now that all screen objects exist */
    if (btn_home_sensors) {
        lv_obj_add_event_cb(btn_home_sensors, nav_event_handler, LV_EVENT_CLICKED, scr_sensors);
    }
    if (btn_home_settings) {
        lv_obj_add_event_cb(btn_home_settings, nav_event_handler, LV_EVENT_CLICKED, scr_settings);
    }
    if (btn_sensors_back) {
        lv_obj_add_event_cb(btn_sensors_back, nav_event_handler, LV_EVENT_CLICKED, scr_home);
    }
    if (btn_settings_info) {
        lv_obj_add_event_cb(btn_settings_info, nav_event_handler, LV_EVENT_CLICKED, scr_info);
    }
    if (btn_settings_back) {
        lv_obj_add_event_cb(btn_settings_back, nav_event_handler, LV_EVENT_CLICKED, scr_home);
    }
    if (btn_info_back) {
        lv_obj_add_event_cb(btn_info_back, nav_event_handler, LV_EVENT_CLICKED, scr_settings);
    }
    if (btn_home_pins) {
        lv_obj_add_event_cb(btn_home_pins, nav_event_handler, LV_EVENT_CLICKED, scr_pins);
    }
    if (btn_pins_back) {
        lv_obj_add_event_cb(btn_pins_back, nav_event_handler, LV_EVENT_CLICKED, scr_home);
    }
    if (btn_pin_detail_back) {
        lv_obj_add_event_cb(btn_pin_detail_back, nav_event_handler, LV_EVENT_CLICKED, scr_pins);
    }

    /* Step 4: Load home screen as default */
    lv_screen_load(scr_home);

    /* Force a refresh to ensure the screen is displayed */
    lv_refr_now(NULL);
    log_debug("LVGL: Application initialized successfully");
}

/*-----------------------------------------------------------------------------
 * LVGL Tick Handler - Process LVGL Tasks
 *---------------------------------------------------------------------------*/
/* Call this function periodically (every 1-5ms) in your main loop.
 *
 * What it does:
 * - Processes LVGL's internal timers
 * - Triggers redraws when needed
 * - Handles animations and user input
 *
 * Calling frequency:
 * - Faster (1ms): Smoother animations, more CPU usage
 * - Slower (5ms): Less smooth, saves CPU time
 * - Recommended: 5ms for most applications
 *
 * Example usage in main.c:
 *   while(1) {
 *     LVGL_App_Tick();
 *     HAL_Delay(5);  // 5ms delay = 200 times per second
 *   }
 *
 * Advanced: Call from a timer interrupt for precise timing
 */
void LVGL_App_Tick(void)
{
    lv_timer_handler();  /* Process LVGL's internal tasks */
}

/*-----------------------------------------------------------------------------
 * Helper Functions for Updating UI Elements
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Update Temperature Display
 *---------------------------------------------------------------------------*/
void LVGL_App_UpdateTemperature(int temp_celsius)
{
    if(temp_arc != NULL) {
        lv_arc_set_value(temp_arc, temp_celsius);
    }

    if(temp_label != NULL) {
        static char temp_str[16];
        lv_snprintf(temp_str, sizeof(temp_str), "%d°C\nTemp", temp_celsius);
        lv_label_set_text(temp_label, temp_str);
    }
}

/*-----------------------------------------------------------------------------
 * Update Humidity Display
 *---------------------------------------------------------------------------*/
void LVGL_App_UpdateHumidity(int humidity_percent)
{
    if(humidity_bar != NULL) {
        lv_bar_set_value(humidity_bar, humidity_percent, LV_ANIM_ON);
    }

    if(humidity_label != NULL) {
        static char hum_str[16];
        lv_snprintf(hum_str, sizeof(hum_str), "%d%%\nHumid", humidity_percent);
        lv_label_set_text(humidity_label, hum_str);
    }
}

/*-----------------------------------------------------------------------------
 * Update Status Message
 *---------------------------------------------------------------------------*/
void LVGL_App_UpdateStatus(const char *status)
{
    if(status_label != NULL && status != NULL) {
        lv_label_set_text(status_label, status);
        /* Force a refresh of the label */
        lv_obj_invalidate(status_label);
    }
}

/*-----------------------------------------------------------------------------
 * Add Data Point to Sensor Chart
 *---------------------------------------------------------------------------*/
void LVGL_App_AddChartData(int value)
{
    if(chart_sensor != NULL && chart_series != NULL) {
        lv_chart_set_next_value(chart_sensor, chart_series, value);
    }
}
