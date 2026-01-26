/**
  ******************************************************************************
  * @file    keypad_example.c
  * @brief   4x4 Matrix Keypad usage examples
  * @version 1.0
  * @date    2026-01-03
  ******************************************************************************
  */

#include "keypad.h"
#include "log.h"
#include <stdio.h>

/* Private variables ---------------------------------------------------------*/
static KeypadHandle_t hKeypad;

/**
 * @example Basic keypad initialization and reading
 *
 * Hardware connection (example using GPIOD for rows, GPIOE for columns):
 *
 *           COL0   COL1   COL2   COL3
 *           PE0    PE1    PE2    PE3
 *            |      |      |      |
 *   ROW0 PD0-+------+------+------+--[1][2][3][A]
 *            |      |      |      |
 *   ROW1 PD1-+------+------+------+--[4][5][6][B]
 *            |      |      |      |
 *   ROW2 PD2-+------+------+------+--[7][8][9][C]
 *            |      |      |      |
 *   ROW3 PD3-+------+------+------+--[*][0][#][D]
 */

/**
 * @brief   Example: Initialize keypad with specific GPIO pins
 */
void Keypad_Example_Init(void)
{
    KeypadConfig_t config = {
        /* Row pins (directly connected, directly drive) */
        .rows = {
            {GPIOD, GPIO_PIN_0},    /* Row 0 */
            {GPIOD, GPIO_PIN_1},    /* Row 1 */
            {GPIOD, GPIO_PIN_2},    /* Row 2 */
            {GPIOD, GPIO_PIN_3}     /* Row 3 */
        },
        /* Column pins (directly connected, directly read) */
        .cols = {
            {GPIOE, GPIO_PIN_0},    /* Column 0 */
            {GPIOE, GPIO_PIN_1},    /* Column 1 */
            {GPIOE, GPIO_PIN_2},    /* Column 2 */
            {GPIOE, GPIO_PIN_3}     /* Column 3 */
        },
        .debounceMs = 30
    };

    if (Keypad_Init(&hKeypad, &config)) {
        log_info("Keypad initialized successfully");
    } else {
        log_error("Keypad initialization failed");
    }
}

/**
 * @brief   Example: Set custom key mapping
 * @note    Useful for different keypad layouts (phone, calculator, etc.)
 */
void Keypad_Example_CustomKeyMap(void)
{
    /* Phone-style keymap */
    const char phoneKeyMap[KEYPAD_ROWS][KEYPAD_COLS] = {
        {'1', '2', '3', 'U'},    /* U = Up */
        {'4', '5', '6', 'D'},    /* D = Down */
        {'7', '8', '9', 'L'},    /* L = Left */
        {'*', '0', '#', 'R'}     /* R = Right */
    };

    Keypad_SetKeyMap(&hKeypad, phoneKeyMap);
    log_info("Custom keymap set");
}

/**
 * @brief   Example: Polling for key press (non-blocking)
 * @note    Call this in your main loop
 */
void Keypad_Example_Poll(void)
{
    char key = Keypad_GetKey(&hKeypad);

    if (key != KEYPAD_NO_KEY) {
        log_info("Key pressed: %c", key);
    }
}

/**
 * @brief   Example: Wait for a specific key
 * @param   expectedKey The key to wait for
 */
void Keypad_Example_WaitForSpecificKey(char expectedKey)
{
    log_info("Press '%c' to continue...", expectedKey);

    char key;
    do {
        key = Keypad_WaitForKey(&hKeypad);
    } while (key != expectedKey);

    log_info("Correct key pressed!");
}

/**
 * @brief   Example: Enter a PIN code
 * @param   buffer Buffer to store PIN
 * @param   pinLength Number of digits required
 */
void Keypad_Example_EnterPIN(char* buffer, uint8_t pinLength)
{
    log_info("Enter %d-digit PIN:", pinLength);

    for (uint8_t i = 0; i < pinLength; i++) {
        char key = Keypad_WaitForKey(&hKeypad);

        /* Only accept numeric keys */
        if (key >= '0' && key <= '9') {
            buffer[i] = key;
            log_info("*");  /* Mask the digit */
        } else {
            i--;  /* Invalid key, retry */
        }
    }

    buffer[pinLength] = '\0';
    log_info("PIN entered");
}

/**
 * @brief   Example: Menu navigation using keypad
 */
void Keypad_Example_MenuNavigation(void)
{
    log_info("Menu Navigation:");
    log_info("  2 = Up");
    log_info("  8 = Down");
    log_info("  4 = Left");
    log_info("  6 = Right");
    log_info("  5 = Select");
    log_info("  * = Back");

    while (1) {
        char key = Keypad_WaitForKey(&hKeypad);

        switch (key) {
            case '2':
                log_info("Navigate UP");
                break;
            case '8':
                log_info("Navigate DOWN");
                break;
            case '4':
                log_info("Navigate LEFT");
                break;
            case '6':
                log_info("Navigate RIGHT");
                break;
            case '5':
                log_info("SELECTED");
                break;
            case '*':
                log_info("BACK");
                return;
            case '#':
                log_info("Exiting menu");
                return;
            default:
                log_info("Key: %c", key);
                break;
        }
    }
}

/**
 * @brief   Example: Calculator input
 */
void Keypad_Example_Calculator(void)
{
    int32_t result = 0;
    int32_t currentNum = 0;
    char operation = '\0';

    log_info("Simple Calculator:");
    log_info("  A = Add, B = Subtract, C = Multiply, D = Divide");
    log_info("  # = Calculate, * = Clear");

    while (1) {
        char key = Keypad_WaitForKey(&hKeypad);

        if (key >= '0' && key <= '9') {
            currentNum = currentNum * 10 + (key - '0');
            log_info("%c", key);
        } else if (key == 'A' || key == 'B' || key == 'C' || key == 'D') {
            if (operation == '\0') {
                result = currentNum;
            }
            operation = key;
            currentNum = 0;
            log_info(" %c ", key);
        } else if (key == '#') {
            /* Calculate result */
            switch (operation) {
                case 'A': result += currentNum; break;
                case 'B': result -= currentNum; break;
                case 'C': result *= currentNum; break;
                case 'D':
                    if (currentNum != 0) {
                        result /= currentNum;
                    }
                    break;
            }
            log_info(" = %d", result);
            currentNum = 0;
            operation = '\0';
        } else if (key == '*') {
            /* Clear */
            result = 0;
            currentNum = 0;
            operation = '\0';
            log_info("Cleared");
        }
    }
}

/**
 * @brief   Example: Get key position (row, column)
 */
void Keypad_Example_GetPosition(void)
{
    uint8_t row, col;

    if (Keypad_GetKeyPosition(&hKeypad, &row, &col)) {
        log_info("Key at Row: %d, Col: %d", row, col);
    } else {
        log_info("No key pressed");
    }
}

/**
 * @brief   Example: Full keypad demo
 */
void Keypad_Example_Demo(void)
{
    /* Initialize with default pins */
    Keypad_Example_Init();

    log_info("=== 4x4 Keypad Demo ===");
    log_info("Press any key (# to exit):");

    while (1) {
        char key = Keypad_GetKey(&hKeypad);

        if (key != KEYPAD_NO_KEY) {
            log_info("Key: %c", key);

            if (key == '#') {
                log_info("Demo ended");
                break;
            }
        }

        HAL_Delay(10);  /* Small delay to prevent CPU hogging */
    }
}
