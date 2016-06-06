/*
 * interrupts.c
 * JR-808 firmware ATMEGA328PB
 * minisystem
 * system79.com
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "drums.h"
#include "spi.h"
#include "clock.h"
#include "sequencer.h"
#include "xnormidi-develop/midi.h"
#include "xnormidi-develop/midi_device.h"
#include "midi.h"


ISR (TIMER0_COMPA_vect) {
	
	TCCR0B = 0; //turn off timer
	TIMSK0 &= ~(1<<OCIE0A); //turn off output compare 
	
	//if (sequencer.mode == MANUAL_PLAY) { 
		//spi_data[drum_hit[current_drum_hit].spi_byte_num] &= ~(drum_hit[current_drum_hit].trig_bit);
	//} else {
		//
		//spi_data[8] = 0;
	//}
	//spi_data[8] = 0;
	//spi_data[6] &= 0b1111000;
	//uint8_t current_drum_hit  = midi_note_queue[note_queue_index];
	//note_queue_index--;
	spi_data[drum_hit[current_drum_hit].spi_byte_num] &= ~(drum_hit[current_drum_hit].trig_bit);
	////toggle(drum_hit[current_drum_hit].led_index);
	////toggle(ACCENT_1_LED);
	//update_spi(); //should set flag here and update SPI from main loop. SPI should take about 10 microseconds
	trigger_finished = 1;
	
}

ISR (TIMER3_COMPA_vect) { //led flashing interrupt. Will this be too much overhead to do something simple like flash LEDs?
	
	//turn_off_all_inst_leds();
	//update_inst_leds();
	
}

ISR (TIMER1_COMPA_vect) { //output compare match for internal clock
	//midi_send_clock(&midi_device); //much more setup and overhead is required to send MIDI data
	//update_inst_leds();
	if (++internal_clock.ppqn_counter == internal_clock.divider)
	{
		flag.next_step = 1;
		internal_clock.beat_counter++; //overflows every 4 beats
		internal_clock.ppqn_counter = 0;
		if (sequencer.current_step++ == sequencer.step_num[sequencer.part_playing] && sequencer.START) { 
			flag.new_measure = 1;
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
				internal_clock.divider = pre_scale[pre_scale_index];
				
			}
			


			//}
			//sequencer.current_measure++;
		}		
	
	} //should make the else if so second condition doesn't need to be tested
	
	
	
	if (internal_clock.ppqn_counter == internal_clock.divider >> 1) { //50% step width, sort of - this is going to get long and complicated fast - need to set flag and handle in main loop refresh function
		flag.half_step = 1;
		turn_off_all_inst_leds();
		if (!sequencer.SHIFT) turn_on(drum_hit[sequencer.current_inst].led_index);
		spi_data[5] &= ~(led[BASIC_VAR_A_LED].spi_bit | led[BASIC_VAR_B_LED].spi_bit); //this clears basic variation LEDs
		if (sequencer.START) { 	
	
			spi_data[1] = sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst]; //this keeps inst lights on while blinking step light
			spi_data[0] = sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst] >> 8;

			//turn_off_all_inst_leds();
			//if (!sequencer.SHIFT) turn_on(drum_hit[sequencer.current_inst].led_index);
								
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
			
			if (internal_clock.beat_counter <2) {
				
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
		
			if (internal_clock.beat_counter <2) { //1/8 note, regardless of scale (based on original 808 behavior) - don't take this as gospel. may need to adjust with different pre-scales
			

				if (sequencer.variation_mode == VAR_AB) sequencer.var_led_mask |= led[BASIC_VAR_B_LED].spi_bit;	//turn on VAR_B LED for flashing to indicate A/B mode
					

			
				turn_on(STEP_1_LED); //eventually need to turn on current pattern LED in pattern mode - other modes will require different behavior to be coded
			}
		}
		
		spi_data[5] |= sequencer.var_led_mask;
		
	} 
	
	

	
}