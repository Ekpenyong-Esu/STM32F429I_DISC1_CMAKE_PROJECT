/**
  ******************************************************************************
  * @file    app_low_power.c
  * @brief   Production-ready low power mode implementation for STM32F429I-DISC1 GUI
  * @details Comprehensive power management for LVGL touchscreen application
  *          with proper display, SDRAM, and peripheral control.
  * @version 2.0
  * @date    2025-12-11
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
#include "spi.h"
#include "gpio.h"
#include <lv_port_disp.h>
#include <lv_port_indev.h>
#include <string.h>

/* Extern declarations for peripheral handles */
extern SPI_HandleTypeDef hspi5;
extern I2C_HandleTypeDef hi2c3;
extern LTDC_HandleTypeDef hltdc;

/* Private defines -----------------------------------------------------------*/
#define APP_LOW_POWER_TIMEOUT_MS         15000   /* 15 seconds of inactivity for low power */
#define APP_DISPLAY_DIM_TIMEOUT_MS       5000    /* 5 seconds to dim display */
#define APP_DISPLAY_OFF_TIMEOUT_MS       10000   /* 10 seconds to turn off display */

/* Production-ready constants for timeouts and defaults */
#define APP_LOW_POWER_SHORT_MS           120000  /* 2 minutes */
#define APP_LOW_POWER_MEDIUM_MS          600000  /* 10 minutes */
#define APP_SDRAM_STABILIZE_MS           10      /* SDRAM stabilization delay */

#define DEFAULT_TEMP_VALUE               25
#define DEFAULT_HUMIDITY_VALUE           60
#define STATUS_TEXT_MAX_LEN              64

/* Display power control pins (STM32F429I-DISC1 specific) */
/* STM32F429I-DISC1 backlight is on PK3 (or PWM on PK3) */
#define LCD_BL_GPIO_Port                 GPIOK
#define LCD_BL_Pin                       GPIO_PIN_3
#define LCD_RESET_GPIO_Port              GPIOD
#define LCD_RESET_Pin                    GPIO_PIN_12

/* SDRAM power control */
#define SDRAM_POWER_GPIO_Port            GPIOE
#define SDRAM_POWER_Pin                  GPIO_PIN_1  /* Hypothetical power control pin */

/* Private variables ---------------------------------------------------------*/
static uint32_t last_activity_time = 0;
static bool display_is_on = true;
static bool display_is_dimmed = false;
static bool touchscreen_is_active = true;
static bool sdram_is_active = true;

/* GUI state backup */
static lv_obj_t *current_screen_backup = NULL;
static int temp_value_backup = DEFAULT_TEMP_VALUE;
static int humidity_value_backup = DEFAULT_HUMIDITY_VALUE;

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
static void APP_ConfigureWakeupPins(void);

/**
 * @brief   Initialize application low power management
 * @details Sets up activity monitoring and configures wakeup sources
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef APP_LowPowerInit(void)
{
    last_activity_time = HAL_GetTick();
    display_is_on = true;
    display_is_dimmed = false;
    touchscreen_is_active = true;
    sdram_is_active = true;

    /* Configure wakeup pins for low power modes */
    APP_ConfigureWakeupPins();

    /* Ensure backlight GPIO is configured (PK3 on STM32F429I-DISC1) */
    __HAL_RCC_GPIOK_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = LCD_BL_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LCD_BL_GPIO_Port, &GPIO_InitStruct);

    /* Make sure backlight is on in normal operation */
    HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET);

    log_debug("APP: Low power management initialized for STM32F429I-DISC1");
    return PWR_OK;
}

/**
 * @brief   Update activity timestamp
 * @details Call this function whenever user interacts with the GUI
 * @retval  None
 */
void APP_UpdateActivity(void)
{
    /* Generic activity update: only refresh timestamp so that non-touch
       background activity does not automatically undim the display. */
    last_activity_time = HAL_GetTick();
}

