/**
 * @file    seven_segment.c
 * @brief   Seven-Segment Display Driver Implementation
 * @details Core implementation for seven-segment display driver
 * @version 1.0
 * @date    2026-01-03
 */

/* Includes ------------------------------------------------------------------*/
#include "seven_segment.h"
#include <string.h>
#include <stdlib.h>

/* Private constants ---------------------------------------------------------*/

/**
 * @brief Digit patterns lookup table (0-F)
 */
static const uint8_t digitPatterns[16] = {
    SEG_PATTERN_0, SEG_PATTERN_1, SEG_PATTERN_2, SEG_PATTERN_3,
    SEG_PATTERN_4, SEG_PATTERN_5, SEG_PATTERN_6, SEG_PATTERN_7,
    SEG_PATTERN_8, SEG_PATTERN_9, SEG_PATTERN_A, SEG_PATTERN_B,
    SEG_PATTERN_C, SEG_PATTERN_D, SEG_PATTERN_E, SEG_PATTERN_F
};

/* Private function prototypes -----------------------------------------------*/
static void Seg_GPIO_Init(SegDisplayHandle_t* handle);
static void Seg_GPIO_WriteSegments(SegDisplayHandle_t* handle, uint8_t pattern);
static void Seg_GPIO_SelectDigit(SegDisplayHandle_t* handle, uint8_t digit);
static void Seg_GPIO_DeselectAllDigits(SegDisplayHandle_t* handle);

static void Seg_HT1621_Init(SegDisplayHandle_t* handle);
static void Seg_HT1621_SendCommand(SegDisplayHandle_t* handle, uint8_t cmd);
static void Seg_HT1621_SendData(SegDisplayHandle_t* handle, uint8_t addr, uint8_t data);
static void Seg_HT1621_WriteBit(SegDisplayHandle_t* handle, bool bit);

static void Seg_DelayUs(uint32_t us);

/* Exported functions --------------------------------------------------------*/

/**
 * @brief   Initialize seven-segment display
 */
SegStatus_t Seg_Init(SegDisplayHandle_t* handle, const SegDisplayConfig_t* config)
{
    if (handle == NULL || config == NULL) {
        return SEG_INVALID_PARAM;
    }

    /* Validate digit count */
    uint8_t digitCount = (config->driverType == SEG_DRIVER_GPIO)
                         ? config->config.gpio.digitCount
                         : config->config.ht1621.digitCount;

    if (digitCount == 0 || digitCount > SEG_MAX_DIGITS) {
        return SEG_INVALID_PARAM;
    }

    /* Copy configuration */
    memcpy(&handle->config, config, sizeof(SegDisplayConfig_t));

    /* Allocate display buffer */
    handle->displayBuffer = (uint8_t*)malloc(digitCount * sizeof(uint8_t));
    if (handle->displayBuffer == NULL) {
        return SEG_ERROR;
    }

    /* Clear buffer */
    memset(handle->displayBuffer, 0, digitCount);

    /* Initialize based on driver type */
    if (config->driverType == SEG_DRIVER_GPIO) {
        Seg_GPIO_Init(handle);
    } else {
        Seg_HT1621_Init(handle);
    }

    handle->currentDigit = 0;
    handle->enabled = true;
    handle->brightness = 100;
    handle->initialized = true;

    return SEG_OK;
}

/**
 * @brief   Deinitialize seven-segment display
 */
SegStatus_t Seg_DeInit(SegDisplayHandle_t* handle)
{
    if (handle == NULL) {
        return SEG_INVALID_PARAM;
    }

    if (!handle->initialized) {
        return SEG_NOT_INITIALIZED;
    }

    /* Disable display */
    Seg_Disable(handle);

    /* Free display buffer */
    if (handle->displayBuffer != NULL) {
        free(handle->displayBuffer);
        handle->displayBuffer = NULL;
    }

    handle->initialized = false;

    return SEG_OK;
}

/**
 * @brief   Enable display output
 */
