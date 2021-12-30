#ifndef _AUDIO_CONTROL_H
#define _AUDIO_CONTROL_H
//audio controller related

typedef enum
{
    SOURCE_MIC = 0,
    SOURCE_LINEIN
} INPUT_SOURCE;

typedef enum
{
    DEEMPHASIS_NONE,
    DEEMPHASIS_48K,
    DEEMPHASIS_44K1,
    DEEMPHASIS_32K
} DEEMPHASIS_TYPE;

typedef enum
{
    // MCLK = 18.432 (BOSR==1)
    RATE_ADC48K_DAC48K,
    RATE_ADC48K_DAC8K,
    RATE_ADC8K_DAC48K,
    RATE_ADC8K_DAC8K,
    RATE_ADC32K_DAC32K,
    RATE_ADC96K_DAC96K,
    RATE_ADC44K1_DAC44K1
} AUDIO_SAMPLE_RATE;

#define TRUE 1
#define FALSE 0

#define MAX_TRY_CNT 1024
#define LINEOUT_DEFUALT_VOL 0x79 // 0 dB

void oc_i2c_audio_init(void);
int oc_i2c_audio_wr_reg(int reg, int data);
int oc_i2c_audio_rd_reg(int reg);
int AUDIO_Init(void);
int AUDIO_InterfaceActive(int bActive);
int AUDIO_MicBoost(int bBoost);
int AUDIO_AdcEnableHighPassFilter(int bEnable);
int AUDIO_DacDeemphasisControl(int deemphasis_type);
int AUDIO_DacEnableSoftMute(int bEnable);
int AUDIO_MicMute(int bMute);
int AUDIO_LineInMute(int bMute);
int AUDIO_SetInputSource(int InputSource);
int AUDIO_SetSampleRate(int SampleRate);
int AUDIO_SetLineInVol(int l_vol, int r_vol);
int AUDIO_SetLineOutVol(int l_vol, int r_vol);
int AUDIO_EnableByPass(int bEnable);
int AUDIO_EnableSiteTone(int bEnable);
int AUDIO_DacFifoNotFull(void);
void AUDIO_DacFifoSetData(int ch_left, int ch_right);
void AUDIO_FifoClear(void);
int init_audio(void);
#endif //_AUDIO_CONTROL_H