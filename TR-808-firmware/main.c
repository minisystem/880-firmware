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



void refresh(void) {
	//if (sequencer.SHIFT) update_tempo(); //this analog reading is noisy - need to do it less often, like maybe only when shift is pressed?
	update_tempo(); //meh, doesn't seem to make a huge difference.
	read_switches();
	check_start_stop_tap();
	
	parse_switch_data();
	if (sequencer.mode == MANUAL_PLAY) live_hits();
	update_mode();
	check_clear_switch();
	check_variation_switches();
	//update_variation();
	check_inst_switches();	
	update_step_board();
	//update_step_led_mask();
	process_step();
	
	if (flag.trig_finished) { //hmmm. trigger width doesn't seem to matter. in this case, it's several 10s of milliseconds. Will still be needed for MIDI sequencing
		
		//flag.trig_finished = 0;
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
	
	UCSR0B = (1<<RXEN0)|(1<<TXEN0) | (1<<RXCIE0) | (1<<TXCIE0);
	DDRD |= (1<<PD1); //set PD1 and UART TX
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

ISR (USART0_TX_vect) {


	
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
	turn_on(MODE_2_FIRST_PART_PART);
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
	flag.trig_finished = 1;
	sequencer.START = 0;
	//update_tempo();
	
	//set up default start up state. Eventually this should be recalled from EEPROM
	sequencer.step_num[FIRST] = 15; //0-15 - default 16 step sequence - will change with pre-scale? and can by dynamically changed while programming pattern
	sequencer.step_num[SECOND] = NO_STEPS; //default is that second part is not active
	sequencer.step_num_new = 15;
	sequencer.variation_mode = VAR_A;
	turn_on(BASIC_VAR_A_LED);
	sequencer.mode = FIRST_PART;
	sequencer.part_playing = FIRST;
	sequencer.part_editing = FIRST;
	turn_on(FIRST_PART_LED);
	turn_on(SCALE_3_LED);
	sei(); //enable global interrupts	
	
    while (1) 
    {
	midi_device_process(&midi_device); //this needs to be called 'frequently' in order for MIDI to work
	
	refresh();		

	
	}
}

