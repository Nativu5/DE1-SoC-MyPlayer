#include <stdio.h>

#include "audio_control.h"
#include "hwlib.h"
#include "socal/socal.h"
#include "socal/hps.h"

#define AUDIO_DAC_LFIFO_PORT 0
#define AUDIO_DAC_RFIFO_PORT 1
#define AUDIO_ADC_LFIFO_PORT 2
#define AUDIO_ADC_RFIFO_PORT 3
#define AUDIO_CMD_PORT 4
#define AUDIO_STATUS_PORT 5

#define MASK_STATUS_DAC_FULL 0x01
#define MASK_STATUS_ADC_EMPTY 0x02

#define LINEOUT_DEFUALT_VOL 0x79 // 0 dB

extern volatile unsigned long *oc_i2c_audio_addr;
extern volatile unsigned long *audio_addr;

//i2c initial
void oc_i2c_audio_init(void)
{
    uint32_t this_data;
    //clock prescale register... set the frequency 100KHz for I2C ( 50M/(5*100K) = 99)
    alt_write_word(oc_i2c_audio_addr, 0x16);
    alt_write_word(oc_i2c_audio_addr + 1, 0x00);
    //oc_i2c_audio_addr[1] = 0x00;

    //enable the I2C core, but disable the IRQ
    oc_i2c_audio_addr[2] = 0x80;

    //this_data = oc_i2c_audio_addr[0];
    this_data = alt_read_word(oc_i2c_audio_addr);
    if ((this_data & 0x00ff) == 0x19)
    {
        printf("[INFO] Prescale low byte set is success! \n");
        this_data = alt_read_word(oc_i2c_audio_addr + 1);
        if ((this_data & 0x00ff) == 0x00)
            printf("[INFO] Prescale high byte set is success! \n");
        else
            printf("[INFO] Prescale high byte set is NG! \n");
    }
    else
        printf("[INFO] Prescale low byte set is NG! \n");

    this_data = oc_i2c_audio_addr[2];
    if ((this_data & 0x00ff) == 0x80)
        printf("[INFO] I2C core is enabled! \n");
    else
        printf("[INFO] I2C core is not enabled! \n");
}

//audio i2c write reg
//if dev_addr 7bit+write
int oc_i2c_audio_wr_reg(int reg, int data)
{
    uint32_t this_data;
    uint32_t reg_data = 0x0;
    int bsucces = 0;

    //set the tx reg audio chip dev address with write bit
    //oc_i2c_audio_addr[3] = 0x34;
    alt_write_word(oc_i2c_audio_addr + 3, 0x34);
    //set STA and WR bits(bit7 and bit4)
    //oc_i2c_audio_addr[4] = 0x90;
    alt_write_word(oc_i2c_audio_addr + 4, 0x90);
    //oc_i2c_audio_addr[4] = 0x10;
    //wait TIP bit go to 0 to end Tx
    this_data = alt_read_word(oc_i2c_audio_addr + 4);
    while (this_data & 0x02)
    {
        this_data = alt_read_word(oc_i2c_audio_addr + 4);
    }
    //wait the rx ACK signal 0-valid
    this_data = alt_read_word(oc_i2c_audio_addr + 4);
    while (this_data & 0x80)
    {
        this_data = alt_read_word(oc_i2c_audio_addr + 4);
    }
    //   printf("\n receive ACK-device address! \n");

    //set the txr reg data with reg address + 1 data MSB
    reg_data = (reg << 1) & 0xFE;
    reg_data |= ((data >> 8) & 0x01);
    // oc_i2c_audio_addr[3] = (reg_data & 0xff);
    alt_write_word(oc_i2c_audio_addr + 3, reg_data & 0xff);
    //set WR bits(bit4)
    // oc_i2c_audio_addr[4] = 0x10;
    alt_write_word(oc_i2c_audio_addr + 4, 0x10);
    //wait TIP bit go to 0 to end Tx
    this_data = alt_read_word(oc_i2c_audio_addr + 4);
    while (this_data & 0x02)
    {
        this_data = alt_read_word(oc_i2c_audio_addr + 4);
    }
    //wait the rx ACK signal 0-valid
    this_data = alt_read_word(oc_i2c_audio_addr + 4);
    while (this_data & 0x80)
    {
        this_data = alt_read_word(oc_i2c_audio_addr + 4);
    }
    //   printf("\n receive ACK-reg address! \n");

    //set the txr reg data with the other data 8 bit LSB
    //oc_i2c_audio_addr[3] = (data & 0xff);
    alt_write_word(oc_i2c_audio_addr + 3, data & 0xff);
    //set STO and WR bits(bit7 and bit4)
    //oc_i2c_audio_addr[4] = 0x10;
    alt_write_word(oc_i2c_audio_addr + 4, 0x10);
    //wait TIP bit go to 0 to end Tx
    this_data = alt_read_word(oc_i2c_audio_addr + 4);
    while (this_data & 0x02)
    {
        this_data = alt_read_word(oc_i2c_audio_addr + 4);
    }
    //wait the rx ACK signal 0-valid
    this_data = alt_read_word(oc_i2c_audio_addr + 4);
    while (this_data & 0x80)
    {
        this_data = alt_read_word(oc_i2c_audio_addr + 4);
    }
    //oc_i2c_audio_addr[4] = 0x40;
    alt_write_word(oc_i2c_audio_addr + 4, 0x40);

    // printf("[INFO] Wr_reg receive ACK-data! \n");
    bsucces = 1;

    return bsucces;
}

