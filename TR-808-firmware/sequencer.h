/*
 * sequencer.h
 * JR-808 firmware ATMEGA328PB
 * minisystem
 * system79.com
 */



#ifndef SEQUENCER_H
#define SEQUENCER_H

#include "mode.h"
#include "drums.h"

#define VAR_A	0
#define VAR_B	1
#define VAR_AB	2

struct pattern { //current pattern is loaded into ram from eeprom. changing pattern will write to eeprom and load next pattern
	
	uint16_t part[32]; 
	uint32_t accent; //32 steps of accent data
	uint16_t step_led_mask[17];
	//uint8_t step_num:4; //1-16 (0-15)
	uint8_t pre_scale:2; //1-4 (0-3) //IS THIS GLOBAL OR IS IT PATTERN SPECIFIC?
	
	
};



struct sequencer {
	
	enum global_mode mode;
	uint8_t SHIFT:1; //is SHIFT key being held?
	uint8_t START:1; //is sequencer running or not?
	struct pattern pattern[2]; //Variation A:0, Variation B: 1
	uint8_t variation:1; //variation A or variation B
	uint8_t variation_mode:2; //0 = A, 1 = B, 2 = toggle AB
	uint8_t var_change:1; //flag to indicate variation has changed - reset at end of measure
	uint8_t step_num:5;
	uint8_t current_step:5; //will need to increase this with sequences >16 steps, or use offset?
	uint8_t next_step_flag:1;
	uint8_t trigger_finished:1;
	uint8_t pattern_num:4;
	uint8_t current_measure;
	enum drum current_inst; //this is index of drum_hit struct
	
	uint8_t var_led_mask;
	
	};
		

extern struct sequencer sequencer;


#endif 