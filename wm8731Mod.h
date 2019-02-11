#ifndef WM8731_H
#define WM8731_H

#define   INP_BUFF            256
#define   BIT_LEN_PER_CHANNEL  32
#define   SSC_DMAC_RX_CH        3 // fifo=32
#define   SSC_DMAC_RX_ID        4 // ssc receive

#define   SSC_DMAC_TX_CH        5 // fifo=32
#define   SSC_DMAC_TX_ID        3 // ssc transit

#define WM8731_LLINEIN              0x00
#define WM8731_RLINEIN              0x01
#define WM8731_LHEADOUT             0x02
#define WM8731_RHEADOUT             0x03
#define WM8731_ANALOG               0x04
#define WM8731_DIGITAL              0x05
#define WM8731_POWERDOWN            0x06
#define WM8731_INTERFACE            0x07
#define WM8731_SAMPLING             0x08
#define WM8731_CONTROL              0x09
#define WM8731_RESET                0x0F     // writing 0 resets device

/* ----- Flags ----- */

#define WM8731_LLINEIN_LINVOL(n)    ((unsigned char)(n & 0x1f))       // Left channel line-input volume (0..31)
#define WM8731_LLINEIN_LINVOL_MASK  ((unsigned char)0xe0)
#define WM8731_LLINEIN_LINMUTE      ((unsigned char)0x80)             // Left line input mute to ADC
#define WM8731_LLINEIN_LRINBOTH     ((unsigned short)0x100)           // Left to Right Mic Control Join

#define WM8731_RLINEIN_RINVOL(n)    ((unsigned char)(n & 0x1f))       // Right channel line-input volume (0..31)
#define WM8731_RLINEIN_RINVOL_MASK  ((unsigned char)0xe0)
#define WM8731_RLINEIN_RINMUTE      ((unsigned char)0x80)             // Right line input mute to ADC
#define WM8731_RLINEIN_RLINBOTH     ((unsigned short)0x100)           // Right to Left Mic Control Join

#define WM8731_LHEADOUT_LHPVOL(n)   ((unsigned char)(n & 0x3f))       // Left Headphone Output Volume (0..127)
#define WM8731_LHEADOUT_LHPVOL_MASK ((unsigned char)0xc0)
#define WM8731_LHEADOUT_LZCEN       ((unsigned char)0x80)             // Left Channel Zero Cross Detect
#define WM8731_LHEADOUT_LRHPBOTH    ((unsigned short)0x100)           // Left to Right Headphone Control Join

#define WM8731_RHEADOUT_RHPVOL(n)   ((unsigned char)(n & 0x3f))       // Right Headphone Output Volume (0..127)
#define WM8731_RHEADOUT_RHPVOL_MASK ((unsigned char)0xc0)
#define WM8731_RHEADOUT_RZCEN       ((unsigned char)0x80)             // Right Channel Zero Cross Detect
#define WM8731_RHEADOUT_RLHPBOTH    ((unsigned short)0x100)           // Right to Left Headphone Control Join

#define WM8731_ANALOG_MICBOOST      ((unsigned char)0x01)             // Mic Input Level Boost
#define WM8731_ANALOG_MUTEMIC       ((unsigned char)0x02)             // Mic Input Mute to ADC
#define WM8731_ANALOG_INSEL         ((unsigned char)0x04)             // Mic Input Select to ADC (zero: select Line input to ADC)
#define WM8731_ANALOG_BYPASS        ((unsigned char)0x08)             // Bypass (line inputs summed to line out)
#define WM8731_ANALOG_DACSEL        ((unsigned char)0x10)             // DAC select (only send DAC output to line out)
#define WM8731_ANALOG_SIDETONE      ((unsigned char)0x20)             // Sidetone (mic inputs summed to line out)
#define WM8731_ANALOG_SIDEATT(n)    ((unsigned char)(n & 3)<<6)       // Sidetone Attenuation 0=-6dB, 1=-9dB, 2=-12dB, 3=-15dB

