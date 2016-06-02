/*
 * mode.c
 * JR-808 firmware ATMEGA328PB
 * minisystem
 * system79.com
*/

#include <stdio.h>
#include <avr/io.h>
#include "mode.h"
#include "leds.h"
#include "switches.h"
#include "sequencer.h"
#include "spi.h"

uint8_t mode_index = 0; 

enum global_mode current_mode[6] = {PATTERN_CLEAR, FIRST_PART, SECOND_PART, MANUAL_PLAY, PLAY_RHYTHM, COMPOSE_RHYTHM};

void update_mode(void) {
	
	if (button[MODE_SW].state) {
		
		button[MODE_SW].state ^= button[MODE_SW].state; //toggle switch state
		
		if (sequencer.SHIFT) {
			
			
			if (mode_index-- == 0) mode_index = NUM_MODES -1;
			
		} else {
			
			 if (++mode_index == NUM_MODES) mode_index = 0;
			 
		}
		
		sequencer.mode = current_mode[mode_index];
		uint8_t data_mask = spi_data[4] & 0b11000000; //mask to preserve top two bits of SPI byte 4
		spi_data[4] = (1<< mode_index) | data_mask; 
		
		
		//if (sequencer.step_num[SECOND] != NO_STEPS) sequencer.step_num_new = sequencer.step_num[sequencer.part_editing]; //another annoying except
		if (sequencer.mode == FIRST_PART || sequencer.mode == SECOND_PART) {
			
			sequencer.part_editing = sequencer.mode == FIRST_PART? FIRST : SECOND;
			sequencer.step_num_new = sequencer.step_num[sequencer.part_editing];
			update_step_led_mask(); //want to update led mask immediately, otherwise it only gets updated at end of measure
		}
		
		//update_spi(); //move this out of this function make it part of refresh after all spi output data has been updated
		
	}
	

	
	};
	
	