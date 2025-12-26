# Industrial HMI Project - Complete TODO Checklist

**Start Date**: December 12, 2025  
**Estimated Completion**: February 2026 (8 weeks)  
**Status Tracking**: ☐ Not Started | 🔄 In Progress | ✅ Complete | ⏸️ Blocked

---

## WEEK 1: Foundation & Setup (5-8 hours)

### Day 1: Environment Setup & Verification
- [ ] ☐ **TODO 1.1**: Build base project successfully
  - Open terminal in project root
  - Run: `mkdir -p build/Debug && cd build/Debug`
  - Run: `cmake -DCMAKE_BUILD_TYPE=Debug ../..`
  - Run: `make -j4`
  - **Success**: No compilation errors, `Sensor_Console.elf` created

- [ ] ☐ **TODO 1.2**: Flash firmware to board
  - Connect STM32F429I-DISC1 via USB
  - Run: `STM32_Programmer_CLI --connect port=swd --download build/Debug/Sensor_Console.elf -hardRst -rst --start`
  - **Success**: Green LED on board turns on

- [ ] ☐ **TODO 1.3**: Verify UART debugging
  - Connect FT232RL: TX→PA10, RX→PA9, GND→GND
  - Open terminal: `screen /dev/ttyUSB0 115200`
  - Modify `main.c` to add: `printf("System Ready\r\n");`
  - Rebuild and flash
  - **Success**: See "System Ready" in terminal

### Day 2: Test All Peripheral Drivers
- [ ] ☐ **TODO 1.4**: Test DHT22 sensor
  - Wire: VCC→3.3V, GND→GND, DATA→PA1 (add 10kΩ pullup)
  - Add to main: `#include "Peripherals/DHT/dht.h"`
  - Read temperature/humidity
  - Print values to UART
  - **Success**: See valid temp/humidity readings

- [ ] ☐ **TODO 1.5**: Test Relay control
  - Wire relay: VCC→5V, GND→GND, IN→PB0
  - Add to main: `#include "Peripherals/RELAY/relay.h"`
  - Toggle relay on/off every 2 seconds
  - **Success**: Hear relay clicking

- [ ] ☐ **TODO 1.6**: Test Buzzer
  - Wire active buzzer: VCC→3.3V, GND→GND, I/O→PC0
  - Add to main: `#include "Peripherals/BUZZER/buzzer.h"`
  - Beep buzzer 3 times
  - **Success**: Hear beeps

- [ ] ☐ **TODO 1.7**: Test LCD Display
  - Built-in on board, no wiring needed
  - Add to main: `#include "Peripherals/LTDC/ltdc.h"`
  - Clear screen and display "Hello World"
  - **Success**: See text on LCD

- [ ] ☐ **TODO 1.8**: Test Touchscreen
  - Built-in on board
  - Add to main: `#include "Peripherals/TOUCHSCREEN/touchscreen.h"`
  - Detect and print touch coordinates
  - **Success**: Touch screen, see coordinates in UART

### Day 3: Create Project Structure
- [ ] ☐ **TODO 1.9**: Create directory structure
  ```bash
  mkdir -p Projects/IndustrialHMI/src/{config,app/{screens},ui/{core,widgets},middleware/{data,alarm,logger,comm}}
  mkdir -p Projects/IndustrialHMI/inc
  ```
  - **Success**: All directories created

- [ ] ☐ **TODO 1.10**: Create types.h
  - File: `Projects/IndustrialHMI/src/config/types.h`
  - Define: `SensorValue_t`, `AlarmLevel_t`, `ScreenID_t`, `WidgetType_t`
  - Add include guards
  - **Success**: File compiles without errors

- [ ] ☐ **TODO 1.11**: Create hmi_config.h
  - File: `Projects/IndustrialHMI/src/config/hmi_config.h`
  - Define: LCD dimensions, max sensors, timing constants
  - Add all `#define` constants
  - **Success**: File compiles without errors

---

## WEEK 2: Data Management Layer (6-8 hours)