SegStatus_t Seg_Enable(SegDisplayHandle_t* handle)
{
    if (handle == NULL) {
        return SEG_INVALID_PARAM;
    }

    if (!handle->initialized) {
        return SEG_NOT_INITIALIZED;
    }

    handle->enabled = true;

    if (handle->config.driverType == SEG_DRIVER_HT1621) {
        Seg_HT1621_SendCommand(handle, HT1621_CMD_LCD_ON);
    }

    return SEG_OK;
}

/**
 * @brief   Disable display output
 */
SegStatus_t Seg_Disable(SegDisplayHandle_t* handle)
{
    if (handle == NULL) {
        return SEG_INVALID_PARAM;
    }

    if (!handle->initialized) {
        return SEG_NOT_INITIALIZED;
    }

    handle->enabled = false;

    if (handle->config.driverType == SEG_DRIVER_GPIO) {
        Seg_GPIO_DeselectAllDigits(handle);
    } else {
        Seg_HT1621_SendCommand(handle, HT1621_CMD_LCD_OFF);
    }

    return SEG_OK;
}

/**
 * @brief   Clear all digits
 */
SegStatus_t Seg_Clear(SegDisplayHandle_t* handle)
{
    if (handle == NULL) {
        return SEG_INVALID_PARAM;
    }

    if (!handle->initialized) {
        return SEG_NOT_INITIALIZED;
    }

    uint8_t digitCount = (handle->config.driverType == SEG_DRIVER_GPIO)
                         ? handle->config.config.gpio.digitCount
                         : handle->config.config.ht1621.digitCount;

    memset(handle->displayBuffer, SEG_PATTERN_BLANK, digitCount);

    if (handle->config.driverType == SEG_DRIVER_HT1621) {
        Seg_Refresh(handle);
    }

    return SEG_OK;
}

/**
 * @brief   Set display brightness (HT1621 only)
 */
SegStatus_t Seg_SetBrightness(SegDisplayHandle_t* handle, uint8_t brightness)
{
    if (handle == NULL) {
        return SEG_INVALID_PARAM;
    }

    if (!handle->initialized) {
        return SEG_NOT_INITIALIZED;
    }

    if (brightness > 100) {
        brightness = 100;
    }

    handle->brightness = brightness;

    /* Note: HT1621 doesn't have built-in brightness control
     * This would require PWM on backlight or bias adjustment
     * For now, just store the value for potential future use */

    return SEG_OK;
}

/**
 * @brief   Display a single digit value
 */
SegStatus_t Seg_SetDigit(SegDisplayHandle_t* handle, uint8_t position,
                         uint8_t value, bool showDp)
{
    if (handle == NULL) {
        return SEG_INVALID_PARAM;
    }

    if (!handle->initialized) {
        return SEG_NOT_INITIALIZED;
    }

    uint8_t digitCount = (handle->config.driverType == SEG_DRIVER_GPIO)
                         ? handle->config.config.gpio.digitCount
                         : handle->config.config.ht1621.digitCount;

    if (position >= digitCount || value > 15) {
        return SEG_INVALID_PARAM;
    }

    uint8_t pattern = digitPatterns[value];
    if (showDp) {
        pattern |= SEG_PATTERN_DP;
    }

    handle->displayBuffer[position] = pattern;

    if (handle->config.driverType == SEG_DRIVER_HT1621) {
        Seg_Refresh(handle);
    }

    return SEG_OK;
}

/**
 * @brief   Display raw segment pattern
 */
SegStatus_t Seg_SetPattern(SegDisplayHandle_t* handle, uint8_t position,
                           uint8_t pattern)
{
    if (handle == NULL) {
        return SEG_INVALID_PARAM;
    }

    if (!handle->initialized) {
        return SEG_NOT_INITIALIZED;
    }

    uint8_t digitCount = (handle->config.driverType == SEG_DRIVER_GPIO)
                         ? handle->config.config.gpio.digitCount
                         : handle->config.config.ht1621.digitCount;

    if (position >= digitCount) {
        return SEG_INVALID_PARAM;
    }

    handle->displayBuffer[position] = pattern;

    if (handle->config.driverType == SEG_DRIVER_HT1621) {
        Seg_Refresh(handle);
    }

    return SEG_OK;
}

/**
 * @brief   Display an integer value
 */
