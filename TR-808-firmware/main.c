/*
 *main.c
 *JR-808 firmware ATMEGA328PB
 *minisystem
 *system79.com
*/

#include <avr/io.h>
#include <stdio.h>
#define F_CPU 16000000UL

#include <avr/interrupt.h>
#include <util/delay.h>
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
#include "xnormidi-develop/midi.h"
#include "xnormidi-develop/midi_device.h"
//#include "xnormidi-develop/bytequeue/bytequeue.h" //this is required for MIDI sending

MidiDevice midi_device;

static uint16_t new_tempo_adc = 0;
static uint16_t current_tempo_adc = 0;

	
uint8_t step_number = 0;	

void update_step_led_mask(void) {
	
	sequencer.step_led_mask[sequencer.current_inst] = 0;
	for (int i = 0; i < 16; i++) {
		
		sequencer.step_led_mask[sequencer.current_inst] |= sequencer.current_pattern.first_part[i] & (1<<sequencer.current_inst);
		
	}
	
}

void update_tempo() {
	
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

void update_step_board() {
	
	if (sequencer.START && (sequencer.mode == PATTERN_FIRST || sequencer.mode == PATTERN_SECOND)) {
		
		if (sequencer.current_inst == AC) { //bah, inefficient duplicate code to handle ACCENT
				
			for (int i = 0; i < 16; i++) { //button and led indices match for 0-15. How convenient.
							
				if (button[i].state) {
								
					toggle(i);
					button[i].state ^= button[i].state;
					sequencer.current_pattern.accent[i] ^= 1<<0; //just toggle first bit
					sequencer.step_led_mask[sequencer.current_inst] ^= 1<<i; //this creates array out of bound issue, because AC = 16. Why no compile errors or warnings?
				}
			}
			return;		
		}
		for (int i = 0; i < 16; i++) { //button and led indices match for 0-15. How convenient.
				
			if (button[i].state) {
					
				toggle(i);
				button[i].state ^= button[i].state;
				sequencer.current_pattern.first_part[i] ^= 1<<sequencer.current_inst; //just work with first part of pattern and only 16 steps for now				
				sequencer.step_led_mask[sequencer.current_inst] ^= 1<<i;				
			}			
		}
	}
}

void live_hits(void) {
	
	if (button[INST_BD_2_SW].state) {
		
		button[INST_BD_2_SW].state ^= button[INST_BD_2_SW].state;
		trigger_drum(BD, 0);
	}
	
	if (button[INST_SD_3_SW].state) {
		
		button[INST_SD_3_SW].state ^= button[INST_SD_3_SW].state;
		trigger_drum(SD,0);
	}
	
	if (button[INST_LT_4_SW].state) {
		
		button[INST_LT_4_SW].state ^= button[INST_LT_4_SW].state;
		trigger_drum(LT, 0);
	}
	
	if (button[INST_MT_5_SW].state) {
		
		button[INST_MT_5_SW].state ^= button[INST_MT_5_SW].state;
		trigger_drum(MT,0);
	}	
	
	if (button[INST_HT_6_SW].state) {
		
		button[INST_HT_6_SW].state ^= button[INST_HT_6_SW].state;
		trigger_drum(HT, 0);
	}
	
	if (button[INST_RS_7_SW].state) {
		
		button[INST_RS_7_SW].state ^= button[INST_RS_7_SW].state;
		trigger_drum(RS,0);
	}
	
	if (button[INST_CP_8_SW].state) {
		
		button[INST_CP_8_SW].state ^= button[INST_CP_8_SW].state;
		trigger_drum(CP, 0);
	}
	
	if (button[INST_CB_9_SW].state) {
		
		button[INST_CB_9_SW].state ^= button[INST_CB_9_SW].state;
		trigger_drum(CB,0);
	}	
	if (button[INST_CY_10_SW].state) {
		
		button[INST_CY_10_SW].state ^= button[INST_CY_10_SW].state;
		trigger_drum(CY,0);
	}
	
	if (button[INST_OH_11_SW].state) {
		
		button[INST_OH_11_SW].state ^= button[INST_OH_11_SW].state;
		trigger_drum(OH, 0);
	}
	
	if (button[INST_CH_12_SW].state) {
		
		button[INST_CH_12_SW].state ^= button[INST_CH_12_SW].state;
		trigger_drum(CH,0);
	}
		
	
}




void refresh(void) {
	update_tempo();
	read_switches();
	check_start_stop_tap();
	
	parse_switch_data();
	if (sequencer.mode == MANUAL_PLAY) live_hits();
	update_mode();
	check_inst_switches();
	update_step_board();
	if (sequencer.START) { //this is an effort to synchronize SPI update within main loop - basically manipulate SPI data bytes and then do one single update_spi() call per loop
		
		if (sequencer.next_step_flag) {
			sequencer.next_step_flag = 0;
			while(sequencer.trigger_finished == 0); //make sure previous instrument trigger is finished before initiating next one
			PORTD |= (1<<TRIG);
			spi_data[1] = (1 << sequencer.current_step) | sequencer.step_led_mask[sequencer.current_inst];// | sequencer.current_pattern.first_part[sequencer.current_inst];
			spi_data[1] &= ~(sequencer.step_led_mask[sequencer.current_inst] & (1<<sequencer.current_step));
			spi_data[0] = ((1 << sequencer.current_step) >> 8) | (sequencer.step_led_mask[sequencer.current_inst] >> 8);// | (sequencer.current_pattern.first_part[sequencer.current_inst] >> 8);
			spi_data[0] &= ~((sequencer.step_led_mask[sequencer.current_inst]>>8) & ((1<<sequencer.current_step) >>8));
			trigger_step(); 
			if (sequencer.current_pattern.accent[sequencer.current_step] &1) spi_data[8] |= 1<<ACCENT;
			TIMSK0 |= (1<<OCIE0A); //enable output compare match A
			TCCR0B |= (1<<CS01) | (1<<CS00); //set to /64 of system clock start timer
			sequencer.trigger_finished = 0;
			
		} else {
			

			
		}		
	} else if (sequencer.next_step_flag){
		
			sequencer.next_step_flag = 0;
			//spi_data[1] = 0;
			//spi_data[0] = 0;
			//turn_on(STEP_1_LED);
		
	}
	
	if (sequencer.trigger_finished) { //hmmm. trigger width doesn't seem to matter. in this case, it's several 10s of milliseconds
		
		//sequencer.trigger_finished = 0;
		//clear_all_trigs();
		//spi_data[8] = 0; //err wait a sec - this trigger only works for AC, BC...CP. Does this mean only rising edge matters? Need to look into this.
	}
	
	update_spi();
	PORTD &= ~(1<<TRIG);
	//if (trigger_finished && sequencer.SHIFT) update_tempo(); //turning off SPI during pot read creates problem for trigger interrupt
	
}


void note_on_event(MidiDevice * device, uint8_t status, uint8_t note, uint8_t velocity) {
	

	if (note < 16) { //TODO: implement MIDI learn function to dynamically map notes to drum hits
		
		trigger_drum(note, velocity);
	
	}

		
}

void note_off_event(MidiDevice * device, uint8_t status, uint8_t note, uint8_t velocity) {}

void real_time_event(MidiDevice * device, uint8_t real_time_byte) {}

void setup_midi_usart(void)
{
	uint16_t ubbr_value = 31; //16MHz/(16*31250 BAUD) - 1
	//write ubbr_value to H and L UBBR1 registers:
	UBRR0L = (unsigned char) ubbr_value;
	UBRR0H = (unsigned char) (ubbr_value >> 8);
	
	UCSR0B = (1<<RXEN0)|(1<<TXEN0) | (1<<RXCIE0);
	//UCSR0C |= (0<<UMSEL0)|(0<<UMSEL01)|(0<<UPM01)|(0<<UPM00)|(0<<USBS0)|(0<<UCSZ02)|(1<<UCSZ01)|(1<<UCSZ00);
}



ISR (USART0_RX_vect) { // USART receive interrupt
	//PORTB ^= (1<<ARP_SYNC_LED); //toggle arp VCO_SYNC_LATCH_BIT LED
	uint8_t inByte = UDR0;
	midi_device_input(&midi_device, 1, &inByte);
	//calling a function in an interrupt is inefficient according to AVR C guidelines
	// so this function should maybe be inlined in main loop if inByte is made volatile
	//***HOWEVER***, xnor-midi example code has this function being called from USART_RX_vect ISR
}



int main(void)
{
	
	
	
    DDRD |= (1<<TRIG); //set PD5, TRIG to output
	
	//setup SPI
	DDRE |= (1<<SPI_MOSI) | (1<<SPI_SS); //set MOSI and SS as outs (SS needs to be set as output or it breaks SPI
	DDRC |= (1<<SPI_CLK) | (1<<SPI_LED_LATCH) | (1<<SPI_SW_LATCH);
	DDRB |= (1<<SPI_EN);
	//DDRB &= ~((1<<TAP) | (1<<START_STOP)); //set start/stop tap pins as inputs
	
	PORTE &= ~(1<<SPI_MOSI );
	PORTC &= ~(1<<SPI_CLK | 1<<SPI_LED_LATCH | 1<<SPI_SW_LATCH);
	PORTB &= ~(1<<SPI_EN); //active low
	
	PORTC |= (1<<SPI_LED_LATCH); //toggle LED LATCH HIGH (disabled)
	
	SPCR1 = (1<<SPE1) | (1<<MSTR1); //Start SPI as MASTER
	SPSR1 |= (1<<SPI2X); //set clock rate to XTAL/2 (8 MHz)
	
	
	
	turn_on(STEP_1_LED);
	turn_on(MODE_2_PATTERN_FIRST_PART);
	turn_on(FILL_MANUAL);
	
	update_spi();
	
	
	//setup Timer0 for drum triggering interrupt
	
	TCCR0A |= (1<<WGM01); //clear on compare match A
	OCR0A = 225; //gives period of about 0.9ms
	
	
	
	
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
	
	setup_internal_clock();
	internal_clock.divider = 6; //6 pulses is 1/16th note - this is are default fundamental step
	internal_clock.ppqn_counter = 1;
	//internal_clock.rate = 400; //use fixed rate to get clock working
	//update_clock_rate(internal_clock.rate);
	setup_adc();
	sequencer.trigger_finished = 1;
	sequencer.START = 0;
	//update_tempo();
	sei(); //enable global interrupts	
	
    while (1) 
    {
	midi_device_process(&midi_device); //this needs to be called 'frequently' in order for MIDI to work
	
	refresh();		

	
	}
}

