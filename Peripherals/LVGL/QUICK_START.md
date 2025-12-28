# Quick Start Guide - STM32F429I GUI

## 🚀 Getting Started

This guide will help you build, flash, and run the GUI application on your STM32F429I-DISC1 board.

## 📋 Prerequisites

### Hardware Required
- ✅ STM32F429I-DISC1 Discovery board
- ✅ USB cable (Micro-USB or USB-C depending on board revision)
- ✅ Computer with USB port

### Software Required
- ✅ ARM GCC toolchain (installed)
- ✅ CMake 3.20+ (installed)
- ✅ STM32CubeProgrammer (for flashing)
- ✅ VS Code with CMake extension (recommended)

## 🛠️ Build Instructions

### Option 1: Using VS Code Tasks (Recommended)

1. **Open the workspace** in VS Code
   ```bash
   cd /home/mahonri/Desktop/BareMetal/Sensor_Cons
   code .
   ```

2. **Build the project**
   - Press `Ctrl+Shift+P`
   - Type "Tasks: Run Task"
   - Select "CMake: clean rebuild"
   - Wait for compilation to complete

3. **Flash to board**
   - Connect your STM32F429I board via USB
   - Press `Ctrl+Shift+P`
   - Type "Tasks: Run Task"
   - Select "Build + Flash"
   - The firmware will build and flash automatically

### Option 2: Using Terminal Commands

```bash
# Navigate to project directory
cd /home/mahonri/Desktop/BareMetal/Sensor_Cons

# Clean and rebuild
cmake --build build/Debug --target clean
cmake --build build/Debug --target all -j$(nproc)

# Flash to board (ensure board is connected)
STM32_Programmer_CLI --connect port=swd --download build/Debug/Sensor_Console.elf -hardRst -rst --start
```

### Option 3: Quick Build Command

```bash
# One-line build
cd /home/mahonri/Desktop/BareMetal/Sensor_Cons && cmake --build build/Debug -j$(nproc)
```

## 🔌 Hardware Setup

### 1. Board Connections
```
STM32F429I-DISC1
├── USB Cable → CN1 (ST-LINK/V2 port)
├── Display → Built-in 2.4" LCD (240x320)
└── Touch → Built-in capacitive touch
```

### 2. Power On
- Connect USB cable to ST-LINK port (CN1)
- Board will power on automatically
- Red LED (PWR) should illuminate

### 3. Verify Display
- LCD backlight should turn on
- If LTDC is initialized, you'll see GUI after flashing

## 📺 First Run

### What You Should See

**Immediately after flashing:**
1. Board resets automatically
2. LCD backlight turns on
3. LVGL initializes (~1-2 seconds)
4. **Home Dashboard appears** with:
   - Title: "STM32F429 Dashboard"
   - Status: "✓ System Ready"
   - Temperature gauge showing 25°C
   - Humidity bar showing 60%
   - Two buttons: "☰ Sensors" and "⚙ Config"

### Demo Mode

The current implementation includes **automatic demo animation**:
- Temperature cycles between 25-40°C
- Humidity cycles between 50-80%
- Sensor chart updates with simulated data
- Updates every 500ms

### Testing Touch Screen

1. **Navigate to Sensors**
   - Touch the green "☰ Sensors" button
   - Screen fades to sensor monitor view
   - See live chart with 4 sensor cards

2. **Return Home**
   - Touch "← Back" button
   - Returns to main dashboard

3. **Open Settings**
   - Touch the yellow "⚙ Config" button
   - See sliders and toggles
   - Try moving sliders with your finger
   - Toggle switches on/off

4. **View System Info**
   - From Settings, touch "📞 System Info"
   - See hardware specifications
   - Touch "← Back" twice to return home

## 🐛 Troubleshooting

### Display is blank or black

**Possible causes:**
1. LTDC not initialized
2. Framebuffer address incorrect
3. LCD power issue

**Solutions:**
```bash
# Check LTDC initialization
# Verify in lv_port_disp.c:
#define LVGL_FB_ADDR 0xD0000000  // Uncomment and verify address
```

### Touch not responding

**Possible causes:**
1. I2C not initialized
2. Touch controller not detected
3. Wrong touch driver

**Solutions:**
- Check I2C3 initialization (used by touch controller)
- Verify touch controller type (FT6206 or STMPE811)
- Check `lv_port_indev.c` configuration

### Colors look wrong

**Possible causes:**
1. Color format mismatch
2. Byte swap needed

**Solutions:**
```c
// In lv_conf.h, try toggling:
#define LV_COLOR_16_SWAP 0  // Try 1 if colors inverted
```

### Screen flickers or tears

**Possible causes:**
1. Draw buffer too small
2. Single buffering
3. LVGL tick rate issues

**Solutions:**
```c
// In lv_port_disp.c, increase buffer size:
#define LVGL_DRAW_BUF_LINES 40  // Increase from 20
```

### Compilation errors

