/**
  ******************************************************************************
  * @file    audio_example.c
  * @brief   Audio subsystem example implementation
  * @details This file contains example functions demonstrating how to use
  *          the audio driver for playback, recording, and processing.
  * @version 1.0
  * @date    2025-09-01
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "audio.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* Private defines -----------------------------------------------------------*/

/** @defgroup AUDIO_EXAMPLE_Private_Defines Private Defines
 * @{
 */

#define AUDIO_EXAMPLE_BUFFER_SIZE     4096U
#define AUDIO_EXAMPLE_SAMPLE_RATE     AUDIO_FREQ_44K
#define AUDIO_EXAMPLE_VOLUME_DEFAULT  75U

/** @} */

/* Private variables ---------------------------------------------------------*/

/** @defgroup AUDIO_EXAMPLE_Private_Variables Private Variables
 * @{
 */

static uint8_t AudioBuffer[AUDIO_EXAMPLE_BUFFER_SIZE];
static bool AudioInitialized = false;

/** @} */

/* Private function prototypes -----------------------------------------------*/

/** @defgroup AUDIO_EXAMPLE_Private_Functions Private Functions
 * @{
 */

static void AUDIO_Example_GenerateSineWave(uint8_t* buffer, uint32_t size,
                                         uint32_t frequency, uint32_t sampleRate);
static void AUDIO_Example_ProcessAudioData(uint8_t* input, uint8_t* output,
                                         uint32_t size);

/** @} */

/* Exported functions ---------------------------------------------------------*/

/** @defgroup AUDIO_EXAMPLE_Exported_Functions Exported Functions
 * @{
 */

/**
 * @brief   Audio initialization example
 * @details Demonstrates how to initialize the audio subsystem
 * @param   None
 * @retval  None
 */
void AUDIO_Example_Init(void)
{
    AUDIO_StatusTypeDef status = AUDIO_OK;
    AUDIO_ConfigTypeDef audioConfig = {
        .Interface = AUDIO_INTERFACE_SAI,
        .SampleRate = AUDIO_EXAMPLE_SAMPLE_RATE,
        .BitDepth = AUDIO_FORMAT_16BIT,
        .Channels = AUDIO_CHANNEL_STEREO,
        .BufferSize = AUDIO_EXAMPLE_BUFFER_SIZE,
        .EnableDMA = true,
        .EnableInterrupts = true
    };

    printf("Initializing Audio System...\n");

    /* Initialize audio with custom configuration */
    status = AUDIO_Init_Custom(&audioConfig);
    if (status != AUDIO_OK) {
        printf("Audio initialization failed: %s\n", AUDIO_GetStatusString(status));
        return;
    }

    /* Set default volume */
    status = AUDIO_SetVolume(AUDIO_EXAMPLE_VOLUME_DEFAULT);
    if (status != AUDIO_OK) {
        printf("Failed to set volume: %s\n", AUDIO_GetStatusString(status));
    }

    AudioInitialized = true;
    printf("Audio system initialized successfully\n");
}

/**
 * @brief   Audio playback example
 * @details Demonstrates audio playback with generated sine wave
 * @param   None
 * @retval  None
 */
void AUDIO_Example_Playback(void)
{
    AUDIO_StatusTypeDef status = AUDIO_OK;

    if (!AudioInitialized) {
        printf("Audio not initialized. Call AUDIO_Example_Init() first.\n");
        return;
    }

    printf("Starting audio playback...\n");

    /* Generate a 440Hz sine wave (A4 note) */
    AUDIO_Example_GenerateSineWave(AudioBuffer, AUDIO_EXAMPLE_BUFFER_SIZE, 440, 44100);

    /* Write audio data to buffer */
    status = AUDIO_WriteBuffer(AudioBuffer, AUDIO_EXAMPLE_BUFFER_SIZE);
    if (status != AUDIO_OK) {
        printf("Failed to write audio buffer: %s\n", AUDIO_GetStatusString(status));
        return;
    }

    /* Start playback */
    status = AUDIO_Play();
    if (status != AUDIO_OK) {
        printf("Failed to start playback: %s\n", AUDIO_GetStatusString(status));
        return;
    }

    printf("Audio playback started. Playing 440Hz sine wave...\n");

    /* Let it play for a few seconds */
    HAL_Delay(3000);

    /* Stop playback */
    status = AUDIO_Stop();
    if (status != AUDIO_OK) {
        printf("Failed to stop playback: %s\n", AUDIO_GetStatusString(status));
    } else {
        printf("Audio playback stopped\n");
    }
}

/**
 * @brief   Audio recording example
 * @details Demonstrates audio recording and processing
 * @param   None
 * @retval  None
 */