### Day 4: Data Manager Implementation
- [ ] ☐ **TODO 2.1**: Create data_manager.h
  - File: `Projects/IndustrialHMI/src/middleware/data/data_manager.h`
  - Define `SensorTag_t` struct
  - Declare functions: `DataMgr_Init()`, `DataMgr_UpdateSensor()`, `DataMgr_GetSensor()`
  - **Success**: Header compiles

- [ ] ☐ **TODO 2.2**: Implement data_manager.c
  - File: `Projects/IndustrialHMI/src/middleware/data/data_manager.c`
  - Create static array: `SensorTag_t sensors[10]`
  - Implement `DataMgr_Init()`: Initialize all sensor names and defaults
  - Implement `DataMgr_UpdateSensor()`: Store value, timestamp, min/max
  - Implement `DataMgr_GetSensor()`: Return current sensor value
  - **Success**: Can store and retrieve sensor values

- [ ] ☐ **TODO 2.3**: Test data manager
  - Create test in main.c
  - Update sensor 0 with value 25.5
  - Read back and print to UART
  - **Success**: Correct value retrieved

### Day 5: Alarm Manager Implementation
- [ ] ☐ **TODO 2.4**: Create alarm_manager.h
  - File: `Projects/IndustrialHMI/src/middleware/alarm/alarm_manager.h`
  - Define `Alarm_t` struct
  - Declare functions: `AlarmMgr_Init()`, `AlarmMgr_CheckAlarms()`, `AlarmMgr_SetAlarm()`
  - **Success**: Header compiles

- [ ] ☐ **TODO 2.5**: Implement alarm_manager.c
  - File: `Projects/IndustrialHMI/src/middleware/alarm/alarm_manager.c`
  - Create static array: `Alarm_t alarms[20]`
  - Implement `AlarmMgr_Init()`: Clear all alarms
  - Implement `AlarmMgr_SetAlarm()`: Configure threshold and sensor ID
  - Implement `AlarmMgr_CheckAlarms()`: Compare sensor values to thresholds
  - Implement `AlarmMgr_TriggerAction()`: Activate buzzer/LED/relay
  - **Success**: Alarm triggers when threshold exceeded

- [ ] ☐ **TODO 2.6**: Test alarm manager
  - Set alarm for temp > 30°C
  - Update sensor to 35°C
  - Verify buzzer sounds
  - **Success**: Alarm triggers correctly

---

## WEEK 3: UI Framework Core (8-10 hours)

### Day 6: UI Core Engine
- [ ] ☐ **TODO 3.1**: Create ui_core.h
  - File: `Projects/IndustrialHMI/src/ui/core/ui_core.h`
  - Define `UI_Widget_t` and `UI_Screen_t` structs
  - Declare: `UI_Init()`, `UI_Update()`, `UI_Render()`, `UI_HandleTouch()`
  - **Success**: Header compiles

- [ ] ☐ **TODO 3.2**: Implement ui_core.c
  - File: `Projects/IndustrialHMI/src/ui/core/ui_core.c`
  - Implement `UI_Init()`: Initialize LTDC, clear screen
  - Implement `UI_Update()`: Process touch input
  - Implement `UI_Render()`: Call draw on all widgets
  - Implement `UI_HandleTouch()`: Detect which widget was touched
  - **Success**: UI initializes, can render basic shapes

- [ ] ☐ **TODO 3.3**: Test UI rendering
  - Draw rectangle at (10, 10, 50, 50)
  - Draw filled circle at (120, 60)
  - **Success**: Shapes visible on screen

### Day 7: Screen Manager
- [ ] ☐ **TODO 3.4**: Create ui_screen.h
  - File: `Projects/IndustrialHMI/src/ui/core/ui_screen.h`
  - Declare: `ScreenMgr_Init()`, `ScreenMgr_SwitchScreen()`, `ScreenMgr_RegisterScreen()`
  - **Success**: Header compiles

- [ ] ☐ **TODO 3.5**: Implement ui_screen.c
  - File: `Projects/IndustrialHMI/src/ui/core/ui_screen.c`
  - Create static array: `UI_Screen_t screens[6]`
  - Implement `ScreenMgr_SwitchScreen()`: Clear old screen, load new one
  - Implement `ScreenMgr_RegisterScreen()`: Store screen in array
  - **Success**: Can switch between screens

