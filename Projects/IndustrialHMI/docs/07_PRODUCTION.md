# Phase 7: Production and Manufacturing

## 7.1 Production Overview

### Manufacturing Stages

```
┌─────────────────────────────────────────────────────────────────────┐
│                    Production Pipeline                               │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  Design        Prototype       Pilot         Mass                   │
│  Validation    Build           Run           Production             │
│                                                                      │
│  ┌────┐       ┌────┐          ┌────┐        ┌────┐                 │
│  │ 1  │──────►│ 5  │─────────►│ 50 │───────►│500+│                 │
│  │unit│       │unit│          │unit│        │unit│                 │
│  └────┘       └────┘          └────┘        └────┘                 │
│     │            │               │             │                     │
│     ▼            ▼               ▼             ▼                     │
│  Verify       Debug          Refine       Scale                     │
│  Concept      Design         Process      Production                │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 7.2 Bill of Materials (Detailed)

### Core Electronics

| Item | Description | Manufacturer | Part Number | Unit Cost | Qty | Extended |
|------|-------------|--------------|-------------|-----------|-----|----------|
| MCU | STM32F429ZIT6 | ST | STM32F429ZIT6 | $12.50 | 1 | $12.50 |
| LCD | 4.3" TFT 480x272 | - | Various | $18.00 | 1 | $18.00 |
| Touch | Capacitive controller | FocalTech | FT5336 | $2.50 | 1 | $2.50 |
| SDRAM | 64Mbit SDRAM | ISSI | IS42S16400J | $3.00 | 1 | $3.00 |
| Flash | 8MB QSPI Flash | Winbond | W25Q64JV | $1.50 | 1 | $1.50 |
| Power | 5V to 3.3V LDO | TI | TPS7333 | $0.80 | 1 | $0.80 |
| RTC | Battery backup IC | Maxim | DS3231 | $4.00 | 1 | $4.00 |
| CAN | CAN transceiver | TI | SN65HVD230 | $1.50 | 1 | $1.50 |
| RS485 | RS485 transceiver | Maxim | MAX485 | $1.00 | 1 | $1.00 |
| ESD | TVS diode arrays | Nexperia | PRTR5V0U2X | $0.30 | 4 | $1.20 |

**Electronics Subtotal: ~$46.00**

### Passive Components

| Item | Description | Cost per 100 |
|------|-------------|-------------|
| Capacitors | 0603 MLCC assortment | $15.00 |
| Resistors | 0603 1% assortment | $10.00 |
| Inductors | Power inductors | $8.00 |
| Crystals | 8MHz, 32.768kHz | $5.00 |
| Ferrite beads | EMI filtering | $3.00 |

**Passives Subtotal: ~$5.00 per unit (at volume)**

### Mechanical

| Item | Description | Unit Cost | Notes |
|------|-------------|-----------|-------|
| Enclosure | IP65 plastic housing | $15.00 | Custom mold |
| Membrane | Front panel overlay | $3.00 | Screen printed |
| Gasket | Silicone seal | $1.00 | Die cut |
| Fasteners | Screws, standoffs | $1.00 | Stainless |
| Cable glands | M12/M16 | $2.00 | IP68 rated |
| Connectors | Terminal blocks | $3.00 | Phoenix style |

**Mechanical Subtotal: ~$25.00**

### Total Cost Breakdown

| Category | Prototype | Volume (500+) |
|----------|-----------|---------------|
| Electronics | $46.00 | $35.00 |
| PCB | $15.00 | $5.00 |
| Passives | $8.00 | $5.00 |
| Mechanical | $35.00 | $25.00 |
| Assembly | $20.00 | $10.00 |
| **Total** | **$124.00** | **$80.00** |

**Target Retail Price: $249 - $349**  
**Gross Margin: 50-65%**

---

## 7.3 PCB Design Guidelines

### Layer Stack-Up (4-Layer)

```
┌─────────────────────────────────────────┐
│  Layer 1 - Signal/Components            │  1.0 oz copper
├─────────────────────────────────────────┤
│  Prepreg - FR4                          │  0.2mm
├─────────────────────────────────────────┤
│  Layer 2 - Ground Plane                 │  1.0 oz copper
├─────────────────────────────────────────┤
│  Core - FR4                             │  1.0mm
├─────────────────────────────────────────┤
│  Layer 3 - Power Plane                  │  1.0 oz copper
├─────────────────────────────────────────┤
│  Prepreg - FR4                          │  0.2mm
├─────────────────────────────────────────┤
│  Layer 4 - Signal/Connectors            │  1.0 oz copper
└─────────────────────────────────────────┘
```

### Design Rules

| Parameter | Value | Notes |
|-----------|-------|-------|
| Min trace width | 0.15mm | Signal traces |
| Power traces | 0.5mm+ | Based on current |
| Via size | 0.3mm drill | Standard |
| Clearance | 0.15mm | Signal-to-signal |
| Annular ring | 0.15mm | Minimum |
| Solder mask | 0.05mm | Expansion |

### EMC Considerations

```
Layout Best Practices:
├── Keep high-speed signals short
├── Route clock signals on inner layers
├── Ground plane under all signals
├── Decoupling caps close to IC pins
├── Separate analog and digital grounds
├── Guard rings around sensitive circuits
└── ESD protection at all external connections
```

---

## 7.4 Enclosure Design

### Industrial Requirements

```
Environmental:
├── IP65 rating (dust/water tight)
├── Operating temp: -20°C to +60°C
├── Storage temp: -40°C to +85°C
├── Humidity: 0-95% non-condensing
└── Vibration: IEC 60068-2-6

