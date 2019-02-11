#include "Arduino.h"
#include <Wire.h>
#include "wm8731Mod.h"

static const uint8_t  wm8731_adr   =  0x1a;     // 0011010

typedef struct {
  uint32_t ul_source_addr;      
  uint32_t ul_destination_addr; 
  uint32_t ul_ctrlA;            
  uint32_t ul_ctrlB;            
  uint32_t ul_descriptor_addr;  
} dma_transfer_descriptor_t;

dma_transfer_descriptor_t        desc[4];  
   
boolean        end_input         = false;  
uint32_t       adres_reg         =     0;         
uint32_t       value_reg         =     0;         
uint8_t       bufer_dat[3]      = {  0};         
volatile uint32_t error_i2c         =     0;  

volatile int16_t flag_adcv = 0;
volatile int16_t flag_dacv = 0;

volatile int16_t adcdac_first = 0;

int32_t inp[2][INP_BUFF] =  { 0}; // DMA likes ping-pongs buffer
int32_t f_r[INP_BUFF] =  { 0};
int32_t  lrc[2][INP_BUFF /2] =  { 0};
int32_t  out[2][INP_BUFF] =  { 0};     

uint8_t print_inp  = 0;     // print switch
uint8_t print_out  = 0;
uint8_t distr_lvl  = 0;

uint32_t dac_addr0,  dac_addr1,  adc_addr0,  adc_addr1;
const    uint32_t   msk_addr         =  ~(INP_BUFF -1);


void set_buffer_addrs()
{
  adc_addr0 = msk_addr& ((uint32_t) inp[0]);
  adc_addr1 = msk_addr& ((uint32_t) inp[1]);

  dac_addr0 = msk_addr& ((uint32_t) out[0]);
  dac_addr1 = msk_addr& ((uint32_t) out[1]);
}


void set_reg( uint8_t reg_adr, uint16_t reg_val )
{
  uint8_t uc_temp_data[2];
  uint16_t us_temp;
  
  us_temp = (reg_adr << 9) | (reg_val & 0x1ff);
  uc_temp_data[0] = (us_temp & 0xff00) >> 8;
  uc_temp_data[1] = us_temp & 0xff;
  
  Wire.beginTransmission( wm8731_adr ); // 1A ili 1B
  Wire.write(uc_temp_data[0]);
  Wire.write(uc_temp_data[1]);
  Wire.endTransmission();
}



