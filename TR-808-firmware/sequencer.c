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
volatile struct flag flag;

uint8_t pre_scale_index = 1; //default is 4/4, so PRE_SCALE_3
uint8_t pre_scale[4] = {PRE_SCALE_4, PRE_SCALE_3, PRE_SCALE_2, PRE_SCALE_1};

void update_tempo(void) {
	static uint16_t new_tempo_adc = 0;
	static uint16_t current_tempo_adc = 0;
	int tempo_adc_change = 0;
	new_tempo_adc = read_tempo_pot();
	tempo_adc_change = new_tempo_adc - current_tempo_adc;
	current_tempo_adc = current_tempo_adc + (tempo_adc_change >>2);
	
	clock.rate = (1023 - current_tempo_adc) + TIMER_OFFSET; //offset to get desirable tempo range

	if (clock.rate != clock.previous_rate) {
		
		update_clock_rate(clock.rate);
		
	}
	
	clock.previous_rate = clock.rate;
	
}

void process_tick(void) {

		if (++clock.ppqn_counter == clock.divider) {
			flag.next_step = 1;
			if (sequencer.current_step++ == sequencer.step_num[sequencer.part_playing] && sequencer.START) flag.new_measure = 1;
			clock.beat_counter++; //overflows every 4 beats
			clock.ppqn_counter = 0;
		} else if (clock.ppqn_counter == clock.divider >> 1) { //50% step width, sort of - this is going to get long and complicated fast - need to set flag and handle in main loop refresh function
				
			flag.half_step = 1;

		}
	
}

void process_start(void) {
	
		sequencer.current_step = 0;
		flag.next_step = 1;
		//flag.new_measure = 1;
		clock.ppqn_counter = 0;
			
		flag.variation_change = 0;
		if (sequencer.variation_mode == VAR_A || sequencer.variation_mode == VAR_AB) {
				
			sequencer.variation = VAR_A; //start on variation A
			} else {
				
			sequencer.variation = VAR_B;
		}

}

