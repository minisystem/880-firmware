/*
 * sequencer.h
 * JR-808 firmware ATMEGA328PB
 * minisystem
 * system79.com
 */

#include "mode.h"

#ifndef SEQUENCER_H
#define SEQUENCER_H

struct sequencer {
	
	enum global_mode mode;
	uint8_t SHIFT:1;
	
	
	};

extern struct sequencer sequencer;


#endif 