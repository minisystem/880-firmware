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
#include "twi_eeprom.h"

uint8_t mode_index = 1; //default mode is pattern edit 1st part
uint8_t fill_index = 0;
uint8_t sync_index = 0;


void update_clock_mode_leds(uint8_t sync_index) {
	
//MK1 panel graphics need the last 4 LEDs of AUTO FILL IN for indication
//MK2 panel graphics need the last 5 LEDs of AUTO FILL IN for indication
	
//******************************MK1************************************************//	
/*	if (sequencer.clock_mode == PULSE_SYNC_SLAVE) {
				
		spi_data[LATCH_2] |= (clock.sync_led_mask << (sync_index - 1));// & (clock.sync_led_mask <<; //AND this with toggle bit from SYNC handler in interrupts.c
				
	} else {
				
		spi_data[LATCH_2] |= (1 << sync_index);
	}
	*/
//*********************************************************************************//

//******************************MK2************************************************//
	if (sync_index < 1) {
		
		spi_data[LATCH_4] |= 1 << (sync_index + 7);
		
		
		} else {
		
		spi_data[LATCH_2] |= 1 << (sync_index -1);
		
	}

//*********************************************************************************//


	
	
	
}

void update_fill_leds(uint8_t fill_index) {
	
	if (fill_index < 2) {
		
		spi_data[LATCH_4] |= 1 << (fill_index + 6);
		
	} else {
		
		spi_data[LATCH_2] |= 1 << (fill_index - 2);
	}
	
}	

	
	

void check_mode_switch(void) { //rename to check_mode_switch
	
	if (button[MODE_SW].state) {
		
		button[MODE_SW].state ^= button[MODE_SW].state; //toggle switch state
		
		if (sequencer.SHIFT) {
			
			if ((sequencer.mode == COMPOSE_RHYTHM) && (sequencer.track_mode == CREATE)) { //need to unstage last rhythm track edit
				if (sequencer.track_measure > 0) sequencer.track_measure--;
				if (sequencer.track_measure > 0) {
					rhythm_track.length = sequencer.track_measure - 1;
				} else {
					rhythm_track.length = 0;
				}
				
			} else if (sequencer.mode == SECOND_PART && sequencer.ALT) {
				copy_part(SECOND);
			}
			if (mode_index-- == 0) mode_index = NUM_MODES - 1;
			
		} else {
			 if (sequencer.mode == FIRST_PART && sequencer.ALT) copy_part(FIRST);
			 if (++mode_index == NUM_MODES) mode_index = 0;
			 
		}
		
		if (flag.track_edit) {
			
			write_rhythm_track(); //write current pattern to eeprom
			flag.track_edit = 0;
		}
		
		//replace code below with 
		//set_mode(mode_index) or something so that mode can be set outside of update_mode() for TII setting
		set_mode(mode_index);
	
	}
		
}
	
void check_fill_switch(void) {
	uint8_t fill_mode[NUM_FILL_MODES] = {MANUAL, 15, 11, 7, 3, 1};
	
	//does this need to be done here? only need to clear bits if there's a change, so only if FILL_SW = 1, right?	
	spi_data[LATCH_4] &= FILL_MODE_LATCH_4_LED_MASK; //clear FILL LED bits
	spi_data[LATCH_2] &= FILL_MODE_LATCH_2_LED_MASK;

	if (button[FILL_SW].state) {
		button[FILL_SW].state ^= button[FILL_SW].state; //toggle switch state
		
		if (sequencer.SHIFT) {

			//change sync mode
			if (++sync_index == NUM_CLOCK_MODES) sync_index = 0;
			//move below into set_sync_mode(sync_index) so sync mode can be set outside update_fill_mode()
			set_clock_mode(sync_index);
			//eeprom_write_recall_data(); //save clock mode to recall memory - writing here causes spurious change to DIN run stop line
					
		} else if (sequencer.ALT) {
			
			eeprom_write_recall_data();
			
		} else {
					
			if (++fill_index == NUM_FILL_MODES) fill_index = 0;
			
			sequencer.fill_mode = fill_mode[fill_index];
			sequencer.current_measure_auto_fill = 0;			
			
		}
		
	}
	sync_index = sequencer.clock_mode;
	if (sequencer.SHIFT || sequencer.ALT) {
		update_clock_mode_leds(sync_index);
	} else {
		update_fill_leds(fill_index);
	}
}

