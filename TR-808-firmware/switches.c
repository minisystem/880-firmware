/*
 * switches.c
 * Open808 firmware ATMEGA328PB
 * minisystem
 * system80.net
 */

#include <stdio.h>
#include <avr/io.h>
#include <string.h>
#include "hardware.h"
#include "leds.h"
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
	
	//if ((sequencer.START && (current_start_stop_tap_state >> TAP) &1)) {
			//
		//current_start_stop_tap_state ^= (1<<TAP); //toggle tap switch bit
		//flag.tap = 1;
			//
	//}
	
	if ((current_start_stop_tap_state >> TAP) & 1) {
		
		current_start_stop_tap_state ^= (1<<TAP); //toggle tap switch bit
		flag.tap = 1;
		
	}
	
	if (clock.source == EXTERNAL) return; //get out of here because when using external clock you don't need to process start/stop button activity - ACTUALLY, you really should!
	
	uint8_t start_state = sequencer.START;
	sequencer.START ^= current_start_stop_tap_state >> START_STOP;
	
	if (sequencer.START && (start_state == 0)) { //initialize sequencer when start is detected
		
		process_start();
	}
	
	if ((sequencer.START == 0) && (start_state == 1)) {//when stop is first pressed need to handle lingering instrument LEDs 
		
		process_stop();
		
	} 
	

	
}

uint8_t read_track_switches(void) {
 
	//if (button[INST_AC_1_SW].state) {
  		//button[INST_AC_1_SW].state ^= button[INST_AC_1_SW].state; //toggle state
		//return AC;
    //}
    
    for (int i = INST_AC_1_SW; i <= INST_CH_12_SW; i++) { //scan AC to CH
      	
      	if (button[i].state) {
        	
        	button[i].state ^= button[i].state;
			return i - NUM_INST;  
        }
    } 
    
    return 255;                     
  
}  

void test_update_track_leds(void) {
 
	uint8_t track_inst = read_track_switches();
  
	if (track_inst != 255) {
		turn_off_all_inst_leds();
		sequencer.current_rhythm_track = track_inst;
		if (track_inst == 0) {
			turn_on(ACCENT_1_LED);
			
		} else {
			
			turn_on(drum_hit[track_inst-1].led_index);
      
		}            
    
	}    
  
}  
	 
