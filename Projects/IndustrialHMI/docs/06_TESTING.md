# Phase 6: Testing and Validation

## 6.1 Testing Strategy

### Test Pyramid

```
        ╱╲
       ╱  ╲
      ╱ E2E╲        End-to-End Tests (Manual + Automated)
     ╱──────╲       - Full system validation
    ╱        ╲      - User acceptance tests
   ╱ Integr.  ╲     Integration Tests
  ╱────────────╲    - Component interactions
 ╱              ╲   - Communication protocols
╱   Unit Tests   ╲  Unit Tests
──────────────────  - Individual functions
                    - Module behavior
```

### Test Categories

| Category | Coverage | Automation | Tools |
|----------|----------|------------|-------|
| Unit Tests | 80% | Automated | Unity/CppUTest |
| Integration | 60% | Semi-auto | Custom harness |
| System | 100% | Manual | Test plan |
| Regression | Critical paths | Automated | CI/CD |

---

## 6.2 Unit Testing Framework

### Test File Structure

```
tests/
├── unity/                  # Unity test framework
│   ├── unity.c
│   ├── unity.h
│   └── unity_internals.h
├── test_runners/
│   ├── test_runner.c
│   └── test_main.c
├── test_data_manager.c
├── test_alarm_manager.c
├── test_modbus.c
├── test_screen_manager.c
└── mocks/
    ├── mock_hal.c
    ├── mock_uart.c
    └── mock_touch.c
```

### Unity Test Example

```c
// test_alarm_manager.c
#include "unity.h"
#include "alarm_manager.h"
#include "mock_time.h"

void setUp(void)
{
    AlarmManager_Init();
}

void tearDown(void)
{
    // Cleanup
}

// Test: Create alarm
void test_AlarmManager_CreateAlarm_ValidId_ReturnsAlarm(void)
{
    AlarmDef_t* alarm = AlarmManager_CreateAlarm(1, "Test Alarm");
    
    TEST_ASSERT_NOT_NULL(alarm);
    TEST_ASSERT_EQUAL_UINT16(1, alarm->id);
    TEST_ASSERT_EQUAL_STRING("Test Alarm", alarm->name);
    TEST_ASSERT_EQUAL(ALARM_STATE_NORMAL, alarm->state);
}

// Test: Alarm activation
void test_AlarmManager_HighAlarm_ValueExceedsLimit_AlarmsActive(void)
{
    AlarmDef_t* alarm = AlarmManager_CreateAlarm(10, "High Temp");
    alarm->condition = ALARM_COND_HIGH;
    alarm->limit = 80.0f;
    alarm->tagId = TAG_TEMPERATURE;
    alarm->enabled = true;
    
    // Set value above limit
    DataManager_SetValue(TAG_TEMPERATURE, 85.0f);
    AlarmManager_Process();
    
    TEST_ASSERT_EQUAL(ALARM_STATE_ACTIVE, alarm->state);
    TEST_ASSERT_EQUAL_UINT16(1, AlarmManager_GetActiveCount());
}

// Test: Alarm acknowledgment
void test_AlarmManager_Acknowledge_ActiveAlarm_StateChanges(void)
{
    // Setup active alarm
    AlarmDef_t* alarm = AlarmManager_CreateAlarm(20, "Test");
    alarm->condition = ALARM_COND_HIGH;
    alarm->limit = 50.0f;
    alarm->enabled = true;
    alarm->requiresAck = true;
    
    DataManager_SetValue(0, 60.0f);
    AlarmManager_Process();
    
    // Acknowledge
    AlarmManager_Acknowledge(20);
    
    TEST_ASSERT_EQUAL(ALARM_STATE_ACKNOWLEDGED, alarm->state);
}

// Test: Deadband hysteresis
void test_AlarmManager_Deadband_ValueReturns_NoChatter(void)
{
    AlarmDef_t* alarm = AlarmManager_CreateAlarm(30, "Deadband Test");
    alarm->condition = ALARM_COND_HIGH;
    alarm->limit = 80.0f;
    alarm->deadband = 5.0f;
    alarm->enabled = true;
    
    // Trigger alarm
    DataManager_SetValue(0, 82.0f);
    AlarmManager_Process();
    TEST_ASSERT_EQUAL(ALARM_STATE_ACTIVE, alarm->state);
    
    // Return to limit (but within deadband)
    DataManager_SetValue(0, 78.0f);
    AlarmManager_Process();
    TEST_ASSERT_EQUAL(ALARM_STATE_ACTIVE, alarm->state);
    
    // Return below deadband
    DataManager_SetValue(0, 74.0f);
    AlarmManager_Process();
    TEST_ASSERT_EQUAL(ALARM_STATE_RETURNED, alarm->state);
}

// Test runner
int main(void)
{
    UNITY_BEGIN();
    
    RUN_TEST(test_AlarmManager_CreateAlarm_ValidId_ReturnsAlarm);
    RUN_TEST(test_AlarmManager_HighAlarm_ValueExceedsLimit_AlarmsActive);
    RUN_TEST(test_AlarmManager_Acknowledge_ActiveAlarm_StateChanges);
    RUN_TEST(test_AlarmManager_Deadband_ValueReturns_NoChatter);
    
    return UNITY_END();
}
```

