/**
  ******************************************************************************
  * @file    app_low_power.c
  * @brief   CORRECTED Production-ready low power for STM32F429I-DISC1
  * @details Fixed version with proper EXTI wake-up and peripheral handling
  * @version 3.0
  * @date    2025-01-25
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "pwr.h"
#include "log.h"
#include "lvgl.h"
#include "lvgl_app.h"
#include "ltdc.h"
#include "ili9341.h"
#include "i2c.h"
#include "touchscreen.h"
#include "fmc.h"
#include "gpio.h"
#include <lv_port_disp.h>
#include <lv_port_indev.h>

/* Private defines -----------------------------------------------------------*/
#define APP_LOW_POWER_TIMEOUT_MS         15000   /* 15 seconds */
#define APP_DISPLAY_DIM_TIMEOUT_MS       5000    /* 5 seconds */
#define APP_DISPLAY_OFF_TIMEOUT_MS       10000   /* 10 seconds */
#define APP_LOW_POWER_SHORT_MS           120000  /* 2 minutes */
#define APP_LOW_POWER_MEDIUM_MS          600000  /* 10 minutes */
#define APP_SDRAM_STABILIZE_MS           10
#define APP_FADE_DIM_MS                  300
#define APP_FADE_POWER_MS                500
#define DEFAULT_TEMP_VALUE               25
#define DEFAULT_HUMIDITY_VALUE           60
/* Debounce for application-level touch activity coalescing (ms) */
#define APP_TOUCH_ACTIVITY_DEBOUNCE_MS   80

/* Display power control pins */
#define LCD_BL_GPIO_Port                 GPIOK
#define LCD_BL_Pin                       GPIO_PIN_3
#define LCD_RESET_GPIO_Port              GPIOD
#define LCD_RESET_Pin                    GPIO_PIN_12

/* Private variables ---------------------------------------------------------*/
static uint32_t last_activity_time = 0;
static bool display_is_on = true;
static bool display_is_dimmed = false;
static bool touchscreen_is_active = true;
static bool sdram_is_active = true;
static volatile bool s_auto_sleep_request = false;
/* Last accepted touch tick for debounce */
static uint32_t _app_last_touch_tick = 0;

/* GUI state backup */
static lv_obj_t *current_screen_backup = NULL;
static int temp_value_backup = DEFAULT_TEMP_VALUE;
static int humidity_value_backup = DEFAULT_HUMIDITY_VALUE;

/* Dim overlay */
static lv_obj_t *s_dim_overlay = NULL;


/* Private function prototypes -----------------------------------------------*/
static void APP_DisplayPowerOff(void);
static void APP_DisplayPowerOn(void);
static void APP_DisplayDim(bool dim);
static void APP_TouchscreenPowerOff(void);
static void APP_TouchscreenPowerOn(void);
static void APP_SDRAM_PowerOff(void);
static void APP_SDRAM_PowerOn(void);
static void APP_SaveGUIState(void);
static void APP_RestoreGUIState(void);
static void APP_FadeToBlack(uint32_t msec, bool power_off_after);
static void APP_FadeFromBlack(uint32_t msec);
static void APP_DimAnimExec(void * var, int32_t value);
static void APP_DimAnimReady_Dim(lv_anim_t * anim);
static void APP_DimAnimReady_PowerOff(lv_anim_t * anim);
static void APP_DimAnimReady_Remove(lv_anim_t * anim);

/* Public accessors ----------------------------------------------------------*/
bool APP_IsAutoSleepRequested(void)
{
    return s_auto_sleep_request;
}

void APP_ClearAutoSleepRequest(void)
{
    s_auto_sleep_request = false;
}

/**
 * @brief   Initialize application low power management
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef APP_LowPowerInit(void)
{
    last_activity_time = HAL_GetTick();
    display_is_on = true;
    display_is_dimmed = false;
    touchscreen_is_active = true;
    sdram_is_active = true;

    /* Wakeup pins are delegated to the touchscreen driver (TS_ITConfig during TS_Init).
     * Ensure TS_Init()/TS_ITConfig is run before entering low power if wake is required. */
    log_debug("APP: Wakeup pins delegated to touchscreen driver");

    /* Ensure backlight GPIO is configured */
    __HAL_RCC_GPIOK_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = LCD_BL_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LCD_BL_GPIO_Port, &GPIO_InitStruct);

    /* Make sure backlight is on */
    HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET);

    log_debug("APP: Low power management initialized for STM32F429I-DISC1");
    return PWR_OK;
}

