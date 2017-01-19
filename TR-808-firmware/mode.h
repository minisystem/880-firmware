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
#define NUM_SYNC_MODES 4
#define MANUAL 0
enum global_mode {
	
	PATTERN_CLEAR,
	FIRST_PART,
	SECOND_PART,
	MANUAL_PLAY,
	PLAY_RHYTHM,
	COMPOSE_RHYTHM
		
	};
	
enum sync_mode {
	
	MIDI_SLAVE,
	MIDI_MASTER,
	DIN_SYNC_SLAVE,
	DIN_SYNC_MASTER
	
	};	
	


extern uint8_t mode_index;
extern uint8_t fill_index; 

void update_mode(void);
void update_fill_mode(void);

#endif