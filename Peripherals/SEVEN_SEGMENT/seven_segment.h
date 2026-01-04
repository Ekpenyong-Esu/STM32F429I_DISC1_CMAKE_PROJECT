/**
 * @file    seven_segment.h
 * @brief   Seven-Segment Display Driver for STM32F429
 * @details Supports both HT1621 LCD driver and direct GPIO control
 *          with options for common cathode and common anode configurations
 * @version 1.0
 * @date    2026-01-03
 */

#ifndef SEVEN_SEGMENT_H
#define SEVEN_SEGMENT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdbool.h>
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Seven-segment display driver type
 */
typedef enum {
    SEG_DRIVER_GPIO = 0,    /**< Direct GPIO control (no driver IC) */
    SEG_DRIVER_HT1621       /**< HT1621 LCD driver IC */
} SegDriverType_t;

/**
 * @brief Seven-segment display polarity (for GPIO mode)
 */
typedef enum {
    SEG_COMMON_CATHODE = 0, /**< Common cathode - segments are active HIGH */
    SEG_COMMON_ANODE        /**< Common anode - segments are active LOW */
} SegPolarity_t;

/**
 * @brief Seven-segment operation status
 */
typedef enum {
    SEG_OK = 0,             /**< Operation successful */
    SEG_ERROR,              /**< General error */
    SEG_INVALID_PARAM,      /**< Invalid parameter */
    SEG_NOT_INITIALIZED,    /**< Driver not initialized */
    SEG_BUSY                /**< Driver busy */
} SegStatus_t;

/**
 * @brief Segment identifiers (standard 7-segment + decimal point)
 *
 *      --A--
 *     |     |
 *     F     B
 *     |     |
 *      --G--
 *     |     |
 *     E     C
 *     |     |
 *      --D--  .DP
 */
typedef enum {
    SEG_A = 0,              /**< Top segment */
    SEG_B,                  /**< Top-right segment */
    SEG_C,                  /**< Bottom-right segment */
    SEG_D,                  /**< Bottom segment */
    SEG_E,                  /**< Bottom-left segment */
    SEG_F,                  /**< Top-left segment */
    SEG_G,                  /**< Middle segment */
    SEG_DP,                 /**< Decimal point */
    SEG_COUNT               /**< Total segment count */
} SegmentId_t;

/**
 * @brief GPIO pin configuration for a single segment
 */
typedef struct {
    GPIO_TypeDef* port;     /**< GPIO port */
    uint16_t pin;           /**< GPIO pin */
} SegGpioPin_t;

/**
 * @brief GPIO configuration for direct GPIO mode
 */
typedef struct {
    SegGpioPin_t segments[SEG_COUNT];   /**< Segment GPIO pins (A-G + DP) */
    SegGpioPin_t* digits;               /**< Digit select pins (array) */
    uint8_t digitCount;                 /**< Number of digits */
    SegPolarity_t polarity;             /**< Common cathode or anode */
    bool digitActiveHigh;               /**< Digit select active level */
} SegGpioConfig_t;

/**
 * @brief HT1621 pin configuration
 */
typedef struct {
    GPIO_TypeDef* csPort;   /**< Chip select port */
    uint16_t csPin;         /**< Chip select pin */
    GPIO_TypeDef* wrPort;   /**< Write clock port */
    uint16_t wrPin;         /**< Write clock pin */
    GPIO_TypeDef* dataPort; /**< Data port */
    uint16_t dataPin;       /**< Data pin */
} SegHT1621Pins_t;

/**
 * @brief HT1621 configuration
 */
typedef struct {
    SegHT1621Pins_t pins;           /**< HT1621 GPIO pins */
    uint8_t digitCount;             /**< Number of digits */
    uint8_t* segmentMap;            /**< Custom segment mapping (optional) */
    uint8_t bias;                   /**< LCD bias (2, 3, or 4) */
    uint8_t commons;                /**< Number of commons (2, 3, or 4) */
} SegHT1621Config_t;

/**
 * @brief Seven-segment display configuration
 */
typedef struct {
    SegDriverType_t driverType;     /**< Driver type selection */
    union {
        SegGpioConfig_t gpio;       /**< GPIO mode configuration */
        SegHT1621Config_t ht1621;   /**< HT1621 mode configuration */
    } config;
    uint16_t multiplexDelayUs;      /**< Multiplex delay (GPIO mode only) */
    bool leadingZeros;              /**< Display leading zeros */
} SegDisplayConfig_t;

/**
 * @brief Seven-segment display handle
 */
typedef struct {
    SegDisplayConfig_t config;      /**< Display configuration */
    uint8_t* displayBuffer;         /**< Display buffer (one byte per digit) */
    uint8_t currentDigit;           /**< Current digit for multiplexing */
    bool initialized;               /**< Initialization flag */
    bool enabled;                   /**< Display enabled flag */
    uint8_t brightness;             /**< Brightness level (0-100) for HT1621 */
} SegDisplayHandle_t;