Mechanical:
├── Wall/panel mount options
├── DIN rail adapter available
├── Cable entry protection
├── Strain relief for cables
└── Tamper-resistant screws (option)
```

### Panel Cutout Drawing

```
                    210mm
    ┌───────────────────────────────────┐
    │           ┌─────────────────┐      │
    │           │                 │      │
130mm│           │    Display      │      │ 115mm
    │           │    Cutout       │      │
    │           │   180x105mm     │      │
    │           └─────────────────┘      │
    │                                    │
    │  ○                            ○   │ ← Mounting holes
    │                                    │   M4 x 4 corners
    └───────────────────────────────────┘
    
Side View:
    ┌──────┬───────────────────────────┐
    │██████│                           │
    │██LCD█│        PCB                │  35mm
    │██████│                           │  depth
    └──────┴───────────────────────────┘
```

---

## 7.5 Assembly Process

### PCB Assembly (PCBA)

```
Assembly Steps:
1. Solder paste application (stencil)
2. SMD component placement (pick-and-place)
3. Reflow soldering
4. Through-hole component insertion
5. Wave/selective soldering
6. Visual inspection
7. Automated optical inspection (AOI)
8. Functional test
```

### Final Assembly

```
1. Flash firmware to MCU
2. Run board-level tests
3. Mount PCB in enclosure
4. Connect LCD/touch panel
5. Connect internal cables
6. Install gaskets
7. Close enclosure
8. Apply labels
9. Final test
10. Package
```

### Test Procedure

```python
# production_test.py

def run_production_test():
    """Production test sequence"""
    
    tests = [
        ("Power supply", test_power),
        ("MCU communication", test_uart),
        ("LCD display", test_lcd_pattern),
        ("Touch panel", test_touch_cal),
        ("SDRAM", test_sdram_check),
        ("Flash memory", test_qspi),
        ("RTC", test_rtc),
        ("CAN bus", test_can_loopback),
        ("RS485", test_rs485_loopback),
        ("SD card", test_sdcard),
        ("Firmware", test_firmware_version),
    ]
    
    results = []
    for name, test_func in tests:
        try:
            result = test_func()
            results.append((name, "PASS" if result else "FAIL"))
        except Exception as e:
            results.append((name, f"ERROR: {e}"))
    
    # Generate test report
    generate_report(results)
    
    return all(r[1] == "PASS" for r in results)
