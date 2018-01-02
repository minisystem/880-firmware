/*
 *leds.c
 *Open808 firmware ATMEGA328PB
 *minisystem
 *system80.net
*/

#include <stdio.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "hardware.h"
#include "spi.h"
#include "leds.h"
#include "sequencer.h"


struct led led[NUM_LEDS] = { //any way to put this in PROGMEM?
	
	{	1<<0	,	1	,	0	,	0},
	{	1<<1	,	1	,	0	,	0},
	{	1<<2	,	1	,	0	,	0},
	{	1<<3	,	1	,	0	,	0},
	{	1<<4	,	1	,	0	,	0},
	{	1<<5	,	1	,	0	,	0},
	{	1<<6	,	1	,	0	,	0},
	{	1<<7	,	1	,	0	,	0},
	{	1<<0	,	0	,	0	,	0},
	{	1<<1	,	0	,	0	,	0},
	{	1<<2	,	0	,	0	,	0},
	{	1<<3	,	0	,	0	,	0},
	{	1<<4	,	0	,	0	,	0},
	{	1<<5	,	0	,	0	,	0},
	{	1<<6	,	0	,	0	,	0},
	{	1<<7	,	0	,	0	,	0},
	{	1<<0	,	2	,	0	,	0},
	{	1<<1	,	2	,	0	,	0},
	{	1<<2	,	2	,	0	,	0},
	{	1<<3	,	2	,	0	,	0},
	{	1<<4	,	2	,	0	,	0},
	{	1<<5	,	2	,	0	,	0},
	{	1<<6	,	2	,	0	,	0},
	{	1<<7	,	2	,	0	,	0},
	{	1<<4	,	3	,	0	,	0},
	{	1<<5	,	3	,	0	,	0},
	{	1<<6	,	3	,	0	,	0},
	{	1<<7	,	3	,	0	,	0},
	{	1<<0	,	4	,	0	,	0},
	{	1<<1	,	4	,	0	,	0},
	{	1<<2	,	4	,	0	,	0},
	{	1<<3	,	4	,	0	,	0},
	{	1<<4	,	4	,	0	,	0},
	{	1<<5	,	4	,	0	,	0},
	{	1<<6	,	4	,	0	,	0},
	{	1<<7	,	4	,	0	,	0},
	{	1<<0	,	5	,	0	,	0},
	{	1<<1	,	5	,	0	,	0},
	{	1<<2	,	5	,	0	,	0},
	{	1<<3	,	5	,	0	,	0},
	{	1<<4	,	5	,	0	,	0},
	{	1<<5	,	5	,	0	,	0},
	{	1<<6	,	5	,	0	,	0},
	{	1<<7	,	5	,	0	,	0},
	{	1<<4	,	6	,	0	,	0},
	{	1<<5	,	6	,	0	,	0},
	{	1<<6	,	6	,	0	,	0},
	{	1<<7	,	6	,	0	,	0},
	{	1<<0	,	7	,	0	,	0},
	{	1<<1	,	7	,	0	,	0},
	{	1<<2	,	7	,	0	,	0},
	{	1<<3	,	7	,	0	,	0},
	{	1<<4	,	7	,	0	,	0},
	{	1<<6	,	7	,	0	,	0},
	{	1<<7	,	7	,	0	,	0}
	
};

uint8_t track_led[NUM_TRACKS] = {ACCENT_1_LED, BD_2_LED, SD_3_LED, LT_4_LED, MT_5_LED, HT_6_LED, RS_7_LED, CP_8_LED, CB_9_LED, CY_10_LED, OH_11_LED, CH_12_LED};

void turn_on(uint8_t led_index) {
	
	spi_data[led[led_index].spi_byte] |= led[led_index].spi_bit;
	led[led_index].state = 1;

}

void turn_off(uint8_t led_index) {
	
	spi_data[led[led_index].spi_byte] &= ~(led[led_index].spi_bit);
	led[led_index].state = 0;
}

void flash_once(uint8_t led_index) { //need to think about how to flash LED for fixed interval - say ~5ms - work into main spi updating interrupt somehow. or maybe just flash asynchronously?
	
	//or what about a flash flag and if LED is ON and flash flag is ON then turn it off?
	
}

void toggle(uint8_t led_index) { //careful with this - state is not preserved when doing entire SPI byte manipulation (see  turn_off_all_inst_leds() for example)

	//led[led_index].state ^= led[led_index].state;
	//
	//spi_data[led[led_index].spi_byte] ^= (-led[led_index].state^spi_data[led[led_index].spi_byte]) & led[led_index].spi_bit;
		
	if (led[led_index].state) {
		
		turn_off(led_index);
		
	} else { 
		
		turn_on(led_index);
	
	}
}

