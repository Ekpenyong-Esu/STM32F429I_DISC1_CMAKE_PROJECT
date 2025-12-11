/**
 ******************************************************************************
 * @file    sdcard_example.c
 * @author  Mahonri
 * @brief   SD Card example implementation for STM32F429I Discovery board
 *          This file contains example functions demonstrating SD Card usage
 ******************************************************************************
 * @attention
 *
 * This software is provided as-is, without any express or implied warranties.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "sdcard_example.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define EXAMPLE_BUFFER_SIZE      4096    /* 8 blocks */
#define EXAMPLE_TEST_ADDRESS     2048    /* Safe test address */
#define EXAMPLE_STRING_SIZE      256

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint8_t writeBuffer[EXAMPLE_BUFFER_SIZE];
static uint8_t readBuffer[EXAMPLE_BUFFER_SIZE];

/* Private function prototypes -----------------------------------------------*/
static void PrintStatus(SDCARD_StatusTypeDef status);
static void PrintCardInfo(SDCARD_CardInfoTypeDef* pCardInfo);
static void FillTestPattern(uint8_t* buffer, uint32_t size, uint8_t pattern);
static bool VerifyTestPattern(uint8_t* buffer, uint32_t size, uint8_t pattern);

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  SD Card basic example - Initialize and display card information
 * @retval None
 */
void SDCARD_BasicExample(void)
{
    SDCARD_StatusTypeDef status;
    SDCARD_CardInfoTypeDef cardInfo;

    printf("=== SD Card Basic Example ===\n");

    /* Initialize SD Card */
    printf("Initializing SD Card...\n");
    status = SDCARD_Init();
    printf("Initialization: ");
    PrintStatus(status);

    if (status != SDCARD_STATUS_OK)
    {
        printf("Failed to initialize SD Card\n");
        return;
    }

    /* Check if card is present */
    if (SDCARD_IsCardPresent())
    {
        printf("SD Card is present\n");
    }
    else
    {
        printf("No SD Card detected\n");
        return;
    }

    /* Check write protection */
    if (SDCARD_IsWriteProtected())
    {
        printf("SD Card is write protected\n");
    }
    else
    {
        printf("SD Card is writable\n");
    }

    /* Get card information */
    status = SDCARD_GetCardInfo(&cardInfo);
    if (status == SDCARD_STATUS_OK)
    {
        PrintCardInfo(&cardInfo);
    }
    else
    {
        printf("Failed to get card information\n");
    }

    /* Get card capacity */
    uint64_t capacity = SDCARD_GetCapacity();
    printf("Card capacity: %llu bytes (%.2f MB)\n",
           capacity, (double)capacity / (1024.0 * 1024.0));

    printf("=== End Basic Example ===\n\n");
}

/**
 * @brief  SD Card read/write example
 * @retval None
 */
