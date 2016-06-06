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


//define part indices
#define FIRST 0
#define SECOND 1

#define NUM_PARTS 2
#define NUM_STEPS 16
#define NO_STEPS 16 //null state of part 2 step number

//define pre-scale ppqn dividers
#define NUM_PRE_SCALES 4
#define PRE_SCALE_LED_MASK 0b11000011

#define PRE_SCALE_1 8
#define PRE_SCALE_2 4
#define	PRE_SCALE_3 6
#define	PRE_SCALE_4 3
	

extern uint8_t pre_scale[4];	

enum variation_mode {
	
	VAR_A,
	VAR_B,
	VAR_AB
	
	
};

struct blewt {
	
	uint8_t mode:4;
	uint8_t pre_scale:4;
	
	};

struct flag {
	
	uint8_t next_step:1;
	uint8_t half_step:1;
	uint8_t variation_change:1;
	uint8_t trig_finished:1;
	uint8_t step_num_change:1;
	uint8_t new_measure:1;
	uint8_t pre_scale_change:1;
	uint8_t tap:1;
	
}; 
struct pattern { //current pattern will be loaded into ram from eeprom. changing pattern will write to eeprom and load next pattern
	
	uint16_t part[NUM_PARTS][NUM_STEPS]; //2 parts, 16 steps each. thanks to Omar
	uint16_t accent[NUM_PARTS]; // 2 parts of 16 steps of accent data, 2 parts
	uint16_t step_led_mask[17];
};



//typedef STEP_NUM_BITS uint8_t:

struct sequencer {
	
	enum global_mode mode;
	enum sync_mode sync_mode;
	uint8_t SHIFT:1; //is SHIFT key being held?
	uint8_t START:1; //is sequencer running or not?
	uint8_t CLEAR:1; //is the clear button being held?
	uint8_t SLAVE:1; //is the sequencer a tempo slave?
	struct pattern pattern[2]; //Variation A:0, Variation B: 1
	uint8_t variation:1; //variation A or variation B
	enum variation_mode variation_mode; //0 = A, 1 = B, 2 = toggle AB
	uint8_t step_num[NUM_PARTS];
	uint8_t step_num_new:5; //holder to change step number at end of measure - extra bit to hold NO_STEPS exception. harrumph.
	uint8_t current_step:4; //max 16 steps per part
	uint8_t part_playing:1; //0 or 1 first part or second part - will toggle
	uint8_t part_editing:1; //part currently being edited. Determiend by mode
	uint8_t pre_scale:2;
	uint8_t pattern_num:4;
	uint8_t current_measure;
	enum drum current_inst; //this is index of drum_hit struct
	uint8_t var_led_mask;
	
	};
		

extern struct sequencer sequencer;
volatile struct flag flag;
extern uint8_t pre_scale_index;
void update_tempo(void);
void process_step(void);
void update_step_board(void);
//uint8_t step_mask(void);

void update_variation(void);
void update_prescale(void);
void check_tap(void);
void toggle_variation(void);

#endif 