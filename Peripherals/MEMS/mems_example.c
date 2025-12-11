/**
  ******************************************************************************
  * @file    mems_example.c
  * @brief   MEMS sensor examples implementation for STM32F429 Discovery Board
  * @details This file provides implementation of various MEMS sensor examples
  *          demonstrating different use cases and capabilities.
  * @version 1.0
  * @date    2025-09-04
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "mems_example.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* Private constants ---------------------------------------------------------*/
#define PRINTF_BUFFER_SIZE              256
#define MOTION_THRESHOLD_DEFAULT        50.0f   /* dps */
#define TEMPERATURE_OFFSET              25.0f   /* degrees C */
#define DATA_COLLECTION_DELAY           10      /* ms */
#define PERFORMANCE_TEST_SAMPLES        1000
#define CALIBRATION_SETTLE_TIME         2000    /* ms */

/* Private variables ---------------------------------------------------------*/
static char printf_buffer[PRINTF_BUFFER_SIZE];

/* Private function prototypes -----------------------------------------------*/
static void delay_ms(uint32_t ms);
static float calculate_magnitude(const MEMS_AxesTypeDef *axes);
static bool is_motion_detected(const MEMS_AxesTypeDef *current,
                              const MEMS_AxesTypeDef *previous,
                              float threshold);

/* Exported functions --------------------------------------------------------*/

/**
 * @brief Basic MEMS sensor example
 * @param hmems Pointer to MEMS handle structure
 * @param hspi Pointer to SPI handle structure
 * @retval MEMS_ExampleResultTypeDef Example execution result
 */
MEMS_ExampleResultTypeDef MEMS_Example_Basic(MEMS_HandleTypeDef *hmems, SPI_HandleTypeDef *hspi)
{
    MEMS_ExampleResultTypeDef result = {0};
    MEMS_StatusTypeDef status;
    MEMS_DeviceInfoTypeDef device_info;
    MEMS_AxesTypeDef gyro_data;
    float temperature = 0.0f;
    uint32_t start_tick;

    result.Description = "Basic MEMS Sensor Initialization and Reading";
    start_tick = MEMS_Example_GetTick();

    printf("Starting Basic MEMS Example...\n");

    /* Initialize MEMS sensor */
    status = MEMS_Init(hmems, hspi);
    if (status != MEMS_OK) {
        printf("ERROR: MEMS initialization failed (Status: %d)\n", status);
        result.Success = false;
        result.ExecutionTime = MEMS_Example_GetTick() - start_tick;
        return result;
    }

    printf("MEMS sensor initialized successfully\n");

    /* Get device information */
    status = MEMS_GetDeviceInfo(hmems, &device_info);
    if (status == MEMS_OK) {
        printf("Device: %s\n", device_info.DeviceName);
        printf("WHO_AM_I: 0x%02X\n", device_info.WhoAmI);
        printf("Present: %s\n", device_info.IsPresent ? "Yes" : "No");
    }

    /* Read gyroscope data */
    status = MEMS_GyroRead(hmems, &gyro_data);
    if (status == MEMS_OK) {
        printf("Gyroscope Data (dps):\n");
        MEMS_Example_PrintData(&gyro_data, "Gyro");
        result.AverageValue = calculate_magnitude(&gyro_data);
    } else {
        printf("ERROR: Failed to read gyroscope data\n");
        result.Success = false;
        result.ExecutionTime = MEMS_Example_GetTick() - start_tick;
        return result;
    }

    /* Read temperature */
    status = MEMS_ReadTemperature(hmems, &temperature);
    if (status == MEMS_OK) {
        printf("Temperature: %.2f°C\n", temperature);
    }

    result.Success = true;
    result.ExecutionTime = MEMS_Example_GetTick() - start_tick;
    result.MaxValue = result.AverageValue;
    result.MinValue = result.AverageValue;

    printf("Basic MEMS Example completed successfully\n\n");
    return result;
}

/**
 * @brief Gyroscope data reading example
 * @param hmems Pointer to MEMS handle structure
 * @param duration_ms Duration of data collection in milliseconds
 * @retval MEMS_ExampleResultTypeDef Example execution result
 */
