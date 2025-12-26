# Phase 1: Requirements & Planning

## 1.1 Market Research

### Target Customers
1. **Small Manufacturing Workshops**
   - Machine status monitoring
   - Production counting
   - Simple process control

2. **HVAC/Building Automation**
   - Temperature/humidity monitoring
   - Fan/pump control
   - Scheduling

3. **Agricultural**
   - Greenhouse monitoring
   - Irrigation control
   - Environmental logging

4. **Educational/Training**
   - PLC/HMI training labs
   - University projects
   - Technical schools

### Competitor Analysis
| Product | Price | Features | Our Advantage |
|---------|-------|----------|---------------|
| Siemens SIMATIC | $500-2000 | Full industrial | 10x cheaper |
| Weintek MT8071iE | $300-500 | Mid-range HMI | Customizable |
| Nextion Display | $30-80 | Basic touch | More powerful MCU |

---

## 1.2 Feature Specification

### MVP (Minimum Viable Product) Features
- [ ] **F1**: Multi-screen navigation with touch
- [ ] **F2**: Real-time analog value display (gauges)
- [ ] **F3**: Digital I/O status indicators
- [ ] **F4**: Basic alarm system with buzzer
- [ ] **F5**: Configuration screen
- [ ] **F6**: Date/time display

### Standard Features (Phase 2)
- [ ] **F7**: Modbus RTU communication
- [ ] **F8**: Data trending (line charts)
- [ ] **F9**: Alarm history with timestamps
- [ ] **F10**: SD card data logging
- [ ] **F11**: User PIN protection
- [ ] **F12**: Multiple languages support

### Premium Features (Phase 3)
- [ ] **F13**: WiFi connectivity
- [ ] **F14**: Web dashboard / remote access
- [ ] **F15**: Email/SMS notifications
- [ ] **F16**: Recipe management
- [ ] **F17**: PID loop control
- [ ] **F18**: Custom scripting

---

## 1.3 Hardware Requirements

### Core Components (✅ Already Available)
```
[x] STM32F429ZIT6 MCU (180MHz, 2MB Flash, 256KB RAM)
[x] 2.4" QVGA TFT LCD (ILI9341, 240x320)
[x] Resistive touchscreen (STMPE811)
[x] 64Mbit SDRAM (IS42S16400J)
[x] L3GD20 Gyroscope (bonus feature)
[x] USB OTG
[x] User button + 2 LEDs
```

### Additional Hardware (✅ You Already Own)
```
[x] DS3231 RTC Module
    - I2C interface (address 0x68)
    - Pins: PB6 (I2C1_SCL), PB9 (I2C1_SDA)
    - Driver: Peripherals/RTC/rtc.c
    - Battery backup for timekeeping
    
[x] DHT22 Temperature/Humidity Sensor
    - Single-wire protocol
    - Pin: PA1 (with 10kΩ pullup)
    - Driver: Peripherals/DHT/dht.c
    - For environmental monitoring
    
[x] SRD-05V Relay Module
    - GPIO control
    - Pin: PB0 (configurable)
    - Driver: Peripherals/RELAY/relay.c
    - For output control
    
[x] Active/Passive Buzzers
    - Active: GPIO control (PC0)
    - Passive: PWM control (PA6 - TIM3_CH1)
    - Driver: Peripherals/BUZZER/buzzer.c
    - For alarms and notifications
    
[x] Waveshare CAN Module
    - CAN bus interface
    - Pins: PA12 (CAN1_TX), PA11 (CAN1_RX)
    - Driver: Peripherals/CAN/can.c
    - 120Ω termination required
    
[x] FT232RL USB-to-UART
    - Debugging console
    - Pins: PA9 (USART1_TX), PA10 (USART1_RX)
    - Driver: Peripherals/UART/uart.c
```

### Additional Hardware (⚠️ Still Need to Purchase)
```
[ ] RS485 Transceiver (MAX485 or SP485)
    - Pins: USART2 (PA2-TX, PA3-RX) + DE/RE on PB0
    - **PRIORITY**: Required for Modbus RTU
    - Cost: $2-5
    
[ ] MicroSD Card Socket (OPTIONAL - can use internal flash)
    - Pins: SDIO (PC8-D0, PC9-D1, PC10-D2, PC11-D3, PC12-CLK, PD2-CMD)
    - Alternative: Use internal Flash Sector 11 (128KB) for logging
    - Driver: Peripherals/FLASH/flash.c (already available)
    
[ ] ESP8266 or ESP32 Module (OPTIONAL - Future upgrade)
    - Pins: USART3 (PB10-TX, PB11-RX)
    - For WiFi connectivity
    
[ ] External Power Supply (5V 2A)
    - For standalone operation
```

**See**: [../../HARDWARE_INVENTORY.md](../../HARDWARE_INVENTORY.md) for complete wiring diagrams and integration details.

