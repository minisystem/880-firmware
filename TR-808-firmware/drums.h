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

struct drum_hit {

	uint8_t note_num:7; //assigned MIDI note number
	uint8_t spi_byte_num:4; //SPI byte number
	uint8_t trig_bit; //trigger bit for drum hit
	uint8_t switch_bit; //bit for switching toms/congas and rs/cl - will be -1 for all other instruments
	uint8_t switch_value :1; //is the switch one or 0?
	uint8_t led_index; //index for intrument LED
	
};

extern struct drum_hit drum_hit[17];


extern uint8_t current_drum_hit; //global to hold current drum hit
volatile uint8_t trigger_finished;

void trigger_drum(uint8_t note, uint8_t velocity);
void trigger_step(void);
void clear_all_trigs(void);
void live_hits(void);

#endif