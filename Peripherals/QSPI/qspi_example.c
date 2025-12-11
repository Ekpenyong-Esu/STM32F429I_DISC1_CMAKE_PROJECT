/**
  ******************************************************************************
  * @file    qspi_example.c
  * @brief   QSPI driver example implementation for STM32F429 Discovery Board
  * @details This file provides comprehensive examples for using external
  *          QSPI Flash memory with the STM32F429 Discovery board.
  * @version 1.0
  * @date    2025-09-04
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "qspi_example.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Private constants ---------------------------------------------------------*/
#define QSPI_EXAMPLE_PATTERN_SEED       0x12345678  /* Test pattern seed */
#define QSPI_EXAMPLE_SPEED_TEST_SIZE    4096        /* Speed test data size */
#define QSPI_EXAMPLE_MAX_RETRIES        3           /* Maximum retry attempts */
#define QSPI_EXAMPLE_TIMEOUT_MS         5000        /* Example timeout */
#define QSPI_EXAMPLE_PRINT_CHUNK_SIZE   16          /* Print chunk size */

/* Private variables ---------------------------------------------------------*/
static QSPI_HandleStructTypeDef hqspi;
static QSPI_TestResultsTypeDef TestResults;
static uint8_t TestBuffer[QSPI_EXAMPLE_BUFFER_SIZE];
static uint8_t ReadBuffer[QSPI_EXAMPLE_BUFFER_SIZE];

/* Private function prototypes -----------------------------------------------*/
static void QSPI_Example_InitTestResults(void);
static uint32_t QSPI_Example_GetTickMs(void);
static void QSPI_Example_UpdateTestResult(bool passed, const char *error_msg);

/* Function implementations --------------------------------------------------*/

/**
 * @brief Initialize QSPI example system
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_Example_Init(void)
{
    QSPI_StatusTypeDef status;

    printf("Initializing QSPI Example System...\n");

    /* Initialize test results */
    QSPI_Example_InitTestResults();

    /* Initialize QSPI driver */
    status = QSPI_Init(&hqspi);
    if (status != QSPI_OK) {
        printf("QSPI initialization failed: %s\n", QSPI_GetStatusString(status));
        QSPI_Example_UpdateTestResult(false, "QSPI Init Failed");
        return status;
    }

    printf("QSPI initialization successful\n");
    QSPI_Example_UpdateTestResult(true, "");
    return QSPI_OK;
}

/**
 * @brief Basic read/write example
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_Example_BasicReadWrite(void)
{
    QSPI_StatusTypeDef status;
    uint32_t test_address = QSPI_EXAMPLE_TEST_ADDRESS;
    uint32_t data_size = 64;

    printf("\n=== Basic Read/Write Test ===\n");

    /* Generate test pattern */
    uint32_t pattern_seed = QSPI_Example_GenerateTestPattern(TestBuffer, data_size, QSPI_EXAMPLE_PATTERN_SEED);
    printf("Generated test pattern with seed: 0x%08X\n", pattern_seed);

    /* Display test data */
    QSPI_Example_PrintBuffer(TestBuffer, data_size, "Test Data to Write");

    /* Write test data to Flash */
    printf("Writing test data to address 0x%08X...\n", test_address);
    status = QSPI_Write(&hqspi, test_address, TestBuffer, data_size);
    if (status != QSPI_OK) {
        printf("Write operation failed: %s\n", QSPI_GetStatusString(status));
        QSPI_Example_UpdateTestResult(false, "Write Failed");
        return status;
    }
    printf("Write operation completed successfully\n");

    /* Read back the data */
    memset(ReadBuffer, 0, sizeof(ReadBuffer));
    printf("Reading data back from address 0x%08X...\n", test_address);
    status = QSPI_Read(&hqspi, test_address, ReadBuffer, data_size);
    if (status != QSPI_OK) {
        printf("Read operation failed: %s\n", QSPI_GetStatusString(status));
        QSPI_Example_UpdateTestResult(false, "Read Failed");
        return status;
    }

    QSPI_Example_PrintBuffer(ReadBuffer, data_size, "Read Back Data");

    /* Verify data integrity */
    bool data_match = QSPI_Example_VerifyTestPattern(ReadBuffer, data_size, pattern_seed);
    if (data_match) {
        printf("✓ Data verification successful - Write/Read operation completed\n");
        QSPI_Example_UpdateTestResult(true, "");
    } else {
        printf("✗ Data verification failed - Data mismatch detected\n");
        QSPI_Example_UpdateTestResult(false, "Data Verification Failed");
        return QSPI_ERROR;
    }

    return QSPI_OK;
}

