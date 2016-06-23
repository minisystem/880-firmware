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
#include "midi.h"
#include "twi_eeprom.h"
#include "xnormidi-develop/midi.h"
#include "xnormidi-develop/midi_device.h"
#include "xnormidi-develop/bytequeue/bytequeue.h"

struct sequencer sequencer;
volatile struct flag flag;
//pattern_data next_pattern;
//uint8_t pre_scale_index = 1; //default is 4/4, so PRE_SCALE_3
uint8_t pre_scale[4] = {PRE_SCALE_4, PRE_SCALE_3, PRE_SCALE_2, PRE_SCALE_1};

void update_tempo(void) {
	static uint16_t new_tempo_adc = 0;
	static uint16_t current_tempo_adc = 0;
	int tempo_adc_change = 0;
	new_tempo_adc = read_tempo_pot();
	tempo_adc_change = new_tempo_adc - current_tempo_adc;
	current_tempo_adc = current_tempo_adc + (tempo_adc_change >>2);
	
	clock.rate = (ADC_MAX - current_tempo_adc) + TIMER_OFFSET; //offset to get desirable tempo range

	if (clock.rate != clock.previous_rate) {
		
		update_clock_rate(clock.rate);
		
	}
	
	clock.previous_rate = clock.rate;
	
}

