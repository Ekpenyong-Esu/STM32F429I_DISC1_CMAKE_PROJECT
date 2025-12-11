/**
  ******************************************************************************
  * @file    mic_example.c
  * @brief   Microphone driver example implementation for STM32F429 Discovery Board
  * @details This file contains example functions demonstrating the use of the
  *          MP45DT02 digital MEMS microphone driver.
  * @version 1.0
  * @date    2025-09-04
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "mic_example.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* Private constants ---------------------------------------------------------*/
#define EXAMPLE_DELAY_MS(x)             HAL_Delay(x)
#define EXAMPLE_PI                      3.14159265359f
#define EXAMPLE_LOG10                   2.302585093f
#define EXAMPLE_VU_ATTACK_TIME          0.001f  /* 1ms */
#define EXAMPLE_VU_RELEASE_TIME         0.1f    /* 100ms */
#define EXAMPLE_COMPRESSOR_ATTACK       0.003f  /* 3ms */
#define EXAMPLE_COMPRESSOR_RELEASE      0.1f    /* 100ms */
#define EXAMPLE_VAD_ENERGY_THRESHOLD    0.01f
#define EXAMPLE_VAD_ZCR_THRESHOLD       0.3f
#define EXAMPLE_SPECTRAL_SMOOTHING      0.8f

/* Private variables ---------------------------------------------------------*/
static I2S_HandleTypeDef hi2s3;
static DMA_HandleTypeDef hdma_spi3_rx;
static MIC_HandleTypeDef hmic;
static MIC_ExampleData_t example_data;

/* Private function prototypes -----------------------------------------------*/
static void MIC_Example_InitHardware(void);
static float MIC_Example_LinearToDecibel(float linear);
static float MIC_Example_DecibelToLinear(float decibel);
static void MIC_Example_Hamming(float *window, uint32_t length);
static void MIC_Example_SimpleFFT(int16_t *input, float *magnitude, uint32_t length);
static float MIC_Example_CalculateZCR(int16_t *buffer, uint32_t length);
static float MIC_Example_CalculateEnergy(int16_t *buffer, uint32_t length);

/* Exported functions --------------------------------------------------------*/

/**
 * @brief Basic microphone recording example
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_Example_BasicRecording(void)
{
    MIC_StatusTypeDef status;
    int16_t audio_buffer[512];
    uint32_t samples_recorded = 0;

    printf("Starting basic microphone recording example...\n");

    /* Initialize hardware */
    MIC_Example_InitHardware();

    /* Initialize microphone */
    status = MIC_Init(&hmic, &hi2s3, &hdma_spi3_rx);
    if (status != MIC_OK) {
        printf("Error initializing microphone: %d\n", status);
        return status;
    }

    /* Start recording */
    status = MIC_StartRecording(&hmic);
    if (status != MIC_OK) {
        printf("Error starting recording: %d\n", status);
        return status;
    }

    printf("Recording for 5 seconds...\n");

    /* Record for 5 seconds */
    uint32_t start_time = HAL_GetTick();
    while ((HAL_GetTick() - start_time) < 5000) {
        /* Check if data is ready */
        if (hmic.BufferReady) {
            status = MIC_ReadAudioData(&hmic, audio_buffer, 512);
            if (status == MIC_OK) {
                samples_recorded += 512;

                /* Calculate and display RMS level */
                float rms = MIC_CalculateRMS(audio_buffer, 512);
                float db = MIC_Example_LinearToDecibel(rms);
                printf("RMS Level: %.1f dB\n", db);
            }
        }
        EXAMPLE_DELAY_MS(10);
    }

    /* Stop recording */
    MIC_StopRecording(&hmic);

    printf("Recording complete. Samples recorded: %lu\n", samples_recorded);

    return MIC_OK;
}

