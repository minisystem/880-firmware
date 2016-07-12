/*
 * mode.c
 * JR-808 firmware ATMEGA328PB
 * minisystem
 * system79.com
*/

#include <stdio.h>
#include <avr/io.h>
#include "mode.h"
#include "clock.h"
#include "leds.h"
#include "switches.h"
#include "sequencer.h"
#include "spi.h"

uint8_t mode_index = 1; //default mode is pattern edit 1st part
uint8_t fill_index = 0;
uint8_t sync_index = 0;


	

	
	

void update_mode(void) {
	enum global_mode current_mode[NUM_MODES] = {PATTERN_CLEAR, FIRST_PART, SECOND_PART, MANUAL_PLAY, PLAY_RHYTHM, COMPOSE_RHYTHM};
	if (button[MODE_SW].state) {
		
		button[MODE_SW].state ^= button[MODE_SW].state; //toggle switch state
		
		if (sequencer.SHIFT) {
			
			
			if (mode_index-- == 0) mode_index = NUM_MODES -1;
			
		} else {
			
			 if (++mode_index == NUM_MODES) mode_index = 0;
			 
		}
		
		sequencer.mode = current_mode[mode_index];
		//uint8_t data_mask = spi_data[4] & 0b11000000; //mask to preserve top two bits of SPI byte 4
		spi_data[4] &= MODE_LED_MASK;
		spi_data[4] |= (1<< mode_index);
		
		
		//if (sequencer.step_num[SECOND] != NO_STEPS) sequencer.step_num_new = sequencer.step_num[sequencer.part_editing]; //another annoying except
		if (sequencer.mode == FIRST_PART || sequencer.mode == SECOND_PART) {
			
			sequencer.part_editing = sequencer.mode == FIRST_PART? FIRST : SECOND;
			sequencer.step_num_new = sequencer.step_num[sequencer.part_editing];
			update_step_led_mask(); //want to update led mask immediately, otherwise it only gets updated at end of measure
		}
		
		//update_spi(); //move this out of this function make it part of refresh after all spi output data has been updated
		
	}
	

	
	}
	
void update_fill_mode(void) {
	uint8_t fill_mode[NUM_FILL_MODES] = {MANUAL, 15, 11, 7, 3, 1};
	enum sync_mode sync_mode[4] = {MIDI_MASTER, MIDI_SLAVE, DIN_SYNC_MASTER, DIN_SYNC_SLAVE};
		
	spi_data[4] &= FILL_MODE_LATCH_4_LED_MASK; //clear FILL LED bits
	spi_data[2] &= FILL_MODE_LATCH_2_LED_MASK;

		
	if (sequencer.SHIFT) {

		
		if (button[FILL_SW].state) {
			button[FILL_SW].state ^= button[FILL_SW].state; //toggle switch state
			//change sync mode
			if (++sync_index == NUM_SYNC_MODES) sync_index = 0;
			sequencer.sync_mode = sync_mode[sync_index];
			TCCR2B = 0; //turn off Timer2			
			//spi_data[2] |= (1 << fill_index);
			switch (sequencer.sync_mode) {
							
				case MIDI_MASTER:
					clock.source = INTERNAL;
					break;
							
				case MIDI_SLAVE:
					clock.source = EXTERNAL;
					break;
							
				case DIN_SYNC_MASTER:
					clock.source = INTERNAL;
					
					DDRD |= (1 << DIN_CLOCK | 1 << DIN_RUN_STOP | 1 << DIN_FILL | 1 << DIN_RESET); //set up DIN pins as outputs
					//TCCR2A |= (1 << COM2B0); //toggle OC2B/PD3 on compare match
					TCCR2A |= (1 << WGM21); //clear timer on OCRA compare match where OCRA = OCRB
					TCCR2B |= (1<<CS22) | (1<<CS21) | (1<<CS20);
					TCNT2 = 0;
					
					break;
							
				case DIN_SYNC_SLAVE:
					clock.source = EXTERNAL;
					//TCCR1B = 0; //stop master tempo timer - necessary?
					
					break;
							
							
			}
			
		}
		
		spi_data[2] |= (1 << sync_index);
		
	} else {
		
		if (button[FILL_SW].state) {

			button[FILL_SW].state ^= button[FILL_SW].state; //toggle switch state
			spi_data[4] &= FILL_MODE_LATCH_4_LED_MASK; //clear FILL LED bits
			spi_data[2] &= FILL_MODE_LATCH_2_LED_MASK;	
					
			if (++fill_index == NUM_FILL_MODES) fill_index = 0;
			
			sequencer.fill_mode = fill_mode[fill_index];
			sequencer.current_measure = 0;			
		}
		if (fill_index < 2) {
				
			spi_data[4] |= 1 << (fill_index + 6);
				
			} else {
				
			spi_data[2] |= 1 << (fill_index - 2);
		}
		
		
	}
}
		
	//if (button[FILL_SW].state) {
//
		//button[FILL_SW].state ^= button[FILL_SW].state; //toggle switch state
		//
		//if (sequencer.SHIFT) {
	//
			////change sync mode
			//if (++sync_index == NUM_SYNC_MODES) sync_index = 0;
			//sequencer.sync_mode = sync_mode[sync_index];
			//
			//spi_data[2] |= (1 << fill_index);
			//switch (sequencer.sync_mode) {
			//
				//case MIDI_MASTER:
					//clock.source = INTERNAL;
					//break;
				//
				//case MIDI_SLAVE:
					//clock.source = EXTERNAL;
					//break;
				//
				//case DIN_SYNC_MASTER:
					//clock.source = INTERNAL;
					//break;
				//
				//case DIN_SYNC_SLAVE:
					//clock.source = EXTERNAL;
					//break;
			//
				//
			//}
					//
	//
		//} else {
			//
			//if (++fill_index == NUM_FILL_MODES) fill_index = 0;
			//
			//sequencer.fill_mode = fill_mode[fill_index];
			//sequencer.current_measure = 0;
			//
//
			//
			//if (fill_index < 2) {
				//
				//spi_data[4] |= 1 << (fill_index + 6);
				//
				//} else {
				//
				//spi_data[2] |= 1 << (fill_index - 2);
			//}
		//}
		//
	//}	
	
//}
	
	