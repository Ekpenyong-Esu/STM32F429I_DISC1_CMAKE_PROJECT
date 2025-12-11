/**
  ******************************************************************************
  * @file    mic.c
  * @brief   Microphone driver implementation for STM32F429 Discovery Board
  * @details This file provides the implementation of microphone functions
  *          for the MP45DT02 digital MEMS microphone.
  * @version 1.0
  * @date    2025-09-04
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "mic.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>

/* Private constants ---------------------------------------------------------*/
#define MIC_DELAY_MS(x)                 HAL_Delay(x)
#define MIC_PDM_CLOCK_FREQUENCY         1280000  /* 1.28 MHz */
#define MIC_TEMPERATURE_OFFSET          (25.0f)
#define MIC_GAIN_SCALE_FACTORS          {1.0f, 2.0f, 4.0f, 8.0f, 16.0f}
#define MIC_NOISE_FLOOR                 (-60.0f)   /* dB */
#define MIC_MAX_AMPLITUDE               32767
#define MIC_MIN_AMPLITUDE               (-32768)
#define MIC_SPECTRAL_BINS               32

/* Private variables ---------------------------------------------------------*/
static MIC_HandleTypeDef *g_hmic = NULL;
static const float mic_gain_factors[] = MIC_GAIN_SCALE_FACTORS;

/* Private function prototypes -----------------------------------------------*/
static MIC_StatusTypeDef MIC_InitI2S(MIC_HandleTypeDef *hmic);
static MIC_StatusTypeDef MIC_InitDMA(MIC_HandleTypeDef *hmic);
static MIC_StatusTypeDef MIC_InitGPIO(void);
static void MIC_ProcessPDMData(MIC_HandleTypeDef *hmic);
static void MIC_ApplyVolumeControl(MIC_HandleTypeDef *hmic, int16_t *buffer, uint32_t length);
static void MIC_ApplyGainControl(MIC_HandleTypeDef *hmic, int16_t *buffer, uint32_t length);
static void MIC_UpdateStatistics(MIC_HandleTypeDef *hmic, int16_t *buffer, uint32_t length);
static MIC_StatusTypeDef MIC_ConfigurePDMFilter(MIC_HandleTypeDef *hmic);
static bool MIC_IsNoiseGateOpen(MIC_HandleTypeDef *hmic, float level);
static void MIC_PerformFFT(int16_t *input, float *magnitude, uint32_t length);
static float MIC_CalculateSpectralCentroid(float *spectrum, uint32_t length, uint32_t sample_rate);

/* Exported functions --------------------------------------------------------*/

/**
 * @brief Initialize microphone system
 * @param hmic Pointer to microphone handle structure
 * @param hi2s Pointer to I2S handle
 * @param hdma Pointer to DMA handle
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_Init(MIC_HandleTypeDef *hmic, I2S_HandleTypeDef *hi2s, DMA_HandleTypeDef *hdma)
{
    MIC_StatusTypeDef status = MIC_OK;

    /* Check parameters */
    if (hmic == NULL || hi2s == NULL || hdma == NULL) {
        return MIC_INVALID_PARAM;
    }

    /* Initialize structure */
    memset(hmic, 0, sizeof(MIC_HandleTypeDef));
    hmic->hi2s = hi2s;
    hmic->hdma = hdma;
    g_hmic = hmic;

    /* Initialize GPIO pins */
    status = MIC_InitGPIO();
    if (status != MIC_OK) {
        return status;
    }

    /* Initialize I2S peripheral */
    status = MIC_InitI2S(hmic);
    if (status != MIC_OK) {
        return status;
    }

    /* Initialize DMA */
    status = MIC_InitDMA(hmic);
    if (status != MIC_OK) {
        return status;
    }

    /* Set default configuration */
    MIC_ConfigTypeDef default_config = {
        .SampleRate = MIC_SAMPLE_RATE_16KHZ,
        .AudioFormat = MIC_FORMAT_PCM16,
        .Gain = MIC_GAIN_0DB,
        .Volume = MIC_DEFAULT_VOLUME,
        .NoiseGateEnable = false,
        .NoiseGateThreshold = MIC_NOISE_FLOOR,
        .AutoGainControl = false,
        .HighPassFilter = true,
        .BufferSize = MIC_PCM_SAMPLES
    };

    status = MIC_Configure(hmic, &default_config);
    if (status != MIC_OK) {
        return status;
    }

    /* Configure PDM filter */
    status = MIC_ConfigurePDMFilter(hmic);
    if (status != MIC_OK) {
        return status;
    }

    hmic->IsInitialized = true;
    return MIC_OK;
}