/**
 * @brief Memory detection example
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_Example_MemoryDetection(void)
{
    QSPI_StatusTypeDef status;
    QSPI_MemoryInfoTypeDef memInfo;
    uint8_t device_id[3];

    printf("\n=== Memory Detection Test ===\n");

    /* Read device ID */
    status = QSPI_ReadID(&hqspi, device_id);
    if (status != QSPI_OK) {
        printf("Device ID read failed: %s\n", QSPI_GetStatusString(status));
        QSPI_Example_UpdateTestResult(false, "ID Read Failed");
        TestResults.MemoryDetected = false;
        return status;
    }

    printf("Device ID: 0x%02X 0x%02X 0x%02X\n", device_id[0], device_id[1], device_id[2]);

    /* Get memory information */
    status = QSPI_GetMemoryInfo(&hqspi, &memInfo);
    if (status == QSPI_OK) {
        QSPI_Example_PrintMemoryInfo(&memInfo);
        TestResults.MemoryDetected = true;
    } else {
        printf("Memory info retrieval failed: %s\n", QSPI_GetStatusString(status));
        TestResults.MemoryDetected = false;
    }

    /* Try to read unique ID */
    uint8_t unique_id[8];
    status = QSPI_ReadUniqueID(&hqspi, unique_id);
    if (status == QSPI_OK) {
        printf("Unique ID: ");
        for (int i = 0; i < 8; i++) {
            printf("0x%02X ", unique_id[i]);
        }
        printf("\n");
    } else {
        printf("Unique ID read failed: %s\n", QSPI_GetStatusString(status));
    }

    QSPI_Example_UpdateTestResult(TestResults.MemoryDetected, "");
    return TestResults.MemoryDetected ? QSPI_OK : QSPI_ERROR;
}

/**
 * @brief Speed test example
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_Example_SpeedTest(void)
{
    QSPI_StatusTypeDef status;
    uint32_t start_time, end_time, duration;
    uint32_t test_size = QSPI_EXAMPLE_SPEED_TEST_SIZE;
    uint32_t chunks = test_size / QSPI_EXAMPLE_BUFFER_SIZE;

    printf("\n=== Speed Test ===\n");
    printf("Testing read speed with %lu bytes...\n", test_size);

    /* Read speed test */
    start_time = QSPI_Example_GetTickMs();

    for (uint32_t i = 0; i < chunks; i++) {
        uint32_t address = QSPI_EXAMPLE_TEST_ADDRESS + (i * QSPI_EXAMPLE_BUFFER_SIZE);
        status = QSPI_Read(&hqspi, address, ReadBuffer, QSPI_EXAMPLE_BUFFER_SIZE);
        if (status != QSPI_OK) {
            printf("Read failed at chunk %lu: %s\n", i, QSPI_GetStatusString(status));
            QSPI_Example_UpdateTestResult(false, "Speed Test Read Failed");
            return status;
        }
    }

    end_time = QSPI_Example_GetTickMs();
    duration = end_time - start_time;

    if (duration > 0) {
        TestResults.ReadSpeed = (test_size * 1000) / (duration * 1024);  /* KB/s */
        printf("Read Speed: %lu KB/s (%lu bytes in %lu ms)\n",
               TestResults.ReadSpeed, test_size, duration);
    } else {
        printf("Read completed too quickly to measure\n");
        TestResults.ReadSpeed = 0;
    }

    /* Fast read speed test */
    printf("Testing fast read speed...\n");
    start_time = QSPI_Example_GetTickMs();

    for (uint32_t i = 0; i < chunks; i++) {
        uint32_t address = QSPI_EXAMPLE_TEST_ADDRESS + (i * QSPI_EXAMPLE_BUFFER_SIZE);
        status = QSPI_FastRead(&hqspi, address, ReadBuffer, QSPI_EXAMPLE_BUFFER_SIZE);
        if (status != QSPI_OK) {
            printf("Fast read failed at chunk %lu: %s\n", i, QSPI_GetStatusString(status));
            break;
        }
    }

    end_time = QSPI_Example_GetTickMs();
    duration = end_time - start_time;

    if (duration > 0) {
        uint32_t fast_read_speed = (test_size * 1000) / (duration * 1024);  /* KB/s */
        printf("Fast Read Speed: %lu KB/s (%lu bytes in %lu ms)\n",
               fast_read_speed, test_size, duration);
    } else {
        printf("Fast read completed too quickly to measure\n");
    }

    QSPI_Example_UpdateTestResult(true, "");
    return QSPI_OK;
}

