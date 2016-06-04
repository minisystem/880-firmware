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

struct clock midi_clock;
struct clock din_clock;
struct clock internal_clock;


void setup_internal_clock(void) {
	
	
	TCCR1B = (1<<CS12) | (1<<CS10) | (1<<WGM12);//TIMER1_DIVIDE_1024, clear on output compare match. Should probably reduce 
	TIMSK1 = (1<<OCIE1A);
	
	
	
}



void update_clock_rate(uint16_t rate) {
	
	OCR1A = rate;
	if (TCNT1 > rate) TCNT1 = rate - 1; //this prevents wrapping. setting TCNT1 = rate would cause immediate interrupt. Is that OK?
	
	
}