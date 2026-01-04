# DHT22 Temperature & Humidity Sensor - Simplified Version

## What Does This Sensor Do?
The DHT22 measures:
- **Temperature** in Celsius (°C)
- **Humidity** as a percentage (%)

It uses a single digital pin for communication.

## How to Use

### 1. Initialize the Sensor
```c
#include "dht.h"

DHT_Handle_t dht;
DHT_Init(&dht, GPIOA, GPIO_PIN_0);  // Connect to PA0
```

### 2. Read Temperature and Humidity
```c
if (DHT_Read(&dht) == HAL_OK) {
    printf("Temperature: %.1f°C\n", dht.temperature);
    printf("Humidity: %.1f%%\n", dht.humidity);
} else {
    printf("Read failed!\n");
}
```

### 3. Important Notes
- **Wait 2 seconds** between reads (sensor limitation)
- First read after power-on may fail (normal)
- Data is stored directly in the handle structure

## Connection
- VCC → 3.3V
- GND → GND
- DATA → Any GPIO pin (e.g., PA0)
- Add 4.7kΩ pull-up resistor between DATA and VCC

## Common Issues
- **Read fails:** Check wiring and pull-up resistor
- **Wrong values:** Wait longer between reads
- **Always returns 0:** Sensor not connected or damaged

## What Was Simplified?
- Removed DHT11 support (focus on DHT22 only)
- Removed checksum validation (can be added back)
- Removed read interval enforcement (trust user)
- Simplified timing using basic delays instead of DWT cycle counter
- Direct access to data (no getter functions)

## Future Enhancements (TODO)
- Add DHT11 support for beginners
- Add checksum validation for reliability
- Add automatic retry on failure
- Enforce minimum read interval