void SDCARD_ReadWriteExample(void)
{
    SDCARD_StatusTypeDef status;
    const char* testString = "Hello, SD Card World! This is a test string for read/write operations.";
    uint32_t testAddress = EXAMPLE_TEST_ADDRESS;

    printf("=== SD Card Read/Write Example ===\n");

    /* Initialize SD Card */
    status = SDCARD_Init();
    if (status != SDCARD_STATUS_OK)
    {
        printf("Failed to initialize SD Card\n");
        return;
    }

    if (!SDCARD_IsCardPresent())
    {
        printf("No SD Card detected\n");
        return;
    }

    if (SDCARD_IsWriteProtected())
    {
        printf("SD Card is write protected - cannot perform write test\n");
        return;
    }

    /* Clear buffers */
    memset(writeBuffer, 0, EXAMPLE_BUFFER_SIZE);
    memset(readBuffer, 0, EXAMPLE_BUFFER_SIZE);

    /* Prepare test data */
    strncpy((char*)writeBuffer, testString, strlen(testString));
    FillTestPattern(writeBuffer + 256, EXAMPLE_BUFFER_SIZE - 256, 0xAA);

    printf("Writing test data to address %lu...\n", testAddress);

    /* Write data */
    status = SDCARD_WriteBlocks(writeBuffer, testAddress,
                               EXAMPLE_BUFFER_SIZE / SDCARD_BLOCK_SIZE,
                               SDCARD_TIMEOUT_WRITE);
    printf("Write status: ");
    PrintStatus(status);

    if (status != SDCARD_STATUS_OK)
    {
        printf("Write operation failed\n");
        return;
    }

    printf("Reading data back...\n");

    /* Read data back */
    status = SDCARD_ReadBlocks(readBuffer, testAddress,
                              EXAMPLE_BUFFER_SIZE / SDCARD_BLOCK_SIZE,
                              SDCARD_TIMEOUT_READ);
    printf("Read status: ");
    PrintStatus(status);

    if (status != SDCARD_STATUS_OK)
    {
        printf("Read operation failed\n");
        return;
    }

    /* Verify data */
    if (memcmp(writeBuffer, readBuffer, EXAMPLE_BUFFER_SIZE) == 0)
    {
        printf("Data verification: PASSED\n");
        printf("Read string: \"%.70s\"\n", (char*)readBuffer);
    }
    else
    {
        printf("Data verification: FAILED\n");

        /* Show first few differences */
        int differences = 0;
        for (int i = 0; i < EXAMPLE_BUFFER_SIZE && differences < 10; i++)
        {
            if (writeBuffer[i] != readBuffer[i])
            {
                printf("Diff at byte %d: wrote 0x%02X, read 0x%02X\n",
                       i, writeBuffer[i], readBuffer[i]);
                differences++;
            }
        }
    }

    printf("=== End Read/Write Example ===\n\n");
}

/**
 * @brief  SD Card DMA operations example
 * @retval None
 */
void SDCARD_DMAExample(void)
{
    SDCARD_StatusTypeDef status;
    uint32_t testAddress = EXAMPLE_TEST_ADDRESS + 100;
    uint32_t timeout;

    printf("=== SD Card DMA Example ===\n");

    /* Initialize SD Card */
    status = SDCARD_Init();
    if (status != SDCARD_STATUS_OK)
    {
        printf("Failed to initialize SD Card\n");
        return;
    }

    if (!SDCARD_IsCardPresent())
    {
        printf("No SD Card detected\n");
        return;
    }

    if (SDCARD_IsWriteProtected())
    {
        printf("SD Card is write protected - cannot perform DMA write test\n");
        return;
    }

    /* Prepare test data */
    FillTestPattern(writeBuffer, EXAMPLE_BUFFER_SIZE, 0x55);
    memset(readBuffer, 0, EXAMPLE_BUFFER_SIZE);

    printf("Starting DMA write operation...\n");

    /* Start DMA write */
    status = SDCARD_WriteBlocks_DMA(writeBuffer, testAddress,
                                   EXAMPLE_BUFFER_SIZE / SDCARD_BLOCK_SIZE);
    printf("DMA Write start: ");
    PrintStatus(status);

    if (status == SDCARD_STATUS_OK)
    {
        /* Wait for completion (simplified polling) */
        timeout = 0;
        while (SDCARD_GetCardState() != HAL_SD_CARD_TRANSFER && timeout < 1000)
        {
            HAL_Delay(1);
            timeout++;
        }

        if (timeout >= 1000)
        {
            printf("DMA write timeout\n");
            return;
        }

        printf("DMA write completed\n");
    }

    printf("Starting DMA read operation...\n");

    /* Start DMA read */
    status = SDCARD_ReadBlocks_DMA(readBuffer, testAddress,
                                  EXAMPLE_BUFFER_SIZE / SDCARD_BLOCK_SIZE);
    printf("DMA Read start: ");
    PrintStatus(status);

    if (status == SDCARD_STATUS_OK)
    {
        /* Wait for completion */
        timeout = 0;
        while (SDCARD_GetCardState() != HAL_SD_CARD_TRANSFER && timeout < 1000)
        {
            HAL_Delay(1);
            timeout++;
        }

        if (timeout >= 1000)
        {
            printf("DMA read timeout\n");
            return;
        }

        printf("DMA read completed\n");

        /* Verify data */
        if (VerifyTestPattern(readBuffer, EXAMPLE_BUFFER_SIZE, 0x55))
        {
            printf("DMA data verification: PASSED\n");
        }
        else
        {
            printf("DMA data verification: FAILED\n");
        }
    }

    printf("=== End DMA Example ===\n\n");
}