**Common issues:**

1. **LVGL not found**
   ```bash
   # LVGL is auto-downloaded by CMake
   # Clean and rebuild:
   rm -rf build/Debug/_deps
   cmake --build build/Debug --target all
   ```

2. **Missing HAL drivers**
   ```bash
   # Ensure all peripherals initialized in CubeMX
   # Regenerate code if needed
   ```

## 🔧 Customization

### Modify Demo Values

Edit [main.c](../../Core/Src/main.c):
```c
// Change update interval (default: 500ms)
if(HAL_GetTick() - last_update >= 1000) {  // Now 1 second
    // ...
}

// Change temperature range
temp = 20 + (demo_counter % 10);  // 20-30°C instead of 25-40°C
```

### Add Real Sensors

Replace demo code with actual sensor readings:
```c
#include "accel.h"
#include "temp_sensor.h"

int main(void) {
    // ... initialization ...
    
    // Initialize sensors
    ACCEL_Init();
    TEMP_Init();
    
    while(1) {
        LVGL_App_Tick();
        
        if(HAL_GetTick() - last_update >= 500) {
            // Read real sensors
            float temp = TEMP_Read();
            int accel = ACCEL_ReadX();
            
            // Update GUI
            LVGL_App_UpdateTemperature((int)temp);
            LVGL_App_AddChartData(accel);
            
            last_update = HAL_GetTick();
        }
        
        HAL_Delay(5);
    }
}
```

### Change Colors

Edit [lvgl_app.c](lvgl_app.c) color definitions:
```c
// Example: Change background color
lv_obj_set_style_bg_color(scr_home, lv_color_hex(0x000000), 0);  // Pure black

// Change button colors
lv_obj_set_style_bg_color(btn_sensors, lv_color_hex(0x00ff00), 0);  // Bright green
```

## 📊 Performance Monitoring

### Check FPS
```c
// Add to main loop
static uint32_t frame_count = 0;
static uint32_t last_fps_check = 0;

if(HAL_GetTick() - last_fps_check >= 1000) {
    printf("FPS: %lu\n", frame_count);
    frame_count = 0;
    last_fps_check = HAL_GetTick();
}
frame_count++;
```

### Memory Usage
```c
// Check heap usage
extern uint8_t _end;
extern uint8_t _estack;
uint32_t heap_size = (uint32_t)&_estack - (uint32_t)&_end;
printf("Heap size: %lu bytes\n", heap_size);
```

## 📚 Next Steps

1. **Explore LVGL Features**
   - Read [GUI_DESIGN_GUIDE.md](GUI_DESIGN_GUIDE.md)
   - Check [LVGL documentation](https://docs.lvgl.io)

2. **Add More Screens**
   - Create custom screen functions
   - Add navigation buttons
   - Implement new features

3. **Integrate Hardware**
   - Connect accelerometer
   - Add temperature sensors
   - Implement CAN bus display
   - Add USB data logging

4. **Optimize Performance**
   - Enable DMA2D acceleration
   - Tune buffer sizes
   - Profile with SEGGER SystemView

## 🎯 Example Projects

### Project 1: Environmental Monitor
- Display temperature, humidity, pressure
- Log data to SD card
- Set alarms for threshold violations

### Project 2: Motion Tracker
- Display accelerometer/gyroscope data
- 3D orientation visualization
- Fall detection alerts

### Project 3: Industrial HMI
- Multi-zone temperature control
- Process status monitoring
- Alarm management system

## 📞 Support

### Resources
- **LVGL Forum**: https://forum.lvgl.io
- **STM32 Community**: https://community.st.com
- **Project Docs**: See [DOCUMENTATION_INDEX.md](../../DOCUMENTATION_INDEX.md)

### Common Commands Cheat Sheet

```bash
# Build
cmake --build build/Debug -j$(nproc)

# Clean
cmake --build build/Debug --target clean

# Flash
STM32_Programmer_CLI --connect port=swd --download build/Debug/Sensor_Console.elf -hardRst

# List available ST-LINKs
STM32_Programmer_CLI --list

# Full rebuild + flash
cmake --build build/Debug --target clean && \
cmake --build build/Debug -j$(nproc) && \
STM32_Programmer_CLI --connect port=swd --download build/Debug/Sensor_Console.elf -hardRst -rst --start
```

## ✅ Checklist

Before running:
- [ ] Board connected via USB
- [ ] Drivers installed (ST-LINK)
- [ ] Project compiled successfully
- [ ] Display turns on when powered
- [ ] Touch screen is clean and unobstructed

First test:
- [ ] Home screen visible
- [ ] Status shows "✓ System Ready"
- [ ] Temperature and humidity updating
- [ ] Touch response working
- [ ] Navigation buttons functional
- [ ] Screen transitions smooth

---

**Happy coding! 🎉**

For detailed GUI documentation, see [GUI_DESIGN_GUIDE.md](GUI_DESIGN_GUIDE.md)
