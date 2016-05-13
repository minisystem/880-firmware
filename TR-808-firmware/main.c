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
#include "hardware.h"
#include "leds.h"
#include "switches.h"
#include "spi.h"
#include "midi.h"
#include "drums.h"
#include "xnormidi-develop/midi.h"
#include "xnormidi-develop/midi_device.h"
//#include "xnormidi-develop/bytequeue/bytequeue.h" //this is required for MIDI sending

MidiDevice midi_device;


//struct step {
	//
	//struct button *button;
	//struct led *led;
	//
	//};	
	//
//struct step step[16];	
//
//step[0].button = &button[STEP_1_SW];

	
uint8_t step_number = 0;	

void update_step_board() {
	
	for (int i = 0; i < 16; i++) { //button and led indices match for 0-15. How convenient.
		
		if (button[i].state) {
			
			toggle(i);
			button[i].state ^= button[i].state;
			
		}
		
	}

	
	update_spi();
	
	
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
	
	read_switches();
	parse_switch_data();
	live_hits();
	update_step_board();

	
}


void note_on_event(MidiDevice * device, uint8_t status, uint8_t note, uint8_t velocity) {
	
	//spi_data[1] = 1<<step_number;
	//spi_data[0] = (1<<step_number)>>8;
	//if (step_number++ == 15) step_number = 0;
	if (note < 16) {
		
		//problem is hits can come in before trigger is finished. need to ensure that timer is done
		//while(current_drum_hit != -1 );//move this into trigger_drum()
		trigger_drum(note, velocity);
		//spi_data[drum_hit[note].spi_byte_num] |= drum_hit[note].trig_bit;
		//spi_data[drum_hit[note].spi_led_byte_num] |= drum_hit[note].led_bit;
		//
		//if (drum_hit[note].switch_bit != -1) {//need to set instrument switch
			//
			//
			//spi_data[3] ^= (-(drum_hit[note].switch_value) ^ spi_data[3]) & drum_hit[note].switch_bit; //this sets switch_value in spi_data byte to switch_value (0 or 1)
			//
		//}
	//
	//if (velocity > 64) {
		//spi_data[8] |= (1<<ACCENT);
		//turn_on(ACCENT_1_LED);
	//}
		//PORTD |= 1<<TRIG; //move all of this into one tidy function something like play_drum(drum_index) - this will then be applicable to sequencer as well
		//
		//update_spi();
		//
//
		//
		//PORTD &= ~(1<<TRIG);
		//
		//_delay_us(900); //deal with this bullshit
		//
		//spi_data[drum_hit[note].spi_byte_num] &= ~(drum_hit[note].trig_bit);
		//spi_data[drum_hit[note].spi_led_byte_num] &= ~(drum_hit[note].led_bit);
		//spi_data[8] &= ~(1<<ACCENT);
		//turn_off(ACCENT_1_LED);
		//
		//update_spi();
		//

		
		
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
	
	sei(); //enable global interrupts	
	
    while (1) 
    {
	midi_device_process(&midi_device); //this needs to be called 'frequently' in order for MIDI to work
	refresh();		

	
	}
}

