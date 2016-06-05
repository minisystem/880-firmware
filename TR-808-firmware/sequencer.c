/*
 * sequencer.c
 * JR-808 firmware ATMEGA328PB
 * minisystem
 * system79.com
 */
#include <stdio.h>
#include <avr/io.h>
#include "sequencer.h"
#include "mode.h"
#include "leds.h"
#include "switches.h"
#include "hardware.h"
#include "clock.h"
#include "drums.h"
#include "adc.h"
#include "spi.h"

struct sequencer sequencer;
struct flag flag;

uint8_t pre_scale_index = 1; //default is 4/4, so PRE_SCALE_3
uint8_t pre_scale[4] = {PRE_SCALE_4, PRE_SCALE_3, PRE_SCALE_2, PRE_SCALE_1};

void update_tempo(void) {
	static uint16_t new_tempo_adc = 0;
	static uint16_t current_tempo_adc = 0;
	int tempo_adc_change = 0;
	new_tempo_adc = read_tempo_pot();
	tempo_adc_change = new_tempo_adc - current_tempo_adc;
	current_tempo_adc = current_tempo_adc + (tempo_adc_change >>2);
	
	internal_clock.rate = (1023 - current_tempo_adc) + TIMER_OFFSET; //offset to get desirable tempo range

	if (internal_clock.rate != internal_clock.previous_rate) {
		
		update_clock_rate(internal_clock.rate);
		
	}
	
	internal_clock.previous_rate = internal_clock.rate;
	
}

//uint8_t step_mask(void) {
	//
	//uint8_t step_mask = 255;
		//
	//if (sequencer.step_num_first < 8) {
		//
		//step_mask = step_mask >> (7-sequencer.step_num_first);
//
	//} else if (sequencer.step_num_first < 16) {
		//
		//
	//}
	//
	//return step_mask;
//}

void process_step(void) {
	
		if (sequencer.START) { //this is an effort to synchronize SPI update within main loop - basically manipulate SPI data bytes and then do one single update_spi() call per loop
			
			//if (sequencer.part_playing == FIRST || sequencer.part_playing == SECOND) {	
				if (flag.next_step) {
					flag.next_step = 0;
					while(flag.trig_finished == 0); //make sure previous instrument trigger is finished before initiating next one
					
					check_tap();
					PORTD |= (1<<TRIG);
					
					
					if (sequencer.part_editing == sequencer.part_playing) {	//only blink if the part playing is the same as the part being edited
						spi_data[1] = (1 << sequencer.current_step) | sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst];
						spi_data[1] &= ~(sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst] & (1<<sequencer.current_step));
						spi_data[0] = ((1 << sequencer.current_step) >> 8) | (sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst] >> 8);
						spi_data[0] &= ~((sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst]>>8) & ((1<<sequencer.current_step) >>8));
					} else {
						
						
					}

					trigger_step();
					if ((sequencer.pattern[sequencer.variation].accent[sequencer.part_playing] >> sequencer.current_step) &1) {
						spi_data[8] |= 1<<ACCENT;
						if (!sequencer.SHIFT) turn_on(ACCENT_1_LED);
					}
					TIMSK0 |= (1<<OCIE0A); //enable output compare match A
					TCCR0B |= (1<<CS01) | (1<<CS00); //set to /64 of system clock start timer
					flag.trig_finished = 0;
				
					} else {
				
				}
			//} else {//if (sequencer.part_playing == SECOND) {
				
				//handle patterns >16 steps here, or maybe not?

				
			//}
				
		} else if (flag.next_step){
			
			flag.next_step = 0;
			//spi_data[1] = 0;
			//spi_data[0] = 0;
			//turn_on(STEP_1_LED);
			
		}
}

