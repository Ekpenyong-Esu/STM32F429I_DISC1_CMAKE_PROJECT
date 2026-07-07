/**
 * @file    lcd.c
 * @brief   Character LCD Driver Implementation for STM32F429
 * @details HD44780 compatible LCD driver with 4-bit and 8-bit mode support
 * @version 1.0
 * @date    2026-01-03
 */

/* Includes ------------------------------------------------------------------*/
#include "lcd.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/* Private macros ------------------------------------------------------------*/
#define LCD_CHECK_HANDLE(h)     do { if ((h) == NULL) return LCD_INVALID_PARAM; } while(0)
#define LCD_CHECK_INIT(h)       do { if (!(h)->initialized) return LCD_NOT_INITIALIZED; } while(0)

/* Private variables ---------------------------------------------------------*/

/** Row addresses for different display sizes */
static const uint8_t rowOffsets_16x2[] = {0x00, 0x40, 0x00, 0x40};
static const uint8_t rowOffsets_20x4[] = {0x00, 0x40, 0x14, 0x54};
static const uint8_t rowOffsets_16x4[] = {0x00, 0x40, 0x10, 0x50};

/* Private function prototypes -----------------------------------------------*/
static void LCD_DelayUs(uint32_t us);
static void LCD_DelayMs(uint32_t ms);
static void LCD_SetPin(const LCD_PinTypeDef* pin, GPIO_PinState state);
static void LCD_PulseEnable(LCD_HandleTypeDef* handle);
static void LCD_WriteNibble(LCD_HandleTypeDef* handle, uint8_t nibble);
static void LCD_WriteByte(LCD_HandleTypeDef* handle, uint8_t byte, uint8_t rs);
static void LCD_InitGPIO(LCD_HandleTypeDef* handle);
static void LCD_UpdateDisplayControl(LCD_HandleTypeDef* handle);
static const uint8_t* LCD_GetRowOffsets(LCD_SizeTypeDef size);
static void LCD_GetDimensions(LCD_SizeTypeDef size, uint8_t* cols, uint8_t* rows);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Microsecond delay using DWT or loop
 * @param   us Delay in microseconds
 */
static void LCD_DelayUs(uint32_t us)
{
    /* Use DWT cycle counter if available, otherwise approximate with loop */
    uint32_t clk = SystemCoreClock / 1000000U;
    volatile uint32_t count = us * clk / 4U;
    while (count--) {
        __NOP();
    }
}

/**
 * @brief   Millisecond delay
 * @param   ms Delay in milliseconds
 */
static void LCD_DelayMs(uint32_t ms)
{
    HAL_Delay(ms);
}

/**
 * @brief   Set GPIO pin state
 * @param   pin Pin configuration
 * @param   state Pin state
 */
static void LCD_SetPin(const LCD_PinTypeDef* pin, GPIO_PinState state)
{
    if (pin->port != NULL) {
        HAL_GPIO_WritePin(pin->port, pin->pin, state);
    }
}

/**
 * @brief   Generate enable pulse
 * @param   handle LCD handle
 */
static void LCD_PulseEnable(LCD_HandleTypeDef* handle)
{
    LCD_SetPin(&handle->config.pins.en, GPIO_PIN_RESET);
    LCD_DelayUs(1);
    LCD_SetPin(&handle->config.pins.en, GPIO_PIN_SET);
    LCD_DelayUs(LCD_ENABLE_PULSE_US);
    LCD_SetPin(&handle->config.pins.en, GPIO_PIN_RESET);
    LCD_DelayUs(LCD_COMMAND_DELAY_US);
}

/**
 * @brief   Write 4-bit nibble to LCD
 * @param   handle LCD handle
 * @param   nibble 4-bit data (lower nibble)
 */