#define WM8731_DIGITAL_ADCHPD       ((unsigned char)0x01)             // ADC High Pass Filter Enable
#define WM8731_DIGITAL_DEEMP(n)     ((unsigned char)(n & 3)<<1)       // De-Emph: 0=disable, 1=32kHz, 2=44k1, 3=48k
#define WM8731_DIGITAL_DACMU        ((unsigned char)0x08)             // DAC Soft Mute (digital)
#define WM8731_DIGITAL_HPOR         ((unsigned char)0x10)             // Store DC offset when High Pass filter disabled

#define WM8731_POWERDOWN_LINEINPD   ((unsigned char)0x01)             // Line Input Power Down
#define WM8731_POWERDOWN_MICPD      ((unsigned char)0x02)             // Mic Input Power Down
#define WM8731_POWERDOWN_ADCPD      ((unsigned char)0x04)             // ADC Power Down
#define WM8731_POWERDOWN_DACPD      ((unsigned char)0x08)             // DAC Power Down
#define WM8731_POWERDOWN_OUTPD      ((unsigned char)0x10)             // Line Output Power Down
#define WM8731_POWERDOWN_OSCPD      ((unsigned char)0x20)             // Oscillator Power Down
#define WM8731_POWERDOWN_CLKOUTPD   ((unsigned char)0x40)             // CLKOUT Power Down
#define WM8731_POWERDOWN_POWEROFF   ((unsigned char)0x80)             // Power Off Device

#define WM8731_INTERFACE_FORMAT(n)  ((unsigned char)(n & 3))          // Format: 0=MSB-first RJ, 1=MSB-first LJ, 2=I2S, 3=DSP
#define WM8731_INTERFACE_WORDLEN(n) ((unsigned char)(n & 3)<<2)       // Word Length: 0=16 1=20 2=24 3=32
#define WM8731_INTERFACE_LRP        ((unsigned char)0x10)             // DACLRC phase control
#define WM8731_INTERFACE_LRSWAP     ((unsigned char)0x20)             // DAC Left Right Clock Swap
#define WM8731_INTERFACE_MASTER     ((unsigned char)0x40)             // Master/Slave Mode (1: codec is master)
#define WM8731_INTERFACE_BCLKINV    ((unsigned char)0x80)             // Bit Clock Invert

#define WM8731_SAMPLING_USBMODE     ((unsigned char)0x01)             // USB Mode Select
#define WM8731_SAMPLING_BOSR        ((unsigned char)0x02)             // Base OverSampling Rate
#define WM8731_SAMPLING_RATE(n)     ((unsigned char)(n & 0x0f)<<2)    // Sample Rate
#define WM8731_SAMPLING_CLKIDIV2    ((unsigned char)0x40)             // Core Clock Divider Select (0=MCLK, 1=MCLK/2)
#define WM8731_SAMPLING_CLKODIV2    ((unsigned char)0x80)             // CLKOUT Divider Select (0=MCLK, 1=MCLK/2)

#define WM8731_CONTROL_ACTIVE       ((unsigned char)1)

typedef enum {
      hz48000 = 0,  /* 48kHz from 12.288MHz MCLK */
      hz8000 = 3,   /* 8kHz from 12.288MHz MCLK */
      hz32000 = 6,  /* 32kHz from 12.288MHz MCLK */
      hz96000 = 7,  /* 96kHz from 12.288MHz MCLK */
      hz44100 = 8,  /* 44.1kHz from 11.2896MHz MCLK */
      hz88200 = 15  /* 88.2kHz from 11.2896MHz MCLK */
    } WM8731_sampling_rate;    

// METHODS
void set_buffer_addrs();
void set_reg( uint8_t reg_adr, uint16_t reg_val );
void DMAC_Handler();
void init_ssc();
void pio_B_SSC();
void init_dma();
void ssc_dma_cfg();
void prep_adc_dma(int32_t *buf);
void prep_adc_dma();
uint16_t prep_dac_dma();

extern int32_t  out[2][INP_BUFF];     
extern int32_t inp[2][INP_BUFF];
extern int32_t f_r[INP_BUFF];

#endif
