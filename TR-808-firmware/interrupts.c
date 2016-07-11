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
#include "xnormidi-develop/bytequeue/bytequeue.h"
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
	turn_off(ACCENT_1_LED);
	spi_data[8] &= ~(1<<ACCENT);
	turn_off(drum_hit[current_drum_hit].led_index);
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
	
	if (clock.source == INTERNAL) {

		process_tick();
		if (sequencer.START) {
			if (sequencer.sync_mode == MIDI_MASTER) {
				midi_send_clock(&midi_device); //send MIDI clock
				
			} else { //send DIN Sync clock pulse
								
				TCCR2A |= (1 << COM2B1) | (1 << COM2B0); //set up OCR2B to set on compare match				
				TCCR2B |= (1 << FOC2B);// force output compare to set OCR2B (PD3) - normal PORTD functions are overridden, so need to use timer events to set or clear PD3
				//now  set up Timer2 to automatically clear PD3 on compare match:
				TCCR2A &= ~(1 << COM2B0);
				TCNT2 = 0;	//reset timer
				
			}
			
		}
		//if (sequencer.START && sequencer.sync_mode == MIDI_MASTER) {
			//midi_send_clock(&midi_device); //send MIDI clock
		//} else { //send DIN sync tick
			//
			//
		//}
	}
	//if (clock.source == INTERNAL) {//could set tick flag here and process it in one function used by both MIDI, DIN and INTERNAL clocks?
		//if (++clock.ppqn_counter == clock.divider) {
			//flag.next_step = 1;
			//if (sequencer.current_step++ == sequencer.step_num[sequencer.part_playing] && sequencer.START) flag.new_measure = 1;
			//clock.beat_counter++; //overflows every 4 beats
			//clock.ppqn_counter = 0;
		//} else if (clock.ppqn_counter == clock.divider >> 1) { //50% step width, sort of - this is going to get long and complicated fast - need to set flag and handle in main loop refresh function
			//
			//flag.half_step = 1;
//
		//} 
//
		//
	//}
	
	

	
}

ISR (USART0_RX_vect) { // USART receive interrupt
	//PORTB ^= (1<<ARP_SYNC_LED); //toggle arp VCO_SYNC_LATCH_BIT LED
	uint8_t inByte = UDR0;
	midi_device_input(&midi_device, 1, &inByte);
	//calling a function in an interrupt is inefficient according to AVR C guidelines
	// so this function should maybe be inlined in main loop if inByte is made volatile
	//***HOWEVER***, xnor-midi example code has this function being called from USART_RX_vect ISR
}

//ISR (USART0_TX_vect) {
//
//
	//
//}
ISR(USART0_UDRE_vect) {
	uint8_t val;

	// check if bytes are available for transmission

	if (bytequeue_length(&midi_byte_queue) > 0) {
		//first, grab a byte
		val = bytequeue_get(&midi_byte_queue, 0);

		//then transmit
		//and remove from queue
		UDR0 = val;
		bytequeue_remove(&midi_byte_queue, 1);
	}

	// if queue is empty, stop!
	if(bytequeue_length(&midi_byte_queue) == 0)
	UCSR0B &= ~( 1 << UDRIE0 );
}