### Pin Allocation Table
| Function | Peripheral | Pins | Driver | Status |
|----------|-----------|------|--------|--------|
| LCD | LTDC + SPI5 | PF7-10, PA4-7, etc | Peripherals/LTDC/ | ✅ Available |
| Touch | I2C3 | PA8-SCL, PC9-SDA | Peripherals/TOUCHSCREEN/ | ✅ Available |
| Debug UART | USART1 | PA9-TX, PA10-RX | Peripherals/UART/ | ✅ Tested |
| RS485 | USART2 | PA2-TX, PA3-RX, PB0-DE | *To be implemented* | ⚠️ Hardware needed |
| RTC (DS3231) | I2C1 | PB6-SCL, PB9-SDA | Peripherals/RTC/ | ✅ Available |
| DHT22 | GPIO | PA1 + 10kΩ pullup | Peripherals/DHT/ | ✅ Tested |
| CAN Bus | CAN1 | PA12-TX, PA11-RX | Peripherals/CAN/ | ✅ Available |
| Relay | GPIO | PB0 (configurable) | Peripherals/RELAY/ | ✅ Tested |
| Buzzer (Active) | GPIO | PC0 | Peripherals/BUZZER/ | ✅ Tested |
| Buzzer (Passive) | TIM3_CH1 | PA6 | Peripherals/BUZZER/ | ✅ Tested |
| WiFi (Future) | USART3 | PB10-TX, PB11-RX | *Future* | 🔄 Optional |
| SD Card (Alt) | Flash | Internal Sector 11 | Peripherals/FLASH/ | ✅ Available |
| User Button | GPIO | PA0 | Peripherals/GPIO/ | ✅ Available |
| LEDs | GPIO | PG13 (Green), PG14 (Red) | Peripherals/GPIO/ | ✅ Available |

**Note**: Pin assignments verified against existing peripheral drivers. See [../../GETTING_STARTED.md](../../GETTING_STARTED.md) for testing each peripheral.

---

## 1.4 Software Architecture Overview

### Layer Diagram
```
┌─────────────────────────────────────────────────────────┐
│                    APPLICATION LAYER                     │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐   │
│  │ Monitor  │ │  Alarm   │ │  Config  │ │  Trend   │   │
│  │  Screen  │ │  Screen  │ │  Screen  │ │  Screen  │   │
│  └──────────┘ └──────────┘ └──────────┘ └──────────┘   │
├─────────────────────────────────────────────────────────┤
│                      UI FRAMEWORK                        │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────────┐   │
│  │ Widgets │ │ Screens │ │  Touch  │ │   Fonts/    │   │
│  │ Library │ │ Manager │ │ Handler │ │   Graphics  │   │
│  └─────────┘ └─────────┘ └─────────┘ └─────────────┘   │
├─────────────────────────────────────────────────────────┤
│                   MIDDLEWARE LAYER                       │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────────┐   │
│  │ Modbus  │ │  Data   │ │  Alarm  │ │    File     │   │
│  │  Stack  │ │ Manager │ │ Manager │ │   System    │   │
│  └─────────┘ └─────────┘ └─────────┘ └─────────────┘   │
├─────────────────────────────────────────────────────────┤
│                      HAL / DRIVERS                       │
│  ┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐ ┌─────────┐  │
│  │ LCD │ │Touch│ │UART │ │ SPI │ │ RTC │ │  SDIO   │  │
│  └─────┘ └─────┘ └─────┘ └─────┘ └─────┘ └─────────┘  │
├─────────────────────────────────────────────────────────┤
│                   STM32F4 HARDWARE                       │
└─────────────────────────────────────────────────────────┘
```

---

## 1.5 Project Timeline

### Week 1: Planning & Setup
| Day | Task |
|-----|------|
| 1 | Complete requirements document |
| 2 | Design system architecture |
| 3 | Set up project structure |
| 4 | Order additional components |
| 5 | Review existing peripheral drivers |

### Week 2-3: Core Development
| Day | Task |
|-----|------|
| 6-7 | Implement graphics primitives |
| 8-9 | Build widget library |
| 10-11 | Create screen manager |
| 12-13 | Implement touch handling |
| 14 | Integration testing |

### Week 4: Communication
| Day | Task |
|-----|------|
| 15-16 | Implement Modbus RTU |
| 17 | Add RS485 hardware support |
| 18-19 | Create data manager |
| 20 | Protocol testing |

### Week 5-6: Features
| Day | Task |
|-----|------|
| 21-23 | Build application screens |
| 24-25 | Implement alarm system |
| 26-27 | Add data logging |
| 28 | User authentication |

### Week 7-8: Polish
| Day | Task |
|-----|------|
| 29-30 | UI refinement |
| 31-32 | Performance optimization |
| 33-34 | Testing & bug fixes |
| 35-36 | Documentation |
| 37-38 | Enclosure design |
| 39-40 | Production prep |

---

## 1.6 Cost Analysis

### Bill of Materials (BOM)
| Item | Qty | Unit Cost | Total |
|------|-----|-----------|-------|
| STM32F429I-DISC1 | 1 | $30 | $30 |
| MAX485 Module | 1 | $2 | $2 |
| MicroSD Socket | 1 | $1 | $1 |
| CR2032 Holder | 1 | $0.50 | $0.50 |
| Piezo Buzzer | 1 | $0.50 | $0.50 |
| ESP8266 (optional) | 1 | $3 | $3 |
| Enclosure (3D print) | 1 | $10 | $10 |
| Cables/Connectors | - | $5 | $5 |
| **Total BOM** | | | **$52** |

### Pricing Strategy
| Model | Features | BOM | Margin | Price |
|-------|----------|-----|--------|-------|
| Basic | F1-F6 | $42 | 150% | $99 |
| Standard | F1-F12 | $47 | 200% | $149 |
| Premium | F1-F18 | $55 | 250% | $199 |

### Break-Even Analysis
- Development time: ~160 hours
- Hourly rate target: $25/hour
- Development cost: $4,000
- Break-even units (Standard): 4000 / (149-47) = **40 units**

---

## 1.7 Next Steps

1. ✅ Read this requirements document
2. ➡️ Proceed to `02_ARCHITECTURE.md` for system design
3. Set up development environment
4. Order additional hardware components
5. Begin Phase 2 implementation

---

## Checklist

- [ ] Requirements reviewed and approved
- [ ] Hardware components ordered
- [ ] Development environment ready
- [ ] Version control set up
- [ ] Project structure created
