/**
 * @file    lcd.h
 * @brief   Character LCD Driver for STM32F429 (HD44780 compatible)
 * @details This file contains all the function prototypes for
 *          character LCD display control. Supports 16x2, 20x4, and
 *          other HD44780-based displays in 4-bit or 8-bit mode.
 * @version 1.0
 * @date    2026-01-03
 */

#ifndef __LCD_H__
#define __LCD_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief LCD Status enumeration
 */
typedef enum {
    LCD_OK = 0,                 /**< Operation completed successfully */
    LCD_ERROR,                  /**< General error occurred */
    LCD_BUSY,                   /**< LCD is busy */
    LCD_TIMEOUT,                /**< Operation timed out */
    LCD_INVALID_PARAM,          /**< Invalid parameter provided */
    LCD_NOT_INITIALIZED         /**< Driver not initialized */
} LCD_StatusTypeDef;

/**
 * @brief LCD interface mode
 */
typedef enum {
    LCD_MODE_4BIT = 0,          /**< 4-bit data interface */
    LCD_MODE_8BIT               /**< 8-bit data interface */
} LCD_ModeTypeDef;

/**
 * @brief LCD size type
 */
typedef enum {
    LCD_SIZE_16x2 = 0,          /**< 16 columns x 2 rows */
    LCD_SIZE_20x4,              /**< 20 columns x 4 rows */
    LCD_SIZE_16x4,              /**< 16 columns x 4 rows */
    LCD_SIZE_20x2,              /**< 20 columns x 2 rows */
    LCD_SIZE_24x2,              /**< 24 columns x 2 rows */
    LCD_SIZE_8x2,               /**< 8 columns x 2 rows */
    LCD_SIZE_40x2,              /**< 40 columns x 2 rows */
    LCD_SIZE_CUSTOM             /**< Custom size */
} LCD_SizeTypeDef;

/**
 * @brief LCD GPIO pin configuration
 */
typedef struct {
    GPIO_TypeDef* port;         /**< GPIO port */
    uint16_t pin;               /**< GPIO pin */
} LCD_PinTypeDef;

/**
 * @brief LCD pin configuration structure
 */
typedef struct {
    LCD_PinTypeDef rs;          /**< Register Select pin */
    LCD_PinTypeDef rw;          /**< Read/Write pin (optional, can be tied to GND) */
    LCD_PinTypeDef en;          /**< Enable pin */
    LCD_PinTypeDef d0;          /**< Data pin 0 (8-bit mode only) */
    LCD_PinTypeDef d1;          /**< Data pin 1 (8-bit mode only) */
    LCD_PinTypeDef d2;          /**< Data pin 2 (8-bit mode only) */
    LCD_PinTypeDef d3;          /**< Data pin 3 (8-bit mode only) */
    LCD_PinTypeDef d4;          /**< Data pin 4 */
    LCD_PinTypeDef d5;          /**< Data pin 5 */
    LCD_PinTypeDef d6;          /**< Data pin 6 */
    LCD_PinTypeDef d7;          /**< Data pin 7 */
    LCD_PinTypeDef backlight;   /**< Backlight control pin (optional) */
} LCD_PinsTypeDef;

/**
 * @brief LCD configuration structure
 */
typedef struct {
    LCD_PinsTypeDef pins;       /**< GPIO pin assignments */
    LCD_ModeTypeDef mode;       /**< 4-bit or 8-bit mode */
    LCD_SizeTypeDef size;       /**< Display size preset */
    uint8_t cols;               /**< Number of columns (for custom size) */
    uint8_t rows;               /**< Number of rows (for custom size) */
    bool useRW;                 /**< Use R/W pin (false = tied to GND) */
    bool useBacklight;          /**< Use backlight control pin */
} LCD_ConfigTypeDef;

/**
 * @brief LCD handle structure
 */
typedef struct {
    LCD_ConfigTypeDef config;   /**< LCD configuration */
    uint8_t cols;               /**< Number of columns */
    uint8_t rows;               /**< Number of rows */
    uint8_t cursorCol;          /**< Current cursor column */
    uint8_t cursorRow;          /**< Current cursor row */
    bool displayOn;             /**< Display on/off state */
    bool cursorOn;              /**< Cursor on/off state */
    bool blinkOn;               /**< Cursor blink on/off state */
    bool backlightOn;           /**< Backlight on/off state */
    bool initialized;           /**< Initialization flag */
} LCD_HandleTypeDef;

/* Exported constants --------------------------------------------------------*/

/** @defgroup LCD_Commands HD44780 Command Definitions
 * @{
 */
