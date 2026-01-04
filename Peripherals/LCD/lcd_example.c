/**
 * @file    lcd_example.c
 * @brief   LCD Driver Usage Examples
 * @details Examples demonstrating the LCD driver features
 * @version 1.0
 * @date    2026-01-03
 */

/* Includes ------------------------------------------------------------------*/
#include "lcd.h"

/* Example Handles -----------------------------------------------------------*/
static LCD_HandleTypeDef hLCD;

/* Custom Character Definitions ----------------------------------------------*/

/** Heart symbol */
static const uint8_t charHeart[] = {
    0b00000,
    0b01010,
    0b11111,
    0b11111,
    0b01110,
    0b00100,
    0b00000,
    0b00000
};

/** Battery full symbol */
static const uint8_t charBattery[] = {
    0b01110,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111
};

/** Degree symbol */
static const uint8_t charDegree[] = {
    0b01100,
    0b10010,
    0b10010,
    0b01100,
    0b00000,
    0b00000,
    0b00000,
    0b00000
};

/** Bell symbol */
static const uint8_t charBell[] = {
    0b00100,
    0b01110,
    0b01110,
    0b01110,
    0b11111,
    0b00000,
    0b00100,
    0b00000
};

/** Smiley face */
static const uint8_t charSmiley[] = {
    0b00000,
    0b01010,
    0b01010,
    0b00000,
    0b10001,
    0b01110,
    0b00000,
    0b00000
};

/* Example Functions ---------------------------------------------------------*/

/**
 * @brief   Example 1: Basic 16x2 LCD initialization (4-bit mode)
 * @details Initialize a standard 16x2 LCD in 4-bit mode
 */
void LCD_Example_Basic16x2(void)
{
    LCD_PinsTypeDef pins = {
        .rs = {GPIOA, GPIO_PIN_0},
        .rw = {NULL, 0},                /* RW tied to GND */
        .en = {GPIOA, GPIO_PIN_1},
        .d4 = {GPIOA, GPIO_PIN_2},
        .d5 = {GPIOA, GPIO_PIN_3},
        .d6 = {GPIOA, GPIO_PIN_4},
        .d7 = {GPIOA, GPIO_PIN_5},
        .backlight = {GPIOA, GPIO_PIN_6}
    };

    /* Initialize with default 16x2, 4-bit mode */
    LCD_InitDefault(&hLCD, &pins);

    /* Print welcome message */
    LCD_PrintString(&hLCD, "Hello, World!");
    LCD_SetCursor(&hLCD, 0, 1);
    LCD_PrintString(&hLCD, "STM32F429");
}

/**
 * @brief   Example 2: 20x4 LCD with full configuration
 * @details Initialize a 20x4 LCD with custom configuration
 */
void LCD_Example_20x4LCD(void)
{
    LCD_ConfigTypeDef config = {
        .pins = {
            .rs = {GPIOB, GPIO_PIN_0},
            .rw = {GPIOB, GPIO_PIN_1},  /* Using RW pin */
            .en = {GPIOB, GPIO_PIN_2},
            .d4 = {GPIOB, GPIO_PIN_4},
            .d5 = {GPIOB, GPIO_PIN_5},
            .d6 = {GPIOB, GPIO_PIN_6},
            .d7 = {GPIOB, GPIO_PIN_7},
            .backlight = {GPIOB, GPIO_PIN_8}
        },
        .mode = LCD_MODE_4BIT,
        .size = LCD_SIZE_20x4,
        .useRW = true,
        .useBacklight = true
    };

    LCD_Init(&hLCD, &config);

    /* Print on all 4 lines */
    LCD_PrintStringAt(&hLCD, 0, 0, "Line 1: Temperature");
    LCD_PrintStringAt(&hLCD, 0, 1, "Line 2: Humidity");
    LCD_PrintStringAt(&hLCD, 0, 2, "Line 3: Pressure");
    LCD_PrintStringAt(&hLCD, 0, 3, "Line 4: Status: OK");
}

/**
 * @brief   Example 3: 8-bit mode initialization
 * @details Initialize LCD in 8-bit mode (uses more pins but faster)
 */
void LCD_Example_8BitMode(void)
{
    LCD_ConfigTypeDef config = {
        .pins = {
            .rs = {GPIOC, GPIO_PIN_0},
            .rw = {NULL, 0},
            .en = {GPIOC, GPIO_PIN_1},
            .d0 = {GPIOD, GPIO_PIN_0},
            .d1 = {GPIOD, GPIO_PIN_1},
            .d2 = {GPIOD, GPIO_PIN_2},
            .d3 = {GPIOD, GPIO_PIN_3},
            .d4 = {GPIOD, GPIO_PIN_4},
            .d5 = {GPIOD, GPIO_PIN_5},
            .d6 = {GPIOD, GPIO_PIN_6},
            .d7 = {GPIOD, GPIO_PIN_7},
            .backlight = {GPIOC, GPIO_PIN_2}
        },
        .mode = LCD_MODE_8BIT,
        .size = LCD_SIZE_16x2,
        .useRW = false,
        .useBacklight = true
    };

    LCD_Init(&hLCD, &config);

    LCD_PrintString(&hLCD, "8-bit Mode!");
}

