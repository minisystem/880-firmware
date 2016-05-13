/*
 * drums.c
 *JR-808 firmware ATMEGA328PB
 *minisystem
 *system79.com
 */
#include <stdio.h>
#include <avr/io.h>
#include "drums.h"
#include "leds.h"
#include "spi.h"

uint8_t current_drum_hit = 0;
volatile uint8_t trigger_finished = 1;

struct drum_hit drum_hit[16] = {
	
	{0,8, 1<<BD_TRIG,-1, 0, 2, 1<<BD_LED_BIT},
	{1,8, 1<<SD_TRIG,-1, 0, 2, 1<<SD_LED_BIT},
	{2,8, 1<<LT_TRIG, 1<<LT_LC_SW, 1, 2, 1<<LT_LED_BIT},
	{3,8, 1<<MT_TRIG, 1<<MT_MC_SW, 1, 7, 1<<MT_LED_BIT},
	{4,8, 1<<HT_TRIG, 1<<HT_HC_SW, 1, 7, 1<<HT_LED_BIT},
	{5,8, 1<<RS_TRIG, 1<<RS_CL_SW, 0, 7, 1<<RS_LED_BIT},
	{6,8, 1<<CP_TRIG,-1, 0, 7, 1<<CP_LED_BIT},
	{7,6, 1<<CB_TRIG,-1, 0, 7, 1<<CB_LED_BIT},
	{8,6, 1<<CY_TRIG,-1, 0, 7, 1<<CY_LED_BIT},
	{9,6, 1<<OH_TRIG,-1, 0, 6, 1<<OH_LED_BIT},
	{10,6, 1<<CH_TRIG,-1, 0, 6, 1<<CH_LED_BIT},
	{11,8, 1<<LT_TRIG, 1<<LT_LC_SW, 0, 3, 1<<LC_LED_BIT},
	{12,8, 1<<MT_TRIG, 1<<MT_MC_SW, 0, 3, 1<<MC_LED_BIT},
	{13,8, 1<<HT_TRIG, 1<<HT_HC_SW, 0, 3, 1<<HC_LED_BIT},
	{14,8, 1<<RS_TRIG, 1<<RS_CL_SW, 1, 3, 1<<CL_LED_BIT},
	{15,7, 1<<MA_TRIG,-1, 0, 7, 1<<MA_LED_BIT}
};

void trigger_drum(uint8_t note, uint8_t velocity) {
	
		while(trigger_finished == 0);	//need to wait until trigger interrupt is complete before trigger new drum sound
		current_drum_hit = note;

		spi_data[drum_hit[note].spi_byte_num] |= drum_hit[note].trig_bit;
		spi_data[drum_hit[note].spi_led_byte_num] |= drum_hit[note].led_bit;
			
		if (drum_hit[note].switch_bit != -1) {//need to set instrument switch
				
				
			spi_data[3] ^= (-(drum_hit[note].switch_value) ^ spi_data[3]) & drum_hit[note].switch_bit; //this sets switch_value in spi_data byte to switch_value (0 or 1)
				
		}
			
		if (velocity > 64) {
			spi_data[8] |= (1<<ACCENT);
			turn_on(ACCENT_1_LED);
		}
		PORTD |= 1<<TRIG; //move all of this into one tidy function something like play_drum(drum_index) - this will then be applicable to sequencer as well
			
		update_spi();
			
		PORTD &= ~(1<<TRIG);
		
		
		//now need to set up interrupt for roughly 1 ms. 
		//start timer
		TIMSK0 |= (1<<OCIE0A); //enable output compare match A
		TCCR0B |= (1<<CS01) | (1<<CS00); //set to /64 of system clock start timer
		trigger_finished = 0;
}