/**
 * @brief Deinitialize microphone system
 * @param hmic Pointer to microphone handle structure
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_DeInit(MIC_HandleTypeDef *hmic)
{
    if (hmic == NULL) {
        return MIC_INVALID_PARAM;
    }

    /* Stop recording if active */
    if (hmic->IsRecording) {
        MIC_StopRecording(hmic);
    }

    /* Deinitialize peripherals */
    if (hmic->hi2s != NULL) {
        HAL_I2S_DeInit(hmic->hi2s);
    }

    if (hmic->hdma != NULL) {
        HAL_DMA_DeInit(hmic->hdma);
    }

    /* Reset structure */
    hmic->IsInitialized = false;
    hmic->IsRecording = false;
    g_hmic = NULL;

    return MIC_OK;
}

/**
 * @brief Configure microphone parameters
 * @param hmic Pointer to microphone handle structure
 * @param config Pointer to configuration structure
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_Configure(MIC_HandleTypeDef *hmic, MIC_ConfigTypeDef *config)
{
    if (hmic == NULL || config == NULL) {
        return MIC_INVALID_PARAM;
    }

    if (!hmic->IsInitialized) {
        return MIC_NOT_INITIALIZED;
    }

    /* Validate parameters */
    if (config->Volume > MIC_MAX_VOLUME) {
        return MIC_INVALID_PARAM;
    }

    if (config->BufferSize > MIC_MAX_SAMPLES) {
        return MIC_INVALID_PARAM;
    }

    /* Store configuration */
    hmic->Config = *config;

    /* Allocate audio buffer if needed */
    if (hmic->AudioBuffer.pData == NULL) {
        hmic->AudioBuffer.pData = (uint16_t*)malloc(config->BufferSize * sizeof(uint16_t));
        if (hmic->AudioBuffer.pData == NULL) {
            return MIC_ERROR;
        }
        hmic->AudioBuffer.Size = config->BufferSize;
        hmic->AudioBuffer.Position = 0;
        hmic->AudioBuffer.IsFull = false;
    }

    /* Reconfigure I2S for new sample rate */
    hmic->hi2s->Init.AudioFreq = config->SampleRate;
    if (HAL_I2S_Init(hmic->hi2s) != HAL_OK) {
        return MIC_I2S_ERROR;
    }

    return MIC_OK;
}

/**
 * @brief Start audio recording
 * @param hmic Pointer to microphone handle structure
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_StartRecording(MIC_HandleTypeDef *hmic)
{
    if (hmic == NULL) {
        return MIC_INVALID_PARAM;
    }

    if (!hmic->IsInitialized) {
        return MIC_NOT_INITIALIZED;
    }

    if (hmic->IsRecording) {
        return MIC_BUSY;
    }

    /* Reset buffer position */
    hmic->AudioBuffer.Position = 0;
    hmic->AudioBuffer.IsFull = false;
    hmic->BufferReady = false;

    /* Reset statistics */
    MIC_ResetStatistics(hmic);

    /* Start I2S DMA transfer */
    if (HAL_I2S_Receive_DMA(hmic->hi2s, (uint16_t*)hmic->PDMBuffer, MIC_PDM_BUFFER_SIZE) != HAL_OK) {
        return MIC_DMA_ERROR;
    }

    hmic->IsRecording = true;
    hmic->IsPaused = false;

    return MIC_OK;
}

