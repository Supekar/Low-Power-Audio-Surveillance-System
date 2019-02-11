#include <Wire.h>
#include "WM8731_AudioMod.h"
#include "arduinoFFT.h"
#include<LoRa.h>
#include <PowerDue.h>

arduinoFFT FFT = arduinoFFT(); /* Create FFT object */
/*
These values can be changed in order to evaluate the functions
*/
const uint16_t SAMPLES = OUT_BUFF; //This value MUST NOT be modified, ALWAYS be a power of 2
const double SAMP_FREC = 88200.0; //This value MUST NOT be modified

/*
These are the input and output vectors
Input vectors receive computed results from FFT
*/
double vReal[SAMPLES]; //= out_buf;
double vImag[SAMPLES]; // = {0.0};
double maxFreq, SecondMaxFreq;
int maxFreqint, SecondMaxFreqint;
int sleepTimeInSeconds = 5;
#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03
#define SET_FREQUENCY 915E6
#define TX_POWER 8
#define SPREADING_FACTOR 7

unsigned long prevTime, elapsedTime;

void sleepFor(uint32_t forHowManySeconds){
  // read the status bit to reset interrupt
  RTT->RTT_SR;
  // wake up every forHowManySeconds seconds
  RTT_SetAlarm(RTT, forHowManySeconds); 
  // enable interrupt from alarm
  RTT_EnableIT(RTT, RTT_MR_ALMIEN);
  // reset timer
  RTT->RTT_MR |= RTT_MR_RTTRST; 
  // enable wake up from realtime timer
  SUPC->SUPC_WUMR |= SUPC_WUMR_RTTEN_ENABLE;
  pmc_enable_backupmode();
}

void setup(void) {
  //delay(5000);
  //int sleepTimeInSeconds = 25;
  pinMode(6,OUTPUT);
  pinMode(7,OUTPUT);
  pinMode(8,OUTPUT);
  digitalWrite(6,LOW);
  digitalWrite(7,LOW);
  digitalWrite(8,LOW);

  SerialUSB.begin(115200);
  while(!SerialUSB); 
  SerialUSB.println("started");
  digitalWrite(6,HIGH);
  digitalWrite(7,HIGH);
  digitalWrite(8,HIGH);
  
  Codec.begin();
   
//  SerialUSB.print("out buffer at: ");
//  SerialUSB.println((long)out_buf);
  LoRa.setPins(22,59,51);
 
  while(!LoRa.begin(SET_FREQUENCY));
  SerialUSB.println("waiting");
  prevTime = millis();
  while(!out_buf_ready);
  elapsedTime = millis() - prevTime;
  SerialUSB.print("buffer delay time: ");
  SerialUSB.println(elapsedTime);  
  SerialUSB.println("buf ready");

  //print out the audio signal values in the buffer
//  for(uint16_t i=0; i<OUT_BUFF; ++i){
//    SerialUSB.println(out_buf[i]);
//  }  
 
  int i,x;
  int maxFreqint=0;
  int SecondMaxFreqint=0;
  int maxFreqint1=100;
  int SecondMaxFreqint1=0;
  int maxFreqLow=100;
  int maxFreqHigh =0;
  float F1_ans,F2_ans;
  prevTime = millis();
  
  while(maxFreqint1>maxFreqHigh||maxFreqint1<maxFreqLow||maxFreqint>maxFreqHigh||maxFreqint<maxFreqLow||maxFreqint1<200||maxFreqint>19000||SecondMaxFreqint1<200||SecondMaxFreqint>19000){
    for (uint16_t i = 0; i < SAMPLES; i++)
    {
      vReal[i] = (double)out_buf[i];
      vImag[i] = 0.0; //Imaginary part must be zeroed in case of looping to avoid wrong calculations and overflows
    }
   
    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_BLACKMAN, FFT_FORWARD);  /* Weigh data */
  
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD); /* Compute FFT */
   
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES); /* Compute magnitudes */
  
    double maxFreq = FFT.MajorPeak(vReal, SAMPLES, SAMP_FREC);
    double SecondMaxFreq = FFT.SecondMajorPeak();
   
    maxFreqint = (int) maxFreq;
    SecondMaxFreqint = (int) SecondMaxFreq;
    
    if(maxFreqint>SecondMaxFreqint){
      
      }
    else{
      
      int temp=maxFreqint;
      maxFreqint=SecondMaxFreqint;
      SecondMaxFreqint=temp;
      }
    maxFreqLow = maxFreqint-10; 
    maxFreqHigh = maxFreqint+10;
    SerialUSB.println("1 max");
    SerialUSB.println(maxFreqint);
    SerialUSB.println("1 2nd max");
    SerialUSB.println(SecondMaxFreqint);
    
    
  
    for (uint16_t i = 0; i < SAMPLES; i++)
    {
      vReal[i] = (double)out_buf[i];
      vImag[i] = 0.0; //Imaginary part must be zeroed in case of looping to avoid wrong calculations and overflows
    }
  
    /* Print the results of the simulated sampling according to time */
   
    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_BLACKMAN, FFT_FORWARD);  /* Weigh data */
  
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD); /* Compute FFT */
   
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES); /* Compute magnitudes */
  
    double maxFreq1 = FFT.MajorPeak(vReal, SAMPLES, SAMP_FREC);
    double SecondMaxFreq1 = FFT.SecondMajorPeak();
   
    int maxFreqint1 = (int) maxFreq1;
    int SecondMaxFreqint1 = (int) SecondMaxFreq1;
    if(maxFreqint1>SecondMaxFreqint1){
      
      }
    else{
      
      int temp=maxFreqint1;
      maxFreqint=SecondMaxFreqint1;
      SecondMaxFreqint1=temp;
      }
  
    SerialUSB.println("2 max");
    SerialUSB.println(maxFreqint1);
    SerialUSB.println("2 2nd max");
    SerialUSB.println(SecondMaxFreqint1);
  
    if(maxFreqint1<maxFreqHigh&&maxFreqint1>maxFreqLow&&maxFreqint<maxFreqHigh&&maxFreqint>maxFreqLow&&maxFreqint1>200&&maxFreqint<19000&&SecondMaxFreqint1>200&&SecondMaxFreqint<19000){
      SerialUSB.println("Prepared to break");
      SerialUSB.print("F1:");
      SerialUSB.println(maxFreqint1);
      F1_ans =(float)maxFreqint1;
      SerialUSB.print("F2:");
      SerialUSB.println(SecondMaxFreqint1);
      F2_ans = (float)SecondMaxFreqint1;
      break;
      }
      else{
        
        }
  }

  
  elapsedTime = millis() - prevTime;
  
  SerialUSB.println("finally sending");
  SerialUSB.println(F1_ans);
  SerialUSB.println(F2_ans);
  
  