void process_tick(void) {

		if (++clock.ppqn_counter == clock.divider) {
			flag.next_step = 1;
			///toggle(IF_VAR_A_LED);
			if (sequencer.current_step++ == sequencer.step_num[sequencer.part_playing] && sequencer.START) {
				//toggle(IF_VAR_B_LED);	
				flag.new_measure = 1;
			}
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
		if (clock.source == INTERNAL) {
			
			midi_send_start(&midi_device);
			midi_send_clock(&midi_device);
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
		turn_on(sequencer.current_pattern);	
		if (clock.source == INTERNAL) midi_send_stop(&midi_device);
	
}
void process_step(void) {
	//uint8_t pre_scale[NUM_PRE_SCALES] = {PRE_SCALE_4, PRE_SCALE_3, PRE_SCALE_2, PRE_SCALE_1};		
	if (flag.next_step) {
		
		flag.next_step = 0;
		//spi_data[1] = 0;
		//spi_data[0] = 0;
		if (sequencer.START) {
		//*************************TAKEN FROM INTERRUPT*****************************//
			if (flag.new_measure) {
			
				flag.new_measure = 0;
				sequencer.current_step = 0;
				//toggle(IF_VAR_B_LED);
				if (flag.pattern_edit == 1) {
					
					flag.pattern_edit = 0;
					toggle(IF_VAR_B_LED);
					write_current_pattern(sequencer.current_pattern); //save changed pattern at end of measure
					
				}
				

				
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
				
				if (flag.pattern_change) {
					
					flag.pattern_change = 0;
					flag.pre_scale_change = 1; //need to handle any change in pre-scale
					sequencer.current_pattern = sequencer.new_pattern;
					read_next_pattern(sequencer.current_pattern);
					sequencer.variation = VAR_A;
					sequencer.part_playing = FIRST;
					turn_off(SECOND_PART_LED);
					turn_on(FIRST_PART_LED);
					
				}				
				
				
				//update step number
				sequencer.step_num[sequencer.part_editing] = sequencer.step_num_new;
				update_step_led_mask();
							
				//handle pre-scale change
				if (flag.pre_scale_change) {
					flag.pre_scale_change = 0;
					
					clock.divider = pre_scale[sequencer.pre_scale];
				}
				//sequencer.current_measure++;

			}			

		//*************************************************************************//
			while(trigger_finished == 0); //make sure previous instrument trigger is finished before initiating next one - this really only applies when there is incoming MIDI data. May have to do away
			//with allowing drums to be triggered by MIDI when sequencer is running?
					
			check_tap();
			//PORTD |= (1<<TRIG);
			if (sequencer.SHIFT) { //shift allows display of current pattern on step board while sequencer is running
				//spi_data[1] = 0;
				//spi_data[0] = 0;
				//turn_on(sequencer.current_pattern);
			} else {
				if (sequencer.part_editing == sequencer.part_playing) {	//only blink if the part playing is the same as the part being edited
					spi_data[1] = (1 << sequencer.current_step) | sequencer.step_led_mask[sequencer.variation][sequencer.current_inst];
					spi_data[1] &= ~(sequencer.step_led_mask[sequencer.variation][sequencer.current_inst] & (1<<sequencer.current_step));
					spi_data[0] = ((1 << sequencer.current_step) >> 8) | (sequencer.step_led_mask[sequencer.variation][sequencer.current_inst] >> 8);
					spi_data[0] &= ~((sequencer.step_led_mask[sequencer.variation][sequencer.current_inst]>>8) & ((1<<sequencer.current_step) >>8));
				}
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
			spi_data[1] = 0;
			spi_data[0] = 0;
			if (sequencer.SHIFT) {
				//spi_data[1] = 0;
				//spi_data[0] = 0;
				//turn_on(sequencer.new_pattern);	
			} else {	
				spi_data[1] = sequencer.step_led_mask[sequencer.variation][sequencer.current_inst]; //this keeps inst lights on while blinking step light
				spi_data[0] = sequencer.step_led_mask[sequencer.variation][sequencer.current_inst] >> 8;
			}
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
				if (sequencer.SHIFT) {
					
					turn_on(sequencer.new_pattern);
						
				}
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
					turn_on(sequencer.new_pattern); //eventually need to turn on current pattern LED in pattern mode - other modes will require different behavior to be coded
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


void update_step_board() {
	uint8_t press = EMPTY;
	if (sequencer.START) {
		
		switch (sequencer.mode) { 
			
		case FIRST_PART: case SECOND_PART:
			press = check_step_press();
			if (press == EMPTY) break;
			
			if (sequencer.SHIFT) {		
				flag.pattern_change = 1;
				sequencer.new_pattern = press;
				//turn_off(sequencer.current_pattern);
				//turn_on(sequencer.new_pattern);
				break;			
			}

			if (sequencer.CLEAR) { //clear button is pressed, check if step buttons are pressed and change step number accordingly
				//press = check_step_press();
				sequencer.step_num_new = press;
				break; //break or return?
			}			
				
			if (sequencer.current_inst == AC) { //bah, inefficient duplicate code to handle ACCENT
				//press = check_step_press();
				//if (press != EMPTY)	{
				if (press <= sequencer.step_num[sequencer.part_editing]) { //need handle all button presses, but only use presses that are below current step number
					toggle(press);
					sequencer.pattern[sequencer.variation].accent[sequencer.part_editing] ^= 1<<press;
					sequencer.step_led_mask[sequencer.variation][sequencer.current_inst] ^= 1<<press;
					flag.pattern_edit = 1;
				}
						
				//}

				return; //break or return?
			}

			//press = check_step_press();
			//if (press != EMPTY)	{
			if (press <= sequencer.step_num[sequencer.part_editing]) {
				toggle(press);
				sequencer.pattern[sequencer.variation].part[sequencer.part_editing][press] ^= 1<<sequencer.current_inst;
				sequencer.step_led_mask[sequencer.variation][sequencer.current_inst] ^= 1<<press;
				flag.pattern_edit = 1;
				//toggle(IF_VAR_B_LED);
			//}					
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
		press = check_step_press();
		if (press != EMPTY) {
			sequencer.current_pattern = sequencer.new_pattern = press;
			read_next_pattern(sequencer.current_pattern);
			//sequencer.variation = VAR_A;
			sequencer.part_playing = FIRST;
			sequencer.current_step = 0;
			clock.ppqn_counter = 0; //need to reset ppqn_counter here. there's a glitch when switching to new patterns that can somehow cause overflow and next_step and half_step flags aren't set
			clock.beat_counter = 0;


		}

		
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
		
		if (sequencer.pre_scale-- == 0) { //decrement to go from 3 to 4 to 1 to 2 to 3...
			
			sequencer.pre_scale = NUM_PRE_SCALES -1;
					
		}
		flag.pre_scale_change = flag.pattern_edit = 1;
		update_prescale_leds();

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
		sequencer.step_led_mask[sequencer.variation][sequencer.current_inst] |= 1<<sequencer.current_step;
		flag.pattern_edit = 1; //set pattern edit flag
		
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

void read_next_pattern(uint8_t pattern_num) {
	
	pattern_data next_pattern;
	
	next_pattern = read_pattern(pattern_num*PAGES_PER_PATTERN*PAGE_SIZE);
	sequencer.pattern[VAR_A] = next_pattern.variation_a;
	sequencer.pattern[VAR_B] = next_pattern.variation_b;
	sequencer.step_num[FIRST] = next_pattern.step_num[FIRST];
	sequencer.step_num[SECOND] = next_pattern.step_num[SECOND];
	sequencer.pre_scale = next_pattern.pre_scale;
	clock.divider = pre_scale[sequencer.pre_scale];
	update_step_led_mask();
	update_prescale_leds();
	//sequencer.part_playing = sequencer.step_num_new = FIRST;
	
	sequencer.step_num_new = sequencer.step_num[sequencer.part_editing];
	
	//sequencer.part_playing = sequencer.part_editing;
	
}

void write_current_pattern(uint8_t pattern_num) {
	
	pattern_data current_pattern;
	
	current_pattern.variation_a = sequencer.pattern[VAR_A];
	current_pattern.variation_b = sequencer.pattern[VAR_B];
	current_pattern.step_num[FIRST] = sequencer.step_num[FIRST];
	current_pattern.step_num[SECOND] = sequencer.step_num[SECOND];
	current_pattern.pre_scale = sequencer.pre_scale;
	
	write_pattern(pattern_num*PAGES_PER_PATTERN*PAGE_SIZE, &current_pattern);
}