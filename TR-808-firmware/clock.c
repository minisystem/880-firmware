/*
 * clock.c
 * Open808 firmware ATMEGA328PB
 * minisystem
 * system80.net
 */

#include <avr/io.h>
#include <stdio.h>

#include "clock.h"
#include "hardware.h"
#include "sequencer.h"

//struct clock midi_clock;
//struct clock din_clock;
volatile struct clock clock;


void setup_clock(void) {
	
	
	//TCCR1B = (1<<CS12) | (1<<CS10) | (1<<WGM12);//TIMER1_DIVIDE_1024, clear on output compare match. Should probably reduce? 
	//Setup Timer1, master internal timer
	TCCR1B = (1<<CS12) | (1<<WGM12); //changed to /256 divider
	TIMSK1 = (1<<OCIE1A);
	//Setup Timer3, timer for processing external timing events
	TCCR3B = (1<<CS32) | (1<<CS30); // /1024 divider, 4X slower than master timer1
	
	//set up DIN Sync timer output compare registers: maybe make this a variable for nudging and shifting DIN sync timing?
	OCR2A = OCR2B = 70;//TIMER_OFFSET - 25; //need minimum 5 ms DIN sync clock pulse width	
	clock.ppqn_divider_tick = 0;
}



void update_clock_rate(uint16_t rate) {
	
	OCR1A = rate;
	if (TCNT1 > rate) TCNT1 = rate - 1; //this prevents wrapping. setting TCNT1 = rate would cause immediate interrupt. Is that OK?
	
	//OCR2A = OCR2B = rate >> 9; //output compare happens at 1/2 period of Timer1
}

void process_external_clock_event(void) {
	
			clock.external_rate = TCNT3; //need to handle overflow, probably in Timer3 overflow interrupt
			
			if (flag.slave_start) { //don't update clock if it's the first pulse
					flag.slave_start = 0;
					TCNT1 = 0; //reset timer1 - it should be zeroed after called process_start() but if there's a delay between the MIDI start byte and the first clock pulse ,it could advance by an internal pulse or two.
				} else {
					update_clock_rate(clock.external_rate);
					if (clock.slave_ppqn_ticks != 0) {  //in cases of large tempo speed ups internal clock may not have counted enough internal clock ticks, which means slave_ppqn_ticks will not be reset to 0
						clock.ppqn_counter += ((PPQN_SKIP_VALUE + 1) - clock.slave_ppqn_ticks); //need to make up for these ticks by incrementing master ppqn counter the appropriate number of missed slave ticks
					}
				
			}
			flag.wait_for_master_tick = 0;
			clock.slave_ppqn_ticks = 0;
			TCNT3 = 0; //reset timer3
			process_tick();
			TCCR1B |= (1<<CS12);//turn timer1 on	
	
}