SegStatus_t Seg_DisplayInt(SegDisplayHandle_t* handle, int32_t value)
{
    if (handle == NULL) {
        return SEG_INVALID_PARAM;
    }

    if (!handle->initialized) {
        return SEG_NOT_INITIALIZED;
    }

    uint8_t digitCount = (handle->config.driverType == SEG_DRIVER_GPIO)
                         ? handle->config.config.gpio.digitCount
                         : handle->config.config.ht1621.digitCount;

    bool negative = (value < 0);
    if (negative) {
        value = -value;
    }

    /* Clear buffer first */
    memset(handle->displayBuffer, SEG_PATTERN_BLANK, digitCount);

    /* Fill digits from right to left */
    int8_t pos = digitCount - 1;

    if (value == 0) {
        handle->displayBuffer[pos] = SEG_PATTERN_0;
    } else {
        while (value > 0 && pos >= 0) {
            handle->displayBuffer[pos] = digitPatterns[value % 10];
            value /= 10;
            pos--;
        }
    }

    /* Add minus sign if negative */
    if (negative && pos >= 0) {
        handle->displayBuffer[pos] = SEG_PATTERN_MINUS;
    }

    /* Fill leading zeros if configured */
    if (handle->config.leadingZeros) {
        int8_t start = negative ? 1 : 0;
        for (int8_t i = start; i < digitCount - 1; i++) {
            if (handle->displayBuffer[i] == SEG_PATTERN_BLANK) {
                handle->displayBuffer[i] = SEG_PATTERN_0;
            }
        }
    }

    if (handle->config.driverType == SEG_DRIVER_HT1621) {
        Seg_Refresh(handle);
    }

    return SEG_OK;
}

/**
 * @brief   Display a floating-point value
 */
SegStatus_t Seg_DisplayFloat(SegDisplayHandle_t* handle, float value,
                             uint8_t decimals)
{
    if (handle == NULL) {
        return SEG_INVALID_PARAM;
    }

    if (!handle->initialized) {
        return SEG_NOT_INITIALIZED;
    }

    uint8_t digitCount = (handle->config.driverType == SEG_DRIVER_GPIO)
                         ? handle->config.config.gpio.digitCount
                         : handle->config.config.ht1621.digitCount;

    if (decimals >= digitCount) {
        decimals = digitCount - 1;
    }

    bool negative = (value < 0);
    if (negative) {
        value = -value;
    }

    /* Scale value to integer */
    int32_t multiplier = 1;
    for (uint8_t i = 0; i < decimals; i++) {
        multiplier *= 10;
    }
    int32_t intValue = (int32_t)(value * multiplier + 0.5f);

    /* Clear buffer first */
    memset(handle->displayBuffer, SEG_PATTERN_BLANK, digitCount);

    /* Fill digits from right to left */
    int8_t pos = digitCount - 1;
    uint8_t decimalPos = digitCount - 1 - decimals;

    for (int8_t i = pos; i >= 0 && (intValue > 0 || i >= decimalPos); i--) {
        handle->displayBuffer[i] = digitPatterns[intValue % 10];
        intValue /= 10;

        /* Add decimal point */
        if (i == decimalPos && decimals > 0) {
            handle->displayBuffer[i] |= SEG_PATTERN_DP;
        }
        pos = i - 1;
    }

    /* Add minus sign if negative */
    if (negative && pos >= 0) {
        handle->displayBuffer[pos] = SEG_PATTERN_MINUS;
    }

    if (handle->config.driverType == SEG_DRIVER_HT1621) {
        Seg_Refresh(handle);
    }

    return SEG_OK;
}

/**
 * @brief   Display hexadecimal value
 */