/**
 * @brief   Example 4: Custom characters
 * @details Create and display custom characters
 */
void LCD_Example_CustomCharacters(void)
{
    /* Assume LCD already initialized */

    /* Create custom characters */
    LCD_CreateChar(&hLCD, 0, charHeart);
    LCD_CreateChar(&hLCD, 1, charBattery);
    LCD_CreateChar(&hLCD, 2, charDegree);
    LCD_CreateChar(&hLCD, 3, charBell);
    LCD_CreateChar(&hLCD, 4, charSmiley);

    /* Display custom characters */
    LCD_Clear(&hLCD);
    LCD_PrintString(&hLCD, "Custom: ");
    LCD_PrintCustomChar(&hLCD, 0);  /* Heart */
    LCD_PrintCustomChar(&hLCD, 1);  /* Battery */
    LCD_PrintCustomChar(&hLCD, 2);  /* Degree */
    LCD_PrintCustomChar(&hLCD, 3);  /* Bell */
    LCD_PrintCustomChar(&hLCD, 4);  /* Smiley */

    /* Use degree symbol in temperature display */
    LCD_SetCursor(&hLCD, 0, 1);
    LCD_PrintString(&hLCD, "Temp: 25");
    LCD_PrintCustomChar(&hLCD, 2);
    LCD_PrintString(&hLCD, "C");
}

/**
 * @brief   Example 5: Numeric display
 * @details Display various numeric formats
 */
void LCD_Example_NumericDisplay(void)
{
    /* Assume LCD already initialized */

    LCD_Clear(&hLCD);

    /* Integer display */
    LCD_PrintString(&hLCD, "Int: ");
    LCD_PrintInt(&hLCD, -12345);

    /* Float display */
    LCD_SetCursor(&hLCD, 0, 1);
    LCD_PrintString(&hLCD, "Float: ");
    LCD_PrintFloat(&hLCD, 3.14159f, 2);
}

/**
 * @brief   Example 6: Printf-style formatting
 * @details Use formatted printing
 */
void LCD_Example_Printf(void)
{
    int temperature = 25;
    int humidity = 65;

    /* Assume LCD already initialized */

    LCD_Clear(&hLCD);
    LCD_Printf(&hLCD, "Temp: %d C", temperature);

    LCD_SetCursor(&hLCD, 0, 1);
    LCD_Printf(&hLCD, "Humid: %d%%", humidity);
}

/**
 * @brief   Example 7: Cursor control
 * @details Demonstrate cursor operations
 */
void LCD_Example_CursorControl(void)
{
    /* Assume LCD already initialized */

    LCD_Clear(&hLCD);
    LCD_PrintString(&hLCD, "Cursor Demo");

    /* Show cursor */
    LCD_CursorOn(&hLCD);
    HAL_Delay(1000);

    /* Enable blinking */
    LCD_BlinkOn(&hLCD);
    HAL_Delay(1000);

    /* Move cursor */
    LCD_SetCursor(&hLCD, 5, 1);
    HAL_Delay(1000);

    /* Hide cursor */
    LCD_CursorOff(&hLCD);
    LCD_BlinkOff(&hLCD);
}

/**
 * @brief   Example 8: Scrolling text
 * @details Demonstrate display scrolling
 */
void LCD_Example_Scrolling(void)
{
    /* Assume LCD already initialized */

    LCD_Clear(&hLCD);
    LCD_PrintString(&hLCD, "Scroll Demo >>>");

    /* Scroll display left */
    for (int i = 0; i < 16; i++) {
        HAL_Delay(300);
        LCD_ScrollLeft(&hLCD);
    }

    /* Scroll display right */
    for (int i = 0; i < 16; i++) {
        HAL_Delay(300);
        LCD_ScrollRight(&hLCD);
    }
}

/**
 * @brief   Example 9: Backlight control
 * @details Demonstrate backlight on/off
 */
void LCD_Example_BacklightControl(void)
{
    /* Assume LCD already initialized */

    LCD_Clear(&hLCD);
    LCD_PrintString(&hLCD, "Backlight Demo");

    /* Toggle backlight */
    for (int i = 0; i < 5; i++) {
        LCD_BacklightOff(&hLCD);
        HAL_Delay(500);
        LCD_BacklightOn(&hLCD);
        HAL_Delay(500);
    }
}

/**
 * @brief   Example 10: Live data display
 * @details Display live sensor data (simulated)
 */
