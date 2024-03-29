/*
 * sequencer.c
 * Open808 firmware ATMEGA328PB
 * minisystem
 * system80.net
 */
#include <stdio.h>
#include <avr/io.h>
#include "string.h"
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
struct rhythm_track rhythm_track;//[NUM_PATTERNS];
struct recall recall;
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
	current_tempo_adc = current_tempo_adc + (tempo_adc_change >>2); //low pass filter
	
	clock.rate = (ADC_MAX - current_tempo_adc) + TIMER_OFFSET; //offset to get desirable tempo range

	//if (clock.rate != clock.previous_rate) { //this step is maybe not necessary? Just always update OCR1A?
		
		update_clock_rate(clock.rate);
		
	//}
	
	//clock.previous_rate = clock.rate;
	
}

void show_current_measure(void) {
	
	//turn_on(sequencer.current_rhythm_track);
	
	if (sequencer.track_measure < 16) {
		
			turn_on(sequencer.track_measure);
		
		} else if (sequencer.track_measure < 32) {
		
			turn_on(sequencer.track_measure - 16);
		
		} else if (sequencer.track_measure < 48) {
			
			turn_on(sequencer.track_measure - 32);
		} else {
		
			turn_on(sequencer.track_measure - 48);
	}
	
	
	
}

void show_version_steps(void) {
	uint8_t version_step = (sequencer.version/10)%10;
	if (version_step > 0) turn_on(version_step - 1);
}

void process_tick(void) {
	//TRIGGER_OUT |= (1<<TRIGGER_OUT_2);
		
	if (flag.shuffle_step) { 
		
		//ok, maybe use shuffle_amount to index an array of the best sounding shuffle ppqn counts
		if (++sequencer.shuffle_ppqn_count == (sequencer.shuffle_amount*sequencer.shuffle_multplier)) { //X2 to increase shuffle time. 1-5 ppqn @ new 96 ppqn internal resolution was a bit too subtle. X2 is compatible with all shuffle levels for all pre-scales whereas X3 was not
				
			//sequencer.shuffle_ppqn_count = 0;
			flag.next_step = 1;
			flag.shuffle_step = 0;
				
		}

	}
	
	uint8_t even_step = sequencer.current_step & 1;	//is current step even or odd?
	
	if (++clock.ppqn_counter == clock.divider) { //trigger next step
		//clock.tick_counter = 0;
		if (flag.pre_scale_change) {
			
			clock.divider = pre_scale[sequencer.pre_scale];       
		}
		if (flag.shuffle_change) {TCCR1B |= (1<<CS12);
			
			sequencer.shuffle_amount = sequencer.new_shuffle_amount;
			flag.shuffle_change = 0;
			
		}
		//handle shuffle
		if ((sequencer.shuffle_amount != 0) && !even_step && !flag.shuffle_change) { //confusing because step is incremented below, so it becomes even but is currently odd
			//set shuffle_step flag and then use shuffle ppqn counter to count amount of shuffle pqqn before setting next step flag
			sequencer.shuffle_ppqn_count = 0;
			flag.shuffle_step = 1;
			
		}
		
		if (!flag.shuffle_step) flag.next_step = 1;
		//if (sequencer.primed == 1) {
			//sequencer.current_measure_auto_fill = -1;
			//sequencer.track_measure = -1;
			//sequencer.primed = 0;
		//}	
		//sequencer.primed = 0;
		if (sequencer.current_step++ == sequencer.step_num[sequencer.part_playing] && sequencer.START) {
			if (sequencer.primed) { //this causes problems when first part has less than 16 steps. why? - FIXED!
				sequencer.primed = 0;
				//turn_on(IF_VAR_B_LED);
				sequencer.current_step = 0;
				//need to do something here to handle first part with less than 16 steps.
			} else {
				flag.new_measure = 1;
			}
			
		}
		
		clock.beat_counter++; //overflows every 4 beats 
		clock.ppqn_counter = 0;
	//50% step width, sort of - use for flashing step and variation LEDs to tempo
	} else if (((clock.ppqn_counter == clock.divider >> 1) && !even_step) || ((even_step && (clock.ppqn_counter == (clock.divider >> 1) + sequencer.shuffle_amount)))  ){
		//if it's an odd step then set half_step flag OR if it's an even step and the appropriate amount of shuffle has transpired then set half_step flag
		//this is gobbledygook is so that LEDs flash with right timing and period
		flag.half_step = 1;

	}
	
	if (clock.source == EXTERNAL) { 
		
		if (++clock.slave_ppqn_ticks == clock.sync_count) { //this works for 24 ppqn MIDI and DIN sync clock, but needs tweaking for external sync pulse that runs at 2 ppqn or 1 ppqn
			clock.slave_ppqn_ticks = 0; //reset
			flag.wait_for_master_tick = 1; //not currently used
			TCCR1B &= ~(1<<CS12); //turn off master timer
			//TCNT1 = 0;
		}
		
	}
	//TRIGGER_OUT &= ~(1<<TRIGGER_OUT_1);
		
}
	


void process_start(void) {
		
		PORTC &= ~(1<<SYNC_LED_R);
		PORTE &= ~(1<<SYNC_LED_Y);
		clock.sync_led_mask = 0;
		sequencer.part_playing = FIRST;
		//if (sequencer.step_num[SECOND] != NO_STEPS) sequencer.part_playing = SECOND; //noodle fryer here - priming the sequencer on START forces process_new_measure to be called before first step is played, causing part playing to be toggled
		//so ugly and clunky. how to simplify?
		//sequencer.current_step = 0;
		if (sequencer.mode != COMPOSE_RHYTHM) sequencer.track_measure = 0; //ugly bodge here. need to think how Rhythm Play and Compose modes should work when started and stopped. bleh.
		if (sequencer.mode == PLAY_RHYTHM) { //handle rhythm track looping
			
			if (sequencer.SHIFT) {
				sequencer.track_loop = 1;
			}
		}
		//if (sequencer.clock_mode != DIN_SYNC_MASTER) flag.next_step = 1; //change this to make it more generalized. Maybe need a switch:case statement to handle different sync modes?
		//flag.new_measure = 1;
		//clock.ppqn_counter = 0;
		if (sequencer.clock_mode == DIN_SYNC_MASTER) {
			clock.ppqn_counter = 0;
		} else {
			clock.ppqn_divider_tick = 0; //need to think about what's happening here - does it need to be processed ad ppqn_divider_tick = ppqn_divider -1 when starting as slave?
			//need to prime sequencer so that first step (downbeat) occurs on first incoming clock pulse, hence -1 for current_step and divider	
			//sequencer.current_step = -1;
			sequencer.current_step = sequencer.step_num[FIRST]; //oh boy. this was a pernicious bug.  the first part priming assumes 16 steps but you need to catch first down beat on fi
			clock.ppqn_counter = clock.divider - 1; //need to change this priming if we want to implement independent MIDI/DIN transport control while synced to a master
			sequencer.primed = 1;
		}
		if (clock.source == EXTERNAL) { 
			flag.slave_start = 1;
			clock.slave_ppqn_ticks = 0;
			PORTE |= (1 << SYNC_LED_Y); //orange sync light when slaved
			PORTC |= (1 << SYNC_LED_R);
			clock.tick_counter = 0;//clock.tick_value; //this line is required to get 1/16th note sync up to 24 ppqn master, but it seems weird to reset it as it should be 0 as soon as the while loop above is released?

		} else {
			
			flag.slave_start = 0;
			PORTE &= ~(1 << SYNC_LED_Y);
			PORTC |= (1 << SYNC_LED_R); //red for internal clock running
		}		
		
		//reset variation on start	
		flag.variation_change = 0;
		if (sequencer.variation_mode == VAR_A || sequencer.variation_mode == VAR_AB) {
				
			sequencer.current_variation = VAR_A; //start on variation A
		} else {
				
			sequencer.current_variation = VAR_B;
		}
		
		sequencer.basic_variation = sequencer.current_variation;
		
		if (clock.source == INTERNAL) {

			

		} else {
			
			
		}
		
		//if (sequencer.clock_mode == PULSE_SYNC_SLAVE) PORTC |= (1<<SYNC_LED_R); //maintain phase of SYNC LED
		if (sequencer.clock_mode == DIN_SYNC_MASTER || sequencer.clock_mode == DIN_SYNC_SLAVE) {
			//don't set flag.next_step here because need to send a couple of DIN Sync clock pulses before start
			//PORTE |= (1<<SYNC_LED_Y);
			flag.din_start = 1; 
			clock.din_ppqn_pulses = 0;
			//if (sequencer.clock_mode == DIN_SYNC_MASTER) PORTD |= (1 << DIN_RUN_STOP); //set run/stop line high
			
		} else { //otherwise set flag.next_step and send MIDI if MIDI_MASTER
			
			//flag.next_step = 1; - getting rid of means first step isn't triggered - see if (clock.source == EXTERNAL) above for workround - not tested with 880 as master though....
			if (sequencer.clock_mode == MIDI_MASTER) {
				
				midi_send_start(&midi_device); //should clock be sent before start?
				midi_send_clock(&midi_device);	
				//PORTD |= (1 << DIN_RUN_STOP); //set RUN_STOP high for enclosure but also to disable enclosure's spi0 MISO line			
			}					
		}

		if (sequencer.mode == MANUAL_PLAY) { //works, but need to handle intro/fill variation here (or if not here, where?)
			
			if (flag.intro) {
			
				flag.intro = 0;
				sequencer.current_variation = sequencer.intro_fill_var;
				flag.pattern_change = 1;
			} 
			if (sequencer.fill_mode != MANUAL) { //need to prime fill count on first measure
				if (++sequencer.current_measure_auto_fill == sequencer.fill_mode) {
					flag.fill = 1;
				}					
			}			
		}
		
		
		
		spi_data[LATCH_3] |= CONGAS_OFF;
		
		//sequencer.step_num_new = sequencer.step_num[FIRST];
		
		
		//set trigger off timer
		TIMER0_OUTPUT_COMPARE = TIMER0_15_MS;
		
		//reset timers:
		TCNT1 = 0;
		TCNT3 = 0;
		TCCR1B |= (1<<CS12); //ensure timer is on
}

