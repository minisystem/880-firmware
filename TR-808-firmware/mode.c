/*
 * mode.c
 * Open808 firmware ATMEGA328PB
 * minisystem
 * system80.net
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
		spi_data[LATCH_4] &= MODE_LED_MASK;
		spi_data[LATCH_4] |= (1<< mode_index);
		
		
		//if (sequencer.step_num[SECOND] != NO_STEPS) sequencer.step_num_new = sequencer.step_num[sequencer.part_editing]; //another annoying except
		
		//should eventually implement a switch case statement here to handle rhyth write and play modes
		if (sequencer.mode == FIRST_PART || sequencer.mode == SECOND_PART) {
			
			sequencer.part_editing = sequencer.mode == FIRST_PART? FIRST : SECOND;
			sequencer.step_num_new = sequencer.step_num[sequencer.part_editing];
			update_step_led_mask(); //want to update led mask immediately, otherwise it only gets updated at end of measure
		} else if (sequencer.mode == MANUAL_PLAY) {
			//if current pattern is 12 or greater then it needs to be set to 12 to avoid overlap with intro/fill pattern selection - this mimics original TR-808 behaviour
			if (sequencer.current_pattern > 11) { //make 11 a constant here
				sequencer.new_pattern = 11;
				flag.pattern_change = 1;
			}
			
		} else if (sequencer.mode == PLAY_RHYTHM) {
			//will need to update inst/track leds to current rhythm track, rather than resetting to 0
			read_rhythm_track();
			//flag.pattern_change = 1;
			sequencer.track_measure = 0;
			sequencer.current_pattern = sequencer.new_pattern = rhythm_track.patterns[sequencer.track_measure].current_pattern;
			sequencer.pattern_bank = rhythm_track.patterns[sequencer.track_measure].current_bank;
			if (sequencer.START) {
				flag.pattern_change = 1;
				} else {
				read_next_pattern(sequencer.current_pattern, sequencer.pattern_bank);
			}		
		}
		
		//update_spi(); //move this out of this function make it part of refresh after all spi output data has been updated
		
	}
	

	
	}
	
void update_fill_mode(void) {
	uint8_t fill_mode[NUM_FILL_MODES] = {MANUAL, 15, 11, 7, 3, 1};
	enum clock_mode clock_mode[4] = {MIDI_MASTER, MIDI_SLAVE, DIN_SYNC_MASTER, DIN_SYNC_SLAVE};
		
	spi_data[LATCH_4] &= FILL_MODE_LATCH_4_LED_MASK; //clear FILL LED bits
	spi_data[LATCH_2] &= FILL_MODE_LATCH_2_LED_MASK;

		
	if (sequencer.SHIFT) {

		
		if (button[FILL_SW].state) {
			button[FILL_SW].state ^= button[FILL_SW].state; //toggle switch state
			//change sync mode
			if (++sync_index == NUM_CLOCK_MODES) sync_index = 0;
			sequencer.clock_mode = clock_mode[sync_index];
			TCCR2B = 0; //turn off Timer2
			EIMSK = 0; //turn off external interrupts
			PCICR = 0; //turn off pin change interrupts			
			//spi_data[2] |= (1 << fill_index);
			switch (sequencer.clock_mode) {
							
				case MIDI_MASTER:
					clock.source = INTERNAL;
					//TIMSK3 &= ~(1<<OCIE3A); //disable timer3 interrupts
					//sequencer.shuffle_multplier = 4;
					PORTC &= ~(1<<SYNC_LED_Y);
					PORTE &= ~(1<<SYNC_LED_R);
					break;
							
				case MIDI_SLAVE:
					clock.source = EXTERNAL;
					//TIMSK3 |= (1<<OCIE3A); //turn on timer3 output compare interrupt
					TCNT3 = 0; //reset timer3
					//sequencer.shuffle_multplier = 1;
					PORTE |= (1<<SYNC_LED_R);
					break;
							
				case DIN_SYNC_MASTER:
					clock.source = INTERNAL;
					//TIMSK3 &= ~(1<<OCIE3A); //disable timer3 interrupts
					//sequencer.shuffle_multplier = 4;
					PORTD |= (1<<SYNC_LED_R);
					DDRD |= (1 << DIN_CLOCK | 1 << DIN_RUN_STOP | 1 << DIN_FILL | 1 << DIN_RESET); //set up DIN pins as outputs
					//TCCR2A |= (1 << COM2B0); //toggle OC2B/PD3 on compare match
					TCCR2A |= (1 << WGM21); //clear timer on OCRA compare match where OCRA = OCRB
					TCCR2B |= (1<<CS22) | (1<<CS21) | (1<<CS20); 
					TCNT2 = 0;
					
					break;
							
				case DIN_SYNC_SLAVE:
					clock.source = EXTERNAL;
					//TIMSK3 |= (1<<OCIE3A); //turn on timer3 output compare interrupt
					TCNT3 = 0; //rest timer3
					//sequencer.shuffle_multplier = 1;
					PORTE &= ~(1<<SYNC_LED_R);
					DDRD &= ~((1 << DIN_CLOCK | 1 << DIN_RUN_STOP | 1 << DIN_FILL | 1 << DIN_RESET)); //set up DIN pins as inputs
					EIMSK |= (1 << INT1) | (1<< INT0); //turn on INT1 interrupt for DIN Sync clock and INT0 for external SYNC input jack
					PCICR |= (1 << PCIE2); //turn on pin change interrupt for what?
					
					
					
					//TCCR1B = 0; //stop master tempo timer - necessary?
					
					break;
							
							
			}
			
		}
		
		spi_data[LATCH_2] |= (1 << sync_index);
		
	} else {
		
		if (button[FILL_SW].state) {

			button[FILL_SW].state ^= button[FILL_SW].state; //toggle switch state
			spi_data[LATCH_4] &= FILL_MODE_LATCH_4_LED_MASK; //clear FILL LED bits
			spi_data[LATCH_2] &= FILL_MODE_LATCH_2_LED_MASK;	
					
			if (++fill_index == NUM_FILL_MODES) fill_index = 0;
			
			sequencer.fill_mode = fill_mode[fill_index];
			sequencer.current_measure_auto_fill = 0;			
		}
		if (fill_index < 2) {
				
			spi_data[LATCH_4] |= 1 << (fill_index + 6);
				
			} else {
				
			spi_data[LATCH_2] |= 1 << (fill_index - 2);
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
			//if (++sync_index == NUM_CLOCK_MODES) sync_index = 0;
			//sequencer.clock_mode = clock_mode[sync_index];
			//
			//spi_data[2] |= (1 << fill_index);
			//switch (sequencer.clock_mode) {
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
	
	