void LCD_Example_LiveData(void)
{
    float temperature = 25.0f;
    int humidity = 60;
    uint32_t counter = 0;

    /* Assume LCD already initialized */

    LCD_Clear(&hLCD);

    /* Create degree symbol */
    LCD_CreateChar(&hLCD, 0, charDegree);

    while (1) {
        /* Simulate sensor readings */
        temperature = 25.0f + (float)(counter % 10) * 0.5f;
        humidity = 60 + (counter % 20);

        /* Update line 1: Temperature */
        LCD_SetCursor(&hLCD, 0, 0);
        LCD_Printf(&hLCD, "Temp: %.1f", temperature);
        LCD_PrintCustomChar(&hLCD, 0);
        LCD_PrintString(&hLCD, "C   ");

        /* Update line 2: Humidity + Counter */
        LCD_SetCursor(&hLCD, 0, 1);
        LCD_Printf(&hLCD, "RH:%d%% #%lu  ", humidity, counter);

        counter++;
        HAL_Delay(500);
    }
}

/**
 * @brief   Example 11: Menu system
 * @details Simple menu navigation display
 */
void LCD_Example_MenuSystem(void)
{
    const char* menuItems[] = {
        "1. Settings",
        "2. Calibrate",
        "3. View Data",
        "4. About"
    };
    uint8_t selectedItem = 0;

    /* Assume LCD already initialized with 20x4 display */

    LCD_Clear(&hLCD);
    LCD_PrintStringAt(&hLCD, 2, 0, "=== MAIN MENU ===");

    /* Display menu items */
    for (int i = 0; i < 4 && i < 3; i++) {
        LCD_SetCursor(&hLCD, 0, i + 1);
        if (i == selectedItem) {
            LCD_PrintString(&hLCD, "> ");
        } else {
            LCD_PrintString(&hLCD, "  ");
        }
        LCD_PrintString(&hLCD, menuItems[i]);
    }
}

/**
 * @brief   Example 12: Progress bar
 * @details Display a progress bar using custom characters
 */
void LCD_Example_ProgressBar(void)
{
    /* Custom characters for progress bar */
    static const uint8_t charFull[] = {
        0b11111, 0b11111, 0b11111, 0b11111,
        0b11111, 0b11111, 0b11111, 0b11111
    };
    static const uint8_t charEmpty[] = {
        0b11111, 0b10001, 0b10001, 0b10001,
        0b10001, 0b10001, 0b10001, 0b11111
    };

    /* Assume LCD already initialized */

    LCD_CreateChar(&hLCD, 5, charFull);
    LCD_CreateChar(&hLCD, 6, charEmpty);

    LCD_Clear(&hLCD);
    LCD_PrintString(&hLCD, "Progress:");

    /* Animate progress bar */
    for (int progress = 0; progress <= 100; progress += 10) {
        LCD_SetCursor(&hLCD, 0, 1);
        LCD_PrintString(&hLCD, "[");

        int filledBlocks = progress / 10;
        for (int i = 0; i < 10; i++) {
            if (i < filledBlocks) {
                LCD_PrintCustomChar(&hLCD, 5);  /* Full */
            } else {
                LCD_PrintCustomChar(&hLCD, 6);  /* Empty */
            }
        }

        LCD_Printf(&hLCD, "] %3d%%", progress);

        HAL_Delay(200);
    }
}

/**
 * @brief   Example 13: Hex dump display
 * @details Display data in hexadecimal format
 */
void LCD_Example_HexDisplay(void)
{
    uint8_t data[] = {0xDE, 0xAD, 0xBE, 0xEF};

    /* Assume LCD already initialized */

    LCD_Clear(&hLCD);
    LCD_PrintString(&hLCD, "Hex Data:");

    LCD_SetCursor(&hLCD, 0, 1);
    for (int i = 0; i < 4; i++) {
        LCD_PrintHex(&hLCD, data[i], 2);
        LCD_PrintString(&hLCD, " ");
    }
}

/**
 * @brief   Example 14: Clock display
 * @details Simple time display
 */
void LCD_Example_ClockDisplay(void)
{
    uint8_t hours = 12;
    uint8_t minutes = 30;
    uint8_t seconds = 0;

    /* Assume LCD already initialized */

    LCD_Clear(&hLCD);
    LCD_PrintStringAt(&hLCD, 3, 0, "Digital Clock");

    while (1) {
        LCD_SetCursor(&hLCD, 4, 1);
        LCD_Printf(&hLCD, "%02d:%02d:%02d", hours, minutes, seconds);

        /* Increment time */
        seconds++;
        if (seconds >= 60) {
            seconds = 0;
            minutes++;
            if (minutes >= 60) {
                minutes = 0;
                hours++;
                if (hours >= 24) {
                    hours = 0;
                }
            }
        }

        HAL_Delay(1000);
    }
}
