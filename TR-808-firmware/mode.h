#ifndef MODE_H
#define MODE_H

enum global_mode {
	
	PATTERN_CLEAR,
	PATTERN_FIRST,
	PATTERN_SECOND,
	MANUAL_PLAY,
	PLAY_RHYTHM,
	COMPOSE_RHYTHM
		
	};

uint8_t update_mode(void);

#endif