MEMS_ExampleResultTypeDef MEMS_Example_GyroReading(MEMS_HandleTypeDef *hmems, uint32_t duration_ms)
{
    MEMS_ExampleResultTypeDef result = {0};
    MEMS_StatusTypeDef status;
    MEMS_AxesTypeDef gyro_data;
    uint32_t start_tick, current_tick;
    uint32_t sample_count = 0;
    float sum_magnitude = 0.0f;
    float max_magnitude = 0.0f;
    float min_magnitude = 1000000.0f;
    float magnitude;

    result.Description = "Continuous Gyroscope Data Reading";
    start_tick = MEMS_Example_GetTick();

    printf("Starting Gyroscope Reading Example (Duration: %lu ms)...\n", duration_ms);

    if (!hmems->IsInitialized) {
        printf("ERROR: MEMS sensor not initialized\n");
        result.Success = false;
        return result;
    }

    printf("Reading gyroscope data continuously...\n");
    printf("Time(ms)\tX(dps)\t\tY(dps)\t\tZ(dps)\t\tMagnitude\n");
    printf("---------------------------------------------------------------\n");

    current_tick = start_tick;
    while ((current_tick - start_tick) < duration_ms) {
        status = MEMS_GyroRead(hmems, &gyro_data);
        if (status == MEMS_OK) {
            magnitude = calculate_magnitude(&gyro_data);

            /* Update statistics */
            sum_magnitude += magnitude;
            if (magnitude > max_magnitude) {
                max_magnitude = magnitude;
            }
            if (magnitude < min_magnitude) {
                min_magnitude = magnitude;
            }
            sample_count++;

            /* Print every 100ms */
            if ((sample_count % 10) == 0) {
                printf("%lu\t\t%.2f\t\t%.2f\t\t%.2f\t\t%.2f\n",
                       current_tick - start_tick, gyro_data.X, gyro_data.Y, gyro_data.Z, magnitude);
            }
        }

        delay_ms(DATA_COLLECTION_DELAY);
        current_tick = MEMS_Example_GetTick();
    }

    if (sample_count > 0) {
        result.AverageValue = sum_magnitude / sample_count;
        result.MaxValue = max_magnitude;
        result.MinValue = min_magnitude;
        result.Success = true;

        printf("\nStatistics:\n");
        printf("Samples collected: %lu\n", sample_count);
        printf("Average magnitude: %.2f dps\n", result.AverageValue);
        printf("Maximum magnitude: %.2f dps\n", result.MaxValue);
        printf("Minimum magnitude: %.2f dps\n", result.MinValue);
    } else {
        result.Success = false;
        printf("ERROR: No valid samples collected\n");
    }

    result.ExecutionTime = MEMS_Example_GetTick() - start_tick;
    printf("Gyroscope Reading Example completed\n\n");
    return result;
}

/**
 * @brief Gyroscope calibration example
 * @param hmems Pointer to MEMS handle structure
 * @retval MEMS_ExampleResultTypeDef Example execution result
 */