/**
 * @brief  SD Card performance test example
 * @retval None
 */
void SDCARD_PerformanceExample(void)
{
    SDCARD_StatusTypeDef status;
    uint32_t readSpeed = 0;
    uint32_t writeSpeed = 0;

    printf("=== SD Card Performance Example ===\n");

    /* Initialize SD Card */
    status = SDCARD_Init();
    if (status != SDCARD_STATUS_OK)
    {
        printf("Failed to initialize SD Card\n");
        return;
    }

    if (!SDCARD_IsCardPresent())
    {
        printf("No SD Card detected\n");
        return;
    }

    printf("Running performance test...\n");
    printf("Note: This will write test data to the SD Card\n");

    if (SDCARD_IsWriteProtected())
    {
        printf("SD Card is write protected - cannot perform performance test\n");
        return;
    }

    /* Run performance test */
    status = SDCARD_TestPerformance(&readSpeed, &writeSpeed);
    printf("Performance test: ");
    PrintStatus(status);

    if (status == SDCARD_STATUS_OK)
    {
        printf("Read speed:  %lu KB/s (%.2f MB/s)\n",
               readSpeed, (double)readSpeed / 1024.0);
        printf("Write speed: %lu KB/s (%.2f MB/s)\n",
               writeSpeed, (double)writeSpeed / 1024.0);

        /* Performance classification */
        if (readSpeed > 10000) /* > 10 MB/s */
        {
            printf("Read performance: Excellent (Class 10+)\n");
        }
        else if (readSpeed > 6000) /* > 6 MB/s */
        {
            printf("Read performance: Good (Class 6+)\n");
        }
        else if (readSpeed > 4000) /* > 4 MB/s */
        {
            printf("Read performance: Moderate (Class 4+)\n");
        }
        else
        {
            printf("Read performance: Slow (Class 2 or lower)\n");
        }

        if (writeSpeed > 10000) /* > 10 MB/s */
        {
            printf("Write performance: Excellent (Class 10+)\n");
        }
        else if (writeSpeed > 6000) /* > 6 MB/s */
        {
            printf("Write performance: Good (Class 6+)\n");
        }
        else if (writeSpeed > 4000) /* > 4 MB/s */
        {
            printf("Write performance: Moderate (Class 4+)\n");
        }
        else
        {
            printf("Write performance: Slow (Class 2 or lower)\n");
        }
    }

    printf("=== End Performance Example ===\n\n");
}

/**
 * @brief  SD Card format example
 * @retval None
 */
void SDCARD_FormatExample(void)
{
    SDCARD_StatusTypeDef status;
    char input[10];

    printf("=== SD Card Format Example ===\n");
    printf("WARNING: This will erase all data on the SD Card!\n");
    printf("Type 'yes' to continue, anything else to cancel: ");

    /* Simple input simulation - in real application, get user input */
    strcpy(input, "no"); /* Default to not format for safety */
    printf("%s\n", input);

    if (strcmp(input, "yes") != 0)
    {
        printf("Format cancelled by user\n");
        printf("=== End Format Example ===\n\n");
        return;
    }

    /* Initialize SD Card */
    status = SDCARD_Init();
    if (status != SDCARD_STATUS_OK)
    {
        printf("Failed to initialize SD Card\n");
        return;
    }

    if (!SDCARD_IsCardPresent())
    {
        printf("No SD Card detected\n");
        return;
    }

    if (SDCARD_IsWriteProtected())
    {
        printf("SD Card is write protected - cannot format\n");
        return;
    }

    printf("Starting quick format...\n");

    /* Perform quick format */
    status = SDCARD_Format(0); /* 0 = quick format */
    printf("Format status: ");
    PrintStatus(status);

    if (status == SDCARD_STATUS_OK)
    {
        printf("SD Card formatted successfully\n");
        printf("Note: You may need to create a file system to use the card with a PC\n");
    }
    else
    {
        printf("Format failed\n");
    }

    printf("=== End Format Example ===\n\n");
}

