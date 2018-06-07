/*
 *main.c
 *Open808 firmware ATMEGA328PB
 *minisystem
 *system80.net
*/

#include <avr/io.h>
#include <stdio.h>
#define F_CPU 16000000UL

#include <avr/interrupt.h>
//#include <util/delay.h>
#include "mode.h"
#include "sequencer.h"
#include "hardware.h"
#include "leds.h"
#include "switches.h"
#include "spi.h"
#include "midi.h"
#include "drums.h"
#include "clock.h"
#include "adc.h"
#include "twi_eeprom.h"
#include "xnormidi-develop/midi.h"
#include "xnormidi-develop/midi_device.h"
#include "xnormidi-develop/bytequeue/bytequeue.h" //this is required for MIDI sending

MidiDevice midi_device;

//pattern_data next_pattern;



void refresh(void) {
	//if (sequencer.SHIFT) update_tempo(); //this analog reading is noisy - need to do it less often, like maybe only when shift is pressed? //meh, doesn't seem to make a huge difference.	
	if (clock.source == INTERNAL) {
		update_tempo(); 	
	}
	check_start_stop_tap();
	read_switches();
	parse_switch_data();
	if (sequencer.mode == MANUAL_PLAY && !sequencer.SHIFT) live_hits(); //live_hits() needs to be updated to work with synchronized spi updating. to prevent double triggering maybe update less frequently?
	update_mode();
	update_fill_mode();
	check_clear_switch();
	check_intro_fill_variation_switch();
	check_variation_switches();
	update_prescale(); //why does this need to be called constantly? At the moment, yes. But could just call when pre-scale is changed?
//	if (sequencer.mode == PLAY_RHYTHM || sequencer.mode == COMPOSE_RHYTHM ) {
    
//    test_update_track_leds();
    
//  } else {
	check_inst_switches();
	update_inst_leds();	//updating too frequently? Not a functional problem yet, but unecessary overhead?
//  }  
	check_write_sw();
	update_step_board();
	process_step();
	update_spi();
	
	write_next_pattern_page();
	
	PORTD &= ~(1<<TRIG); //is trigger pulse width long enough? Could be affecting accent - need to test.
	//TRIGGER_OUT &= TRIGGER_OFF;
	
}