SegStatus_t Seg_DisplayHex(SegDisplayHandle_t* handle, uint32_t value)
{
    if (handle == NULL) {
        return SEG_INVALID_PARAM;
    }

    if (!handle->initialized) {
        return SEG_NOT_INITIALIZED;
    }

    uint8_t digitCount = (handle->config.driverType == SEG_DRIVER_GPIO)
                         ? handle->config.config.gpio.digitCount
                         : handle->config.config.ht1621.digitCount;

    /* Clear buffer first */
    memset(handle->displayBuffer, SEG_PATTERN_BLANK, digitCount);

    /* Fill digits from right to left */
    int8_t pos = digitCount - 1;

    if (value == 0) {
        handle->displayBuffer[pos] = SEG_PATTERN_0;
    } else {
        while (value > 0 && pos >= 0) {
            handle->displayBuffer[pos] = digitPatterns[value & 0x0F];
            value >>= 4;
            pos--;
        }
    }

    /* Fill leading zeros if configured */
    if (handle->config.leadingZeros) {
        for (int8_t i = 0; i < digitCount - 1; i++) {
            if (handle->displayBuffer[i] == SEG_PATTERN_BLANK) {
                handle->displayBuffer[i] = SEG_PATTERN_0;
            }
        }
    }

    if (handle->config.driverType == SEG_DRIVER_HT1621) {
        Seg_Refresh(handle);
    }

    return SEG_OK;
}

/**
 * @brief   Display a character
 */
SegStatus_t Seg_SetChar(SegDisplayHandle_t* handle, uint8_t position, char ch)
{
    if (handle == NULL) {
        return SEG_INVALID_PARAM;
    }

    if (!handle->initialized) {
        return SEG_NOT_INITIALIZED;
    }

    uint8_t digitCount = (handle->config.driverType == SEG_DRIVER_GPIO)
                         ? handle->config.config.gpio.digitCount
                         : handle->config.config.ht1621.digitCount;

    if (position >= digitCount) {
        return SEG_INVALID_PARAM;
    }

    handle->displayBuffer[position] = Seg_CharToPattern(ch);

    if (handle->config.driverType == SEG_DRIVER_HT1621) {
        Seg_Refresh(handle);
    }

    return SEG_OK;
}

/**
 * @brief   Display a string
 */
SegStatus_t Seg_DisplayString(SegDisplayHandle_t* handle, const char* str)
{
    if (handle == NULL || str == NULL) {
        return SEG_INVALID_PARAM;
    }

    if (!handle->initialized) {
        return SEG_NOT_INITIALIZED;
    }

    uint8_t digitCount = (handle->config.driverType == SEG_DRIVER_GPIO)
                         ? handle->config.config.gpio.digitCount
                         : handle->config.config.ht1621.digitCount;

    /* Clear buffer first */
    memset(handle->displayBuffer, SEG_PATTERN_BLANK, digitCount);

    uint8_t pos = 0;
    while (*str && pos < digitCount) {
        if (*str == '.') {
            /* Add decimal point to previous digit */
            if (pos > 0) {
                handle->displayBuffer[pos - 1] |= SEG_PATTERN_DP;
            }
        } else {
            handle->displayBuffer[pos] = Seg_CharToPattern(*str);
            pos++;
        }
        str++;
    }

    if (handle->config.driverType == SEG_DRIVER_HT1621) {
        Seg_Refresh(handle);
    }

    return SEG_OK;
}

/**
 * @brief   Update display (for GPIO multiplexing)
 */
SegStatus_t Seg_Update(SegDisplayHandle_t* handle)
{
    if (handle == NULL) {
        return SEG_INVALID_PARAM;
    }

    if (!handle->initialized) {
        return SEG_NOT_INITIALIZED;
    }

    if (!handle->enabled) {
        return SEG_OK;
    }

    if (handle->config.driverType != SEG_DRIVER_GPIO) {
        return SEG_OK;  /* Not needed for HT1621 */
    }

    uint8_t digitCount = handle->config.config.gpio.digitCount;

    /* Turn off all digits first */
    Seg_GPIO_DeselectAllDigits(handle);

    /* Write segment pattern for current digit */
    Seg_GPIO_WriteSegments(handle, handle->displayBuffer[handle->currentDigit]);

    /* Select current digit */
    Seg_GPIO_SelectDigit(handle, handle->currentDigit);

    /* Move to next digit */
    handle->currentDigit++;
    if (handle->currentDigit >= digitCount) {
        handle->currentDigit = 0;
    }

    return SEG_OK;
}

/**
 * @brief   Refresh all digits (for HT1621 mode)
 */
