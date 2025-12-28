# LVGL (Light and Versatile Graphics Library) - Professional GUI

## 🎨 What's New - Modern Multi-Screen GUI!

**A complete, professional GUI application is now implemented!** 

### ✨ Features
✅ **4 Beautiful Screens** - Home, Sensors, Settings, System Info  
✅ **Dark Modern Theme** - Professional appearance with vibrant accents  
✅ **Interactive Widgets** - Gauges, charts, sliders, switches, buttons  
✅ **Smooth Navigation** - Fade transitions between screens  
✅ **Real-time Updates** - Live sensor data visualization  
✅ **Touch-Optimized** - Large buttons, responsive controls  
✅ **Demo Mode** - Automatic animation for testing  

### 📺 Preview
```
Home Dashboard → Sensor Monitor → Settings → System Info
    ↓              ↓               ↓           ↓
Temperature    Live Charts     Sliders    Hardware Specs
Humidity       4 Sensors       Toggles    System Status
Status Card    Back Button     Info Btn   Back Button
```

## 📚 Documentation

Choose your documentation based on your needs:

### 🚀 [QUICK_START.md](QUICK_START.md) - **Start Here!**
**For beginners** - Step-by-step guide to:
- Build and flash the project
- First run and testing
- Troubleshooting common issues
- Hardware setup verification

### 🎨 [GUI_DESIGN_GUIDE.md](GUI_DESIGN_GUIDE.md) - **Design Details**
**For developers** - Complete design documentation:
- Screen-by-screen breakdown
- API usage and examples
- Color palette and typography
- Customization guide
- Performance optimization

### 📱 [GUI_SCREENS_OVERVIEW.md](GUI_SCREENS_OVERVIEW.md) - **Visual Reference**
**For visual learners** - ASCII diagrams of:
- All 4 screens layouts
- Navigation flow
- Widget types used
- Color coding
- Touch target sizes

## 🎯 Quick Start (TL;DR)

### 1. Build & Flash
```bash
cd /home/mahonri/Desktop/BareMetal/Sensor_Cons
cmake --build build/Debug -j$(nproc)
STM32_Programmer_CLI --connect port=swd --download build/Debug/Sensor_Console.elf -hardRst -rst --start
```

### 2. What You'll See
- **Home Dashboard** with temperature gauge and humidity bar
- **Animated demo** (temperature/humidity cycling automatically)
- Touch **green "Sensors"** button to see live chart
- Touch **yellow "Config"** button for settings
- **Smooth fade** transitions between screens

### 3. Update Display Values
```c
#include "lvgl_app.h"

// Update temperature (0-100°C)
LVGL_App_UpdateTemperature(25);

// Update humidity (0-100%)
LVGL_App_UpdateHumidity(60);

// Add sensor data point
LVGL_App_AddChartData(sensor_value);

// Update status message
LVGL_App_UpdateStatus(LV_SYMBOL_OK " All Systems OK");
```

## 📖 Original Overview
LVGL is a free and open-source embedded graphics library for creating beautiful user interfaces on embedded systems. This peripheral provides LVGL integration for the STM32F429I-Discovery board.

### What's Already Configured
✅ LVGL v8.3.11 automatically downloaded via CMake  
✅ Display configuration for 240x320 LCD (RGB565)  
✅ Complete multi-screen GUI application  
✅ Professional dark theme with modern widgets  
✅ Touch navigation and smooth animations  
✅ Demo mode with live updates  

### Implementation Status
✅ **Core LVGL** - Fully integrated and configured  
✅ **Display Port** - Ready for LTDC framebuffer  
✅ **Input Port** - Ready for touch integration  
✅ **GUI Application** - Complete 4-screen interface  
✅ **Helper Functions** - Easy-to-use update APIs  
✅ **Demo Mode** - Automatic animations included  

## 📁 File Structure

```
Peripherals/LVGL/
├── README.md                    ← You are here!
├── QUICK_START.md              ← Build, flash, and test guide
├── GUI_DESIGN_GUIDE.md         ← Complete design documentation
├── GUI_SCREENS_OVERVIEW.md     ← Visual screen layouts
├── lv_conf.h                   ← LVGL configuration
├── lvgl_app.h/c                ← Multi-screen GUI application
├── lv_port_disp.h/c            ← Display driver port
└── lv_port_indev.h/c           ← Touch input port
```

## 🔧 Configuration Files