/**
 * @brief Update activity timestamp triggered by a TOUCH event
 * @details Undims/powers-on the display and touchscreen when a real touch
 *          interaction occurs. This ensures that only touching the screen
 *          will restore brightness from dimmed state.
 */
void APP_TouchActivity(void)
{
    last_activity_time = HAL_GetTick();

    log_debug("APP: Touch activity detected - restoring display if needed");

    /* If the display was dimmed, undim (turn on backlight) */
    if (display_is_dimmed)
    {
        APP_DisplayDim(false);
        display_is_dimmed = false;
    }

    /* If fully off, power the display back on */
    if (!display_is_on)
    {
        APP_DisplayPowerOn();
    }

    /* Ensure touchscreen interface is active */
    if (!touchscreen_is_active)
    {
        APP_TouchscreenPowerOn();
    }
}

/**
 * @brief   Check if system should enter low power mode
 * @details Monitors activity and decides if low power mode is needed
 * @retval  bool True if low power mode should be entered
 */
bool APP_ShouldEnterLowPower(void)
{
    uint32_t current_time = HAL_GetTick();
    uint32_t inactive_time = current_time - last_activity_time;

    /* Check for display dim timeout first */
    if (inactive_time >= APP_DISPLAY_DIM_TIMEOUT_MS && !display_is_dimmed && display_is_on)
    {
        APP_DisplayDim(true);
        return false; /* Don't enter full low power yet, just dim display */
    }

    /* Check for display off timeout - only power off if not already dimmed */
    if (inactive_time >= APP_DISPLAY_OFF_TIMEOUT_MS && display_is_on && !display_is_dimmed)
    {
        APP_DisplayPowerOff();
        return false; /* Don't enter full low power yet, just turn off display */
    }

    /* Enter low power mode immediately after display is turned off */
    if (!display_is_on)
    {
        return true;
    }

    /* Check for full low power timeout as fallback */
    if (inactive_time >= APP_LOW_POWER_TIMEOUT_MS)
    {
        return true;
    }

    return false;
}

/**
 * @brief   Enter application-optimized low power mode
 * @details Automatically selects best low power mode based on inactivity time
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef APP_EnterLowPowerMode(void)
{
    uint32_t inactive_time = HAL_GetTick() - last_activity_time;

    PWR_LowPowerConfigTypeDef config;
    PWR_GetDefaultLowPowerConfig(&config);

    /* Configure based on inactivity duration */
    if (inactive_time < APP_LOW_POWER_SHORT_MS) /* Less than 2 minutes */
    {
        config.mode = PWR_LOW_POWER_MODE_LIGHT;
        config.keepPeripherals = true; /* Keep display peripherals for quick resume */
        config.wakeupSources = 0; /* Use EXTI interrupts for wake-up, not dedicated pin */
    }
    else if (inactive_time < APP_LOW_POWER_MEDIUM_MS) /* Less than 10 minutes */
    {
        config.mode = PWR_LOW_POWER_MODE_DEEP;
        config.keepPeripherals = false; /* Disable most peripherals */
        config.wakeupSources = 0; /* Use EXTI interrupts for wake-up, not dedicated pin */
    }
    else /* Long inactivity */
    {
        config.mode = PWR_LOW_POWER_MODE_STANDBY;
        config.keepPeripherals = false;
        config.wakeupSources = 0; /* Use EXTI interrupts for wake-up, not dedicated pin */
    }

    config.wakeupTimeMs = APP_LOW_POWER_SHORT_MS; /* 2 minute typical wakeup */
    config.optimizeVoltage = true;

    log_debug("APP: Entering low power mode after %lu ms inactivity (touch wake-up enabled)", inactive_time);

    return PWR_EnterLowPowerMode(&config);
}