int main(void)
{
		
    DDRD |= (1<<TRIG); //set PD5, TRIG to output
	
	//setup SPI
	DDRE |= (1<<SPI_MOSI) | (1<<SPI_SS) | (1<<SPI_LED_LATCH) | (1<<SPI_SW_LATCH); //set MOSI and SS as outs (SS needs to be set as output or it breaks SPI)
	DDRC |= (1<<SPI_CLK);
	DDRB |= (1<<SPI_EN);
	//DDRB &= ~((1<<TAP) | (1<<START_STOP)); //set start/stop tap pins as inputs
	
	PORTE &= ~(1<<SPI_MOSI) | 1<<SPI_LED_LATCH | 1<<SPI_SW_LATCH;
	PORTC &= ~(1<<SPI_CLK);
	PORTB &= ~(1<<SPI_EN); //active low - yeah but have you enabled internal pullup?????
	
	PORTE |= (1<<SPI_LED_LATCH); //toggle LED LATCH HIGH (disabled)
	
	SPCR1 = (1<<SPE1) | (1<<MSTR1); //Start SPI as MASTER
	SPSR1 |= (1<<SPI2X); //set clock rate to XTAL/2 (8 MHz)
	
	//setup SYNC LEDs as outputs
	DDRE |= (1<<SYNC_LED_R);
	DDRC |= (1<<SYNC_LED_Y);
	
	//setup TRIGGER 1 and 2 pins as outputs
	DDRB |= (1<<TRIGGER_OUT_1);
	DDRB |= (1<<TRIGGER_OUT_2);

	
	//setup external interrupts
	EICRA |= (1 << ISC11) | (1 << ISC10) | (1 << ISC01); //set up DIN sync to trigger on rising edge of DIN clock, falling edge for INT0, external SYNC in jack
	PCMSK2 |= (1 << PCINT20); //set up DIN Run/Stop pin change interrupt
	
	sequencer.current_pattern = sequencer.new_pattern = 0;
	turn_on(sequencer.current_pattern);
	turn_on(MODE_2_FIRST_PART_PART);
	turn_on(FILL_MANUAL);
	
	spi_data[LATCH_3] |= CONGAS_OFF; //set LT, MT and HT switches to keep congas OFF when not in use - lowers background noise from howling congas
	
	update_spi();
	
	//setup Timer0 for drum triggering interrupt	
	TCCR0A |= (1<<WGM01); //clear on compare match A
	//OCR0A = 225; //gives period of about 0.9ms
	TIMER0_OUTPUT_COMPARE = TIMER0_1_MS; //1 ms pulse width for trigger is for MIDI triggering. 15 ms pulsewidth is for internal sequencer. 15 ms is too long for MIDI, noticeable delay can be heard when triggering simultaneous MIDI notes.
	//setup MIDI
	//initialize MIDI device
	midi_device_init(&midi_device);
	
	//register callbacks
	midi_register_noteon_callback(&midi_device, note_on_event);
	midi_register_noteoff_callback(&midi_device, note_off_event);
	midi_register_realtime_callback(&midi_device, real_time_event);
	//midi_register_songposition_callback(&midi_device, song_position_event);
	//setup MIDI USART
	setup_midi_usart();
	
	sequencer.midi_channel = 0;
	
	setup_clock();
	//sequencer.pre_scale = PRE_SCALE_3;
	clock.divider = PRE_SCALE_3;//.pre_scale;; //6 pulses is 1/16th note - this is are default fundamental step
	clock.ppqn_counter = 1;
	clock.source = INTERNAL;
	clock.sync_count = PPQN_24_TICK_COUNT; //24 PPQN external sync is default
	//clock.rate = 400; //use fixed rate to get clock working
	//update_clock_rate(clock.rate);
	setup_adc();
	flag.trig_finished = 1;
	flag.pre_scale_change = 0;
	sequencer.START = 0;
	//update_tempo();
	
	//set up default start up state. Eventually this should be recalled from EEPROM
	//sequencer.step_num[FIRST] = 15; //0-15 - default 16 step sequence - will change with pre-scale? and can by dynamically changed while programming pattern
	//sequencer.step_num[SECOND] = NO_STEPS; //default is that second part is not active
	//sequencer.step_num_new = 15;
	sequencer.variation_mode = VAR_A;
	turn_on(BASIC_VAR_A_LED);
	sequencer.intro_fill_var = VAR_A;
	turn_on(IF_VAR_A_LED);
	sequencer.mode = FIRST_PART;
	//sequencer.SLAVE = 0;
	sequencer.clock_mode = MIDI_MASTER;

	sequencer.part_playing = FIRST;
	sequencer.part_editing = FIRST;
	turn_on(FIRST_PART_LED);
	turn_on(SCALE_3_LED);
	sequencer.current_intro_fill = 12;//first of 4 intro/fill patterns
	sequencer.fill_mode = MANUAL;
	sequencer.pattern_bank = 0;
	sequencer.roll_mode = 0; 
	eeprom_init();
	//flag.twi_init_error = 0;
	

	
	//DDRE |= (1<<PE0); //set PE0, pin 3 as output for diagnostic purposes (currently used for TWI timing measurement)
	//DDRD |= 1 << PD3; //set up PD3 as output 
	sei(); //enable global interrupts	
	//sequencer.pattern[0] = read_pattern(0);
	//clear_all_patterns(); //run just to clean out eeprom
	//clear_pattern_bank(0);
	//clear_pattern_bank(1);
	//clear_pattern_bank(2);
	//clear_pattern_bank(3);
	flag.last_pattern = 0;
	sequencer.previous_pattern = 0;
	sequencer.previous_bank = 0;
	read_next_pattern(0, 0); //load first pattern
	//update_step_led_mask();
	
	//sequencer.SHUFFLE = 1;
	sequencer.shuffle_amount = 0;
	sequencer.shuffle_multplier = 2;
	
	//set default trigger assignments:
	sequencer.trigger_1 = RS;
	sequencer.trigger_2 = MA;
	
    while (1) 
    {
		//PORTD |= (1<< PD3);
		//TRIGGER_OUT |= 1 << TRIGGER_OUT_1;
		midi_device_process(&midi_device); //this needs to be called 'frequently' in order for MIDI to work
		refresh();
		//TRIGGER_OUT &= ~(1 << TRIGGER_OUT_1);
		
		//PORTD &= ~(1<<PD3);

	}
}