```

---

## 7.6 Firmware Deployment

### Version Management

```c
// version.h
#define FW_VERSION_MAJOR    1
#define FW_VERSION_MINOR    0
#define FW_VERSION_PATCH    0
#define FW_VERSION_BUILD    __BUILD_NUMBER__

#define FW_VERSION_STRING   "1.0.0"
#define FW_BUILD_DATE       __DATE__
#define FW_BUILD_TIME       __TIME__
```

### OTA Update System

```c
// ota_update.h
typedef enum {
    OTA_STATUS_IDLE,
    OTA_STATUS_CHECKING,
    OTA_STATUS_DOWNLOADING,
    OTA_STATUS_VERIFYING,
    OTA_STATUS_INSTALLING,
    OTA_STATUS_COMPLETE,
    OTA_STATUS_ERROR
} OTAStatus_t;

typedef struct {
    uint32_t currentVersion;
    uint32_t availableVersion;
    uint32_t downloadSize;
    uint32_t downloadProgress;
    OTAStatus_t status;
    char releaseNotes[256];
} OTAInfo_t;

// OTA API
void OTA_Init(void);
bool OTA_CheckForUpdate(void);
bool OTA_StartDownload(void);
bool OTA_ApplyUpdate(void);
OTAInfo_t* OTA_GetStatus(void);
```

### Flash Memory Map

```
┌─────────────────────────────────────────┐ 0x0800_0000
│  Bootloader (32KB)                      │
├─────────────────────────────────────────┤ 0x0800_8000
│  Application A (448KB)                  │
│  - Main firmware                        │
├─────────────────────────────────────────┤ 0x0807_8000
│  Application B (448KB)                  │
│  - Backup/update firmware               │
├─────────────────────────────────────────┤ 0x080E_8000
│  Configuration (64KB)                   │
│  - Device settings                      │
│  - Calibration data                     │
├─────────────────────────────────────────┤ 0x080F_8000
│  Factory Data (32KB)                    │
│  - Serial number                        │
│  - MAC address                          │
│  - Certificates                         │
└─────────────────────────────────────────┘ 0x0810_0000
```

---

## 7.7 Quality Control

### Incoming Quality Control (IQC)

```
Component Inspection:
[ ] Visual inspection of packaging
[ ] Verify part numbers match BOM
[ ] Check date codes (not expired)
[ ] Sample testing (1-5%)
[ ] Certificate of conformance
```

### In-Process Quality Control (IPQC)

```
Assembly Checkpoints:
[ ] Solder paste inspection
[ ] Component placement verification
[ ] Reflow profile monitoring
[ ] First article inspection
[ ] Statistical process control
```

### Outgoing Quality Control (OQC)

```
Final Test:
[ ] 100% functional test
[ ] Random sample burn-in (48 hours)
[ ] Packaging inspection
[ ] Documentation check
[ ] Batch tracking
```

### Batch Records

```
Batch Record Template:
├── Batch number: HMI-2025-001
├── Production date: 2025-12-11
├── Quantity: 100 units
├── BOM revision: 2.1
├── Firmware version: 1.0.0
├── Test results:
│   ├── Passed: 98
│   ├── Failed: 2
│   └── Yield: 98%
├── Failures:
│   ├── Unit 023: LCD defect
│   └── Unit 067: Touch calibration
└── Disposition: Released to inventory
```

---

## 7.8 Certifications

### Required Certifications

| Market | Certification | Requirement | Cost Estimate |
|--------|---------------|-------------|---------------|
| CE (Europe) | EMC | EN 61000-6-2, EN 61000-6-4 | $5,000 |
| CE (Europe) | LVD | EN 62368-1 | $3,000 |
| CE (Europe) | RoHS | 2011/65/EU | $1,000 |
| USA | FCC Part 15B | Unintentional radiator | $4,000 |
| Canada | ISED | RSS-Gen | $3,000 |
| Australia | RCM | AS/NZS CISPR 32 | $3,000 |
| Industrial | IEC 61131-2 | PLC standard | Optional |

### CE Marking Process

```
CE Certification Steps:
1. Design for compliance
   - EMC filtering
   - Proper grounding
   - Shielding where needed