---

## 6.3 Integration Tests

### Communication Protocol Tests

```c
// test_modbus_integration.c
void test_ModbusRTU_ReadHoldingRegisters_ValidRequest_ReturnsData(void)
{
    uint8_t request[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x0A, 0xC5, 0xCD};
    uint8_t response[256];
    uint16_t responseLen;
    
    // Process request
    ModbusRTU_ProcessRequest(request, sizeof(request));
    ModbusRTU_GetResponse(response, &responseLen);
    
    // Verify response
    TEST_ASSERT_EQUAL_UINT8(0x01, response[0]);     // Slave address
    TEST_ASSERT_EQUAL_UINT8(0x03, response[1]);     // Function code
    TEST_ASSERT_EQUAL_UINT8(20, response[2]);       // Byte count
    TEST_ASSERT_GREATER_THAN(0, responseLen);
    
    // Verify CRC
    uint16_t crc = ModbusCRC(response, responseLen - 2);
    TEST_ASSERT_EQUAL_UINT8(crc & 0xFF, response[responseLen - 2]);
    TEST_ASSERT_EQUAL_UINT8(crc >> 8, response[responseLen - 1]);
}

void test_ModbusRTU_InvalidFunction_ReturnsException(void)
{
    uint8_t request[] = {0x01, 0x07, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00};
    uint8_t response[256];
    uint16_t responseLen;
    
    ModbusRTU_ProcessRequest(request, sizeof(request));
    ModbusRTU_GetResponse(response, &responseLen);
    
    // Verify exception response
    TEST_ASSERT_EQUAL_UINT8(0x01, response[0]);     // Slave address
    TEST_ASSERT_EQUAL_UINT8(0x87, response[1]);     // Exception (0x80 | 0x07)
    TEST_ASSERT_EQUAL_UINT8(0x01, response[2]);     // Illegal function
}
```

### UI Integration Tests

```c
// test_screen_navigation.c
void test_ScreenManager_Navigate_MainToMonitor_Succeeds(void)
{
    ScreenManager_Init();
    ScreenManager_ShowScreen(SCREEN_MAIN_MENU);
    
    // Simulate button press
    TouchEvent_t event = {
        .type = TOUCH_PRESS,
        .x = 60,    // Monitor button location
        .y = 100
    };
    
    ScreenManager_ProcessTouch(&event);
    
    TEST_ASSERT_EQUAL(SCREEN_MONITOR, ScreenManager_GetCurrentScreen());
}

void test_ScreenManager_BackButton_ReturnsToParent(void)
{
    ScreenManager_ShowScreen(SCREEN_MAIN_MENU);
    ScreenManager_ShowScreen(SCREEN_CONFIG);
    ScreenManager_ShowScreen(SCREEN_CONFIG_COMM);
    
    ScreenManager_GoBack();
    TEST_ASSERT_EQUAL(SCREEN_CONFIG, ScreenManager_GetCurrentScreen());
    
    ScreenManager_GoBack();
    TEST_ASSERT_EQUAL(SCREEN_MAIN_MENU, ScreenManager_GetCurrentScreen());
}
```

---

## 6.4 Hardware-in-Loop Tests

### Test Bench Setup

