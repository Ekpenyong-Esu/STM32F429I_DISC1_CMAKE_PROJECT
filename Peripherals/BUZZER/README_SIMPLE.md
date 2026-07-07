# Buzzer Driver (Simplified)

This folder contains a small, beginner-friendly buzzer driver supporting:

- Active buzzers (simple GPIO on/off)
- Passive buzzers (tone generation via timer PWM)

Files:
- `buzzer.h` - Header file and API
- `buzzer.c` - Implementation
- `buzzer_example.c` - Basic examples (already present)

Usage (active):
```c
Buzzer_t buz;
Buzzer_InitActive(&buz, GPIOB, GPIO_PIN_1);
Buzzer_On(&buz);
HAL_Delay(200);
Buzzer_Off(&buz);
```

Usage (passive):
```c
extern TIM_HandleTypeDef htim3; // configured elsewhere
Buzzer_t buz;
Buzzer_InitPassive(&buz, &htim3, TIM_CHANNEL_1, 84000000U /* timer clock Hz */);
Buzzer_Tone(&buz, 2000U, 50); // 2 kHz, 50% duty
HAL_Delay(200);
Buzzer_Off(&buz);
```

Notes & TODO:
- The passive buzzer code directly modifies the timer ARR and CCR registers; if the same timer is used for other tasks, dedicate a timer for the buzzer or adapt the code to save/restore configuration.
- Add blocking tone-with-duration helper
- Add tone generation using interrupts or DMA for more advanced use cases
- Add safety checks for very low/high frequencies
