/*
 * switches.c
 * JR-808 firmware ATMEGA328PB
 * minisystem
 * system79.com
 */

#include <stdio.h>
#include <avr/io.h>
#include <string.h>
#include "hardware.h"
#include "switches.h"
#include "spi.h"
#include "sequencer.h"
#include "drums.h"
#include "clock.h"
#include "midi.h"
#include "twi_eeprom.h"
#include "xnormidi-develop/midi.h"
#include "xnormidi-develop/midi_device.h"
#include "xnormidi-develop/bytequeue/bytequeue.h"

struct button button[NUM_BUTTONS] = {
	
{	0	,	4	,0,0,},
{	1	,	4	,0,0,},
{	2	,	4	,0,0,},
{	3	,	4	,0,0,},
{	4	,	4	,0,0,},
{	5	,	4	,0,0,},
{	6	,	4	,0,0,},
{	7	,	4	,0,0,},
{	0	,	3	,0,0,},
{	1	,	3	,0,0,},
{	2	,	3	,0,0,},
{	3	,	3	,0,0,},
{	4	,	3	,0,0,},
{	5	,	3	,0,0,},
{	6	,	3	,0,0,},
{	7	,	3	,0,0,},
{	5	,	2	,0,0,},
{	6	,	2	,0,0,},
{	7	,	2	,0,0,},
{	0	,	1	,0,0,},
{	1	,	1	,0,0,},
{	2	,	1	,0,0,},
{	3	,	1	,0,0,},
{	4	,	1	,0,0,},
{	5	,	1	,0,0,},
{	6	,	1	,0,0,},
{	7	,	1	,0,0,},
{	0	,	0	,0,0,},
{	1	,	0	,0,0,},
{	2	,	0	,0,0,},
{	0	,	2	,0,0,},
{	1	,	2	,0,0,},
{	2	,	2	,0,0,},
{	3	,	2	,0,0,},
{	4	,	2	,0,0,}	
	
	
	};
	
uint8_t current_start_stop_tap_state = 0;
uint8_t previous_start_stop_tap_state = 0;	
	
void parse_switch_data(void) {
	
	for (int i = 0; i < NUM_BUTTONS; i++) {
		
		//button[i].current_state = (switch_states[button[i].spi_byte] & button[i].spi_bit) & 1; //need to fix this
		uint8_t current_state = (spi_current_switch_data[button[i].spi_byte] >> button[i].spi_bit) &1 ;
		button[i].state ^= current_state;
		
	}
	
	
}	

void check_start_stop_tap(void) {
	//if (sequencer.mode == PATTERN_CLEAR) return; //do nothing
	current_start_stop_tap_state = PINB;
	current_start_stop_tap_state ^= previous_start_stop_tap_state;
	previous_start_stop_tap_state ^= current_start_stop_tap_state;
	current_start_stop_tap_state &= previous_start_stop_tap_state;
	
	if ((sequencer.START && (current_start_stop_tap_state >> TAP) &1)) {
			
		current_start_stop_tap_state ^= (1<<TAP); //toggle tap switch bit
		flag.tap = 1;
			
	}
	
	if (clock.source == EXTERNAL) return; //get out of here because when using external clock you don't need to process start/stop button activity
	
	uint8_t start_state = sequencer.START;
	sequencer.START ^= current_start_stop_tap_state >> START_STOP;
	
	if (sequencer.START && (start_state == 0)) { //initialize sequencer when start is detected
		
		process_start();
	}
	
	if ((sequencer.START == 0) && (start_state == 1)) {//when stop is first pressed need to handle lingering instrument LEDs 
		
		process_stop();
		
	} 
	

	
}
	
void check_inst_switches(void) {
	
	if (button[INST_AC_1_SW].state) {
		button[INST_AC_1_SW].state ^= button[INST_AC_1_SW].state; //toggle state
		if (!sequencer.SHIFT) {
			turn_off_all_inst_leds();
			turn_on(ACCENT_1_LED);
			sequencer.current_inst = AC;
		}
		return;
	}
	
	for (int i = INST_BD_2_SW; i <= INST_CH_12_SW; i++) { //scan BD to CH
		
		if (button[i].state) {
			
			button[i].state ^= button[i].state; //toggle state
			
			if (sequencer.SHIFT) {
				
				if (drum_hit[i-INST_BD_2_SW].switch_bit != NO_SWITCH || (i - INST_BD_2_SW == CP)) { //need to handle toggling between instrument
					//maybe evaluate the two drum states as 00, 01, 10, 11 and then use switch case
					uint8_t mute_state = (drum_hit[i - INST_BD_2_SW].muted) | (drum_hit[i - INST_BD_2_SW + 9].muted << 1);
					switch (mute_state) {
						
						case 0:
							drum_hit[i - INST_BD_2_SW].muted = 1;
							drum_hit[i - INST_BD_2_SW + 9].muted = 0;
						break;
						
						case 1:
							drum_hit[i - INST_BD_2_SW].muted = 0;
							drum_hit[i - INST_BD_2_SW + 9].muted = 1;									
						break;
						
						case 2:
							drum_hit[i - INST_BD_2_SW].muted = 1;
							drum_hit[i - INST_BD_2_SW + 9].muted = 1;						
						break;
						
						case 3:
							drum_hit[i - INST_BD_2_SW].muted = 0;
							drum_hit[i - INST_BD_2_SW + 9].muted = 0;						
						break;
						
					}
					
				} else {
					
						drum_hit[i - INST_BD_2_SW].muted ^= 1<<0; //toggle drum mute
					
				}
				
			} else {	
			
				turn_off_all_inst_leds(); 
			
			
				if(drum_hit[i - INST_BD_2_SW].switch_bit != NO_SWITCH) { // need to handle instrument toggle here
				
				
					if (sequencer.current_inst == i - INST_BD_2_SW) {
						//alternative drum hits are offset by 9 places in drum_hit array
						sequencer.current_inst = i - INST_BD_2_SW + 9;
					
					} else {			
						sequencer.current_inst = i - INST_BD_2_SW;
					}
				
				
				} else {
				
					if ((sequencer.current_inst == CP) && (i - INST_BD_2_SW == CP)) { //exception to handle CP/MA as they don't use a switch bit

						sequencer.current_inst = MA;
					
					} else {
					
						sequencer.current_inst = i - INST_BD_2_SW; //inst index starts with BD = 0
					}
				
		
				
				}
			
			}
						
				//return; //could break out here and not bother scanning everything - means only one button press can be detected
		}
		
	}
	

	
}	