```
┌─────────────────────────────────────────────────────────┐
│                    Test Bench                           │
├─────────────────────────────────────────────────────────┤
│                                                         │
│   ┌───────────┐        ┌───────────┐                   │
│   │   HMI     │◄──────►│   PLC     │                   │
│   │ STM32F429 │ Modbus │ (Simul.)  │                   │
│   └───────────┘        └───────────┘                   │
│        │                    │                           │
│        │ USB               │                           │
│        ▼                    ▼                           │
│   ┌───────────┐        ┌───────────┐                   │
│   │   Test    │        │  Signal   │                   │
│   │    PC     │◄──────►│ Generator │                   │
│   └───────────┘        └───────────┘                   │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

### Automated Test Script (Python)

```python
#!/usr/bin/env python3
# test_hmi_automation.py

import serial
import time
import struct

class HMITester:
    def __init__(self, port='/dev/ttyUSB0', baudrate=115200):
        self.ser = serial.Serial(port, baudrate, timeout=1)
        self.test_results = []
        
    def send_modbus_request(self, slave_id, function, address, count):
        """Send Modbus RTU request and get response"""
        request = struct.pack('>BBHH', slave_id, function, address, count)
        crc = self.calculate_crc(request)
        request += struct.pack('<H', crc)
        
        self.ser.write(request)
        time.sleep(0.1)
        
        response = self.ser.read(256)
        return response
        
    def test_read_registers(self):
        """Test: Read holding registers"""
        response = self.send_modbus_request(1, 3, 0, 10)
        
        assert len(response) > 5, "No response received"
        assert response[0] == 1, f"Wrong slave ID: {response[0]}"
        assert response[1] == 3, f"Wrong function: {response[1]}"
        
        self.test_results.append(("Read Registers", "PASS"))
        
    def test_write_register(self):
        """Test: Write single register"""
        # Write value 100 to register 0
        request = struct.pack('>BBHH', 1, 6, 0, 100)
        crc = self.calculate_crc(request)
        request += struct.pack('<H', crc)
        
        self.ser.write(request)
        time.sleep(0.1)
        
        response = self.ser.read(256)
        assert len(response) == 8, "Invalid response length"
        
        # Verify echo
        assert response[:6] == request[:6], "Write not echoed"
        
        self.test_results.append(("Write Register", "PASS"))
        
    def test_communication_timeout(self):
        """Test: Communication timeout handling"""
        # Send invalid slave ID
        response = self.send_modbus_request(99, 3, 0, 1)
        
        # Should get no response (timeout)
        assert len(response) == 0, "Unexpected response from invalid slave"
        
        self.test_results.append(("Timeout Handling", "PASS"))
        
    def run_all_tests(self):
        """Run all automated tests"""
        tests = [
            self.test_read_registers,
            self.test_write_register,
            self.test_communication_timeout,
        ]
        
        for test in tests:
            try:
                test()
                print(f"✓ {test.__name__}")
            except AssertionError as e:
                print(f"✗ {test.__name__}: {e}")
                self.test_results.append((test.__name__, f"FAIL: {e}"))
                
        self.print_summary()
        
    def print_summary(self):
        """Print test summary"""
        passed = sum(1 for _, result in self.test_results if result == "PASS")
        total = len(self.test_results)
        
        print(f"\n{'='*50}")
        print(f"Test Summary: {passed}/{total} passed")
        print(f"{'='*50}")

if __name__ == '__main__':
    tester = HMITester('/dev/ttyUSB0')
    tester.run_all_tests()
