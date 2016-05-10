#ifndef DRUMS_H
#define DRUMS_H

#include "hardware.h"

struct drum_hit { //maybe turn some of these into bitfields

	uint8_t note_num; //assigned MIDI note number
	int8_t note_offset; //use this to add/subtract value dependent on assigned MIDI note number so that it can be mapped to drum map (0-14). Default is 0
	uint8_t spi_byte_num; //SPI byte number
	uint8_t trig_bit; //trigger bit for drum hit
	uint8_t switch_bit; //bit for switching toms/congas and rs/cl - will be 255 for all other instruments
	uint8_t switch_value :1; //is the switch one or 0?
	uint8_t spi_led_byte_num; //SPI byte number for instrument LED
	uint8_t led_bit; //bit for instrument LED
	
};

struct drum_hit drum_hit[16] = {
	
	{0,0,6, 1<<BD_TRIG, 255, 0, 0, 1<<BD_LED},
	{1,0,6, 1<<SD_TRIG, 255, 0, 0, 1<<SD_LED},
	{2,0,6, 1<<LT_TRIG, 1<<LT_LC_SW, 1, 0, 1<<LT_LED},	
	{3,0,6, 1<<MT_TRIG, 1<<MT_MC_SW, 1, 5, 1<<MT_LED},
	{4,0,6, 1<<HT_TRIG, 1<<HT_HC_SW, 1, 5, 1<<HT_LED},
	{5,0,6, 1<<RS_TRIG, 1<<RS_CL_SW, 0, 5, 1<<RS_LED},
	{6,0,6, 1<<CP_TRIG, 255, 0, 5, 1<<CP_LED},
	{7,0,4, 1<<CB_TRIG, 255, 0, 5, 1<<CB_LED},
	{8,0,4, 1<<CY_TRIG, 255, 0, 5, 1<<CY_LED},
	{9,0,4, 1<<OH_TRIG, 255, 0, 4, 1<<OH_LED},
	{10,0,4, 1<<CH_TRIG,255, 0, 4, 1<<CH_LED},
	{11,0,6, 1<<LT_TRIG, 1<<LT_LC_SW, 0, 1, 1<<LC_LED},
	{12,0,6, 1<<MT_TRIG, 1<<MT_MC_SW, 0, 1, 1<<MC_LED},
	{13,0,6, 1<<HT_TRIG, 1<<HT_HC_SW, 0, 1, 1<<HC_LED},
	{14,0,6, 1<<RS_TRIG, 1<<RS_CL_SW, 1, 1, 1<<CL_LED},
	{15,0,5, 1<<MA_TRIG, 255, 0, 5, 1<<MA_LED}											
	};

//initialize array of drum hit structs

//drum_hit[0] = {0,0,6, 1<<BD_TRIG, 255, 0, 1<<BD_LED};

#endif