/**
 * @brief Data integrity test example
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_Example_DataIntegrityTest(void)
{
    QSPI_StatusTypeDef status;
    uint32_t test_rounds = 10;
    uint32_t errors = 0;

    printf("\n=== Data Integrity Test ===\n");
    printf("Testing data integrity over %lu rounds...\n", test_rounds);

    for (uint32_t round = 0; round < test_rounds; round++) {
        /* Generate unique test pattern for this round */
        uint32_t seed = QSPI_EXAMPLE_PATTERN_SEED + round;
        QSPI_Example_GenerateTestPattern(TestBuffer, QSPI_EXAMPLE_BUFFER_SIZE, seed);

        /* Read from different addresses */
        uint32_t test_address = QSPI_EXAMPLE_TEST_ADDRESS + (round * QSPI_EXAMPLE_BUFFER_SIZE);

        /* Read data */
        status = QSPI_Read(&hqspi, test_address, ReadBuffer, QSPI_EXAMPLE_BUFFER_SIZE);
        if (status != QSPI_OK) {
            printf("Round %lu: Read failed: %s\n", round + 1, QSPI_GetStatusString(status));
            errors++;
            continue;
        }

        /* Compare first few bytes for demonstration */
        bool data_ok = true;
        for (int i = 0; i < 16; i++) {
            if (ReadBuffer[i] != ((i + seed) & 0xFF)) {
                data_ok = false;
                break;
            }
        }

        if (!data_ok) {
            printf("Round %lu: Data integrity check failed\n", round + 1);
            errors++;
        } else {
            printf("Round %lu: Data integrity OK\n", round + 1);
        }
    }

    TestResults.ErrorRate = (float)errors / test_rounds * 100.0f;
    printf("Data integrity test completed: %.1f%% error rate\n", TestResults.ErrorRate);

    QSPI_Example_UpdateTestResult(errors == 0, "Data Integrity Issues");
    return (errors == 0) ? QSPI_OK : QSPI_ERROR;
}