2. Pre-compliance testing
   - In-house EMC tests
   - Fix any issues

3. Select notified body
   - EMC test lab
   - Safety test lab

4. Submit for testing
   - EMC: 2-3 weeks
   - Safety: 2-3 weeks

5. Receive test reports

6. Create technical file
   - Circuit diagrams
   - PCB layouts
   - Test reports
   - Risk assessment
   - User manual

7. Sign Declaration of Conformity

8. Affix CE marking to product
```

---

## 7.9 Documentation Package

### Required Documentation

```
Documentation Deliverables:
├── User Manual
│   ├── Getting started
│   ├── Installation
│   ├── Operation
│   ├── Troubleshooting
│   └── Specifications
│
├── Quick Start Guide (1-2 pages)
│
├── Technical Manual
│   ├── Electrical specifications
│   ├── Mechanical drawings
│   ├── Protocol documentation
│   └── API reference
│
├── Installation Manual
│   ├── Mounting instructions
│   ├── Wiring diagrams
│   ├── Configuration
│   └── First-time setup
│
└── Declaration of Conformity
```

### Label Requirements

```
Required Label Information:
┌─────────────────────────────────────┐
│  Model: HMI-429-PRO                 │
│  S/N: HMI-2025-00001               │
│  Input: 12-24VDC, 500mA max        │
│                                     │
│  ▢ CE  ▢ FCC  ▢ RoHS               │
│                                     │
│  [QR Code]                          │
│  www.company.com                    │
│  Made in [Country]                  │
└─────────────────────────────────────┘
```

---

## 7.10 Production Timeline

### Gantt Chart (Simplified)

```
Week:        1  2  3  4  5  6  7  8  9  10 11 12
────────────────────────────────────────────────
DFM Review   ██
PCB Order    ██ ██
Component   ██ ██ ██
  Sourcing
Tooling         ██ ██ ██
First         ██ ██ ██
  Articles
Pilot                  ██ ██ ██
  Run
Testing                      ██ ██
Certification                ██ ██ ██ ██
Mass                                  ██ ██ ██
  Production
```

---

## 7.11 Supplier Management

### Key Suppliers

| Component | Primary | Backup | Lead Time |
|-----------|---------|--------|-----------|
| MCU | Digi-Key | Mouser | 2-4 weeks |
| LCD | Alibaba vendor | Local | 4-6 weeks |
| PCB | JLCPCB | PCBWay | 1-2 weeks |
| Enclosure | Custom mold | 3D print | 6-8 weeks |
| Assembly | Local CM | Backup CM | 2-3 weeks |

### Dual-Source Strategy

```
Risk Mitigation:
├── Always have 2+ suppliers for critical parts
├── Maintain 4-6 week buffer stock
├── Design for alternative components
├── Document alternates in BOM
└── Test alternates before production
```

---

## 7.12 Next Steps

1. ✅ Production planning complete
2. ➡️ Proceed to `08_BUSINESS.md` for go-to-market strategy
3. Finalize BOM and suppliers
4. Begin enclosure design
5. Plan certification testing

---

## Checklist

- [ ] BOM finalized and costed
- [ ] PCB design completed
- [ ] Enclosure designed
- [ ] Assembly process defined
- [ ] Test procedures written
- [ ] Certifications planned
- [ ] Documentation prepared
- [ ] Suppliers selected