```

---

## 6.5 Manual Test Plans

### System Test Checklist

#### Power-On Tests
```
[ ] System boots within 5 seconds
[ ] Splash screen displays correctly
[ ] LCD backlight works
[ ] Touch calibration valid
[ ] RTC time is correct
[ ] SD card detected (if present)
[ ] Communication ports initialized
```

#### Display Tests
```
[ ] All colors render correctly
[ ] Text is readable at all angles
[ ] Touch is responsive and accurate
[ ] Screen transitions are smooth
[ ] Widgets update in real-time
[ ] Backlight brightness adjustable
```

#### Communication Tests
```
[ ] Modbus RTU master mode works
[ ] Modbus slave mode works
[ ] CAN bus transmit/receive works
[ ] USB enumeration succeeds
[ ] Communication timeouts handled
[ ] Error recovery works
```

#### Alarm Tests
```
[ ] Alarms trigger at correct thresholds
[ ] Alarm colors indicate priority
[ ] Audible alert sounds
[ ] Acknowledge function works
[ ] Alarm history records correctly
[ ] Deadband prevents chatter
```

#### Data Logging Tests
```
[ ] Logging starts/stops correctly
[ ] Data saved to SD card
[ ] File rotation works
[ ] Timestamps are accurate
[ ] CSV format is correct
[ ] Export to USB works
```

#### Security Tests
```
[ ] Login screen appears
[ ] PIN authentication works
[ ] Access levels enforced
[ ] Auto-logout after timeout
[ ] Cannot access restricted screens
```

---

## 6.6 Performance Tests

### Timing Requirements

| Operation | Requirement | Measure Method |
|-----------|-------------|----------------|
| Boot time | < 5 seconds | Stopwatch |
| Screen transition | < 200ms | Logic analyzer |
| Touch response | < 100ms | Oscilloscope |
| Data update rate | ≥ 10 Hz | Counter |
| Modbus response | < 50ms | Protocol analyzer |
| Alarm activation | < 1 second | Test script |

### Stress Tests

```c
// stress_test.c
void StressTest_ContinuousDisplay(void)
{
    // Run for 24 hours, update display every 100ms
    uint32_t startTime = HAL_GetTick();
    uint32_t updateCount = 0;
    uint32_t errorCount = 0;
    
    while (HAL_GetTick() - startTime < 24 * 3600 * 1000)
    {
        uint32_t before = HAL_GetTick();
        
        // Update all widgets
        Screen_Update();
        
        uint32_t elapsed = HAL_GetTick() - before;
        
        if (elapsed > 100) {
            errorCount++;
            printf("Update too slow: %lu ms\n", elapsed);
        }
        
        updateCount++;
        HAL_Delay(100);
    }
    
    printf("Stress test complete:\n");
    printf("  Updates: %lu\n", updateCount);
    printf("  Errors: %lu\n", errorCount);
    printf("  Success rate: %.2f%%\n", 
           100.0f * (updateCount - errorCount) / updateCount);
}
```

---

## 6.7 Environmental Tests

### Temperature Testing
```
[ ] Operates at 0°C (cold start)
[ ] Operates at 25°C (nominal)
[ ] Operates at 50°C (hot)
[ ] LCD readable at all temperatures
[ ] Touch works at all temperatures
```

### Power Testing
```
[ ] Operates at 4.5V (minimum USB)
[ ] Operates at 5.0V (nominal)
[ ] Operates at 5.5V (maximum)
[ ] Clean shutdown on power loss
[ ] Data integrity after power cycle
```

### EMC Considerations
```
[ ] ESD protection tested
[ ] No interference from motors nearby
[ ] No interference to other equipment
[ ] Proper grounding verified
```

---

## 6.8 Validation Checklist

### Before Release

```
Unit Tests:
[ ] All unit tests passing
[ ] Code coverage > 80%
[ ] No compiler warnings
[ ] Static analysis clean

Integration Tests:
[ ] All communication protocols verified
[ ] UI navigation correct
[ ] Alarm system validated
[ ] Data logging verified

System Tests:
[ ] All manual test cases passed
[ ] 24-hour stress test complete
[ ] Temperature range verified
[ ] Power range verified

Documentation:
[ ] User manual complete
[ ] API documentation complete
[ ] Test reports generated
[ ] Known issues documented
```

---

## 6.9 Bug Tracking

### Bug Template

```markdown
## Bug Report

**ID:** BUG-001
**Severity:** Critical / High / Medium / Low
**Component:** [UI / Modbus / Alarm / Logging / etc.]

**Description:**
[Clear description of the bug]

**Steps to Reproduce:**
1. [Step 1]
2. [Step 2]
3. [Step 3]

**Expected Result:**
[What should happen]

**Actual Result:**
[What actually happens]

**Environment:**
- Firmware version: 1.0.0
- Hardware revision: A
- Test date: 2025-12-11

**Attachments:**
[Screenshots, logs, etc.]

**Status:** Open / In Progress / Fixed / Verified / Closed
```

---

## 6.10 Next Steps

1. ✅ Testing strategy defined
2. ➡️ Proceed to `07_PRODUCTION.md` for manufacturing preparation
3. Set up CI/CD for automated testing
4. Create test fixtures
5. Begin systematic testing

---

## Checklist

- [ ] Unity framework integrated
- [ ] Unit tests written
- [ ] Integration tests written
- [ ] Test bench assembled
- [ ] Manual test plans executed
- [ ] Performance verified
- [ ] All bugs resolved