/* Exported constants --------------------------------------------------------*/

/** @defgroup SEG_Patterns Segment Patterns for digits 0-9 and characters
 * @{
 */
#define SEG_PATTERN_0       0x3F    /**< 0: A,B,C,D,E,F */
#define SEG_PATTERN_1       0x06    /**< 1: B,C */
#define SEG_PATTERN_2       0x5B    /**< 2: A,B,D,E,G */
#define SEG_PATTERN_3       0x4F    /**< 3: A,B,C,D,G */
#define SEG_PATTERN_4       0x66    /**< 4: B,C,F,G */
#define SEG_PATTERN_5       0x6D    /**< 5: A,C,D,F,G */
#define SEG_PATTERN_6       0x7D    /**< 6: A,C,D,E,F,G */
#define SEG_PATTERN_7       0x07    /**< 7: A,B,C */
#define SEG_PATTERN_8       0x7F    /**< 8: A,B,C,D,E,F,G */
#define SEG_PATTERN_9       0x6F    /**< 9: A,B,C,D,F,G */
#define SEG_PATTERN_A       0x77    /**< A: A,B,C,E,F,G */
#define SEG_PATTERN_B       0x7C    /**< b: C,D,E,F,G */
#define SEG_PATTERN_C       0x39    /**< C: A,D,E,F */
#define SEG_PATTERN_D       0x5E    /**< d: B,C,D,E,G */
#define SEG_PATTERN_E       0x79    /**< E: A,D,E,F,G */
#define SEG_PATTERN_F       0x71    /**< F: A,E,F,G */
#define SEG_PATTERN_MINUS   0x40    /**< -: G */
#define SEG_PATTERN_BLANK   0x00    /**< Blank */
#define SEG_PATTERN_DP      0x80    /**< Decimal point only */

/** @} */

/** @defgroup HT1621_Commands HT1621 Command Definitions
 * @{
 */
#define HT1621_CMD_SYS_DIS      0x00    /**< System disable */
#define HT1621_CMD_SYS_EN       0x01    /**< System enable */
#define HT1621_CMD_LCD_OFF      0x02    /**< LCD off */
#define HT1621_CMD_LCD_ON       0x03    /**< LCD on */
#define HT1621_CMD_TIMER_DIS    0x04    /**< Timer disable */
#define HT1621_CMD_WDT_DIS      0x05    /**< WDT disable */
#define HT1621_CMD_TIMER_EN     0x06    /**< Timer enable */
#define HT1621_CMD_WDT_EN       0x07    /**< WDT enable */
#define HT1621_CMD_RC_256K      0x18    /**< RC 256K oscillator */
#define HT1621_CMD_EXT_256K     0x1C    /**< External 256K oscillator */
#define HT1621_CMD_BIAS_2_2     0x20    /**< 1/2 bias, 2 commons */
#define HT1621_CMD_BIAS_2_3     0x24    /**< 1/2 bias, 3 commons */
#define HT1621_CMD_BIAS_2_4     0x28    /**< 1/2 bias, 4 commons */
#define HT1621_CMD_BIAS_3_2     0x21    /**< 1/3 bias, 2 commons */
#define HT1621_CMD_BIAS_3_3     0x25    /**< 1/3 bias, 3 commons */
#define HT1621_CMD_BIAS_3_4     0x29    /**< 1/3 bias, 4 commons */
#define HT1621_CMD_TONE_OFF     0x08    /**< Tone off */
#define HT1621_CMD_TONE_ON      0x09    /**< Tone on */

/** @} */

/* Default configurations */
#define SEG_DEFAULT_MULTIPLEX_DELAY_US  2000    /**< 2ms default multiplex delay */
#define SEG_MAX_DIGITS                  8       /**< Maximum supported digits */

/* Exported functions --------------------------------------------------------*/

/** @defgroup SEG_Init Initialization Functions
 * @{
 */

/**
 * @brief   Initialize seven-segment display
 * @param   handle Pointer to display handle
 * @param   config Pointer to configuration structure
 * @retval  SegStatus_t Operation status
 */
SegStatus_t Seg_Init(SegDisplayHandle_t* handle, const SegDisplayConfig_t* config);

/**
 * @brief   Deinitialize seven-segment display
 * @param   handle Pointer to display handle
 * @retval  SegStatus_t Operation status
 */
SegStatus_t Seg_DeInit(SegDisplayHandle_t* handle);

/** @} */

/** @defgroup SEG_Control Control Functions
 * @{
 */