/**
 * @brief   Update activity timestamp (non-touch events)
 */
void APP_UpdateActivity(void)
{
    last_activity_time = HAL_GetTick();
}

/**
 * @brief   Update activity from touch event
 * @details Restores display if dimmed/off
 */
void APP_TouchActivity(void)
{
    uint32_t _now = HAL_GetTick();

    /* Coalesce repeated touch activity calls coming from EXTI and LVGL input.
     * Some hardware/drivers trigger both an EXTI callback and LVGL sampling
     * on the same touch. Ignore duplicate calls within debounce window. */
    if ((_now - _app_last_touch_tick) < APP_TOUCH_ACTIVITY_DEBOUNCE_MS)
    {
        log_debug("APP: Touch activity ignored (debounce)");
        return;
    }

    _app_last_touch_tick = _now;
    last_activity_time = _now;

    log_debug("APP: Touch activity detected");

    /* Undim if dimmed */
    if (display_is_dimmed)
    {
        APP_DisplayDim(false);
        display_is_dimmed = false;
    }

    /* Power on if off */
    if (!display_is_on)
    {
        APP_DisplayPowerOn();
    }

    /* Ensure touchscreen is active */
    if (!touchscreen_is_active)
    {
        APP_TouchscreenPowerOn();
    }
}

/**
 * @brief   Check if system should enter low power mode
 * @retval  bool True if should enter low power
 */
bool APP_ShouldEnterLowPower(void)
{
    uint32_t current_time = HAL_GetTick();
    uint32_t inactive_time = current_time - last_activity_time;

    /* Check for auto-sleep request from animation */
    if (s_auto_sleep_request)
    {
        return true;
    }

    /* Dim timeout */
    if (inactive_time >= APP_DISPLAY_DIM_TIMEOUT_MS && !display_is_dimmed && display_is_on)
    {
        APP_DisplayDim(true);
        return false;
    }

    /* Display off timeout */
    if (inactive_time >= APP_DISPLAY_OFF_TIMEOUT_MS && display_is_on && !display_is_dimmed)
    {
        APP_DisplayPowerOff();
        return false;
    }

    /* Enter low power after display is off */
    if (!display_is_on)
    {
        return true;
    }

    /* Fallback timeout */
    if (inactive_time >= APP_LOW_POWER_TIMEOUT_MS)
    {
        return true;
    }

    return false;
}

/**
 * @brief   Enter application-optimized low power mode
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef APP_EnterLowPowerMode(void)
{
    uint32_t inactive_time = HAL_GetTick() - last_activity_time;

    PWR_LowPowerConfigTypeDef config;
    PWR_GetDefaultLowPowerConfig(&config);

    /* Select mode based on inactivity */
    if (inactive_time < APP_LOW_POWER_SHORT_MS)
    {
        config.mode = PWR_LOW_POWER_MODE_LIGHT;
        config.keepPeripherals = true;
        config.wakeupSources = 0;  /* EXTI wake-up only */
    }
    else if (inactive_time < APP_LOW_POWER_MEDIUM_MS)
    {
        config.mode = PWR_LOW_POWER_MODE_DEEP;
        config.keepPeripherals = false;
        config.wakeupSources = 0;  /* EXTI wake-up only */
    }
    else
    {
        /* Use DEEP for long sleep (EXTI can wake from Stop mode) */
        config.mode = PWR_LOW_POWER_MODE_DEEP;
        config.keepPeripherals = false;
        config.wakeupSources = 0;
    }

    config.wakeupTimeMs = APP_LOW_POWER_SHORT_MS;
    config.optimizeVoltage = true;

    log_debug("APP: Entering low power mode after %lu ms inactivity", inactive_time);

    return PWR_EnterLowPowerMode(&config);
}