void update_step_board() {
	
	if (sequencer.START) {
		
			switch (sequencer.mode) {		
			
			case FIRST_PART: case SECOND_PART:
				
				if (sequencer.CLEAR) { //clear button is pressed, check if step buttons are pressed and change step number accordingly
					
					for (int i = 0; i < NUM_STEPS; i++) {
						
							if (button[i].state) {
								
								button[i].state ^= button[i].state;
								sequencer.step_num_new = i;
								break;// - should we break out of here? multiple presses will mess things up, right?
							}
						
					}
					
					break; //break or return? or is it needed?
				}
				
				
				if (sequencer.current_inst == AC) { //bah, inefficient duplicate code to handle ACCENT
			
					for (int i = 0; i < NUM_STEPS; i++) { //button and led indices match for 0-15. How convenient. Will need to use offset of 16 for steps 17-32 of SECOND_PART
				
						if (button[i].state) {
					
							
							button[i].state ^= button[i].state;
							if (i <= sequencer.step_num[sequencer.part_editing]) { //need handle all button presses, but only use presses that are below current step number
								toggle(i);
								sequencer.pattern[sequencer.variation].accent[sequencer.part_editing] ^= 1<<i; 
								sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst] ^= 1<<i;
							}
						}
					}
					return;
				}
				for (int i = 0; i < NUM_STEPS; i++) { //button and led indices match for 0-15. How convenient.
			
					if (button[i].state) {
						
						
						button[i].state ^= button[i].state;
						if (i <= sequencer.step_num[sequencer.part_editing]) {
							toggle(i);
							sequencer.pattern[sequencer.variation].part[sequencer.part_editing][i] ^= 1<<sequencer.current_inst;
							sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst] ^= 1<<i;
						}
					}
				}
				break;
			
			case MANUAL_PLAY:
			
				break;
				
			case PLAY_RHYTHM:
			
				break;
				
			case COMPOSE_RHYTHM:
			
				break;
				
			case PATTERN_CLEAR:
			
				break;		 			
			}
			
	} else {
		
		//handle changing selected pattern and rhythm. Not currently handling switches presses now when sequencer is stopped, which means they get added once sequencer starts
		
	}
}

void update_variation(void) { //not currently used 
	
	if (flag.new_measure) {
		
		if (flag.variation_change == 1) {
			flag.variation_change = 0;
			switch (sequencer.variation_mode) {
							
				case VAR_A: case VAR_AB:
				sequencer.variation = VAR_A;
				break;
				case VAR_B:
				sequencer.variation = VAR_B;
				break;
							
							
			}
						
			} else if (sequencer.variation_mode == VAR_AB) {
						
			sequencer.variation ^= 1<<0; //toggle state
		}
		
	}
	
	
		if (flag.half_step) {
			
			
			
			
		}
	
}

void update_prescale(void) {
	
	if (button[BASIC_VAR_A_SW].state && sequencer.SHIFT) {
	
		button[BASIC_VAR_A_SW].state ^= button[BASIC_VAR_A_SW].state; //toggle switch state
		
		if (pre_scale_index-- == 0) { //decrement to go from 3 to 4 to 1 to 2 to 3...
			
			pre_scale_index = NUM_PRE_SCALES -1;
					
		}
		flag.pre_scale_change = 1;
		spi_data[5] &= PRE_SCALE_LED_MASK; //clear pre-scale LED bits
		spi_data[5] |= (1<< (pre_scale_index +2)); //need 2 bit offset on latch 5 (pre-scale leds are bit 2-5)

	}
}

void check_tap(void) {
	
	if (flag.tap) {
		
		flag.tap = 0;
		if (sequencer.current_inst == AC) {
			sequencer.pattern[sequencer.variation].accent[sequencer.part_editing] |= 1<<sequencer.current_step;	
		} else {
			sequencer.pattern[sequencer.variation].part[sequencer.part_editing][sequencer.current_step] |= 1<<sequencer.current_inst;
		}
		sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst] |= 1<<sequencer.current_step;
		
	}
	
	
}

void toggle_variation(void) {
	
	if (flag.variation_change == 1) {
		flag.variation_change = 0;
		switch (sequencer.variation_mode) {
					
			case VAR_A: case VAR_AB:
			sequencer.variation = VAR_A;
			break;
			case VAR_B:
			sequencer.variation = VAR_B;
			break;
					
					
		}
				
		} else if (sequencer.variation_mode == VAR_AB) {
				
		sequencer.variation ^= 1<<0; //toggle state
	}	
	
	
}