- [ ] ☐ **TODO 3.6**: Test screen switching
  - Create 2 test screens
  - Switch between them on touch
  - **Success**: Screens change on touch

### Day 8: Touch Handler
- [ ] ☐ **TODO 3.7**: Implement ui_touch.c
  - File: `Projects/IndustrialHMI/src/ui/core/ui_touch.c`
  - Implement debouncing (20ms delay)
  - Call `Touchscreen_GetState()` from peripheral driver
  - Route touch to `UI_HandleTouch()`
  - **Success**: Touch events debounced and processed

---

## WEEK 4: Widget Library (10-12 hours)

### Day 9: Button Widget
- [ ] ☐ **TODO 4.1**: Create widget_button.h and .c
  - Files: `Projects/IndustrialHMI/src/ui/widgets/widget_button.{h,c}`
  - Define `ButtonWidget_t` struct
  - Implement `Button_Create()`: Allocate and initialize button
  - Implement `Button_Draw()`: Draw rectangle with text
  - Implement `Button_OnTouch()`: Check if touch inside bounds, call callback
  - **Success**: Button displays and responds to touch

- [ ] ☐ **TODO 4.2**: Test button widget
  - Create button at (50, 100) with label "Test"
  - Add callback to print "Button Pressed" to UART
  - Touch button
  - **Success**: See "Button Pressed" in UART

### Day 10: Label Widget
- [ ] ☐ **TODO 4.3**: Create widget_label.h and .c
  - Files: `Projects/IndustrialHMI/src/ui/widgets/widget_label.{h,c}`
  - Define `LabelWidget_t` struct
  - Implement `Label_Create()`: Initialize with text
  - Implement `Label_Draw()`: Render text at position
  - Implement `Label_Update()`: Change text dynamically
  - **Success**: Label displays text, can be updated

- [ ] ☐ **TODO 4.4**: Test label widget
  - Create label showing "Temp: 25.5°C"
  - Update every second with new value
  - **Success**: Label updates on screen

### Day 11: Gauge Widget
- [ ] ☐ **TODO 4.5**: Create widget_gauge.h and .c
  - Files: `Projects/IndustrialHMI/src/ui/widgets/widget_gauge.{h,c}`
  - Define `GaugeWidget_t` struct
  - Implement `Gauge_Create()`: Set min/max/unit
  - Implement `Gauge_Draw()`: Draw arc and needle
  - Implement `Gauge_SetValue()`: Update needle position
  - **Success**: Gauge displays with rotating needle

- [ ] ☐ **TODO 4.6**: Test gauge widget
  - Create temp gauge (0-100°C)
  - Update value from 0 to 100 in loop
  - **Success**: Needle sweeps across gauge

### Day 12: Chart Widget
- [ ] ☐ **TODO 4.7**: Create widget_chart.h and .c
  - Files: `Projects/IndustrialHMI/src/ui/widgets/widget_chart.{h,c}`
  - Define `ChartWidget_t` struct with ring buffer
  - Implement `Chart_Create()`: Allocate data buffer
  - Implement `Chart_AddPoint()`: Add value to ring buffer
  - Implement `Chart_Draw()`: Plot line graph with auto-scaling
  - **Success**: Chart plots trend line

- [ ] ☐ **TODO 4.8**: Test chart widget
  - Create chart, add 50 random points
  - **Success**: Line graph visible on screen

---

## WEEK 5: Application Screens (8-10 hours)

### Day 13: Main Menu Screen
- [ ] ☐ **TODO 5.1**: Create screen_main.c
  - File: `Projects/IndustrialHMI/src/app/screens/screen_main.c`
  - Add 6 buttons in grid layout
  - Button callbacks switch to other screens
  - Add header with title "HMI Panel"
  - **Success**: Main menu displays with working buttons

- [ ] ☐ **TODO 5.2**: Test main menu
  - Touch each button
  - Verify screen switching works
  - **Success**: Can navigate to all screens

### Day 14: Monitor Screen
- [ ] ☐ **TODO 5.3**: Create screen_monitor.c
  - File: `Projects/IndustrialHMI/src/app/screens/screen_monitor.c`
  - Add 2 gauges (temperature, pressure)
  - Add 4 labels for values
  - Add back button to return to menu
  - Update gauges every 100ms from data manager
  - **Success**: Monitor screen shows live sensor values

