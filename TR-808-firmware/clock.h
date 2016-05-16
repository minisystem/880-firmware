/*
 * clock.h
 * JR-808 firmware ATMEGA328PB
 * minisystem
 * system79.com
 */


#ifndef CLOCK_H
#define CLOCK_H

struct clock {
	
	uint8_t divider;
	uint8_t ppqn_counter;
	uint16_t rate; //output compare value for clock timer
	uint16_t previous_rate;
	
	
};

//maybe only need 1 clock? Not sure yet
extern struct clock midi_clock;
extern struct clock din_clock; 
extern struct clock internal_clock;


void setup_internal_clock(void);
void update_clock_rate(uint16_t rate);




#endif 