/**
 * @brief  SD Card file operations example (basic file system simulation)
 * @retval None
 */
void SDCARD_FileOperationsExample(void)
{
    SDCARD_StatusTypeDef status;
    const char* fileData = "This is a simulated file content.\nLine 2 of the file.\nEnd of file.";
    uint32_t fileAddress = EXAMPLE_TEST_ADDRESS + 200;
    char readData[EXAMPLE_STRING_SIZE];

    printf("=== SD Card File Operations Example ===\n");
    printf("Note: This is a basic simulation without a real file system\n");

    /* Initialize SD Card */
    status = SDCARD_Init();
    if (status != SDCARD_STATUS_OK)
    {
        printf("Failed to initialize SD Card\n");
        return;
    }

    if (!SDCARD_IsCardPresent())
    {
        printf("No SD Card detected\n");
        return;
    }

    if (SDCARD_IsWriteProtected())
    {
        printf("SD Card is write protected - cannot perform file operations\n");
        return;
    }

    /* Simulate file write */
    printf("Writing simulated file...\n");
    memset(writeBuffer, 0, SDCARD_BLOCK_SIZE);
    strncpy((char*)writeBuffer, fileData, strlen(fileData));

    status = SDCARD_WriteBlocks(writeBuffer, fileAddress, 1, SDCARD_TIMEOUT_WRITE);
    printf("File write: ");
    PrintStatus(status);

    if (status != SDCARD_STATUS_OK)
    {
        printf("File write failed\n");
        return;
    }

    /* Simulate file read */
    printf("Reading simulated file...\n");
    memset(readBuffer, 0, SDCARD_BLOCK_SIZE);

    status = SDCARD_ReadBlocks(readBuffer, fileAddress, 1, SDCARD_TIMEOUT_READ);
    printf("File read: ");
    PrintStatus(status);

    if (status == SDCARD_STATUS_OK)
    {
        /* Copy to string buffer for safe printing */
        strncpy(readData, (char*)readBuffer, sizeof(readData) - 1);
        readData[sizeof(readData) - 1] = '\0';

        printf("File content:\n");
        printf("--- Start of File ---\n");
        printf("%s\n", readData);
        printf("--- End of File ---\n");

        /* Verify file integrity */
        if (strncmp(fileData, readData, strlen(fileData)) == 0)
        {
            printf("File integrity: PASSED\n");
        }
        else
        {
            printf("File integrity: FAILED\n");
        }
    }

    /* Simulate file append */
    printf("Appending to simulated file...\n");
    const char* appendData = "\nAppended line 1\nAppended line 2";

    /* Read existing content */
    memset(writeBuffer, 0, SDCARD_BLOCK_SIZE);
    memcpy(writeBuffer, readBuffer, SDCARD_BLOCK_SIZE);

    /* Find end of existing content */
    size_t existingLen = strlen((char*)writeBuffer);
    if (existingLen < SDCARD_BLOCK_SIZE - strlen(appendData) - 1)
    {
        strncpy((char*)writeBuffer + existingLen, appendData, strlen(appendData));

        status = SDCARD_WriteBlocks(writeBuffer, fileAddress, 1, SDCARD_TIMEOUT_WRITE);
        printf("File append: ");
        PrintStatus(status);

        if (status == SDCARD_STATUS_OK)
        {
            /* Read back appended file */
            memset(readBuffer, 0, SDCARD_BLOCK_SIZE);
            status = SDCARD_ReadBlocks(readBuffer, fileAddress, 1, SDCARD_TIMEOUT_READ);

            if (status == SDCARD_STATUS_OK)
            {
                strncpy(readData, (char*)readBuffer, sizeof(readData) - 1);
                readData[sizeof(readData) - 1] = '\0';

                printf("Updated file content:\n");
                printf("--- Start of File ---\n");
                printf("%s\n", readData);
                printf("--- End of File ---\n");
            }
        }
    }
    else
    {
        printf("Not enough space to append data\n");
    }

    printf("=== End File Operations Example ===\n\n");
}

/**
 * @brief  SD Card complete example - Demonstrates all features
 * @retval None
 */