/**
 * @brief Volume control example
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_Example_VolumeControl(void)
{
    MIC_StatusTypeDef status;
    int16_t audio_buffer[256];
    uint8_t volume;

    printf("Volume control example...\n");

    /* Initialize hardware and microphone */
    MIC_Example_InitHardware();
    status = MIC_Init(&hmic, &hi2s3, &hdma_spi3_rx);
    if (status != MIC_OK) {
        return status;
    }

    /* Start recording */
    MIC_StartRecording(&hmic);

    /* Test different volume levels */
    for (volume = 10; volume <= 100; volume += 20) {
        printf("Setting volume to: %d%%\n", volume);
        MIC_SetVolume(&hmic, volume);

        EXAMPLE_DELAY_MS(1000);

        /* Read and analyze audio */
        if (hmic.BufferReady) {
            MIC_ReadAudioData(&hmic, audio_buffer, 256);
            float rms = MIC_CalculateRMS(audio_buffer, 256);
            printf("RMS at %d%% volume: %.3f\n", volume, rms);
        }
    }

    MIC_StopRecording(&hmic);
    return MIC_OK;
}

/**
 * @brief Gain control example
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_Example_GainControl(void)
{
    MIC_StatusTypeDef status;
    int16_t audio_buffer[256];
    MIC_GainTypeDef gains[] = {MIC_GAIN_0DB, MIC_GAIN_6DB, MIC_GAIN_12DB, MIC_GAIN_18DB, MIC_GAIN_24DB};
    const char* gain_names[] = {"0dB", "6dB", "12dB", "18dB", "24dB"};

    printf("Gain control example...\n");

    /* Initialize hardware and microphone */
    MIC_Example_InitHardware();
    status = MIC_Init(&hmic, &hi2s3, &hdma_spi3_rx);
    if (status != MIC_OK) {
        return status;
    }

    MIC_StartRecording(&hmic);

    /* Test different gain levels */
    for (uint32_t i = 0; i < 5; i++) {
        printf("Setting gain to: %s\n", gain_names[i]);
        MIC_SetGain(&hmic, gains[i]);

        EXAMPLE_DELAY_MS(1000);

        /* Read and analyze audio */
        if (hmic.BufferReady) {
            MIC_ReadAudioData(&hmic, audio_buffer, 256);
            float peak = MIC_CalculatePeak(audio_buffer, 256);
            printf("Peak at %s gain: %.3f\n", gain_names[i], peak);
        }
    }

    MIC_StopRecording(&hmic);
    return MIC_OK;
}

/**
 * @brief Audio levels monitoring example
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_Example_AudioLevels(void)
{
    MIC_StatusTypeDef status;
    int16_t audio_buffer[512];
    float rms_level, peak_level;
    MIC_StatisticsTypeDef stats;

    printf("Audio levels monitoring example...\n");

    /* Initialize hardware and microphone */
    MIC_Example_InitHardware();
    status = MIC_Init(&hmic, &hi2s3, &hdma_spi3_rx);
    if (status != MIC_OK) {
        return status;
    }

    MIC_StartRecording(&hmic);

    /* Monitor levels for 10 seconds */
    uint32_t start_time = HAL_GetTick();
    while ((HAL_GetTick() - start_time) < 10000) {
        if (hmic.BufferReady) {
            MIC_ReadAudioData(&hmic, audio_buffer, 512);

            /* Get audio levels */
            MIC_GetAudioLevel(&hmic, &rms_level, &peak_level);

            /* Get statistics */
            MIC_GetStatistics(&hmic, &stats);

            /* Display levels */
            printf("RMS: %.1f dB, Peak: %.1f dB, SNR: %.1f dB\n",
                   MIC_Example_LinearToDecibel(rms_level),
                   MIC_Example_LinearToDecibel(peak_level),
                   stats.SNR);

            /* Categorize level */
            MIC_LevelCategory_t category = MIC_Example_GetLevelCategory(MIC_Example_LinearToDecibel(rms_level));
            const char* category_names[] = {"SILENCE", "QUIET", "NORMAL", "LOUD", "VERY_LOUD"};
            printf("Level Category: %s\n", category_names[category]);
        }

        EXAMPLE_DELAY_MS(500);
    }

    MIC_StopRecording(&hmic);
    return MIC_OK;
}