/**
 * @brief Stop audio recording
 * @param hmic Pointer to microphone handle structure
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_StopRecording(MIC_HandleTypeDef *hmic)
{
    if (hmic == NULL) {
        return MIC_INVALID_PARAM;
    }

    if (!hmic->IsRecording) {
        return MIC_OK; /* Already stopped */
    }

    /* Stop I2S DMA transfer */
    HAL_I2S_DMAStop(hmic->hi2s);

    hmic->IsRecording = false;
    hmic->IsPaused = false;
    hmic->BufferReady = false;

    return MIC_OK;
}

/**
 * @brief Pause audio recording
 * @param hmic Pointer to microphone handle structure
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_PauseRecording(MIC_HandleTypeDef *hmic)
{
    if (hmic == NULL) {
        return MIC_INVALID_PARAM;
    }

    if (!hmic->IsRecording || hmic->IsPaused) {
        return MIC_OK;
    }

    /* Pause DMA transfer */
    HAL_I2S_DMAPause(hmic->hi2s);
    hmic->IsPaused = true;

    return MIC_OK;
}

/**
 * @brief Resume audio recording
 * @param hmic Pointer to microphone handle structure
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_ResumeRecording(MIC_HandleTypeDef *hmic)
{
    if (hmic == NULL) {
        return MIC_INVALID_PARAM;
    }

    if (!hmic->IsRecording || !hmic->IsPaused) {
        return MIC_OK;
    }

    /* Resume DMA transfer */
    HAL_I2S_DMAResume(hmic->hi2s);
    hmic->IsPaused = false;

    return MIC_OK;
}

/**
 * @brief Read audio data
 * @param hmic Pointer to microphone handle structure
 * @param buffer Pointer to data buffer
 * @param length Number of samples to read
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_ReadAudioData(MIC_HandleTypeDef *hmic, int16_t *buffer, uint32_t length)
{
    if (hmic == NULL || buffer == NULL || length == 0) {
        return MIC_INVALID_PARAM;
    }

    if (!hmic->IsInitialized) {
        return MIC_NOT_INITIALIZED;
    }

    if (!hmic->BufferReady) {
        return MIC_BUSY; /* No data ready */
    }

    /* Copy available data */
    uint32_t copy_length = (length < MIC_PCM_SAMPLES) ? length : MIC_PCM_SAMPLES;
    memcpy(buffer, hmic->PCMBuffer, copy_length * sizeof(int16_t));

    /* Apply volume and gain control */
    MIC_ApplyVolumeControl(hmic, buffer, copy_length);
    MIC_ApplyGainControl(hmic, buffer, copy_length);

    /* Update statistics */
    MIC_UpdateStatistics(hmic, buffer, copy_length);

    hmic->BufferReady = false;

    return MIC_OK;
}