- [ ] ☐ **TODO 5.4**: Test monitor screen
  - Update DHT22 sensor
  - Watch gauge move in real-time
  - **Success**: Gauges reflect sensor values

### Day 15: Alarm Screen
- [ ] ☐ **TODO 5.5**: Create screen_alarm.c
  - File: `Projects/IndustrialHMI/src/app/screens/screen_alarm.c`
  - Display list of active alarms
  - Show alarm level (red/yellow/orange)
  - Add ACK button for each alarm
  - Add back button
  - **Success**: Alarm screen shows active alarms

- [ ] ☐ **TODO 5.6**: Test alarm screen
  - Trigger 2 alarms
  - Navigate to alarm screen
  - Acknowledge alarms
  - **Success**: Alarms display and can be acknowledged

### Day 16: Trend Screen
- [ ] ☐ **TODO 5.7**: Create screen_trend.c
  - File: `Projects/IndustrialHMI/src/app/screens/screen_trend.c`
  - Add chart widget
  - Plot temperature history (last 100 points)
  - Add time range buttons (1hr, 8hr, 24hr)
  - Add legend with color coding
  - **Success**: Trend screen plots historical data

### Day 17: Config Screen
- [ ] ☐ **TODO 5.8**: Create screen_config.c
  - File: `Projects/IndustrialHMI/src/app/screens/screen_config.c`
  - Add slider for alarm threshold
  - Add buttons to set date/time
  - Add brightness control
  - Save settings to flash
  - **Success**: Config screen allows parameter changes

---

## WEEK 6: Communication Layer (8-10 hours)

### Day 18: Purchase RS485 Module
- [ ] ☐ **TODO 6.1**: Order MAX485 module
  - Search Amazon/eBay for "MAX485 RS485 module"
  - Order quantity: 1-2 modules
  - Cost: $2-5
  - **Success**: Module ordered

- [ ] ⏸️ **TODO 6.2**: Wait for RS485 module delivery (3-7 days)

### Day 19: Install RS485 Hardware
- [ ] ☐ **TODO 6.3**: Wire RS485 module
  - VCC → 5V
  - GND → GND
  - DI → PA2 (USART2_TX)
  - RO → PA3 (USART2_RX)
  - DE/RE → PB0 (GPIO for direction control)
  - **Success**: Module wired correctly

### Day 20: Modbus RTU Implementation
- [ ] ☐ **TODO 6.4**: Create modbus_rtu.h
  - File: `Projects/IndustrialHMI/src/middleware/comm/modbus_rtu.h`
  - Define `ModbusRequest_t` struct
  - Declare: `ModbusRTU_Init()`, `ModbusRTU_ReadRegisters()`, `ModbusRTU_WriteRegister()`
  - **Success**: Header compiles

- [ ] ☐ **TODO 6.5**: Implement modbus_rtu.c
  - File: `Projects/IndustrialHMI/src/middleware/comm/modbus_rtu.c`
  - Implement Function Code 03 (Read Holding Registers)
  - Implement Function Code 06 (Write Single Register)
  - Implement CRC-16 calculation
  - Add timeout handling (1000ms)
  - **Success**: Can read/write Modbus registers

- [ ] ☐ **TODO 6.6**: Test Modbus communication
  - Use Modbus simulator or second device
  - Read register 0x0001
  - Write value 1234 to register 0x0001
  - **Success**: Communication verified with analyzer

### Day 21: CAN Bus Integration
- [ ] ☐ **TODO 6.7**: Wire Waveshare CAN module
  - Already have module
  - Add 120Ω termination resistors
  - Verify wiring: PA12(TX), PA11(RX)
  - **Success**: CAN module connected

- [ ] ☐ **TODO 6.8**: Create can_protocol.c
  - File: `Projects/IndustrialHMI/src/middleware/comm/can_protocol.c`
  - Map sensor values to CAN message IDs
  - Implement message parsing
  - Use existing CAN driver from `Peripherals/CAN/`
  - **Success**: Can send/receive sensor data via CAN

---

## WEEK 7: Data Logging & Integration (6-8 hours)

