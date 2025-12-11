/**
  ******************************************************************************
  * @file    rng_example.c
  * @brief   Random Number Generator (RNG) usage examples
  * @details This file provides example code demonstrating how to use
  *          the hardware RNG for various random number generation scenarios.
  * @version 1.0
  * @date    2025-12-11
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "rng.h"
#include <stdio.h>

/* Example 1: Basic random number generation */
void RNG_Example_Basic(void)
{
    printf("=== RNG Basic Example ===\r\n");

    /* Initialize RNG */
    RNG_StatusTypeDef status = RNG_Init();
    if (status != RNG_OK)
    {
        printf("RNG initialization failed: %s\r\n", RNG_GetStatusString(status));
        return;
    }

    printf("RNG initialized successfully\r\n\r\n");

    /* Generate and display 10 random numbers */
    printf("Generating 10 random numbers:\r\n");
    for (int i = 0; i < 10; i++)
    {
        uint32_t randomNumber = 0;
        status = RNG_Generate(&randomNumber);
        if (status == RNG_OK)
        {
            printf("  [%2d] 0x%08X (%u)\r\n", i + 1, (unsigned int)randomNumber, (unsigned int)randomNumber);
        }
        else
        {
            printf("  [%2d] Error: %s\r\n", i + 1, RNG_GetStatusString(status));
        }
    }

    printf("\r\nBasic example complete\r\n");
}

/* Example 2: Range-limited random numbers */
void RNG_Example_Range(void)
{
    printf("=== RNG Range Example ===\r\n");

    RNG_Init();

    /* Dice roll simulation (1-6) */
    printf("\r\nDice rolls (1-6):\r\n");
    for (int i = 0; i < 10; i++)
    {
        uint32_t dice = 0;
        RNG_GenerateInRange(1, 6, &dice);
        printf("  Roll %2d: %u\r\n", i + 1, (unsigned int)dice);
    }

    /* Coin flip simulation */
    printf("\r\nCoin flips:\r\n");
    for (int i = 0; i < 10; i++)
    {
        bool heads = false;
        RNG_GenerateBool(&heads);
        printf("  Flip %2d: %s\r\n", i + 1, heads ? "Heads" : "Tails");
    }

    /* Percentage (0-100) */
    printf("\r\nRandom percentages (0-100):\r\n");
    for (int i = 0; i < 5; i++)
    {
        uint32_t percent = 0;
        RNG_GenerateInRange(0, 100, &percent);
        printf("  Sample %d: %u%%\r\n", i + 1, (unsigned int)percent);
    }
}