/**
 * @brief Spectral analysis example
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_Example_SpectralAnalysis(void)
{
    MIC_StatusTypeDef status;
    int16_t audio_buffer[MIC_EXAMPLE_FFT_SIZE];
    MIC_SpectralAnalysis_t spectral;

    printf("Spectral analysis example...\n");

    /* Initialize hardware and microphone */
    MIC_Example_InitHardware();
    status = MIC_Init(&hmic, &hi2s3, &hdma_spi3_rx);
    if (status != MIC_OK) {
        return status;
    }

    MIC_StartRecording(&hmic);

    /* Perform spectral analysis for 10 seconds */
    uint32_t start_time = HAL_GetTick();
    while ((HAL_GetTick() - start_time) < 10000) {
        if (hmic.BufferReady) {
            MIC_ReadAudioData(&hmic, audio_buffer, MIC_EXAMPLE_FFT_SIZE);

            /* Perform spectral analysis */
            status = MIC_Example_PerformSpectralAnalysis(audio_buffer, MIC_EXAMPLE_FFT_SIZE, &spectral);
            if (status == MIC_OK) {
                printf("Spectral Centroid: %.1f Hz\n", spectral.Centroid);
                printf("Brightness: %.3f\n", spectral.Brightness);
                printf("Spectral Rolloff: %.1f Hz\n", spectral.Rolloff);

                /* Display frequency bands */
                printf("Frequency Bands: ");
                for (uint32_t i = 0; i < MIC_EXAMPLE_SPECTRAL_BANDS; i++) {
                    printf("%.2f ", spectral.Bands[i]);
                }
                printf("\n");
            }
        }

        EXAMPLE_DELAY_MS(1000);
    }

    MIC_StopRecording(&hmic);
    return MIC_OK;
}

/**
 * @brief Voice activity detection example
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_Example_VoiceActivityDetection(void)
{
    MIC_StatusTypeDef status;
    int16_t audio_buffer[MIC_EXAMPLE_VAD_FRAME_SIZE];
    MIC_VoiceActivity_t vad = {0};

    printf("Voice activity detection example...\n");
    printf("Speak into the microphone...\n");

    /* Initialize hardware and microphone */
    MIC_Example_InitHardware();
    status = MIC_Init(&hmic, &hi2s3, &hdma_spi3_rx);
    if (status != MIC_OK) {
        return status;
    }

    MIC_StartRecording(&hmic);

    /* Monitor for voice activity */
    uint32_t start_time = HAL_GetTick();
    uint32_t voice_start = 0;
    bool was_voice_active = false;

    while ((HAL_GetTick() - start_time) < 30000) { /* 30 seconds */
        if (hmic.BufferReady) {
            MIC_ReadAudioData(&hmic, audio_buffer, MIC_EXAMPLE_VAD_FRAME_SIZE);

            /* Detect voice activity */
            status = MIC_Example_DetectVoiceActivity(audio_buffer, MIC_EXAMPLE_VAD_FRAME_SIZE, &vad);
            if (status == MIC_OK) {
                if (vad.IsVoiceActive && !was_voice_active) {
                    voice_start = HAL_GetTick();
                    printf("Voice detected! (Confidence: %.2f)\n", vad.Confidence);
                } else if (!vad.IsVoiceActive && was_voice_active) {
                    uint32_t voice_duration = HAL_GetTick() - voice_start;
                    printf("Voice ended. Duration: %lu ms\n", voice_duration);
                }

                was_voice_active = vad.IsVoiceActive;

                if (vad.IsVoiceActive) {
                    printf("Voice frames: %lu, Confidence: %.2f\n", vad.VoiceFrames, vad.Confidence);
                }
            }
        }

        EXAMPLE_DELAY_MS(10); /* 10ms frame rate */
    }

    MIC_StopRecording(&hmic);
    printf("Voice Activity Summary:\n");
    printf("Total voice frames: %lu\n", vad.VoiceFrames);
    printf("Total silence frames: %lu\n", vad.SilenceFrames);

    return MIC_OK;
}