### Day 22: Data Logger Implementation
- [ ] ☐ **TODO 7.1**: Create data_logger.h
  - File: `Projects/IndustrialHMI/src/middleware/logger/data_logger.h`
  - Define `LogEntry_t` struct
  - Declare: `Logger_Init()`, `Logger_LogData()`, `Logger_ReadLog()`
  - **Success**: Header compiles

- [ ] ☐ **TODO 7.2**: Implement data_logger.c
  - File: `Projects/IndustrialHMI/src/middleware/logger/data_logger.c`
  - Use internal Flash Sector 11 (128KB at 0x080E0000)
  - Implement circular buffer
  - Log every 10 seconds
  - Include timestamp, temp, pressure, alarms
  - **Success**: Data persists across power cycles

- [ ] ☐ **TODO 7.3**: Test data logging
  - Log 100 entries
  - Power cycle board
  - Read back logs
  - **Success**: All 100 entries retrieved correctly

### Day 23: FreeRTOS Task Creation
- [ ] ☐ **TODO 7.4**: Create app_main.c
  - File: `Projects/IndustrialHMI/src/app/app_main.c`
  - Create task: `vTaskUI` (512 words, priority 2)
  - Create task: `vTaskSensor` (256 words, priority 2)
  - Create task: `vTaskAlarm` (256 words, priority 2)
  - Create task: `vTaskComm` (256 words, priority 1)
  - **Success**: All 4 tasks created

- [ ] ☐ **TODO 7.5**: Implement task functions
  - `vTaskUI`: Update UI every 100ms
  - `vTaskSensor`: Read DHT22 every 1 second
  - `vTaskAlarm`: Check alarms every 100ms
  - `vTaskComm`: Handle Modbus/CAN every 500ms
  - **Success**: Tasks execute without blocking

- [ ] ☐ **TODO 7.6**: Start FreeRTOS scheduler
  - Call `vTaskStartScheduler()` in main
  - Verify all tasks running
  - **Success**: System runs with multi-tasking

---

## WEEK 8: Testing & Polish (8-10 hours)

### Day 24: Integration Testing
- [ ] ☐ **TODO 8.1**: Test full data flow
  - DHT22 sensor → Data Manager → Monitor Screen
  - Verify values update on screen
  - **Success**: End-to-end data flow works

- [ ] ☐ **TODO 8.2**: Test alarm triggering
  - Set alarm for temp > 30°C
  - Heat DHT22 sensor
  - Verify buzzer sounds, LED turns on
  - Acknowledge alarm on screen
  - **Success**: Complete alarm workflow works

- [ ] ☐ **TODO 8.3**: Test screen navigation
  - Navigate through all 6 screens
  - Verify touch responsiveness
  - Test back buttons
  - **Success**: No navigation bugs

- [ ] ☐ **TODO 8.4**: Test trend logging
  - Let system run for 30 minutes
  - View trend screen
  - Verify chart shows history
  - **Success**: Trend data accurate

### Day 25: Stability Testing
- [ ] ☐ **TODO 8.5**: 24-hour stability test
  - Flash firmware
  - Let system run for 24 hours
  - Monitor for crashes, memory leaks
  - Check UART log for errors
  - **Success**: No crashes, system stable

- [ ] ☐ **TODO 8.6**: Memory usage check
  - Check heap usage: Should be < 16KB (50% of 32KB)
  - Check stack usage: No stack overflow
  - Use debugger to monitor
  - **Success**: Memory usage acceptable

### Day 26: Performance Optimization
- [ ] ☐ **TODO 8.7**: Profile UI rendering
  - Measure FPS (target: 10 FPS minimum)
  - Optimize slow draw functions
  - Reduce screen flicker
  - **Success**: Smooth UI performance

- [ ] ☐ **TODO 8.8**: Optimize communication
  - Measure Modbus response time (target: < 500ms)
  - Reduce CAN latency
  - **Success**: Communication responsive

### Day 27: Code Cleanup
- [ ] ☐ **TODO 8.9**: Remove debug code
  - Comment out debug `printf()` statements
  - Or make them conditional with `#ifdef DEBUG`
  - **Success**: Clean release build

