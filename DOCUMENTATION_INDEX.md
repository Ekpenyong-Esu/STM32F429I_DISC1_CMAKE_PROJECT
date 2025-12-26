# 📚 Documentation Index

Welcome to the STM32F429I Sensor Console Project! This index will guide you to the right documentation based on your needs.

---

## 🎯 I Want To...

### Start Building the Project
👉 **[GETTING_STARTED.md](GETTING_STARTED.md)** - Complete step-by-step guide from setup to FreeRTOS
- Beginner-friendly explanations
- Detailed wiring diagrams
- Troubleshooting tips
- Code examples with explanations
- **Estimated time:** 14 hours (2-3 days)

### Get Quick Code Snippets
👉 **[QUICK_REFERENCE.md](QUICK_REFERENCE.md)** - Copy-paste ready code for all peripherals
- Condensed implementation steps
- Pin assignment table
- Common troubleshooting
- **Estimated time:** 2 hours (experienced developers)

### Understand the Project Flow
👉 **[PROJECT_FLOWCHART.md](PROJECT_FLOWCHART.md)** - Visual roadmap and decision trees
- Complete implementation flowchart
- Dependency visualization
- Milestone checklist
- Learning path guidance

### See What Hardware I Need
👉 **[HARDWARE_INVENTORY.md](HARDWARE_INVENTORY.md)** - Complete hardware integration guide
- What you already have
- Wiring diagrams for each module
- Pin assignments
- Recommended purchase order

### Check Shopping List
👉 **[SHOPPING_LIST.md](SHOPPING_LIST.md)** - Components needed/owned
- Owned hardware marked ✅
- Still needed components
- Price estimates
- Where to buy

### Understand Project Structure
👉 **[README.md](README.md)** - Project overview and build instructions
- Project goals
- Directory structure
- Build system usage
- Available peripherals

---

## 📖 By Experience Level

### 🌱 Beginner (New to STM32/Embedded)

**Start here:**
1. [README.md](README.md) - Understand what this project does
2. [GETTING_STARTED.md](GETTING_STARTED.md) - Follow Phase 1 → Phase 9
3. [PROJECT_FLOWCHART.md](PROJECT_FLOWCHART.md) - Track your milestones
4. [HARDWARE_INVENTORY.md](HARDWARE_INVENTORY.md) - Wire each peripheral

**Learning order:** UART → Relay → Buzzer → DHT22 → RTC → CAN → LCD → Flash → FreeRTOS

### 🔧 Intermediate (Familiar with STM32)

**Start here:**
1. [QUICK_REFERENCE.md](QUICK_REFERENCE.md) - Copy example code
2. [HARDWARE_INVENTORY.md](HARDWARE_INVENTORY.md) - Verify pin assignments
3. [GETTING_STARTED.md](GETTING_STARTED.md) - Reference as needed

**Recommended:** Build all peripherals in parallel, test individually

### 🚀 Advanced (Embedded Expert)