/**
 * @brief Erase test example
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_Example_EraseTest(void)
{
    QSPI_StatusTypeDef status;
    uint32_t test_address = 0x1000;  /* Use address 0x1000 for erase test */

    printf("\n=== Erase Test ===\n");
    printf("Testing sector erase functionality...\n");

    /* First, write some test data */
    printf("Writing test pattern before erase...\n");
    QSPI_Example_GenerateTestPattern(TestBuffer, 256, 0xABCDEF00);
    status = QSPI_Write(&hqspi, test_address, TestBuffer, 256);
    if (status != QSPI_OK) {
        printf("Write before erase failed: %s\n", QSPI_GetStatusString(status));
        QSPI_Example_UpdateTestResult(false, "Pre-erase Write Failed");
        return status;
    }

    /* Read back to verify data was written */
    status = QSPI_Read(&hqspi, test_address, ReadBuffer, 256);
    if (status == QSPI_OK) {
        printf("Data successfully written before erase\n");
        QSPI_Example_PrintBuffer(ReadBuffer, 32, "Data Before Erase");
    }

    /* Perform sector erase */
    printf("Erasing sector at address 0x%08X...\n", test_address);
    status = QSPI_EraseSector(&hqspi, test_address);
    if (status != QSPI_OK) {
        printf("Sector erase failed: %s\n", QSPI_GetStatusString(status));
        QSPI_Example_UpdateTestResult(false, "Sector Erase Failed");
        return status;
    }
    printf("Sector erase completed successfully\n");

    /* Read back to verify erase */
    memset(ReadBuffer, 0x55, sizeof(ReadBuffer));  /* Fill with non-0xFF pattern */
    status = QSPI_Read(&hqspi, test_address, ReadBuffer, 256);
    if (status != QSPI_OK) {
        printf("Read after erase failed: %s\n", QSPI_GetStatusString(status));
        QSPI_Example_UpdateTestResult(false, "Post-erase Read Failed");
        return status;
    }

    /* Verify erase (should be all 0xFF) */
    bool erase_ok = true;
    for (int i = 0; i < 256; i++) {
        if (ReadBuffer[i] != 0xFF) {
            erase_ok = false;
            break;
        }
    }

    if (erase_ok) {
        printf("✓ Sector erase verification successful - All bytes are 0xFF\n");
        QSPI_Example_UpdateTestResult(true, "");
    } else {
        printf("✗ Sector erase verification failed - Some bytes are not 0xFF\n");
        QSPI_Example_PrintBuffer(ReadBuffer, 32, "Data After Erase");
        QSPI_Example_UpdateTestResult(false, "Erase Verification Failed");
        return QSPI_ERASE_ERROR;
    }

    /* Test block erase */
    printf("\nTesting 64KB block erase...\n");
    uint32_t block_address = 0x10000;  /* 64KB aligned address */

    /* Write test data to multiple locations in the block */
    for (int i = 0; i < 4; i++) {
        uint32_t addr = block_address + (i * 4096);  /* Write to different sectors */
        QSPI_Example_GenerateTestPattern(TestBuffer, 256, 0x12345678 + i);
        status = QSPI_Write(&hqspi, addr, TestBuffer, 256);
        if (status != QSPI_OK) {
            printf("Block pre-write failed at 0x%08X\n", addr);
            break;
        }
    }

    /* Erase the entire 64KB block */
    printf("Erasing 64KB block at address 0x%08X...\n", block_address);
    status = QSPI_EraseBlock64K(&hqspi, block_address);
    if (status != QSPI_OK) {
        printf("64KB block erase failed: %s\n", QSPI_GetStatusString(status));
        QSPI_Example_UpdateTestResult(false, "Block Erase Failed");
        return status;
    }
    printf("64KB block erase completed successfully\n");

    /* Verify block erase by checking multiple locations */
    bool block_erase_ok = true;
    for (int i = 0; i < 4; i++) {
        uint32_t addr = block_address + (i * 4096);
        status = QSPI_Read(&hqspi, addr, ReadBuffer, 256);
        if (status == QSPI_OK) {
            for (int j = 0; j < 256; j++) {
                if (ReadBuffer[j] != 0xFF) {
                    block_erase_ok = false;
                    break;
                }
            }
        }
        if (!block_erase_ok) break;
    }

    if (block_erase_ok) {
        printf("✓ 64KB block erase verification successful\n");
        QSPI_Example_UpdateTestResult(true, "");
    } else {
        printf("✗ 64KB block erase verification failed\n");
        QSPI_Example_UpdateTestResult(false, "Block Erase Verification Failed");
        return QSPI_ERASE_ERROR;
    }

    return QSPI_OK;
}

/**
 * @brief Power management example
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_Example_PowerManagement(void)
{
    QSPI_StatusTypeDef status;

    printf("\n=== Power Management Test ===\n");

    /* Test deep power down */
    printf("Entering deep power down...\n");
    status = QSPI_EnterDeepPowerDown(&hqspi);
    if (status == QSPI_OK) {
        printf("Deep power down successful\n");
    } else {
        printf("Deep power down failed: %s\n", QSPI_GetStatusString(status));
    }

    /* Wait a bit */
    HAL_Delay(100);

    /* Exit deep power down */
    printf("Exiting deep power down...\n");
    status = QSPI_ExitDeepPowerDown(&hqspi);
    if (status == QSPI_OK) {
        printf("Exit deep power down successful\n");
    } else {
        printf("Exit deep power down failed: %s\n", QSPI_GetStatusString(status));
    }

    /* Wait for device to be ready */
    HAL_Delay(10);

    /* Test device responsiveness */
    uint8_t device_id[3];
    status = QSPI_ReadID(&hqspi, device_id);
    if (status == QSPI_OK) {
        printf("Device responsive after power cycle\n");
        QSPI_Example_UpdateTestResult(true, "");
    } else {
        printf("Device not responsive after power cycle\n");
        QSPI_Example_UpdateTestResult(false, "Power Cycle Failed");
    }

    return status;
}

