/**
  ******************************************************************************
  * @file    mic_example.h
  * @brief   Microphone driver example header for STM32F429 Discovery Board
  * @details This file contains example functions and configurations for
  *          using the MP45DT02 digital MEMS microphone.
  * @version 1.0
  * @date    2025-09-04
  ******************************************************************************
  */

#ifndef MIC_EXAMPLE_H
#define MIC_EXAMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "mic.h"
#include "main.h"

/* Exported constants --------------------------------------------------------*/
#define MIC_EXAMPLE_BUFFER_SIZE         1024
#define MIC_EXAMPLE_RECORDING_TIME      5000    /* ms */
#define MIC_EXAMPLE_VOLUME_DEFAULT      50      /* 0-100 */
#define MIC_EXAMPLE_GAIN_DEFAULT        MIC_GAIN_12DB
#define MIC_EXAMPLE_SAMPLE_RATE         MIC_SAMPLE_RATE_16KHZ

/* Audio analysis thresholds */
#define MIC_EXAMPLE_SILENCE_THRESHOLD   -40.0f  /* dB */
#define MIC_EXAMPLE_VOICE_THRESHOLD     -20.0f  /* dB */
#define MIC_EXAMPLE_LOUD_THRESHOLD      -6.0f   /* dB */

/* VU meter levels */
#define MIC_EXAMPLE_VU_LEVELS           10
#define MIC_EXAMPLE_VU_MIN_DB           -60.0f
#define MIC_EXAMPLE_VU_MAX_DB           0.0f

/* FFT analysis */
#define MIC_EXAMPLE_FFT_SIZE            64
#define MIC_EXAMPLE_SPECTRAL_BANDS      8

/* Voice activity detection */
#define MIC_EXAMPLE_VAD_FRAME_SIZE      160     /* 10ms at 16kHz */
#define MIC_EXAMPLE_VAD_HISTORY_SIZE    10

/* Audio effects */
#define MIC_EXAMPLE_ECHO_DELAY_MS       200
#define MIC_EXAMPLE_ECHO_FEEDBACK       0.3f
#define MIC_EXAMPLE_COMPRESSOR_RATIO    4.0f
#define MIC_EXAMPLE_COMPRESSOR_THRESHOLD -20.0f

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Audio level categories
 */
typedef enum {
    MIC_LEVEL_SILENCE = 0,
    MIC_LEVEL_QUIET,
    MIC_LEVEL_NORMAL,
    MIC_LEVEL_LOUD,
    MIC_LEVEL_VERY_LOUD
} MIC_LevelCategory_t;

/**
 * @brief VU meter structure
 */
typedef struct {
    float LevelsBars[MIC_EXAMPLE_VU_LEVELS];    /* VU meter bars (0.0-1.0) */
    float PeakHold;                             /* Peak hold value */
    uint32_t PeakHoldTime;                      /* Peak hold timer */
    float Smoothed;                             /* Smoothed level */
} MIC_VUMeter_t;

/**
 * @brief Spectral analysis structure
 */
typedef struct {
    float Bands[MIC_EXAMPLE_SPECTRAL_BANDS];    /* Frequency band levels */
    float Centroid;                             /* Spectral centroid */
    float Rolloff;                              /* Spectral rolloff */
    float Flux;                                 /* Spectral flux */
    float Brightness;                           /* High frequency content */
} MIC_SpectralAnalysis_t;

/**
 * @brief Voice activity detection structure
 */
typedef struct {
    bool IsVoiceActive;                         /* Current VAD state */
    float Confidence;                           /* VAD confidence (0.0-1.0) */
    uint32_t VoiceFrames;                       /* Voice frame count */
    uint32_t SilenceFrames;                     /* Silence frame count */
    float History[MIC_EXAMPLE_VAD_HISTORY_SIZE]; /* Recent frame energies */
    uint32_t HistoryIndex;                      /* History buffer index */
} MIC_VoiceActivity_t;

/**
 * @brief Audio compressor structure
 */
typedef struct {
    float Threshold;                            /* Compression threshold */
    float Ratio;                                /* Compression ratio */
    float Attack;                               /* Attack time constant */
    float Release;                              /* Release time constant */
    float GainReduction;                        /* Current gain reduction */
    float EnvelopeFollower;                     /* Envelope follower state */
} MIC_Compressor_t;

/**
 * @brief Echo effect structure
 */
typedef struct {
    int16_t *DelayBuffer;                       /* Delay line buffer */
    uint32_t DelaySize;                         /* Delay buffer size */
    uint32_t WriteIndex;                        /* Write pointer */
    uint32_t ReadIndex;                         /* Read pointer */
    float Feedback;                             /* Feedback amount */
    float Mix;                                  /* Wet/dry mix */
} MIC_Echo_t;

/**
 * @brief Example configuration structure
 */
typedef struct {
    uint32_t SampleRate;                        /* Sample rate */
    uint8_t Volume;                             /* Volume level */
    MIC_GainTypeDef Gain;                       /* Microphone gain */
    bool EnableVAD;                             /* Voice activity detection */
    bool EnableCompressor;                      /* Audio compressor */
    bool EnableEcho;                            /* Echo effect */
    bool EnableSpectralAnalysis;                /* Spectral analysis */
    bool EnableVUMeter;                         /* VU meter */
} MIC_ExampleConfig_t;

/**
 * @brief Example data structure
 */