static void LCD_WriteNibble(LCD_HandleTypeDef* handle, uint8_t nibble)
{
    LCD_SetPin(&handle->config.pins.d4, (nibble & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    LCD_SetPin(&handle->config.pins.d5, (nibble & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    LCD_SetPin(&handle->config.pins.d6, (nibble & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    LCD_SetPin(&handle->config.pins.d7, (nibble & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    LCD_PulseEnable(handle);
}

/**
 * @brief   Write byte to LCD (4-bit or 8-bit mode)
 * @param   handle LCD handle
 * @param   byte Data byte
 * @param   rs Register select (0=command, 1=data)
 */
static void LCD_WriteByte(LCD_HandleTypeDef* handle, uint8_t byte, uint8_t rs)
{
    /* Set RS pin */
    LCD_SetPin(&handle->config.pins.rs, rs ? GPIO_PIN_SET : GPIO_PIN_RESET);

    /* Set RW pin to write if used */
    if (handle->config.useRW) {
        LCD_SetPin(&handle->config.pins.rw, GPIO_PIN_RESET);
    }

    if (handle->config.mode == LCD_MODE_8BIT) {
        /* 8-bit mode: write all 8 data pins */
        LCD_SetPin(&handle->config.pins.d0, (byte & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        LCD_SetPin(&handle->config.pins.d1, (byte & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        LCD_SetPin(&handle->config.pins.d2, (byte & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        LCD_SetPin(&handle->config.pins.d3, (byte & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        LCD_SetPin(&handle->config.pins.d4, (byte & 0x10) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        LCD_SetPin(&handle->config.pins.d5, (byte & 0x20) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        LCD_SetPin(&handle->config.pins.d6, (byte & 0x40) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        LCD_SetPin(&handle->config.pins.d7, (byte & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);

        LCD_PulseEnable(handle);
    } else {
        /* 4-bit mode: write high nibble first, then low nibble */
        LCD_WriteNibble(handle, (byte >> 4) & 0x0F);
        LCD_WriteNibble(handle, byte & 0x0F);
    }
}

/**
 * @brief   Initialize GPIO pins for LCD
 * @param   handle LCD handle
 */
static void LCD_InitGPIO(LCD_HandleTypeDef* handle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Configure control pins */
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    /* RS pin */
    if (handle->config.pins.rs.port != NULL) {
        GPIO_InitStruct.Pin = handle->config.pins.rs.pin;
        HAL_GPIO_Init(handle->config.pins.rs.port, &GPIO_InitStruct);
        HAL_GPIO_WritePin(handle->config.pins.rs.port, handle->config.pins.rs.pin, GPIO_PIN_RESET);
    }

    /* RW pin (if used) */
    if (handle->config.useRW && handle->config.pins.rw.port != NULL) {
        GPIO_InitStruct.Pin = handle->config.pins.rw.pin;
        HAL_GPIO_Init(handle->config.pins.rw.port, &GPIO_InitStruct);
        HAL_GPIO_WritePin(handle->config.pins.rw.port, handle->config.pins.rw.pin, GPIO_PIN_RESET);
    }

    /* EN pin */
    if (handle->config.pins.en.port != NULL) {
        GPIO_InitStruct.Pin = handle->config.pins.en.pin;
        HAL_GPIO_Init(handle->config.pins.en.port, &GPIO_InitStruct);
        HAL_GPIO_WritePin(handle->config.pins.en.port, handle->config.pins.en.pin, GPIO_PIN_RESET);
    }

    /* Data pins D4-D7 (always used) */
    if (handle->config.pins.d4.port != NULL) {
        GPIO_InitStruct.Pin = handle->config.pins.d4.pin;
        HAL_GPIO_Init(handle->config.pins.d4.port, &GPIO_InitStruct);
    }
    if (handle->config.pins.d5.port != NULL) {
        GPIO_InitStruct.Pin = handle->config.pins.d5.pin;
        HAL_GPIO_Init(handle->config.pins.d5.port, &GPIO_InitStruct);
    }
    if (handle->config.pins.d6.port != NULL) {
        GPIO_InitStruct.Pin = handle->config.pins.d6.pin;
        HAL_GPIO_Init(handle->config.pins.d6.port, &GPIO_InitStruct);
    }
    if (handle->config.pins.d7.port != NULL) {
        GPIO_InitStruct.Pin = handle->config.pins.d7.pin;
        HAL_GPIO_Init(handle->config.pins.d7.port, &GPIO_InitStruct);
    }

    /* Data pins D0-D3 (8-bit mode only) */
    if (handle->config.mode == LCD_MODE_8BIT) {
        if (handle->config.pins.d0.port != NULL) {
            GPIO_InitStruct.Pin = handle->config.pins.d0.pin;
            HAL_GPIO_Init(handle->config.pins.d0.port, &GPIO_InitStruct);
        }
        if (handle->config.pins.d1.port != NULL) {
            GPIO_InitStruct.Pin = handle->config.pins.d1.pin;
            HAL_GPIO_Init(handle->config.pins.d1.port, &GPIO_InitStruct);
        }
        if (handle->config.pins.d2.port != NULL) {
            GPIO_InitStruct.Pin = handle->config.pins.d2.pin;
            HAL_GPIO_Init(handle->config.pins.d2.port, &GPIO_InitStruct);
        }
        if (handle->config.pins.d3.port != NULL) {
            GPIO_InitStruct.Pin = handle->config.pins.d3.pin;
            HAL_GPIO_Init(handle->config.pins.d3.port, &GPIO_InitStruct);
        }
    }

    /* Backlight pin (if used) */
    if (handle->config.useBacklight && handle->config.pins.backlight.port != NULL) {
        GPIO_InitStruct.Pin = handle->config.pins.backlight.pin;
        HAL_GPIO_Init(handle->config.pins.backlight.port, &GPIO_InitStruct);
        HAL_GPIO_WritePin(handle->config.pins.backlight.port,
                          handle->config.pins.backlight.pin, GPIO_PIN_SET);
    }
}

/**
 * @brief   Update display control register
 * @param   handle LCD handle
 */
static void LCD_UpdateDisplayControl(LCD_HandleTypeDef* handle)
{
    uint8_t cmd = LCD_CMD_DISPLAY_CONTROL;

    if (handle->displayOn) cmd |= LCD_DISPLAY_ON;
    if (handle->cursorOn)  cmd |= LCD_CURSOR_ON;
    if (handle->blinkOn)   cmd |= LCD_BLINK_ON;

    LCD_WriteByte(handle, cmd, 0);
}

/**
 * @brief   Get row offsets for display size
 * @param   size Display size type
 * @retval  Pointer to row offset array
 */
static const uint8_t* LCD_GetRowOffsets(LCD_SizeTypeDef size)
{
    switch (size) {
        case LCD_SIZE_20x4:
            return rowOffsets_20x4;
        case LCD_SIZE_16x4:
            return rowOffsets_16x4;
        default:
            return rowOffsets_16x2;
    }
}

/**
 * @brief   Get display dimensions from size type
 * @param   size Display size type
 * @param   cols Pointer to store column count
 * @param   rows Pointer to store row count
 */
static void LCD_GetDimensions(LCD_SizeTypeDef size, uint8_t* cols, uint8_t* rows)
{
    switch (size) {
        case LCD_SIZE_16x2:
            *cols = 16; *rows = 2;
            break;
        case LCD_SIZE_20x4:
            *cols = 20; *rows = 4;
            break;
        case LCD_SIZE_16x4:
            *cols = 16; *rows = 4;
            break;
        case LCD_SIZE_20x2:
            *cols = 20; *rows = 2;
            break;
        case LCD_SIZE_24x2:
            *cols = 24; *rows = 2;
            break;
        case LCD_SIZE_8x2:
            *cols = 8; *rows = 2;
            break;
        case LCD_SIZE_40x2:
            *cols = 40; *rows = 2;
            break;
        default:
            *cols = 16; *rows = 2;
            break;
    }
}

/* Exported functions --------------------------------------------------------*/

/**
 * @brief   Initialize LCD with configuration
 */
LCD_StatusTypeDef LCD_Init(LCD_HandleTypeDef* handle, const LCD_ConfigTypeDef* config)
{
    LCD_CHECK_HANDLE(handle);

    if (config == NULL) {
        return LCD_INVALID_PARAM;
    }

    /* Copy configuration */
    memcpy(&handle->config, config, sizeof(LCD_ConfigTypeDef));

    /* Set dimensions */
    if (config->size == LCD_SIZE_CUSTOM) {
        handle->cols = config->cols;
        handle->rows = config->rows;
    } else {
        LCD_GetDimensions(config->size, &handle->cols, &handle->rows);
    }

    /* Initialize state */
    handle->cursorCol = 0;
    handle->cursorRow = 0;
    handle->displayOn = true;
    handle->cursorOn = false;
    handle->blinkOn = false;
    handle->backlightOn = true;
    handle->initialized = false;

    /* Initialize GPIO pins */
    LCD_InitGPIO(handle);

    /* HD44780 initialization sequence */
    LCD_DelayMs(LCD_INIT_DELAY_MS);

    /* Set RS and RW low */
    LCD_SetPin(&handle->config.pins.rs, GPIO_PIN_RESET);
    if (handle->config.useRW) {
        LCD_SetPin(&handle->config.pins.rw, GPIO_PIN_RESET);
    }

    if (handle->config.mode == LCD_MODE_4BIT) {
        /* 4-bit initialization sequence */
        /* First: try to set 8-bit mode 3 times (in case LCD is in unknown state) */
        LCD_WriteNibble(handle, 0x03);
        LCD_DelayMs(5);

        LCD_WriteNibble(handle, 0x03);
        LCD_DelayUs(150);

        LCD_WriteNibble(handle, 0x03);
        LCD_DelayUs(150);

        /* Now set 4-bit mode */
        LCD_WriteNibble(handle, 0x02);
        LCD_DelayUs(150);

        /* Function set: 4-bit, 2-line, 5x8 dots */
        LCD_WriteByte(handle, LCD_CMD_FUNCTION_SET | LCD_4BIT_MODE | LCD_2LINE | LCD_5x8_DOTS, 0);
    } else {
        /* 8-bit initialization sequence */
        LCD_WriteByte(handle, LCD_CMD_FUNCTION_SET | LCD_8BIT_MODE | LCD_2LINE | LCD_5x8_DOTS, 0);
        LCD_DelayMs(5);

        LCD_WriteByte(handle, LCD_CMD_FUNCTION_SET | LCD_8BIT_MODE | LCD_2LINE | LCD_5x8_DOTS, 0);
        LCD_DelayUs(150);

        LCD_WriteByte(handle, LCD_CMD_FUNCTION_SET | LCD_8BIT_MODE | LCD_2LINE | LCD_5x8_DOTS, 0);
    }

    /* Display off */
    LCD_WriteByte(handle, LCD_CMD_DISPLAY_CONTROL, 0);

    /* Clear display */
    LCD_WriteByte(handle, LCD_CMD_CLEAR_DISPLAY, 0);
    LCD_DelayMs(LCD_CLEAR_DELAY_MS);

    /* Entry mode set: increment cursor, no display shift */
    LCD_WriteByte(handle, LCD_CMD_ENTRY_MODE_SET | LCD_ENTRY_LEFT | LCD_ENTRY_SHIFT_OFF, 0);

    /* Display on */
    LCD_UpdateDisplayControl(handle);

    handle->initialized = true;

    return LCD_OK;
}

/**
 * @brief   Initialize LCD with default 16x2, 4-bit configuration
 */
LCD_StatusTypeDef LCD_InitDefault(LCD_HandleTypeDef* handle, const LCD_PinsTypeDef* pins)
{
    LCD_CHECK_HANDLE(handle);

    if (pins == NULL) {
        return LCD_INVALID_PARAM;
    }

    LCD_ConfigTypeDef config = {0};
    memcpy(&config.pins, pins, sizeof(LCD_PinsTypeDef));
    config.mode = LCD_MODE_4BIT;
    config.size = LCD_SIZE_16x2;
    config.useRW = false;
    config.useBacklight = (pins->backlight.port != NULL);

    return LCD_Init(handle, &config);
}

/**
 * @brief   Deinitialize LCD
 */
LCD_StatusTypeDef LCD_DeInit(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);

    if (handle->initialized) {
        LCD_DisplayOff(handle);
        LCD_BacklightOff(handle);
    }

    handle->initialized = false;

    return LCD_OK;
}

/**
 * @brief   Clear entire display
 */
LCD_StatusTypeDef LCD_Clear(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    LCD_WriteByte(handle, LCD_CMD_CLEAR_DISPLAY, 0);
    LCD_DelayMs(LCD_CLEAR_DELAY_MS);

    handle->cursorCol = 0;
    handle->cursorRow = 0;

    return LCD_OK;
}

/**
 * @brief   Return cursor to home position
 */
LCD_StatusTypeDef LCD_Home(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    LCD_WriteByte(handle, LCD_CMD_RETURN_HOME, 0);
    LCD_DelayMs(LCD_CLEAR_DELAY_MS);

    handle->cursorCol = 0;
    handle->cursorRow = 0;

    return LCD_OK;
}

/**
 * @brief   Turn display on
 */
LCD_StatusTypeDef LCD_DisplayOn(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    handle->displayOn = true;
    LCD_UpdateDisplayControl(handle);

    return LCD_OK;
}

/**
 * @brief   Turn display off
 */
LCD_StatusTypeDef LCD_DisplayOff(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    handle->displayOn = false;
    LCD_UpdateDisplayControl(handle);

    return LCD_OK;
}

/**
 * @brief   Turn backlight on
 */
LCD_StatusTypeDef LCD_BacklightOn(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    if (handle->config.useBacklight) {
        LCD_SetPin(&handle->config.pins.backlight, GPIO_PIN_SET);
        handle->backlightOn = true;
    }

    return LCD_OK;
}

/**
 * @brief   Turn backlight off
 */
LCD_StatusTypeDef LCD_BacklightOff(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    if (handle->config.useBacklight) {
        LCD_SetPin(&handle->config.pins.backlight, GPIO_PIN_RESET);
        handle->backlightOn = false;
    }

    return LCD_OK;
}

/**
 * @brief   Set cursor position
 */
LCD_StatusTypeDef LCD_SetCursor(LCD_HandleTypeDef* handle, uint8_t col, uint8_t row)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    if (col >= handle->cols || row >= handle->rows) {
        return LCD_INVALID_PARAM;
    }

    const uint8_t* rowOffsets = LCD_GetRowOffsets(handle->config.size);
    uint8_t addr = col + rowOffsets[row];

    LCD_WriteByte(handle, LCD_CMD_SET_DDRAM_ADDR | addr, 0);

    handle->cursorCol = col;
    handle->cursorRow = row;

    return LCD_OK;
}

/**
 * @brief   Show cursor
 */
LCD_StatusTypeDef LCD_CursorOn(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    handle->cursorOn = true;
    LCD_UpdateDisplayControl(handle);

    return LCD_OK;
}

/**
 * @brief   Hide cursor
 */
LCD_StatusTypeDef LCD_CursorOff(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    handle->cursorOn = false;
    LCD_UpdateDisplayControl(handle);

    return LCD_OK;
}

/**
 * @brief   Enable cursor blinking
 */
LCD_StatusTypeDef LCD_BlinkOn(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    handle->blinkOn = true;
    LCD_UpdateDisplayControl(handle);

    return LCD_OK;
}

/**
 * @brief   Disable cursor blinking
 */
LCD_StatusTypeDef LCD_BlinkOff(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    handle->blinkOn = false;
    LCD_UpdateDisplayControl(handle);

    return LCD_OK;
}

/**
 * @brief   Move cursor left
 */
LCD_StatusTypeDef LCD_CursorLeft(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    LCD_WriteByte(handle, LCD_CMD_CURSOR_SHIFT | 0x00, 0);

    if (handle->cursorCol > 0) {
        handle->cursorCol--;
    }

    return LCD_OK;
}

/**
 * @brief   Move cursor right
 */
LCD_StatusTypeDef LCD_CursorRight(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    LCD_WriteByte(handle, LCD_CMD_CURSOR_SHIFT | 0x04, 0);

    if (handle->cursorCol < handle->cols - 1) {
        handle->cursorCol++;
    }

    return LCD_OK;
}

/**
 * @brief   Print single character
 */
LCD_StatusTypeDef LCD_PrintChar(LCD_HandleTypeDef* handle, char ch)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    LCD_WriteByte(handle, (uint8_t)ch, 1);

    /* Update cursor position */
    handle->cursorCol++;
    if (handle->cursorCol >= handle->cols) {
        handle->cursorCol = 0;
        handle->cursorRow++;
        if (handle->cursorRow >= handle->rows) {
            handle->cursorRow = 0;
        }
    }

    return LCD_OK;
}

/**
 * @brief   Print string
 */
LCD_StatusTypeDef LCD_PrintString(LCD_HandleTypeDef* handle, const char* str)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    if (str == NULL) {
        return LCD_INVALID_PARAM;
    }

    while (*str) {
        LCD_PrintChar(handle, *str++);
    }

    return LCD_OK;
}

/**
 * @brief   Print string at position
 */
LCD_StatusTypeDef LCD_PrintStringAt(LCD_HandleTypeDef* handle, uint8_t col,
                                     uint8_t row, const char* str)
{
    LCD_StatusTypeDef status = LCD_OK;

    status = LCD_SetCursor(handle, col, row);
    if (status != LCD_OK) {
        return status;
    }

    return LCD_PrintString(handle, str);
}

/**
 * @brief   Print integer value
 */
LCD_StatusTypeDef LCD_PrintInt(LCD_HandleTypeDef* handle, int32_t value)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    char buffer[12] = {0};
    int idx = 0;
    bool negative = false;
    uint32_t uval = 0;

    if (value < 0) {
        negative = true;
        uval = (uint32_t)(-(value + 1)) + 1U;
    } else {
        uval = (uint32_t)value;
    }

    /* Convert to string (reverse order) */
    char temp[12] = {0};
    int tempIdx = 0;

    if (uval == 0) {
        temp[tempIdx++] = '0';
    } else {
        while (uval > 0) {
            temp[tempIdx++] = '0' + (uval % 10);
            uval /= 10;
        }
    }

    /* Add negative sign */
    if (negative) {
        buffer[idx++] = '-';
    }

    /* Reverse the digits */
    while (tempIdx > 0) {
        buffer[idx++] = temp[--tempIdx];
    }
    buffer[idx] = '\0';

    return LCD_PrintString(handle, buffer);
}

/**
 * @brief   Print floating-point value
 */
LCD_StatusTypeDef LCD_PrintFloat(LCD_HandleTypeDef* handle, float value, uint8_t decimals)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    char buffer[20] = {0};
    int idx = 0;
    bool negative = false;

    if (value < 0) {
        negative = true;
        value = -value;
    }

    /* Get integer part */
    int32_t intPart = (int32_t)value;
    float fracPart = value - (float)intPart;

    /* Convert integer part */
    char temp[12] = {0};
    int tempIdx = 0;

    if (intPart == 0) {
        temp[tempIdx++] = '0';
    } else {
        while (intPart > 0) {
            temp[tempIdx++] = '0' + (intPart % 10);
            intPart /= 10;
        }
    }

    /* Add negative sign */
    if (negative) {
        buffer[idx++] = '-';
    }

    /* Reverse integer part */
    while (tempIdx > 0) {
        buffer[idx++] = temp[--tempIdx];
    }

    /* Add decimal point and fractional part */
    if (decimals > 0) {
        buffer[idx++] = '.';

        for (uint8_t i = 0; i < decimals; i++) {
            fracPart *= 10.0f;
            int digit = (int)fracPart;
            buffer[idx++] = '0' + digit;
            fracPart -= (float)digit;
        }
    }

    buffer[idx] = '\0';

    return LCD_PrintString(handle, buffer);
}

/**
 * @brief   Print hexadecimal value
 */
LCD_StatusTypeDef LCD_PrintHex(LCD_HandleTypeDef* handle, uint32_t value, uint8_t digits)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    const char hexChars[] = "0123456789ABCDEF";
    char buffer[12] = {0};

    if (digits > 8) digits = 8;
    if (digits == 0) digits = 1;

    buffer[0] = '0';
    buffer[1] = 'x';

    for (uint8_t i = 0; i < digits; i++) {
        uint8_t nibble = (value >> (4 * (digits - 1 - i))) & 0x0F;
        buffer[2 + i] = hexChars[nibble];
    }
    buffer[2 + digits] = '\0';

    return LCD_PrintString(handle, buffer);
}

/**
 * @brief   Print formatted string
 */
LCD_StatusTypeDef LCD_Printf(LCD_HandleTypeDef* handle, const char* format, ...)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    if (format == NULL) {
        return LCD_INVALID_PARAM;
    }

    char buffer[64] = {0};
    va_list args;

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    return LCD_PrintString(handle, buffer);
}

/**
 * @brief   Clear a specific line
 */
LCD_StatusTypeDef LCD_ClearLine(LCD_HandleTypeDef* handle, uint8_t row)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    if (row >= handle->rows) {
        return LCD_INVALID_PARAM;
    }

    LCD_StatusTypeDef status = LCD_SetCursor(handle, 0, row);
    if (status != LCD_OK) {
        return status;
    }

    for (uint8_t i = 0; i < handle->cols; i++) {
        LCD_PrintChar(handle, ' ');
    }

    return LCD_SetCursor(handle, 0, row);
}

/**
 * @brief   Scroll display left
 */
LCD_StatusTypeDef LCD_ScrollLeft(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    LCD_WriteByte(handle, LCD_CMD_CURSOR_SHIFT | 0x08, 0);

    return LCD_OK;
}

/**
 * @brief   Scroll display right
 */
LCD_StatusTypeDef LCD_ScrollRight(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    LCD_WriteByte(handle, LCD_CMD_CURSOR_SHIFT | 0x0C, 0);

    return LCD_OK;
}

/**
 * @brief   Enable auto-scroll mode
 */
LCD_StatusTypeDef LCD_AutoScrollOn(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    LCD_WriteByte(handle, LCD_CMD_ENTRY_MODE_SET | LCD_ENTRY_LEFT | LCD_ENTRY_SHIFT_ON, 0);

    return LCD_OK;
}

/**
 * @brief   Disable auto-scroll mode
 */
LCD_StatusTypeDef LCD_AutoScrollOff(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    LCD_WriteByte(handle, LCD_CMD_ENTRY_MODE_SET | LCD_ENTRY_LEFT | LCD_ENTRY_SHIFT_OFF, 0);

    return LCD_OK;
}

/**
 * @brief   Create custom character
 */
LCD_StatusTypeDef LCD_CreateChar(LCD_HandleTypeDef* handle, uint8_t location,
                                  const uint8_t* charmap)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    if (charmap == NULL || location > 7) {
        return LCD_INVALID_PARAM;
    }

    /* Set CGRAM address */
    LCD_WriteByte(handle, LCD_CMD_SET_CGRAM_ADDR | (location << 3), 0);

    /* Write character pattern (8 bytes) */
    for (uint8_t i = 0; i < 8; i++) {
        LCD_WriteByte(handle, charmap[i], 1);
    }

    /* Return to DDRAM */
    LCD_SetCursor(handle, handle->cursorCol, handle->cursorRow);

    return LCD_OK;
}

/**
 * @brief   Print custom character
 */
LCD_StatusTypeDef LCD_PrintCustomChar(LCD_HandleTypeDef* handle, uint8_t location)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    if (location > 7) {
        return LCD_INVALID_PARAM;
    }

    LCD_WriteByte(handle, location, 1);

    return LCD_OK;
}

/**
 * @brief   Send command to LCD
 */
LCD_StatusTypeDef LCD_SendCommand(LCD_HandleTypeDef* handle, uint8_t cmd)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    LCD_WriteByte(handle, cmd, 0);

    return LCD_OK;
}

/**
 * @brief   Send data to LCD
 */
LCD_StatusTypeDef LCD_SendData(LCD_HandleTypeDef* handle, uint8_t data)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    LCD_WriteByte(handle, data, 1);

    return LCD_OK;
}
