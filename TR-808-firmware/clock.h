/*
 * clock.h
 * Open808 firmware ATMEGA328PB
 * minisystem
 * system80.net
 */


#ifndef CLOCK_H
#define CLOCK_H


#define TIMER_OFFSET 140 //~30 - 250 BPM
//#define _9_MS	140 //output compare count for 9 ms delay for Timer2 with /1024 divider

#define TIMER0_1_MS 16 //output compare count for 1 ms Timer0 with /1024 divider
#define TIMER0_15_MS 235 //output compare count for 15 ms Timer0 with /1024 divider
#define TIMER0_OUTPUT_COMPARE OCR0A

#define PPQN_DIVIDER 3 //master divider for converting internal 96PPQN to 24PPQN for MIDI and DIN SYNC

#define NUM_SYNC_COUNTS 6
#define PPQN_24_TICK_COUNT 4//master ppqn skip value for converting incoming MIDI or DIN SYNC clock to internal 96PPQN. 3 for 24 PPQN, 1 for 48 PPQN, 0 for 96PPQN
#define PPQN_12_TICK_COUNT 8
#define PPQN_8_TICK_COUNT 12
#define PPQN_6_TICK_COUNT 16
#define PPQN_4_TICK_COUNT 24
#define PPQN_2_TICK_COUNT 48 //master ppqn skip value for converting incoming 2 ppqn (1/8 note pulse) from Korg/Volca clock


enum clock_source {
	INTERNAL,
	EXTERNAL
	};

struct clock {
	
	uint8_t divider;
	uint8_t sync_count;
	uint8_t sync_count_index:3;
	uint8_t ppqn_counter;
	uint8_t ppqn_divider_tick:3; //counter for ppqn divider 
	uint8_t din_ppqn_pulses:3; //counter for din sync clock pulses sent before DIN master start event
	uint8_t slave_ppqn_ticks;
	uint8_t beat_counter:2; //counts quarter notes, may need to change it and handle it more when dealing with different time signatures
	uint16_t rate; //output compare value for clock timer
	//uint16_t previous_rate;
	uint16_t external_rate;
	//uint16_t previous_external_rate;
	uint8_t tick_counter; //counter for blinking LEDs independent of sequencer clock rate
	//uint8_t tick_value; //store tick value when slave is stopped
	uint8_t sync_led_mask:1;
	enum clock_source source;
	
};

//maybe only need 1 clock? Not sure yet
//extern struct clock midi_clock;
//extern struct clock din_clock; 
volatile struct clock clock;


void setup_clock(void);
void update_clock_rate(uint16_t rate);
void process_external_clock_event(void);
void process_external_sync_pulse(void);
void poll_din_reset(void);



#endif 