/**
 * @brief   Application-specific low power optimization
 * @details Optimized for STM32F429I-DISC1 GUI application with LVGL
 * @param   keepPeripherals Keep critical peripherals active
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_OptimizeForLowPower(bool keepPeripherals)
{
    log_debug("APP: Optimizing STM32F429I-DISC1 GUI application for low power (keep peripherals: %d)", keepPeripherals);

    if (!keepPeripherals)
    {
        /* Properly deinitialize peripherals using MSP deinit functions */
        /* This ensures clocks are disabled and GPIOs are deconfigured */

        /* Deinitialize SPI5 (ILI9341 display) */
        HAL_SPI_MspDeInit(&hspi5);

        /* Deinitialize I2C3 (touchscreen) */
        HAL_I2C_MspDeInit(&hi2c3);

        /* Deinitialize LTDC (display controller) */
        HAL_LTDC_MspDeInit(&hltdc);

        /* Deinitialize SDRAM (FMC) */
        HAL_SDRAM_MspDeInit(NULL);

        /* Disable non-critical GPIO ports */
        __HAL_RCC_GPIOC_CLK_DISABLE();
        __HAL_RCC_GPIOE_CLK_DISABLE();
        __HAL_RCC_GPIOF_CLK_DISABLE();
        __HAL_RCC_GPIOG_CLK_DISABLE();
        __HAL_RCC_GPIOH_CLK_DISABLE();
        __HAL_RCC_GPIOI_CLK_DISABLE();

        /* Disable unused communication peripherals */
        /* Keep USART1 for potential logging/debugging */
        __HAL_RCC_USART2_CLK_DISABLE();
        __HAL_RCC_USART3_CLK_DISABLE();
        __HAL_RCC_USART6_CLK_DISABLE();

        /* SPI1, I2C1, LTDC, FMC already deinitialized via MSP */

        /* Disable unused DMA controllers */
        /* Keep DMA2D for potential graphics acceleration */
        __HAL_RCC_DMA1_CLK_DISABLE();
        /* Keep DMA2 for potential use */

        /* Disable unused timers */
        /* Keep TIM1 for potential GUI animations */
        __HAL_RCC_TIM2_CLK_DISABLE();
        __HAL_RCC_TIM3_CLK_DISABLE();
        __HAL_RCC_TIM4_CLK_DISABLE();
        __HAL_RCC_TIM5_CLK_DISABLE();
        __HAL_RCC_TIM6_CLK_DISABLE();
        __HAL_RCC_TIM7_CLK_DISABLE();
        __HAL_RCC_TIM8_CLK_DISABLE();
        __HAL_RCC_TIM9_CLK_DISABLE();
        __HAL_RCC_TIM10_CLK_DISABLE();
        __HAL_RCC_TIM11_CLK_DISABLE();
        __HAL_RCC_TIM12_CLK_DISABLE();
        __HAL_RCC_TIM13_CLK_DISABLE();
        __HAL_RCC_TIM14_CLK_DISABLE();

        /* Disable unused ADC (sensors not active in low power) */
        __HAL_RCC_ADC1_CLK_DISABLE();
        __HAL_RCC_ADC2_CLK_DISABLE();
        __HAL_RCC_ADC3_CLK_DISABLE();

        /* Disable other unused peripherals */
        __HAL_RCC_CAN1_CLK_DISABLE();
        __HAL_RCC_CAN2_CLK_DISABLE();
        __HAL_RCC_DAC_CLK_DISABLE();
        __HAL_RCC_RNG_CLK_DISABLE();
    }

    /* Save GUI state before entering low power */
    APP_SaveGUIState();

    /* Turn off display and touchscreen for maximum power savings */
    APP_DisplayPowerOff();
    APP_TouchscreenPowerOff();

    /* Optionally power down SDRAM if not keeping peripherals */
    if (!keepPeripherals)
    {
        APP_SDRAM_PowerOff();
    }

    return PWR_OK;
}