/**
 * @brief   Enable display output
 * @param   handle Pointer to display handle
 * @retval  SegStatus_t Operation status
 */
SegStatus_t Seg_Enable(SegDisplayHandle_t* handle);

/**
 * @brief   Disable display output
 * @param   handle Pointer to display handle
 * @retval  SegStatus_t Operation status
 */
SegStatus_t Seg_Disable(SegDisplayHandle_t* handle);

/**
 * @brief   Clear all digits
 * @param   handle Pointer to display handle
 * @retval  SegStatus_t Operation status
 */
SegStatus_t Seg_Clear(SegDisplayHandle_t* handle);

/**
 * @brief   Set display brightness (HT1621 only)
 * @param   handle Pointer to display handle
 * @param   brightness Brightness level (0-100)
 * @retval  SegStatus_t Operation status
 */
SegStatus_t Seg_SetBrightness(SegDisplayHandle_t* handle, uint8_t brightness);

/** @} */

/** @defgroup SEG_Display Display Functions
 * @{
 */

/**
 * @brief   Display a single digit value
 * @param   handle Pointer to display handle
 * @param   position Digit position (0 = leftmost)
 * @param   value Value to display (0-15 for hex, 0-9 for decimal)
 * @param   showDp Show decimal point
 * @retval  SegStatus_t Operation status
 */
SegStatus_t Seg_SetDigit(SegDisplayHandle_t* handle, uint8_t position,
                         uint8_t value, bool showDp);

/**
 * @brief   Display raw segment pattern
 * @param   handle Pointer to display handle
 * @param   position Digit position
 * @param   pattern Raw segment pattern (bits 0-6 = A-G, bit 7 = DP)
 * @retval  SegStatus_t Operation status
 */
SegStatus_t Seg_SetPattern(SegDisplayHandle_t* handle, uint8_t position,
                           uint8_t pattern);

/**
 * @brief   Display an integer value
 * @param   handle Pointer to display handle
 * @param   value Integer value to display
 * @retval  SegStatus_t Operation status
 */
SegStatus_t Seg_DisplayInt(SegDisplayHandle_t* handle, int32_t value);

/**
 * @brief   Display a floating-point value
 * @param   handle Pointer to display handle
 * @param   value Float value to display
 * @param   decimals Number of decimal places
 * @retval  SegStatus_t Operation status
 */
SegStatus_t Seg_DisplayFloat(SegDisplayHandle_t* handle, float value,
                             uint8_t decimals);

/**
 * @brief   Display hexadecimal value
 * @param   handle Pointer to display handle
 * @param   value Hex value to display
 * @retval  SegStatus_t Operation status
 */
SegStatus_t Seg_DisplayHex(SegDisplayHandle_t* handle, uint32_t value);

/**
 * @brief   Display a character
 * @param   handle Pointer to display handle
 * @param   position Digit position
 * @param   ch Character to display (0-9, A-F, -, space)
 * @retval  SegStatus_t Operation status
 */
SegStatus_t Seg_SetChar(SegDisplayHandle_t* handle, uint8_t position, char ch);

/**
 * @brief   Display a string
 * @param   handle Pointer to display handle
 * @param   str String to display
 * @retval  SegStatus_t Operation status
 */
SegStatus_t Seg_DisplayString(SegDisplayHandle_t* handle, const char* str);

/** @} */

/** @defgroup SEG_Multiplex Multiplexing Functions (GPIO mode)
 * @{
 */

/**
 * @brief   Update display (call periodically for GPIO multiplexing)
 * @param   handle Pointer to display handle
 * @retval  SegStatus_t Operation status
 * @note    For GPIO mode, call this in a timer interrupt or main loop
 *          at a rate >= 100Hz * number of digits for flicker-free display
 */
SegStatus_t Seg_Update(SegDisplayHandle_t* handle);

/**
 * @brief   Refresh all digits (for HT1621 mode)
 * @param   handle Pointer to display handle
 * @retval  SegStatus_t Operation status
 */
SegStatus_t Seg_Refresh(SegDisplayHandle_t* handle);

/** @} */

/** @defgroup SEG_Utility Utility Functions
 * @{
 */

/**
 * @brief   Get segment pattern for a digit value
 * @param   value Digit value (0-15)
 * @retval  uint8_t Segment pattern
 */
uint8_t Seg_GetPattern(uint8_t value);

/**
 * @brief   Get segment pattern for a character
 * @param   ch Character
 * @retval  uint8_t Segment pattern
 */
uint8_t Seg_CharToPattern(char ch);

/**
 * @brief   Test all segments (display 8 with DP on all digits)
 * @param   handle Pointer to display handle
 * @retval  SegStatus_t Operation status
 */
SegStatus_t Seg_Test(SegDisplayHandle_t* handle);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* SEVEN_SEGMENT_H */
