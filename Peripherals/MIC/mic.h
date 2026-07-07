/**
  ******************************************************************************
  * @file    mic.h
  * @brief   Microphone driver interface for STM32F429 Discovery Board
  * @details This file contains function prototypes and definitions for
  *          the MP45DT02 digital MEMS microphone driver. The microphone
  *          uses PDM (Pulse Density Modulation) output with I2S clock.
  * @version 1.0
  * @date    2025-09-04
  ******************************************************************************
  */

#ifndef MIC_H
#define MIC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f4xx_hal_i2s.h"
#include "stm32f4xx_hal_dma.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported constants --------------------------------------------------------*/
#define MIC_OPERATION_TIMEOUT           1000
#define MIC_MAX_SAMPLES                 8192
#define MIC_DEFAULT_SAMPLE_RATE         16000   /* Hz */
#define MIC_DEFAULT_VOLUME              75      /* Percentage */

/* MP45DT02 Microphone Constants */
#define MIC_SENSITIVITY                 (-26.0f)  /* dBFS */
#define MIC_SNR                         (61.0f)   /* dB */
#define MIC_AOP                         (120.0f)  /* dB SPL */
#define MIC_FREQUENCY_RESPONSE_MIN      100       /* Hz */
#define MIC_FREQUENCY_RESPONSE_MAX      10000     /* Hz */

/* GPIO Pin Definitions */
#define MIC_CLK_PIN                     GPIO_PIN_10   /* PC10 - I2S3_CK */
#define MIC_CLK_GPIO_PORT               GPIOC
#define MIC_DATA_PIN                    GPIO_PIN_3    /* PC3 - PDM_OUT */
#define MIC_DATA_GPIO_PORT              GPIOC
#define MIC_DATA_EXTI_IRQn              EXTI3_IRQn

/* I2S Definitions */
#define MIC_I2S                         SPI3
#define MIC_I2S_CLK_ENABLE()            __HAL_RCC_SPI3_CLK_ENABLE()
#define MIC_I2S_CLK_DISABLE()           __HAL_RCC_SPI3_CLK_DISABLE()

/* DMA Definitions */
#define MIC_DMA_STREAM                  DMA1_Stream0
#define MIC_DMA_CHANNEL                 DMA_CHANNEL_0
#define MIC_DMA_IRQ                     DMA1_Stream0_IRQn
#define MIC_DMA_CLK_ENABLE()            __HAL_RCC_DMA1_CLK_ENABLE()

/* Audio Processing Constants */
#define MIC_PCM_SAMPLES                 128
#define MIC_PDM_BUFFER_SIZE             512
#define MIC_DECIMATION_FACTOR           64
#define MIC_FILTER_ORDER                4
#define MIC_GAIN_DEFAULT                (1.0f)
#define MIC_FFT_SIZE                    64      /* FFT analysis size */

/* Audio Format Constants */
#define MIC_MAX_VOLUME                  100     /* Maximum volume percentage */
#define MIC_PCM_MAX_VALUE               32767   /* Maximum 16-bit PCM value */
#define MIC_PCM_SCALE_FACTOR            32768.0f /* PCM scaling factor */
#define MIC_PDM_WORD_SIZE               32      /* PDM word size in bits */

/* Audio Processing Coefficients */
#define MIC_HP_FILTER_COEFF             (0.95f)   /* High-pass filter coefficient */
#define MIC_VOLUME_SCALE_FACTOR         (0.01f)   /* Volume scaling factor */
#define MIC_STATISTICS_ALPHA            (0.1f)    /* Statistics smoothing factor */
#define MIC_NOISE_FLOOR_LINEAR          (0.001f)  /* Noise floor (-60 dB) */
#define MIC_DB_SCALE_FACTOR             (20.0f)   /* dB conversion factor */
#define MIC_LOG_BASE                    (10.0f)   /* Logarithm base for dB */
#define MIC_FFT_SCALE_FACTOR            (2.0f)    /* FFT scaling factor */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Microphone status enumeration
 */