/**
 * @brief Set microphone volume
 * @param hmic Pointer to microphone handle structure
 * @param volume Volume level (0-100)
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_SetVolume(MIC_HandleTypeDef *hmic, uint8_t volume)
{
    if (hmic == NULL) {
        return MIC_INVALID_PARAM;
    }

    if (volume > MIC_MAX_VOLUME) {
        return MIC_INVALID_PARAM;
    }

    hmic->Config.Volume = volume;

    return MIC_OK;
}

/**
 * @brief Get microphone volume
 * @param hmic Pointer to microphone handle structure
 * @param volume Pointer to store volume level
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_GetVolume(MIC_HandleTypeDef *hmic, uint8_t *volume)
{
    if (hmic == NULL || volume == NULL) {
        return MIC_INVALID_PARAM;
    }

    *volume = hmic->Config.Volume;

    return MIC_OK;
}

/**
 * @brief Set microphone gain
 * @param hmic Pointer to microphone handle structure
 * @param gain Gain setting
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_SetGain(MIC_HandleTypeDef *hmic, MIC_GainTypeDef gain)
{
    if (hmic == NULL) {
        return MIC_INVALID_PARAM;
    }

    if (gain > MIC_GAIN_24DB) {
        return MIC_INVALID_PARAM;
    }

    hmic->Config.Gain = gain;

    return MIC_OK;
}

/**
 * @brief Get audio level
 * @param hmic Pointer to microphone handle structure
 * @param rms_level Pointer to store RMS level
 * @param peak_level Pointer to store peak level
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_GetAudioLevel(MIC_HandleTypeDef *hmic, float *rms_level, float *peak_level)
{
    if (hmic == NULL || rms_level == NULL || peak_level == NULL) {
        return MIC_INVALID_PARAM;
    }

    *rms_level = hmic->Statistics.RMSLevel;
    *peak_level = hmic->Statistics.PeakLevel;

    return MIC_OK;
}

/**
 * @brief Get audio statistics
 * @param hmic Pointer to microphone handle structure
 * @param stats Pointer to store statistics
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_GetStatistics(MIC_HandleTypeDef *hmic, MIC_StatisticsTypeDef *stats)
{
    if (hmic == NULL || stats == NULL) {
        return MIC_INVALID_PARAM;
    }

    *stats = hmic->Statistics;

    return MIC_OK;
}

/**
 * @brief Reset audio statistics
 * @param hmic Pointer to microphone handle structure
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_ResetStatistics(MIC_HandleTypeDef *hmic)
{
    if (hmic == NULL) {
        return MIC_INVALID_PARAM;
    }

    memset(&hmic->Statistics, 0, sizeof(MIC_StatisticsTypeDef));

    return MIC_OK;
}

/**
 * @brief Perform audio analysis
 * @param hmic Pointer to microphone handle structure
 * @param analysis Pointer to store analysis results
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_PerformAudioAnalysis(MIC_HandleTypeDef *hmic, MIC_AudioAnalysisTypeDef *analysis)
{
    if (hmic == NULL || analysis == NULL) {
        return MIC_INVALID_PARAM;
    }

    if (!hmic->BufferReady) {
        return MIC_BUSY;
    }

    /* Perform FFT analysis */
    MIC_PerformFFT(hmic->PCMBuffer, analysis->MagnitudeSpectrum, MIC_FFT_SIZE);

    /* Calculate spectral centroid */
    analysis->Centroid = MIC_CalculateSpectralCentroid(analysis->MagnitudeSpectrum,
                                                      MIC_SPECTRAL_BINS,
                                                      hmic->Config.SampleRate);

    /* Calculate zero crossing rate */
    uint32_t zero_crossings = 0;
    for (uint32_t i = 1; i < MIC_PCM_SAMPLES; i++) {
        if ((hmic->PCMBuffer[i-1] >= 0 && hmic->PCMBuffer[i] < 0) ||
            (hmic->PCMBuffer[i-1] < 0 && hmic->PCMBuffer[i] >= 0)) {
            zero_crossings++;
        }
    }
    analysis->ZeroCrossingRate = (float)zero_crossings / MIC_PCM_SAMPLES;

    return MIC_OK;
}

/**
 * @brief Configure noise gate
 * @param hmic Pointer to microphone handle structure
 * @param enable Enable/disable flag
 * @param threshold Noise gate threshold
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_ConfigureNoiseGate(MIC_HandleTypeDef *hmic, bool enable, float threshold)
{
    if (hmic == NULL) {
        return MIC_INVALID_PARAM;
    }

    hmic->Config.NoiseGateEnable = enable;
    hmic->Config.NoiseGateThreshold = threshold;

    return MIC_OK;
}

/**
 * @brief Register callback functions
 * @param hmic Pointer to microphone handle structure
 * @param record_callback Record complete callback
 * @param error_callback Error callback
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_RegisterCallbacks(MIC_HandleTypeDef *hmic,
                                       void (*record_callback)(void),
                                       void (*error_callback)(void))
{
    if (hmic == NULL) {
        return MIC_INVALID_PARAM;
    }

    hmic->RecordCallback = record_callback;
    hmic->ErrorCallback = error_callback;

    return MIC_OK;
}

/**
 * @brief Check if recording is active
 * @param hmic Pointer to microphone handle structure
 * @retval bool True if recording is active
 */
