# Industrial HMI Panel Project

> **⚠️ Prerequisites:** Before starting this project, complete the base system setup from the main project documentation:
> - **[../../GETTING_STARTED.md](../../GETTING_STARTED.md)** - Build and test all peripheral drivers first
> - **[../../QUICK_REFERENCE.md](../../QUICK_REFERENCE.md)** - Verify UART, I2C, CAN, LCD are working
> - **[../../HARDWARE_INVENTORY.md](../../HARDWARE_INVENTORY.md)** - Ensure you have required hardware
> - **[../../DOCUMENTATION_INDEX.md](../../DOCUMENTATION_INDEX.md)** - Complete documentation guide

---

## 📚 Quick Navigation

**HMI Project Documentation:**
- [Project Overview](#project-overview) - What this project does
- [Hardware Requirements](#hardware-requirements) - What you have vs. what you need
- [Development Roadmap](#project-development-roadmap) - Current status and next steps
- [Detailed Docs](docs/) - Implementation guides for each phase

**Main Project Resources:**
- [Build System](../../README.md#building-the-project) - How to compile and flash
- [Base Peripheral Drivers](../../Peripherals/) - UART, I2C, SPI, CAN, etc.
- [Shopping List](../../SHOPPING_LIST.md) - Components needed

---

## Project Overview
A professional-grade Human-Machine Interface (HMI) panel built on the STM32F429I-DISC1 for industrial monitoring and control applications.

**Target Market**: Small factories, workshops, automation hobbyists, educational institutions

**Estimated Selling Price**: $100-250 (depending on features and enclosure)

**Current Status**: 🟡 **Planning Phase** - Base peripheral drivers complete, HMI application layer in development

---

## Project Development Roadmap

### ✅ Phase 0: Base System (COMPLETED)
- [x] Set up project structure and build system
- [x] Implement peripheral drivers (UART, I2C, SPI, CAN, GPIO)
- [x] Create relay control driver
- [x] Create buzzer driver (active/passive PWM)
- [x] Create DHT22 sensor driver
- [x] Test LTDC display driver
- [x] Test touchscreen input
- [x] Verify FreeRTOS integration

**See**: [../../GETTING_STARTED.md](../../GETTING_STARTED.md) for implementation details

### Phase 1: Requirements & Planning (Week 1) - 🟡 IN PROGRESS
- [x] Define target use cases and customer requirements
- [x] Create feature specification document
- [x] Design system architecture
- [x] Plan hardware additions (sensors, communication modules)
- [x] Estimate BOM cost and pricing strategy
- [ ] Finalize pin assignments for RS485 and remaining peripherals

### Phase 2: Core Infrastructure (Week 2-3) - 🔄 NEXT
- [x] Display driver ready (LTDC peripheral tested)
- [x] Touchscreen ready (existing driver)
- [ ] Build UI framework (widgets, screens, navigation)
- [ ] Implement screen manager for multi-screen navigation
- [ ] Create widget library (buttons, gauges, charts)

### Phase 3: Communication Layer (Week 4) - ⏸️ PLANNED
- [x] CAN bus driver available (tested)
- [ ] Implement Modbus RTU (RS485) for industrial devices
- [ ] Add RS485 hardware module
- [ ] Optional: WiFi/Ethernet for IoT connectivity
- [ ] Create protocol abstraction layer

### Phase 4: Application Features (Week 5-6) - ⏸️ PLANNED
- [x] Real-time sensor data available (DHT22)
- [x] RTC available for timestamps (DS3231)
- [ ] Real-time data visualization (gauges, charts, trends)
- [ ] Alarm/notification system with buzzer
- [ ] Data logging to internal flash (128KB available)
- [ ] Configuration/settings screens
- [ ] User authentication (PIN/password)

### Phase 5: Polish & Production (Week 7-8) - ⏸️ PLANNED
- [ ] UI/UX refinement and branding
- [ ] Performance optimization
- [ ] Comprehensive testing
- [ ] Documentation (user manual, API docs)
- [ ] Enclosure design and production

---

## Detailed Step-by-Step Implementation Guide

See individual files in this folder:
1. `01_REQUIREMENTS.md` - Requirements specification
2. `02_ARCHITECTURE.md` - System architecture design
3. `03_UI_FRAMEWORK.md` - GUI framework implementation
4. `04_COMMUNICATION.md` - Communication protocols
5. `05_FEATURES.md` - Application features
6. `06_TESTING.md` - Testing strategy
7. `07_PRODUCTION.md` - Production and deployment

---

## Hardware Requirements

### Base Platform (✅ You Have This)
- ✅ STM32F429I-DISC1 Discovery Board
- ✅ 2.4" QVGA TFT LCD (240x320) with touchscreen (built-in)
- ✅ DS3231 RTC Module (owned - see [HARDWARE_INVENTORY.md](../../HARDWARE_INVENTORY.md))
- ✅ DHT22 Temperature/Humidity Sensor (owned)
- ✅ SRD-05V Relay (owned)
- ✅ Active/Passive Buzzers (owned)
- ✅ Waveshare CAN Module (owned)
- ✅ FT232RL USB-UART (owned - for debugging)

### Recommended Additions (Still Needed)
| Component | Purpose | Est. Cost | Status |
|-----------|---------|-----------|--------|
| RS485 Module (MAX485) | Modbus RTU communication | $2-5 | ⚠️ **Priority** |
| MicroSD Card Module | Data logging | $1-3 | 🔄 Optional (use internal flash) |
| ESP8266/ESP32 | WiFi connectivity | $3-8 | 🔄 Future upgrade |
| 3D Printed Enclosure | Professional housing | $10-20 | 🔄 Final stage |

**Total Additional BOM**: ~$12-17 (essential only) or ~$25-35 (all features)

**See**: [../../SHOPPING_LIST.md](../../SHOPPING_LIST.md) for complete parts list

---

## Revenue Model

### Direct Sales
- Basic HMI Panel: $100-150
- Advanced HMI (WiFi + logging): $150-200
- Custom branded solutions: $200-300

### Services
- Custom GUI development: $50-100/screen
- Protocol integration: $100-200
- On-site installation: $50-100
- Training sessions: $50/hour

### Recurring Revenue
- Software updates/support subscription: $10-20/month
- Cloud dashboard integration: $15-30/month

---

## Competitive Advantages

1. **Cost**: 5-10x cheaper than commercial HMIs (Siemens, Allen-Bradley)
2. **Customization**: Open-source, fully customizable
3. **Local Support**: Direct customer relationship
4. **Quick Turnaround**: Faster than ordering from distributors

---

## Getting Started

Start with Phase 1 documentation: `01_REQUIREMENTS.md`