void process_stop(void) {
		//turn_off(IF_VAR_B_LED);
		PORTC &= ~(1<<SYNC_LED_R);
		PORTE &= ~(1<<SYNC_LED_Y);
		clock.sync_led_mask = 0;
		//sequencer.current_step = 0;
		sequencer.track_measure = 0; //reset track measure. No continue for now
		sequencer.track_loop = 0; //reset track loop to no looping
		//clock.ppqn_counter = 0;
		//clock.ppqn_divider_tick = 0;
	
		if (sequencer.part_playing == SECOND) { //reset part playing
			sequencer.part_playing = FIRST;
			turn_off(SECOND_PART_LED);
			turn_on (FIRST_PART_LED);
				
		}
		turn_off_all_inst_leds();
		turn_on(drum_hit[sequencer.current_inst].led_index);
		sequencer.current_pattern = sequencer.new_pattern;
		sequencer.current_measure_auto_fill = 0; //reset current measure
		flag.pattern_change = 0;
		flag.fill = 0;	
		//blank all step leds and turn on current pattern LED
		spi_data[LATCH_1] = 0;
		spi_data[LATCH_0] = 0;
		turn_on(sequencer.current_pattern); //turn this on after next section which potentially changes current pattern?
		switch (sequencer.mode) {
			
			case MANUAL_PLAY:
				turn_on(sequencer.current_intro_fill);
				sequencer.current_pattern = sequencer.new_pattern;
				read_next_pattern(sequencer.current_pattern, sequencer.pattern_bank); //calling read_next_pattern here causes a crash when slaved to DIN SYNC. why?
			break;
			
			case COMPOSE_RHYTHM: 
				if (rhythm_track.length > 0) sequencer.track_mode = EDIT;
				sequencer.current_pattern = sequencer.new_pattern = rhythm_track.patterns[0].current_pattern; //return to first pattern of rhythm track
				sequencer.pattern_bank = rhythm_track.patterns[0].current_bank;
				read_next_pattern(sequencer.current_pattern, sequencer.pattern_bank);
				flag.pattern_change = 1;	
			break;
			
			case PLAY_RHYTHM: //this is all reused from above - consolidate somehow?
				sequencer.current_pattern = sequencer.new_pattern = rhythm_track.patterns[0].current_pattern; //return to first pattern of rhythm track
				sequencer.pattern_bank = rhythm_track.patterns[0].current_bank;
				read_next_pattern(sequencer.current_pattern, sequencer.pattern_bank);
				flag.pattern_change = 1;		
			
			default:
			
			break;
		}
		/*
		if (sequencer.mode == MANUAL_PLAY) { //need to restore original basic pattern in case sequencer is stopped while playing fill
			//sequencer.current_measure_auto_fill = 1;
			turn_on(sequencer.current_intro_fill);
			sequencer.current_pattern = sequencer.new_pattern;
			read_next_pattern(sequencer.current_pattern, sequencer.pattern_bank);
		}
		if (sequencer.mode == COMPOSE_RHYTHM) {
			if (rhythm_track.length > 0) sequencer.track_mode = EDIT;
			sequencer.current_pattern = sequencer.new_pattern = rhythm_track.patterns[0].current_pattern; //return to first pattern of rhythm track
			sequencer.pattern_bank = rhythm_track.patterns[0].current_bank;
			read_next_pattern(sequencer.current_pattern, sequencer.pattern_bank);
			flag.pattern_change = 1; //need to 
		}	
		*/
		if (clock.source == INTERNAL) {
			PORTD &= ~(1 << DIN_RUN_STOP);
			if (sequencer.clock_mode == MIDI_MASTER) {

				midi_send_stop(&midi_device);
			} /*else {
				
				PORTD &= ~(1 << DIN_RUN_STOP);
			}*/
			
		}
		
		spi_data[LATCH_3] |= CONGAS_OFF;
		
		//set trigger off timer for incoming MIDI, currently only applies to MIDI_SLAVE sync mode
		TIMER0_OUTPUT_COMPARE = TIMER0_1_MS;
}

void update_fill(void) {
	

	if (flag.fill) { //problem with current_measure overfow here - only need to increment measure if fill_mode != 0
		sequencer.current_measure_auto_fill = 0;
		flag.fill = 0;
		flag.pre_scale_change = 1;
		read_next_pattern(sequencer.current_intro_fill, sequencer.pattern_bank);
		sequencer.part_playing = FIRST;
		turn_off(SECOND_PART_LED);
		turn_on(FIRST_PART_LED);
		sequencer.new_pattern = sequencer.current_pattern;
		sequencer.current_pattern = sequencer.current_intro_fill;
		//flag.intro = 0;
		sequencer.basic_variation = sequencer.current_variation; //store current variation
		sequencer.current_variation = sequencer.intro_fill_var;
		//flag.variation_change = 1;
		flag.pattern_change = 1;
	} else {		
		if (sequencer.fill_mode != MANUAL) {
			if (++sequencer.current_measure_auto_fill == sequencer.fill_mode) {
				flag.fill = 1;
			}		
		}		
	}	
}

void update_track_play() {
	
	
}

