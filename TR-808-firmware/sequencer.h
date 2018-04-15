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

#define NUM_BANKS 12 //12 banks of patterns

#define BASIC_RHYTHM 12 //number of basic rhythms

#define NUM_PATTERNS 64 //number of patterns per rhythm track

#define NUM_TRACKS 12 //12 rhythm tracks

//define pre-scale ppqn dividers
#define NUM_PRE_SCALES 4


//pulses per qarter note for different pre-scales
//8/4/6/3 for 24 ppqn, 32/16/24/12 for 96 ppqn
#define PRE_SCALE_1 32
#define PRE_SCALE_2 16
#define	PRE_SCALE_3 24
#define	PRE_SCALE_4 12

#define SHUFFLE_MIN 0
#define SHUFFLE_MAX 6
#define ROLL_MIN	8 //ROLL_MIN/MAX refer to absolute switch positions - used as offsets for setting ROLL LEDs and reading step switches
#define ROLL_MAX	14
#define NO_ROLL		0 //roll mode off
#define ROLL_32		5 //max roll mode

//track mode
#define CREATE 0
#define EDIT 1

//track edit
#define INSERT 14
#define DELETE 15
	

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
	uint8_t last_pattern:1;
	uint8_t track_edit:1;
	uint8_t new_measure:1;
	uint8_t pre_scale_change:1;
	uint8_t tap:1;
	uint8_t intro:1; //flag for starting with selected intro pattern in manual play mode
	uint8_t fill:1;
	uint8_t slave_start:1;
	uint8_t wait_for_master_tick:1;	
	uint8_t shuffle_step:1; //flag to delay step when shuffle is active
	uint8_t shuffle_change:1; //flag to indicate shuffle amount has changed
	uint8_t blink:1;
	uint8_t din_start:1;
	uint8_t nudge_down:1;
	uint8_t nudge_up:1;
	
}; 
struct pattern { //current pattern will be loaded into ram from eeprom. changing pattern will write to eeprom and load next pattern
	uint16_t accent[NUM_PARTS]; // 2 parts of 16 steps of accent data, 2 parts
	uint16_t part[NUM_PARTS][NUM_STEPS]; //2 parts, 16 steps each. thanks to Omar
	//uint16_t step_led_mask[17];
};

struct track_pattern { //maybe don't need 64 patterns in RAM, just current pattern and address of next pattern? Just have pattern_position to tell it where to get the next pattern in memory: pattern_position + 1 up to NUM_PATTERNS
	
	uint8_t current_pattern:4;
	uint8_t current_bank:4;
	//what about variation? Can store variation here
	//uint8_t current_variation:1;
	//uint8_t length:7; //need to know when we've hit last measure of rhythm track
	
	
	};
	
struct rhythm_track {
	
	struct track_pattern patterns[NUM_PATTERNS];
	uint8_t	length;
	//uint8_t current_measure;
	
	};	

extern struct rhythm_track rhythm_track;	
	
//struct rhythm_track {
//
	//rhythm_pattern pattern[NUM_PATTERNS];	
	//
//}
	
//extern struct rhythm_track rhythm_track;//[NUM_PATTERNS];

struct sequencer {
	
	enum global_mode mode;
	enum clock_mode clock_mode;
	uint8_t fill_mode:4;
	uint8_t SHIFT:1; //is SHIFT key being held?
	uint8_t START:1; //is sequencer running or not?
	uint8_t CLEAR:1; //is the clear button being held?
	uint8_t TAP_HELD:1; //is the TAP button beign held?
	uint8_t ALT:1; //alternative function mode
	uint8_t shuffle_amount:3;
	uint8_t shuffle_multplier:3; //1 for MIDI and DIN SYNC slave modes, 4 for master modes @ 96 ppqn
	uint8_t new_shuffle_amount:3;
	uint8_t shuffle_ppqn_count:4;//may need to up bit depth for 96ppqn
	uint8_t roll_mode:3;
	struct pattern pattern[2]; //Variation A:0, Variation B: 1
	uint8_t pattern_bank:4;
	uint8_t previous_bank:4; //place holder for bank to return to when cycling through modes back to pattern edit mode
	//uint16_t step_led_mask[2][17];
	uint16_t led_mask;
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
	uint8_t previous_pattern:4; //place holder for pattern being edited when cycling between modes
	uint8_t new_pattern:4;//will need to use this when in manual play and rhythm compose modes
	uint8_t current_intro_fill:4;
	uint8_t current_measure_auto_fill:6; //counter used for counting measures for AUTO FILL INs
	uint8_t track_measure:7; //counter used to count measures during rhythm track play/compose
	uint8_t track_mode:1;
	uint8_t current_rhythm_track:4;
	//struct rhythm_track rhythm_track;
	enum drum current_inst; //this is index of drum_hit struct
	uint8_t var_led_mask;
	uint8_t trigger_1:5; //trigger assignments from AC, BD-CH - 17
	uint8_t trigger_2:5;
	uint8_t midi_channel:4;
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

void show_current_measure(void);

void read_next_pattern(uint8_t pattern_num, uint8_t pattern_bank);
void write_current_pattern(uint8_t pattern_num, uint8_t pattern_bank);

void delete_track_pattern(uint8_t track_num);
void insert_track_pattern(uint8_t track_num);

//void read_next_track_pattern(uint8_t rhythm_track_num, uint8_t pattern_num);
void write_rhythm_track(void);
void read_rhythm_track(void);
void update_rhythm_track(uint8_t track_number);

#endif 