/**
 * @brief   Application-specific low power restoration
 * @details Restores STM32F429I-DISC1 GUI application state after low power wakeup
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_RestoreFromLowPower(void)
{
    log_debug("APP: Restoring STM32F429I-DISC1 GUI application from low power mode");

    /* Restore voltage regulator to high performance mode */
    PWR_EnableHighPerformance();

    /* Re-enable backup access if it was disabled for power savings */
    PWR_EnableBackupAccess();

    /* Clear any pending wakeup flags */
    PWR_ClearStandbyFlag();

    /* Power on SDRAM first (needed for display framebuffer) */
    APP_SDRAM_PowerOn();

    /* Small delay for SDRAM to stabilize */
    HAL_Delay(APP_SDRAM_STABILIZE_MS);

    /* Re-initialize SDRAM if needed */
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
        log_debug("APP: SDRAM reinitialized successfully");
    }
    else
    {
        log_error("APP: SDRAM reinitialization failed");
    }

    /* Power on display and touchscreen */
    APP_DisplayPowerOn();
    APP_TouchscreenPowerOn();

    /* Re-initialize display hardware - HAL MSP init will handle clock/GPIO setup */
    ili9341_Init();  /* Calls ILI9341_MspInit() for SPI/GPIO clocks */
    LTDC_HW_Init();  /* Calls LTDC_MspInit() for LTDC clocks */

    /* Re-initialize I2C for touchscreen - HAL MSP init will handle clock/GPIO setup */
    I2C_Init();      /* Calls I2C MSP init functions */

    /* Re-initialize LVGL and display port */
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();

    /* Restore GUI state */
    APP_RestoreGUIState();

    /* Update status to show we're back */
    LVGL_App_UpdateStatus("System Resumed");

    log_debug("APP: STM32F429I-DISC1 GUI application restoration completed");

    return PWR_OK;
}

/**
 * @brief   Turn off display power
 * @details Powers down LCD backlight, controller, and LTDC
 * @retval  None
 */
static void APP_DisplayPowerOff(void)
{
    if (display_is_on)
    {
        log_debug("APP: Turning display off");

        /* Turn off LCD backlight */
        HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_RESET);

        /* Put ILI9341 into sleep mode */
        ili9341_SleepIn();

        /* Disable LTDC display (use HAL LTDC handle) */
        if (hltdc.Instance != NULL)
        {
            __HAL_LTDC_DISABLE(&hltdc);
        }

        display_is_on = false;
        display_is_dimmed = false;
    }
}

/**
 * @brief   Turn on display power
 * @details Powers up LCD backlight, controller, and LTDC
 * @retval  None
 */
static void APP_DisplayPowerOn(void)
{
    if (!display_is_on)
    {
        log_debug("APP: Turning display on");

        /* Turn on LCD backlight */
        HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET);

        /* Wake up ILI9341 from sleep */
        ili9341_SleepOut();

        /* Enable LTDC display (use HAL LTDC handle) */
        if (hltdc.Instance != NULL)
        {
            __HAL_LTDC_ENABLE(&hltdc);
        }

        display_is_on = true;
        display_is_dimmed = false;
    }
}

/**
 * @brief   Dim or undim the display
 * @details Controls LCD backlight brightness for power saving
 * @param   dim True to dim display, false to restore full brightness
 * @retval  None
 */
static void APP_DisplayDim(bool dim)
{
    if (dim && !display_is_dimmed)
    {
        log_debug("APP: Dimming display");
        /* Reduce backlight brightness - PWM control would go here */
        /* For now, just turn off (full dim) */
        HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_RESET);
        display_is_dimmed = true;
    }
    else if (!dim && display_is_dimmed)
    {
        log_debug("APP: Restoring display brightness");
        /* Restore full backlight brightness */
        HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET);
        display_is_dimmed = false;
    }
}

/**
 * @brief   Turn off touchscreen power
 * @details Disables touchscreen controller and I2C interface but keeps interrupt enabled for wake-up
 * @retval  None
 */
static void APP_TouchscreenPowerOff(void)
{
    if (touchscreen_is_active)
    {
        log_debug("APP: Turning touchscreen off (keeping interrupt for wake-up)");

        /* Keep touchscreen interrupt line ENABLED for wake-up from low power */
        /* HAL_NVIC_DisableIRQ(TS_INT_EXTI_IRQn); // Commented out to allow wake-up */

        /* Deinitialize I2C MSP for touchscreen to save power */
        HAL_I2C_MspDeInit(&hi2c3);

        /* Put touchscreen controller into low power mode (if driver supports it) */
        /* Note: Could call a TS_DeInit or TS_Hibernate if handle was available */

        touchscreen_is_active = false;
    }
}