/**
 * @brief Continuous read example
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_Example_ContinuousRead(void)
{
    QSPI_StatusTypeDef status;
    uint32_t total_bytes = 0;
    uint32_t consecutive_reads = 100;

    printf("\n=== Continuous Read Test ===\n");
    printf("Performing %lu consecutive reads...\n", consecutive_reads);

    for (uint32_t i = 0; i < consecutive_reads; i++) {
        uint32_t address = QSPI_EXAMPLE_TEST_ADDRESS + (i % 16) * QSPI_EXAMPLE_BUFFER_SIZE;

        status = QSPI_Read(&hqspi, address, ReadBuffer, 64);
        if (status != QSPI_OK) {
            printf("Read %lu failed: %s\n", i + 1, QSPI_GetStatusString(status));
            QSPI_Example_UpdateTestResult(false, "Continuous Read Failed");
            return status;
        }

        total_bytes += 64;

        if ((i + 1) % 10 == 0) {
            printf("Completed %lu reads (%lu bytes)\n", i + 1, total_bytes);
        }
    }

    printf("Continuous read test completed: %lu bytes read\n", total_bytes);
    QSPI_Example_UpdateTestResult(true, "");
    return QSPI_OK;
}

/**
 * @brief Display memory information
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_Example_MemoryInfo(void)
{
    QSPI_StatusTypeDef status;
    QSPI_MemoryInfoTypeDef memInfo;

    printf("\n=== Memory Information ===\n");

    status = QSPI_GetMemoryInfo(&hqspi, &memInfo);
    if (status == QSPI_OK) {
        QSPI_Example_PrintMemoryInfo(&memInfo);
        QSPI_Example_UpdateTestResult(true, "");
    } else {
        printf("Failed to get memory information: %s\n", QSPI_GetStatusString(status));
        QSPI_Example_UpdateTestResult(false, "Memory Info Failed");
    }

    return status;
}

/**
 * @brief Sector management example
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_Example_SectorManagement(void)
{
    printf("\n=== Sector Management ===\n");

    /* Demonstrate address calculations */
    uint32_t test_addresses[] = {0x0000, 0x1000, 0x2500, 0x10000, 0x55AA};
    uint32_t num_addresses = sizeof(test_addresses) / sizeof(test_addresses[0]);

    for (uint32_t i = 0; i < num_addresses; i++) {
        uint32_t address = test_addresses[i];
        uint32_t sector_addr = QSPI_GetSectorAddress(address);
        uint32_t block_addr = QSPI_GetBlockAddress(address);

        printf("Address 0x%08lX: Sector 0x%08lX, Block 0x%08lX\n",
               address, sector_addr, block_addr);
    }

    /* Validate address range */
    printf("\nAddress validation tests:\n");
    uint32_t test_cases[][2] = {
        {0x0000, 256},      /* Valid */
        {0x1000, 4096},     /* Valid */
        {0xFFFFFF, 1},      /* Invalid - beyond memory */
        {0x100000, 1024}    /* May be invalid depending on Flash size */
    };

    for (int i = 0; i < 4; i++) {
        bool valid = QSPI_IsAddressValid(test_cases[i][0], test_cases[i][1]);
        printf("Address 0x%08lX, Size %lu: %s\n",
               test_cases[i][0], test_cases[i][1], valid ? "Valid" : "Invalid");
    }

    QSPI_Example_UpdateTestResult(true, "");
    return QSPI_OK;
}

/**
 * @brief File system simulation example
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_Example_FileSystem(void)
{
    printf("\n=== File System Simulation ===\n");
    printf("This example demonstrates how QSPI Flash could be used for file storage\n");

    /* Simulate file allocation table */
    printf("Simulated File Allocation:\n");
    printf("- Boot sector:     0x000000 - 0x001000 (4KB)\n");
    printf("- FAT table:       0x001000 - 0x002000 (4KB)\n");
    printf("- Root directory:  0x002000 - 0x003000 (4KB)\n");
    printf("- Data area:       0x003000 - 0x100000 (1MB-12KB)\n");

    /* Demonstrate reading "file" data */
    uint32_t file_address = 0x003000;  /* Start of data area */
    QSPI_StatusTypeDef status = QSPI_Read(&hqspi, file_address, ReadBuffer, 256);

    if (status == QSPI_OK) {
        printf("\nSample 'file' data from address 0x%08lX:\n", file_address);
        QSPI_Example_PrintBuffer(ReadBuffer, 64, "File Data");
        QSPI_Example_UpdateTestResult(true, "");
    } else {
        printf("Failed to read 'file' data: %s\n", QSPI_GetStatusString(status));
        QSPI_Example_UpdateTestResult(false, "File System Test Failed");
    }

    return status;
}

