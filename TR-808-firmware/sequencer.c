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
			
			//if (sequencer.part_num == FIRST || sequencer.part_num == SECOND) {	
				if (flag.next_step) {
					flag.next_step = 0;
					while(flag.trig_finished == 0); //make sure previous instrument trigger is finished before initiating next one
					PORTD |= (1<<TRIG);
					
					if (sequencer.mode == FIRST_PART && sequencer.part_num == FIRST) { //only blink step LEDs if in current parts mode (ie. part_num == FIRST && mode == FIRST_PART
						spi_data[1] = (1 << sequencer.current_step) | sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst];
						spi_data[1] &= ~(sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst] & (1<<sequencer.current_step));
						spi_data[0] = ((1 << sequencer.current_step) >> 8) | (sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst] >> 8);
						spi_data[0] &= ~((sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst]>>8) & ((1<<sequencer.current_step) >>8));
					} else if (sequencer.mode == SECOND_PART && sequencer.part_num == SECOND) {
						spi_data[1] = (1 << (sequencer.current_step - sequencer.step_num_first -1)) | sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst];
						spi_data[1] &= ~(sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst] & (1<<(sequencer.current_step - sequencer.step_num_first-1)));
						spi_data[0] = ((1 << (sequencer.current_step - sequencer.step_num_first-1)) >> 8) | (sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst] >> 8);
						spi_data[0] &= ~((sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst]>>8) & ((1<<(sequencer.current_step - sequencer.step_num_first-1)) >>8));
						//
					}
					
					//if (sequencer.part_num == FIRST) {
						//spi_data[1] = (1 << sequencer.current_step) | sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst];
						//spi_data[1] &= ~(sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst] & (1<<sequencer.current_step));
						//spi_data[0] = ((1 << sequencer.current_step) >> 8) | (sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst] >> 8);
						//spi_data[0] &= ~((sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst]>>8) & ((1<<sequencer.current_step) >>8));
						//
					//} else {
						//spi_data[1] = (1 << (sequencer.current_step - sequencer.step_num_first -1)) | sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst];
						//spi_data[1] &= ~(sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst] & (1<<(sequencer.current_step - sequencer.step_num_first-1)));
						//spi_data[0] = ((1 << (sequencer.current_step - sequencer.step_num_first-1)) >> 8) | (sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst] >> 8);
						//spi_data[0] &= ~((sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst]>>8) & ((1<<(sequencer.current_step - sequencer.step_num_first-1)) >>8));						
						//
					//}
					

					trigger_step();
					if ((sequencer.pattern[sequencer.variation].accent >> sequencer.current_step) &1) {
						spi_data[8] |= 1<<ACCENT;
						turn_on(ACCENT_1_LED);
					}
					TIMSK0 |= (1<<OCIE0A); //enable output compare match A
					TCCR0B |= (1<<CS01) | (1<<CS00); //set to /64 of system clock start timer
					flag.trig_finished = 0;
				
					} else {
				
				}
			//} else {//if (sequencer.part_num == SECOND) {
				
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
					
					for (int i = 0; i < 16; i++) {
						
							if (button[i].state) {
								
								button[i].state ^= button[i].state;
								sequencer.step_num_new = i;
								break;// - should we break out of here? multiple presses will mess things up, right?
							}
						
					}
					
					break; //break or return? or is it needed?
				}
				
				uint8_t step_num = 0;
				uint8_t offset = 0;
				
				if (sequencer.mode == FIRST_PART) {
					
					step_num = sequencer.step_num_first;
					offset = 0;
				} else if (sequencer.mode == SECOND_PART) {
					
					step_num = sequencer.step_num_second;
					offset = 16; //offset for steps 16-31
					
				}
				
				if (sequencer.current_inst == AC) { //bah, inefficient duplicate code to handle ACCENT
			
					for (int i = 0; i <= step_num; i++) { //button and led indices match for 0-15. How convenient. Will need to use offset of 16 for steps 17-32 of SECOND_PART
				
						if (button[i].state) {
					
							toggle(i);
							button[i].state ^= button[i].state;
							sequencer.pattern[sequencer.variation].accent ^= 1<<(i + offset); 
							sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst] ^= 1<<i;
						}
					}
					return;
				}
				for (int i = 0; i <= step_num; i++) { //button and led indices match for 0-15. How convenient.
			
					if (button[i].state) {
						//toggle(SECOND_PART_LED);
						toggle(i);
						button[i].state ^= button[i].state;
						sequencer.pattern[sequencer.variation].part[i + offset] ^= 1<<sequencer.current_inst;
						sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst] ^= 1<<i;
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
		
		//handle what here? changing selected pattern or rhythm? 
		
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