//  SerialUSB.print("max frequency: ");
//  SerialUSB.println(maxFreq, 6);
//  SerialUSB.print("second max frequency: ");
//  SerialUSB.println(SecondMaxFreq, 6);
//  
//  SerialUSB.print("elapsedTime: ");
// SerialUSB.println(elapsedTime);

 int count;
 for(count=0;count<5;count++){
  
  LoRa.beginPacket();
  //LoRa.print("r");
  LoRa.print("r");
  //LoRa.print("F");
  LoRa.print(maxFreqint);
  LoRa.print(",");
  //LoRa.print(" F2:");
  LoRa.print(SecondMaxFreqint);
  LoRa.print("s");
  //LoRa.print("y");
  LoRa.endPacket();
  //delay(5000);
  //sleepFor(sleepTimeInSeconds);
  }
  SerialUSB.println("hi");
  int checksize=LoRa.parsePacket();
  
  if(checksize){
    char readarray[1];
    readarray[0]=(char)LoRa.read();
    if(readarray[0]=='V'){
      LoRa.sleep();
      
      }
    }
  sleepFor(sleepTimeInSeconds);
}

void loop()
{
  //int maxFreqint = (int) maxFreq;
  //int SecondMaxFreqint = (int) SecondMaxFreq;
  //SerialUSB.println(maxFreq);
  //SerialUSB.println(SecondMaxFreq); 
  //LoRa.beginPacket();
  //LoRa.print("r");
  //LoRa.print("F");
  //LoRa.print(maxFreqint);
  //LoRa.print(",");
  //LoRa.print(" F2:");
  //LoRa.print(SecondMaxFreqint);
  //LoRa.print("s");
  //LoRa.endPacket(); 
  //sleepFor(sleepTimeInSeconds);
  //delay(5000);
}


void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType)
{
  for (uint16_t i = 0; i < bufferSize; i++)
  {
    double abscissa;
    /* Print abscissa value */
    switch (scaleType)
    {
      case SCL_INDEX:
        abscissa = (i * 1.0);
        break;
      case SCL_TIME:
        abscissa = ((i * 1.0) / SAMP_FREC);
        break;
      case SCL_FREQUENCY:
        abscissa = ((i * 1.0 * SAMP_FREC) / SAMPLES);
        break;
    }
    SerialUSB.print(abscissa, 6);
    if(scaleType==SCL_FREQUENCY)
      //SerialUSB.print("Hz");
      SerialUSB.print(",");
    SerialUSB.print(" ");
    SerialUSB.println(vData[i], 4);
  }
  SerialUSB.println();
}