void turn_off_all_inst_leds(void) {
	
	//This is quick and dirty fast way to turn off all LEDs, but doesn't preserve their states for toggling 
	//TODO: make masks constants
	spi_data[LATCH_2] &= spi_data[LATCH_2] & 0b00001111; //turn off AC, BD, SD, LT
	spi_data[LATCH_7] &= spi_data[LATCH_7] & 0b00100000; // turn off MT, HT, RS, CP, MA, CB, CY
	spi_data[LATCH_3] &= spi_data[LATCH_3] & 0b00001111; // turn off LC, MC, HC, CL
	spi_data[LATCH_6] &= spi_data[LATCH_6] & 0b11001111; //turn off OH, CH	
	
	//for (int i = BD; i <= MA; i++) {
		//
		//if (!drum_hit[i].muted) turn_off(drum_hit[i].led_index);
		//
	//}
	
}

void show_mutes(void) {
	for (int i = BD; i <= MA; i++) {
				 
				 
		if (drum_hit[i].muted) {
					 
			turn_on(drum_hit[i].led_index);
					 
			} else {
					 
			turn_off(drum_hit[i].led_index);
		}
	}
	
}



void update_inst_leds(void) {
	
	
  
	switch (sequencer.mode) {
 
	 case PATTERN_CLEAR: case FIRST_PART: case SECOND_PART:
 
		 if (sequencer.SHIFT) {
   
			if (sequencer.ALT) { //show trigger assignment
     
				 turn_off_all_inst_leds();
				 (sequencer.intro_fill_var == 0) ? turn_on(drum_hit[sequencer.trigger_1].led_index) : turn_on(drum_hit[sequencer.trigger_2].led_index);
     
			} else { //show muted instruments
				show_mutes();
		
		  }
    
		} else {
     
			 turn_on(drum_hit[sequencer.current_inst].led_index);
		  }
     
	 break;
 
	 case MANUAL_PLAY:
		if (sequencer.SHIFT) {
		
			show_mutes();
		
		} else {
			
			turn_on(drum_hit[sequencer.current_inst].led_index);
		}
 
	 break;
 
 
	 case PLAY_RHYTHM:
		if (sequencer.SHIFT) {
		
			show_mutes();
		} else {
			
			turn_on(track_led[sequencer.current_rhythm_track]);
		}
		
	 break;
 
	 case COMPOSE_RHYTHM:
		if (sequencer.SHIFT) {
		
			show_mutes();
		} else {
			
		turn_on(track_led[sequencer.current_rhythm_track]);
		}
		
	 break;
  
	} 
}
	
//if (sequencer.SHIFT) {
	//
	//if (sequencer.ALT) { //show trigger assignment
		//
		//turn_off_all_inst_leds();
		//(sequencer.intro_fill_var == 0) ? turn_on(drum_hit[sequencer.trigger_1].led_index) : turn_on(drum_hit[sequencer.trigger_2].led_index);
		//
	//} else { //show muted instruments
	//
		//for (int i = BD; i <= MA; i++) {
			//
			//
			//if (drum_hit[i].muted) {
				//
				//turn_on(drum_hit[i].led_index);
				//
			//} else {
				//
				//turn_off(drum_hit[i].led_index);
			//}
			//
		//}
	//}
	//
//} else if (sequencer.mode == MANUAL_PLAY) {
	//
	//
	//
//} else {
	//
	//turn_on(drum_hit[sequencer.current_inst].led_index);	
//}
	
//}

void update_inst_led_mask(void) {
	
	sequencer.led_mask = 0;
	
	if (sequencer.current_inst == AC) { //annoying exception for accent
		
		for (int i = 0; i <= sequencer.step_num[sequencer.part_editing]; i++) {
			if ((sequencer.pattern[sequencer.variation].accent[sequencer.part_editing] >> i) &1) sequencer.led_mask |= 1<<i;
		}
		
	} else {
	
		for (int i = 0; i <= sequencer.step_num[sequencer.part_editing]; i++) {
		
			if ((sequencer.pattern[sequencer.variation].part[sequencer.part_editing][i] >> sequencer.current_inst) & 1) sequencer.led_mask |= 1 << i;
		}
	
		
	}
	
}
	
