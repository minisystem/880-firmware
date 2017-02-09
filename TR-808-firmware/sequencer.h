/*
 * sequencer.h
 * Open808 firmware ATMEGA328PB
 * minisystem
 * system80.net
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

#define NUM_PATTERNS 64 //number of patterns per rhythm track

//define pre-scale ppqn dividers
#define NUM_PRE_SCALES 4
#define PRE_SCALE_LED_MASK 0b11000011

#define PRE_SCALE_1 8
#define PRE_SCALE_2 4
#define	PRE_SCALE_3 6
#define	PRE_SCALE_4 3
	

//extern uint8_t pre_scale[NUM_PRE_SCALES]; //does this need to be exturned? Just data. Could go in update_prescale() function?	

enum variation_mode {
	
	VAR_A,
	VAR_B,
	VAR_AB
	
	
};



struct flag {
	
	uint8_t next_step:1;
	uint8_t half_step:1;
	uint8_t variation_change:1;
	uint8_t trig_finished:1;
	//uint8_t step_num_change:1;//not currently used
	uint8_t pattern_edit:1; //flag if pattern is edited, need to write to eeprom at end of measure
	uint8_t pattern_change:1;
	uint8_t new_measure:1;
	uint8_t pre_scale_change:1;
	uint8_t tap:1;
	uint8_t intro:1; //flag for starting with selected intro pattern in manual play mode
	uint8_t fill:1;
	uint8_t din_start:1;
	uint8_t shuffle_step:1; //flag to delay step when shuffle is active
	uint8_t shuffle_change:1; //flag to indicate shuffle amount has changed
	
}; 
struct pattern { //current pattern will be loaded into ram from eeprom. changing pattern will write to eeprom and load next pattern
	uint16_t accent[NUM_PARTS]; // 2 parts of 16 steps of accent data, 2 parts
	uint16_t part[NUM_PARTS][NUM_STEPS]; //2 parts, 16 steps each. thanks to Omar
	//uint16_t step_led_mask[17];
};

struct rhythm_pattern {
	
	uint8_t pattern_num:5; //currently only 16 patterns available, but could have 32 patterns by implementing feature to access patterns 17-32: TODO
	uint8_t variation:1; //store variation with rhythm pattern - distinct from original 808 rhythm play
	
	};
	
//struct rhythm_track {
//
	//rhythm_pattern pattern[NUM_PATTERNS];	
	//
//}
	
extern struct rhythm_pattern rhythm_track[NUM_PATTERNS];

struct sequencer {
	
	enum global_mode mode;
	enum sync_mode sync_mode;
	uint8_t fill_mode:4;
	uint8_t SHIFT:1; //is SHIFT key being held?
	uint8_t START:1; //is sequencer running or not?
	uint8_t CLEAR:1; //is the clear button being held?
	uint8_t FUNC:1; //alternative function mode
	//uint8_t SHUFFLE:1;
	uint8_t shuffle_amount:3;
	uint8_t new_shuffle_amount:3;
	uint8_t shuffle_ppqn_count:4;
	uint8_t roll_mode:3;
	uint8_t roll_instrument:4;
	struct pattern pattern[2]; //Variation A:0, Variation B: 1
	uint16_t step_led_mask[2][17];
	uint8_t variation_toggle:1;
	uint8_t variation:1; //variation A or variation B
	uint8_t intro_fill_var:1; //intro/fill variation
	enum variation_mode variation_mode; //0 = A, 1 = B, 2 = toggle AB
	uint8_t step_num[NUM_PARTS];
	uint8_t step_num_new:5; //holder to change step number at end of measure - extra bit to hold NO_STEPS exception. harrumph.
	uint8_t current_step:4; //max 16 steps per part
	uint8_t part_playing:1; //0 or 1 first part or second part - will toggle
	uint8_t part_editing:1; //part currently being edited. Determined by mode
	uint8_t pre_scale:2;
	uint8_t current_pattern:4;
	uint8_t new_pattern:4;//will need to use this when in manual play and rhythm compose modes
	uint8_t current_intro_fill:4;
	uint8_t current_measure;
	enum drum current_inst; //this is index of drum_hit struct
	uint8_t var_led_mask;
	uint8_t trigger_1:5; //trigger assignments from AC, BD-CH - 17
	uint8_t trigger_2:5;
	};
		

extern struct sequencer sequencer;
volatile struct flag flag;
extern uint8_t pre_scale_index;
void update_tempo(void);
void process_tick(void);
void process_step(void);
void update_step_board(void);

void process_start(void);
void process_stop(void);
void process_new_measure(void);
void update_variation(void);
void update_prescale(void);
void update_fill(void);
void update_shuffle(uint8_t shuffle_amount);
void check_tap(void);
void toggle_variation(void);

void read_next_pattern(uint8_t pattern_num);
void write_current_pattern(uint8_t pattern_num);

#endif 