void check_inst_switches(void) {
	
	
	if (button[INST_AC_1_SW].state) {
		button[INST_AC_1_SW].state ^= button[INST_AC_1_SW].state; //toggle state
		
		switch(sequencer.mode) {
		
		case FIRST_PART: case SECOND_PART:	
			
			if (!sequencer.SHIFT) {
				turn_off_all_inst_leds();
				turn_on(ACCENT_1_LED);
				sequencer.current_inst = AC;
			} else if (sequencer.ALT) {
			
				turn_off_all_inst_leds();
				turn_on(ACCENT_1_LED);
				(sequencer.intro_fill_var == 0) ? (sequencer.trigger_1 = AC) : (sequencer.trigger_2 = AC); //use intro_fill state to determine which trigger is being set 
			}
		break;
		
		case PLAY_RHYTHM: case COMPOSE_RHYTHM:
			
			if (sequencer.current_rhythm_track != 0) {
				//do some stuff to switch track change - like update current measure based on track change and set new pattern flag
				
			}
			sequencer.current_rhythm_track = 0;
			turn_on(ACCENT_1_LED);
			
		
		break;			
		}

		return; //no multiple presses currently supported - if it's the accent button, then get the heck out of here?
	}
	
	for (int i = INST_BD_2_SW; i <= INST_CH_12_SW; i++) { //scan BD to CH
		int drum_index = i - INST_BD_2_SW; //BD is 0, need to subtract INST_BD_2_SW offset so that drum_hit index is 0
		if (button[i].state) {
			
			button[i].state ^= button[i].state; //toggle state
			
			switch (sequencer.mode) {
				
			case FIRST_PART: case SECOND_PART:
			
				process_inst_press(drum_index);
			
			break;
			
			case PLAY_RHYTHM: case COMPOSE_RHYTHM: case MANUAL_PLAY:
			
				if (sequencer.SHIFT) {
					
					assign_mutes(drum_index);
					
				} else {
					
					if (sequencer.current_rhythm_track != (drum_index + 1)) {
						
						//do some stuff to swtich track change - like update current measure based on track change and set new pattern flag
					}
					turn_off(track_led[sequencer.current_rhythm_track]); //want to immediately refresh LEDs, rather than rely on sequencer to turn them off
					sequencer.current_rhythm_track = drum_index + 1;//+1 because AC is 0 and BD is 1 in track_led array
					turn_on(track_led[sequencer.current_rhythm_track]); 
				}
			
			default:
			
			break;	
				
			}
			
						
			return; //could break out here and not bother scanning everything - means only one button press can be detected
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
	if (sequencer.SHIFT) return; 
	if (sequencer.CLEAR) {// && sequencer.START == 0) {
		button[CLEAR_SW].state ^= button[CLEAR_SW].state; //need to reset CLEAR SW state here otherwise it gets handled elsewhere when we don't want it to.
		switch (sequencer.mode) {
			
			case PATTERN_CLEAR:
				//turn on timer2 interrupt for blinking clear LED
				TCCR2B |= (1<<CS22) | (1<<CS21) | (1<<CS20); //turn on Timer2 /1024 divide
				TCCR2A &= ~(1<<COM2A1) | ~(1<<COM2A0); //disconnect OC0A
				TIMSK2 |= (1<<OCIE2A); //enable Timer2 output compare A interrupt
				TCCR2A |= (1 << WGM20);// | (1<<WGM20); //clear timer on OCRA compare match where OCRA = OCRB
				TCCR2B |=  (1<<WGM22);
				OCR2A = 140; //alright, what the hell is this? Make it a constant so it actually means something you twit.
				
				if (flag.blink) {
					flag.blink = 0;
					toggle(MODE_1_PATTERN_CLEAR);
				}
										
				memset(sequencer.pattern[sequencer.variation].part, 0, sizeof(sequencer.pattern[sequencer.variation].part));	
				memset(sequencer.step_led_mask[sequencer.variation], 0, sizeof(sequencer.step_led_mask[sequencer.variation]));			
				sequencer.pattern[sequencer.variation].accent[FIRST] = 0;
				sequencer.pattern[sequencer.variation].accent[SECOND] = 0;
				sequencer.step_num[FIRST] = 15;
				sequencer.step_num[SECOND]	= NO_STEPS;	//reset second part to no steps
				sequencer.pre_scale = 1; //default PRE_SCALE_3
				clock.divider = PRE_SCALE_3;
				update_prescale_leds();
				write_current_pattern(sequencer.current_pattern, sequencer.pattern_bank); //clear it from eeprom too		
				break;
				
			case FIRST_PART: case SECOND_PART:
			
				break;	

				
			case MANUAL_PLAY:
			
				break;
				
			case PLAY_RHYTHM:
			
				break;
				
			case COMPOSE_RHYTHM: //clear selected rhythm track here
			
				break;			
			
		}
		
	} else {
		
		if (sequencer.mode == PATTERN_CLEAR) { //need to ensure LED is on after toggling while CLEAR button is held
			//turn off timer2 interupt
			TCCR2A = 0; //turn off timer2
			TIMSK2 &= ~(1<<OCIE2A);
			OCR2A = 70;
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

void check_intro_fill_variation_switch(void) { 
	
	if (button[IF_VAR_SW].state) {
		
		button[IF_VAR_SW].state ^= button[IF_VAR_SW].state;
		if (sequencer.SHIFT) {
			
			return;
			
		} else {
			toggle(IF_VAR_A_LED);
			toggle(IF_VAR_B_LED);
			sequencer.intro_fill_var ^= 1<<0;
		}
		
	}
	
}

void assign_triggers(uint8_t drum_index) {
			if (sequencer.intro_fill_var == 0) { //edit trigger 1
				if (drum_hit[drum_index].switch_bit != NO_SWITCH) { // need to handle instrument toggle here
					
					
					if (sequencer.trigger_1 == drum_index) {
						//toggle between instruments
						sequencer.trigger_1 = drum_index + SW_DRUM_OFFSET;
						
						} else {
						sequencer.trigger_1 = drum_index;
					}
					
					
					} else {
					
					if ((sequencer.trigger_1 == CP) && (drum_index == CP)) { //exception to handle CP/MA as they don't use a switch bit

						sequencer.trigger_1 = MA;
						
						} else {
						sequencer.trigger_1 = drum_index; //inst index starts with BD = 0
					}
				}
				} else { //edit trigger 2. annoying duplication of code here, but use of bitfields prevent assigning pointer to whichever trigger is currently being edited.
				
				if (drum_hit[drum_index].switch_bit != NO_SWITCH) { // need to handle instrument toggle here
					
					
					if (sequencer.trigger_2 == drum_index) {
						//toggle between instruments
						sequencer.trigger_2 = drum_index + SW_DRUM_OFFSET;
						
						} else {
						sequencer.trigger_2 = drum_index;
					}
					
					
					} else {
					
					if ((sequencer.trigger_2 == CP) && (drum_index == CP)) { //exception to handle CP/MA as they don't use a switch bit

						sequencer.trigger_2 = MA;
						
						} else {
						sequencer.trigger_2 = drum_index; //inst index starts with BD = 0
					}
					
				}
			}	
	
}

void assign_mutes(uint8_t drum_index) {
	
	if (drum_hit[drum_index].switch_bit != NO_SWITCH || (drum_index == CP)) { //need to handle toggling between instrument
		//maybe evaluate the two drum states as 00, 01, 10, 11 and then use switch case
		uint8_t mute_state = (drum_hit[drum_index].muted) | (drum_hit[drum_index + SW_DRUM_OFFSET].muted << 1);
		switch (mute_state) {
					
			case 0:
			drum_hit[drum_index].muted = 1;
			drum_hit[drum_index + SW_DRUM_OFFSET].muted = 0;
			break;
					
			case 1:
			drum_hit[drum_index].muted = 0;
			drum_hit[drum_index + SW_DRUM_OFFSET].muted = 1;
			break;
					
			case 2:
			drum_hit[drum_index].muted = 1;
			drum_hit[drum_index + SW_DRUM_OFFSET].muted = 1;
			break;
					
			case 3:
			drum_hit[drum_index].muted = 0;
			drum_hit[drum_index + SW_DRUM_OFFSET].muted = 0;
			break;
					
		}
				
	} else {
				
		drum_hit[drum_index].muted ^= 1<<0; //toggle drum mute
				
	}	
}

void process_inst_press(uint8_t drum_index) {
	if (!sequencer.SHIFT) { //no SHIFT, so just toggle between different instruments
				
		turn_off_all_inst_leds();
				
				
		if(drum_hit[drum_index].switch_bit != NO_SWITCH) { // need to handle instrument toggle here
					
					
			if (sequencer.current_inst == drum_index) {
				//toggle between switched instruments
				sequencer.current_inst = drum_index + SW_DRUM_OFFSET;
						
			} else {
				sequencer.current_inst = drum_index;
			}
					
					
		} else {
					
			if ((sequencer.current_inst == CP) && (drum_index == CP)) { //exception to handle CP/MA as they don't use a switch bit

				sequencer.current_inst = MA;
						
			} else {
						
				sequencer.current_inst = drum_index; //inst index starts with BD = 0
			}
					
		}

				
	} else { //SHIFT pressed 
				
		if (sequencer.ALT) { //ALT mode - change trigger - not sure how this is handled - LEDs won't be properly updated
					
			turn_off_all_inst_leds();
			assign_triggers(drum_index);		

					
		} else { //SHIFT, no ALT - handle mutes
					
			assign_mutes(drum_index);
		}
				
	}	
	
}

void process_track_press(void) {
	
	
}
void clear_pattern_bank(uint8_t bank) {
	
		memset(sequencer.pattern[VAR_A].part, 0, sizeof(sequencer.pattern[VAR_A].part));
		memset(sequencer.step_led_mask[VAR_A], 0, sizeof(sequencer.step_led_mask[VAR_A]));
		sequencer.pattern[VAR_A].accent[FIRST] = 0;
		sequencer.pattern[VAR_A].accent[SECOND] = 0;
		memset(sequencer.pattern[VAR_B].part, 0, sizeof(sequencer.pattern[VAR_B].part));
		memset(sequencer.step_led_mask[VAR_B], 0, sizeof(sequencer.step_led_mask[VAR_B]));
		sequencer.pattern[VAR_B].accent[FIRST] = 0;
		sequencer.pattern[VAR_B].accent[SECOND] = 0;
		
		sequencer.step_num[FIRST] = 15;
		sequencer.step_num[SECOND]	= NO_STEPS;	//reset second part to no steps
		sequencer.pre_scale = 1; //default PRE_SCALE_3
		
		for (int pattern = 0; pattern < 16; ++pattern) {
			
			write_current_pattern(pattern, bank);
		}
	
	
}
void clear_all_patterns(void) {
	
	memset(sequencer.pattern[VAR_A].part, 0, sizeof(sequencer.pattern[VAR_A].part));
	memset(sequencer.step_led_mask[VAR_A], 0, sizeof(sequencer.step_led_mask[VAR_A]));
	sequencer.pattern[VAR_A].accent[FIRST] = 0;
	sequencer.pattern[VAR_A].accent[SECOND] = 0;
	memset(sequencer.pattern[VAR_B].part, 0, sizeof(sequencer.pattern[VAR_B].part));
	memset(sequencer.step_led_mask[VAR_B], 0, sizeof(sequencer.step_led_mask[VAR_B]));
	sequencer.pattern[VAR_B].accent[FIRST] = 0;
	sequencer.pattern[VAR_B].accent[SECOND] = 0;
	
	sequencer.step_num[FIRST] = 15;
	sequencer.step_num[SECOND]	= NO_STEPS;	//reset second part to no steps
	sequencer.pre_scale = 1; //default PRE_SCALE_3
	clock.divider = PRE_SCALE_3;
	
	for (int bank = 1; bank < 16; bank++) {
		for (int pattern = 0; pattern < 16; pattern++) {
			
			write_current_pattern(pattern, bank);
			
		}
		
	}
	
}