/**
 * @brief   Application-specific low power optimization
 * @details Overrides weak function in pwr.c
 * @param   keepPeripherals Keep critical peripherals active
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_OptimizeForLowPower(bool keepPeripherals)
{
    log_debug("APP: Optimizing STM32F429I-DISC1 for low power (keep: %d)", keepPeripherals);

    if (!keepPeripherals)
    {
        /* Save GUI state first */
        APP_SaveGUIState();

        /* Power off display and touchscreen but keep INT enabled */
        APP_DisplayPowerOff();

        /* Wait briefly for fade animation to complete so LVGL callbacks finish
         * and any overlay objects are removed before we power off peripherals
         * (prevents dangling LVGL objects or callbacks against powered-down SDRAM/LTDC). */
        uint32_t _wait_start = HAL_GetTick();
        while (s_dim_overlay && (HAL_GetTick() - _wait_start) < 1000)
        {
            lv_timer_handler();
            HAL_Delay(5);
        }

        /* Now safe to power off touchscreen and SDRAM */
        APP_TouchscreenPowerOff();

        /* Power off SDRAM */
        APP_SDRAM_PowerOff();

        /* Deinitialize non-critical peripherals */
        /* Note: Don't disable I2C3/EXTI as we need touchscreen interrupt */

        /* Disable non-critical timers */
        __HAL_RCC_TIM2_CLK_DISABLE();
        __HAL_RCC_TIM3_CLK_DISABLE();
        __HAL_RCC_TIM4_CLK_DISABLE();
        __HAL_RCC_TIM5_CLK_DISABLE();

        /* Disable unused communication interfaces */
        __HAL_RCC_USART2_CLK_DISABLE();
        __HAL_RCC_USART3_CLK_DISABLE();
        __HAL_RCC_USART6_CLK_DISABLE();

        /* Disable unused SPI */
        __HAL_RCC_SPI2_CLK_DISABLE();
        __HAL_RCC_SPI3_CLK_DISABLE();

        /* Disable ADC */
        __HAL_RCC_ADC1_CLK_DISABLE();
        __HAL_RCC_ADC2_CLK_DISABLE();
        __HAL_RCC_ADC3_CLK_DISABLE();

        /* Disable non-critical GPIO ports */
        __HAL_RCC_GPIOE_CLK_DISABLE();
        __HAL_RCC_GPIOF_CLK_DISABLE();
        __HAL_RCC_GPIOG_CLK_DISABLE();
        __HAL_RCC_GPIOH_CLK_DISABLE();
        __HAL_RCC_GPIOI_CLK_DISABLE();

        log_debug("APP: Non-critical peripherals disabled");
    }

    return PWR_OK;
}

/**
 * @brief   Application-specific restoration
 * @details Overrides weak function in pwr.c
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_RestoreFromLowPower(void)
{
    log_debug("APP: Restoring STM32F429I-DISC1 from low power");

    /* Restore voltage regulator */
    PWR_EnableHighPerformance();

    /* Clear wakeup flags */
    PWR_ClearStandbyFlag();

    /* Power on SDRAM first */
    APP_SDRAM_PowerOn();
    HAL_Delay(APP_SDRAM_STABILIZE_MS);

    /* Reinitialize SDRAM */
    FMC_Driver_Handle_t fmcHandle;
    FMC_Driver_SDRAM_Config_t sdramConfig = {
        .bank = FMC_SDRAM_BANK2,
        .columnBits = FMC_SDRAM_COLUMN_BITS_NUM_8,
        .rowBits = FMC_SDRAM_ROW_BITS_NUM_12,
        .dataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16,
        .internalBanks = FMC_SDRAM_INTERN_BANKS_NUM_4,
        .casLatency = FMC_SDRAM_CAS_LATENCY_3,
        .clockPeriod = FMC_SDRAM_CLOCK_PERIOD_3,
        .readBurst = FMC_SDRAM_RBURST_DISABLE,
        .readPipeDelay = FMC_SDRAM_RPIPE_DELAY_1,
        .writeProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE,
        .loadToActiveDelay = 2,
        .exitSelfRefreshDelay = 7,
        .selfRefreshTime = 4,
        .rowCycleDelay = 7,
        .writeRecoveryTime = 2,
        .rpDelay = 2,
        .rcdDelay = 2
    };

    if (FMC_Driver_SDRAM_Init(&fmcHandle, &sdramConfig) == HAL_OK)
    {
        log_debug("APP: SDRAM reinitialized");
    }
    else
    {
        log_error("APP: SDRAM reinit failed");
    }

    /* Power on display and touchscreen */
    APP_DisplayPowerOn();
    APP_TouchscreenPowerOn();

    /* Reinitialize display hardware */
    ili9341_Init();
    LTDC_HW_Init();

    /* Reinitialize I2C for touchscreen */
    I2C_Init();

    /* Reinitialize LVGL */
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();

    /* Restore GUI state */
    APP_RestoreGUIState();

    /* Update status */
    LVGL_App_UpdateStatus("System Resumed");

    log_debug("APP: Restoration completed");

    return PWR_OK;
}