/**
 * @brief Benchmark test
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_Example_Benchmark(void)
{
    printf("\n=== Benchmark Test ===\n");

    /* Run speed test if not already done */
    QSPI_StatusTypeDef status = QSPI_Example_SpeedTest();
    if (status != QSPI_OK) {
        return status;
    }

    /* Calculate theoretical vs actual performance */
    printf("\nPerformance Analysis:\n");
    printf("- Read Speed: %lu KB/s\n", TestResults.ReadSpeed);
    printf("- Theoretical SPI Speed: Depends on clock configuration\n");
    printf("- Protocol Overhead: Command + Address bytes per operation\n");

    QSPI_Example_UpdateTestResult(true, "");
    return QSPI_OK;
}

/**
 * @brief Run all QSPI tests
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_Example_RunAllTests(void)
{
    printf("\n========================================\n");
    printf("         QSPI Complete Test Suite       \n");
    printf("========================================\n");

    /* Initialize */
    if (QSPI_Example_Init() != QSPI_OK) {
        printf("Initialization failed - aborting tests\n");
        return QSPI_ERROR;
    }

    /* Run tests */
    QSPI_Example_MemoryDetection();
    QSPI_Example_MemoryInfo();
    QSPI_Example_BasicReadWrite();
    QSPI_Example_SpeedTest();
    QSPI_Example_DataIntegrityTest();
    QSPI_Example_ContinuousRead();
    QSPI_Example_SectorManagement();
    QSPI_Example_PowerManagement();
    QSPI_Example_FileSystem();
    QSPI_Example_EraseTest();

    /* Print results */
    printf("\n========================================\n");
    printf("            Test Results Summary         \n");
    printf("========================================\n");
    QSPI_Example_PrintTestResults(&TestResults);

    return (TestResults.FailedTests == 0) ? QSPI_OK : QSPI_ERROR;
}

/**
 * @brief Diagnostic test
 * @retval QSPI_StatusTypeDef Status of the operation
 */
QSPI_StatusTypeDef QSPI_Example_DiagnosticTest(void)
{
    printf("\n=== Diagnostic Test ===\n");

    /* Basic connectivity test */
    uint8_t device_id[3];
    QSPI_StatusTypeDef status = QSPI_ReadID(&hqspi, device_id);

    if (status == QSPI_OK) {
        printf("✓ SPI Communication: OK\n");
        printf("✓ Device ID Read: 0x%02X 0x%02X 0x%02X\n",
               device_id[0], device_id[1], device_id[2]);
    } else {
        printf("✗ SPI Communication: FAILED\n");
        printf("✗ Device ID Read: FAILED (%s)\n", QSPI_GetStatusString(status));
    }

    /* Status register test */
    uint8_t status_reg;
    if (QSPI_GetStatus(&hqspi, &status_reg) == QSPI_OK) {
        printf("✓ Status Register: 0x%02X\n", status_reg);
        printf("  - Busy: %s\n", (status_reg & QSPI_SR_BUSY) ? "Yes" : "No");
        printf("  - Write Enable: %s\n", (status_reg & QSPI_SR_WEL) ? "Yes" : "No");
    } else {
        printf("✗ Status Register: Read failed\n");
    }

    QSPI_Example_UpdateTestResult(status == QSPI_OK, "Diagnostic Test");
    return status;
}

/* Utility function implementations ------------------------------------------*/

/**
 * @brief Print test results
 * @param results Pointer to test results structure
 */