typedef enum {
    MIC_OK = 0,                     /**< Operation completed successfully */
    MIC_ERROR,                      /**< General error occurred */
    MIC_BUSY,                       /**< Microphone is busy */
    MIC_TIMEOUT,                    /**< Operation timed out */
    MIC_INVALID_PARAM,              /**< Invalid parameter provided */
    MIC_NOT_INITIALIZED,            /**< Device not initialized */
    MIC_BUFFER_OVERFLOW,            /**< Buffer overflow occurred */
    MIC_DMA_ERROR,                  /**< DMA transfer error */
    MIC_I2S_ERROR                   /**< I2S communication error */
} MIC_StatusTypeDef;

/**
 * @brief Microphone sample rate enumeration
 */
typedef enum {
    MIC_SAMPLE_RATE_8KHZ = 8000,    /**< 8 kHz sample rate */
    MIC_SAMPLE_RATE_16KHZ = 16000,  /**< 16 kHz sample rate */
    MIC_SAMPLE_RATE_22KHZ = 22050,  /**< 22.05 kHz sample rate */
    MIC_SAMPLE_RATE_32KHZ = 32000,  /**< 32 kHz sample rate */
    MIC_SAMPLE_RATE_44KHZ = 44100,  /**< 44.1 kHz sample rate */
    MIC_SAMPLE_RATE_48KHZ = 48000   /**< 48 kHz sample rate */
} MIC_SampleRateTypeDef;

/**
 * @brief Microphone audio format enumeration
 */
typedef enum {
    MIC_FORMAT_PCM16 = 0,           /**< 16-bit PCM format */
    MIC_FORMAT_PCM24,               /**< 24-bit PCM format */
    MIC_FORMAT_PCM32,               /**< 32-bit PCM format */
    MIC_FORMAT_FLOAT32              /**< 32-bit float format */
} MIC_AudioFormatTypeDef;

/**
 * @brief Microphone gain control
 */
typedef enum {
    MIC_GAIN_0DB = 0,               /**< 0 dB gain */
    MIC_GAIN_6DB,                   /**< 6 dB gain */
    MIC_GAIN_12DB,                  /**< 12 dB gain */
    MIC_GAIN_18DB,                  /**< 18 dB gain */
    MIC_GAIN_24DB                   /**< 24 dB gain */
} MIC_GainTypeDef;

/**
 * @brief Microphone filter type
 */
typedef enum {
    MIC_FILTER_LOWPASS = 0,         /**< Low-pass filter */
    MIC_FILTER_HIGHPASS,            /**< High-pass filter */
    MIC_FILTER_BANDPASS,            /**< Band-pass filter */
    MIC_FILTER_NOTCH                /**< Notch filter */
} MIC_FilterTypeTypeDef;

/**
 * @brief Microphone configuration structure
 */
typedef struct {
    MIC_SampleRateTypeDef SampleRate;       /**< Audio sample rate */
    MIC_AudioFormatTypeDef AudioFormat;     /**< Audio data format */
    MIC_GainTypeDef Gain;                   /**< Microphone gain */
    uint8_t Volume;                         /**< Volume level (0-100) */
    bool NoiseGateEnable;                   /**< Noise gate enable */
    float NoiseGateThreshold;               /**< Noise gate threshold */
    bool AutoGainControl;                   /**< Automatic gain control */
    bool HighPassFilter;                    /**< High-pass filter enable */
    uint16_t BufferSize;                    /**< Audio buffer size */
} MIC_ConfigTypeDef;

/**
 * @brief Microphone audio buffer structure
 */
typedef struct {
    uint16_t *pData;                        /**< Pointer to audio data */
    uint32_t Size;                          /**< Buffer size in samples */
    uint32_t Position;                      /**< Current position */
    bool IsFull;                            /**< Buffer full flag */
    uint32_t Timestamp;                     /**< Buffer timestamp */
} MIC_AudioBufferTypeDef;

/**
 * @brief Microphone statistics structure
 */
typedef struct {
    float RMSLevel;                         /**< RMS audio level */
    float PeakLevel;                        /**< Peak audio level */
    float SNR;                              /**< Signal-to-noise ratio */
    uint32_t SampleCount;                   /**< Total samples processed */
    uint32_t OverrunCount;                  /**< Buffer overrun count */
    uint32_t DropoutCount;                  /**< Audio dropout count */
    float AverageLevel;                     /**< Average audio level */
} MIC_StatisticsTypeDef;

/**
 * @brief Microphone handle structure
 */
