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

void process_step(void) {
	
		if (sequencer.START) { //this is an effort to synchronize SPI update within main loop - basically manipulate SPI data bytes and then do one single update_spi() call per loop
			
			if (sequencer.mode == PATTERN_FIRST) {	
				if (sequencer.next_step_flag) {
					sequencer.next_step_flag = 0;
					while(sequencer.trigger_finished == 0); //make sure previous instrument trigger is finished before initiating next one
					PORTD |= (1<<TRIG);
					spi_data[1] = (1 << sequencer.current_step) | sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst];
					spi_data[1] &= ~(sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst] & (1<<sequencer.current_step));
					spi_data[0] = ((1 << sequencer.current_step) >> 8) | (sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst] >> 8);
					spi_data[0] &= ~((sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst]>>8) & ((1<<sequencer.current_step) >>8));
					trigger_step();
					if ((sequencer.pattern[sequencer.variation].accent >> sequencer.current_step) &1) {
						spi_data[8] |= 1<<ACCENT;
						turn_on(ACCENT_1_LED);
					}
					TIMSK0 |= (1<<OCIE0A); //enable output compare match A
					TCCR0B |= (1<<CS01) | (1<<CS00); //set to /64 of system clock start timer
					sequencer.trigger_finished = 0;
				
					} else {
				
				}
			} else if (sequencer.mode == PATTERN_SECOND) {
				
				//handle patterns >16 steps here
				
			}
				
		} else if (sequencer.next_step_flag){
			
			sequencer.next_step_flag = 0;
			//spi_data[1] = 0;
			//spi_data[0] = 0;
			//turn_on(STEP_1_LED);
			
		}
}

void update_step_board() {
	
	if (sequencer.START) {
		
			switch (sequencer.mode) {		
			
			case PATTERN_FIRST:
			
				if (sequencer.CLEAR) { //clear button is pressed, check if step buttons are pressed and change step number accordingly
					
					for (int i = 0; i < 16; i++) {
						
							if (button[i].state) {
								
								button[i].state ^= button[i].state;
								sequencer.step_num_first = i;
								//if (sequencer.current_step > sequencer.step_num_first)
								break;// - should we break out of here? multiple presses will mess things up, right?
							}
						
					}
					
					break;
				}
				if (sequencer.current_inst == AC) { //bah, inefficient duplicate code to handle ACCENT
			
					for (int i = 0; i <= sequencer.step_num_first; i++) { //button and led indices match for 0-15. How convenient. Will need to use offset of 16 for steps 17-32 of PATTERN_SECOND
				
						if (button[i].state) {
					
							toggle(i);
							button[i].state ^= button[i].state;
							sequencer.pattern[sequencer.variation].accent ^= 1<<i; //just toggle first bit
							sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst] ^= 1<<i;
						}
					}
					return;
				}
				for (int i = 0; i <= sequencer.step_num_first; i++) { //button and led indices match for 0-15. How convenient.
			
					if (button[i].state) {
				
						toggle(i);
						button[i].state ^= button[i].state;
						sequencer.pattern[sequencer.variation].part[i] ^= 1<<sequencer.current_inst; //just work with first part of pattern and only 16 steps for now
						sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst] ^= 1<<i;
					}
				}
				break;
			
			case PATTERN_SECOND:
			
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