void QSPI_Example_PrintTestResults(QSPI_TestResultsTypeDef *results)
{
    printf("Total Tests:      %lu\n", results->TotalTests);
    printf("Passed Tests:     %lu\n", results->PassedTests);
    printf("Failed Tests:     %lu\n", results->FailedTests);
    printf("Success Rate:     %.1f%%\n",
           results->TotalTests > 0 ? (float)results->PassedTests / results->TotalTests * 100.0f : 0.0f);
    printf("Memory Detected:  %s\n", results->MemoryDetected ? "Yes" : "No");
    printf("Read Speed:       %lu KB/s\n", results->ReadSpeed);
    printf("Write Speed:      %lu KB/s\n", results->WriteSpeed);
    printf("Error Rate:       %.1f%%\n", results->ErrorRate);

    if (strlen(results->LastError) > 0) {
        printf("Last Error:       %s\n", results->LastError);
    }
}

/**
 * @brief Print memory information
 * @param memInfo Pointer to memory info structure
 */
void QSPI_Example_PrintMemoryInfo(QSPI_MemoryInfoTypeDef *memInfo)
{
    printf("Device Name:      %s\n", memInfo->DeviceName);
    printf("Manufacturer ID:  0x%02X\n", memInfo->ManufacturerID);
    printf("Device ID:        0x%02X 0x%02X\n", memInfo->DeviceID1, memInfo->DeviceID2);
    printf("Flash Size:       %lu bytes (%.1f MB)\n",
           memInfo->FlashSize, (float)memInfo->FlashSize / (1024 * 1024));
    printf("Page Size:        %lu bytes\n", memInfo->PageSize);
    printf("Sector Size:      %lu bytes\n", memInfo->SectorSize);
    printf("Block Size:       %lu bytes\n", memInfo->BlockSize);
}

/**
 * @brief Print buffer contents in hex format
 * @param buffer Pointer to buffer
 * @param size Buffer size
 * @param title Title for the printout
 */
void QSPI_Example_PrintBuffer(const uint8_t *buffer, uint32_t size, const char *title)
{
    printf("%s (%lu bytes):\n", title, size);

    for (uint32_t i = 0; i < size; i++) {
        if (i % QSPI_EXAMPLE_PRINT_CHUNK_SIZE == 0) {
            printf("%04lX: ", i);
        }

        printf("%02X ", buffer[i]);

        if ((i + 1) % QSPI_EXAMPLE_PRINT_CHUNK_SIZE == 0 || i == size - 1) {
            printf("\n");
        }
    }
}

/**
 * @brief Generate test pattern
 * @param buffer Pointer to buffer
 * @param size Buffer size
 * @param seed Pattern seed
 * @retval Generated seed value
 */
uint32_t QSPI_Example_GenerateTestPattern(uint8_t *buffer, uint32_t size, uint32_t seed)
{
    for (uint32_t i = 0; i < size; i++) {
        buffer[i] = (uint8_t)((i + seed) & 0xFF);
    }
    return seed;
}

/**
 * @brief Verify test pattern
 * @param buffer Pointer to buffer
 * @param size Buffer size
 * @param seed Pattern seed
 * @retval true if pattern matches, false otherwise
 */
bool QSPI_Example_VerifyTestPattern(const uint8_t *buffer, uint32_t size, uint32_t seed)
{
    for (uint32_t i = 0; i < size; i++) {
        if (buffer[i] != (uint8_t)((i + seed) & 0xFF)) {
            return false;
        }
    }
    return true;
}

/* Private function implementations ------------------------------------------*/

/**
 * @brief Initialize test results structure
 */
static void QSPI_Example_InitTestResults(void)
{
    memset(&TestResults, 0, sizeof(QSPI_TestResultsTypeDef));
}

/**
 * @brief Get current tick in milliseconds
 * @retval Current tick count
 */
static uint32_t QSPI_Example_GetTickMs(void)
{
    return HAL_GetTick();
}

/**
 * @brief Update test results
 * @param passed Test passed status
 * @param error_msg Error message (if any)
 */
static void QSPI_Example_UpdateTestResult(bool passed, const char *error_msg)
{
    TestResults.TotalTests++;

    if (passed) {
        TestResults.PassedTests++;
    } else {
        TestResults.FailedTests++;
        if (error_msg && strlen(error_msg) > 0) {
            strncpy(TestResults.LastError, error_msg, QSPI_EXAMPLE_ERROR_MSG_MAX - 1);
            TestResults.LastError[QSPI_EXAMPLE_ERROR_MSG_MAX - 1] = '\0';
        }
    }
}