typedef struct {
    I2S_HandleTypeDef *hi2s;                /**< I2S handle */
    DMA_HandleTypeDef *hdma;                /**< DMA handle */
    MIC_ConfigTypeDef Config;               /**< Microphone configuration */
    MIC_AudioBufferTypeDef AudioBuffer;     /**< Audio buffer */
    MIC_StatisticsTypeDef Statistics;       /**< Audio statistics */
    bool IsInitialized;                     /**< Initialization status */
    bool IsRecording;                       /**< Recording status */
    bool IsPaused;                          /**< Pause status */
    uint32_t PDMBuffer[MIC_PDM_BUFFER_SIZE];/**< PDM data buffer */
    int16_t PCMBuffer[MIC_PCM_SAMPLES];     /**< PCM output buffer */
    volatile bool BufferReady;              /**< Buffer ready flag */
    void (*RecordCallback)(void);           /**< Record complete callback */
    void (*ErrorCallback)(void);            /**< Error callback */
} MIC_HandleTypeDef;

/**
 * @brief PDM filter structure
 */
typedef struct {
    uint32_t LP_HZ;                         /**< Low-pass filter frequency */
    uint32_t HP_HZ;                         /**< High-pass filter frequency */
    uint16_t Fs;                            /**< Sample frequency */
    float Out_MicChannels;                  /**< Output channels */
    float In_MicChannels;                   /**< Input channels */
    uint8_t Decimation;                     /**< Decimation factor */
} MIC_PDMFilterTypeDef;

/**
 * @brief Audio analysis structure
 */
typedef struct {
    float FrequencyBins[MIC_FFT_SIZE];      /**< Frequency domain bins */
    float MagnitudeSpectrum[MIC_FFT_SIZE];  /**< Magnitude spectrum */
    float PhaseSpectrum[MIC_FFT_SIZE];      /**< Phase spectrum */
    float Centroid;                         /**< Spectral centroid */
    float Bandwidth;                        /**< Spectral bandwidth */
    float ZeroCrossingRate;                 /**< Zero crossing rate */
} MIC_AudioAnalysisTypeDef;

/* Exported function prototypes ---------------------------------------------*/