void set_mode(uint8_t mode_index) {
	    enum global_mode current_mode[NUM_MODES] = {PATTERN_CLEAR, FIRST_PART, SECOND_PART, MANUAL_PLAY, PLAY_RHYTHM, COMPOSE_RHYTHM};
		sequencer.mode = current_mode[mode_index];
		//uint8_t data_mask = spi_data[4] & 0b11000000; //mask to preserve top two bits of SPI byte 4
		spi_data[LATCH_4] &= MODE_LED_MASK;
		spi_data[LATCH_4] |= (1<< mode_index);
		
		
		//if (sequencer.step_num[SECOND] != NO_STEPS) sequencer.step_num_new = sequencer.step_num[sequencer.part_editing]; //another annoying except
		
		switch (sequencer.mode) {
			
			case FIRST_PART: case SECOND_PART: //don't need to read pattern when switching between FIRST PART and SECOND PART - this is unnecessary overhead, but does it matter?
			//yes because you could read the next pattern before committing an edited pattern to memory... need to fix this
			sequencer.pattern_bank = sequencer.previous_bank;
			sequencer.current_pattern = sequencer.new_pattern = sequencer.previous_pattern;

			if (!flag.pattern_edit) read_next_pattern(sequencer.current_pattern, sequencer.pattern_bank); //not sure if this is good - new copy_part() function only sets flag.pattern_edit, so set_mode() reads pattern from rom before copy is written
			sequencer.part_editing = sequencer.mode == FIRST_PART? FIRST : SECOND;
			sequencer.step_num_new = sequencer.step_num[sequencer.part_editing];
			//update_step_led_mask(); //want to update led mask immediately, otherwise it only gets updated at end of measure
			update_inst_led_mask();
			break;

			case MANUAL_PLAY:
			//if current pattern is 12 or greater then it needs to be set to 12 to avoid overlap with intro/fill pattern selection - this mimics original TR-808 behaviour
			sequencer.pattern_bank = sequencer.previous_bank;
			if (sequencer.current_pattern > 11) { //make 11 a constant here
				sequencer.previous_pattern = sequencer.current_pattern; //save current pattern for return to pattern edit mode
				sequencer.new_pattern = 11;
				flag.pattern_change = 1; //bug here where another pattern is played?
			}
			break;
			
			case PLAY_RHYTHM: case COMPOSE_RHYTHM:
			//will need to update inst/track leds to current rhythm track, rather than resetting to 0
			sequencer.previous_bank = sequencer.pattern_bank;
			update_rhythm_track(sequencer.current_rhythm_track);

			break;
			

			default:
			
			break;
		}

			
}

