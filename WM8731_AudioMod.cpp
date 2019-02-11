#include "Arduino.h"
#include <Wire.h>
#include "WM8731_AudioMod.h"
#include "sine.c"

int32_t out_buf[OUT_BUFF] =  { 0.0};
volatile bool out_buf_ready=false;
int32_t *pos = out_buf;
uint32_t counter=0;
uint32_t delayInterval=0;

void CodecClass::begin()
{
  Wire.begin();
  this->initCodec();
  pio_B_SSC();
  init_ssc();
  init_dma();
  ssc_dma_cfg();
  //Codec.onTxReady(load_sine_wave);

}

// Power off Codec
void CodecClass::halt()
{
  set_reg(WM8731_POWERDOWN, WM8731_POWERDOWN_POWEROFF);
}

// Plays a tone. Does not stop until stopTone is called.
// Params: Frequency in Hz.
void CodecClass::playTone(uint16_t _frequency){
  this-> setCurrentFrequency(_frequency);
}

//Stop playback of any playing tones.
void CodecClass::stopTone(){
  this->setCurrentFrequency(0);
}

uint16_t CodecClass::getCurrentFrequency() {
  return this->frequency;
}

void CodecClass::setCurrentFrequency(uint16_t _frequency) {
  this->frequency = _frequency;
}

// Congifure Codec
void CodecClass::initCodec()
{
  set_buffer_addrs(); //Properly initialize DMA buffer addresses

  set_reg(  WM8731_POWERDOWN, 0x50 ); //outpd=1, osc=0, clkout=1.
  set_reg(  WM8731_LLINEIN, 0x11F ); //line l&r

  set_reg( WM8731_LHEADOUT, ( 0x1FF & WM8731_LHEADOUT_LHPVOL_MASK ) | WM8731_LHEADOUT_LHPVOL(80)); //phone l&r
  set_reg( WM8731_RHEADOUT, ( 0x1FF & WM8731_LHEADOUT_LHPVOL_MASK ) | WM8731_LHEADOUT_LHPVOL(80) ); //phone l&r

  set_reg(  WM8731_ANALOG, 0x14 ); //audio path, dac, enable microphone 0001 0100
  set_reg(  WM8731_DIGITAL, 0x00 ); //digit path, dac mute -(1)000 = 0x08
  set_reg(  WM8731_INTERFACE, 0x4E ); //interface format, master -0(1)00 11 10 = 32-bit
  set_reg(  WM8731_SAMPLING, 0x20); //sampling,
  // 44.1k = 100000, non usb 11.289 MHz : 384
  set_reg(  WM8731_CONTROL, WM8731_CONTROL_ACTIVE ); //active=1
  set_reg(  WM8731_POWERDOWN, 0x40 ); //, osc=0, clkout=1.
}

// Sets the volume. Give a value between 0 and 127
void CodecClass::setOutputVolume( unsigned char value )
{
    unsigned char reg;
    reg = ( 0x1FF & WM8731_LHEADOUT_LHPVOL_MASK ) | WM8731_LHEADOUT_LHPVOL(value) /* | WM8731_LHEADOUT_LZCEN */;
    set_reg( WM8731_LHEADOUT, reg );
    reg = (0x1FF & WM8731_RHEADOUT_RHPVOL_MASK ) | WM8731_RHEADOUT_RHPVOL(value) /* | WM8731_LHEADOUT_RZCEN */;
    set_reg( WM8731_RHEADOUT, reg );
}

uint16_t sineTblPtr = 0;
int16_t sign = 1;
// Waveform index is * 1024 to make fractional steps (interpolation) possible.
uint32_t waveformIndex = 0;

extern const int16_t sine[512];


// Load a sine wave into the output buffer, sampled at freq Hz. Note that we're interpolating a
// lookup table, so especially at the higher frequencies we loose a lot of resolution. Some notes
// may sound the same despite being different frequencies.
void load_sine_wave(uint16_t freq, uint16_t buf_index)
{
  //uint16_t sample_rate = 44100;

  // We assume sample rate is fixed and compute this constant to speed up computations.
  // 1024 is the number of samples in a sine wave period, and the second 1024 is an offset added to
  // allow for fractional stepsizes without using floats.
  // (freq * 1024) /sample_rate/2 *1024  ~= freq * 12;
  uint16_t stepSize = freq * 12;

  for( uint16_t i = 0; i < INP_BUFF; i++) {
    if (waveformIndex>>10 < 512){
      // We can increase the volume further by increasing this shift value here.
      out[buf_index][i] = sign*sine[waveformIndex>>10]<<12;
      waveformIndex += stepSize;
    }
    else{
      out[buf_index][i] = sign*sine[0]<<12;
      sign*=-1;
      sineTblPtr=0;
      waveformIndex = 0;
    }
  }
}

// DMA Interrupt handler
void DMAC_Handler(void)
{
  uint32_t dma_status;
  
  dma_status = DMAC->DMAC_EBCISR;
  if (dma_status & (DMAC_EBCISR_BTC0 << SSC_DMAC_RX_CH)) {
    if(pos-out_buf < OUT_BUFF){
      prep_adc_dma(pos);
      pos = pos + INP_BUFF;
      //to test pos increment correctly
//      SerialUSB.print("pos: ");
//      SerialUSB.println((long)pos);
//      SerialUSB.print("out buf: ");
//      SerialUSB.println((long)out_buf);      
    }else{
      prep_adc_dma();
      //each increment is about 5.8ms 
      delayInterval++;
      //delay interval is about .58s between buffer collection
      if(delayInterval > 100){ 
        delayInterval = 0;
        pos = out_buf;
        counter++;
      }        

      //counter decides actual collection start time after how many delay intervals 
      if(delayInterval==1 && counter==2) {
        //counter =0;
        out_buf_ready= true;
      }
       
    }

  
    //Codec.onRxReadyCallback(10);  
  }

  if (dma_status & (DMAC_EBCISR_BTC0 << SSC_DMAC_TX_CH)) {
    uint16_t buf_index = prep_dac_dma();
    load_sine_wave(Codec.getCurrentFrequency(), buf_index);
    //Codec.onTxReadyCallback(10);
  }
}


// Create the Codec object
CodecClass Codec = CodecClass();
