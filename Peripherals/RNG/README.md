# RNG - Random Number Generator Driver

## Overview

The Random Number Generator (RNG) driver provides access to the STM32F429's hardware true random number generator (TRNG). It generates cryptographically-suitable random numbers using physical noise sources.

## Features

- True hardware random number generation
- Multiple generation functions (32-bit, bytes, range, float)
- Error detection and recovery
- Interrupt mode support
- Cryptographic-quality randomness

## Key Characteristics

| Parameter | Value |
|-----------|-------|
| Output | 32-bit random values |
| Generation Time | ~40 CPU cycles per number |
| Entropy Source | Analog noise |
| Compliance | NIST SP800-22 |

## Quick Start

### Basic Usage

```c
#include "rng.h"

int main(void)
{
    RNG_Init();

    uint32_t random;
    RNG_Generate(&random);

    printf("Random: %lu\n", random);

    RNG_DeInit();
}
```

### Generate Random Bytes

```c
uint8_t key[32];
RNG_GenerateBytes(key, sizeof(key));
```

### Generate Range-Limited Values

```c
/* Dice roll (1-6) */
uint32_t dice;
RNG_GenerateInRange(1, 6, &dice);

/* Random percentage (0-100) */
uint32_t percent;
RNG_GenerateInRange(0, 100, &percent);

/* Random boolean */
bool coinFlip;
RNG_GenerateBool(&coinFlip);
```

### Generate Random Float

```c
float probability;
RNG_GenerateFloat(&probability);  /* Returns [0.0, 1.0) */
```

## API Reference

### Initialization

| Function | Description |
|----------|-------------|
| `RNG_Init()` | Initialize RNG peripheral |
| `RNG_DeInit()` | Deinitialize RNG |

### Generation Functions

| Function | Description |
|----------|-------------|
| `RNG_Generate(uint32_t*)` | Generate 32-bit random |
| `RNG_GenerateWithTimeout(uint32_t*, timeout)` | Generate with timeout |
| `RNG_GenerateBuffer(uint32_t*, count)` | Fill buffer with randoms |
| `RNG_GenerateBytes(uint8_t*, length)` | Generate random bytes |

### Range Functions

| Function | Description |
|----------|-------------|
| `RNG_GenerateRange(max, result)` | Random in [0, max) |
| `RNG_GenerateInRange(min, max, result)` | Random in [min, max] |
| `RNG_GenerateBool(bool*)` | Random true/false |
| `RNG_GenerateFloat(float*)` | Random float [0.0, 1.0) |

### Utility Functions

| Function | Description |
|----------|-------------|
| `RNG_IsReady()` | Check if RNG has data |
| `RNG_HasError()` | Check for errors |
| `RNG_ClearErrors()` | Clear error flags |
| `RNG_GetStatusString()` | Get status description |

## Use Cases

### 1. Cryptographic Keys

```c
/* Generate 256-bit key */
uint8_t aesKey[32];
RNG_GenerateBytes(aesKey, 32);
```

### 2. Unique IDs

```c
/* Generate unique device ID */
uint32_t deviceId[4];
RNG_GenerateBuffer(deviceId, 4);
```

### 3. Random Delays (Anti-Timing-Attack)

```c
/* Random delay 1-100ms */
uint32_t delayMs;
RNG_GenerateInRange(1, 100, &delayMs);
HAL_Delay(delayMs);
```

### 4. Shuffling / Random Selection

```c
/* Random array index */
uint32_t index;
RNG_GenerateRange(arraySize, &index);
```

### 5. Monte Carlo Simulation

```c
/* Random point in unit square */
float x, y;
RNG_GenerateFloat(&x);
RNG_GenerateFloat(&y);
```

## Error Handling

```c
RNG_StatusTypeDef status = RNG_Generate(&value);

if (status == RNG_SEED_ERROR)
{
    /* Seed error - noise source issue */
    RNG_ClearErrors();
    RNG_Init();  /* Reinitialize */
}
else if (status == RNG_CLOCK_ERROR)
{
    /* Clock configuration issue */
    /* Check RCC configuration */
}
```

## Status Codes

| Status | Description |
|--------|-------------|
| `RNG_OK` | Success |
| `RNG_ERROR` | General error |
| `RNG_TIMEOUT` | Generation timeout |
| `RNG_CLOCK_ERROR` | Clock not configured |
| `RNG_SEED_ERROR` | Entropy source error |
| `RNG_NOT_READY` | No random data available |

## Hardware Requirements

- RNG clock must be enabled (done by `RNG_Init()`)
- PLL48CLK must be enabled and configured
- For F4 series: HSE or HSI must be enabled

## Performance Notes

- First generation after init may take longer (analog settling)
- Continuous generation: ~40 CPU cycles per number
- Buffer generation more efficient than single calls

## Security Considerations

1. **Always check return status** - Don't use potentially non-random values
2. **Clear sensitive data** - Zero buffers after use
3. **Monitor errors** - Seed errors may indicate hardware issues
4. **Use full entropy** - Avoid truncating unnecessarily

## Interrupt Mode

For non-blocking operation:

```c
RNG_EnableInterrupt();

/* In HASH_RNG_IRQHandler: */
void HASH_RNG_IRQHandler(void)
{
    HAL_RNG_IRQHandler(&hrng);
}

void HAL_RNG_ReadyDataCallback(RNG_HandleTypeDef *hrng, uint32_t random32bit)
{
    /* Handle random number */
}
```

## Testing Randomness

The generated numbers should pass:
- NIST SP800-22 statistical tests
- Diehard tests
- TestU01 suite

For critical applications, consider additional testing.
