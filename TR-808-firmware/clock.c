/*
 * clock.c
 * JR-808 firmware ATMEGA328PB
 * minisystem
 * system79.com
 */

#include <avr/io.h>
#include <stdio.h>

#include "clock.h"
#include "hardware.h"

//struct clock midi_clock;
//struct clock din_clock;
volatile struct clock clock;


void setup_clock(void) {
	
	
	TCCR1B = (1<<CS12) | (1<<CS10) | (1<<WGM12);//TIMER1_DIVIDE_1024, clear on output compare match. Should probably reduce? 
	TIMSK1 = (1<<OCIE1A);
	//set up DIN Sync timer output compare registers:
	OCR2A = OCR2B = TIMER_OFFSET - 25; //need minimum 5 ms DIN sync clock pulse width	
}



void update_clock_rate(uint16_t rate) {
	
	OCR1A = rate;
	if (TCNT1 > rate) TCNT1 = rate - 1; //this prevents wrapping. setting TCNT1 = rate would cause immediate interrupt. Is that OK?
	
	//OCR2A = OCR2B = rate >> 9; //output compare happens at 1/2 period of Timer1
}