/**
 * @brief Noise gate example
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_Example_NoiseGate(void)
{
    MIC_StatusTypeDef status;
    int16_t audio_buffer[256];
    float noise_gate_thresholds[] = {-60.0f, -40.0f, -20.0f};

    printf("Noise gate example...\n");

    /* Initialize hardware and microphone */
    MIC_Example_InitHardware();
    status = MIC_Init(&hmic, &hi2s3, &hdma_spi3_rx);
    if (status != MIC_OK) {
        return status;
    }

    /* Test different noise gate thresholds */
    for (uint32_t i = 0; i < 3; i++) {
        printf("Testing noise gate at %.1f dB threshold...\n", noise_gate_thresholds[i]);

        /* Configure noise gate */
        MIC_ConfigureNoiseGate(&hmic, true, noise_gate_thresholds[i]);

        MIC_StartRecording(&hmic);

        /* Monitor for 5 seconds */
        uint32_t start_time = HAL_GetTick();
        uint32_t gated_samples = 0;
        uint32_t total_samples = 0;

        while ((HAL_GetTick() - start_time) < 5000) {
            if (hmic.BufferReady) {
                MIC_ReadAudioData(&hmic, audio_buffer, 256);

                /* Count gated samples (silent samples) */
                for (uint32_t j = 0; j < 256; j++) {
                    if (audio_buffer[j] == 0) {
                        gated_samples++;
                    }
                    total_samples++;
                }

                float rms = MIC_CalculateRMS(audio_buffer, 256);
                printf("RMS: %.1f dB\n", MIC_Example_LinearToDecibel(rms));
            }

            EXAMPLE_DELAY_MS(100);
        }

        MIC_StopRecording(&hmic);

        float gate_percentage = (float)gated_samples / total_samples * 100.0f;
        printf("Gated samples: %.1f%%\n\n", gate_percentage);
    }

    return MIC_OK;
}

/**
 * @brief Real-time VU meter example
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_Example_VUMeter(void)
{
    MIC_StatusTypeDef status;
    int16_t audio_buffer[128];
    MIC_VUMeter_t vu_meter = {0};

    printf("Real-time VU meter example...\n");
    printf("Speak into the microphone to see the VU meter...\n");

    /* Initialize hardware and microphone */
    MIC_Example_InitHardware();
    status = MIC_Init(&hmic, &hi2s3, &hdma_spi3_rx);
    if (status != MIC_OK) {
        return status;
    }

    MIC_StartRecording(&hmic);

    /* Display VU meter for 15 seconds */
    uint32_t start_time = HAL_GetTick();
    while ((HAL_GetTick() - start_time) < 15000) {
        if (hmic.BufferReady) {
            MIC_ReadAudioData(&hmic, audio_buffer, 128);

            /* Calculate level */
            float rms = MIC_CalculateRMS(audio_buffer, 128);

            /* Update VU meter */
            MIC_Example_UpdateVUMeter(&vu_meter, rms);

            /* Display VU meter */
            MIC_Example_DisplayVUMeter(&vu_meter);
        }

        EXAMPLE_DELAY_MS(50); /* 20 fps update rate */
    }

    MIC_StopRecording(&hmic);
    return MIC_OK;
}

/**
 * @brief Initialize example data structure
 * @param example_data Pointer to example data structure
 * @param hmic Pointer to microphone handle
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_Example_Init(MIC_ExampleData_t *example_data, MIC_HandleTypeDef *hmic)
{
    if (example_data == NULL || hmic == NULL) {
        return MIC_INVALID_PARAM;
    }

    /* Initialize structure */
    memset(example_data, 0, sizeof(MIC_ExampleData_t));
    example_data->hmic = hmic;

    /* Set default configuration */
    example_data->Config = MIC_Example_GetDefaultConfig();

    /* Initialize compressor */
    example_data->Compressor.Threshold = MIC_EXAMPLE_COMPRESSOR_THRESHOLD;
    example_data->Compressor.Ratio = MIC_EXAMPLE_COMPRESSOR_RATIO;
    example_data->Compressor.Attack = EXAMPLE_COMPRESSOR_ATTACK;
    example_data->Compressor.Release = EXAMPLE_COMPRESSOR_RELEASE;

    return MIC_OK;
}

/**
 * @brief Get level category from dB value
 * @param level_db Level in decibels
 * @retval MIC_LevelCategory_t Level category
 */