void process_stop(void) {
	
		if (sequencer.part_playing == SECOND) { //reset part playing
			sequencer.part_playing = FIRST;
			turn_off(SECOND_PART_LED);
			turn_on (FIRST_PART_LED);
				
		}
		turn_off_all_inst_leds();
		turn_on(drum_hit[sequencer.current_inst].led_index);
			
		//blank all step leds and turn on current pattern LED
		spi_data[1] = 0;
		spi_data[0] = 0;
		turn_on(STEP_1_LED);	
	
}
void process_step(void) {
	
	//if (sequencer.START) { //this is an effort to synchronize SPI update within main loop - basically manipulate SPI data bytes and then do one single update_spi() call per loop
			
	if (flag.next_step) {
		flag.next_step = 0;
		if (sequencer.START) {
		//*************************TAKEN FROM INTERRUPT*****************************//
			if (flag.new_measure) {

				flag.new_measure = 0;
				sequencer.current_step = 0;
				if (sequencer.step_num[SECOND] != NO_STEPS) { //no toggling if second part has 0 steps - annoying exception handler
								
					if (sequencer.part_playing == SECOND) {
						turn_off(SECOND_PART_LED);
						turn_on(FIRST_PART_LED);
						toggle_variation(); //only toggle variation at the end of the 2nd part
						} else {
						turn_off(FIRST_PART_LED);
						turn_on(SECOND_PART_LED);
					}
					sequencer.part_playing ^= 1 << 0;
					} else {
								
					toggle_variation(); //no second part, so toggle variation
								
				}
				//update step number
				sequencer.step_num[sequencer.part_editing] = sequencer.step_num_new;
				update_step_led_mask();
							
				//handle pre-scale change
				if (flag.pre_scale_change) {
					flag.pre_scale_change = 0;
					clock.divider = pre_scale[pre_scale_index];
				}
				//sequencer.current_measure++;

			}			

			//*************************************************************************//
			while(trigger_finished == 0); //make sure previous instrument trigger is finished before initiating next one - this really only applies when there is incoming MIDI data. May have to do away
			//with allowing drums to be triggered by MIDI when sequencer is running?
					
			check_tap();
			//PORTD |= (1<<TRIG);
						
			if (sequencer.part_editing == sequencer.part_playing) {	//only blink if the part playing is the same as the part being edited
				spi_data[1] = (1 << sequencer.current_step) | sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst];
				spi_data[1] &= ~(sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst] & (1<<sequencer.current_step));
				spi_data[0] = ((1 << sequencer.current_step) >> 8) | (sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst] >> 8);
				spi_data[0] &= ~((sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst]>>8) & ((1<<sequencer.current_step) >>8));
			}

			trigger_step();
			if ((sequencer.pattern[sequencer.variation].accent[sequencer.part_playing] >> sequencer.current_step) &1) {
				spi_data[8] |= 1<<ACCENT;
				if (!sequencer.SHIFT) turn_on(ACCENT_1_LED);
			}
		}
	} else if (flag.half_step) {
				
		flag.half_step = 0;
		turn_off_all_inst_leds();
		if (!sequencer.SHIFT) turn_on(drum_hit[sequencer.current_inst].led_index);
		spi_data[5] &= ~(led[BASIC_VAR_A_LED].spi_bit | led[BASIC_VAR_B_LED].spi_bit); //this clears basic variation LEDs
		if (sequencer.START) {
					
			spi_data[1] = sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst]; //this keeps inst lights on while blinking step light
			spi_data[0] = sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst] >> 8;
					
			switch (sequencer.variation_mode) {
						
				case VAR_A:
				sequencer.var_led_mask = led[BASIC_VAR_A_LED].spi_bit;
				break;
				case VAR_B:
				if (flag.variation_change == 1) {
							
					sequencer.var_led_mask = led[BASIC_VAR_A_LED].spi_bit;
							
					}else {
					sequencer.var_led_mask = led[BASIC_VAR_B_LED].spi_bit;
				}
				break;
				case VAR_AB:
				if (sequencer.variation == VAR_A) {
					sequencer.var_led_mask = led[BASIC_VAR_A_LED].spi_bit;
					} else {
					sequencer.var_led_mask = led[BASIC_VAR_B_LED].spi_bit;
				}
				break;
			}
					
			if (clock.beat_counter <2) {
						
				if (flag.variation_change == 1) {
							
					switch (sequencer.variation_mode) {
								
						case VAR_A:
						sequencer.var_led_mask |= led[BASIC_VAR_B_LED].spi_bit;
						break;
						case VAR_B:
						if (flag.variation_change == 1) {
							sequencer.var_led_mask |= led[BASIC_VAR_B_LED].spi_bit;
							} else {
							sequencer.var_led_mask |= led[BASIC_VAR_A_LED].spi_bit;
						}
						break;
						case VAR_AB:
						if (sequencer.variation == VAR_A) {
							sequencer.var_led_mask |= led[BASIC_VAR_B_LED].spi_bit;
							} else {
							sequencer.var_led_mask |= led[BASIC_VAR_A_LED].spi_bit;
						}
						break;
					}
							
							
				}
						
				if (sequencer.variation_mode == VAR_AB) {
					if (sequencer.variation == VAR_A) {
						sequencer.var_led_mask |= led[BASIC_VAR_B_LED].spi_bit;
						} else {
						sequencer.var_led_mask |= led[BASIC_VAR_A_LED].spi_bit;
					}
				}
			}
					
			} else {
					
			spi_data[1] = 0;
			spi_data[0] = 0;
					
			switch (sequencer.variation_mode) {
						
				case VAR_A: case VAR_AB:
				sequencer.var_led_mask = led[BASIC_VAR_A_LED].spi_bit;
				break;
						
				case VAR_B:
				sequencer.var_led_mask = led[BASIC_VAR_B_LED].spi_bit;
				break;
						
			}
					
			if (clock.beat_counter <2) { //1/8 note, regardless of scale (based on original 808 behavior) - don't take this as gospel. may need to adjust with different pre-scales
						

				if (sequencer.variation_mode == VAR_AB) sequencer.var_led_mask |= led[BASIC_VAR_B_LED].spi_bit;	//turn on VAR_B LED for flashing to indicate A/B mode
						

						
				turn_on(STEP_1_LED); //eventually need to turn on current pattern LED in pattern mode - other modes will require different behavior to be coded
			}
		}
				
		//spi_data[5] |= sequencer.var_led_mask;
				
	} else if (clock.source == EXTERNAL && !sequencer.START) { //this handles variation LEDs when waiting for external clock signal
		spi_data[5] &= ~(led[BASIC_VAR_A_LED].spi_bit | led[BASIC_VAR_B_LED].spi_bit); //this clears basic variation LEDs
		switch (sequencer.variation_mode) {
			
			case VAR_A:
				sequencer.var_led_mask = led[BASIC_VAR_A_LED].spi_bit;
				break;
				
			case VAR_B:
				sequencer.var_led_mask = led[BASIC_VAR_B_LED].spi_bit;
				break;
				
			case VAR_AB:
				sequencer.var_led_mask = led[BASIC_VAR_A_LED].spi_bit | led[BASIC_VAR_B_LED].spi_bit;
				break;		
			
		}
		
	}		
	spi_data[5] |= sequencer.var_led_mask;
}
//} else if (flag.next_step) { //sequencer not running, but next_step flag has occurred
	//flag.next_step = 0;
		//
//}
		
//}

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