SegStatus_t Seg_Refresh(SegDisplayHandle_t* handle)
{
    if (handle == NULL) {
        return SEG_INVALID_PARAM;
    }

    if (!handle->initialized) {
        return SEG_NOT_INITIALIZED;
    }

    if (handle->config.driverType != SEG_DRIVER_HT1621) {
        return SEG_OK;  /* Not needed for GPIO mode */
    }

    uint8_t digitCount = handle->config.config.ht1621.digitCount;

    /* Write all digits to HT1621 */
    for (uint8_t i = 0; i < digitCount; i++) {
        Seg_HT1621_SendData(handle, i * 2, handle->displayBuffer[i]);
    }

    return SEG_OK;
}

/**
 * @brief   Get segment pattern for a digit value
 */
uint8_t Seg_GetPattern(uint8_t value)
{
    if (value > 15) {
        return SEG_PATTERN_BLANK;
    }
    return digitPatterns[value];
}

/**
 * @brief   Get segment pattern for a character
 */
uint8_t Seg_CharToPattern(char ch)
{
    if (ch >= '0' && ch <= '9') {
        return digitPatterns[ch - '0'];
    }

    switch (ch) {
        case 'A': case 'a': return SEG_PATTERN_A;
        case 'B': case 'b': return SEG_PATTERN_B;
        case 'C': case 'c': return SEG_PATTERN_C;
        case 'D': case 'd': return SEG_PATTERN_D;
        case 'E': case 'e': return SEG_PATTERN_E;
        case 'F': case 'f': return SEG_PATTERN_F;
        case 'H': case 'h': return 0x76;  /* H: B,C,E,F,G */
        case 'L': case 'l': return 0x38;  /* L: D,E,F */
        case 'P': case 'p': return 0x73;  /* P: A,B,E,F,G */
        case 'U': case 'u': return 0x3E;  /* U: B,C,D,E,F */
        case 'O': case 'o': return SEG_PATTERN_0;
        case 'S': case 's': return SEG_PATTERN_5;
        case 'I': case 'i': return SEG_PATTERN_1;
        case 'G': case 'g': return SEG_PATTERN_9;
        case 'n':           return 0x54;  /* n: C,E,G */
        case 'r':           return 0x50;  /* r: E,G */
        case 't':           return 0x78;  /* t: D,E,F,G */
        case 'y':           return 0x6E;  /* y: B,C,D,F,G */
        case '-':           return SEG_PATTERN_MINUS;
        case '_':           return 0x08;  /* _: D */
        case ' ':           return SEG_PATTERN_BLANK;
        case '.':           return SEG_PATTERN_DP;
        default:            return SEG_PATTERN_BLANK;
    }
}

/**
 * @brief   Test all segments
 */