void check_variation_switches(void) { //at the moment, just check one switch and cycle through A, B and A/B
	
	if (button[BASIC_VAR_A_SW].state && !sequencer.SHIFT) {
		
		button[BASIC_VAR_A_SW].state ^= button[BASIC_VAR_A_SW].state; //toggle  - this is not toggling. need to ^= 1<<0 to toggle a single bit state. hmmm.
		if (++sequencer.variation_mode == 3) sequencer.variation_mode = 0; //cycle through the 3 modes
		if (sequencer.START) {
			
			 flag.variation_change = 1; //set change flag to be handled when new measure starts
		} else { //otherwise change immediately
			
			if (sequencer.variation_mode == VAR_A || sequencer.variation_mode == VAR_AB) {
				
				sequencer.variation = VAR_A;
				
			} else {
				
				sequencer.variation = VAR_B;
				
			}
			
		}
		
	}
	
	
}	

void check_clear_switch(void) {
	
	if (sequencer.CLEAR && sequencer.START == 0) {
		
		switch (sequencer.mode) {
			
			case PATTERN_CLEAR:
			
				toggle(MODE_1_PATTERN_CLEAR);			
				memset(sequencer.pattern[sequencer.variation].part, 0, sizeof(sequencer.pattern[sequencer.variation].part));	
				memset(sequencer.step_led_mask[sequencer.variation], 0, sizeof(sequencer.step_led_mask[sequencer.variation]));			
				sequencer.pattern[sequencer.variation].accent[FIRST] = 0;
				sequencer.pattern[sequencer.variation].accent[SECOND] = 0;
				sequencer.step_num[FIRST] = 15;
				sequencer.step_num[SECOND]	= NO_STEPS;	//reset second part to no steps
				write_current_pattern(sequencer.current_pattern); //clear it from eeprom too		
				break;
				
			case FIRST_PART:
			
				break;	
			
			case SECOND_PART:
			
				break;
				
			case MANUAL_PLAY:
			
				break;
				
			case PLAY_RHYTHM:
			
				break;
				
			case COMPOSE_RHYTHM:
			
				break;			
			
		}
		
	} else {
		
		if (sequencer.mode == PATTERN_CLEAR) { //need to ensure LED is on after toggling while CLEAR button is held
			
			turn_on(MODE_1_PATTERN_CLEAR);
		}
		
	}
	
}

uint8_t check_step_press(void) {
	
	uint8_t switch_num = EMPTY;
	
	for (int i = 0; i < NUM_STEPS; i++) {
				
		if (button[i].state) {
					
			button[i].state ^= button[i].state;
			switch_num = i;
			break;// - should we break out of here? multiple presses will mess things up, right?
		}
				
	}
	
	return switch_num;	
}

void check_intro_fill_variation_switch(void) { //currently just some test code for testing EEPROM
	pattern_data eeprom_pattern;
	if (button[IF_VAR_SW].state) {
		button[IF_VAR_SW].state ^= button[IF_VAR_SW].state;
		
		if (sequencer.SHIFT) {
			//PORTE |= (1<<PE0);
			eeprom_pattern = read_pattern(sequencer.current_pattern*PAGES_PER_PATTERN*PAGE_SIZE);
			sequencer.pattern[VAR_A] = eeprom_pattern.variation_a;
			sequencer.pattern[VAR_B] = eeprom_pattern.variation_b;
			sequencer.step_num[FIRST] = eeprom_pattern.step_num[FIRST];
			sequencer.step_num[SECOND] = eeprom_pattern.step_num[SECOND];
			sequencer.pre_scale = eeprom_pattern.pre_scale;
			sequencer.step_num_new = sequencer.step_num[sequencer.part_editing];
			update_step_led_mask();
			//PORTE &= ~(1<<PE0);
			turn_on(IF_VAR_B_LED);
			turn_off(IF_VAR_A_LED);
			
		} else {
			
			
			//pattern_data eeprom_pattern;
			eeprom_pattern.variation_a = sequencer.pattern[VAR_A];
			eeprom_pattern.variation_b = sequencer.pattern[VAR_B];
			eeprom_pattern.step_num[FIRST] = sequencer.step_num[FIRST];
			eeprom_pattern.step_num[SECOND] = sequencer.step_num[SECOND];
			eeprom_pattern.pre_scale = sequencer.pre_scale;
			//struct pattern *pattern_buffer = &sequencer.pattern[0];
			//PORTE |= (1<<PE0);
			write_pattern(sequencer.current_pattern*PAGES_PER_PATTERN*PAGE_SIZE, &eeprom_pattern);
			//write_current_pattern(sequencer.current_pattern);
			//PORTE &= ~(1<<PE0);
			turn_on(IF_VAR_A_LED);
			turn_off(IF_VAR_B_LED);
		}

	}
	
	
}