#define LCD_CMD_CLEAR_DISPLAY       0x01U   /**< Clear display */
#define LCD_CMD_RETURN_HOME         0x02U   /**< Return cursor to home */
#define LCD_CMD_ENTRY_MODE_SET      0x04U   /**< Entry mode set */
#define LCD_CMD_DISPLAY_CONTROL     0x08U   /**< Display on/off control */
#define LCD_CMD_CURSOR_SHIFT        0x10U   /**< Cursor/display shift */
#define LCD_CMD_FUNCTION_SET        0x20U   /**< Function set */
#define LCD_CMD_SET_CGRAM_ADDR      0x40U   /**< Set CGRAM address */
#define LCD_CMD_SET_DDRAM_ADDR      0x80U   /**< Set DDRAM address */

/** @} */

/** @defgroup LCD_EntryMode Entry Mode Flags
 * @{
 */
#define LCD_ENTRY_RIGHT             0x00U   /**< Decrement cursor position */
#define LCD_ENTRY_LEFT              0x02U   /**< Increment cursor position */
#define LCD_ENTRY_SHIFT_ON          0x01U   /**< Shift display on write */
#define LCD_ENTRY_SHIFT_OFF         0x00U   /**< No display shift */

/** @} */

/** @defgroup LCD_DisplayControl Display Control Flags
 * @{
 */
#define LCD_DISPLAY_ON              0x04U   /**< Display on */
#define LCD_DISPLAY_OFF             0x00U   /**< Display off */
#define LCD_CURSOR_ON               0x02U   /**< Cursor on */
#define LCD_CURSOR_OFF              0x00U   /**< Cursor off */
#define LCD_BLINK_ON                0x01U   /**< Cursor blink on */
#define LCD_BLINK_OFF               0x00U   /**< Cursor blink off */

/** @} */

/** @defgroup LCD_FunctionSet Function Set Flags
 * @{
 */
#define LCD_8BIT_MODE               0x10U   /**< 8-bit data interface */
#define LCD_4BIT_MODE               0x00U   /**< 4-bit data interface */
#define LCD_2LINE                   0x08U   /**< 2-line display */
#define LCD_1LINE                   0x00U   /**< 1-line display */
#define LCD_5x10_DOTS               0x04U   /**< 5x10 dot character font */
#define LCD_5x8_DOTS                0x00U   /**< 5x8 dot character font */

/** @} */

/** @defgroup LCD_DDRAM_Addresses DDRAM Row Addresses
 * @{
 */
#define LCD_ROW0_ADDR               0x00U   /**< Row 0 start address */
#define LCD_ROW1_ADDR               0x40U   /**< Row 1 start address */
#define LCD_ROW2_ADDR               0x14U   /**< Row 2 start address (20x4) */
#define LCD_ROW3_ADDR               0x54U   /**< Row 3 start address (20x4) */

/** @} */

/** @defgroup LCD_Timing Timing Constants
 * @{
 */
#define LCD_ENABLE_PULSE_US         1U      /**< Enable pulse width (us) */
#define LCD_COMMAND_DELAY_US        50U     /**< Command execution delay (us) */
#define LCD_CLEAR_DELAY_MS          2U      /**< Clear/home command delay (ms) */
#define LCD_INIT_DELAY_MS           50U     /**< Power-on initialization delay (ms) */

/** @} */

/* Exported functions prototypes ---------------------------------------------*/

/** @defgroup LCD_Init Initialization Functions
 * @{
 */

/**
 * @brief   Initialize LCD with configuration
 * @param   handle Pointer to LCD handle
 * @param   config Pointer to configuration structure
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_Init(LCD_HandleTypeDef* handle, const LCD_ConfigTypeDef* config);

/**
 * @brief   Initialize LCD with default 16x2, 4-bit configuration
 * @param   handle Pointer to LCD handle
 * @param   pins Pointer to pin configuration
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_InitDefault(LCD_HandleTypeDef* handle, const LCD_PinsTypeDef* pins);

/**
 * @brief   Deinitialize LCD
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_DeInit(LCD_HandleTypeDef* handle);

/** @} */

/** @defgroup LCD_Basic Basic Display Functions
 * @{
 */

/**
 * @brief   Clear entire display
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_Clear(LCD_HandleTypeDef* handle);

/**
 * @brief   Return cursor to home position (0,0)
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_Home(LCD_HandleTypeDef* handle);

/**
 * @brief   Turn display on
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_DisplayOn(LCD_HandleTypeDef* handle);

/**
 * @brief   Turn display off
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_DisplayOff(LCD_HandleTypeDef* handle);

/**
 * @brief   Turn backlight on
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_BacklightOn(LCD_HandleTypeDef* handle);

/**
 * @brief   Turn backlight off
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_BacklightOff(LCD_HandleTypeDef* handle);

/** @} */