int oc_i2c_audio_rd_reg(int reg)
{
    uint32_t this_data;
    uint32_t reg_data = 0x0;
    uint32_t data = 0x0;
    int bsucces = 0;

    //set the tx reg audio chip dev address with write bit
    // oc_i2c_audio_addr[3] = 0x34;
    alt_write_word(oc_i2c_audio_addr + 3, 0x34);

    //set STA and WR bits(bit7 and bit4)
    //oc_i2c_audio_addr[4] = 0x90;
    alt_write_word(oc_i2c_audio_addr + 4, 0x90);
    //oc_i2c_audio_addr[4] = 0x10;
    //wait TIP bit go to 0 to end Tx
    this_data = alt_read_word(oc_i2c_audio_addr + 4);
    while (this_data & 0x02)
    {
        this_data = alt_read_word(oc_i2c_audio_addr + 4);
    }
    //wait the rx ACK signal 0-valid
    this_data = alt_read_word(oc_i2c_audio_addr + 4);
    while (this_data & 0x80)
    {
        this_data = alt_read_word(oc_i2c_audio_addr + 4);
    }
    // printf("\n read receive ACK-device address! \n");

    //set the txr reg data with reg address + 0
    reg_data = (reg << 1) & 0xFE;
    oc_i2c_audio_addr[3] = (reg_data & 0xff);
    //set WR bits(bit4)
    //oc_i2c_audio_addr[4] = 0x10;
    alt_write_word(oc_i2c_audio_addr + 4, 0x10);
    //wait TIP bit go to 0 to end Tx
    this_data = alt_read_word(oc_i2c_audio_addr + 4);
    while (this_data & 0x02)
    {
        this_data = alt_read_word(oc_i2c_audio_addr + 4);
    }
    //wait the rx ACK signal 0-valid
    this_data = alt_read_word(oc_i2c_audio_addr + 4);
    while (this_data & 0x80)
    {
        this_data = alt_read_word(oc_i2c_audio_addr + 4);
    }
    //  printf("\n read receive ACK-reg address! \n");

    //read
    //set the tx reg audio chip dev address with read bit 1
    //oc_i2c_audio_addr[3] = 0x35;
    alt_write_word(oc_i2c_audio_addr + 3, 0x35);

    //set STA and WR bits(bit7 and bit4)
    //oc_i2c_audio_addr[4] = 0x90;
    alt_write_word(oc_i2c_audio_addr + 4, 0x90);
    // oc_i2c_audio_addr[4] = 0x10;
    //wait TIP bit go to 0 to end Tx
    this_data = alt_read_word(oc_i2c_audio_addr + 4);
    while (this_data & 0x02)
    {
        this_data = alt_read_word(oc_i2c_audio_addr + 4);
    }
    //wait the rx ACK signal 0-valid
    this_data = alt_read_word(oc_i2c_audio_addr + 4);
    while (this_data & 0x80)
    {
        this_data = alt_read_word(oc_i2c_audio_addr + 4);
    }
    //   printf("\n read receive ACK-device address(read)! \n");

    //set the RD and ACK bit(bit5 and bit3)
    //oc_i2c_audio_addr[4] = 0x20;
    alt_write_word(oc_i2c_audio_addr + 4, 0x20);
    //wait TIP bit go to 0 to end Tx
    this_data = alt_read_word(oc_i2c_audio_addr + 4);
    while (this_data & 0x02)
    {
        this_data = alt_read_word(oc_i2c_audio_addr + 4);
    }

    // printf("\n read receive ACK-device address(read)! \n");

    // oc_i2c_audio_addr[4] = 0x00;
    //read the rxr data
    data = alt_read_word(oc_i2c_audio_addr + 3) & 0xff;

    // set the STO ,RD and NACK bit(bit 6,bit5 and bit3)
    oc_i2c_audio_addr[4] = 0x28;
    // alt_write_word(oc_i2c_audio_addr+4, 0x20);

    //wait TIP bit go to 0 to end Tx
    this_data = alt_read_word(oc_i2c_audio_addr + 4);
    while (this_data & 0x02)
    {
        this_data = alt_read_word(oc_i2c_audio_addr + 4);
    }
    // printf("\n read receive ACK-device address(read)! \n");
    //oc_i2c_audio_addr[4] = 0x04;

    //alt_write_word(oc_i2c_audio_addr+4, 0x08);
    // alt_write_word(oc_i2c_audio_addr+4, 0x08);
    // alt_write_word(oc_i2c_audio_addr+4, 0x40);

    //oc_i2c_audio_addr[4] = 0x40;

    //read the rxr data
    this_data = (alt_read_word(oc_i2c_audio_addr + 3)) & 0xff;
    // printf("this data = %Xh\r\n", this_data);
    data |= ((this_data << 8) & 0x100);

    alt_write_word(oc_i2c_audio_addr + 4, 0x40);

    printf("[INFO] read audio reg[%02d] = %04Xh\r\n", reg, data);

    bsucces = 1;

    return bsucces;
}