void update_pattern() {
		turn_off(SECOND_PART_LED);
		turn_on(FIRST_PART_LED);
		
		if (sequencer.mode == PLAY_RHYTHM) {
			//sequencer.track_measure++;
			if ((sequencer.variation_mode == VAR_AB) && (sequencer.current_variation == VAR_A)) { //kludge to get alternating A/B to play A then B before moving on to next track
				//if (sequencer.track_measure == 0) sequencer.track_measure++;//sequencer.track_measure--; 
			} else if (sequencer.track_measure++ == rhythm_track.length) { //you done
						
				sequencer.track_measure = 0;
						
				if (sequencer.track_loop) {
							
				} else {				
					sequencer.START = 0;
					process_stop();						
				}
				flag.pattern_change = 1;		
						
			} else {
				flag.pattern_change = 1;
			}
			sequencer.new_pattern = rhythm_track.patterns[sequencer.track_measure].current_pattern;
			sequencer.pattern_bank = rhythm_track.patterns[sequencer.track_measure].current_bank;
		}
		toggle_variation(); //only toggle variation at the end of the 2nd part - don't need to call this if pattern changes, yeah? so maybe put this as an else after if (flag.pattern_change)?
		if (flag.pattern_change) {
					
			flag.pattern_change = 0;
			flag.pre_scale_change = 1; //need to handle any change in pre-scale
			sequencer.current_pattern = sequencer.new_pattern;
			read_next_pattern(sequencer.current_pattern, sequencer.pattern_bank);
			//if (sequencer.mode  == MANUAL_PLAY) {
				sequencer.current_variation = VAR_A;
				if (sequencer.variation_mode == VAR_B) sequencer.current_variation = VAR_B;
			//not sure about A/B variation in the case of a pattern change - should A/B variation be reset to A when a pattern changes? YES - David Lush pointed this out
				if (sequencer.variation_mode == VAR_AB) sequencer.current_variation = VAR_A;// ~sequencer.basic_variation;
			//}
			sequencer.part_playing = FIRST;
			turn_off(SECOND_PART_LED);
			turn_on(FIRST_PART_LED);
					
		} //else { test this - toggle variation up there is redundant if there is a pattern change. faster or no?
			//toggle_variation();
		//}
				
		if (sequencer.mode == MANUAL_PLAY) {
			update_fill();
		}
		

	
}

