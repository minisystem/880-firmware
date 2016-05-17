/*
 * mode.h
 * JR-808 firmware ATMEGA328PB
 * minisystem
 * system79.com
 */
#ifndef MODE_H
#define MODE_H

#define NUM_MODES 6 //number of modes
enum global_mode {
	
	PATTERN_CLEAR,
	PATTERN_FIRST,
	PATTERN_SECOND,
	MANUAL_PLAY,
	PLAY_RHYTHM,
	COMPOSE_RHYTHM
		
	};

extern uint8_t mode_index; 

void update_mode(void);

#endif