MEMS_ExampleResultTypeDef MEMS_Example_GyroCalibration(MEMS_HandleTypeDef *hmems)
{
    MEMS_ExampleResultTypeDef result = {0};
    MEMS_StatusTypeDef status;
    MEMS_AxesTypeDef gyro_before, gyro_after;
    uint32_t start_tick;

    result.Description = "Gyroscope Calibration";
    start_tick = MEMS_Example_GetTick();

    printf("Starting Gyroscope Calibration Example...\n");

    if (!hmems->IsInitialized) {
        printf("ERROR: MEMS sensor not initialized\n");
        result.Success = false;
        return result;
    }

    /* Read data before calibration */
    status = MEMS_GyroRead(hmems, &gyro_before);
    if (status == MEMS_OK) {
        printf("Data before calibration:\n");
        MEMS_Example_PrintData(&gyro_before, "Before");
    }

    printf("Please keep the device stationary during calibration...\n");
    delay_ms(CALIBRATION_SETTLE_TIME);

    /* Perform calibration */
    printf("Performing calibration with 200 samples...\n");
    status = MEMS_CalibrateGyroscope(hmems, 200);
    if (status != MEMS_OK) {
        printf("ERROR: Calibration failed (Status: %d)\n", status);
        result.Success = false;
        result.ExecutionTime = MEMS_Example_GetTick() - start_tick;
        return result;
    }

    printf("Calibration completed successfully\n");
    printf("Calibration offsets:\n");
    printf("X: %.3f dps\n", hmems->CalibrationOffset.X);
    printf("Y: %.3f dps\n", hmems->CalibrationOffset.Y);
    printf("Z: %.3f dps\n", hmems->CalibrationOffset.Z);

    /* Read data after calibration */
    delay_ms(100);
    status = MEMS_GyroRead(hmems, &gyro_after);
    if (status == MEMS_OK) {
        printf("\nData after calibration:\n");
        MEMS_Example_PrintData(&gyro_after, "After");

        /* Calculate improvement */
        float before_magnitude = calculate_magnitude(&gyro_before);
        float after_magnitude = calculate_magnitude(&gyro_after);

        printf("\nCalibration improvement:\n");
        printf("Before magnitude: %.3f dps\n", before_magnitude);
        printf("After magnitude:  %.3f dps\n", after_magnitude);
        printf("Improvement:      %.3f dps (%.1f%%)\n",
               before_magnitude - after_magnitude,
               ((before_magnitude - after_magnitude) / before_magnitude) * 100.0f);

        result.AverageValue = after_magnitude;
        result.MaxValue = before_magnitude;
        result.MinValue = after_magnitude;
    }

    result.Success = true;
    result.ExecutionTime = MEMS_Example_GetTick() - start_tick;

    printf("Gyroscope Calibration Example completed\n\n");
    return result;
}

/**
 * @brief Temperature monitoring example
 * @param hmems Pointer to MEMS handle structure
 * @param sample_count Number of temperature samples to collect
 * @retval MEMS_ExampleResultTypeDef Example execution result
 */
MEMS_ExampleResultTypeDef MEMS_Example_TemperatureMonitoring(MEMS_HandleTypeDef *hmems, uint16_t sample_count)
{
    MEMS_ExampleResultTypeDef result = {0};
    MEMS_StatusTypeDef status;
    float temperature;
    float temp_sum = 0.0f;
    float temp_max = -100.0f;
    float temp_min = 100.0f;
    uint16_t valid_samples = 0;
    uint32_t start_tick;

    result.Description = "Temperature Monitoring";
    start_tick = MEMS_Example_GetTick();

    printf("Starting Temperature Monitoring Example (%d samples)...\n", sample_count);

    if (!hmems->IsInitialized) {
        printf("ERROR: MEMS sensor not initialized\n");
        result.Success = false;
        return result;
    }

    printf("Sample\tTemperature (°C)\n");
    printf("------------------------\n");

    for (uint16_t i = 0; i < sample_count; i++) {
        status = MEMS_ReadTemperature(hmems, &temperature);
        if (status == MEMS_OK) {
            temp_sum += temperature;
            if (temperature > temp_max) {
                temp_max = temperature;
            }
            if (temperature < temp_min) {
                temp_min = temperature;
            }
            valid_samples++;

            printf("%d\t%.2f\n", i + 1, temperature);
        } else {
            printf("%d\tERROR\n", i + 1);
        }

        delay_ms(100);
    }

    if (valid_samples > 0) {
        result.AverageValue = temp_sum / valid_samples;
        result.MaxValue = temp_max;
        result.MinValue = temp_min;
        result.Success = true;

        printf("\nTemperature Statistics:\n");
        printf("Valid samples: %d/%d\n", valid_samples, sample_count);
        printf("Average: %.2f°C\n", result.AverageValue);
        printf("Maximum: %.2f°C\n", result.MaxValue);
        printf("Minimum: %.2f°C\n", result.MinValue);
        printf("Range: %.2f°C\n", result.MaxValue - result.MinValue);
    } else {
        result.Success = false;
        printf("ERROR: No valid temperature readings\n");
    }

    result.ExecutionTime = MEMS_Example_GetTick() - start_tick;
    printf("Temperature Monitoring Example completed\n\n");
    return result;
}