void set_clock_mode(uint8_t sync_index) {
	//enum clock_mode clock_mode[5] = {MIDI_MASTER, MIDI_SLAVE, DIN_SYNC_MASTER, DIN_SYNC_SLAVE, PULSE_SYNC_SLAVE}; //add an extra mode here for SYNC_SLAVE where SYNC IN LED flashes when SHIFT is held
	sequencer.clock_mode = sync_index;// clock_mode[sync_index];
	TCCR2B = 0; //turn off Timer2
	EIMSK = 0; //turn off external interrupts
	PCICR = 0; //turn off pin change interrupts
	//spi_data[2] |= (1 << fill_index);
	//clock.sync_count = PPQN_24_TICK_COUNT; //restore default, only changes for external non-DIN sync pulse - this is how external clock rate is set - can be 24, 12, 8, 4 or 2 PPQN - need to implement interface to change clock response
	PORTC &= ~(1<<SYNC_LED_R);
	PORTE &= ~(1<<SYNC_LED_Y);
			
	//should DIN_CLOCK and DIN_RUN_STOP be set default as outputs here?
	//DDRD |= (1 << DIN_CLOCK | 1 << DIN_RUN_STOP);
	switch (sequencer.clock_mode) {
				
		case MIDI_MASTER:
		clock.source = INTERNAL;
		TCCR1B |= (1<<CS12); //ensure timer is ON


		break;
				
		case MIDI_SLAVE:
		clock.source = EXTERNAL;
		//TIMSK3 |= (1<<OCIE3A); //turn on timer3 output compare interrupt
		TCNT3 = 0; //reset timer3
		clock.sync_count = PPQN_24_TICK_COUNT;
		//sequencer.shuffle_multplier = 1;
		//PORTC |= (1<<SYNC_LED_R);
		PORTE |= (1<<SYNC_LED_Y);
		//spi_data[LATCH_2] |= (1 << sync_index);
		break;
				
		case DIN_SYNC_MASTER:
		clock.source = INTERNAL;
		TCCR1B |= (1<<CS12);
		//TIMSK3 &= ~(1<<OCIE3A); //disable timer3 interrupts
		//sequencer.shuffle_multplier = 4;
		//PORTD |= (1<<SYNC_LED_R);
		DDRD |= (1 << DIN_CLOCK | 1 << DIN_RUN_STOP);// | 1 << DIN_FILL | 1 << DIN_RESET); //set up DIN pins as outputs
		//TCCR2A |= (1 << COM2B0); //toggle OC2B/PD3 on compare match
		TCCR2A |= (1 << WGM21); //clear timer on OCRA compare match where OCRA = OCRB
		TCCR2B |= (1<<CS22) | (1<<CS21) | (1<<CS20);
		TCNT2 = 0;
		//spi_data[LATCH_2] |= (1 << sync_index);
		break;
				
		case DIN_SYNC_SLAVE:
		clock.source = EXTERNAL;
		//TIMSK3 |= (1<<OCIE3A); //turn on timer3 output compare interrupt
		TCNT3 = 0; //rest timer3
		//sequencer.shuffle_multplier = 1;
		//PORTC |= (1<<SYNC_LED_R);
		PORTE |= (1<<SYNC_LED_Y);
		
		//PIND |= (1 << DIN_RUN_STOP);
		EIMSK |= (1 << INT1);// | (1<< INT0); //turn on INT1 interrupt for DIN Sync clock and INT0 for external SYNC input jack
		PCICR |= (1 << PCIE2); //turn on pin change interrupt for RUN/STOP line
		DDRD &= ~((1 << DIN_CLOCK | 1 << DIN_RUN_STOP));// | 1 << DIN_FILL | 1 << DIN_RESET)); //set up DIN pins as inputs
		//spi_data[LATCH_2] |= (clock.sync_led_mask << sync_index);
		//clock.sync_count = PPQN_2_TICK_COUNT;
		//TCCR1B = 0; //stop master tempo timer - necessary?
		
				
		break;
				
		case PULSE_SYNC_SLAVE:
		clock.source = EXTERNAL;
		TCNT3 = 0;
		PORTC |= (1<<SYNC_LED_R);
		PORTE |= (1<<SYNC_LED_Y);
		//DDRD &= ~((1 << DIN_CLOCK | 1 << DIN_RUN_STOP | 1 << DIN_FILL | 1 << DIN_RESET)); //set up DIN pins as inputs
		EIMSK |= (1<< INT0); //turn on INT1 interrupt for DIN Sync clock and INT0 for external SYNC input jack
		//PCICR |= (1 << PCIE2); //turn on pin change interrupt for what?
		//PINC |= (1<<SYNC_LED_Y);
		clock.sync_led_mask = 0;
		clock.sync_count = PPQN_2_TICK_COUNT; //generate 48 internal clock pulses for each external clock pulse
	}
			
		
		
	
}
	