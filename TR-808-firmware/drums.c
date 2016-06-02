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
#include "switches.h"

uint8_t current_drum_hit = 0;
volatile uint8_t trigger_finished = 1;

struct drum_hit drum_hit[17] = {
	
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
	{15,7, 1<<MA_TRIG,255, 0, MA_LED},
	{16,8, 1<<ACCENT, 255, 0, ACCENT_1_LED} //this last accent element is a bit of a hack - not currently used to access accent, but useful to turn on accent LED when accent it triggered by step sequencer (see interrupt.c)
			
};

void trigger_drum(uint8_t note, uint8_t velocity) { //this needs rework to be compatible with synchronized spi updating
	
		//while(trigger_finished == 0);	//need to wait until trigger interrupt is complete before triggering new drum sound, otherwise new hits come and and 'overwrite' old hits, preventing their triggers from finishing
		//could implement a trigger queue instead of waiting but this is really more of a concern from simultaneous drum hits coming from MIDI or live play. Sequencer triggers won't have this problem unless 
		//individual accents are implemented for sequencer
		current_drum_hit = note;

		spi_data[drum_hit[note].spi_byte_num] |= drum_hit[note].trig_bit;
		//toggle(drum_hit[note].led_index);
		//spi_data[drum_hit[note].spi_led_byte_num] |= drum_hit[note].led_bit;
			
		if (drum_hit[note].switch_bit != 255) {//need to set instrument switch
				
			toggle(ACCENT_1_LED); //TODO: make this optional. It's a bit of a distracting light show, so need to be able to let user turn it off	
			spi_data[3] ^= (-(drum_hit[note].switch_value) ^ spi_data[3]) & drum_hit[note].switch_bit; //this sets switch_value in spi_data byte to switch_value (0 or 1)
					
		}
			
		if (velocity > 64) {
			spi_data[8] |= (1<<ACCENT);
			//toggle(ACCENT_1_LED);
		}
		PORTD |= 1<<TRIG; //move all of this into one tidy function something like play_drum(drum_index) - this will then be applicable to sequencer as well
			
		update_spi(); //can't do this here, not synchronized. duh.
			
		PORTD &= ~(1<<TRIG);
		
		
		//now need to set up interrupt for roughly 1 ms. 
		//start timer
		TIMSK0 |= (1<<OCIE0A); //enable output compare match A
		TCCR0B |= (1<<CS01) | (1<<CS00); //set to /64 of system clock start timer
		trigger_finished = 0;
}

void clear_all_trigs(void) {

	spi_data[8] = 0;
	spi_data[6] &= 0b11110000; //make these masks constants
	spi_data[7] &= 0b11011111;	
	
}

void trigger_step(void) { //trigger all drums on current step

	clear_all_trigs();
	for (int i = BD; i <= MA; i++) {
		
		if ((sequencer.pattern[sequencer.variation].part[sequencer.part_playing][sequencer.current_step] >> i) &1) {
			turn_on(drum_hit[i].led_index);
			spi_data[drum_hit[i].spi_byte_num] |= drum_hit[i].trig_bit;
			if (drum_hit[i].switch_bit != 255) {//need to set instrument switch
						
				spi_data[3] ^= (-(drum_hit[i].switch_value) ^ spi_data[3]) & drum_hit[i].switch_bit; //this sets switch_value in spi_data byte to switch_value (0 or 1)
						
			}		
		}
	}
}

void live_hits(void) { //use switch case here you twit
	
	if (button[INST_BD_2_SW].state) {
		
		button[INST_BD_2_SW].state ^= button[INST_BD_2_SW].state;
		trigger_drum(BD, 0);
	}
	
	if (button[INST_SD_3_SW].state) {
		
		button[INST_SD_3_SW].state ^= button[INST_SD_3_SW].state;
		trigger_drum(SD,0);
	}
	
	if (button[INST_LT_4_SW].state) {
		
		button[INST_LT_4_SW].state ^= button[INST_LT_4_SW].state;
		trigger_drum(LT, 0);
	}
	
	if (button[INST_MT_5_SW].state) {
		
		button[INST_MT_5_SW].state ^= button[INST_MT_5_SW].state;
		trigger_drum(MT,0);
	}
	
	if (button[INST_HT_6_SW].state) {
		
		button[INST_HT_6_SW].state ^= button[INST_HT_6_SW].state;
		trigger_drum(HT, 0);
	}
	
	if (button[INST_RS_7_SW].state) {
		
		button[INST_RS_7_SW].state ^= button[INST_RS_7_SW].state;
		trigger_drum(RS,0);
	}
	
	if (button[INST_CP_8_SW].state) {
		
		button[INST_CP_8_SW].state ^= button[INST_CP_8_SW].state;
		trigger_drum(CP, 0);
	}
	
	if (button[INST_CB_9_SW].state) {
		
		button[INST_CB_9_SW].state ^= button[INST_CB_9_SW].state;
		trigger_drum(CB,0);
	}
	if (button[INST_CY_10_SW].state) {
		
		button[INST_CY_10_SW].state ^= button[INST_CY_10_SW].state;
		trigger_drum(CY,0);
	}
	
	if (button[INST_OH_11_SW].state) {
		
		button[INST_OH_11_SW].state ^= button[INST_OH_11_SW].state;
		trigger_drum(OH, 0);
	}
	
	if (button[INST_CH_12_SW].state) {
		
		button[INST_CH_12_SW].state ^= button[INST_CH_12_SW].state;
		trigger_drum(CH,0);
	}
	
	
}