int AUDIO_InterfaceActive(int bActive)
{
    int bSuccess;
    bSuccess = oc_i2c_audio_wr_reg(9, bActive ? 0x0001 : 0x0000);
    printf("[INFO] AUDIO_InterfaceActive... %s\r\n", bSuccess ? "success" : "fail");
    return bSuccess;
}

int AUDIO_MicBoost(int bBoost)
{
    int bSuccess;
    uint32_t control;
    control = 0x0014;
    if (bBoost)
        control |= 0x0001;
    else
        control &= 0xFFFE;
    bSuccess = oc_i2c_audio_wr_reg(0b0000100, control); // Left Line In: set left line in volume
    printf("[INFO] AUDIO_MicBoost... %s\r\n", bSuccess ? "success" : "fail");
    return bSuccess;
}

int AUDIO_AdcEnableHighPassFilter(int bEnable)
{
    int bSuccess;
    uint32_t control;
    control = 0x0000;
    if (bEnable)
        control &= 0xFFFE;
    else
        control |= 0x0001;
    bSuccess = oc_i2c_audio_wr_reg(5, control); // Left Line In: set left line in volume

    printf("[INFO] AUDIO_AdcEnableHighPassFilter... %s\r\n", bSuccess ? "success" : "fail");
    return bSuccess;
}