/**
 * @brief Motion detection example
 * @param hmems Pointer to MEMS handle structure
 * @param threshold Motion detection threshold in dps
 * @param duration_ms Monitoring duration in milliseconds
 * @retval MEMS_MotionResultTypeDef Motion detection result
 */
MEMS_MotionResultTypeDef MEMS_Example_MotionDetection(MEMS_HandleTypeDef *hmems, float threshold, uint32_t duration_ms)
{
    MEMS_MotionResultTypeDef motion_result = {0};
    MEMS_AxesTypeDef current_data, previous_data = {0};
    MEMS_StatusTypeDef status;
    uint32_t start_tick, current_tick;
    uint32_t motion_start_tick = 0;
    bool motion_active = false;
    uint32_t motion_count = 0;
    float max_intensity = 0.0f;

    printf("Starting Motion Detection Example...\n");
    printf("Threshold: %.2f dps, Duration: %lu ms\n", threshold, duration_ms);

    if (!hmems->IsInitialized) {
        printf("ERROR: MEMS sensor not initialized\n");
        return motion_result;
    }

    start_tick = MEMS_Example_GetTick();

    /* Get initial reading */
    status = MEMS_GyroRead(hmems, &previous_data);
    if (status != MEMS_OK) {
        printf("ERROR: Failed to read initial gyroscope data\n");
        return motion_result;
    }

    printf("Monitoring for motion...\n");
    printf("Time(ms)\tMotion\tIntensity(dps)\n");
    printf("----------------------------------\n");

    current_tick = start_tick;
    while ((current_tick - start_tick) < duration_ms) {
        status = MEMS_GyroRead(hmems, &current_data);
        if (status == MEMS_OK) {
            bool motion_detected = is_motion_detected(&current_data, &previous_data, threshold);
            float intensity = calculate_magnitude(&current_data);

            if (motion_detected) {
                if (!motion_active) {
                    motion_active = true;
                    motion_start_tick = current_tick;
                    printf("%lu\t\tSTART\t%.2f\n", current_tick - start_tick, intensity);
                }

                motion_count++;
                motion_result.MotionDetected = true;
                motion_result.MotionVector = current_data;

                if (intensity > max_intensity) {
                    max_intensity = intensity;
                }

                if ((motion_count % 10) == 0) {
                    printf("%lu\t\tACTIVE\t%.2f\n", current_tick - start_tick, intensity);
                }
            } else {
                if (motion_active) {
                    motion_active = false;
                    motion_result.MotionDuration += (current_tick - motion_start_tick);
                    printf("%lu\t\tEND\t%.2f\n", current_tick - start_tick, intensity);
                }
            }

            previous_data = current_data;
        }

        delay_ms(DATA_COLLECTION_DELAY);
        current_tick = MEMS_Example_GetTick();
    }

    /* Handle case where motion is still active at end */
    if (motion_active) {
        motion_result.MotionDuration += (current_tick - motion_start_tick);
    }

    motion_result.MotionIntensity = max_intensity;

    printf("\nMotion Detection Results:\n");
    printf("Motion detected: %s\n", motion_result.MotionDetected ? "Yes" : "No");
    printf("Maximum intensity: %.2f dps\n", motion_result.MotionIntensity);
    printf("Total motion duration: %lu ms\n", motion_result.MotionDuration);
    printf("Motion events: %lu\n", motion_count);

    if (motion_result.MotionDetected) {
        printf("Final motion vector:\n");
        MEMS_Example_PrintData(&motion_result.MotionVector, "Motion");
    }

    printf("Motion Detection Example completed\n\n");
    return motion_result;
}

/**
 * @brief Self-test example
 * @param hmems Pointer to MEMS handle structure
 * @retval MEMS_ExampleResultTypeDef Example execution result
 */
