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

//ISR (TIMER3_COMPA_vect) { //led flashing interrupt. Will this be too much overhead to do something simple like flash LEDs?
	
	//turn_off_all_inst_leds();
	//update_inst_leds();
	
//}

//ISR (TIMER1_COMPB_vect) {
	//
	//TIMSK1 &= ~(1 << OCIE1B); //disable output compare B interrupt 
	//PORTD |= (1 << DIN_RUN_STOP); //set DIN RUN/STOP pin
						//
	////clock.ppqn_counter = 0;
	////flag.next_step = 1;
	////now  set up Timer2 to automatically clear PD3 on compare match:
	////TCCR2A &= ~(1 << COM2B0);
	////OCR2A = OCR2B = TIMER_OFFSET -25;
	//
//}

ISR (TIMER1_COMPA_vect) { //output compare match for internal clock
	//midi_send_clock(&midi_device); //much more setup and overhead is required to send MIDI data
	//update_inst_leds();
	
	if (clock.source == INTERNAL) {

		process_tick();
		
		//if (sequencer.sync_mode == DIN_SYNC_MASTER) {
			//if (flag.din_master_start) {
				//PORTD |= (1 << DIN_RUN_STOP); //set DIN RUN/STOP pin
				//flag.din_master_start = 0;
					//
			//}
			//TCCR2A &= ~(1 << COM2B0);
			//
			//TCCR2B |= (1 << FOC2B);// force output compare to set OCR2B (PD3) - normal PORTD functions are overridden, so need to use timer events to set or clear PD3
			////now  set up Timer2 to automatically clear PD3 on compare match:
			//TCCR2A |= (1 << COM2B1) | (1 << COM2B0); //set up OCR2B to set on compare match
			//TCNT2 = 0;	//reset timer
			//return;
		//}
		if (sequencer.START) {
			if (sequencer.sync_mode == MIDI_MASTER) {
				midi_send_clock(&midi_device); //send MIDI clock
				
			} else { //send DIN Sync clock pulse
				//this set up counter-intuitively puts DIN clock out of phase with master timer. Not sure why this is the case, but it works. Really need to dig into this. Not sure what other horrors this will reveal
				//For example, instrument LED timing is not correct on first step (very brief pulse, rather than a proper half step of being lighted)
				//need to have a good long think about this and figure out what the problem is. 
				TCCR2A &= ~(1 << COM2B0);			
				TCCR2B |= (1 << FOC2B);// force output compare to CLEAR OCR2B (PD3) - normal PORTD functions are overridden, so need to use timer events to set or clear PD3
				//now  set up Timer2 to automatically clear PD3 on compare match:	
				TCCR2A |= (1 << COM2B1) | (1 << COM2B0); //set up OCR2B to set on compare match				
				
				TCNT2 = 0;	//reset timer				
				
				if (flag.din_master_start) {
					
					if (++clock.din_ppqn_pulses == 3) { //send 2 DIN clock pulses before bringing RUN/STOP line high: http://www.e-rm.de/data/ERM_DinSync_Report_10_14.pdf
				
						flag.din_master_start = 0;
						//OK - how about setting up Timer1 COMPB interrupt here to delay DIN Sync Start line. This wont' affect phase of DIN Sync clock
						//OCR1B = OCR1A >> 1;
						//TIFR1 |= (1 << OCF1B); //this is already set, triggering output compare interrupt immediately. Not sure why. Setting it to 1 here clears it and allows interrupt to execute with expected timing. Thorough read of datasheet should explain this quirk.
						//if ((TIFR1 >> OCF1B) & 1) PORTD |= (1 << DIN_RUN_STOP); //set DIN RUN/STOP pin
						//TIMSK1 |= (1<<OCIE1B);
						
						
						//now need to add 9 ms delay before next clock pulse
						//TCCR2A |= (1 << COM2B1) | (1 << COM2B0); //set up OCR2B to set on compare match
						//TCCR2B |= (1 << FOC2B);// force output compare
						
						//TIMSK2 |= (1<<OCIE2B); //activate output compare A interrupt
						//
						//OCR2A = OCR2B = 70;
						PORTD |= (1 << DIN_RUN_STOP); //set DIN RUN/STOP pin
						clock.ppqn_counter = 0;
						flag.next_step = 1;
						TCNT1 = 0; //reset master timer
						
					}// else {
						//TCCR2B |= (1 << FOC2B);// force output compare to set OCR2B (PD3) - normal PORTD functions are overridden, so need to use timer events to set or clear PD3
						////now  set up Timer2 to automatically clear PD3 on compare match:
						//TCCR2A &= ~(1 << COM2B0);						
					//}			
				} else {
			
					
				}

	
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