int AUDIO_DacDeemphasisControl(int deemphasis_type)
{
    int bSuccess;
    uint32_t control;
    control = 0x0000;
    control &= 0xFFF9;
    switch (deemphasis_type)
    {
    case DEEMPHASIS_48K:
        control |= ((0x03) << 1);
        break;
    case DEEMPHASIS_44K1:
        control |= ((0x02) << 1);
        break;
    case DEEMPHASIS_32K:
        control |= ((0x01) << 1);
        break;
    }
    bSuccess = oc_i2c_audio_wr_reg(5, control); // Left Line In: set left line in volume
    printf("[INFO] AUDIO_DacDeemphasisControl... %s\r\n", bSuccess ? "success" : "fail");

    return bSuccess;
}

int AUDIO_DacEnableSoftMute(int bEnable)
{
    int bSuccess;
    uint32_t control;
    uint32_t mask;
    control = 0x0000;
    mask = 0x01 << 3;
    if (bEnable)
        control |= mask;
    else
        control &= ~mask;
    bSuccess = oc_i2c_audio_wr_reg(5, control); // Left Line In: set left line in volume
    printf("[INFO] AUDIO_DacEnableSoftMute... %s\r\n", bSuccess ? "success" : "fail");

    return bSuccess;
}

int AUDIO_MicMute(int bMute)
{
    int bSuccess;
    uint32_t control;
    uint32_t mask;
    control = 0x014;
    mask = 0x01 << 1;
    if (bMute)
        control |= mask;
    else
        control &= ~mask;
    bSuccess = oc_i2c_audio_wr_reg(4, control); // Left Line In: set left line in volume
    printf("[INFO] AUDIO_MicMute... %s\r\n", bSuccess ? "success" : "fail");
    return bSuccess;
}

int AUDIO_LineInMute(int bMute)
{
    int bSuccess;
    uint32_t control_l, control_r;
    uint32_t mask;
    control_l = 0x0017;
    control_r = 0x0017;
    mask = 0x01 << 7;
    if (bMute)
    {
        control_l |= mask;
        control_r |= mask;
    }
    else
    {
        control_l &= ~mask;
        control_r &= ~mask;
    }
    bSuccess = oc_i2c_audio_wr_reg(0, control_l); // Left Line In: set left line in volume

    printf("[INFO] AUDIO_LineInMute...\r\n");

    if (bSuccess)
        bSuccess = oc_i2c_audio_wr_reg(1, control_r); // Left Line In: set left line in volume

    printf("[INFO] AUDIO_LineInMute...%s\r\n", bSuccess ? "success" : "fail");
    return bSuccess;
}

int AUDIO_SetInputSource(int InputSource)
{
    int bSuccess;
    uint32_t control;
    uint32_t mask;
    control = 0x0014;
    mask = 0x01 << 2;
    if (InputSource == SOURCE_MIC)
        control |= mask;
    else
        control &= ~mask;
    bSuccess = oc_i2c_audio_wr_reg(4, control); // Left Line In: set left line in volume
    printf("[INFO] AUDIO_SetInputSource... %s\r\n", bSuccess ? "success" : "fail");
    return bSuccess;
}
//audio controller related