void ssc_dma_cfg()
{
  desc[0].ul_source_addr      = (uint32_t)(&SSC->SSC_RHR);
  desc[0].ul_destination_addr = (uint32_t) inp[0];
  desc[0].ul_ctrlA = DMAC_CTRLA_BTSIZE(INP_BUFF) |/* Set Buffer Transfer Size */
                     DMAC_CTRLA_SRC_WIDTH_WORD |  /* Source transfer size is set to 32-bit width */
                     DMAC_CTRLA_DST_WIDTH_WORD;   /* Destination transfer size is set to 32-bit width */

  desc[0].ul_ctrlB = DMAC_CTRLB_SRC_DSCR_FETCH_DISABLE |
                     DMAC_CTRLB_DST_DSCR_FETCH_FROM_MEM |           
                     DMAC_CTRLB_FC_PER2MEM_DMA_FC |                  
                     DMAC_CTRLB_SRC_INCR_FIXED |              
                     DMAC_CTRLB_DST_INCR_INCREMENTING;       
  desc[0].ul_descriptor_addr = (uint32_t) &desc[1]; 

  desc[1].ul_source_addr      = (uint32_t)(&SSC->SSC_RHR);
  desc[1].ul_destination_addr = (uint32_t) inp[1];
  desc[1].ul_ctrlA = DMAC_CTRLA_BTSIZE(INP_BUFF) |    
                     DMAC_CTRLA_SRC_WIDTH_WORD |                  
                     DMAC_CTRLA_DST_WIDTH_WORD;                   

  desc[1].ul_ctrlB = DMAC_CTRLB_SRC_DSCR_FETCH_DISABLE | 
                     DMAC_CTRLB_DST_DSCR_FETCH_FROM_MEM |            
                     DMAC_CTRLB_FC_PER2MEM_DMA_FC |                  
                     DMAC_CTRLB_SRC_INCR_FIXED |             
                     DMAC_CTRLB_DST_INCR_INCREMENTING;           
  desc[1].ul_descriptor_addr = (uint32_t) &desc[0];       
  
  DMAC->DMAC_CH_NUM[SSC_DMAC_RX_CH].DMAC_SADDR = desc[0].ul_source_addr;
  DMAC->DMAC_CH_NUM[SSC_DMAC_RX_CH].DMAC_DADDR = desc[0].ul_destination_addr;
  DMAC->DMAC_CH_NUM[SSC_DMAC_RX_CH].DMAC_CTRLA = desc[0].ul_ctrlA;
  DMAC->DMAC_CH_NUM[SSC_DMAC_RX_CH].DMAC_CTRLB = desc[0].ul_ctrlB;
  DMAC->DMAC_CH_NUM[SSC_DMAC_RX_CH].DMAC_DSCR  = desc[0].ul_descriptor_addr;

  DMAC->DMAC_CHER = DMAC_CHER_ENA0 << SSC_DMAC_RX_CH;
  ssc_enable_rx(SSC);

// transmit
  desc[2].ul_source_addr      = (uint32_t) out[0];
  desc[2].ul_destination_addr = (uint32_t) (&SSC->SSC_THR);
  desc[2].ul_ctrlA = DMAC_CTRLA_BTSIZE(INP_BUFF) |
                     DMAC_CTRLA_SRC_WIDTH_WORD |  
                     DMAC_CTRLA_DST_WIDTH_WORD;   

  desc[2].ul_ctrlB = DMAC_CTRLB_SRC_DSCR_FETCH_FROM_MEM | 
                     DMAC_CTRLB_DST_DSCR_FETCH_DISABLE |       
                     DMAC_CTRLB_FC_MEM2PER_DMA_FC |                  
                     DMAC_CTRLB_SRC_INCR_INCREMENTING |              
                     DMAC_CTRLB_DST_INCR_FIXED;                        
  desc[2].ul_descriptor_addr = (uint32_t) &desc[3];       

  desc[3].ul_source_addr      = (uint32_t) out[1];
  desc[3].ul_destination_addr = (uint32_t) (&SSC->SSC_THR);
  desc[3].ul_ctrlA = DMAC_CTRLA_BTSIZE(INP_BUFF) | 
                     DMAC_CTRLA_SRC_WIDTH_WORD |               
                     DMAC_CTRLA_DST_WIDTH_WORD;                

  desc[3].ul_ctrlB = DMAC_CTRLB_SRC_DSCR_FETCH_FROM_MEM | 
                     DMAC_CTRLB_DST_DSCR_FETCH_DISABLE |       
                     DMAC_CTRLB_FC_MEM2PER_DMA_FC |                  
                     DMAC_CTRLB_SRC_INCR_INCREMENTING |              
                     DMAC_CTRLB_DST_INCR_FIXED;                        
  desc[3].ul_descriptor_addr = (uint32_t) &desc[2];       

  DMAC->DMAC_CH_NUM[SSC_DMAC_TX_CH].DMAC_SADDR = desc[2].ul_source_addr;
  DMAC->DMAC_CH_NUM[SSC_DMAC_TX_CH].DMAC_DADDR = desc[2].ul_destination_addr;
  DMAC->DMAC_CH_NUM[SSC_DMAC_TX_CH].DMAC_CTRLA = desc[2].ul_ctrlA;
  DMAC->DMAC_CH_NUM[SSC_DMAC_TX_CH].DMAC_CTRLB = desc[2].ul_ctrlB;
  DMAC->DMAC_CH_NUM[SSC_DMAC_TX_CH].DMAC_DSCR  = desc[2].ul_descriptor_addr;

  DMAC->DMAC_CHER = DMAC_CHER_ENA0 << SSC_DMAC_TX_CH;
  ssc_enable_tx(SSC);
}

