/**
  ******************************************************************************
  * @file    qspi_example.h
  * @brief   QSPI driver example header for STM32F429 Discovery Board
  * @details This file contains example functions and configurations for
  *          using external QSPI Flash memory with the STM32F429 Discovery board.
  * @version 1.0
  * @date    2025-09-04
  ******************************************************************************
  */

#ifndef QSPI_EXAMPLE_H
#define QSPI_EXAMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "qspi.h"

/* Exported constants --------------------------------------------------------*/
#define QSPI_EXAMPLE_TEST_SIZE          1024    /* Test data size */
#define QSPI_EXAMPLE_TEST_ADDRESS       0x0000  /* Test start address */
#define QSPI_EXAMPLE_BUFFER_SIZE        256     /* Buffer size for operations */
#define QSPI_EXAMPLE_ERROR_MSG_MAX      64      /* Maximum error message length */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief QSPI test results structure
 */
typedef struct {
    uint32_t TotalTests;                /**< Total number of tests */
    uint32_t PassedTests;               /**< Number of passed tests */
    uint32_t FailedTests;               /**< Number of failed tests */
    uint32_t ReadSpeed;                 /**< Read speed in KB/s */
    uint32_t WriteSpeed;                /**< Write speed in KB/s */
    float ErrorRate;                    /**< Error rate percentage */
    bool MemoryDetected;                /**< Memory detection status */
    char LastError[QSPI_EXAMPLE_ERROR_MSG_MAX];                 /**< Last error description */
} QSPI_TestResultsTypeDef;

/* Exported function prototypes ---------------------------------------------*/

/* Basic examples */
QSPI_StatusTypeDef QSPI_Example_Init(void);
QSPI_StatusTypeDef QSPI_Example_BasicReadWrite(void);
QSPI_StatusTypeDef QSPI_Example_MemoryDetection(void);
QSPI_StatusTypeDef QSPI_Example_SpeedTest(void);

/* Advanced examples */
QSPI_StatusTypeDef QSPI_Example_DataIntegrityTest(void);
QSPI_StatusTypeDef QSPI_Example_EraseTest(void);
QSPI_StatusTypeDef QSPI_Example_PowerManagement(void);
QSPI_StatusTypeDef QSPI_Example_ContinuousRead(void);

/* Utility examples */
QSPI_StatusTypeDef QSPI_Example_MemoryInfo(void);
QSPI_StatusTypeDef QSPI_Example_SectorManagement(void);
QSPI_StatusTypeDef QSPI_Example_FileSystem(void);
QSPI_StatusTypeDef QSPI_Example_Benchmark(void);

/* Test functions */
QSPI_StatusTypeDef QSPI_Example_RunAllTests(void);
QSPI_StatusTypeDef QSPI_Example_DiagnosticTest(void);
void QSPI_Example_PrintTestResults(QSPI_TestResultsTypeDef *results);

/* Utility functions */
void QSPI_Example_PrintMemoryInfo(QSPI_MemoryInfoTypeDef *memInfo);
void QSPI_Example_PrintBuffer(const uint8_t *buffer, uint32_t size, const char *title);
uint32_t QSPI_Example_GenerateTestPattern(uint8_t *buffer, uint32_t size, uint32_t seed);
bool QSPI_Example_VerifyTestPattern(const uint8_t *buffer, uint32_t size, uint32_t seed);

#ifdef __cplusplus
}
#endif

#endif /* QSPI_EXAMPLE_H */