### `lv_conf.h` - Main LVGL Settings
Controls LVGL behavior and features:
- **Color depth**: 16-bit RGB565 for STM32F429I display
- **Resolution**: 240x320 pixels
- **Features**: Minimal setup to save memory
- **Logging**: Disabled by default

**Beginner Tip**: Don't modify this unless you know what feature you need!

### `lvgl_app.c` - Your Starting Point
Simple wrapper functions:
- `LVGL_App_Init()` - Sets up LVGL and creates a "Hello LVGL" label
- `LVGL_App_Tick()` - Handles LVGL timers and rendering

**Modify this file** to create your own UI!

### `lv_port_disp.c` - Display Driver
Connects LVGL rendering to the actual LCD hardware.

**Current Status**: ⚠️ Framebuffer address not configured  
**Action Required**: Uncomment and set `LVGL_FB_ADDR` once display hardware is initialized

### `lv_port_indev.c` - Touch Input Driver  
Connects LVGL to touchscreen input.

**Current Status**: ⚠️ Touch driver not implemented  
**Action Required**: Implement `touch_read()` function with FT6206 or STMPE811 I2C driver

## 🔌 Hardware Setup

### Display (LTDC + ILI9341)
1. Initialize LTDC peripheral in STM32CubeMX
2. Configure framebuffer in SDRAM (address typically 0xD0000000)
3. Update `LVGL_FB_ADDR` in [lv_port_disp.c](lv_port_disp.c#L21)

Example:
```c
#define LVGL_FB_ADDR 0xD0000000  // Your SDRAM framebuffer address
```

### Touchscreen (Optional)
The STM32F429I-Discovery board uses one of:
- **FT6206** capacitive touch controller (I2C)
- **STMPE811** resistive touch controller (I2C)

Implement touch reading in [lv_port_indev.c](lv_port_indev.c#L17)

## 🎨 Creating Your First UI

Replace the content in `LVGL_App_Init()`:

```c
void LVGL_App_Init(void)
{
#if LVGL_AVAILABLE
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();

    // Create a simple button
    lv_obj_t *btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 120, 50);
    lv_obj_center(btn);
    
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "Click Me!");
    lv_obj_center(label);
#endif
}
```

## 📚 Learning Resources

- **LVGL Documentation**: https://docs.lvgl.io/8.3/
- **LVGL Examples**: https://docs.lvgl.io/8.3/examples.html
- **Widgets Overview**: https://docs.lvgl.io/8.3/widgets/index.html

## ⚙️ CMake Integration

LVGL is automatically fetched and configured via CMake. See [`cmake/lvgl.cmake`](../../cmake/lvgl.cmake) for details.

The main [CMakeLists.txt](../../CMakeLists.txt) already:
- ✅ Includes LVGL source code
- ✅ Links LVGL to your project
- ✅ Sets required defines (`LV_CONF_INCLUDE_SIMPLE`)

## 🐛 Troubleshooting

### Build Errors
- **"lvgl.h not found"**: CMake should auto-download. Check internet connection.
- **Linking errors**: Ensure `${LVGL_TARGET}` is in `target_link_libraries()` in CMakeLists.txt

### Runtime Issues
- **Nothing displays**: Check framebuffer address in `lv_port_disp.c`
- **Touch not working**: Implement touch driver in `lv_port_indev.c`
- **Crashes**: Ensure `LVGL_App_Tick()` is called regularly (every 1-5ms)

### Display Issues
- **Wrong colors**: Verify `LV_COLOR_DEPTH` matches your LCD (16 for RGB565)
- **Garbage on screen**: Check framebuffer memory allocation and LTDC config

## 🎓 For Beginners

**Don't understand something?** That's normal! Here's the learning path:

1. **Start Simple**: Use the default `LVGL_App_Init()` to see "Hello LVGL"
2. **Explore Widgets**: Try creating buttons, labels, sliders from LVGL docs
3. **Handle Input**: Later, add touch support when you're comfortable
4. **Advanced**: Custom styles, animations, layouts

## 📝 Next Steps

- [ ] Initialize display hardware (LTDC)
- [ ] Set framebuffer address in `lv_port_disp.c`
- [ ] Test basic rendering with "Hello LVGL"
- [ ] Implement touch input driver (optional)
- [ ] Create your custom UI in `lvgl_app.c`

## 💡 Tips

- Call `LVGL_App_Tick()` from a timer interrupt for smooth animations
- Start with simple widgets before complex layouts
- Use LVGL simulator on PC for rapid prototyping
- Keep `lv_conf.h` minimal to save flash/RAM

---
**Need Help?** Check the LVGL documentation or STM32 community forums!
