# STM32F429I-DISC1 Timer (TIM) Modular Features

## How It Works

This timer module is designed to be modular and easy to use. Each timer feature (Base, PWM, Input Capture, Output Compare, Encoder) is implemented in its own pair of header (`.h`) and source (`.c`) files. This allows you to include and use only the features you need in your project.

**General Flow:**

1. **Initialization:**
   - Create a `TIM_HandleTypeDef` structure for your timer instance.
   - Call the appropriate `*_Init` function (e.g., `TIM_PWM_Init`) to configure the timer for the desired feature.

2. **Configuration (if needed):**
   - For features like PWM, Input Capture, or Output Compare, configure the specific channel(s) using the provided config functions (e.g., `TIM_PWM_ConfigChannel`).

3. **Start/Stop:**
   - Start the timer or feature using the provided start function (e.g., `TIM_PWM_Start`).
   - Stop it with the corresponding stop function when needed.

4. **Interrupts/Callbacks:**
   - For features that use interrupts (e.g., periodic base timer, input capture), handle events in the appropriate HAL callback (e.g., `HAL_TIM_PeriodElapsedCallback`, `HAL_TIM_IC_CaptureCallback`).

**Why Modular?**

- You only include the code you need, keeping your project clean and efficient.
- Each feature is independent, so you can use multiple timer features in the same project without conflicts.
- The API is consistent across features, making it easy to switch or combine functionalities.

This folder provides a modular implementation of all major timer (TIM) features for the STM32F429I-DISC1 board. Each feature is separated into its own header and source file for clarity and reusability.


## Features Implemented & When to Use Them

- **Base Timer** (`tim_base.h`/`.c`):
  - *What*: Provides a simple up-counter for periodic interrupts or timekeeping.
  - *When to use*: For basic time delays, periodic tasks, or as a timebase for other modules.

- **PWM** (`tim_pwm.h`/`.c`):
  - *What*: Generates Pulse Width Modulation signals on timer output channels.
  - *When to use*: For motor control, LED dimming, or any application needing variable duty cycle outputs.

- **Input Capture** (`tim_input_capture.h`/`.c`):
  - *What*: Captures the timer value on an external event (e.g., rising/falling edge).
  - *When to use*: For measuring frequency, pulse width, or timing of external signals.

- **Output Compare** (`tim_output_compare.h`/`.c`):
  - *What*: Toggles or sets an output pin when the timer reaches a specific value.
  - *When to use*: For generating precise output pulses, square waves, or event scheduling.

- **Encoder** (`tim_encoder.h`/`.c`):
  - *What*: Interfaces with quadrature encoders for position/speed feedback.
  - *When to use*: For reading rotary encoders in robotics, motor control, or user interfaces.

- **Timer with DMA** (`tim_dma.h`/`.c`):
  - *What*: Uses DMA to automatically update timer registers (e.g., for dynamic PWM duty cycles) without CPU intervention.
  - *When to use*: For high-speed or real-time waveform generation, audio, or applications where the timer's compare values need to change rapidly and efficiently.


## Usage Examples

### 1. Base Timer (Periodic Interrupt)
```c
#include "Peripherals/TIM/tim_base.h"
TIM_HandleTypeDef htim3;
TIM_Base_Init(&htim3, TIM3, 8399, 9999); // 10Hz at 84MHz
TIM_Base_Start(&htim3);
// Use HAL_TIM_PeriodElapsedCallback for interrupt handling
```

### 2. PWM Output
```c
#include "Peripherals/TIM/tim_pwm.h"
TIM_HandleTypeDef htim2;
TIM_PWM_Init(&htim2, TIM2, 83, 999); // 1kHz PWM
TIM_PWM_ConfigChannel(&htim2, TIM_CHANNEL_1, 500); // 50% duty
TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
```

### 3. Input Capture (Frequency Measurement)
```c
#include "Peripherals/TIM/tim_input_capture.h"
TIM_HandleTypeDef htim4;
TIM_IC_Init(&htim4, TIM4, 83, 0xFFFF);
TIM_IC_ConfigChannel(&htim4, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
TIM_IC_Start(&htim4, TIM_CHANNEL_1);
// Read captured value in HAL_TIM_IC_CaptureCallback
```

### 4. Output Compare (Pulse Generation)
```c
#include "Peripherals/TIM/tim_output_compare.h"
TIM_HandleTypeDef htim5;
TIM_OC_Init(&htim5, TIM5, 83, 9999);
TIM_OC_ConfigChannel(&htim5, TIM_CHANNEL_1, 5000); // Toggle at half period
TIM_OC_Start(&htim5, TIM_CHANNEL_1);
```

### 5. Encoder Interface
```c
#include "Peripherals/TIM/tim_encoder.h"
TIM_HandleTypeDef htim1;
TIM_Encoder_Init(&htim1, TIM1, 0, 0xFFFF);
TIM_Encoder_Start(&htim1);
// Read encoder count: __HAL_TIM_GET_COUNTER(&htim1)
```

### 6. Timer with DMA (Dynamic PWM Duty Cycle)
```c
#include "Peripherals/TIM/tim_dma.h"
TIM_HandleTypeDef htim8;
uint32_t pwm_buffer[3] = {100, 500, 900}; // Example duty cycles
// Timer and PWM must be initialized first (see PWM example)
TIM_DMA_PWM_Start(&htim8, TIM_CHANNEL_1, pwm_buffer, 3);
// To stop DMA:
TIM_DMA_PWM_Stop(&htim8, TIM_CHANNEL_1);
```

## File Structure

- `tim_base.*`         - Base timer (up-counter) operations
- `tim_pwm.*`          - PWM output
- `tim_input_capture.*`- Input capture
- `tim_output_compare.*`- Output compare
- `tim_encoder.*`      - Encoder interface
- `tim_dma.*`          - Timer with DMA (dynamic register updates)


Each feature is independent and can be used separately or together as needed. Choose the feature based on your application requirements as described above.

## Notes
- All functions use STM32 HAL types and must be called after HAL initialization.
- You can extend this structure for additional timer features (e.g., one-pulse, master/slave, etc.).