MIC_LevelCategory_t MIC_Example_GetLevelCategory(float level_db)
{
    if (level_db < -40.0f) {
        return MIC_LEVEL_SILENCE;
    } else if (level_db < -20.0f) {
        return MIC_LEVEL_QUIET;
    } else if (level_db < -6.0f) {
        return MIC_LEVEL_NORMAL;
    } else if (level_db < 0.0f) {
        return MIC_LEVEL_LOUD;
    } else {
        return MIC_LEVEL_VERY_LOUD;
    }
}

/**
 * @brief Update VU meter with new level
 * @param vu_meter Pointer to VU meter structure
 * @param level Linear level value
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_Example_UpdateVUMeter(MIC_VUMeter_t *vu_meter, float level)
{
    if (vu_meter == NULL) {
        return MIC_INVALID_PARAM;
    }

    /* Convert to dB */
    float level_db = MIC_Example_LinearToDecibel(level);

    /* Apply smoothing */
    float attack = (level > vu_meter->Smoothed) ? EXAMPLE_VU_ATTACK_TIME : EXAMPLE_VU_RELEASE_TIME;
    vu_meter->Smoothed = vu_meter->Smoothed + attack * (level - vu_meter->Smoothed);

    /* Update peak hold */
    if (level > vu_meter->PeakHold) {
        vu_meter->PeakHold = level;
        vu_meter->PeakHoldTime = HAL_GetTick();
    } else if ((HAL_GetTick() - vu_meter->PeakHoldTime) > 1000) { /* 1 second hold */
        vu_meter->PeakHold *= 0.95f; /* Slow decay */
    }

    /* Update level bars */
    float range = MIC_EXAMPLE_VU_MAX_DB - MIC_EXAMPLE_VU_MIN_DB;
    for (uint32_t i = 0; i < MIC_EXAMPLE_VU_LEVELS; i++) {
        float bar_threshold = MIC_EXAMPLE_VU_MIN_DB + (i * range / MIC_EXAMPLE_VU_LEVELS);
        vu_meter->LevelsBars[i] = (level_db > bar_threshold) ? 1.0f : 0.0f;
    }

    return MIC_OK;
}

/**
 * @brief Perform spectral analysis on audio buffer
 * @param buffer Audio buffer
 * @param length Buffer length
 * @param spectral Pointer to spectral analysis structure
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_Example_PerformSpectralAnalysis(int16_t *buffer, uint32_t length, MIC_SpectralAnalysis_t *spectral)
{
    if (buffer == NULL || spectral == NULL || length == 0) {
        return MIC_INVALID_PARAM;
    }

    float magnitude[MIC_EXAMPLE_FFT_SIZE / 2];

    /* Perform FFT */
    MIC_Example_SimpleFFT(buffer, magnitude, length);

    /* Calculate frequency bands */
    uint32_t bins_per_band = (MIC_EXAMPLE_FFT_SIZE / 2) / MIC_EXAMPLE_SPECTRAL_BANDS;
    for (uint32_t i = 0; i < MIC_EXAMPLE_SPECTRAL_BANDS; i++) {
        spectral->Bands[i] = 0.0f;
        for (uint32_t j = 0; j < bins_per_band; j++) {
            uint32_t bin = i * bins_per_band + j;
            if (bin < (MIC_EXAMPLE_FFT_SIZE / 2)) {
                spectral->Bands[i] += magnitude[bin];
            }
        }
        spectral->Bands[i] /= bins_per_band;
    }

    /* Calculate spectral centroid */
    float weighted_sum = 0.0f;
    float magnitude_sum = 0.0f;
    for (uint32_t i = 0; i < (MIC_EXAMPLE_FFT_SIZE / 2); i++) {
        float frequency = (float)i * MIC_EXAMPLE_SAMPLE_RATE / MIC_EXAMPLE_FFT_SIZE;
        weighted_sum += frequency * magnitude[i];
        magnitude_sum += magnitude[i];
    }
    spectral->Centroid = (magnitude_sum > 0.0f) ? (weighted_sum / magnitude_sum) : 0.0f;

    /* Calculate brightness (high frequency content) */
    float high_freq_sum = 0.0f;
    uint32_t high_freq_start = (MIC_EXAMPLE_FFT_SIZE / 2) / 2; /* Above Nyquist/2 */
    for (uint32_t i = high_freq_start; i < (MIC_EXAMPLE_FFT_SIZE / 2); i++) {
        high_freq_sum += magnitude[i];
    }
    spectral->Brightness = (magnitude_sum > 0.0f) ? (high_freq_sum / magnitude_sum) : 0.0f;

    /* Calculate spectral rolloff (90% of energy) */
    float energy_threshold = magnitude_sum * 0.9f;
    float running_sum = 0.0f;
    for (uint32_t i = 0; i < (MIC_EXAMPLE_FFT_SIZE / 2); i++) {
        running_sum += magnitude[i];
        if (running_sum >= energy_threshold) {
            spectral->Rolloff = (float)i * MIC_EXAMPLE_SAMPLE_RATE / MIC_EXAMPLE_FFT_SIZE;
            break;
        }
    }

    return MIC_OK;
}