void process_new_measure(void) { //should break this up into switch/case statements based on mode? Yes, you should. Why haven't you done it yet?
	
	//need to update step number changes before 
	sequencer.step_num[sequencer.part_editing] = sequencer.step_num_new; //will eventually want to be able to change step number in MANUAL PLAY mode, but leave it here for now
	

	sequencer.current_step = 0;
	//toggle(IF_VAR_B_LED);
	if (flag.pattern_edit == 1) {
					
		flag.pattern_edit = 0;
		//toggle(IF_VAR_B_LED);
		start_write_current_pattern(sequencer.current_pattern, sequencer.pattern_bank); //save changed pattern at end of measure
					
	}
/* 	
	if (sequencer.mode == PLAY_RHYTHM) {
		//only advance track_measure when in VAR_A or VAR_B mode. If in VAR_AB mode, only advance measure when VAR_B has finished playing, yeah?
		if (sequencer.variation_mode == VAR_AB && sequencer.current_variation == VAR_A) { //don't advance track measure
			
		} else {
			if (sequencer.track_measure++ < rhythm_track.length) { //do anything in this case? 
			
			} else {
			
				//rhythm track is finished, so stop or loop
				//stop for now
				if (!sequencer.track_loop) {
					sequencer.START = 0;
					process_stop();
				}
				sequencer.track_measure = 0; //reset track measure
				sequencer.new_pattern = rhythm_track.patterns[0].current_pattern; //return to first pattern of rhythm track
				sequencer.pattern_bank = rhythm_track.patterns[0].current_bank;
				//read_next_pattern(sequencer.new_pattern, sequencer.pattern_bank); 
				//return; //bah. this works but is confusing and bad form? - YES IT IS! returning here prevents A/B toggling.
				//maybe need to do some LED housekeeping before leaving?
				//return;
			}
			sequencer.new_pattern = rhythm_track.patterns[sequencer.track_measure].current_pattern;
			sequencer.pattern_bank = rhythm_track.patterns[sequencer.track_measure].current_bank;
			flag.pattern_change = 1;
		}
	} else if (sequencer.mode == COMPOSE_RHYTHM) {
		
		//if (sequencer.track_measure++ >= rhythm_track.length) sequencer.track_measure = rhythm_track.length;
		
		//sequencer.new_pattern = rhythm_track.patterns[sequencer.track_measure].current_pattern;
		//sequencer.pattern_bank = rhythm_track.patterns[sequencer.track_measure].current_bank;
		//flag.pattern_change = 1;			
			
	
		
	}
*/	
/*	if (sequencer.mode == PLAY_RHYTHM) {
		sequencer.track_measure++; //advance track measure
		
		if ((sequencer.variation_mode == VAR_AB) && (sequencer.current_variation == VAR_A)) { //kludge to get alternating A/B to play A then B before moving on to next track
			sequencer.track_measure--;
			//toggle_variation();
			sequencer.current_variation = sequencer.basic_variation = VAR_B;
			return; //nope can't return here because first part/second part isn't handled later in this function, you dunce.
		}
			
		if (sequencer.track_measure > rhythm_track.length) { //you done 
				
				sequencer.track_measure = 0;
				//flag.pattern_change = 1;
							
				if (sequencer.track_loop) {

					
				} else {
					
					sequencer.START = 0;
					process_stop();						
				
			}
		
			
		} else {
			//flag.pattern_change = 1;	
				
		}
		//this needs to be conditional is VAR_AB and B still needs to play, but wait - decrementing track measure will do the trick, no?
		sequencer.new_pattern = rhythm_track.patterns[sequencer.track_measure].current_pattern;
		sequencer.pattern_bank = rhythm_track.patterns[sequencer.track_measure].current_bank;
		flag.pattern_change = 1;	
			
			
	
		
	} else if (sequencer.mode == COMPOSE_RHYTHM) {
		
		
	}
	
*/
		
				
	if (sequencer.step_num[SECOND] != NO_STEPS) { //no toggling if second part has 0 steps - annoying exception handler
					
		if (sequencer.part_playing == SECOND) {
			sequencer.part_playing ^= 1 << 0; //toggle part playing
			update_pattern();
			
		} else {
			turn_off(FIRST_PART_LED);
			turn_on(SECOND_PART_LED);
			sequencer.part_playing ^= 1 << 0; //toggle part playing
		}
		
	} else {//annoying exception in the case of second part being reset to 0 steps
		
	
		sequencer.part_playing = FIRST; 
		update_pattern();


	}
	
	if (sequencer.mode == FIRST_PART || sequencer.mode == SECOND_PART) { //only need to update this when step number changes, right now it's being called at end of every measure!
		
		//update step number
		
		//sequencer.step_num[sequencer.part_editing] = sequencer.step_num_new; //will eventually want to be able to change step number in MANUAL PLAY mode, but leave it here for now
		
		//update_step_led_mask();
		update_inst_led_mask();
		
		}// else if {sequencer.mode == SECOND_PART)} {
		
		
		
		//}
	
				
		

				
	//handle pre-scale change
	if (flag.pre_scale_change) {
		flag.pre_scale_change = 0;
		sequencer.shuffle_amount = 0; //reset shuffle amount when pre-scale changes			
		clock.divider = pre_scale[sequencer.pre_scale];
	}	
	
	
}
void process_step(void){ 
	if (flag.next_step) {
		
		
		flag.next_step = 0;
		
		if (sequencer.START) {
		//check for reset
		//if (reset) flag.new_measure = 1;	
		//*************************TAKEN FROM INTERRUPT*****************************//
			if (flag.new_measure) {
			
				flag.new_measure = 0;
				//TRIGGER_OUT |= (1 << TRIGGER_OUT_2);
				process_new_measure(); //moved all the new measure housekeeping into its own function.
				//TRIGGER_OUT &= ~(1<<TRIGGER_OUT_2);
				if (!sequencer.START) return; //in RHYTHM PLAY mode, process_new_measure() will stop play at end of rhythm track and first step of next pattern will play unless we get out of here. Klunky junk.
				//sequencer.current_measure++;
			}			
		//*************************************************************************//
			//or what about (if !flag.trig_finished) return; ?
			//while(flag.trig_finished == 0); //make sure previous instrument trigger is finished before initiating next one - this really only applies when there is incoming MIDI data. May have to do away
			//with allowing drums to be triggered by MIDI when sequencer is running?
					
			//check_tap();
			//PORTD |= (1<<TRIG);
			switch (sequencer.mode) {
				
				case FIRST_PART: case SECOND_PART: case PATTERN_CLEAR:
					check_tap();
					if (sequencer.ALT) break;				
					if ((!sequencer.SHIFT) && sequencer.part_editing == sequencer.part_playing) {//only blink if the part playing is the same as the part being edited and SHIFT is not being held
						spi_data[LATCH_1] = (1 << sequencer.current_step) | sequencer.led_mask;//sequencer.step_led_mask[sequencer.variation][sequencer.current_inst];
						spi_data[LATCH_1] &= ~(sequencer.led_mask & (1<<sequencer.current_step));
						spi_data[LATCH_0] = ((1 << sequencer.current_step) >> 8) | (sequencer.led_mask >> 8);
						spi_data[LATCH_0] &= ~((sequencer.led_mask >> 8) & ((1<<sequencer.current_step) >>8));
						
					}				
				break;
				
				case MANUAL_PLAY:
					check_tap();
					if (sequencer.SHIFT) break;
					spi_data[LATCH_1] = (1 << sequencer.current_step) | (1<<sequencer.new_pattern);
					spi_data[LATCH_1] &= ~(1<< sequencer.current_step & (1<<sequencer.new_pattern));
			
					spi_data[LATCH_0] = ((1 << sequencer.current_step) >> 8) | ((1 << sequencer.new_pattern) >> 8) | ((1<<sequencer.current_intro_fill) >> 8);								
					spi_data[LATCH_0] &= ~(((1<<sequencer.current_step) >> 8) & ((1 << sequencer.new_pattern | 1 << sequencer.current_intro_fill) >> 8)); //little tricky to get the correct mask here
					
					//spi_data[0] &= ~(((1<<sequencer.current_step) >> 8) & ((1 << sequencer.new_pattern) >> 8) & ((1<<sequencer.current_intro_fill) >>8));// & ((1<<sequencer.current_intro_fill) >> 8));	
				break;
				
				case COMPOSE_RHYTHM: case PLAY_RHYTHM:
					if (sequencer.ALT || sequencer.TAP_HELD) break;
					spi_data[LATCH_1] = (1 << sequencer.current_step) | (1<<sequencer.new_pattern);
					spi_data[LATCH_1] &= ~(1<< sequencer.current_step & (1<<sequencer.new_pattern));
										
					spi_data[LATCH_0] = ((1 << sequencer.current_step) >> 8) | ((1 << sequencer.new_pattern) >> 8);
					spi_data[LATCH_0] &= ~(((1<<sequencer.current_step) >> 8) & ((1 << sequencer.new_pattern) >> 8)); //little tricky to get the correct mask here
				break;	
				
			}

			trigger_step(sequencer.part_playing);
			//if (sequencer.clock_mode == DIN_SYNC_MASTER) PORTD |= (1<<DIN_RESET);
			//TRIGGER_OUT &= ~(1 << TRIGGER_OUT_2);
		}
	  //update_spi();
	} else if (flag.half_step) {
		//uint8_t rhythm_track_led[12] = {ACCENT_1_LED, BD_2_LED, SD_3_LED, LT_4_LED, MT_5_LED, HT_6_LED, RS_7_LED, CP_8_LED, CB_9_LED, CY_10_LED, OH_11_LED, CH_12_LED};
		//flag.half_step = 0;
		if (!sequencer.SHIFT) turn_off_all_inst_leds();
		//if (!sequencer.SHIFT) turn_on(drum_hit[sequencer.current_inst].led_index);
		spi_data[LATCH_5] &= ~(led[BASIC_VAR_A_LED].spi_bit | led[BASIC_VAR_B_LED].spi_bit); //this clears basic variation LEDs
		if (sequencer.START) {
			spi_data[LATCH_1] = 0;
			spi_data[LATCH_0] = 0;	
					
			//if (sequencer.roll_mode == ROLL_32) {
				//trigger_drum(sequencer.current_inst, 0);	//default 15ms timer interrupt used in this function is going to cause problems at high tempos - consider making this 1 ms
			//} else if (sequencer.step_num[SECOND] == NO_STEPS) {
				//
				//trigger_step(SECOND);
			//}
			//if (sequencer.clock_mode == DIN_SYNC_MASTER) PORTD &= ~(1<<DIN_RESET);
			if (sequencer.step_num[SECOND] == NO_STEPS) {
				
				trigger_step(SECOND);
			} else if (sequencer.roll_mode == ROLL_32) {
				trigger_drum(sequencer.current_inst, 0);
			}
			
			
			//if (sequencer.step_num[SECOND] == NO_STEPS) { //trigger second part steps on half step to create substeps when second part has no length
				//
				//trigger_step(SECOND);
				//if (sequencer.roll_mode == ROLL_32) trigger_roll();
			//} else {
				//
				//
			//
			//}			
			switch (sequencer.mode) {
				
				case FIRST_PART: case SECOND_PART: case PATTERN_CLEAR:
				
					if (sequencer.SHIFT) {
						
						if (sequencer.ALT) { //show current current bank
							//turn_on(sequencer.pattern_bank);
							
						} else if (sequencer.TAP_HELD && sequencer.mode == PATTERN_CLEAR) {
							show_version_steps();
							
						} else {
							turn_on(sequencer.new_shuffle_amount); //turn on shuffle amount LED
							turn_on(sequencer.roll_mode + ROLL_MIN);
						}
					} else {
						if (sequencer.ALT) {
							turn_on(sequencer.pattern_bank);
						} else {
							turn_on(drum_hit[sequencer.current_inst].led_index);
							spi_data[LATCH_1] = sequencer.led_mask; //sequencer.step_led_mask[sequencer.variation][sequencer.current_inst]; //this keeps inst lights on while blinking step light
							spi_data[LATCH_0] = sequencer.led_mask >> 8;
																		
						}
					}
				break;
				
				case MANUAL_PLAY:
					if (sequencer.SHIFT) { 
						if (sequencer.ALT) {
														
							turn_on(sequencer.pattern_bank);
							
						} else if (sequencer.TAP_HELD) {
							
							if (sequencer.din_reset_enable) turn_on(TRIGGER_ENABLE);
						
						} else {
							turn_on(sequencer.new_shuffle_amount); //turn on shuffle amount LED
							turn_on(sequencer.roll_mode + ROLL_MIN);
							if (sequencer.live_hits) turn_on(LIVE_HITS);
							if (sequencer.trigger_enable) turn_on(TRIGGER_ENABLE);	
						}
					} else {
						if (!sequencer.live_hits) turn_on(drum_hit[sequencer.current_inst].led_index); //display current instrument if not in live_hits mode
						spi_data[LATCH_1] = (1<<sequencer.new_pattern);
						spi_data[LATCH_0] = (1<<sequencer.new_pattern) >> 8 | ((1<<sequencer.current_intro_fill) >> 8);
					}
				break;
				case COMPOSE_RHYTHM: case PLAY_RHYTHM:
					if (sequencer.ALT && (!sequencer.SHIFT)) {
						//if (sequencer.ALT) {
							
						turn_on(sequencer.pattern_bank);
							
						//} else {
							
						//}
					} else if (sequencer.TAP_HELD) {
						show_current_measure();
					} else {
						//turn_on(drum_hit[sequencer.current_rhythm_track].led_index);
						spi_data[LATCH_1] = (1<<sequencer.new_pattern);
						spi_data[LATCH_0] = (1<<sequencer.new_pattern) >> 8;
					}
				
				break;
				
			}

			sequencer.var_led_mask = led[BASIC_VAR_A_LED].spi_bit;
			if (sequencer.current_variation == sequencer.variation_mode) {
				
				//sequencer.var_led_mask = led[BASIC_VAR_A_LED].spi_bit;
				if (sequencer.current_variation == VAR_B && !flag.variation_change) sequencer.var_led_mask = led[BASIC_VAR_B_LED].spi_bit;
			} else {
				
				//sequencer.var_led_mask = led[BASIC_VAR_B_LED].spi_bit;
				if (sequencer.current_variation == VAR_B) sequencer.var_led_mask = led[BASIC_VAR_B_LED].spi_bit;
				
			}
			
					
			if (clock.beat_counter <2) { //handle LED behavior on half beat - keeps pattern and basic-variation LEDs flashing in time with internal master clock
				
				switch(sequencer.mode) {
					
					case FIRST_PART: case SECOND_PART: case PATTERN_CLEAR: //what about manual play? this allows immediate LED updating when patterns or shuffle/roll modes are changed - extend to other modes
					//so no switch case required?
						if (sequencer.SHIFT) { 
							if (sequencer.ALT) {
								turn_on(sequencer.new_pattern);
							} else {					
								turn_on(sequencer.new_shuffle_amount);
							}
						}
					
					break;
					
					case MANUAL_PLAY:
					
					break;
					
					case COMPOSE_RHYTHM:
					
					break;
					
					default:
					
					break;
					
					
				}

				
				if (sequencer.current_variation != sequencer.variation_mode) {
					if (sequencer.current_variation == VAR_A) {
						sequencer.var_led_mask |= led[BASIC_VAR_B_LED].spi_bit;
					} else {
						sequencer.var_led_mask |= led[BASIC_VAR_A_LED].spi_bit;
					}
					
				} else if (sequencer.current_variation == VAR_A && flag.variation_change) {
					
					sequencer.var_led_mask |= led[BASIC_VAR_B_LED].spi_bit;
				}
				
				

			}
					
		} else { //STOP LED behavior when sequencer is not running
					
			spi_data[LATCH_1] = 0;
			spi_data[LATCH_0] = 0;
				
			switch (sequencer.variation_mode) { //more efficient to use if/else here?
						
				case VAR_A: case VAR_AB:
				sequencer.var_led_mask = led[BASIC_VAR_A_LED].spi_bit;
				break;
						
				case VAR_B:
				sequencer.var_led_mask = led[BASIC_VAR_B_LED].spi_bit;
				break;
						
			}
			switch (sequencer.mode) {
			case MANUAL_PLAY:	
				
				if (sequencer.SHIFT) {	
					
					if (sequencer.ALT) {
						
						turn_on(sequencer.pattern_bank);
					} else if (sequencer.TAP_HELD) {	
						if (sequencer.din_reset_enable) turn_on(TRIGGER_ENABLE);
					} else {
						
						//need to display shuffle/roll/live hits status here
						if (sequencer.live_hits) turn_on(LIVE_HITS);
						if (sequencer.trigger_enable) turn_on(TRIGGER_ENABLE);
					}

				} else {
					
					if (flag.intro) {
						turn_on(sequencer.new_pattern);
					} else {
						turn_on(sequencer.current_intro_fill);						
					}					
					
				}
				break;
			
			case FIRST_PART: case SECOND_PART: case PATTERN_CLEAR:
				if (sequencer.SHIFT) {
					
					if (sequencer.ALT) {
						turn_on(sequencer.midi_channel);
						
					
					} else if (sequencer.TAP_HELD && sequencer.mode == PATTERN_CLEAR) {
						show_version_steps();
					} else {
						turn_on(clock.sync_count_index);
						//need to display shuffle/roll/live hits status here
						
					}
					
				} else {
					
					if (sequencer.ALT) {
						turn_on(sequencer.pattern_bank);
					}
				}
				break;
			case PLAY_RHYTHM: case COMPOSE_RHYTHM:
			
				if (sequencer.ALT && (!sequencer.SHIFT)) {
					
					turn_on(sequencer.pattern_bank);						
						
				} else if (sequencer.TAP_HELD) {
					show_current_measure();
				} else {
					
					
				}
				break;		
			//}
			}
			
			if ((clock.beat_counter <2) && ((!sequencer.SHIFT) && (!sequencer.TAP_HELD) && (!sequencer.ALT))) { //1/8 note, regardless of scale (based on original 808 behavior) - don't take this as gospel. may need to adjust with different pre-scales
						
				if (sequencer.variation_mode == VAR_AB) sequencer.var_led_mask |= led[BASIC_VAR_B_LED].spi_bit;	//turn on VAR_B LED for flashing to indicate A/B mode			
				turn_on(sequencer.new_pattern); //eventually need to turn on current pattern LED in pattern mode - other modes will require different behavior to be coded
				if (sequencer.mode == MANUAL_PLAY) turn_on(sequencer.current_intro_fill);
			}
		}
		flag.half_step = 0;		
		//spi_data[5] |= sequencer.var_led_mask;
		//update_spi();		
	} else if (clock.source == EXTERNAL && !sequencer.START) { //this handles variation LEDs when waiting for external clock signal
		spi_data[LATCH_5] &= ~(led[BASIC_VAR_A_LED].spi_bit | led[BASIC_VAR_B_LED].spi_bit); //this clears basic variation LEDs
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
	spi_data[LATCH_5] |= sequencer.var_led_mask;
}


void update_step_board() { //should this be in switches.c ?
	uint8_t press = EMPTY;
	if (sequencer.START) {
		press = check_step_press();
		if (press == EMPTY) return;
		
		switch (sequencer.mode) { 
			
		case FIRST_PART: case SECOND_PART:
			//press = check_step_press();
			//if (press == EMPTY) break;
			
			if (sequencer.SHIFT) {
				
				refresh_step_leds();
				if (sequencer.ALT) {
					if (sequencer.current_pattern != press) { //is this step necessary or is current/new pattern checked somewhere else?
						flag.pattern_change = 1;
						sequencer.new_pattern = sequencer.previous_pattern = press;
						turn_off(sequencer.current_pattern); 
						turn_on(sequencer.new_pattern);
						eeprom_write_recall_data(); //write pattern change to eeprom
					}
					
					
				} else {
					
					update_shuffle(press);
					
				}

				break;			
			} else if (sequencer.ALT) { //handle bank presses
				
				if ((press < NUM_BANKS) && (press != sequencer.pattern_bank)) {

					turn_off(sequencer.pattern_bank);
					sequencer.pattern_bank = sequencer.previous_bank = press;
					turn_on(sequencer.pattern_bank);
					if (sequencer.track_mode == EDIT) {
						rhythm_track.patterns[sequencer.track_measure].current_bank = sequencer.pattern_bank;						
					}
					flag.pattern_change = 1;
					eeprom_write_recall_data(); //write pattern change to eeprom
				}
				
				break;
			}

			if (sequencer.CLEAR) { //clear button is pressed, check if step buttons are pressed and change step number accordingly
				button[CLEAR_SW].state ^= button[CLEAR_SW].state; //need to reset CLEAR SW state here, otherwise it gets handled elsewhere when we don't want it to
				if ((press == 0) && (sequencer.step_num_new == 0) && (sequencer.mode == SECOND_PART)) {
					sequencer.step_num_new = NO_STEPS; //if pressing first step again then reset 2nd PART to NO_STEPS
					//now need to clear second part trigger data:
					sequencer.pattern[VAR_A].accent[SECOND] = 0;
					sequencer.pattern[VAR_B].accent[SECOND] = 0;
					memset(sequencer.pattern[VAR_A].part[SECOND], 0, sizeof(sequencer.pattern[VAR_A].part[SECOND]));
					memset(sequencer.pattern[VAR_B].part[SECOND], 0, sizeof(sequencer.pattern[VAR_B].part[SECOND]));
					flag.pattern_edit = 1;
				} else {
					sequencer.step_num_new = press;
					if (sequencer.step_num_new != (sequencer.step_num[sequencer.part_editing])) flag.pattern_edit = 1;
					
					
				}
				
				break; //break or return?
			}			
				
			if (sequencer.current_inst == AC) { //bah, inefficient duplicate code to handle ACCENT
				//press = check_step_press();
				//if (press != EMPTY)	{
				if (press <= sequencer.step_num[sequencer.part_editing]) { //need handle all button presses, but only use presses that are below current step number
					toggle(press);
					sequencer.pattern[sequencer.current_variation].accent[sequencer.part_editing] ^= 1<<press;
					sequencer.led_mask ^= 1<<press;
					flag.pattern_edit = 1;
				}
						
				//}

				return; //break or return?
			}
			 
			if (press <= sequencer.step_num[sequencer.part_editing]) {
				toggle(press); 
				//use ALT here to add presses to 2nd PART
				//uint8_t part_editing = sequencer.part_editing;
				//if ((sequencer.ALT) && (sequencer.part_editing == FIRST)) { //if ALT and editing first part, then edit second part as 32nd note layer on first part
					//part_editing = SECOND;
				//}
				sequencer.pattern[sequencer.current_variation].part[sequencer.part_editing][press] ^= 1<<sequencer.current_inst;
				sequencer.led_mask ^= 1<<press;
				flag.pattern_edit = 1;
				//toggle(IF_VAR_B_LED);
			//}					
			}

			break;
			
		case MANUAL_PLAY:
		
			if (sequencer.SHIFT) { //need to handle bank changes here too.
				
				if (sequencer.ALT) {
					if ((press < NUM_BANKS) && (press != sequencer.pattern_bank)) {

						turn_off(sequencer.pattern_bank);
						sequencer.pattern_bank = sequencer.previous_bank = press;
						turn_on(sequencer.pattern_bank);
						//read_next_pattern(sequencer.current_pattern, sequencer.pattern_bank);
						flag.pattern_change = 1;
					}
				} else {
					update_shuffle(press);
					if (press == LIVE_HITS) { //bit of kludge to make live hit toggling only work in MANUAL PLAY mode
						sequencer.live_hits ^= 1;
						if (sequencer.live_hits) turn_on(LIVE_HITS);
					} else if (press == TRIGGER_ENABLE) { //turning on trigger enable here maybe doesn't make sense as it doesn't work when sequencer is running?
						//sequencer.trigger_enable ^= 1;
						//if (sequencer.trigger_enable) turn_on(TRIGGER_ENABLE);
					}// else if (press == PERF_LOCK) {
						//
						//flag.mute_lock = 1;
						////led[MODE_4_MANUAL_PLAY].blink = 1;
						//
					//}
				}
				
				
			} else {

				if (press < NUM_BANKS) { //first 12 pattern places are for main patterns 
					sequencer.new_pattern = sequencer.previous_pattern = press;
					if (sequencer.new_pattern != sequencer.current_pattern) flag.pattern_change = 1;
				
				} else { //remaining 4 patterns places are for intro/fills
				
					sequencer.current_intro_fill = press;
						
				}
			}
			
			break;
				
		case PLAY_RHYTHM:
			
			break;
				
		case COMPOSE_RHYTHM:
			if (sequencer.ALT && (!sequencer.SHIFT)) {
						
				if ((press < NUM_BANKS) && (press != sequencer.pattern_bank)) {

					turn_off(sequencer.pattern_bank);
					sequencer.pattern_bank = press;
					turn_on(sequencer.pattern_bank);
					if (sequencer.track_mode == EDIT) {
						rhythm_track.patterns[sequencer.track_measure].current_bank = sequencer.pattern_bank;
						
					}
					flag.pattern_change = 1;
				}	
 
			} else if (sequencer.SHIFT) { //handle insert and delete here
				//there will be an issue if user holds WRITE/SHIFT long enough so that next measure is loaded. It will be unstaged, but will be confusing as sequencer will then play previous rhythm. Maybe not confusing?
				if (press == DELETE) { //need exception here for when deleting last pattern in rhythm track
					//if ((!flag.track_edit) && (sequencer.track_measure != rhythm_track.length)) { //confusing - if pattern has changed then track has been edited and write has NOT advanced to next measure, so only need to unstage track measure advance if track_edit flag is not set - see check_write_sw()
						//if (sequencer.track_measure > 0) sequencer.track_measure--; //unstage track measure advance 
						//sequencer.new_pattern = rhythm_track.patterns[sequencer.track_measure].current_pattern;
						//sequencer.pattern_bank = rhythm_track.patterns[sequencer.track_measure].current_bank;
					//}
					
					//if (sequencer.track_measure != rhythm_track.length) {
					if (sequencer.track_measure > 0) sequencer.track_measure--; //unstage track measure advance 
					//}
					delete_track_pattern(sequencer.track_measure);
					//if (sequencer.track_measure > 0) sequencer.track_measure--; //decrement current measure
					//if (sequencer.track_measure > rhythm_track.length) { //exception to handle deleting last pattern in rhythm track
					//	sequencer.track_measure = rhythm_track.length;
					//}
					sequencer.new_pattern = rhythm_track.patterns[sequencer.track_measure].current_pattern;
					sequencer.pattern_bank = rhythm_track.patterns[sequencer.track_measure].current_bank; 
					flag.track_edit = 1;
					flag.pattern_change = 1;
					
				} else if (press == INSERT) {
					if ((++rhythm_track.length) == NUM_PATTERNS) {
						rhythm_track.length = NUM_PATTERNS - 1;						
					}
					if (!flag.last_pattern) { 
						if (sequencer.track_measure > 0) sequencer.track_measure--; //unstage track measure advance
						sequencer.pattern_bank = rhythm_track.patterns[sequencer.track_measure].current_bank; //and also need to unstage setting pattern and bank, which is done in check_write_sw()
						sequencer.new_pattern = rhythm_track.patterns[sequencer.track_measure].current_pattern;
						insert_track_pattern(sequencer.track_measure);
					} else {
						
						sequencer.track_measure = rhythm_track.length; //if last pattern in rhythm track, then it's just appending, so go to last pattern
						
					}
					rhythm_track.patterns[sequencer.track_measure].current_bank = sequencer.pattern_bank;
					rhythm_track.patterns[sequencer.track_measure].current_pattern = sequencer.new_pattern;
					flag.pattern_change = 1;
					flag.track_edit = 1;
				}
				
				
			} else {
				sequencer.new_pattern = sequencer.previous_pattern = press; //want to be able to edit this pattern when cycling back to pattern edit mode
				if (sequencer.new_pattern != sequencer.current_pattern) {
					flag.pattern_change = 1;
					if (sequencer.track_mode == EDIT) {
						//rhythm_track.patterns[sequencer.track_measure].current_bank = sequencer.pattern_bank; //need to handle bank change just above, not here!
						rhythm_track.patterns[sequencer.track_measure].current_pattern = sequencer.new_pattern;
						flag.track_edit = 1;
						//flag.pattern_change = 1;
					}
					
					
				}
			}
			break;
				
		case PATTERN_CLEAR:
			
			break;		 			
		}
			
	} else { //SEQUENCER.STOP
		if (sequencer.mode == MANUAL_PLAY) check_tap(); //check toggling between intro and basic rhythm
		//handle changing selected pattern and rhythm.
		press = check_step_press();
		if (press != EMPTY) {
		
			switch (sequencer.mode) {
		
			case FIRST_PART: case SECOND_PART: case PATTERN_CLEAR:
			
				if (sequencer.SHIFT) {
				
					if (sequencer.ALT) { //change MIDI channel
						turn_off(sequencer.midi_channel);
						sequencer.midi_channel = press;
						turn_on(sequencer.midi_channel);					
						eeprom_write_recall_data(); //write midi channel change
					} else if (sequencer.clock_mode == DIN_SYNC_SLAVE) {// if (sequencer.TAP_HELD) { //update clock.sync_count?
						if (press < NUM_SYNC_COUNTS) {
							turn_off(clock.sync_count_index);
							clock.sync_count_index = press;
							turn_on(clock.sync_count_index);
							//uint8_t sync_count[6] = {
								//PPQN_24_TICK_COUNT,
								//PPQN_12_TICK_COUNT,
								//PPQN_8_TICK_COUNT,
								//PPQN_6_TICK_COUNT,
								//PPQN_4_TICK_COUNT,
								//PPQN_2_TICK_COUNT
							//};
							//clock.sync_count = sync_count[press];
							
						}

					
					}
				
				} else if (sequencer.ALT) { //change pattern bank
					
						//if current pattern has been edited need to write it to current bank before changing bank, otherwise edited pattern will be written to new bank!
						//BUT, can't edit patterns when sequencer isn't running except for changing pre-scale.
						if (press < NUM_BANKS) {
							if (flag.pattern_edit == 1) { //there is a bug where clearing a pattern in one bank messes with patterns in another bank. Perhaps the problem originates here. NOPE - bug is you not being able to tell the difference between 64 kilo*bits* and kilo*bytes*, you nitwit.							
								flag.pattern_edit= 0;						
								start_write_current_pattern(sequencer.current_pattern, sequencer.pattern_bank); //save changed pattern at end of measure	
													
							}
							turn_off(sequencer.pattern_bank);
							sequencer.pattern_bank = sequencer.previous_bank = press;
							turn_on(sequencer.pattern_bank);
							eeprom_write_recall_data(); //write pattern change to eeprom	
							read_next_pattern(sequencer.current_pattern, sequencer.pattern_bank);
						}					

			
				} else {
					
					if (sequencer.CLEAR) { //when the sequencer isn't running pressing clear will copy current pattern into new slot- copy/paste
										
						if (sequencer.current_pattern != press) {
							blocking_copy_active_pattern(press);
							
							// NOTE: this call below did not work because we suspect that it tried to read the pattern data from the pressed location before the write had completed
							// start_write_current_pattern(press, sequencer.pattern_bank); //write current pattern to new pattern slot
							
						}
						
						
					} else {
						
						read_next_pattern(press, sequencer.pattern_bank);	
					}
					sequencer.current_pattern = sequencer.new_pattern = sequencer.previous_pattern = press;
					eeprom_write_recall_data(); //write pattern change to eeprom	
					//sequencer.variation = VAR_A;
					sequencer.part_playing = FIRST;
					sequencer.current_step = 0;
					clock.ppqn_counter = 0; //need to reset ppqn_counter here. there's a glitch when switching to new patterns that can somehow cause overflow and next_step and half_step flags aren't set
					clock.beat_counter = 0;					
					
				}
				break;
		
			case MANUAL_PLAY: //change pattern bank here too? Probably
				if (sequencer.SHIFT) {
					
					if (sequencer.ALT) { //change pattern bank
						if ((press < NUM_BANKS) && (press != sequencer.pattern_bank)) {

							turn_off(sequencer.pattern_bank);
							sequencer.pattern_bank = sequencer.previous_bank = press;
							turn_on(sequencer.pattern_bank);
							read_next_pattern(sequencer.current_pattern, sequencer.pattern_bank);
						}
					} else if (sequencer.TAP_HELD) {
						if (press == TRIGGER_ENABLE) {//enable/disable din_reset
							sequencer.din_reset_enable ^= 1;
							if (sequencer.din_reset_enable) {
								turn_on(TRIGGER_ENABLE);
							} else {
								turn_off(TRIGGER_ENABLE);
							}
							eeprom_write_recall_data();	
						}
						
					} else {
						
						if (press == LIVE_HITS) {
							sequencer.live_hits ^= 1;
							if (sequencer.live_hits) turn_on(LIVE_HITS);
						} else if (press == TRIGGER_ENABLE) { //need to change so that trigger enable onl allowed when in SYNC OUT mode (to stop all drums from sounding when RUN/STOP is floating in other modes)
							sequencer.trigger_enable ^= 1;
							if (sequencer.trigger_enable) {
								turn_on(TRIGGER_ENABLE);
								DDRB |= (1<<SPI0_SCK); //set SCK as output to turn on TII LED
								//purge spi buffer here
								//sync_index = 2;
								//flag.purge_trigger_buffer = 1;
								set_clock_mode(DIN_SYNC_MASTER);
								//process_stop();
							} else {
								turn_off(TRIGGER_ENABLE);
								DDRB &= ~(1<<SPI0_SCK); //float SCK to turn off TII LED
							}
						}

					}
					
					
				} else {
					if (press < BASIC_RHYTHM) {
						sequencer.current_pattern = sequencer.new_pattern = sequencer.previous_pattern = press;
						read_next_pattern(sequencer.current_pattern, sequencer.pattern_bank);
						sequencer.part_playing = FIRST;
						sequencer.current_step = 0;
						clock.ppqn_counter = 0; //need to reset ppqn_counter here. there's a glitch when switching to new patterns that can somehow cause overflow and next_step and half_step flags aren't set
						clock.beat_counter = 0;
					} else {
						sequencer.current_intro_fill = press;
					}
				}
				break;
		
			case PLAY_RHYTHM:
			
				break;
			case COMPOSE_RHYTHM:
			
				if (sequencer.ALT && (!sequencer.SHIFT)) { //change pattern bank
					
					if ((press < NUM_BANKS) && (press != sequencer.pattern_bank)) {

						turn_off(sequencer.pattern_bank);
						sequencer.pattern_bank = sequencer.previous_bank = press;
						turn_on(sequencer.pattern_bank);
						read_next_pattern(sequencer.current_pattern, sequencer.pattern_bank);
					}


				} else {
					sequencer.current_pattern = sequencer.new_pattern = sequencer.previous_pattern = press;
					read_next_pattern(sequencer.current_pattern, sequencer.pattern_bank);
					sequencer.part_playing = FIRST;
					sequencer.current_step = 0;
					clock.ppqn_counter = 0; //need to reset ppqn_counter here. there's a glitch when switching to new patterns that can somehow cause overflow and next_step and half_step flags aren't set
					clock.beat_counter = 0;
				}
				break;	
			}				
		}			
	}
}

void update_shuffle(uint8_t shuffle_amount) {
	
	//if (sequencer.pre_scale == PRE_SCALE_1 || sequencer.pre_scale == PRE_SCALE_3) {
	
		if (shuffle_amount < SHUFFLE_MAX) { //ensure button press is within control range of shuffle selection
			//if shuffle changes need to change shuffle after step changes, otherwise next step flag may not be set on time
			sequencer.new_shuffle_amount = shuffle_amount; //shuffle ranges from 0-5
			//turn_on(shuffle_amount);
			flag.shuffle_change = 1;
			if (sequencer.mode ==  FIRST_PART || sequencer.mode == SECOND_PART) flag.pattern_edit = 1; //only save shuffle changes when in pattern edit mode
		} else if (shuffle_amount >= ROLL_MIN && shuffle_amount < ROLL_MAX) {
		
			sequencer.roll_mode = shuffle_amount - ROLL_MIN;
		
		} /*else if (shuffle_amount == LIVE_HITS) {
			//toggle live hits
			sequencer.live_hits ^= 1;
			
			
		}*/

	//need to turn off LEDs first for immediate response, otherwise response is step time dependent
	//if (sequencer.live_hits) turn_on(LIVE_HITS);
		
	turn_on(sequencer.new_shuffle_amount); //immediately update LEDs
	turn_on(sequencer.roll_mode + ROLL_MIN);
		
	//} else {
	//
		//sequencer.shuffle_amount = 0;
		//
	//}
 	
	
}

void update_variation(void) { //not currently used 
	
	if (flag.new_measure) {
		
		if (flag.variation_change == 1) {
			flag.variation_change = 0;
			switch (sequencer.variation_mode) {
							
				case VAR_A: case VAR_AB:
				sequencer.current_variation = VAR_A;
				break;
				case VAR_B:
				sequencer.current_variation = VAR_B;
				break;
												
			}
						
		} else if (sequencer.variation_mode == VAR_AB) {
						
			sequencer.current_variation ^= 1<<0; //toggle state
		}
		
	}
	
		if (flag.half_step) {
			
					
			
		}
	
}

void update_prescale(void) { //should PRE_SCALE be updated in modes other than 1st/2nd pattern edit? - NO
	
	if (button[CLEAR_SW].state && sequencer.SHIFT) {
	
		button[CLEAR_SW].state ^= button[CLEAR_SW].state; //toggle switch state
		
		if (sequencer.pre_scale-- == 0) { //decrement to go from 3 to 4 to 1 to 2 to 3...
			
			sequencer.pre_scale = NUM_PRE_SCALES -1;
					
		}
		flag.pre_scale_change = flag.pattern_edit = 1;
		//don't change clock divider here - can cause ppqn overflow, so only change divider on ppqn match event
		//clock.divider = pre_scale[sequencer.pre_scale];
		//update_prescale_leds();

	}
	update_prescale_leds(); //this is not efficient - need to reconcile updating pre_scale and updating pre_scale LEDs with TAP_HELD displaying current rhythm track pattern
}

void check_tap(void) { //this is kind of inefficient - not generalized enough. maybe better to check flag.tap in different contexts?
	
	if (flag.tap) {
		flag.tap = 0;
		if (sequencer.mode == FIRST_PART || sequencer.mode == SECOND_PART) {
			//flag.tap = 0;
			if (sequencer.SHIFT) { //fill current instrument in pattern
				if (sequencer.current_inst == AC) { //not valid for AC
					//sequencer.pattern[sequencer.variation].accent[sequencer.part_editing] = 0xFFFF;
					
					
				} else {
					
					for (int i = 0; i <= sequencer.step_num[sequencer.part_editing]; i++) {
						sequencer.pattern[sequencer.current_variation].part[sequencer.part_editing][i] |= 1<<sequencer.current_inst;
						sequencer.led_mask |= 1 << i;
					}
					
				}
				
				//sequencer.led_mask = 0xFFFF;
			//else -HERE IS WHERE TO PUT CLEAR + CURRENT_INST TO CLEAR ALL TRIGGERS FOR SELECTED INSTRUMENT
			} else if (sequencer.CLEAR) { //clear triggers for current instrument in pattern
				if (sequencer.current_inst == AC) { //not valid for AC
									
									
					} else {
									
					for (int i = 0; i <= sequencer.step_num[sequencer.part_editing]; i++) {
						sequencer.pattern[sequencer.current_variation].part[sequencer.part_editing][i] &= ~(1<<sequencer.current_inst);
						sequencer.led_mask &= ~(1 << i);
					}
									
				}
			
			} else { //add current instrument trigger to pattern
				if (sequencer.current_inst == AC) {
					sequencer.pattern[sequencer.current_variation].accent[sequencer.part_editing] |= 1<<sequencer.current_step;	
				} else {
					sequencer.pattern[sequencer.current_variation].part[sequencer.part_editing][sequencer.current_step] |= 1<<sequencer.current_inst;
				}
				sequencer.led_mask |= 1<<sequencer.current_step;
			}
			
			flag.pattern_edit = 1; //set pattern edit flag
		
		} else if (sequencer.mode == MANUAL_PLAY){ 
			
			//handle intro/fill in here
			if (sequencer.START) {
				if (sequencer.fill_mode == MANUAL) flag.fill = 1; //set fill flag
				//flag.pattern_change = 1;
				
			} else {
				
				flag.intro ^= 1<<0;
				if (flag.intro) { //sequencer isn't playing, so load intro now
					read_next_pattern(sequencer.current_intro_fill, sequencer.pattern_bank);
					sequencer.new_pattern = sequencer.current_pattern;
					sequencer.current_pattern = sequencer.current_intro_fill;					
					
					
				} else { //toggle back to current pattern (not tested yet - need to ensure current pattern is restored when toggling intro/fill
					sequencer.current_pattern = sequencer.new_pattern;
					read_next_pattern(sequencer.current_pattern, sequencer.pattern_bank);
					
					
					
				}
			}
			
		}
		
	}
	
	
}

void toggle_variation(void) {
	
	if (flag.variation_change == 1) {
		flag.variation_change = 0;
		switch (sequencer.variation_mode) {
					
			case VAR_A: case VAR_AB:
				sequencer.current_variation = VAR_A;
			break;
			case VAR_B:
				sequencer.current_variation = VAR_B;
			break;
					
					
		}
				
	} else if (sequencer.variation_mode == VAR_AB) {
				
		sequencer.current_variation ^= 1<<0; //toggle state
	}	
	
	
}



void read_next_pattern(uint8_t pattern_num, uint8_t pattern_bank) {
	//turn_on(ACCENT_1_LED);
	//TRIGGER_OUT |= (1<<TRIGGER_OUT_1);
	pattern_data next_pattern;
	
	next_pattern = read_pattern(pattern_num*PAGES_PER_PATTERN*PAGE_SIZE, pattern_bank);
	sequencer.pattern[VAR_A] = next_pattern.variation_a;
	sequencer.pattern[VAR_B] = next_pattern.variation_b;
	sequencer.step_num[FIRST] = next_pattern.step_num[FIRST];
	sequencer.step_num[SECOND] = next_pattern.step_num[SECOND];
	sequencer.pre_scale = next_pattern.pre_scale;
	clock.divider = pre_scale[sequencer.pre_scale];
	if (next_pattern.shuffle > SHUFFLE_MAX) next_pattern.shuffle = 0; //just leave this in here for beta testers so that pre-shuffle storage patterns aren't screwed up
	sequencer.new_shuffle_amount = next_pattern.shuffle;
	flag.shuffle_change = 1;
	//update_step_led_mask();
	update_inst_led_mask();
	update_prescale_leds();
	//sequencer.part_playing = sequencer.step_num_new = FIRST;
	
	sequencer.step_num_new = sequencer.step_num[sequencer.part_editing];
	
	//sequencer.part_playing = sequencer.part_editing;
	//turn_off(ACCENT_1_LED);
}

void read_rhythm_track(void) {
	
	rhythm_track = eeprom_read_rhythm_track(sequencer.current_rhythm_track);
	
	//*rhythm_track.patterns = *new_track.patterns;
	//rhythm_track = eeprom_read_rhythm_track(sequencer.current_rhythm_track);
	
	//memcpy(&rhythm_track.patterns, &new_track.patterns, sizeof(rhythm_track.patterns));
	/*for (int i = 0; i <= new_track.length; i++) {
			
		rhythm_track.patterns[i].current_bank = new_track.patterns[i].current_bank;
		rhythm_track.patterns[i].current_pattern = new_track.patterns[i].current_pattern;
	}
			
	rhythm_track.length = new_track.length;
	*/
	flag.pattern_change = 1; //but do we immediately read new pattern? Only when stopped. If running, then wait for current measure to finish?
	
}
void write_rhythm_track(void) {
	
	rhythm_track_data track;
	
	//memcpy(&track.patterns, &rhythm_track.patterns, sizeof(track.patterns));
	for (int i = 0; i <= rhythm_track.length; i++) {
		
		track.patterns[i].current_bank = rhythm_track.patterns[i].current_bank;
		track.patterns[i].current_pattern = rhythm_track.patterns[i].current_pattern;
		
	}
	track.length = rhythm_track.length;
	
	eeprom_write_rhythm_track(sequencer.current_rhythm_track, &track);
	
}

void update_rhythm_track(uint8_t track_number) {
	
	sequencer.previous_bank = sequencer.pattern_bank; //save current bank for restoring when returning to other modes - but really only need to do this for PLAY, because banks can be changed in compose mode
	sequencer.current_rhythm_track = track_number;
	read_rhythm_track();
	sequencer.track_measure = 0; //default is restart except when reading non-empty rhythm tracks in compose mode
	//if (sequencer.mode == COMPOSE_RHYTHM) {
		//if (rhythm_track.length != 0) sequencer.track_measure = rhythm_track.length + 1; //when changing rhythm tracks in COMPOSE mode, set measure to end of rhythm track so track can be appended. Will need to assert that rhythm_track.length is not at maximum				
//
	//}
	
	if (rhythm_track.length == 0) { 
		sequencer.track_mode = CREATE; 
	} else {
		sequencer.track_mode = EDIT;
	}
	sequencer.current_pattern = sequencer.new_pattern = rhythm_track.patterns[0].current_pattern;
	sequencer.pattern_bank = rhythm_track.patterns[0].current_bank;
	if (sequencer.START) {
		flag.pattern_change = 1;
			} else {
		read_next_pattern(sequencer.current_pattern, sequencer.pattern_bank);
	}	
	
}

void delete_track_pattern(uint8_t track_measure) {
	
	if (!flag.last_pattern) { //if deleting the last pattern no rearrangement necessary
		for (int i = track_measure; i < rhythm_track.length; i++) {
			rhythm_track.patterns[i].current_bank = rhythm_track.patterns[i + 1].current_bank;
			rhythm_track.patterns[i].current_pattern = rhythm_track.patterns[i + 1].current_pattern;
		}		
	}

	
	if (rhythm_track.length > 0) rhythm_track.length--; //decrement rhythm track length
	
	
}

void insert_track_pattern(uint8_t track_measure) {
	

	//if (!flag.last_pattern) { //if last pattern then just need to append pattern to track, don't need to rearrange everything, yeah?
		for (int i = rhythm_track.length; i > track_measure; i--) {
			rhythm_track.patterns[i].current_bank = rhythm_track.patterns[i - 1].current_bank;
			rhythm_track.patterns[i].current_pattern = rhythm_track.patterns[i - 1].current_pattern;
		}
	//}

	
}