MEMS_ExampleResultTypeDef MEMS_Example_SelfTest(MEMS_HandleTypeDef *hmems)
{
    MEMS_ExampleResultTypeDef result = {0};
    MEMS_StatusTypeDef status;
    bool test_result = false;
    uint32_t start_tick;

    result.Description = "MEMS Self-Test";
    start_tick = MEMS_Example_GetTick();

    printf("Starting MEMS Self-Test Example...\n");

    if (!hmems->IsInitialized) {
        printf("ERROR: MEMS sensor not initialized\n");
        result.Success = false;
        return result;
    }

    printf("Performing self-test...\n");
    status = MEMS_SelfTest(hmems, &test_result);

    if (status == MEMS_OK) {
        printf("Self-test completed\n");
        printf("Result: %s\n", test_result ? "PASS" : "FAIL");
        result.Success = test_result;
        result.AverageValue = test_result ? 1.0f : 0.0f;
    } else {
        printf("ERROR: Self-test execution failed (Status: %d)\n", status);
        result.Success = false;
    }

    result.ExecutionTime = MEMS_Example_GetTick() - start_tick;
    printf("Self-Test Example completed\n\n");
    return result;
}

/**
 * @brief Power management example
 * @param hmems Pointer to MEMS handle structure
 * @retval MEMS_ExampleResultTypeDef Example execution result
 */
MEMS_ExampleResultTypeDef MEMS_Example_PowerManagement(MEMS_HandleTypeDef *hmems)
{
    MEMS_ExampleResultTypeDef result = {0};
    MEMS_StatusTypeDef status;
    MEMS_AxesTypeDef gyro_data;
    uint32_t start_tick;

    result.Description = "Power Management";
    start_tick = MEMS_Example_GetTick();

    printf("Starting Power Management Example...\n");

    if (!hmems->IsInitialized) {
        printf("ERROR: MEMS sensor not initialized\n");
        result.Success = false;
        return result;
    }

    /* Read data in normal mode */
    printf("Reading data in normal mode...\n");
    status = MEMS_GyroRead(hmems, &gyro_data);
    if (status == MEMS_OK) {
        MEMS_Example_PrintData(&gyro_data, "Normal Mode");
    }

    /* Enter power-down mode */
    printf("Entering power-down mode...\n");
    status = MEMS_SetPowerMode(hmems, true);
    if (status == MEMS_OK) {
        printf("Device in power-down mode\n");
        delay_ms(1000);

        /* Try to read data (should fail or return zeros) */
        status = MEMS_GyroRead(hmems, &gyro_data);
        if (status == MEMS_OK) {
            MEMS_Example_PrintData(&gyro_data, "Power-Down Mode");
        } else {
            printf("Data reading disabled in power-down mode\n");
        }
    }

    /* Exit power-down mode */
    printf("Exiting power-down mode...\n");
    status = MEMS_SetPowerMode(hmems, false);
    if (status == MEMS_OK) {
        printf("Device back to normal mode\n");
        delay_ms(100); /* Allow time to stabilize */

        /* Read data again */
        status = MEMS_GyroRead(hmems, &gyro_data);
        if (status == MEMS_OK) {
            MEMS_Example_PrintData(&gyro_data, "Normal Mode Restored");
            result.Success = true;
            result.AverageValue = calculate_magnitude(&gyro_data);
        }
    }

    result.ExecutionTime = MEMS_Example_GetTick() - start_tick;
    printf("Power Management Example completed\n\n");
    return result;
}

/**
 * @brief Performance test example
 * @param hmems Pointer to MEMS handle structure
 * @param test_iterations Number of test iterations
 * @retval MEMS_ExampleResultTypeDef Example execution result
 */