/**
 * @brief Initialize microphone system
 * @param hmic Pointer to microphone handle structure
 * @param hi2s Pointer to I2S handle
 * @param hdma Pointer to DMA handle
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_Init(MIC_HandleTypeDef *hmic, I2S_HandleTypeDef *hi2s, DMA_HandleTypeDef *hdma);

/**
 * @brief Deinitialize microphone system
 * @param hmic Pointer to microphone handle structure
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_DeInit(MIC_HandleTypeDef *hmic);

/**
 * @brief Configure microphone parameters
 * @param hmic Pointer to microphone handle structure
 * @param config Pointer to configuration structure
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_Configure(MIC_HandleTypeDef *hmic, MIC_ConfigTypeDef *config);

/**
 * @brief Start audio recording
 * @param hmic Pointer to microphone handle structure
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_StartRecording(MIC_HandleTypeDef *hmic);

/**
 * @brief Stop audio recording
 * @param hmic Pointer to microphone handle structure
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_StopRecording(MIC_HandleTypeDef *hmic);

/**
 * @brief Pause audio recording
 * @param hmic Pointer to microphone handle structure
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_PauseRecording(MIC_HandleTypeDef *hmic);

/**
 * @brief Resume audio recording
 * @param hmic Pointer to microphone handle structure
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_ResumeRecording(MIC_HandleTypeDef *hmic);

/**
 * @brief Read audio data
 * @param hmic Pointer to microphone handle structure
 * @param buffer Pointer to data buffer
 * @param length Number of samples to read
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_ReadAudioData(MIC_HandleTypeDef *hmic, int16_t *buffer, uint32_t length);

/**
 * @brief Set microphone volume
 * @param hmic Pointer to microphone handle structure
 * @param volume Volume level (0-100)
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_SetVolume(MIC_HandleTypeDef *hmic, uint8_t volume);

/**
 * @brief Get microphone volume
 * @param hmic Pointer to microphone handle structure
 * @param volume Pointer to store volume level
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_GetVolume(MIC_HandleTypeDef *hmic, uint8_t *volume);

/**
 * @brief Set microphone gain
 * @param hmic Pointer to microphone handle structure
 * @param gain Gain setting
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_SetGain(MIC_HandleTypeDef *hmic, MIC_GainTypeDef gain);

/**
 * @brief Get audio level
 * @param hmic Pointer to microphone handle structure
 * @param rms_level Pointer to store RMS level
 * @param peak_level Pointer to store peak level
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_GetAudioLevel(MIC_HandleTypeDef *hmic, float *rms_level, float *peak_level);

/**
 * @brief Get audio statistics
 * @param hmic Pointer to microphone handle structure
 * @param stats Pointer to store statistics
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_GetStatistics(MIC_HandleTypeDef *hmic, MIC_StatisticsTypeDef *stats);

/**
 * @brief Reset audio statistics
 * @param hmic Pointer to microphone handle structure
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_ResetStatistics(MIC_HandleTypeDef *hmic);

/**
 * @brief Perform audio analysis
 * @param hmic Pointer to microphone handle structure
 * @param analysis Pointer to store analysis results
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_PerformAudioAnalysis(MIC_HandleTypeDef *hmic, MIC_AudioAnalysisTypeDef *analysis);

/**
 * @brief Apply audio filter
 * @param hmic Pointer to microphone handle structure
 * @param filter_type Type of filter to apply
 * @param frequency Filter frequency in Hz
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_ApplyFilter(MIC_HandleTypeDef *hmic, MIC_FilterTypeTypeDef filter_type, uint32_t frequency);

/**
 * @brief Enable/disable noise gate
 * @param hmic Pointer to microphone handle structure
 * @param enable Enable/disable flag
 * @param threshold Noise gate threshold
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_ConfigureNoiseGate(MIC_HandleTypeDef *hmic, bool enable, float threshold);

/**
 * @brief Register callback functions
 * @param hmic Pointer to microphone handle structure
 * @param record_callback Record complete callback
 * @param error_callback Error callback
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_RegisterCallbacks(MIC_HandleTypeDef *hmic,
                                       void (*record_callback)(void),
                                       void (*error_callback)(void));

/**
 * @brief Check if recording is active
 * @param hmic Pointer to microphone handle structure
 * @retval bool True if recording is active
 */
bool MIC_IsRecording(MIC_HandleTypeDef *hmic);

/**
 * @brief Get current sample rate
 * @param hmic Pointer to microphone handle structure
 * @retval uint32_t Current sample rate in Hz
 */
uint32_t MIC_GetSampleRate(MIC_HandleTypeDef *hmic);

/**
 * @brief Convert PDM to PCM
 * @param pdm_buffer Pointer to PDM input buffer
 * @param pcm_buffer Pointer to PCM output buffer
 * @param length Number of samples to convert
 * @retval MIC_StatusTypeDef Status of the operation
 */
MIC_StatusTypeDef MIC_PDMToPCM(uint32_t *pdm_buffer, int16_t *pcm_buffer, uint32_t length);

/**
 * @brief Calculate audio RMS level
 * @param buffer Pointer to audio buffer
 * @param length Buffer length in samples
 * @retval float RMS level value
 */
float MIC_CalculateRMS(int16_t *buffer, uint32_t length);

/**
 * @brief Calculate audio peak level
 * @param buffer Pointer to audio buffer
 * @param length Buffer length in samples
 * @retval float Peak level value
 */
float MIC_CalculatePeak(int16_t *buffer, uint32_t length);

/* Callback function prototypes (weak definitions) --------------------------*/

/**
 * @brief Half transfer complete callback
 * @param hmic Pointer to microphone handle structure
 */
void MIC_HalfTransferCallback(MIC_HandleTypeDef *hmic);

/**
 * @brief Transfer complete callback
 * @param hmic Pointer to microphone handle structure
 */
void MIC_TransferCompleteCallback(MIC_HandleTypeDef *hmic);

/**
 * @brief Error callback
 * @param hmic Pointer to microphone handle structure
 */
void MIC_ErrorCallback(MIC_HandleTypeDef *hmic);

/* IRQ Handler prototypes ---------------------------------------------------*/

/**
 * @brief DMA interrupt handler
 */
void MIC_DMA_IRQHandler(void);

/**
 * @brief I2S interrupt handler
 */
void MIC_I2S_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* MIC_H */