/* LVGL animation callbacks --------------------------------------------------*/

static void APP_DimAnimExec(void * var, int32_t value)
{
    lv_obj_t *overlay = (lv_obj_t *)var;
    if (overlay)
    {
        lv_obj_set_style_bg_opa(overlay, (lv_opa_t)value, 0);
    }
}

static void APP_DimAnimReady_Remove(lv_anim_t * anim)
{
    lv_obj_t *overlay = (lv_obj_t *)anim->var;
    if (overlay)
    {
        lv_obj_del(overlay);
        if (s_dim_overlay == overlay) { s_dim_overlay = NULL; }
    }
}

static void APP_DimAnimReady_Dim(lv_anim_t * anim)
{
    (void)anim;

    /* Turn off backlight */
    HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_RESET);
    display_is_dimmed = true;

    /* Request low-power entry */
    log_debug("APP: Auto-sleep requested after dim");
    s_auto_sleep_request = true;
}

static void APP_DimAnimReady_PowerOff(lv_anim_t * anim)
{
    (void)anim;

    /* Ensure backlight off */
    HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_RESET);

    /* Put ILI9341 into sleep */
    ili9341_SleepIn();

    /* Disable LTDC */
    if (hltdc.Instance != NULL)
    {
        __HAL_LTDC_DISABLE(&hltdc);
    }

    display_is_on = false;
    display_is_dimmed = false;

    /* Remove overlay */
    lv_obj_t *overlay = (lv_obj_t *)anim->var;
    if (overlay)
    {
        lv_obj_del(overlay);
        if (s_dim_overlay == overlay) { s_dim_overlay = NULL; }
    }
}

/**
 * @brief   Fade to black animation
 * @param   msec Animation duration
 * @param   power_off_after True to power off display after fade
 */
static void APP_FadeToBlack(uint32_t msec, bool power_off_after)
{
    if (s_dim_overlay) { return; }

    s_dim_overlay = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(s_dim_overlay);
    lv_obj_set_size(s_dim_overlay, LV_HOR_RES, LV_VER_RES);
    lv_obj_clear_flag(s_dim_overlay, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(s_dim_overlay, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(s_dim_overlay, LV_OPA_TRANSP, 0);

    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, s_dim_overlay);
    lv_anim_set_exec_cb(&anim, APP_DimAnimExec);
    lv_anim_set_values(&anim, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_time(&anim, msec ? msec : APP_FADE_POWER_MS);
    lv_anim_set_ready_cb(&anim, power_off_after ? APP_DimAnimReady_PowerOff : APP_DimAnimReady_Dim);
    lv_anim_start(&anim);
}

/**
 * @brief   Fade from black animation
 * @param   msec Animation duration
 */
static void APP_FadeFromBlack(uint32_t msec)
{
    if (!s_dim_overlay) { return; }

    /* Ensure backlight is on */
    HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET);
    display_is_dimmed = false;

    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, s_dim_overlay);
    lv_anim_set_exec_cb(&anim, APP_DimAnimExec);
    lv_anim_set_values(&anim, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_time(&anim, msec ? msec : APP_FADE_DIM_MS);
    lv_anim_set_ready_cb(&anim, APP_DimAnimReady_Remove);
    lv_anim_start(&anim);
}

/* Display control functions -------------------------------------------------*/

static void APP_DisplayPowerOff(void)
{
    if (display_is_on)
    {
        log_debug("APP: Turning display off");
        APP_FadeToBlack(APP_FADE_POWER_MS, true);
    }
}

static void APP_DisplayPowerOn(void)
{
    if (!display_is_on)
    {
        log_debug("APP: Turning display on");

        /* Turn on backlight */
        HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET);

        /* Wake up ILI9341 */
        ili9341_SleepOut();

        /* Enable LTDC */
        if (hltdc.Instance != NULL)
        {
            __HAL_LTDC_ENABLE(&hltdc);
        }

        display_is_on = true;
        display_is_dimmed = false;

        /* Fade from black */
        APP_FadeFromBlack(APP_FADE_DIM_MS);
    }
}

