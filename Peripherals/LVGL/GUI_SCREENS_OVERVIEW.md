# GUI Screens Visual Overview

## Screen 1: Home Dashboard
```
╔════════════════════════════════════╗
║    STM32F429 Dashboard             ║
╠════════════════════════════════════╣
║                                    ║
║  ┌──────────────────────────────┐  ║
║  │  ✓ System Ready              │  ║
║  └──────────────────────────────┘  ║
║                                    ║
║      ╭─────╮         ┃            ║
║     ╱       ╲        ┃▓▓          ║
║    │   25°C  │       ┃▓▓          ║
║     ╲  Temp /        ┃▓▓          ║
║      ╰─────╯         ┃▓▓          ║
║                      ┃▓▓          ║
║   Temperature     60%┃▓▓ Humid    ║
║      Gauge          ┃              ║
║                                    ║
║  ┌──────────┐     ┌──────────┐    ║
║  │☰ Sensors │     │⚙ Config  │    ║
║  └──────────┘     └──────────┘    ║
╚════════════════════════════════════╝
```

**Features:**
- Arc gauge for temperature (animated)
- Vertical bar for humidity
- Status card with icon
- Two navigation buttons
- Dark theme with colorful accents

---

## Screen 2: Sensor Monitor
```
╔════════════════════════════════════╗
║ ← Back    👁 Sensor Monitor        ║
╠════════════════════════════════════╣
║                                    ║
║  ┌────────────────────────────┐   ║
║  │        Chart                │   ║
║  │    ╱╲    ╱╲                │   ║
║  │   ╱  ╲  ╱  ╲╱╲             │   ║
║  │  ╱    ╲╱     ╲             │   ║
║  └────────────────────────────┘   ║
║                                    ║
║   Real-time sensor data chart     ║
║                                    ║
║                                    ║
║  ┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐  ║
║  │Accel│ │Gyro │ │Temp │ │Press│  ║
║  │ 1.2g│ │45°/s│ │25°C │ │1013 │  ║
║  └─────┘ └─────┘ └─────┘ └─────┘  ║
║   (Red)  (Teal) (Yellow) (Purple) ║
╚════════════════════════════════════╝
```

**Features:**
- Live updating line chart
- 4 color-coded sensor cards
- Back button to home
- Smooth animations

---

## Screen 3: Settings
```
╔════════════════════════════════════╗
║ ← Back      ⚙ Settings             ║
╠════════════════════════════════════╣
║                                    ║
║  Brightness                        ║
║  ━━━━━━━━━━●────────   70%         ║
║                                    ║
║  Volume                            ║
║  ━━━━━●─────────────   50%         ║
║                                    ║
║                                    ║
║  WiFi                       ○──    ║
║                             OFF    ║
║                                    ║
║  Bluetooth                  ─●○    ║
║                              ON    ║
║                                    ║
║  ┌──────────────────────────────┐  ║
║  │    📞 System Info            │  ║
║  └──────────────────────────────┘  ║
║                                    ║
╚════════════════════════════════════╝
```

**Features:**
- Two adjustment sliders (brightness, volume)
- Toggle switches for connectivity
- Button to system info screen
- Interactive controls

---

## Screen 4: System Info
```
╔════════════════════════════════════╗
║ ← Back   📞 System Information     ║
╠════════════════════════════════════╣
║                                    ║
║  ┌──────────────────────────────┐  ║
║  │                              │  ║
║  │  MCU: STM32F429ZI            │  ║
║  │  Core: ARM Cortex-M4         │  ║
║  │  Freq: 180 MHz               │  ║
║  │  Flash: 2 MB                 │  ║
║  │  RAM: 256 KB                 │  ║
║  │  Display: 240x320            │  ║
║  │  LVGL: v8.3.x                │  ║
║  │                              │  ║
║  │  Status: Running             │  ║
║  │                              │  ║
║  └──────────────────────────────┘  ║
║                                    ║
║                                    ║
╚════════════════════════════════════╝
```

**Features:**
- Detailed system specifications
- Hardware information
- Status indicator
- Clean card layout

---

## Navigation Map
```
          ┌─────────────────┐
          │  Home Dashboard │ ◄─── Default Screen
          └────────┬────────┘
                   │
          ┌────────┴────────┐
          │                 │
    ┌─────▼──────┐    ┌────▼──────┐
    │  Sensors   │    │ Settings  │
    │  Monitor   │    │           │
    └─────┬──────┘    └────┬──────┘
          │                │
          │           ┌────▼──────┐
          │           │  System   │
          │           │   Info    │
          │           └────┬──────┘
          │                │
          └────────┬───────┘
                   │
          ┌────────▼────────┐
          │  Back to Home   │
          └─────────────────┘
```

---

## Color Legend

| Color | Hex Code | Usage |
|-------|----------|-------|
| 🟦 Dark Navy | #1a1a2e | Primary background |
| 🔷 Dark Blue | #16213e | Card backgrounds |
| 🔵 Ocean Blue | #0f4c75 | Borders and accents |
| 🟢 Bright Green | #3be477 | Success, active states |
| 🟡 Golden Yellow | #f7b731 | Warnings, settings |
| 🔴 Coral Red | #ff6b6b | Temperature, errors |
| 🔵 Turquoise | #4ecdc4 | Humidity, info |
| 🟣 Lavender | #a29bfe | Special features |
| ⚫ Gray | #5f6368 | Neutral, disabled |
| ⚪ White | #ffffff | Text, icons |

---

## Widget Types Used

1. **Labels** (`lv_label`) - Text display
2. **Buttons** (`lv_btn`) - Touch-interactive buttons
3. **Arc** (`lv_arc`) - Circular gauge for temperature
4. **Bar** (`lv_bar`) - Vertical progress bar for humidity
5. **Chart** (`lv_chart`) - Line graph for sensor data
6. **Slider** (`lv_slider`) - Adjustable value controls
7. **Switch** (`lv_switch`) - On/off toggles
8. **Container** (`lv_obj`) - Layout containers

---

## Screen Transitions

All screens use **fade animation** for smooth transitions:
- Transition time: 300ms
- Animation type: `LV_SCR_LOAD_ANIM_FADE_ON`
- Smooth and professional appearance

---

## Touch Targets

All interactive elements meet minimum size requirements:
- **Buttons**: 40px height minimum (comfortable touch)
- **Sliders**: 180px width, 10px track height
- **Switches**: Standard LVGL size (~50x25px)
- **Back buttons**: 60x35px (adequate for finger)

---

## Responsive Design

The GUI is optimized for the **240x320** resolution:
- Portrait orientation
- Touch-first interface
- No scrolling required on any screen
- All content fits within viewport