bool MIC_IsRecording(MIC_HandleTypeDef *hmic)
{
    if (hmic == NULL) {
        return false;
    }

    return hmic->IsRecording && !hmic->IsPaused;
}

/**
 * @brief Get current sample rate
 * @param hmic Pointer to microphone handle structure
 * @retval uint32_t Current sample rate in Hz
 */
uint32_t MIC_GetSampleRate(MIC_HandleTypeDef *hmic)
{
    if (hmic == NULL) {
        return 0;
    }

    return (uint32_t)hmic->Config.SampleRate;
}

/**
 * @brief Convert PDM to PCM
 * @param pdm_buffer Pointer to PDM input buffer
 * @param pcm_buffer Pointer to PCM output buffer
 * @param length Number of samples to convert
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_PDMToPCM(uint32_t *pdm_buffer, int16_t *pcm_buffer, uint32_t length)
{
    if (pdm_buffer == NULL || pcm_buffer == NULL || length == 0) {
        return MIC_INVALID_PARAM;
    }

    /* Simple PDM to PCM conversion using decimation */
    uint32_t decimation = MIC_DECIMATION_FACTOR;

    for (uint32_t i = 0; i < length; i++) {
        int32_t accumulator = 0;

        /* Accumulate PDM bits over decimation period */
        for (uint32_t j = 0; j < decimation && (i * decimation + j) < (length * decimation); j++) {
            uint32_t pdm_word = pdm_buffer[(i * decimation + j) / MIC_PDM_WORD_SIZE];
            uint32_t bit_position = (i * decimation + j) % MIC_PDM_WORD_SIZE;

            if (pdm_word & (1 << bit_position)) {
                accumulator++;
            } else {
                accumulator--;
            }
        }

        /* Scale and convert to 16-bit PCM */
        pcm_buffer[i] = (int16_t)((accumulator * MIC_PCM_MAX_VALUE) / (int32_t)decimation);
    }

    return MIC_OK;
}

/**
 * @brief Calculate audio RMS level
 * @param buffer Pointer to audio buffer
 * @param length Buffer length in samples
 * @retval float RMS level value
 */
float MIC_CalculateRMS(int16_t *buffer, uint32_t length)
{
    if (buffer == NULL || length == 0) {
        return 0.0f;
    }

    float sum_squares = 0.0f;

    for (uint32_t i = 0; i < length; i++) {
        float sample = (float)buffer[i] / MIC_PCM_SCALE_FACTOR;
        sum_squares += sample * sample;
    }

    return sqrtf(sum_squares / (float)length);
}

/**
 * @brief Calculate audio peak level
 * @param buffer Pointer to audio buffer
 * @param length Buffer length in samples
 * @retval float Peak level value
 */
