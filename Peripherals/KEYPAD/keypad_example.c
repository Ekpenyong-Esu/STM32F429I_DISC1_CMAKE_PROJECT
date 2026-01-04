/**
  ******************************************************************************
  * @file    keypad_example.c
  * @brief   4x4 Matrix Keypad usage examples
  * @version 1.0
  * @date    2026-01-03
  ******************************************************************************
  */

#include "keypad.h"
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
        printf("Keypad initialized successfully\r\n");
    } else {
        printf("Keypad initialization failed\r\n");
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
    printf("Custom keymap set\r\n");
}

/**
 * @brief   Example: Polling for key press (non-blocking)
 * @note    Call this in your main loop
 */
void Keypad_Example_Poll(void)
{
    char key = Keypad_GetKey(&hKeypad);

    if (key != KEYPAD_NO_KEY) {
        printf("Key pressed: %c\r\n", key);
    }
}

/**
 * @brief   Example: Wait for a specific key
 * @param   expectedKey The key to wait for
 */
void Keypad_Example_WaitForSpecificKey(char expectedKey)
{
    printf("Press '%c' to continue...\r\n", expectedKey);

    char key;
    do {
        key = Keypad_WaitForKey(&hKeypad);
    } while (key != expectedKey);

    printf("Correct key pressed!\r\n");
}

/**
 * @brief   Example: Enter a PIN code
 * @param   buffer Buffer to store PIN
 * @param   pinLength Number of digits required
 */
void Keypad_Example_EnterPIN(char* buffer, uint8_t pinLength)
{
    printf("Enter %d-digit PIN:\r\n", pinLength);

    for (uint8_t i = 0; i < pinLength; i++) {
        char key = Keypad_WaitForKey(&hKeypad);

        /* Only accept numeric keys */
        if (key >= '0' && key <= '9') {
            buffer[i] = key;
            printf("*");  /* Mask the digit */
        } else {
            i--;  /* Invalid key, retry */
        }
    }

    buffer[pinLength] = '\0';
    printf("\r\nPIN entered\r\n");
}

/**
 * @brief   Example: Menu navigation using keypad
 */
void Keypad_Example_MenuNavigation(void)
{
    printf("Menu Navigation:\r\n");
    printf("  2 = Up\r\n");
    printf("  8 = Down\r\n");
    printf("  4 = Left\r\n");
    printf("  6 = Right\r\n");
    printf("  5 = Select\r\n");
    printf("  * = Back\r\n");

    while (1) {
        char key = Keypad_WaitForKey(&hKeypad);

        switch (key) {
            case '2':
                printf("Navigate UP\r\n");
                break;
            case '8':
                printf("Navigate DOWN\r\n");
                break;
            case '4':
                printf("Navigate LEFT\r\n");
                break;
            case '6':
                printf("Navigate RIGHT\r\n");
                break;
            case '5':
                printf("SELECTED\r\n");
                break;
            case '*':
                printf("BACK\r\n");
                return;
            case '#':
                printf("Exiting menu\r\n");
                return;
            default:
                printf("Key: %c\r\n", key);
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

    printf("Simple Calculator:\r\n");
    printf("  A = Add, B = Subtract, C = Multiply, D = Divide\r\n");
    printf("  # = Calculate, * = Clear\r\n");

    while (1) {
        char key = Keypad_WaitForKey(&hKeypad);

        if (key >= '0' && key <= '9') {
            currentNum = currentNum * 10 + (key - '0');
            printf("%c", key);
        } else if (key == 'A' || key == 'B' || key == 'C' || key == 'D') {
            if (operation == '\0') {
                result = currentNum;
            }
            operation = key;
            currentNum = 0;
            printf(" %c ", key);
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
            printf(" = %ld\r\n", result);
            currentNum = 0;
            operation = '\0';
        } else if (key == '*') {
            /* Clear */
            result = 0;
            currentNum = 0;
            operation = '\0';
            printf("\r\nCleared\r\n");
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
        printf("Key at Row: %d, Col: %d\r\n", row, col);
    } else {
        printf("No key pressed\r\n");
    }
}

/**
 * @brief   Example: Full keypad demo
 */
void Keypad_Example_Demo(void)
{
    /* Initialize with default pins */
    Keypad_Example_Init();

    printf("\r\n=== 4x4 Keypad Demo ===\r\n");
    printf("Press any key (# to exit):\r\n");

    while (1) {
        char key = Keypad_GetKey(&hKeypad);

        if (key != KEYPAD_NO_KEY) {
            printf("Key: %c\r\n", key);

            if (key == '#') {
                printf("Demo ended\r\n");
                break;
            }
        }

        HAL_Delay(10);  /* Small delay to prevent CPU hogging */
    }
}