typedef struct {
    MIC_HandleTypeDef *hmic;                    /* Microphone handle */
    MIC_ExampleConfig_t Config;                 /* Configuration */

    /* Audio buffers */
    int16_t AudioBuffer[MIC_EXAMPLE_BUFFER_SIZE];
    uint32_t BufferIndex;
    bool BufferFull;

    /* Analysis results */
    MIC_VUMeter_t VUMeter;
    MIC_SpectralAnalysis_t Spectral;
    MIC_VoiceActivity_t VAD;

    /* Audio effects */
    MIC_Compressor_t Compressor;
    MIC_Echo_t Echo;

    /* Statistics */
    uint32_t ProcessedFrames;
    uint32_t DroppedFrames;
    float MaxLevel;
    float MinLevel;
    uint32_t VoiceTime;
    uint32_t SilenceTime;
} MIC_ExampleData_t;

/* Exported function prototypes ---------------------------------------------*/

/* Basic examples */
MIC_StatusTypeDef MIC_Example_BasicRecording(void);
MIC_StatusTypeDef MIC_Example_VolumeControl(void);
MIC_StatusTypeDef MIC_Example_GainControl(void);
MIC_StatusTypeDef MIC_Example_AudioLevels(void);

/* Advanced examples */
MIC_StatusTypeDef MIC_Example_SpectralAnalysis(void);
MIC_StatusTypeDef MIC_Example_VoiceActivityDetection(void);
MIC_StatusTypeDef MIC_Example_NoiseGate(void);
MIC_StatusTypeDef MIC_Example_AudioEffects(void);

/* Real-time examples */
MIC_StatusTypeDef MIC_Example_RealTimeMonitoring(void);
MIC_StatusTypeDef MIC_Example_VUMeter(void);
MIC_StatusTypeDef MIC_Example_AudioAnalyzer(void);
MIC_StatusTypeDef MIC_Example_VoiceRecorder(void);

/* Utility functions */
MIC_StatusTypeDef MIC_Example_Init(MIC_ExampleData_t *example_data, MIC_HandleTypeDef *hmic);
MIC_StatusTypeDef MIC_Example_Configure(MIC_ExampleData_t *example_data, MIC_ExampleConfig_t *config);
MIC_StatusTypeDef MIC_Example_ProcessAudio(MIC_ExampleData_t *example_data, int16_t *buffer, uint32_t length);
MIC_StatusTypeDef MIC_Example_GetResults(MIC_ExampleData_t *example_data);

/* Analysis functions */
MIC_LevelCategory_t MIC_Example_GetLevelCategory(float level_db);
MIC_StatusTypeDef MIC_Example_UpdateVUMeter(MIC_VUMeter_t *vu_meter, float level);
MIC_StatusTypeDef MIC_Example_PerformSpectralAnalysis(int16_t *buffer, uint32_t length, MIC_SpectralAnalysis_t *spectral);
MIC_StatusTypeDef MIC_Example_DetectVoiceActivity(int16_t *buffer, uint32_t length, MIC_VoiceActivity_t *vad);

/* Audio effects functions */
MIC_StatusTypeDef MIC_Example_ApplyCompressor(int16_t *buffer, uint32_t length, MIC_Compressor_t *compressor);
MIC_StatusTypeDef MIC_Example_ApplyEcho(int16_t *buffer, uint32_t length, MIC_Echo_t *echo);
MIC_StatusTypeDef MIC_Example_ApplyNoiseReduction(int16_t *buffer, uint32_t length, float threshold);

/* Display functions */
void MIC_Example_DisplayVUMeter(MIC_VUMeter_t *vu_meter);
void MIC_Example_DisplaySpectrum(MIC_SpectralAnalysis_t *spectral);
void MIC_Example_DisplayVAD(MIC_VoiceActivity_t *vad);
void MIC_Example_DisplayStatistics(MIC_ExampleData_t *example_data);

/* Test functions */
MIC_StatusTypeDef MIC_Example_TestMicrophone(void);
MIC_StatusTypeDef MIC_Example_TestCalibration(void);
MIC_StatusTypeDef MIC_Example_TestFrequencyResponse(void);
MIC_StatusTypeDef MIC_Example_TestDynamicRange(void);

/* Calibration functions */
MIC_StatusTypeDef MIC_Example_CalibrateGain(MIC_HandleTypeDef *hmic, float target_level);
MIC_StatusTypeDef MIC_Example_CalibrateNoiseFloor(MIC_HandleTypeDef *hmic, float *noise_floor);
MIC_StatusTypeDef MIC_Example_MeasureFrequencyResponse(MIC_HandleTypeDef *hmic, float *response);

/* Callback functions */
void MIC_Example_RecordCompleteCallback(void);
void MIC_Example_ErrorCallback(void);
void MIC_Example_BufferFullCallback(void);
void MIC_Example_VoiceDetectedCallback(void);

/* Configuration helpers */
MIC_ExampleConfig_t MIC_Example_GetDefaultConfig(void);
MIC_StatusTypeDef MIC_Example_LoadConfig(const char *config_file, MIC_ExampleConfig_t *config);
MIC_StatusTypeDef MIC_Example_SaveConfig(const char *config_file, MIC_ExampleConfig_t *config);

/* Debug and logging */
void MIC_Example_PrintConfiguration(MIC_ExampleConfig_t *config);
void MIC_Example_PrintStatistics(MIC_ExampleData_t *example_data);
void MIC_Example_LogAudioData(int16_t *buffer, uint32_t length, const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* MIC_EXAMPLE_H */