- [ ] ☐ **TODO 8.10**: Add code comments
  - Document all functions
  - Add file headers with description
  - **Success**: Code well-documented

- [ ] ☐ **TODO 8.11**: Format code
  - Run: `clang-format -i src/**/*.c`
  - Check for compiler warnings
  - **Success**: No warnings, clean code

### Day 28: Final Testing
- [ ] ☐ **TODO 8.12**: User acceptance test
  - Test all features as end user would
  - Verify all screens work
  - Test alarm workflow
  - Test configuration saving
  - **Success**: All features working as expected

- [ ] ☐ **TODO 8.13**: Create release build
  - Set build type to Release
  - Run: `cmake -DCMAKE_BUILD_TYPE=Release`
  - Build: `make -j4`
  - **Success**: Release firmware created

- [ ] ☐ **TODO 8.14**: Tag release in git
  - Commit all changes
  - Run: `git tag -a v1.0.0 -m "First release"`
  - Run: `git push origin v1.0.0`
  - **Success**: Version tagged

---

## OPTIONAL ENHANCEMENTS (Future)

### WiFi Integration (Optional)
- [ ] ☐ **TODO 9.1**: Purchase ESP8266 module ($3-8)
- [ ] ☐ **TODO 9.2**: Wire ESP8266 to USART3 (PB10/PB11)
- [ ] ☐ **TODO 9.3**: Implement AT command interface
- [ ] ☐ **TODO 9.4**: Create web dashboard (HTML/CSS/JS)
- [ ] ☐ **TODO 9.5**: Test remote monitoring via WiFi

### Enclosure Design (Optional)
- [ ] ☐ **TODO 10.1**: Measure board dimensions
- [ ] ☐ **TODO 10.2**: Design 3D model in CAD software
- [ ] ☐ **TODO 10.3**: Add mounting holes for board
- [ ] ☐ **TODO 10.4**: Add cutouts for USB, LCD, buttons
- [ ] ☐ **TODO 10.5**: 3D print or order enclosure

---

## 📊 Progress Tracking

**How to use this checklist:**
1. Start from Week 1, Day 1
2. Complete TODOs in order (don't skip)
3. Mark ✅ when complete
4. Mark 🔄 when in progress
5. Mark ⏸️ if blocked
6. Update dates as you progress

**Estimated Time by Week:**
- Week 1: 5-8 hours (Setup & Testing)
- Week 2: 6-8 hours (Data Layer)
- Week 3: 8-10 hours (UI Framework)
- Week 4: 10-12 hours (Widgets)
- Week 5: 8-10 hours (Screens)
- Week 6: 8-10 hours (Communication)
- Week 7: 6-8 hours (Integration)
- Week 8: 8-10 hours (Testing & Polish)

**Total**: 59-76 hours (7-10 weeks at 1 hour/day, or 2-3 weeks full-time)

---

## 🎯 Critical Path Items (Must Complete)

These are blocking tasks - complete these first:

1. ✅ TODO 1.1-1.3: Build system working
2. ✅ TODO 1.4-1.8: All peripherals tested
3. ✅ TODO 1.10-1.11: Types and config defined
4. ✅ TODO 2.1-2.6: Data and alarm managers working
5. ✅ TODO 3.1-3.7: UI framework functional
6. ✅ TODO 4.1-4.8: Widgets working
7. ✅ TODO 5.1-5.8: All screens complete
8. ✅ TODO 7.4-7.6: FreeRTOS tasks running
9. ✅ TODO 8.12-8.14: Final testing and release

---

## 📞 Support & Resources

- **Stuck on a TODO?** Check [GETTING_STARTED.md](../../GETTING_STARTED.md)
- **Need code examples?** See [CODE_IMPLEMENTATION_ROADMAP.md](CODE_IMPLEMENTATION_ROADMAP.md)
- **Hardware questions?** Check [HARDWARE_INVENTORY.md](../../HARDWARE_INVENTORY.md)
- **Quick reference?** See [QUICK_REFERENCE.md](../../QUICK_REFERENCE.md)

---

**Last Updated**: December 12, 2025  
**Current Status**: Ready to start Week 1  
**Next TODO**: 1.1 - Build base project
