/*
 * clock.h
 * JR-808 firmware ATMEGA328PB
 * minisystem
 * system79.com
 */


#ifndef CLOCK_H
#define CLOCK_H


#define TIMER_OFFSET 140 //~30 - 250 BPM
//#define _9_MS	140 //output compare count for 9 ms delay for Timer2 with /1024 divider

enum clock_source {
	INTERNAL,
	EXTERNAL
	};

struct clock {
	
	uint8_t divider:4;
	uint8_t ppqn_counter;
	uint8_t din_ppqn_pulses:3; //counter for din sync clock pulses sent before DIN master start event
	uint8_t beat_counter:2; //counts quarter notes, may need to change it and handle it more when dealing with different time signatures
	uint16_t rate; //output compare value for clock timer
	uint16_t previous_rate;
	enum clock_source source;
	
};

//maybe only need 1 clock? Not sure yet
//extern struct clock midi_clock;
//extern struct clock din_clock; 
volatile struct clock clock;


void setup_clock(void);
void update_clock_rate(uint16_t rate);




#endif 