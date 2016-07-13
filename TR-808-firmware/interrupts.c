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

	spi_data[drum_hit[current_drum_hit].spi_byte_num] &= ~(drum_hit[current_drum_hit].trig_bit);
	turn_off(ACCENT_1_LED);
	spi_data[8] &= ~(1<<ACCENT);
	turn_off(drum_hit[current_drum_hit].led_index);

	trigger_finished = 1;
	
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
				//this set up counter-intuitively puts DIN clock out of phase with master timer. Not sure why this is the case, but it works. Really need to dig into this. Not sure what other horrors this will reveal
				//need to have a good long think about this and figure out what the problem is. 
				TCCR2A &= ~(1 << COM2B0);			
				TCCR2B |= (1 << FOC2B);// force output compare to CLEAR OCR2B (PD3) - normal PORTD functions are overridden, so need to use timer events to set or clear PD3
				//now  set up Timer2 to automatically clear PD3 on compare match:	
				TCCR2A |= (1 << COM2B1) | (1 << COM2B0); //set up OCR2B to set on compare match				
				
				TCNT2 = 0;	//reset timer				
				
				if (flag.din_master_start) {
					
					if (++clock.din_ppqn_pulses == 3) { //send 2 DIN clock pulses before bringing RUN/STOP line high: http://www.e-rm.de/data/ERM_DinSync_Report_10_14.pdf
				
						flag.din_master_start = 0;
						PORTD |= (1 << DIN_RUN_STOP); //set DIN RUN/STOP pin
						clock.ppqn_counter = 0;
						flag.next_step = 1;
						TCNT1 = 0; //reset master timer
						flag.half_step = 0; //delayed start requires clearing half_step flag because after start delay it is set, which causes first step LED and first step triggered instrument LEDs to get prematurely cleared
						
						
					}	
				}	
			}			
		}
	}	
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