/**
 * @brief Detect voice activity in audio buffer
 * @param buffer Audio buffer
 * @param length Buffer length
 * @param vad Pointer to voice activity structure
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_Example_DetectVoiceActivity(int16_t *buffer, uint32_t length, MIC_VoiceActivity_t *vad)
{
    if (buffer == NULL || vad == NULL || length == 0) {
        return MIC_INVALID_PARAM;
    }

    /* Calculate frame energy */
    float energy = MIC_Example_CalculateEnergy(buffer, length);

    /* Calculate zero crossing rate */
    float zcr = MIC_Example_CalculateZCR(buffer, length);

    /* Store in history */
    vad->History[vad->HistoryIndex] = energy;
    vad->HistoryIndex = (vad->HistoryIndex + 1) % MIC_EXAMPLE_VAD_HISTORY_SIZE;

    /* Calculate average energy over history */
    float avg_energy = 0.0f;
    for (uint32_t i = 0; i < MIC_EXAMPLE_VAD_HISTORY_SIZE; i++) {
        avg_energy += vad->History[i];
    }
    avg_energy /= MIC_EXAMPLE_VAD_HISTORY_SIZE;

    /* Voice activity decision */
    bool energy_above_threshold = energy > EXAMPLE_VAD_ENERGY_THRESHOLD;
    bool zcr_in_speech_range = (zcr > 0.1f) && (zcr < EXAMPLE_VAD_ZCR_THRESHOLD);
    bool energy_above_background = energy > (avg_energy * 2.0f);

    vad->IsVoiceActive = energy_above_threshold && zcr_in_speech_range && energy_above_background;

    /* Update confidence */
    if (vad->IsVoiceActive) {
        vad->Confidence = fminf(1.0f, energy / EXAMPLE_VAD_ENERGY_THRESHOLD);
        vad->VoiceFrames++;
    } else {
        vad->Confidence = 0.0f;
        vad->SilenceFrames++;
    }

    return MIC_OK;
}

/**
 * @brief Display VU meter on console
 * @param vu_meter Pointer to VU meter structure
 */
void MIC_Example_DisplayVUMeter(MIC_VUMeter_t *vu_meter)
{
    if (vu_meter == NULL) {
        return;
    }

    printf("\r|");
    for (uint32_t i = 0; i < MIC_EXAMPLE_VU_LEVELS; i++) {
        if (vu_meter->LevelsBars[i] > 0.5f) {
            if (i < 6) {
                printf("="); /* Green */
            } else if (i < 8) {
                printf("!"); /* Yellow */
            } else {
                printf("#"); /* Red */
            }
        } else {
            printf(" ");
        }
    }
    printf("| %.1f dB", MIC_Example_LinearToDecibel(vu_meter->Smoothed));
    fflush(stdout);
}

/**
 * @brief Get default configuration
 * @retval MIC_ExampleConfig_t Default configuration
 */