void AUDIO_Example_Recording(void)
{
    AUDIO_StatusTypeDef status = AUDIO_OK;
    uint32_t bytesRead = 0;
    uint8_t processedBuffer[AUDIO_EXAMPLE_BUFFER_SIZE];

    if (!AudioInitialized) {
        printf("Audio not initialized. Call AUDIO_Example_Init() first.\n");
        return;
    }

    printf("Starting audio recording...\n");

    /* Start recording */
    status = AUDIO_Record();
    if (status != AUDIO_OK) {
        printf("Failed to start recording: %s\n", AUDIO_GetStatusString(status));
        return;
    }

    printf("Recording for 2 seconds...\n");
    HAL_Delay(2000);

    /* Stop recording */
    status = AUDIO_Stop();
    if (status != AUDIO_OK) {
        printf("Failed to stop recording: %s\n", AUDIO_GetStatusString(status));
        return;
    }

    /* Read recorded data */
    status = AUDIO_ReadBuffer(AudioBuffer, AUDIO_EXAMPLE_BUFFER_SIZE, &bytesRead);
    if (status != AUDIO_OK) {
        printf("Failed to read audio buffer: %s\n", AUDIO_GetStatusString(status));
        return;
    }

    printf("Recorded %u bytes of audio data\n", bytesRead);

    /* Process the recorded audio (simple example) */
    AUDIO_Example_ProcessAudioData(AudioBuffer, processedBuffer, bytesRead);

    /* Play back the processed audio */
    status = AUDIO_WriteBuffer(processedBuffer, bytesRead);
    if (status != AUDIO_OK) {
        printf("Failed to write processed audio: %s\n", AUDIO_GetStatusString(status));
        return;
    }

    status = AUDIO_Play();
    if (status != AUDIO_OK) {
        printf("Failed to play processed audio: %s\n", AUDIO_GetStatusString(status));
    } else {
        printf("Playing processed audio...\n");
        HAL_Delay(2000);
        AUDIO_Stop();
    }
}

/**
 * @brief   Audio volume control example
 * @details Demonstrates volume control functionality
 * @param   None
 * @retval  None
 */
void AUDIO_Example_VolumeControl(void)
{
    AUDIO_StatusTypeDef status = AUDIO_OK;
    uint8_t currentVolume = 0;

    if (!AudioInitialized) {
        printf("Audio not initialized. Call AUDIO_Example_Init() first.\n");
        return;
    }

    printf("Audio Volume Control Example:\n");

    /* Generate and start playing audio */
    AUDIO_Example_GenerateSineWave(AudioBuffer, AUDIO_EXAMPLE_BUFFER_SIZE, 440, 44100);
    AUDIO_WriteBuffer(AudioBuffer, AUDIO_EXAMPLE_BUFFER_SIZE);
    AUDIO_Play();

    /* Test volume levels */
    for (uint8_t volume = 0; volume <= 100; volume += 25) {
        status = AUDIO_SetVolume(volume);
        if (status != AUDIO_OK) {
            printf("Failed to set volume to %d: %s\n", volume, AUDIO_GetStatusString(status));
            continue;
        }

        status = AUDIO_GetVolume(&currentVolume);
        if (status == AUDIO_OK) {
            printf("Volume set to: %d%%\n", currentVolume);
        }

        HAL_Delay(500);
    }

    /* Test mute functionality */
    printf("Testing mute functionality...\n");
    AUDIO_SetMute(true);
    printf("Audio muted\n");
    HAL_Delay(1000);

    AUDIO_SetMute(false);
    printf("Audio unmuted\n");
    HAL_Delay(1000);

    AUDIO_Stop();
    printf("Volume control example completed\n");
}

/**
 * @brief   Audio mixing example
 * @details Demonstrates audio stream mixing
 * @param   None
 * @retval  None
 */
void AUDIO_Example_AudioMixing(void)
{
    AUDIO_StatusTypeDef status;
    int16_t sineWave1[AUDIO_EXAMPLE_BUFFER_SIZE / 2];
    int16_t sineWave2[AUDIO_EXAMPLE_BUFFER_SIZE / 2];
    int16_t mixedWave[AUDIO_EXAMPLE_BUFFER_SIZE / 2];

    if (!AudioInitialized) {
        printf("Audio not initialized. Call AUDIO_Example_Init() first.\n");
        return;
    }

    printf("Audio Mixing Example:\n");

    /* Generate two different sine waves */
    for (uint32_t i = 0; i < AUDIO_EXAMPLE_BUFFER_SIZE / 4; i++) {
        float t = (float)i / 44100.0f;
        sineWave1[i] = (int16_t)(16000.0f * sinf(2.0f * 3.14159f * 440.0f * t));  /* 440Hz */
        sineWave2[i] = (int16_t)(16000.0f * sinf(2.0f * 3.14159f * 660.0f * t));  /* 660Hz */
    }

    /* Mix the waves */
    status = AUDIO_MixStreams(sineWave1, sineWave2, mixedWave, AUDIO_EXAMPLE_BUFFER_SIZE / 4);
    if (status != AUDIO_OK) {
        printf("Failed to mix audio streams: %s\n", AUDIO_GetStatusString(status));
        return;
    }

    /* Play the mixed audio */
    status = AUDIO_WriteBuffer((uint8_t*)mixedWave, AUDIO_EXAMPLE_BUFFER_SIZE / 2);
    if (status != AUDIO_OK) {
        printf("Failed to write mixed audio: %s\n", AUDIO_GetStatusString(status));
        return;
    }

    status = AUDIO_Play();
    if (status != AUDIO_OK) {
        printf("Failed to play mixed audio: %s\n", AUDIO_GetStatusString(status));
    } else {
        printf("Playing mixed 440Hz + 660Hz sine waves...\n");
        HAL_Delay(3000);
        AUDIO_Stop();
    }

    printf("Audio mixing example completed\n");
}