void update_step_led_mask(void) { //this blanks step_led_mask and then restore it from pattern data to appropriate step number - use to adjust step led mask when step number is changed.
	
	//memset(sequencer.pattern[VAR_A].step_led_mask, 0, sizeof(sequencer.pattern[VAR_A].step_led_mask));
	//memset(sequencer.pattern[VAR_B].step_led_mask, 0, sizeof(sequencer.pattern[VAR_B].step_led_mask));
	
	//memset(sequencer.step_led_mask[VAR_A], 0, sizeof(sequencer.step_led_mask[VAR_A]));
	//memset(sequencer.step_led_mask[VAR_B], 0, sizeof(sequencer.step_led_mask[VAR_B]));
	
	
	
	//memset(sequencer.step_led_mask, 0, sizeof(sequencer.step_led_mask[0][0])*2*17); //2*17 - use constant here that actually means something
	//TRIGGER_OUT |= (1<<TRIGGER_OUT_2);
	
	for (int i = 0; i <= sequencer.step_num[sequencer.part_editing]; i++) { //this loop takes 1.9 ms to execute!!!!
		
		for (int inst = BD; inst <= MA; inst++) {
			//sequencer.step_led_mask[VAR_A][inst] |= (((sequencer.pattern[VAR_A].part[sequencer.part_editing][i]) & (1<<i))); //this doesn't work. not sure why not???
			//sequencer.step_led_mask[VAR_B][inst] |= (((sequencer.pattern[VAR_B].part[sequencer.part_editing][i]) & (1<<i)));
			//sequencer.step_led_mask[VAR_A][inst] |= (((sequencer.pattern[VAR_A].part[sequencer.part_editing][i] >> inst) & 1) <<i);
			//sequencer.step_led_mask[VAR_B][inst] |= (((sequencer.pattern[VAR_B].part[sequencer.part_editing][i] >> inst) & 1) <<i);
			//if ((sequencer.pattern[VAR_A].part[sequencer.part_editing][i] >> inst) & 1) sequencer.step_led_mask[VAR_A][inst] |= 1<<i;
			//if ((sequencer.pattern[VAR_B].part[sequencer.part_editing][i] >> inst) & 1) sequencer.step_led_mask[VAR_B][inst] |= 1<<i;
		}
		
		//also need to rebuild accent led_mask here:
		//if ((sequencer.pattern[VAR_A].accent[sequencer.part_editing] >> i) &1) sequencer.step_led_mask[VAR_A][AC] |= 1<<i;
		//if ((sequencer.pattern[VAR_B].accent[sequencer.part_editing] >> i) &1) sequencer.step_led_mask[VAR_B][AC] |= 1<<i;
	}
	
	/*
	for (int inst = BD; inst <= MA; inst++) {
		
		for (int i = 0; i <= sequencer.step_num[sequencer.part_editing]; i++) {
			sequencer.step_led_mask[VAR_A][inst] |= (((sequencer.pattern[VAR_A].part[sequencer.part_editing][i]) & (1<<i))); //this doesn't work. not sure why not???
			sequencer.step_led_mask[VAR_B][inst] |= (((sequencer.pattern[VAR_B].part[sequencer.part_editing][i]) & (1<<i)));
			
		}
		
		
	}
	*/
	//for (int inst = BD; inst <= MA; inst++) {
		
		//sequencer.step_led_mask[VAR_A][inst] = sequencer.pattern[VAR_A].part[inst]
	//}
	//TRIGGER_OUT &= ~(1<<TRIGGER_OUT_2);
	//^^^^^^This all seems very inefficient. Would it be easier to directly manipulate spi_data step bytes only for the current instrument? not sure.
	
	
}	 

void update_prescale_leds(void) {
	
	spi_data[LATCH_5] &= PRE_SCALE_LED_MASK; //clear pre-scale LED bits
	if (sequencer.TAP_HELD && ((sequencer.mode == COMPOSE_RHYTHM) || (sequencer.mode == PLAY_RHYTHM))) {
		
		if (sequencer.track_measure < 16) {
			
			spi_data[LATCH_5] |= PRE_SCALE_BIT_1;
			
		} else if (sequencer.track_measure < 32) {
			spi_data[LATCH_5] |= PRE_SCALE_BIT_2;
			
		} else if (sequencer.track_measure < 48) {
			spi_data[LATCH_5] |= PRE_SCALE_BIT_3;
		} else {
			
			spi_data[LATCH_5] |= PRE_SCALE_BIT_4;
		}
		
		
	} else {
		spi_data[LATCH_5] |= (1<< (sequencer.pre_scale +2)); //need 2 bit offset on latch 5 (pre-scale leds are bit 2-5)	
	
	}
}

void refresh_step_leds(void) {
	
	spi_data[LATCH_1] = 0;// (1 << sequencer.current_step) | sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst];
	spi_data[LATCH_0] = 0;//((1 << sequencer.current_step) >> 8) | (sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst] >> 8);
	
}

void set_up_led_timer(void) { 

	TCCR3B = (1<<CS32) | (1<<CS30) | (1<<WGM12);//TIMER3_DIVIDE_1024, clear on output compare match.
	TIMSK3 = (1<<OCIE3A);
	OCR3A = 1500; //5 Hz flash
	
}