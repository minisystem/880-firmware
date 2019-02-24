/*
 * interrupts.c
 * Open808 firmware ATMEGA328PB
 * minisystem
 * system80.net
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "drums.h"
#include "leds.h"
#include "spi.h"
#include "clock.h"
#include "sequencer.h"
#include "xnormidi-develop/midi.h"
#include "xnormidi-develop/midi_device.h"
#include "xnormidi-develop/bytequeue/bytequeue.h"
#include "midi.h"

ISR (INT1_vect) { //handler for DIN Sync clock pulse in slave mode
	
	//clock.ppqn_counter+= PPQN_SKIP_VALUE; //add skip value to ppqn counter for 24 ppqn 
	//process_tick();
	//clock.previous_external_rate = clock.external_rate;
	//clock.external_rate = TCNT3;
	//TCNT3 = 0; //reset timer3
	//update_clock_rate(clock.external_rate);
	//clock.ppqn_counter = 0; //reset ppqn counter
	
		
	if (flag.slave_start) {
							//
		if (++clock.din_ppqn_pulses == 1) { //DIN Master devices lag their first steps after DIN START by a clock pulse or two. This adds a start delay when in DIN_SYNC_SLAVE mode
			
			process_external_clock_event();					//
			//flag.slave_start = 0;		
			//clock.ppqn_counter = 0;
			////flag.next_step = 1;		
			////flag.half_step = 0; //delayed start requires clearing half_step flag because after start delay it is set, which causes first step LED and first step triggered instrument LEDs to get prematurely cleared
								//
								//
		}
	//
		//
	} else {
		
		process_external_clock_event();
	}
	
}

ISR (INT0_vect) { //external SYNC IN. By default this is for 2 ppqn, Volca style
	
	//clock.ppqn_counter+= PPQN_SKIP_VALUE;
	//clock.ppqn_counter = clock.divider - 1; //need to finesse this and maybe get it to work with master clock? This is effectively 1 ppqn, so need to get it to work accordingly? In fact, this could be customizable - 1, 2, 4, 6, ... 24, ... 48, ppqn
	//process_tick(); 
	process_external_sync_pulse();
	//process_external_clock_event();
	PINC |= (1<<SYNC_LED_R);
	clock.sync_led_mask++; //^= 1 << clock.sync_led_mask;
}

ISR (PCINT2_vect) { //handler for DIN Sync run/stop in slave mode
	
	//toggle(IF_VAR_B_LED);
	
	if ((PIND >> DIN_RUN_STOP) & 1) { 
		
		sequencer.START = 1;
		process_start();

		
	} else {
		
		flag.din_stop = 1;
		//process_stop(); //does calling stop function from within interrupt cause a problem? memory read could screw something up 
		//maybe need to set a flag here instead?
	}
	
}

ISR (TIMER0_COMPA_vect) { //at the moment this timer is doing double duty as 1MS trigger off for incoming MIDI note drum hits and also 15MS trigger off for triggers 1 and 2 while sequencer is running. At the moment, they are incompatible functions
	
	TCCR0B = 0; //turn off timer
	TIMSK0 &= ~(1<<OCIE0A); //turn off output compare 
	TRIGGER_OUT &= TRIGGER_OFF;
	flag.trig_finished = 1;
	//eventually need to turn all triggers off here for 15 ms trigger width for trigger expander module, unless expander module just uses rising edge maybe?
	//toggle(ACCENT_1_LED);
	//if (sequencer.mode == MANUAL_PLAY && !sequencer.START) turn_off()
	
	if (sequencer.clock_mode == MIDI_SLAVE) { //probably need to come up with a MIDI note off flag that is checked here 
		spi_data[drum_hit[current_drum_hit].spi_byte_num] &= ~(drum_hit[current_drum_hit].trig_bit);
		turn_off(ACCENT_1_LED);
		spi_data[LATCH_8] &= ~(1<<ACCENT);
		turn_off(drum_hit[current_drum_hit].led_index);

		//flag.trig_finished = 1;
	}
	
}

ISR (TIMER2_COMPA_vect) {
	
	flag.blink = 1;
}

ISR (TIMER3_COMPA_vect) {
  
  
}

ISR (TIMER4_COMPA_vect) {
  
  
}


ISR (TIMER1_COMPA_vect) { //output compare match for internal clock
		//spi_read_write();
		//update_spi();
	//OCR1A = clock.rate; 
	//if (clock.source == INTERNAL) {
		if (flag.slave_start) return; //if there's a lag between start and incoming sync pulse we don't want to process_tick - kind of a pain to call this every time. Could be implemented somewhere else - turn compare interrupt off maybe?
		process_tick();	

		if ((clock.ppqn_divider_tick++ == PPQN_DIVIDER)) { //PPQN_DIVIDER used to convert 96 PPQN internal clock to 24 PPQN MIDI standard
			//TRIGGER_OUT |= (1<<TRIGGER_OUT_2);
			
			clock.ppqn_divider_tick = 0;
			if (sequencer.clock_mode == MIDI_MASTER) { 
				midi_send_clock(&midi_device); //send MIDI clock
				//if (sequencer.START) PINC |= (1<<SYNC_LED_Y); //only blink sync LED when sequencer is running
			} else if (sequencer.clock_mode == DIN_SYNC_MASTER) { //send DIN Sync clock pulse
				//this set up counter-intuitively puts DIN clock out of phase with master timer. Not sure why this is the case, but it works. Really need to dig into this. Not sure what other horrors this will reveal
				//need to have a good long think about this and figure out what the problem is. 
				TCCR2A &= ~(1 << COM2B0);			
				TCCR2B |= (1 << FOC2B);// force output compare to CLEAR OCR2B (PD3) - normal PORTD functions are overridden, so need to use timer events to set or clear PD3
				//now  set up Timer2 to automatically clear PD3 on compare match:	
				TCCR2A |= (1 << COM2B1) | (1 << COM2B0); //set up OCR2B to set on compare match				
				
				TCNT2 = 0;	//reset timer				
				//if (sequencer.START) PINC |= (1<<SYNC_LED_R); //toggle SYNC LED
				if (flag.din_start) {
					
					if (++clock.din_ppqn_pulses == 1) { //send 2 DIN clock pulses before bringing RUN/STOP line high: http://www.e-rm.de/data/ERM_DinSync_Report_10_14.pdf
						
						flag.din_start = 0;
						PORTD |= (1 << DIN_RUN_STOP); //set DIN RUN/STOP pin 
						//clock.ppqn_counter = 0;
						//flag.next_step = 1;
						//TCNT1 = 0; //reset master timer, doesn't seem necessary
						//flag.half_step = 0; //delayed start requires clearing half_step flag because after start delay it is set, which causes first step LED and first step triggered instrument LEDs to get prematurely cleared
						clock.ppqn_divider_tick = 0; //need to think about what's happening here - does it need to be processed ad ppqn_divider_tick = ppqn_divider -1 when starting as slave?
						//need to prime sequencer so that first step (downbeat) occurs on first incoming clock pulse, hence -1 for current_step and divider
						sequencer.current_step = -1;
						clock.ppqn_counter = clock.divider - 1;
						sequencer.primed = 1;
						
					}
				}	
			}			
		}
	//}	
	//TRIGGER_OUT &= ~(1<<TRIGGER_OUT_2);
	
}

ISR (USART0_RX_vect) { // USART receive interrupt
	//PORTB ^= (1<<ARP_SYNC_LED); //toggle arp VCO_SYNC_LATCH_BIT LED
	uint8_t inByte = UDR0;
	midi_device_input(&midi_device, 1, &inByte);
	//calling a function in an interrupt is inefficient according to AVR C guidelines
	// so this function should maybe be inlined in main loop if inByte is made volatile
	//***HOWEVER***, xnor-midi example code has this function being called from USART_RX_vect ISR
}


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