/* Example 3: Random bytes for cryptographic use */
void RNG_Example_Bytes(void)
{
    printf("=== RNG Bytes Example ===\r\n");

    RNG_Init();

    /* Generate random key (16 bytes / 128 bits) */
    uint8_t key[16];
    RNG_StatusTypeDef status = RNG_GenerateBytes(key, sizeof(key));

    if (status == RNG_OK)
    {
        printf("\r\n128-bit random key:\r\n  ");
        for (int i = 0; i < 16; i++)
        {
            printf("%02X", key[i]);
            if ((i + 1) % 4 == 0) printf(" ");
        }
        printf("\r\n");
    }

    /* Generate random IV (12 bytes for AES-GCM) */
    uint8_t iv[12];
    status = RNG_GenerateBytes(iv, sizeof(iv));

    if (status == RNG_OK)
    {
        printf("\r\n96-bit random IV:\r\n  ");
        for (int i = 0; i < 12; i++)
        {
            printf("%02X", iv[i]);
            if ((i + 1) % 4 == 0) printf(" ");
        }
        printf("\r\n");
    }

    /* Generate UUID-like random ID */
    uint8_t uuid[16];
    status = RNG_GenerateBytes(uuid, sizeof(uuid));

    if (status == RNG_OK)
    {
        printf("\r\nRandom UUID:\r\n  ");
        printf("%02X%02X%02X%02X-", uuid[0], uuid[1], uuid[2], uuid[3]);
        printf("%02X%02X-", uuid[4], uuid[5]);
        printf("%02X%02X-", uuid[6], uuid[7]);
        printf("%02X%02X-", uuid[8], uuid[9]);
        printf("%02X%02X%02X%02X%02X%02X\r\n",
               uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
    }
}

/* Example 4: Random float generation */
void RNG_Example_Float(void)
{
    printf("=== RNG Float Example ===\r\n");

    RNG_Init();

    printf("\r\nRandom floats [0.0, 1.0):\r\n");
    for (int i = 0; i < 10; i++)
    {
        float randomFloat = 0.0f;
        RNG_GenerateFloat(&randomFloat);
        printf("  [%2d] %.6f\r\n", i + 1, randomFloat);
    }

    /* Generate random coordinates */
    printf("\r\nRandom 2D coordinates [0,1):\r\n");
    for (int i = 0; i < 5; i++)
    {
        float x = 0.0f, y = 0.0f;
        RNG_GenerateFloat(&x);
        RNG_GenerateFloat(&y);
        printf("  Point %d: (%.4f, %.4f)\r\n", i + 1, x, y);
    }
}

/* Example 5: Buffer generation */
void RNG_Example_Buffer(void)
{
    printf("=== RNG Buffer Example ===\r\n");

    RNG_Init();

    /* Generate buffer of random 32-bit values */
    uint32_t buffer[8];
    RNG_StatusTypeDef status = RNG_GenerateBuffer(buffer, 8);

    if (status == RNG_OK)
    {
        printf("\r\nRandom buffer (8 x 32-bit):\r\n");
        for (int i = 0; i < 8; i++)
        {
            printf("  [%d] 0x%08X\r\n", i, (unsigned int)buffer[i]);
        }
    }

    /* Calculate simple statistics */
    printf("\r\nStatistical test (1000 samples):\r\n");
    uint32_t sum = 0;
    uint32_t min = 0xFFFFFFFF;
    uint32_t max = 0;

    for (int i = 0; i < 1000; i++)
    {
        uint32_t value = 0;
        RNG_Generate(&value);
        sum += (value >> 22);  /* Use top 10 bits to avoid overflow */
        if (value < min) min = value;
        if (value > max) max = value;
    }

    printf("  Min: 0x%08X\r\n", (unsigned int)min);
    printf("  Max: 0x%08X\r\n", (unsigned int)max);
    printf("  Avg (top 10 bits): %u\r\n", (unsigned int)(sum / 1000));
}

/* Example 6: Error handling */
void RNG_Example_ErrorHandling(void)
{
    printf("=== RNG Error Handling Example ===\r\n");

    RNG_StatusTypeDef status = RNG_Init();
    if (status != RNG_OK)
    {
        printf("Init error: %s\r\n", RNG_GetStatusString(status));

        /* Try to recover */
        RNG_ClearErrors();
        status = RNG_Init();
        if (status != RNG_OK)
        {
            printf("Recovery failed!\r\n");
            return;
        }
    }

    /* Check RNG status before generation */
    if (RNG_HasError())
    {
        printf("RNG has errors, clearing...\r\n");
        RNG_ClearErrors();
    }

    /* Generate with explicit error checking */
    uint32_t random = 0;
    status = RNG_Generate(&random);

    printf("\r\nGeneration status: %s\r\n", RNG_GetStatusString(status));
    printf("RNG ready: %s\r\n", RNG_IsReady() ? "Yes" : "No");
    printf("RNG has error: %s\r\n", RNG_HasError() ? "Yes" : "No");

    if (status == RNG_OK)
    {
        printf("Random value: 0x%08X\r\n", (unsigned int)random);
    }
}

/* Example 7: Simple game - Guessing game */
void RNG_Example_GuessingGame(void)
{
    printf("=== RNG Guessing Game Example ===\r\n");

    RNG_Init();

    uint32_t secretNumber = 0;
    RNG_GenerateInRange(1, 100, &secretNumber);

    printf("\r\nI'm thinking of a number between 1 and 100.\r\n");
    printf("(The secret number is: %u - for demo purposes)\r\n\r\n", (unsigned int)secretNumber);

    /* Simulate guessing with random guesses */
    printf("Simulating random guesses:\r\n");
    for (int attempt = 1; attempt <= 7; attempt++)
    {
        uint32_t guess = 0;
        RNG_GenerateInRange(1, 100, &guess);

        printf("  Attempt %d: Guessed %u - ", attempt, (unsigned int)guess);

        if (guess == secretNumber)
        {
            printf("CORRECT!\r\n");
            break;
        }
        else if (guess < secretNumber)
        {
            printf("Too low\r\n");
        }
        else
        {
            printf("Too high\r\n");
        }
    }
}

/**
 * @brief   Run all RNG examples
 */
void RNG_RunExamples(void)
{
    printf("\r\n========================================\r\n");
    printf("      RNG Driver Examples\r\n");
    printf("========================================\r\n\r\n");

    RNG_Example_Basic();
    printf("\r\n");

    RNG_Example_Range();
    printf("\r\n");

    RNG_Example_Bytes();
    printf("\r\n");

    RNG_Example_Float();
    printf("\r\n");

    RNG_Example_Buffer();
    printf("\r\n");

    RNG_Example_ErrorHandling();
    printf("\r\n");

    RNG_Example_GuessingGame();

    /* Cleanup */
    RNG_DeInit();
}