// See datasheet page 39
int AUDIO_SetSampleRate(int SampleRate)
{
    int bSuccess;
    uint32_t control;
    control = 0;

    switch (SampleRate)
    {
        // MCLK = 18.432
    case RATE_ADC48K_DAC48K:
        control = (0x0) << 2;
        break;
    case RATE_ADC48K_DAC8K:
        control = (0x1) << 2;
        break;
    case RATE_ADC8K_DAC48K:
        control = (0x2) << 2;
        break;
    case RATE_ADC8K_DAC8K:
        control = (0x3) << 2;
        break;
    case RATE_ADC32K_DAC32K:
        control = (0x6) << 2;
        break;
    case RATE_ADC96K_DAC96K:
        control = (0x7) << 2;
        break;
    case RATE_ADC44K1_DAC44K1:
        control = (0x8) << 2;
        break;

        //  case RATE_ADC44K1_DAC8K: control = (0x9) << 2; break;
        //  case RATE_ADC8K_DAC44K1: control = (0xA) << 2; break;
    }
    control |= 0x02; // BOSR=1 (384fs = 384*48k = 18.432M)

    bSuccess = oc_i2c_audio_wr_reg(8, control); // Left Line In: set left line in volume

    printf("[INFO] AUDIO_SetSampleRate... %s\r\n", bSuccess ? "success" : "fail");
    return bSuccess;
}

int AUDIO_SetLineInVol(int l_vol, int r_vol)
{
    int bSuccess;
    uint32_t control;

    // left
    control = 0x0017;
    control &= 0xFFC0;
    control += l_vol & 0x3F;
    bSuccess = oc_i2c_audio_wr_reg(0, control);

    usleep(10 * 1000);

    if (bSuccess)
    {
        // right
        control = 0x0017;
        control &= 0xFFC0;
        control += r_vol & 0x3F;
        bSuccess = oc_i2c_audio_wr_reg(1, control);
    }

    printf("[INFO] set Line-In vol(%d,%d) %s\r\n", l_vol, r_vol, bSuccess ? "success" : "fail");
    return bSuccess;
}

int AUDIO_SetLineOutVol(int l_vol, int r_vol)
{
    int bSuccess;
    uint32_t control;

    // left
    control = 0x005B;
    control &= 0xFF80;
    control += l_vol & 0x7F;
    bSuccess = oc_i2c_audio_wr_reg(2, control);

    usleep(10 * 1000);

    if (bSuccess)
    {
        // right
        control = 0x005B;
        control &= 0xFF80;
        control += r_vol & 0x7F;
        bSuccess = oc_i2c_audio_wr_reg(3, control);
    }

    printf("[INFO] set Line-Out vol(%x,%x) %s\r\n", l_vol, r_vol, bSuccess ? "success" : "fail");

    return bSuccess;
}

int AUDIO_EnableByPass(int bEnable)
{
    int bSuccess;
    uint32_t control;
    uint32_t mask;
    control = 0x0014;
    mask = 0x01 << 3;
    if (bEnable)
        control |= mask;
    else
        control &= ~mask;
    bSuccess = oc_i2c_audio_wr_reg(4, control);
    return bSuccess;
}

int AUDIO_EnableSiteTone(int bEnable)
{
    int bSuccess;
    uint32_t control;
    uint32_t mask;
    control = 0x0014;
    mask = 0x01 << 5;
    if (bEnable)
        control |= mask;
    else
        control &= ~mask;
    bSuccess = oc_i2c_audio_wr_reg(4, control);
    return bSuccess;
}

// check whether the dac-fifo is full.
int AUDIO_DacFifoNotFull(void)
{
    int bReady;

    bReady = ((alt_read_word(audio_addr + AUDIO_STATUS_PORT) & MASK_STATUS_DAC_FULL) ? 0x1 : 0x0) ? 0x0 : 0x1;
    return bReady;
}

// call AUDIO_PlayIsReady to make sure the fifo is not full before call this function
void AUDIO_DacFifoSetData(int ch_left, int ch_right)
{

    alt_write_word(audio_addr + AUDIO_DAC_LFIFO_PORT, ch_left & 0xFFFF);
    alt_write_word(audio_addr + AUDIO_DAC_RFIFO_PORT, ch_right & 0xFFFF);

    // AUDIO_DAC_WRITE_L(ch_left);
    // AUDIO_DAC_WRITE_R(ch_right);
}

void AUDIO_FifoClear(void)
{
    alt_write_word(audio_addr + AUDIO_CMD_PORT, 0x01);
}