MEMS_ExampleResultTypeDef MEMS_Example_PerformanceTest(MEMS_HandleTypeDef *hmems, uint32_t test_iterations)
{
    MEMS_ExampleResultTypeDef result = {0};
    MEMS_StatusTypeDef status;
    MEMS_AxesTypeDef gyro_data;
    uint32_t start_tick, end_tick;
    uint32_t successful_reads = 0;
    uint32_t total_time;
    float reads_per_second;

    result.Description = "Performance Test";

    printf("Starting Performance Test Example (%lu iterations)...\n", test_iterations);

    if (!hmems->IsInitialized) {
        printf("ERROR: MEMS sensor not initialized\n");
        result.Success = false;
        return result;
    }

    printf("Performing %lu consecutive reads...\n", test_iterations);
    start_tick = MEMS_Example_GetTick();

    for (uint32_t i = 0; i < test_iterations; i++) {
        status = MEMS_GyroRead(hmems, &gyro_data);
        if (status == MEMS_OK) {
            successful_reads++;
        }

        /* Print progress every 100 iterations */
        if ((i % 100) == 0) {
            printf("Progress: %lu/%lu (%.1f%%)\n", i, test_iterations,
                   ((float)i / test_iterations) * 100.0f);
        }
    }

    end_tick = MEMS_Example_GetTick();
    total_time = end_tick - start_tick;

    if (total_time > 0) {
        reads_per_second = ((float)successful_reads * 1000.0f) / total_time;
    } else {
        reads_per_second = 0.0f;
    }

    printf("\nPerformance Test Results:\n");
    printf("Total iterations: %lu\n", test_iterations);
    printf("Successful reads: %lu\n", successful_reads);
    printf("Failed reads: %lu\n", test_iterations - successful_reads);
    printf("Total time: %lu ms\n", total_time);
    printf("Average time per read: %.2f ms\n", (float)total_time / test_iterations);
    printf("Reads per second: %.1f\n", reads_per_second);
    printf("Success rate: %.2f%%\n", ((float)successful_reads / test_iterations) * 100.0f);

    result.Success = (successful_reads > (test_iterations * 0.95f)); /* 95% success rate */
    result.AverageValue = reads_per_second;
    result.MaxValue = reads_per_second;
    result.MinValue = reads_per_second;
    result.ExecutionTime = total_time;

    printf("Performance Test Example completed\n\n");
    return result;
}

/* Helper functions ----------------------------------------------------------*/

/**
 * @brief Print MEMS data to console
 * @param axes Pointer to axes data structure
 * @param label Data label string
 */
void MEMS_Example_PrintData(const MEMS_AxesTypeDef *axes, const char *label)
{
    if (axes != NULL && label != NULL) {
        printf("%s: X=%.3f, Y=%.3f, Z=%.3f dps\n",
               label, axes->X, axes->Y, axes->Z);
    }
}

/**
 * @brief Print example result
 * @param result Pointer to example result structure
 */
void MEMS_Example_PrintResult(const MEMS_ExampleResultTypeDef *result)
{
    if (result != NULL) {
        printf("\n=== Example Result ===\n");
        printf("Description: %s\n", result->Description ? result->Description : "Unknown");
        printf("Success: %s\n", result->Success ? "Yes" : "No");
        printf("Execution Time: %lu ms\n", result->ExecutionTime);
        printf("Average Value: %.3f\n", result->AverageValue);
        printf("Max Value: %.3f\n", result->MaxValue);
        printf("Min Value: %.3f\n", result->MinValue);
        printf("======================\n\n");
    }
}

/**
 * @brief Get system tick count
 * @retval uint32_t Current tick count in milliseconds
 */
uint32_t MEMS_Example_GetTick(void)
{
    return HAL_GetTick();
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Simple delay function
 * @param ms Delay in milliseconds
 */
static void delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

/**
 * @brief Calculate magnitude of 3D vector
 * @param axes Pointer to axes data structure
 * @retval float Magnitude value
 */
static float calculate_magnitude(const MEMS_AxesTypeDef *axes)
{
    if (axes == NULL) {
        return 0.0f;
    }

    return sqrtf((axes->X * axes->X) + (axes->Y * axes->Y) + (axes->Z * axes->Z));
}

/**
 * @brief Check if motion is detected based on threshold
 * @param current Current axes data
 * @param previous Previous axes data
 * @param threshold Motion detection threshold
 * @retval bool True if motion detected
 */
static bool is_motion_detected(const MEMS_AxesTypeDef *current,
                              const MEMS_AxesTypeDef *previous,
                              float threshold)
{
    if (current == NULL || previous == NULL) {
        return false;
    }

    float diff_x = fabsf(current->X - previous->X);
    float diff_y = fabsf(current->Y - previous->Y);
    float diff_z = fabsf(current->Z - previous->Z);

    return (diff_x > threshold) || (diff_y > threshold) || (diff_z > threshold);
}
