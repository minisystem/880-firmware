/*
 * mode.h
 * Open808 firmware ATMEGA328PB
 * minisystem
 * system80.net
 */
#ifndef MODE_H
#define MODE_H

#define NUM_MODES 6 //number of modes
#define MODE_LED_MASK 0b11000000 //mask for latch 4
#define FILL_MODE_LATCH_4_LED_MASK 0b00111111
#define FILL_MODE_LATCH_2_LED_MASK 0b11110000
#define NUM_FILL_MODES 6
#define NUM_CLOCK_MODES 5
#define MANUAL 0
enum global_mode {
	
	PATTERN_CLEAR,
	FIRST_PART,
	SECOND_PART,
	MANUAL_PLAY,
	PLAY_RHYTHM,
	COMPOSE_RHYTHM
		
	};
	
enum clock_mode {
	
	MIDI_MASTER,
	MIDI_SLAVE,
	DIN_SYNC_MASTER,
	DIN_SYNC_SLAVE,
	PULSE_SYNC_SLAVE
	
	};	
	
enum shift_mode { //not yet implemented, but should be useful for set up
	
	TRIGGER_ASSIGN,
	MIDI_ASSIGN,
	SYNC_NUDGE, //nudge ppq for DIN sync tweaking
	SYNC_DIVIDE //divide incoming MIDI clock
	
	};	


extern uint8_t mode_index;
extern uint8_t fill_index; 

void check_mode_switch(void);
void set_mode(uint8_t mode_index);
void check_fill_switch(void);
void update_fill_leds(uint8_t fill_index);
void set_clock_mode (uint8_t sync_index);
void update_clock_mode_leds(uint8_t sync_index);

#endif