float MIC_CalculatePeak(int16_t *buffer, uint32_t length)
{
    if (buffer == NULL || length == 0) {
        return 0.0f;
    }

    int16_t peak = 0;

    for (uint32_t i = 0; i < length; i++) {
        int16_t abs_sample = (buffer[i] >= 0) ? buffer[i] : -buffer[i];
        if (abs_sample > peak) {
            peak = abs_sample;
        }
    }

    return (float)peak / MIC_PCM_SCALE_FACTOR;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Initialize I2S peripheral
 * @param hmic Pointer to microphone handle structure
 * @retval MIC_StatusTypeDef Status of the operation
 */
static MIC_StatusTypeDef MIC_InitI2S(MIC_HandleTypeDef *hmic)
{
    if (hmic->hi2s == NULL) {
        return MIC_INVALID_PARAM;
    }

    /* Enable I2S clock */
    MIC_I2S_CLK_ENABLE();

    /* Configure I2S parameters */
    hmic->hi2s->Instance = MIC_I2S;
    hmic->hi2s->Init.Mode = I2S_MODE_MASTER_RX;
    hmic->hi2s->Init.Standard = I2S_STANDARD_LSB;
    hmic->hi2s->Init.DataFormat = I2S_DATAFORMAT_16B;
    hmic->hi2s->Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
    hmic->hi2s->Init.AudioFreq = MIC_DEFAULT_SAMPLE_RATE;
    hmic->hi2s->Init.CPOL = I2S_CPOL_HIGH;
    hmic->hi2s->Init.ClockSource = I2S_CLOCK_PLL;
    hmic->hi2s->Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;

    if (HAL_I2S_Init(hmic->hi2s) != HAL_OK) {
        return MIC_I2S_ERROR;
    }

    return MIC_OK;
}

/**
 * @brief Initialize DMA for I2S
 * @param hmic Pointer to microphone handle structure
 * @retval MIC_StatusTypeDef Status of the operation
 */
static MIC_StatusTypeDef MIC_InitDMA(MIC_HandleTypeDef *hmic)
{
    if (hmic->hdma == NULL) {
        return MIC_INVALID_PARAM;
    }

    /* Enable DMA clock */
    MIC_DMA_CLK_ENABLE();

    /* Configure DMA parameters */
    hmic->hdma->Instance = MIC_DMA_STREAM;
    hmic->hdma->Init.Channel = MIC_DMA_CHANNEL;
    hmic->hdma->Init.Direction = DMA_PERIPH_TO_MEMORY;
    hmic->hdma->Init.PeriphInc = DMA_PINC_DISABLE;
    hmic->hdma->Init.MemInc = DMA_MINC_ENABLE;
    hmic->hdma->Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hmic->hdma->Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hmic->hdma->Init.Mode = DMA_CIRCULAR;
    hmic->hdma->Init.Priority = DMA_PRIORITY_HIGH;
    hmic->hdma->Init.FIFOMode = DMA_FIFOMODE_DISABLE;

    if (HAL_DMA_Init(hmic->hdma) != HAL_OK) {
        return MIC_DMA_ERROR;
    }

    /* Associate DMA handle with I2S */
    __HAL_LINKDMA(hmic->hi2s, hdmarx, *hmic->hdma);

    return MIC_OK;
}

/**
 * @brief Initialize GPIO pins for microphone
 * @retval MIC_StatusTypeDef Status of the operation
 */
static MIC_StatusTypeDef MIC_InitGPIO(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIO clocks */
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* Configure I2S CLK pin (PC10) */
    GPIO_InitStruct.Pin = MIC_CLK_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
    HAL_GPIO_Init(MIC_CLK_GPIO_PORT, &GPIO_InitStruct);

    /* Configure PDM Data pin (PC3) */
    GPIO_InitStruct.Pin = MIC_DATA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI3;
    HAL_GPIO_Init(MIC_DATA_GPIO_PORT, &GPIO_InitStruct);

    return MIC_OK;
}

/**
 * @brief Process PDM data and convert to PCM
 * @param hmic Pointer to microphone handle structure
 */
static void MIC_ProcessPDMData(MIC_HandleTypeDef *hmic)
{
    if (hmic == NULL) {
        return;
    }

    /* Convert PDM to PCM */
    MIC_PDMToPCM(hmic->PDMBuffer, hmic->PCMBuffer, MIC_PCM_SAMPLES);

    /* Apply filters if enabled */
    if (hmic->Config.HighPassFilter) {
        /* Simple high-pass filter implementation */
        static int16_t prev_input = 0;
        static int16_t prev_output = 0;

        for (uint32_t i = 0; i < MIC_PCM_SAMPLES; i++) {
            int16_t current_input = hmic->PCMBuffer[i];
            int16_t filtered = (int16_t)(((float)current_input - (float)prev_input + (float)prev_output) * MIC_HP_FILTER_COEFF);
            prev_input = current_input;
            prev_output = filtered;
            hmic->PCMBuffer[i] = filtered;
        }
    }

    /* Apply noise gate if enabled */
    if (hmic->Config.NoiseGateEnable) {
        float rms_level = MIC_CalculateRMS(hmic->PCMBuffer, MIC_PCM_SAMPLES);
        if (!MIC_IsNoiseGateOpen(hmic, rms_level)) {
            /* Mute the buffer */
            memset(hmic->PCMBuffer, 0, MIC_PCM_SAMPLES * sizeof(int16_t));
        }
    }

    hmic->BufferReady = true;
}

/**
 * @brief Apply volume control to audio buffer
 * @param hmic Pointer to microphone handle structure
 * @param buffer Pointer to audio buffer
 * @param length Buffer length in samples
 */
static void MIC_ApplyVolumeControl(MIC_HandleTypeDef *hmic, int16_t *buffer, uint32_t length)
{
    if (hmic == NULL || buffer == NULL) {
        return;
    }

    float volume_factor = (float)hmic->Config.Volume * MIC_VOLUME_SCALE_FACTOR;

    for (uint32_t i = 0; i < length; i++) {
        int32_t sample = (int32_t)((float)buffer[i] * volume_factor);

        /* Clamp to 16-bit range */
        if (sample > MIC_MAX_AMPLITUDE) {
            sample = MIC_MAX_AMPLITUDE;
        } else if (sample < MIC_MIN_AMPLITUDE) {
            sample = MIC_MIN_AMPLITUDE;
        }

        buffer[i] = (int16_t)sample;
    }
}

/**
 * @brief Apply gain control to audio buffer
 * @param hmic Pointer to microphone handle structure
 * @param buffer Pointer to audio buffer
 * @param length Buffer length in samples
 */
static void MIC_ApplyGainControl(MIC_HandleTypeDef *hmic, int16_t *buffer, uint32_t length)
{
    if (hmic == NULL || buffer == NULL) {
        return;
    }

    float gain_factor = mic_gain_factors[hmic->Config.Gain];

    for (uint32_t i = 0; i < length; i++) {
        int32_t sample = (int32_t)((float)buffer[i] * gain_factor);

        /* Clamp to 16-bit range */
        if (sample > MIC_MAX_AMPLITUDE) {
            sample = MIC_MAX_AMPLITUDE;
        } else if (sample < MIC_MIN_AMPLITUDE) {
            sample = MIC_MIN_AMPLITUDE;
        }

        buffer[i] = (int16_t)sample;
    }
}

/**
 * @brief Update audio statistics
 * @param hmic Pointer to microphone handle structure
 * @param buffer Pointer to audio buffer
 * @param length Buffer length in samples
 */
static void MIC_UpdateStatistics(MIC_HandleTypeDef *hmic, int16_t *buffer, uint32_t length)
{
    if (hmic == NULL || buffer == NULL) {
        return;
    }

    /* Calculate RMS and peak levels */
    float rms = MIC_CalculateRMS(buffer, length);
    float peak = MIC_CalculatePeak(buffer, length);

    /* Update statistics */
    hmic->Statistics.RMSLevel = rms;
    hmic->Statistics.PeakLevel = peak;
    hmic->Statistics.SampleCount += length;

    /* Update average level (exponential moving average) */
    float alpha = MIC_STATISTICS_ALPHA;
    hmic->Statistics.AverageLevel = (1.0f - alpha) * hmic->Statistics.AverageLevel + alpha * rms;

    /* Calculate SNR (simplified) */
    if (rms > 0.0f) {
        float noise_floor = MIC_NOISE_FLOOR_LINEAR; /* -60 dB */
        hmic->Statistics.SNR = MIC_DB_SCALE_FACTOR * log10f(rms / noise_floor);
    }
}

/**
 * @brief Configure PDM filter
 * @param hmic Pointer to microphone handle structure
 * @retval MIC_StatusTypeDef Status of the operation
 */
static MIC_StatusTypeDef MIC_ConfigurePDMFilter(MIC_HandleTypeDef *hmic)
{
    if (hmic == NULL) {
        return MIC_INVALID_PARAM;
    }

    /* PDM filter configuration would go here */
    /* This is a simplified implementation */

    return MIC_OK;
}

/**
 * @brief Check if noise gate should be open
 * @param hmic Pointer to microphone handle structure
 * @param level Current audio level
 * @retval bool True if gate should be open
 */
static bool MIC_IsNoiseGateOpen(MIC_HandleTypeDef *hmic, float level)
{
    if (hmic == NULL) {
        return true;
    }

    float threshold_linear = powf(MIC_LOG_BASE, hmic->Config.NoiseGateThreshold / MIC_DB_SCALE_FACTOR);
    return level > threshold_linear;
}

/**
 * @brief Perform FFT analysis (simplified implementation)
 * @param input Input time domain data
 * @param magnitude Output magnitude spectrum
 * @param length Input length
 */
static void MIC_PerformFFT(int16_t *input, float *magnitude, uint32_t length)
{
    if (input == NULL || magnitude == NULL) {
        return;
    }

    /* Simplified FFT - just calculate magnitude for demonstration */
    for (uint32_t i = 0; i < length / 2; i++) {
        float real = 0.0f;
        float imag = 0.0f;

        for (uint32_t j = 0; j < length; j++) {
            float angle = -MIC_FFT_SCALE_FACTOR * (float)M_PI * (float)i * (float)j / (float)length;
            real += (float)input[j] * cosf(angle);
            imag += (float)input[j] * sinf(angle);
        }

        magnitude[i] = sqrtf(real * real + imag * imag) / (float)length;
    }
}

/**
 * @brief Calculate spectral centroid
 * @param spectrum Magnitude spectrum
 * @param length Spectrum length
 * @param sample_rate Sample rate in Hz
 * @retval float Spectral centroid in Hz
 */
static float MIC_CalculateSpectralCentroid(float *spectrum, uint32_t length, uint32_t sample_rate)
{
    if (spectrum == NULL || length == 0) {
        return 0.0f;
    }

    float weighted_sum = 0.0f;
    float magnitude_sum = 0.0f;

    for (uint32_t i = 0; i < length; i++) {
        float frequency = (float)i * sample_rate / (2 * length);
        weighted_sum += frequency * spectrum[i];
        magnitude_sum += spectrum[i];
    }

    return (magnitude_sum > 0.0f) ? (weighted_sum / magnitude_sum) : 0.0f;
}

/* Callback functions -------------------------------------------------------*/

/**
 * @brief Half transfer complete callback
 * @param hmic Pointer to microphone handle structure
 */
__weak void MIC_HalfTransferCallback(MIC_HandleTypeDef *hmic)
{
    /* Process first half of PDM buffer */
    if (hmic != NULL) {
        MIC_ProcessPDMData(hmic);
    }
}

/**
 * @brief Transfer complete callback
 * @param hmic Pointer to microphone handle structure
 */
__weak void MIC_TransferCompleteCallback(MIC_HandleTypeDef *hmic)
{
    /* Process second half of PDM buffer */
    if (hmic != NULL) {
        MIC_ProcessPDMData(hmic);

        if (hmic->RecordCallback != NULL) {
            hmic->RecordCallback();
        }
    }
}

/**
 * @brief Error callback
 * @param hmic Pointer to microphone handle structure
 */
__weak void MIC_ErrorCallback(MIC_HandleTypeDef *hmic)
{
    if (hmic != NULL) {
        hmic->Statistics.DropoutCount++;

        if (hmic->ErrorCallback != NULL) {
            hmic->ErrorCallback();
        }
    }
}

/* IRQ Handlers -------------------------------------------------------------*/

/**
 * @brief DMA interrupt handler
 */
void MIC_DMA_IRQHandler(void)
{
    if (g_hmic != NULL && g_hmic->hdma != NULL) {
        HAL_DMA_IRQHandler(g_hmic->hdma);
    }
}

/**
 * @brief I2S interrupt handler
 */
void MIC_I2S_IRQHandler(void)
{
    if (g_hmic != NULL && g_hmic->hi2s != NULL) {
        HAL_I2S_IRQHandler(g_hmic->hi2s);
    }
}