int init_audio(void)
{
    printf("[INFO] Setting up audio chip...");
    int bSuccess = 1;

    // setting up audio chip
    if (bSuccess)
        bSuccess = oc_i2c_audio_wr_reg(15, 0x0000); // reset
    usleep(10 * 1000);
    if (bSuccess)
        bSuccess = oc_i2c_audio_wr_reg(9, 0x0000); // inactive interface
    usleep(10 * 1000);
    if (bSuccess)
        bSuccess = oc_i2c_audio_wr_reg(0, 0x0017); // Left Line In: set left line in volume
    usleep(10 * 1000);
    if (bSuccess)
        bSuccess = oc_i2c_audio_wr_reg(1, 0x0017); // Right Line In: set right line in volume
    usleep(10 * 1000);
    if (bSuccess)
        bSuccess = oc_i2c_audio_wr_reg(2, 0x005B); // Left Headphone Out: set left line out volume
    usleep(10 * 1000);
    if (bSuccess)
        bSuccess = oc_i2c_audio_wr_reg(3, 0x005B); // Right Headphone Out: set right line out volume
    usleep(10 * 1000);
    if (bSuccess)
        bSuccess = oc_i2c_audio_wr_reg(4, 0x0015 | 0x20 | 0x08 | 0x01); // Analogue Audio Path Control: set mic as input and enable dac
    usleep(10 * 1000);
    if (bSuccess)
        bSuccess = oc_i2c_audio_wr_reg(5, 0x0000); // Digital Audio Path Control: disable soft mute
    usleep(10 * 1000);
    if (bSuccess)
        bSuccess = oc_i2c_audio_wr_reg(6, 0x0000); // power down control: power on all
    usleep(10 * 1000);
    if (bSuccess)
        bSuccess = oc_i2c_audio_wr_reg(7, 0x0042); // I2S, iwl=16-bits, Enable Master Mode
    usleep(10 * 1000);
    if (bSuccess)
        bSuccess = oc_i2c_audio_wr_reg(8, 0x0002); // Normal, Base OVer-Sampleing Rate 384 fs (BOSR=1)
    usleep(10 * 1000);
    // if (bSuccess)
    //     bSuccess = oc_i2c_audio_wr_reg(16, 0x007B); //ALC CONTROL 1
    // usleep(10 * 1000);
    // if (bSuccess)
    //     bSuccess = oc_i2c_audio_wr_reg(17, 0x0032); //ALC CONTROL 2
    // usleep(10 * 1000);
    // if (bSuccess)
    //     bSuccess = oc_i2c_audio_wr_reg(18, 0x0000); //NOISE GATE
    // usleep(10 * 1000);
    if (bSuccess)
        bSuccess = oc_i2c_audio_wr_reg(9, 0x0001); // active interface

    printf("%s\n", bSuccess ? "success" : "fail");

    AUDIO_InterfaceActive(0x0);
    usleep(10 * 1000);

    AUDIO_DacEnableSoftMute(0x1);
    usleep(10 * 1000);
    AUDIO_MicMute(0x1);
    usleep(10 * 1000);
    AUDIO_LineInMute(0x1);
    usleep(10 * 1000);
    AUDIO_DacEnableSoftMute(0x0);
    usleep(10 * 1000);

    AUDIO_SetLineOutVol(LINEOUT_DEFUALT_VOL, LINEOUT_DEFUALT_VOL); // max 7F, min: 30, 0x79: 0 db
    usleep(10 * 1000);
    AUDIO_DacEnableSoftMute(0x0);
    usleep(10 * 1000);

    AUDIO_FifoClear();
    usleep(10 * 1000);

    AUDIO_SetSampleRate(RATE_ADC44K1_DAC44K1); // Default
    usleep(10 * 1000);

    AUDIO_InterfaceActive(0x1);

    usleep(10 * 1000);

    return bSuccess;
}