/**
 * @brief   Audio statistics example
 * @details Demonstrates how to monitor audio system statistics
 * @param   None
 * @retval  None
 */
void AUDIO_Example_Statistics(void)
{
    AUDIO_StatusTypeDef status;
    AUDIO_StatsTypeDef stats;

    if (!AudioInitialized) {
        printf("Audio not initialized. Call AUDIO_Example_Init() first.\n");
        return;
    }

    printf("Audio Statistics Example:\n");

    /* Generate and play some audio to create statistics */
    AUDIO_Example_GenerateSineWave(AudioBuffer, AUDIO_EXAMPLE_BUFFER_SIZE, 440, 44100);
    AUDIO_WriteBuffer(AudioBuffer, AUDIO_EXAMPLE_BUFFER_SIZE);
    AUDIO_Play();
    HAL_Delay(1000);
    AUDIO_Stop();

    /* Get statistics */
    status = AUDIO_GetStatistics(&stats);
    if (status != AUDIO_OK) {
        printf("Failed to get statistics: %s\n", AUDIO_GetStatusString(status));
        return;
    }

    /* Display statistics */
    printf("Audio Statistics:\n");
    printf("  Samples processed: %u\n", stats.SampleCount);
    printf("  Buffer overflows: %u\n", stats.BufferOverflows);
    printf("  Buffer underflows: %u\n", stats.BufferUnderflows);
    printf("  Sync errors: %u\n", stats.SyncErrors);
    printf("  CPU usage: %.2f%%\n", stats.CPUUsage);

    /* Reset statistics */
    status = AUDIO_ResetStatistics();
    if (status == AUDIO_OK) {
        printf("Statistics reset\n");
    }

    printf("Statistics example completed\n");
}

/**
 * @brief   Complete audio demonstration
 * @details Runs all audio examples in sequence
 * @param   None
 * @retval  None
 */
void AUDIO_Example_RunAll(void)
{
    printf("=== Audio System Complete Demonstration ===\n\n");

    /* Initialize */
    AUDIO_Example_Init();
    HAL_Delay(1000);

    /* Run examples */
    AUDIO_Example_Playback();
    HAL_Delay(1000);

    AUDIO_Example_VolumeControl();
    HAL_Delay(1000);

    AUDIO_Example_AudioMixing();
    HAL_Delay(1000);

    AUDIO_Example_Statistics();
    HAL_Delay(1000);

    /* Recording example (commented out as it requires microphone input) */
    // AUDIO_Example_Recording();

    /* Cleanup */
    AUDIO_DeInit();
    printf("\n=== Audio demonstration completed ===\n");
}

/* Private functions ---------------------------------------------------------*/

/** @defgroup AUDIO_EXAMPLE_Private_Functions Private Functions Implementation
 * @{
 */

/**
 * @brief   Generate sine wave audio data
 * @details Creates a sine wave at specified frequency and sample rate
 * @param   buffer Buffer to store audio data
 * @param   size Size of buffer in bytes
 * @param   frequency Frequency of sine wave in Hz
 * @param   sampleRate Sample rate in Hz
 * @retval  None
 */
static void AUDIO_Example_GenerateSineWave(uint8_t* buffer, uint32_t size,
                                         uint32_t frequency, uint32_t sampleRate)
{
    uint32_t numSamples = size / 2;  /* 16-bit samples */
    int16_t* samples = (int16_t*)buffer;

    for (uint32_t i = 0; i < numSamples; i++) {
        float t = (float)i / (float)sampleRate;
        float angle = 2.0f * 3.14159f * (float)frequency * t;
        samples[i] = (int16_t)(16000.0f * sinf(angle));  /* 16-bit amplitude */
    }
}

/**
 * @brief   Process audio data
 * @details Simple audio processing example (amplification)
 * @param   input Input audio buffer
 * @param   output Output audio buffer
 * @param   size Size of data in bytes
 * @retval  None
 */
static void AUDIO_Example_ProcessAudioData(uint8_t* input, uint8_t* output,
                                         uint32_t size)
{
    uint32_t numSamples = size / 2;  /* 16-bit samples */
    int16_t* inSamples = (int16_t*)input;
    int16_t* outSamples = (int16_t*)output;

    /* Simple amplification by 1.5x */
    for (uint32_t i = 0; i < numSamples; i++) {
        int32_t amplified = (int32_t)inSamples[i] * 3 / 2;  /* 1.5x amplification */

        /* Clip to 16-bit range */
        if (amplified > 32767) amplified = 32767;
        if (amplified < -32768) amplified = -32768;

        outSamples[i] = (int16_t)amplified;
    }
}

/** @} */

/** @} */