**Start here:**
1. [QUICK_REFERENCE.md](QUICK_REFERENCE.md) - All you need
2. [Peripherals/*/README.md](Peripherals/) - API documentation for each driver
3. Source code directly in `Peripherals/XXX/*.h` and `*.c`

**Time estimate:** 2-4 hours to complete full integration

---

## 🔍 By Peripheral

Find documentation for specific peripherals:

| Peripheral | Quick Start | API Docs | Example Code |
|-----------|-------------|----------|--------------|
| **UART** | [QUICK_REF§2](QUICK_REFERENCE.md#-phase-2-uart-console-10-min) | [Peripherals/UART/](Peripherals/UART/) | `uart_example.c` |
| **Relay** | [QUICK_REF§3](QUICK_REFERENCE.md#-phase-3-relay-5-min) | [Peripherals/RELAY/](Peripherals/RELAY/) | `relay_example.c` |
| **Buzzer** | [QUICK_REF§4](QUICK_REFERENCE.md#-phase-4-buzzer-10-min) | [Peripherals/BUZZER/](Peripherals/BUZZER/) | `buzzer_example.c` |
| **DHT22** | [QUICK_REF§5](QUICK_REFERENCE.md#-phase-5-dht22-sensor-15-min) | [Peripherals/DHT/](Peripherals/DHT/) | `dht_example.c` |
| **RTC** | [QUICK_REF§6](QUICK_REFERENCE.md#-phase-6-ds3231-rtc-15-min) | [Peripherals/RTC/](Peripherals/RTC/) | `rtc_example.c` |
| **CAN** | [QUICK_REF§7](QUICK_REFERENCE.md#-phase-7-can-bus-20-min) | [Peripherals/CAN/](Peripherals/CAN/) | `can_example.c` |
| **LCD** | [QUICK_REF§8](QUICK_REFERENCE.md#-phase-8-lcd-display-10-min) | [Peripherals/LTDC/](Peripherals/LTDC/) | `ltdc_example.c` |
| **Flash** | [QUICK_REF§9](QUICK_REFERENCE.md#-phase-9-flash-data-logging-10-min) | [Peripherals/FLASH/](Peripherals/FLASH/) | `flash_example.c` |
| **I2C** | [GETTING_STARTED](GETTING_STARTED.md#step-61-connect-ds3231-rtc-module) | [Peripherals/I2C/](Peripherals/I2C/) | `i2c_example.c` |
| **SPI** | [Peripherals/SPI/README.md](Peripherals/SPI/) | [Peripherals/SPI/](Peripherals/SPI/) | `spi_example.c` |
| **GPIO** | [Peripherals/GPIO/README.md](Peripherals/GPIO/) | [Peripherals/GPIO/](Peripherals/GPIO/) | `gpio_example.c` |
| **PWR** | [Peripherals/PWR/README.md](Peripherals/PWR/) | [Peripherals/PWR/](Peripherals/PWR/) | `pwr_example.c` |
| **Watchdog** | [Peripherals/IWDG/](Peripherals/IWDG/) | [Peripherals/WWDG/](Peripherals/WWDG/) | `iwdg_example.c` |

---

## 🎓 By Learning Goal

### I want to learn GPIO basics
1. Start: [GETTING_STARTED.md - Phase 3](GETTING_STARTED.md#phase-3-gpio-control---relay--buzzer-1-hour)
2. Build: Relay control
3. Expand: Add buzzer
4. Reference: [Peripherals/GPIO/README.md](Peripherals/GPIO/)

### I want to learn communication protocols
1. **Single-wire:** [DHT22 sensor](GETTING_STARTED.md#phase-4-i2c-sensors---dht1122-2-hours)
2. **I2C:** [DS3231 RTC](GETTING_STARTED.md#phase-5-real-time-clock---ds3231-15-hours)
3. **SPI:** [Peripherals/SPI/README.md](Peripherals/SPI/)
4. **CAN:** [CAN bus integration](GETTING_STARTED.md#phase-6-can-bus-communication-2-hours)
5. **UART:** [Serial console](GETTING_STARTED.md#phase-2-serial-console--debugging-1-hour)

### I want to learn real-time systems
1. Complete: Phases 1-8 (get all peripherals working)
2. Study: [GETTING_STARTED.md - Phase 9](GETTING_STARTED.md#phase-9-freertos-multi-tasking-2-hours)
3. Reference: `Core/Inc/FreeRTOSConfig.h`
4. Build: Multi-task sensor monitoring

### I want to learn display/graphics
1. Complete: Phases 1-2 (build + UART for debugging)
2. Jump to: [GETTING_STARTED.md - Phase 7](GETTING_STARTED.md#phase-7-advanced-integration---lcd-display-2-hours)
3. Reference: [Peripherals/LTDC/README.md](Peripherals/LTDC/)
4. Expand: Add touchscreen input

### I want to learn data logging
1. Complete: Phase 4 (get sensor data)
2. Jump to: [GETTING_STARTED.md - Phase 8](GETTING_STARTED.md#phase-8-data-logging-to-flash-15-hours)
3. Reference: [Peripherals/FLASH/README.md](Peripherals/FLASH/)
4. Implement: Circular buffer for logs

---

## 🛠️ By Task

### Building the Project
```bash
# See: README.md - "Building the Project" section
mkdir -p build/Debug && cd build/Debug
cmake -DCMAKE_BUILD_TYPE=Debug ../..
make -j4
```

### Flashing Firmware
```bash
# See: QUICK_REFERENCE.md - Phase 1
STM32_Programmer_CLI --connect port=swd \
    --download build/Debug/Sensor_Console.elf \
    -hardRst -rst --start
```

### Adding a New Peripheral
1. Create `Peripherals/NEWMODULE/` directory
2. Add `newmodule.h`, `newmodule.c`, `newmodule_example.c`
3. Follow pattern from existing peripherals (e.g., `Peripherals/RELAY/`)
4. Update `CMakeLists.txt` if needed
5. Document in `Peripherals/NEWMODULE/README.md`

### Debugging Issues
1. **Build errors:** [README.md - Troubleshooting](README.md#-troubleshooting)
2. **Flash errors:** [GETTING_STARTED.md - Troubleshooting](GETTING_STARTED.md#flash-programming-errors)
3. **Sensor errors:** [GETTING_STARTED.md - Sensor Errors](GETTING_STARTED.md#sensor-errors)
4. **Quick fixes:** [QUICK_REFERENCE.md - Troubleshooting](QUICK_REFERENCE.md#-quick-troubleshooting)

---

## 📊 Documentation Map

```
PROJECT ROOT
│
├─ README.md .......................... Project overview & build guide
├─ GETTING_STARTED.md ................. 👈 START HERE (beginners)
├─ QUICK_REFERENCE.md ................. Copy-paste code snippets
├─ PROJECT_FLOWCHART.md ............... Visual roadmap & milestones
├─ HARDWARE_INVENTORY.md .............. Wiring diagrams & hardware specs
├─ SHOPPING_LIST.md ................... Components checklist
├─ DOCUMENTATION_INDEX.md ............. 📍 You are here
│
├─ Peripherals/
│  ├─ UART/README.md .................. UART API documentation
│  ├─ RELAY/README.md ................. Relay driver API
│  ├─ BUZZER/ ......................... Buzzer driver (no README yet)
│  ├─ DHT/ ............................ DHT sensor driver (no README yet)
│  ├─ RTC/README.md ................... RTC API documentation
│  ├─ CAN/README.md ................... CAN bus API
│  ├─ I2C/README.md ................... I2C API
│  ├─ SPI/README.md ................... SPI API
│  ├─ FLASH/README.md ................. Flash memory API
│  ├─ LTDC/README.md .................. LCD display API
│  └─ ... (see full list in README.md)
│
└─ docs/
   ├─ DATASHEET.pdf ................... STM32F429 specs
   ├─ REFERENCEMANUAL-dis-1.pdf ....... STM32F4 reference manual
   └─ USERMANUAL.pdf .................. Discovery board manual
```

---

## 🚀 Quick Navigation

**I'm a...**

- **Complete beginner:** [GETTING_STARTED.md](GETTING_STARTED.md) → Start with Phase 1
- **Intermediate user:** [QUICK_REFERENCE.md](QUICK_REFERENCE.md) → Copy examples
- **Advanced developer:** [Peripherals/](Peripherals/) → Read source code directly

**I need to...**

- **See project overview:** [README.md](README.md)
- **Wire hardware:** [HARDWARE_INVENTORY.md](HARDWARE_INVENTORY.md)
- **Build firmware:** [README.md - Building](README.md#building-the-project)
- **Flash board:** [QUICK_REFERENCE.md - Phase 1](QUICK_REFERENCE.md#-phase-1-verify-build-5-min)
- **Debug issues:** [GETTING_STARTED.md - Troubleshooting](GETTING_STARTED.md#-common-issues--solutions)
- **Check progress:** [PROJECT_FLOWCHART.md - Milestones](PROJECT_FLOWCHART.md#-milestone-checklist)

**I'm working on...**

- **UART/Serial:** [GETTING_STARTED Phase 2](GETTING_STARTED.md#phase-2-serial-console--debugging-1-hour)
- **Relay/Buzzer:** [GETTING_STARTED Phase 3](GETTING_STARTED.md#phase-3-gpio-control---relay--buzzer-1-hour)
- **DHT sensor:** [GETTING_STARTED Phase 4](GETTING_STARTED.md#phase-4-i2c-sensors---dht1122-2-hours)
- **RTC:** [GETTING_STARTED Phase 5](GETTING_STARTED.md#phase-5-real-time-clock---ds3231-15-hours)
- **CAN bus:** [GETTING_STARTED Phase 6](GETTING_STARTED.md#phase-6-can-bus-communication-2-hours)
- **LCD display:** [GETTING_STARTED Phase 7](GETTING_STARTED.md#phase-7-advanced-integration---lcd-display-2-hours)
- **Flash logging:** [GETTING_STARTED Phase 8](GETTING_STARTED.md#phase-8-data-logging-to-flash-15-hours)
- **FreeRTOS:** [GETTING_STARTED Phase 9](GETTING_STARTED.md#phase-9-freertos-multi-tasking-2-hours)

---

## 📞 Need Help?

1. **Check documentation** (you're in the right place!)
2. **Read troubleshooting sections** in GETTING_STARTED.md
3. **Verify wiring** in HARDWARE_INVENTORY.md
4. **Check example code** in `Peripherals/XXX/xxx_example.c`
5. **Review STM32 docs** in `docs/` folder

---

## ✅ Checklist: Have I Read...?

Before asking for help, make sure you've checked:

- [ ] [README.md](README.md) - Project overview
- [ ] [GETTING_STARTED.md](GETTING_STARTED.md) - Step-by-step guide for your current phase
- [ ] [HARDWARE_INVENTORY.md](HARDWARE_INVENTORY.md) - Wiring diagram for your peripheral
- [ ] [QUICK_REFERENCE.md](QUICK_REFERENCE.md) - Troubleshooting table
- [ ] `Peripherals/XXX/README.md` - API docs for specific peripheral
- [ ] Example code: `Peripherals/XXX/xxx_example.c`

---

## 🎯 Recommended Reading Order

### For Complete Beginners:
1. **[README.md](README.md)** (5 min) - Understand the project
2. **[HARDWARE_INVENTORY.md](HARDWARE_INVENTORY.md)** (10 min) - See what hardware you have
3. **[PROJECT_FLOWCHART.md](PROJECT_FLOWCHART.md)** (5 min) - Visualize the roadmap
4. **[GETTING_STARTED.md](GETTING_STARTED.md)** (work through it) - Build the project step-by-step

### For Intermediate Users:
1. **[README.md](README.md)** (5 min) - Quick overview
2. **[QUICK_REFERENCE.md](QUICK_REFERENCE.md)** (work through it) - Implement all peripherals
3. **[HARDWARE_INVENTORY.md](HARDWARE_INVENTORY.md)** (reference) - Verify pins when needed

### For Advanced Users:
1. **[QUICK_REFERENCE.md](QUICK_REFERENCE.md)** (reference) - Copy snippets as needed
2. **Source code** in `Peripherals/XXX/*.c` - Read implementation directly
3. **[docs/REFERENCEMANUAL-dis-1.pdf](docs/)** - STM32F4 register-level details

---

**Ready to start?** Go to **[GETTING_STARTED.md](GETTING_STARTED.md)** now! 🚀
