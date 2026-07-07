# 4x4 Matrix Keypad Driver

Matrix keypad driver for STM32F429 with configurable GPIO pins and debouncing.

## Features

- **4x4 matrix scanning** (16 keys)
- **Configurable GPIO** pins for rows and columns
- **Software debouncing** (adjustable timing)
- **Custom key mapping** support
- **Blocking and non-blocking** read modes
- **Position detection** (row/column index)

## Hardware Connection

```
Standard 4x4 Keypad Layout:
┌─────┬─────┬─────┬─────┐
│  1  │  2  │  3  │  A  │
├─────┼─────┼─────┼─────┤
│  4  │  5  │  6  │  B  │
├─────┼─────┼─────┼─────┤
│  7  │  8  │  9  │  C  │
├─────┼─────┼─────┼─────┤
│  *  │  0  │  #  │  D  │
└─────┴─────┴─────┴─────┘
```

**Wiring:**
```
         COL0   COL1   COL2   COL3
          |      |      |      |
ROW0 ─────┼──────┼──────┼──────┼─── (output)
          |      |      |      |
ROW1 ─────┼──────┼──────┼──────┼─── (output)
          |      |      |      |
ROW2 ─────┼──────┼──────┼──────┼─── (output)
          |      |      |      |
ROW3 ─────┼──────┼──────┼──────┼─── (output)
          |      |      |      |
        (input)(input)(input)(input)
        pull-up pull-up pull-up pull-up
```

## Quick Start

```c
#include "keypad.h"

KeypadHandle_t hKeypad;

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    /* Configure keypad GPIO pins */
    KeypadConfig_t config = {
        .rows = {
            {GPIOD, GPIO_PIN_0},
            {GPIOD, GPIO_PIN_1},
            {GPIOD, GPIO_PIN_2},
            {GPIOD, GPIO_PIN_3}
        },
        .cols = {
            {GPIOE, GPIO_PIN_0},
            {GPIOE, GPIO_PIN_1},
            {GPIOE, GPIO_PIN_2},
            {GPIOE, GPIO_PIN_3}
        },
        .debounceMs = 30
    };

    Keypad_Init(&hKeypad, &config);

    while (1) {
        char key = Keypad_GetKey(&hKeypad);
        if (key != KEYPAD_NO_KEY) {
            printf("Key: %c\n", key);
        }
        HAL_Delay(10);
    }
}
```

## API Reference

| Function | Description |
|----------|-------------|
| `Keypad_Init()` | Initialize keypad with GPIO config |
| `Keypad_SetKeyMap()` | Set custom key mapping |
| `Keypad_GetKey()` | Get pressed key (debounced) |
| `Keypad_GetKeyRaw()` | Get pressed key (no debounce) |
| `Keypad_IsKeyPressed()` | Check if any key is pressed |
| `Keypad_WaitForKey()` | Wait for key press (blocking) |
| `Keypad_GetKeyPosition()` | Get row/column of pressed key |
| `Keypad_GetLastKey()` | Get last detected key |
| `Keypad_ClearLastKey()` | Clear the last key buffer |

## Custom Key Mapping

```c
const char customMap[4][4] = {
    {'1', '2', '3', 'U'},    /* U = Up */
    {'4', '5', '6', 'D'},    /* D = Down */
    {'7', '8', '9', 'L'},    /* L = Left */
    {'*', '0', '#', 'R'}     /* R = Right */
};

Keypad_SetKeyMap(&hKeypad, customMap);
```

## Scanning Algorithm

1. Set all ROW pins HIGH
2. For each ROW:
   - Set ROW LOW
   - Read all COL pins
   - If any COL is LOW → key found at (row, col)
   - Set ROW HIGH
3. Apply debounce filtering

## Troubleshooting

| Problem | Solution |
|---------|----------|
| No keys detected | Check wiring, verify GPIO ports/pins |
| Multiple keys detected | Add external diodes for N-key rollover |
| Ghost keys | Avoid pressing 3+ keys simultaneously |
| Bouncing | Increase `debounceMs` value |

## Files

| File | Description |
|------|-------------|
| `keypad.h` | API declarations and types |
| `keypad.c` | Driver implementation |
| `keypad_example.c` | Usage examples |
