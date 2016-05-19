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
#include "sequencer.h"

uint8_t current_drum_hit = 0;
volatile uint8_t trigger_finished = 1;

struct drum_hit drum_hit[16] = {
	
	{0,8, 1<<BD_TRIG,255, 0, BD_2_LED},
	{1,8, 1<<SD_TRIG,255, 0, SD_3_LED},
	{2,8, 1<<LT_TRIG, 1<<LT_LC_SW, 1, LT_4_LED},
	{3,8, 1<<MT_TRIG, 1<<MT_MC_SW, 1, MT_5_LED},
	{4,8, 1<<HT_TRIG, 1<<HT_HC_SW, 1, HT_6_LED},
	{5,8, 1<<RS_TRIG, 1<<RS_CL_SW, 0, RS_7_LED},
	{6,8, 1<<CP_TRIG,255, 0, CP_8_LED},
	{7,6, 1<<CB_TRIG,255, 0, CB_9_LED},
	{8,6, 1<<CY_TRIG,255, 0, CY_10_LED},
	{9,6, 1<<OH_TRIG,255, 0, OH_11_LED},
	{10,6, 1<<CH_TRIG,255, 0, CH_12_LED},
	{11,8, 1<<LT_TRIG, 1<<LT_LC_SW, 0, LC_LED},
	{12,8, 1<<MT_TRIG, 1<<MT_MC_SW, 0, MC_LED},
	{13,8, 1<<HT_TRIG, 1<<HT_HC_SW, 0, HC_LED},
	{14,8, 1<<RS_TRIG, 1<<RS_CL_SW, 1, CL_LED},
	{15,7, 1<<MA_TRIG,255, 0, MA_LED}
};

void trigger_drum(uint8_t note, uint8_t velocity) {
	
		while(trigger_finished == 0);	//need to wait until trigger interrupt is complete before triggering new drum sound, otherwise new hits come and and 'overwrite' old hits, preventing their triggers from finishing
		//could implement a trigger queue instead of waiting but this is really more of a concern from simultaneous drum hits coming from MIDI or live play. Sequencer triggers won't have this problem unless 
		//individual accents are implemented for sequencer
		current_drum_hit = note;

		spi_data[drum_hit[note].spi_byte_num] |= drum_hit[note].trig_bit;
		//toggle(drum_hit[note].led_index);
		//spi_data[drum_hit[note].spi_led_byte_num] |= drum_hit[note].led_bit;
			
		if (drum_hit[note].switch_bit != 255) {//need to set instrument switch
				
			toggle(ACCENT_1_LED);	
			spi_data[3] ^= (-(drum_hit[note].switch_value) ^ spi_data[3]) & drum_hit[note].switch_bit; //this sets switch_value in spi_data byte to switch_value (0 or 1)
					
		}
			
		if (velocity > 64) {
			spi_data[8] |= (1<<ACCENT);
			//toggle(ACCENT_1_LED);
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

void trigger_step(void) { //trigger all drums on current step
	
	while(trigger_finished == 0);
	//
	//spi_data[8] = sequencer.current_pattern.first_part[sequencer.current_step] << 1 //left shift by 1 bit because AC is handled separately - may want to eventually integrate accent into drum_hit array
	////spi_data[6]
	//for (int i = BD; i <= MA; i++) {
		//
		//if ((sequencer.current_pattern.first_part[sequencer.current_step] >> i) &1) {
			//
			//trigger_drum(i, 0); //work out global and individual accents later
		//}
	//}
	
	
	//while(trigger_finished ==0);
	spi_data[8] = sequencer.current_pattern.first_part[sequencer.current_step] << 1;
	//PORTD |= 1<<TRIG;
	//update_spi();
	//PORTD &= ~(1<<TRIG);
	//now need to set up interrupt for roughly 1 ms.
	//start timer
	//TIMSK0 |= (1<<OCIE0A); //enable output compare match A
	//TCCR0B |= (1<<CS01) | (1<<CS00); //set to /64 of system clock start timer
	//trigger_finished = 0;
	
}