/** @defgroup LCD_Cursor Cursor Functions
 * @{
 */

/**
 * @brief   Set cursor position
 * @param   handle Pointer to LCD handle
 * @param   col Column (0-based)
 * @param   row Row (0-based)
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_SetCursor(LCD_HandleTypeDef* handle, uint8_t col, uint8_t row);

/**
 * @brief   Show cursor
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_CursorOn(LCD_HandleTypeDef* handle);

/**
 * @brief   Hide cursor
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_CursorOff(LCD_HandleTypeDef* handle);

/**
 * @brief   Enable cursor blinking
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_BlinkOn(LCD_HandleTypeDef* handle);

/**
 * @brief   Disable cursor blinking
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_BlinkOff(LCD_HandleTypeDef* handle);

/**
 * @brief   Move cursor left
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_CursorLeft(LCD_HandleTypeDef* handle);

/**
 * @brief   Move cursor right
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_CursorRight(LCD_HandleTypeDef* handle);

/** @} */

/** @defgroup LCD_Print Print Functions
 * @{
 */

/**
 * @brief   Print single character at current cursor position
 * @param   handle Pointer to LCD handle
 * @param   ch Character to print
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_PrintChar(LCD_HandleTypeDef* handle, char ch);

/**
 * @brief   Print string at current cursor position
 * @param   handle Pointer to LCD handle
 * @param   str Null-terminated string to print
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_PrintString(LCD_HandleTypeDef* handle, const char* str);

/**
 * @brief   Print string at specified position
 * @param   handle Pointer to LCD handle
 * @param   col Column position
 * @param   row Row position
 * @param   str Null-terminated string to print
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_PrintStringAt(LCD_HandleTypeDef* handle, uint8_t col,
                                     uint8_t row, const char* str);

/**
 * @brief   Print integer value
 * @param   handle Pointer to LCD handle
 * @param   value Integer value to print
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_PrintInt(LCD_HandleTypeDef* handle, int32_t value);

/**
 * @brief   Print floating-point value
 * @param   handle Pointer to LCD handle
 * @param   value Float value to print
 * @param   decimals Number of decimal places
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_PrintFloat(LCD_HandleTypeDef* handle, float value, uint8_t decimals);

/**
 * @brief   Print hexadecimal value
 * @param   handle Pointer to LCD handle
 * @param   value Value to print
 * @param   digits Number of hex digits
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_PrintHex(LCD_HandleTypeDef* handle, uint32_t value, uint8_t digits);

/**
 * @brief   Print formatted string (printf-style)
 * @param   handle Pointer to LCD handle
 * @param   format Format string
 * @param   ... Variable arguments
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_Printf(LCD_HandleTypeDef* handle, const char* format, ...);

/**
 * @brief   Clear a specific line
 * @param   handle Pointer to LCD handle
 * @param   row Row to clear
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_ClearLine(LCD_HandleTypeDef* handle, uint8_t row);

/** @} */

/** @defgroup LCD_Scroll Scroll Functions
 * @{
 */

/**
 * @brief   Scroll display left
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_ScrollLeft(LCD_HandleTypeDef* handle);

/**
 * @brief   Scroll display right
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_ScrollRight(LCD_HandleTypeDef* handle);

/**
 * @brief   Enable auto-scroll mode
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_AutoScrollOn(LCD_HandleTypeDef* handle);

/**
 * @brief   Disable auto-scroll mode
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_AutoScrollOff(LCD_HandleTypeDef* handle);

/** @} */

/** @defgroup LCD_Custom Custom Character Functions
 * @{
 */

/**
 * @brief   Create custom character
 * @param   handle Pointer to LCD handle
 * @param   location Character location (0-7)
 * @param   charmap Pointer to 8-byte character pattern
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_CreateChar(LCD_HandleTypeDef* handle, uint8_t location,
                                  const uint8_t* charmap);

/**
 * @brief   Print custom character
 * @param   handle Pointer to LCD handle
 * @param   location Character location (0-7)
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_PrintCustomChar(LCD_HandleTypeDef* handle, uint8_t location);

/** @} */

/** @defgroup LCD_LowLevel Low-Level Functions
 * @{
 */

/**
 * @brief   Send command to LCD
 * @param   handle Pointer to LCD handle
 * @param   cmd Command byte
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_SendCommand(LCD_HandleTypeDef* handle, uint8_t cmd);

/**
 * @brief   Send data to LCD
 * @param   handle Pointer to LCD handle
 * @param   data Data byte
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_SendData(LCD_HandleTypeDef* handle, uint8_t data);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __LCD_H__ */
