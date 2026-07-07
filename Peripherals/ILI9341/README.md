Peripherals/ILI9341
====================

Minimal ILI9341 driver adapted from ST's BSP and adjusted to the repository conventions:

- High-level API matches ST-style (ili9341_Init, ili9341_ReadID, DisplayOn/Off, GetLcdPixelWidth/Height).
- Uses the repository's central SPI driver (call `SPI_Init()` / `SPI_Transmit` / `SPI_TransmitReceive`).
- Provides weak MSP hooks `ILI9341_MspInit()` and `ILI9341_MspDeInit()` for board-specific pin and bus configuration.
- Keeps the ST init command sequence (trimmed) so behavior is compatible with ST examples.

Integration notes:
- The driver expects the board to either provide `ILI9341_MspInit()` implementation or allow the default weak implementation to configure CS and WRX pins and call `SPI_Init()`.
- This driver intentionally keeps the ST command/data bytes as literals in the init sequence (these are vendor-specified initialization values).

Usage example:

    #include "ili9341.h"

    ili9341_Init();
    uint16_t id = ili9341_ReadID();
    ili9341_DisplayOn();

A minimal example `ili9341_example.c` has been added to this folder showing basic usage. If you want me to adapt the MSP pins to another board or wire this into an application example, tell me which board and I'll update the MSP hook and example accordingly.