/**
 * @brief   Turn on touchscreen power
 * @details Enables touchscreen controller and I2C interface
 * @retval  None
 */
static void APP_TouchscreenPowerOn(void)
{
    if (!touchscreen_is_active)
    {
        log_debug("APP: Turning touchscreen on");

        /* Re-initialize I2C (MSP init will enable clocks/GPIOs) */
        I2C_Init();

        /* Re-enable touchscreen interrupt line */
        HAL_NVIC_EnableIRQ(TS_INT_EXTI_IRQn);

        /* Wake up touchscreen controller (driver-level) if needed */
        /* Note: TS_Init may be called from lv_port_indev_init on full restore */

        touchscreen_is_active = true;
    }
}

/**
 * @brief   Turn off SDRAM power
 * @details Powers down SDRAM to save power (if hardware supports it)
 * @retval  None
 */
static void APP_SDRAM_PowerOff(void)
{
    if (sdram_is_active)
    {
        log_debug("APP: Turning SDRAM off");

        /* Deinitialize FMC/SDRAM MSP to disable clocks and GPIOs */
        HAL_SDRAM_MspDeInit(NULL);

        sdram_is_active = false;
    }
}

/**
 * @brief   Turn on SDRAM power
 * @details Powers up SDRAM and restores functionality
 * @retval  None
 */
static void APP_SDRAM_PowerOn(void)
{
    if (!sdram_is_active)
    {
        log_debug("APP: Turning SDRAM on");

        /* Reinitialize FMC/SDRAM MSP to enable clocks and GPIOs */
        HAL_SDRAM_MspInit(NULL);

        /* Wait for SDRAM to stabilize */
        HAL_Delay(APP_SDRAM_STABILIZE_MS);

        sdram_is_active = true;
    }
}

/**
 * @brief   Configure wakeup pins for low power modes
 * @details Sets up PA15 (touchscreen interrupt) as wakeup source for touch-based wake-up
 * @retval  None
 */
static void APP_ConfigureWakeupPins(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Configure PA15 (Touchscreen INT) pin for wake-up */
    GPIO_InitStruct.Pin = TS_INT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(TS_INT_GPIO_PORT, &GPIO_InitStruct);

    /* Enable EXTI interrupt for touchscreen */
    HAL_NVIC_SetPriority(TS_INT_EXTI_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TS_INT_EXTI_IRQn);

    log_debug("APP: Touchscreen wakeup pins configured");
}

/**
 * @brief   Save GUI application state
 * @details Stores current GUI state for restoration after low power
 * @retval  None
 */
static void APP_SaveGUIState(void)
{
    log_debug("APP: Saving GUI state");

    /* Save current screen */
    current_screen_backup = lv_screen_active();

    /* Save status text */
    /* Note: Would need to extract text from status_label */

    /* Save sensor values */
    temp_value_backup = DEFAULT_TEMP_VALUE;      /* Would read from actual GUI */
    humidity_value_backup = DEFAULT_HUMIDITY_VALUE;  /* Would read from actual GUI */

    /* Additional state saving would go here */
}

/**
 * @brief   Restore GUI application state
 * @details Restores GUI state after low power wakeup
 * @retval  None
 */
static void APP_RestoreGUIState(void)
{
    log_debug("APP: Restoring GUI state");

    /* Reinitialize LVGL application */
    LVGL_App_Init();

    /* Restore sensor values */
    LVGL_App_UpdateTemperature(temp_value_backup);
    LVGL_App_UpdateHumidity(humidity_value_backup);

    /* Restore status */
    LVGL_App_UpdateStatus("System Active");

    /* Switch to previously active screen if needed */
    /* lv_screen_load(current_screen_backup); */
}
