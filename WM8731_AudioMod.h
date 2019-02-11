/*
 **************************  WM8731 driver *************************
 * LINK: http://coolarduino.blogspot.ca
 *
 * Created for  Arduino DUE board.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute copies of the Software, 
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY,  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *
 * Copyright (C) 2015 
 * Anatoly Kuzmenko.
 * k_anatoly@hotmail.com
 * 
 * Copyright (C) 2017
 * Ervin Teng
 * 
 * All Rights Reserved.
 ********************************************************************************************
 */

#ifndef CODEC_H
#define CODEC_H


// PWM - MCK 16.8MHz (non-USB) clock for WM8731
#define   _BV(bit) (0x1u << bit)
#define   CHAN 6

#ifndef SAMPLE_RATE
#define SAMPLE_RATE hz44100
#endif

#define OUT_BUFF  2048

#include "Arduino.h"
#include "wm8731Mod.h"



class CodecClass
{
  public:
    CodecClass() {};
    
    void begin(); // Power on codec and start

    void halt(); // Power off codec
    
    void playTone(uint16_t _frequency); // Plays a tone. Does not stop until stopTone is called. Params: Frequency in Hz. 
   
    void stopTone(); // Stop playback of any current tones. 

    void setOutputVolume(unsigned char); // Set volume of both headphone channels. Value from 0-127. 
    
    uint16_t getCurrentFrequency();
    void setCurrentFrequency(uint16_t);
    
 
  private:
    uint16_t frequency;
    void initCodec();
    //void load_sine_wave(uint16_t freq);


};

extern CodecClass Codec;
extern int32_t out_buf[OUT_BUFF];
extern volatile bool out_buf_ready;

#endif
