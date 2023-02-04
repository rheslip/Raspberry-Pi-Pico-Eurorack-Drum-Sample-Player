
// Copyright 2023 Rich Heslip
//
// Author: Rich Heslip 
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// I/O pin definitions for Raspberry Pi Pico eurorack module


#ifndef IO_H_
#define IO_H_

// SPI pins - we use SPI for MCP4822 dual 2 bit DAC
#define MISO_PIN 16
#define MOSI_PIN 19
#define SCLK_PIN 18
#define DAC0_CS 17 // note that this should be software controlled - MCP4822 doesn't work with HW CS

// I2S pins for PT8211 dual DAC
#define BCLK 20
#define WS 21  // this will always be 1 pin above BCLK - can't change it
#define I2S_DATA 22

// OLED display pins
// OLED is I2C
#define OLED_SDA 8
#define OLED_SCL 9

#define L_ENC_A 11  // left and right encoders
#define L_ENC_B 12
#define L_SW   10 
#define R_ENC_A 6
#define R_ENC_B 7
#define R_SW   5 

#define BUTTON1 14
#define BUTTON2 15
// analog inputs for voltage control - range approx 0-5v
// 
#define AIN0 	26
#define AIN1 	27
#define AIN2 	28
#define AIN3 	29 // not available on standard Pico board

// Gate/trigger digital inputs 
#define GATE0 1
#define GATE1	0
#define GATE2	2
#define GATE3	3
#define GATE4 AIN0  // only if analog not used
#define GATE5	AIN1
#define GATE6	AIN2
//#define GATE7	AIN3 // for AA boards which have AIN3
#define GATE7 4  // for the Pico board

// Gate/trigger digital outputs - use as in or out, not both
#define TRIG0 1
#define TRIG1	0
#define TRIG2	2
#define TRIG3	3
#define TRIG4 AIN0  // only if analog not used
#define TRIG5	AIN1
#define TRIG6	AIN2
//#define TRIG7	AIN3 // for AA boards which have AIN3
#define TRIG7 4  // for the Pico board

// MIDI serial port pins - not really MIDI but the serial port is exposed on the first two jacks
#define MIDIRX 1
#define MIDITX 0

#endif // IO_H_