SegStatus_t Seg_Test(SegDisplayHandle_t* handle)
{
    if (handle == NULL) {
        return SEG_INVALID_PARAM;
    }

    if (!handle->initialized) {
        return SEG_NOT_INITIALIZED;
    }

    uint8_t digitCount = (handle->config.driverType == SEG_DRIVER_GPIO)
                         ? handle->config.config.gpio.digitCount
                         : handle->config.config.ht1621.digitCount;

    /* Display 8. on all digits */
    memset(handle->displayBuffer, SEG_PATTERN_8 | SEG_PATTERN_DP, digitCount);

    if (handle->config.driverType == SEG_DRIVER_HT1621) {
        Seg_Refresh(handle);
    }

    return SEG_OK;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Initialize GPIO pins for direct GPIO mode
 */
static void Seg_GPIO_Init(SegDisplayHandle_t* handle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    SegGpioConfig_t* gpioConfig = &handle->config.config.gpio;

    /* Configure segment pins as outputs */
    for (uint8_t i = 0; i < SEG_COUNT; i++) {
        if (gpioConfig->segments[i].port != NULL) {
            GPIO_InitStruct.Pin = gpioConfig->segments[i].pin;
            GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
            HAL_GPIO_Init(gpioConfig->segments[i].port, &GPIO_InitStruct);
        }
    }

    /* Configure digit select pins as outputs */
    for (uint8_t i = 0; i < gpioConfig->digitCount; i++) {
        if (gpioConfig->digits != NULL && gpioConfig->digits[i].port != NULL) {
            GPIO_InitStruct.Pin = gpioConfig->digits[i].pin;
            GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
            HAL_GPIO_Init(gpioConfig->digits[i].port, &GPIO_InitStruct);
        }
    }

    /* Initialize all segments OFF and all digits deselected */
    Seg_GPIO_DeselectAllDigits(handle);
    Seg_GPIO_WriteSegments(handle, SEG_PATTERN_BLANK);
}

/**
 * @brief   Write segment pattern to GPIO pins
 */
static void Seg_GPIO_WriteSegments(SegDisplayHandle_t* handle, uint8_t pattern)
{
    SegGpioConfig_t* gpioConfig = &handle->config.config.gpio;
    bool invert = (gpioConfig->polarity == SEG_COMMON_ANODE);

    for (uint8_t i = 0; i < SEG_COUNT; i++) {
        if (gpioConfig->segments[i].port != NULL) {
            bool segmentOn = (pattern >> i) & 0x01;
            if (invert) {
                segmentOn = !segmentOn;
            }
            HAL_GPIO_WritePin(gpioConfig->segments[i].port,
                            gpioConfig->segments[i].pin,
                            segmentOn ? GPIO_PIN_SET : GPIO_PIN_RESET);
        }
    }
}

/**
 * @brief   Select a digit for multiplexing
 */
static void Seg_GPIO_SelectDigit(SegDisplayHandle_t* handle, uint8_t digit)
{
    SegGpioConfig_t* gpioConfig = &handle->config.config.gpio;

    if (gpioConfig->digits == NULL || digit >= gpioConfig->digitCount) {
        return;
    }

    GPIO_PinState activeState = gpioConfig->digitActiveHigh ? GPIO_PIN_SET : GPIO_PIN_RESET;

    HAL_GPIO_WritePin(gpioConfig->digits[digit].port,
                      gpioConfig->digits[digit].pin,
                      activeState);
}

/**
 * @brief   Deselect all digits
 */
static void Seg_GPIO_DeselectAllDigits(SegDisplayHandle_t* handle)
{
    SegGpioConfig_t* gpioConfig = &handle->config.config.gpio;

    if (gpioConfig->digits == NULL) {
        return;
    }

    GPIO_PinState inactiveState = gpioConfig->digitActiveHigh ? GPIO_PIN_RESET : GPIO_PIN_SET;

    for (uint8_t i = 0; i < gpioConfig->digitCount; i++) {
        if (gpioConfig->digits[i].port != NULL) {
            HAL_GPIO_WritePin(gpioConfig->digits[i].port,
                            gpioConfig->digits[i].pin,
                            inactiveState);
        }
    }
}

/**
 * @brief   Initialize HT1621 LCD driver
 */
static void Seg_HT1621_Init(SegDisplayHandle_t* handle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    SegHT1621Config_t* ht1621Config = &handle->config.config.ht1621;

    /* Configure CS pin */
    GPIO_InitStruct.Pin = ht1621Config->pins.csPin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(ht1621Config->pins.csPort, &GPIO_InitStruct);

    /* Configure WR pin */
    GPIO_InitStruct.Pin = ht1621Config->pins.wrPin;
    HAL_GPIO_Init(ht1621Config->pins.wrPort, &GPIO_InitStruct);

    /* Configure DATA pin */
    GPIO_InitStruct.Pin = ht1621Config->pins.dataPin;
    HAL_GPIO_Init(ht1621Config->pins.dataPort, &GPIO_InitStruct);

    /* Set initial pin states */
    HAL_GPIO_WritePin(ht1621Config->pins.csPort, ht1621Config->pins.csPin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ht1621Config->pins.wrPort, ht1621Config->pins.wrPin, GPIO_PIN_SET);

    /* Initialize HT1621 */
    Seg_HT1621_SendCommand(handle, HT1621_CMD_SYS_EN);      /* Enable system */
    Seg_HT1621_SendCommand(handle, HT1621_CMD_RC_256K);     /* Use internal RC oscillator */

    /* Set bias based on configuration */
    uint8_t biasCmd = HT1621_CMD_BIAS_3_4;  /* Default: 1/3 bias, 4 commons */
    if (ht1621Config->bias == 2) {
        if (ht1621Config->commons == 2) biasCmd = HT1621_CMD_BIAS_2_2;
        else if (ht1621Config->commons == 3) biasCmd = HT1621_CMD_BIAS_2_3;
        else biasCmd = HT1621_CMD_BIAS_2_4;
    } else {
        if (ht1621Config->commons == 2) biasCmd = HT1621_CMD_BIAS_3_2;
        else if (ht1621Config->commons == 3) biasCmd = HT1621_CMD_BIAS_3_3;
        else biasCmd = HT1621_CMD_BIAS_3_4;
    }
    Seg_HT1621_SendCommand(handle, biasCmd);

    Seg_HT1621_SendCommand(handle, HT1621_CMD_LCD_ON);      /* Turn on LCD */

    /* Clear display */
    for (uint8_t i = 0; i < 32; i++) {
        Seg_HT1621_SendData(handle, i, 0x00);
    }
}

/**
 * @brief   Send command to HT1621
 */
static void Seg_HT1621_SendCommand(SegDisplayHandle_t* handle, uint8_t cmd)
{
    SegHT1621Pins_t* pins = &handle->config.config.ht1621.pins;

    /* CS low */
    HAL_GPIO_WritePin(pins->csPort, pins->csPin, GPIO_PIN_RESET);

    /* Send command mode: 100 */
    Seg_HT1621_WriteBit(handle, true);
    Seg_HT1621_WriteBit(handle, false);
    Seg_HT1621_WriteBit(handle, false);

    /* Send command (8 bits) */
    for (int8_t i = 7; i >= 0; i--) {
        Seg_HT1621_WriteBit(handle, (cmd >> i) & 0x01);
    }

    /* One extra bit */
    Seg_HT1621_WriteBit(handle, false);

    /* CS high */
    HAL_GPIO_WritePin(pins->csPort, pins->csPin, GPIO_PIN_SET);
}

/**
 * @brief   Send data to HT1621 at specified address
 */
static void Seg_HT1621_SendData(SegDisplayHandle_t* handle, uint8_t addr, uint8_t data)
{
    SegHT1621Pins_t* pins = &handle->config.config.ht1621.pins;

    /* CS low */
    HAL_GPIO_WritePin(pins->csPort, pins->csPin, GPIO_PIN_RESET);

    /* Send write mode: 101 */
    Seg_HT1621_WriteBit(handle, true);
    Seg_HT1621_WriteBit(handle, false);
    Seg_HT1621_WriteBit(handle, true);

    /* Send address (6 bits) */
    for (int8_t i = 5; i >= 0; i--) {
        Seg_HT1621_WriteBit(handle, (addr >> i) & 0x01);
    }

    /* Send data (4 bits) - LSB first for HT1621 */
    for (int8_t i = 0; i < 4; i++) {
        Seg_HT1621_WriteBit(handle, (data >> i) & 0x01);
    }

    /* CS high */
    HAL_GPIO_WritePin(pins->csPort, pins->csPin, GPIO_PIN_SET);
}

/**
 * @brief   Write a single bit to HT1621
 */
static void Seg_HT1621_WriteBit(SegDisplayHandle_t* handle, bool bit)
{
    SegHT1621Pins_t* pins = &handle->config.config.ht1621.pins;

    /* WR low */
    HAL_GPIO_WritePin(pins->wrPort, pins->wrPin, GPIO_PIN_RESET);

    /* Set data */
    HAL_GPIO_WritePin(pins->dataPort, pins->dataPin,
                      bit ? GPIO_PIN_SET : GPIO_PIN_RESET);

    Seg_DelayUs(2);

    /* WR high - data is latched on rising edge */
    HAL_GPIO_WritePin(pins->wrPort, pins->wrPin, GPIO_PIN_SET);

    Seg_DelayUs(2);
}

/**
 * @brief   Microsecond delay (simple busy-wait)
 */
static void Seg_DelayUs(uint32_t us)
{
    /* Rough delay - adjust based on clock frequency */
    volatile uint32_t count = us * 42;  /* For 168MHz clock */
    while (count--) {
        __NOP();
    }
}
