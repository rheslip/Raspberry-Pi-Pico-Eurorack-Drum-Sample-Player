# Drum Machine/Sample player sketch for an Pico RP2040 based Eurorack module

My Pico module is a general purpose eurorack module based on the RP2040. Its very similar to Ornament and Crime:

	* 4 Gate/Trigger inputs/outputs - 3.3v levels, protected with schottky diode clamps to 3.3v and GND

	* 4 (**) CV inputs or digital inputs/outputs - nominally 0-5v CV input, protected with schottky diode clamps to 3.3v and GND

	* 2 CV outputs 0-6.144V 12 bit resolution
	
	* Left and Right audio outputs 10vpp, 16 bit PT8211 DAC

	* 128x64 OLED display

	* Two Encoders with switches

	* Two buttons

** Raspberry Pi Pico board has only 3 A/D input pins. There are versions on Aliexpress that have the 4rth A/D on the ADC_VRef pin.

No schematics yet but you can pretty much figure it out by looking at IO.h. Input pins are protected by 3.3k series resistors with BAT54 clamps. Analog ports have a 4.99k/10k divider to give a CV range of approx 0-5V.

This code is a port of my ESP32 drum machine code with a new menu system. Plays 16 bit mono audio samples at 22khz on the DAC outputs. It can be easily modified for 44Khz stereo samples but you will probably want a Pico board with 8-16Mb of flash.

* All inputs are used as sample triggers in this sketch. sample, level, pan etc for each drum voice can be set in the menus. 

* CV inputs - code is in there but you would have to give up some trigger inputs to use them as CV ins

* CV outputs - not used

* Audio plays on the L-R audio outputs. Sample rate and mono/stereo determined by code options and samples used

* NOTE: "save config" in the menu is not currently implemented

The code uses both cores of the Pico. First core is used for handling trigger inputs, encoders, menus. Second core is dedicated to summing samples that are playing and sending them to the I2S output buffer. Utilization of the second core is approx 50% handling 8 drum voices.

# Software Dependendencies:

* Adafruit Graphics library and the SSD1306 driver (code actually uses SH1106 but SSD1306 is recommended) https://github.com/adafruit/Adafruit-GFX-Library https://github.com/adafruit/Adafruit_SSD1306

* Clickencoder (included in the sketch library directory)

* Pico Timer library https://github.com/khoih-prog/RPI_PICO_TimerInterrupt

"wav2header" utility is included which will auto generate the required header files from 22 or 44khz .wav files. see resources/readme.md

Compiled with Arduino 2.01 with Arduino Pico installed. 

More info in the Wiki: https://github.com/rheslip/Motivation-Radio-Drum-Machine/wiki

Rich Heslip Feb 2023

