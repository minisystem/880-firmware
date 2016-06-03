/*
 * mode.h
 * JR-808 firmware ATMEGA328PB
 * minisystem
 * system79.com
 */
#ifndef MODE_H
#define MODE_H

#define NUM_MODES 6 //number of modes
#define MODE_LED_MASK 0b11000000
enum global_mode {
	
	PATTERN_CLEAR,
	FIRST_PART,
	SECOND_PART,
	MANUAL_PLAY,
	PLAY_RHYTHM,
	COMPOSE_RHYTHM
		
	};

extern uint8_t mode_index; 

void update_mode(void);

#endif