static void APP_DisplayDim(bool dim)
{
    if (dim && !display_is_dimmed)
    {
        log_debug("APP: Dimming display");
        APP_FadeToBlack(APP_FADE_DIM_MS, false);
    }
    else if (!dim && display_is_dimmed)
    {
        log_debug("APP: Restoring display brightness");
        APP_FadeFromBlack(APP_FADE_DIM_MS);
    }
}

/* Touchscreen control -------------------------------------------------------*/

static void APP_TouchscreenPowerOff(void)
{
    if (touchscreen_is_active)
    {
        log_debug("APP: Turning touchscreen off (keeping INT enabled)");

        /* CRITICAL: Keep touchscreen interrupt enabled for wake-up!
         * Only deinitialize I2C to save power */

        /* Do NOT disable NVIC for touchscreen EXTI here; keep EXTI configured
         * so the touch INT can reliably wake the MCU from Stop mode.
         * We mark touchscreen as inactive to ignore callbacks until I2C is reinitialized. */

        /* Deinitialize I2C peripheral but keep EXTI GPIO state (keep INT wake configured) */
        I2C_DeInit();

        touchscreen_is_active = false;
    }
}

static void APP_TouchscreenPowerOn(void)
{
    if (!touchscreen_is_active)
    {
        log_debug("APP: Turning touchscreen on");

        /* Reinitialize I2C */
        I2C_Init();

        /* Clear any pending EXTI flag then enable NVIC for touchscreen interrupt.
         * Clearing pending flag avoids immediately invoking the callback while
         * system is still stabilizing. */
        __HAL_GPIO_EXTI_CLEAR_IT(TS_INT_PIN);

        /* Ensure interrupt is enabled now that I2C is ready */
        HAL_NVIC_EnableIRQ(TS_INT_EXTI_IRQn);

        touchscreen_is_active = true;
    }
}

/* SDRAM control -------------------------------------------------------------*/

static void APP_SDRAM_PowerOff(void)
{
    if (sdram_is_active)
    {
        log_debug("APP: Turning SDRAM off");
        /* No public SDRAM handle available here; call MSP deinit to disable FMC GPIO/clock safely. */
        HAL_SDRAM_MspDeInit(NULL);
        sdram_is_active = false;
    }
}

static void APP_SDRAM_PowerOn(void)
{
    if (!sdram_is_active)
    {
        log_debug("APP: Turning SDRAM on");
        /* No public handle here; call MSP init to enable FMC GPIO/clock prior to SDRAM reinit.
           The full reinitialization is performed later using FMC driver in restore path. */
        HAL_SDRAM_MspInit(NULL);
        HAL_Delay(APP_SDRAM_STABILIZE_MS);
        sdram_is_active = true;
    }
}

/* Wakeup configuration ------------------------------------------------------*/


/* State management ----------------------------------------------------------*/

static void APP_SaveGUIState(void)
{
    log_debug("APP: Saving GUI state");

    current_screen_backup = lv_screen_active();
    temp_value_backup = DEFAULT_TEMP_VALUE;
    humidity_value_backup = DEFAULT_HUMIDITY_VALUE;
}

static void APP_RestoreGUIState(void)
{
    log_debug("APP: Restoring GUI state");

    /* Reinitialize LVGL application */
    LVGL_App_Init();

    /* Restore values */
    LVGL_App_UpdateTemperature(temp_value_backup);
    LVGL_App_UpdateHumidity(humidity_value_backup);
    LVGL_App_UpdateStatus("System Active");
}
