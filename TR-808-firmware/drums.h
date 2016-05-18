#ifndef DRUMS_H
#define DRUMS_H

#include "hardware.h"
#include "leds.h"

enum drum { //index for drum hits
	
	BD,
	SD,
	LT,
	MT,
	HT,
	RS,
	CP,
	CB,
	CY,
	OH,
	CH,
	LC,
	MC,
	HC,
	CL,
	MA,
	AC
	
	
};

struct drum_hit { //can save a few bytes here y getting ride of spi led byte and led bit and associating them elsewhere

	uint8_t note_num:7; //assigned MIDI note number
	uint8_t spi_byte_num:4; //SPI byte number
	uint8_t trig_bit; //trigger bit for drum hit
	uint8_t switch_bit; //bit for switching toms/congas and rs/cl - will be -1 for all other instruments
	uint8_t switch_value :1; //is the switch one or 0?
	uint8_t led_index; //index for intrument LED
	
};

extern struct drum_hit drum_hit[16];// = {
	//
	//{0,8, 1<<BD_TRIG,-1, 0, 2, 1<<BD_LED_BIT},
	//{1,8, 1<<SD_TRIG,-1, 0, 2, 1<<SD_LED_BIT},
	//{2,8, 1<<LT_TRIG, 1<<LT_LC_SW, 1, 2, 1<<LT_LED_BIT},	
	//{3,8, 1<<MT_TRIG, 1<<MT_MC_SW, 1, 7, 1<<MT_LED_BIT},
	//{4,8, 1<<HT_TRIG, 1<<HT_HC_SW, 1, 7, 1<<HT_LED_BIT},
	//{5,8, 1<<RS_TRIG, 1<<RS_CL_SW, 0, 7, 1<<RS_LED_BIT},
	//{6,8, 1<<CP_TRIG,-1, 0, 7, 1<<CP_LED_BIT},
	//{7,6, 1<<CB_TRIG,-1, 0, 7, 1<<CB_LED_BIT},
	//{8,6, 1<<CY_TRIG,-1, 0, 7, 1<<CY_LED_BIT},
	//{9,6, 1<<OH_TRIG,-1, 0, 6, 1<<OH_LED_BIT},
	//{10,6, 1<<CH_TRIG,-1, 0, 6, 1<<CH_LED_BIT},
	//{11,8, 1<<LT_TRIG, 1<<LT_LC_SW, 0, 3, 1<<LC_LED_BIT},
	//{12,8, 1<<MT_TRIG, 1<<MT_MC_SW, 0, 3, 1<<MC_LED_BIT},
	//{13,8, 1<<HT_TRIG, 1<<HT_HC_SW, 0, 3, 1<<HC_LED_BIT},
	//{14,8, 1<<RS_TRIG, 1<<RS_CL_SW, 1, 3, 1<<CL_LED_BIT},
	//{15,7, 1<<MA_TRIG,-1, 0, 7, 1<<MA_LED_BIT}											
	//};

extern uint8_t current_drum_hit; //global to hold current drum hit
volatile uint8_t trigger_finished;

void trigger_drum(uint8_t note, uint8_t velocity);

#endif