MIC_ExampleConfig_t MIC_Example_GetDefaultConfig(void)
{
    MIC_ExampleConfig_t config = {
        .SampleRate = MIC_EXAMPLE_SAMPLE_RATE,
        .Volume = MIC_EXAMPLE_VOLUME_DEFAULT,
        .Gain = MIC_EXAMPLE_GAIN_DEFAULT,
        .EnableVAD = false,
        .EnableCompressor = false,
        .EnableEcho = false,
        .EnableSpectralAnalysis = false,
        .EnableVUMeter = true
    };

    return config;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Initialize hardware peripherals
 */
static void MIC_Example_InitHardware(void)
{
    /* This would typically be done in the main initialization */
    /* Here we assume the hardware is already initialized */

    /* Initialize I2S3 */
    hi2s3.Instance = SPI3;
    hi2s3.Init.Mode = I2S_MODE_MASTER_RX;
    hi2s3.Init.Standard = I2S_STANDARD_LSB;
    hi2s3.Init.DataFormat = I2S_DATAFORMAT_16B;
    hi2s3.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
    hi2s3.Init.AudioFreq = MIC_EXAMPLE_SAMPLE_RATE;
    hi2s3.Init.CPOL = I2S_CPOL_HIGH;
    hi2s3.Init.ClockSource = I2S_CLOCK_PLL;
    hi2s3.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;

    /* Initialize DMA */
    hdma_spi3_rx.Instance = DMA1_Stream0;
    hdma_spi3_rx.Init.Channel = DMA_CHANNEL_0;
    hdma_spi3_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_spi3_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi3_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi3_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_spi3_rx.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_spi3_rx.Init.Mode = DMA_CIRCULAR;
    hdma_spi3_rx.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_spi3_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
}

/**
 * @brief Convert linear amplitude to decibels
 * @param linear Linear amplitude value
 * @retval float Value in decibels
 */
static float MIC_Example_LinearToDecibel(float linear)
{
    if (linear <= 0.0f) {
        return -100.0f; /* -100 dB for silence */
    }
    return 20.0f * log10f(linear);
}

/**
 * @brief Convert decibels to linear amplitude
 * @param decibel Value in decibels
 * @retval float Linear amplitude value
 */
static float MIC_Example_DecibelToLinear(float decibel)
{
    return powf(10.0f, decibel / 20.0f);
}

/**
 * @brief Calculate zero crossing rate
 * @param buffer Audio buffer
 * @param length Buffer length
 * @retval float Zero crossing rate
 */
static float MIC_Example_CalculateZCR(int16_t *buffer, uint32_t length)
{
    if (buffer == NULL || length == 0) {
        return 0.0f;
    }

    uint32_t zero_crossings = 0;
    for (uint32_t i = 1; i < length; i++) {
        if ((buffer[i-1] >= 0 && buffer[i] < 0) || (buffer[i-1] < 0 && buffer[i] >= 0)) {
            zero_crossings++;
        }
    }

    return (float)zero_crossings / (length - 1);
}

/**
 * @brief Calculate frame energy
 * @param buffer Audio buffer
 * @param length Buffer length
 * @retval float Energy value
 */
static float MIC_Example_CalculateEnergy(int16_t *buffer, uint32_t length)
{
    if (buffer == NULL || length == 0) {
        return 0.0f;
    }

    float energy = 0.0f;
    for (uint32_t i = 0; i < length; i++) {
        float sample = (float)buffer[i] / 32768.0f;
        energy += sample * sample;
    }

    return energy / length;
}

/**
 * @brief Simple FFT implementation for demonstration
 * @param input Input time domain data
 * @param magnitude Output magnitude spectrum
 * @param length Input length
 */
static void MIC_Example_SimpleFFT(int16_t *input, float *magnitude, uint32_t length)
{
    if (input == NULL || magnitude == NULL) {
        return;
    }

    /* Simplified DFT for demonstration */
    for (uint32_t k = 0; k < length / 2; k++) {
        float real = 0.0f;
        float imag = 0.0f;

        for (uint32_t n = 0; n < length; n++) {
            float angle = -2.0f * EXAMPLE_PI * k * n / length;
            float sample = (float)input[n] / 32768.0f;
            real += sample * cosf(angle);
            imag += sample * sinf(angle);
        }

        magnitude[k] = sqrtf(real * real + imag * imag) / length;
    }
}