void init_dma() {
  uint32_t ul_cfg;

  pmc_enable_periph_clk(ID_DMAC);
  DMAC->DMAC_EN &= (~DMAC_EN_ENABLE);
  DMAC->DMAC_GCFG = (DMAC->DMAC_GCFG & (~DMAC_GCFG_ARB_CFG)) | DMAC_GCFG_ARB_CFG_ROUND_ROBIN;
  DMAC->DMAC_EN = DMAC_EN_ENABLE;

  ul_cfg = 0;
  ul_cfg = DMAC_CFG_SRC_PER(SSC_DMAC_RX_ID) |
           DMAC_CFG_SRC_H2SEL |
           DMAC_CFG_SOD_DISABLE | //SOD: Stop On Done
           DMAC_CFG_FIFOCFG_ALAP_CFG;

  DMAC->DMAC_CH_NUM[SSC_DMAC_RX_CH].DMAC_CFG = ul_cfg;
  DMAC->DMAC_CHDR = DMAC_CHDR_DIS0 << SSC_DMAC_RX_CH;

//transmit
  ul_cfg = 0;
  ul_cfg = DMAC_CFG_DST_PER(SSC_DMAC_TX_ID) |
           DMAC_CFG_DST_H2SEL |
           DMAC_CFG_SOD_DISABLE | //SOD: Stop On Done
           DMAC_CFG_FIFOCFG_ALAP_CFG;

  DMAC->DMAC_CH_NUM[SSC_DMAC_TX_CH].DMAC_CFG = ul_cfg;
  DMAC->DMAC_CHDR = DMAC_CHDR_DIS0 << SSC_DMAC_TX_CH;
//

  NVIC_EnableIRQ(DMAC_IRQn);

  DMAC->DMAC_EBCIER = DMAC_EBCIER_BTC0 << SSC_DMAC_RX_CH;

  DMAC->DMAC_EBCIER = DMAC_EBCIER_BTC0 << SSC_DMAC_TX_CH;

  DMAC->DMAC_EBCISR;
}

void init_ssc() {
  clock_opt_t        rx_clk_option;
  data_frame_opt_t   rx_data_frame_option;
  clock_opt_t        tx_clk_option;
  data_frame_opt_t   tx_data_frame_option;

  memset((uint8_t *)&rx_clk_option,        0, sizeof(clock_opt_t));
  memset((uint8_t *)&rx_data_frame_option, 0, sizeof(data_frame_opt_t));
  memset((uint8_t *)&tx_clk_option,        0, sizeof(clock_opt_t));
  memset((uint8_t *)&tx_data_frame_option, 0, sizeof(data_frame_opt_t));

  pmc_enable_periph_clk(ID_SSC);
  ssc_reset(SSC);

  rx_clk_option.ul_cks               = SSC_RCMR_CKS_RK;
  rx_clk_option.ul_cko               = SSC_RCMR_CKO_NONE;
  rx_clk_option.ul_cki               = SSC_RCMR_CKI;
  //1 = The data inputs (Data and Frame Sync signals) 
  //    are sampled on Receive Clock rising edge.  
  rx_clk_option.ul_ckg               = SSC_RCMR_CKG_NONE; // bylo 0;
  rx_clk_option.ul_start_sel         = SSC_RCMR_START_RF_RISING;
  rx_clk_option.ul_period            = 0;
  rx_clk_option.ul_sttdly            = 1;

  rx_data_frame_option.ul_datlen     = BIT_LEN_PER_CHANNEL - 1;
  rx_data_frame_option.ul_msbf       = SSC_RFMR_MSBF;
  rx_data_frame_option.ul_datnb      = 1;//stereo
  rx_data_frame_option.ul_fslen      = 0;
  rx_data_frame_option.ul_fslen_ext  = 0;
  rx_data_frame_option.ul_fsos       = SSC_RFMR_FSOS_NONE;
  rx_data_frame_option.ul_fsedge     = SSC_RFMR_FSEDGE_POSITIVE;

  ssc_set_receiver(SSC, &rx_clk_option, &rx_data_frame_option);
  ssc_disable_rx(SSC);
//  ssc_disable_interrupt(SSC, 0xFFFFFFFF);

  tx_clk_option.ul_cks               = SSC_TCMR_CKS_RK;
  tx_clk_option.ul_cko               = SSC_TCMR_CKO_NONE;
  tx_clk_option.ul_cki               = 0; //example Atmel. bylo SSC_TCMR_CKI;
  //1 = The data outputs (Data and Frame Sync signals) 
  //    are shifted out on Transmit Clock rising edge.
  tx_clk_option.ul_ckg               = SSC_TCMR_CKG_NONE; // bylo 0.
  tx_clk_option.ul_start_sel         = SSC_TCMR_START_RF_RISING;
  tx_clk_option.ul_period            = 0;
  tx_clk_option.ul_sttdly            = 1;

  tx_data_frame_option.ul_datlen     = BIT_LEN_PER_CHANNEL - 1;
  tx_data_frame_option.ul_msbf       = SSC_TFMR_MSBF;
  tx_data_frame_option.ul_datnb      = 1;
  tx_data_frame_option.ul_fslen      = 0; // :fsden=0
  tx_data_frame_option.ul_fslen_ext  = 0;
  tx_data_frame_option.ul_fsos       = SSC_TFMR_FSOS_NONE; //input-slave
  tx_data_frame_option.ul_fsedge     = SSC_TFMR_FSEDGE_POSITIVE;

  ssc_set_transmitter(SSC, &tx_clk_option, &tx_data_frame_option);
  ssc_disable_tx(SSC);
  ssc_disable_interrupt(SSC, 0xFFFFFFFF);
}

