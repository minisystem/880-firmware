/*
 * sequencer.h
 * JR-808 firmware ATMEGA328PB
 * minisystem
 * system79.com
 */



#ifndef SEQUENCER_H
#define SEQUENCER_H

#include "mode.h"

struct pattern { //patterns stored in eeprom - only retrieve patterns from eeprom as you need them
	
	uint16_t first_part[16]; //use to light step LEDs and trigger drums?
	uint16_t second_part[16];
	uint8_t step_num:4; //1-16 (0-15)
	uint8_t pre_scale:2; //1-4 (0-3)
	
	
};



struct sequencer {
	
	enum global_mode mode;
	uint8_t SHIFT:1; //is SHIFT key being held?
	uint8_t START:1; //is sequencer running or not?
	struct pattern current_pattern;
	uint8_t current_step:4;
	uint8_t current_measure;
	uint8_t current_inst:4;
	
	
	};
		

extern struct sequencer sequencer;


#endif 