void SDCARD_CompleteExample(void)
{
    printf("=== SD Card Complete Example ===\n");
    printf("Running all SD Card examples...\n\n");

    /* Run all examples */
    SDCARD_BasicExample();
    SDCARD_ReadWriteExample();
    SDCARD_DMAExample();
    SDCARD_PerformanceExample();
    SDCARD_FileOperationsExample();
    /* Note: Format example is commented out for safety */
    /* SDCARD_FormatExample(); */

    printf("=== All SD Card Examples Complete ===\n");
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Print SD Card status
 * @param  status: SD Card status to print
 * @retval None
 */
static void PrintStatus(SDCARD_StatusTypeDef status)
{
    switch (status)
    {
        case SDCARD_STATUS_OK:
            printf("OK\n");
            break;
        case SDCARD_STATUS_ERROR:
            printf("ERROR\n");
            break;
        case SDCARD_STATUS_BUSY:
            printf("BUSY\n");
            break;
        case SDCARD_STATUS_TIMEOUT:
            printf("TIMEOUT\n");
            break;
        case SDCARD_STATUS_NOT_READY:
            printf("NOT READY\n");
            break;
        case SDCARD_STATUS_NO_CARD:
            printf("NO CARD\n");
            break;
        case SDCARD_STATUS_WRITE_PROTECTED:
            printf("WRITE PROTECTED\n");
            break;
        default:
            printf("UNKNOWN\n");
            break;
    }
}

/**
 * @brief  Print SD Card information
 * @param  pCardInfo: Pointer to card information structure
 * @retval None
 */
static void PrintCardInfo(SDCARD_CardInfoTypeDef* pCardInfo)
{
    if (pCardInfo == NULL)
    {
        return;
    }

    printf("\n--- SD Card Information ---\n");
    printf("Card Type: 0x%08lX\n", pCardInfo->CardType);
    printf("Card Version: 0x%08lX\n", pCardInfo->CardVersion);
    printf("Card Class: 0x%08lX\n", pCardInfo->Class);
    printf("Relative Card Address: 0x%08lX\n", pCardInfo->RelCardAdd);
    printf("Block Number: %lu\n", pCardInfo->BlockNbr);
    printf("Block Size: %lu bytes\n", pCardInfo->BlockSize);
    printf("Logical Block Number: %lu\n", pCardInfo->LogBlockNbr);
    printf("Logical Block Size: %lu bytes\n", pCardInfo->LogBlockSize);
    printf("Card Size: %lu MB\n", pCardInfo->CardSize);
    printf("Write Protected: %s\n", pCardInfo->WriteProtected ? "Yes" : "No");
    printf("----------------------------\n\n");
}

/**
 * @brief  Fill buffer with test pattern
 * @param  buffer: Buffer to fill
 * @param  size: Size of buffer
 * @param  pattern: Pattern byte
 * @retval None
 */
static void FillTestPattern(uint8_t* buffer, uint32_t size, uint8_t pattern)
{
    for (uint32_t i = 0; i < size; i++)
    {
        buffer[i] = (uint8_t)(pattern + (i % 256));
    }
}

/**
 * @brief  Verify buffer contains expected test pattern
 * @param  buffer: Buffer to verify
 * @param  size: Size of buffer
 * @param  pattern: Expected pattern byte
 * @retval bool: true if pattern matches, false otherwise
 */
static bool VerifyTestPattern(uint8_t* buffer, uint32_t size, uint8_t pattern)
{
    for (uint32_t i = 0; i < size; i++)
    {
        if (buffer[i] != (uint8_t)(pattern + (i % 256)))
        {
            return false;
        }
    }
    return true;
}

/**
 * @brief  SD Card transfer complete callback implementation
 * @retval None
 */
void SDCARD_TxCpltCallback(void)
{
    printf("SD Card DMA Tx Complete\n");
}

/**
 * @brief  SD Card receive complete callback implementation
 * @retval None
 */
void SDCARD_RxCpltCallback(void)
{
    printf("SD Card DMA Rx Complete\n");
}

/**
 * @brief  SD Card error callback implementation
 * @retval None
 */
void SDCARD_ErrorCallback(void)
{
    printf("SD Card Error occurred!\n");
}
