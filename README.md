# Modular STM32 CubeMX Project Structure

This project has been reorganized to follow a better separation of concerns, where each peripheral is isolated in its own module. This structure makes the code more maintainable, reusable, and easier to understand.

## Project Structure

The project is organized as follows:

```
Sensor_Cons/
├── Core/                 # Core application code (main.c, etc.)
├── Drivers/              # STM32 HAL and CMSIS drivers
├── Middlewares/          # Middleware components (FreeRTOS, USB, etc.)
└── Peripherals/          # Modular peripheral code
    ├── GPIO/             # GPIO peripheral
    ├── CRC/              # CRC peripheral
    ├── DMA2D/            # DMA2D peripheral
    ├── FMC/              # FMC/SDRAM peripheral
    ├── I2C/              # I2C peripheral
    ├── LTDC/             # LCD controller peripheral
    ├── SPI/              # SPI peripheral
    ├── TIM/              # Timer peripheral
    └── UART/             # UART peripheral
```

## Module Design

Each peripheral module follows a standard structure:

- **Header File (`xxx.h`)**: Contains the interface declarations, including:
  - Function prototypes for initialization
  - External declarations for peripheral handles
  - Required include directives

- **Source File (`xxx.c`)**: Contains the implementation, including:
  - Peripheral handle definitions
  - Initialization functions
  - Other peripheral-specific functions

## Adding New Peripherals

To add a new peripheral:

1. Create a directory under `Peripherals/` for your peripheral
2. Create header and implementation files following the established pattern
3. Add the initialization call in `main.c`
4. Update the CMakeLists.txt if needed

## Building the Project

The project uses CMake for building. The CMakeLists.txt has been updated to include the new peripheral modules.

To build the project:

```bash
mkdir -p build/Debug
cd build/Debug
cmake -DCMAKE_BUILD_TYPE=Debug ../..
make
```

## Notes for Developers

- **Peripheral Initialization**: All peripherals are now initialized via their respective `XXX_Init()` functions
- **Handle Access**: Peripheral handles are declared in each module's source file and exposed via external declarations in the header
- **CubeMX Updates**: If you regenerate code with CubeMX, remember to update the modular structure with any changes
