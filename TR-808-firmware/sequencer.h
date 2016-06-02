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

enum variation_mode {
	 
	 VAR_A,
	 VAR_B,
	 VAR_AB
	
	
	};

#define FIRST 0
#define SECOND 1

#define NUM_PARTS 2
#define NUM_STEPS 16

struct pattern { //current pattern will be loaded into ram from eeprom. changing pattern will write to eeprom and load next pattern
	
	uint16_t part[NUM_PARTS][NUM_STEPS]; //2 parts, 16 steps each. thanks to Omar
	uint32_t accent; //32 steps of accent data
	uint16_t step_led_mask[17];
	uint8_t pre_scale:2; //1-4 (0-3) //IS THIS GLOBAL OR IS IT PATTERN SPECIFIC?
	
	
};

struct flag {
	
	uint8_t next_step:1;
	uint8_t half_step:1;
	uint8_t variation_change:1;
	uint8_t trig_finished:1;
	uint8_t step_num_change:1;
	uint8_t new_measure:1;
	
	
	
	};

//typedef STEP_NUM_BITS uint8_t:

struct sequencer {
	
	enum global_mode mode;
	uint8_t SHIFT:1; //is SHIFT key being held?
	uint8_t START:1; //is sequencer running or not?
	uint8_t CLEAR:1; //is the clear button being held?
	struct pattern pattern[2]; //Variation A:0, Variation B: 1
	uint8_t variation:1; //variation A or variation B
	enum variation_mode variation_mode; //0 = A, 1 = B, 2 = toggle AB
	
	uint8_t step_num[NUM_PARTS];
	//uint8_t step_num_first:4; //number of steps for first part
	//uint8_t step_num_second:4; //number of steps for second part
	uint8_t step_num_new:4; //holder to change step number at end of measure
	uint8_t current_step:4; //max 16 steps per part
	uint8_t part_playing:1; //0 or 1 first part or second part - will toggle
	uint8_t part_editing:1;
	uint8_t pattern_num:4;
	uint8_t current_measure;
	enum drum current_inst; //this is index of drum_hit struct
	
	uint8_t var_led_mask;
	
	};
		

extern struct sequencer sequencer;
extern struct flag flag;

void update_tempo(void);
void process_step(void);
void update_step_board(void);
//uint8_t step_mask(void);

void update_variation(void);

#endif 