void pio_B_SSC(void) 
{
// DUE: PA15(B)-D24, PA16(B)-A0, PA14(B)-D23 = DACLRC, DACDAT, BCLK  
  PIOA->PIO_PDR   = PIO_PA14B_TK; 
  PIOA->PIO_IDR   = PIO_PA14B_TK; 
  PIOA->PIO_ABSR |= PIO_PA14B_TK; 
  
  PIOA->PIO_PDR   = PIO_PA15B_TF; 
  PIOA->PIO_IDR   = PIO_PA15B_TF; 
  PIOA->PIO_ABSR |= PIO_PA15B_TF; 

  PIOA->PIO_PDR   = PIO_PA16B_TD; 
  PIOA->PIO_IDR   = PIO_PA16B_TD; 
  PIOA->PIO_ABSR |= PIO_PA16B_TD; 
}

void prep_adc_dma(int32_t *buf)
{
  uint32_t adc_addr = DMAC->DMAC_CH_NUM[SSC_DMAC_RX_CH].DMAC_DADDR;
           adc_addr &= msk_addr;
           
  uint16_t indx_a = 0;
           if(adc_addr == adc_addr0) indx_a = 1;
           if(adc_addr == adc_addr1) indx_a = 0;

  for( uint16_t i = 0; i < INP_BUFF; i++) { buf[i] = inp[indx_a][i];}
}

void prep_adc_dma(void)
{
  uint32_t adc_addr = DMAC->DMAC_CH_NUM[SSC_DMAC_RX_CH].DMAC_DADDR;
           adc_addr &= msk_addr;
           
  uint16_t indx_a = 0;
           if(adc_addr == adc_addr0) indx_a = 1;
           if(adc_addr == adc_addr1) indx_a = 0;
}

uint16_t prep_dac_dma(void)
{
  uint32_t dac_addr = DMAC->DMAC_CH_NUM[SSC_DMAC_TX_CH].DMAC_SADDR;
           dac_addr &= msk_addr;
           
  uint16_t indx_b = 0;
           if(dac_addr == dac_addr0) indx_b = 1;
           if(dac_addr == dac_addr1) indx_b = 0;
  return indx_b;
  
}
