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
#include "lvgl.h"
#include "lvgl_app.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "../LOG/log.h"

/*-----------------------------------------------------------------------------
 * Global Variables & Forward Declarations
 *---------------------------------------------------------------------------*/
static lv_obj_t *scr_home;      /* Home dashboard screen */
static lv_obj_t *scr_sensors;   /* Sensor monitoring screen */
static lv_obj_t *scr_settings;  /* Settings screen */
static lv_obj_t *scr_info;      /* System info screen */

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

/* Forward declarations */
static void create_home_screen(void);
static void create_sensor_screen(void);
static void create_settings_screen(void);
static void create_info_screen(void);
static void nav_event_handler(lv_event_t *e);

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
}

/*-----------------------------------------------------------------------------
 * Create Sensor Monitoring Screen
 *---------------------------------------------------------------------------*/
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
        "Display: 240x320\n"
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
