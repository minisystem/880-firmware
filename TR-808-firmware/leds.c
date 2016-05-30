/*
 *leds.c
 *JR-808 firmware ATMEGA328PB
 *minisystem
 *system79.com
*/

#include <stdio.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "hardware.h"
#include "spi.h"
#include "leds.h"
#include "sequencer.h"


struct led led[NUM_LEDS] = {
	
	{	1<<0	,	1	,	0	,	NO_BLINK},
	{	1<<1	,	1	,	0	,	NO_BLINK},
	{	1<<2	,	1	,	0	,	NO_BLINK},
	{	1<<3	,	1	,	0	,	NO_BLINK},
	{	1<<4	,	1	,	0	,	NO_BLINK},
	{	1<<5	,	1	,	0	,	NO_BLINK},
	{	1<<6	,	1	,	0	,	NO_BLINK},
	{	1<<7	,	1	,	0	,	NO_BLINK},
	{	1<<0	,	0	,	0	,	NO_BLINK},
	{	1<<1	,	0	,	0	,	NO_BLINK},
	{	1<<2	,	0	,	0	,	NO_BLINK},
	{	1<<3	,	0	,	0	,	NO_BLINK},
	{	1<<4	,	0	,	0	,	NO_BLINK},
	{	1<<5	,	0	,	0	,	NO_BLINK},
	{	1<<6	,	0	,	0	,	NO_BLINK},
	{	1<<7	,	0	,	0	,	NO_BLINK},
	{	1<<0	,	2	,	0	,	NO_BLINK},
	{	1<<1	,	2	,	0	,	NO_BLINK},
	{	1<<2	,	2	,	0	,	NO_BLINK},
	{	1<<3	,	2	,	0	,	NO_BLINK},
	{	1<<4	,	2	,	0	,	NO_BLINK},
	{	1<<5	,	2	,	0	,	NO_BLINK},
	{	1<<6	,	2	,	0	,	NO_BLINK},
	{	1<<7	,	2	,	0	,	NO_BLINK},
	{	1<<4	,	3	,	0	,	NO_BLINK},
	{	1<<5	,	3	,	0	,	NO_BLINK},
	{	1<<6	,	3	,	0	,	NO_BLINK},
	{	1<<7	,	3	,	0	,	NO_BLINK},
	{	1<<0	,	4	,	0	,	NO_BLINK},
	{	1<<1	,	4	,	0	,	NO_BLINK},
	{	1<<2	,	4	,	0	,	NO_BLINK},
	{	1<<3	,	4	,	0	,	NO_BLINK},
	{	1<<4	,	4	,	0	,	NO_BLINK},
	{	1<<5	,	4	,	0	,	NO_BLINK},
	{	1<<6	,	4	,	0	,	NO_BLINK},
	{	1<<7	,	4	,	0	,	NO_BLINK},
	{	1<<0	,	5	,	0	,	NO_BLINK},
	{	1<<1	,	5	,	0	,	NO_BLINK},
	{	1<<2	,	5	,	0	,	NO_BLINK},
	{	1<<3	,	5	,	0	,	NO_BLINK},
	{	1<<4	,	5	,	0	,	NO_BLINK},
	{	1<<5	,	5	,	0	,	NO_BLINK},
	{	1<<6	,	5	,	0	,	NO_BLINK},
	{	1<<7	,	5	,	0	,	NO_BLINK},
	{	1<<4	,	6	,	0	,	NO_BLINK},
	{	1<<5	,	6	,	0	,	NO_BLINK},
	{	1<<6	,	6	,	0	,	NO_BLINK},
	{	1<<7	,	6	,	0	,	NO_BLINK},
	{	1<<0	,	7	,	0	,	NO_BLINK},
	{	1<<1	,	7	,	0	,	NO_BLINK},
	{	1<<2	,	7	,	0	,	NO_BLINK},
	{	1<<3	,	7	,	0	,	NO_BLINK},
	{	1<<4	,	7	,	0	,	NO_BLINK},
	{	1<<6	,	7	,	0	,	NO_BLINK},
	{	1<<7	,	7	,	0	,	NO_BLINK}
	
};

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

void turn_off_all_inst_leds(void) { //TODO: make masks constants

	spi_data[2] &= spi_data[2] & 0b00001111; //turn off AC, BD, SD, LT
	spi_data[7] &= spi_data[7] & 0b00100000; // turn off MT, HT, RS, CP, MA, CB, CY
	spi_data[3] &= spi_data[3] & 0b00001111; // turn off LC, MC, HC, CL
	spi_data[6] &= spi_data[6] & 0b11001111; //turn off OH, CH	
	
}
	
void update_step_led_mask(void) { //this blanks step_led_mask and then restore it from pattern data to appropriate step number - use to adjust step led mask when step number is changed.
	
	memset(sequencer.pattern[VAR_A].step_led_mask, 0, sizeof(sequencer.pattern[VAR_A].step_led_mask));
	memset(sequencer.pattern[VAR_B].step_led_mask, 0, sizeof(sequencer.pattern[VAR_B].step_led_mask));
	
	for (int i = 0; i <= sequencer.step_num_first; i++) {
		
		for (int inst = BD; inst <= MA; inst++) {
			//sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst] |= ((sequencer.pattern[sequencer.variation].part[i]) & (1<<sequencer.current_inst)); //this doesn't work. not sure why not???
			if ((sequencer.pattern[VAR_A].part[i] >> inst) & 1) sequencer.pattern[VAR_A].step_led_mask[inst] |= 1<<i;
			if ((sequencer.pattern[VAR_B].part[i] >> inst) & 1) sequencer.pattern[VAR_B].step_led_mask[inst] |= 1<<i;
		}
		
		//also need to rebuild accent led_mask here:
		if ((sequencer.pattern[VAR_A].accent >> i) &1) sequencer.pattern[VAR_A].step_led_mask[AC] |= 1<<i;
		if ((sequencer.pattern[VAR_B].accent >> i) &1) sequencer.pattern[VAR_B].step_led_mask[AC] |= 1<<i;
	}
	//^^^^^^This all seems very inefficient. Would it be easier to directly manipulate spi_data step bytes only for the current instrument? not sure.
	
	
	//for (int inst = BD; inst <= MA; inst++) { //this is all me being two clever by half, or a tenth even.
		//
		//if (sequencer.pattern[sequencer.variation].step_led_mask[inst] != 0) { //check if inst has steps programmed
			////if there are steps, clear the mask for both variation A and B
			//sequencer.pattern[VAR_A].step_led_mask[inst] = 0;
			//sequencer.pattern[VAR_B].step_led_mask[inst] = 0;
			//for (int i = 0; i <= sequencer.step_num_first; i++) { //now rebuild step led mask using pattern data 
				//
				//if ((sequencer.pattern[VAR_A].part[i] >> inst) & 1) sequencer.pattern[VAR_A].step_led_mask[inst] |= 1<<i;
				//if ((sequencer.pattern[VAR_B].part[i] >> inst) & 1) sequencer.pattern[VAR_B].step_led_mask[inst] |= 1<<i;
				//
			//}
			//
		//}
		//
	//if (sequencer.pattern[sequencer.variation].step_led_mask[AC] != 0) {
		//sequencer.pattern[VAR_A].step_led_mask[AC] = 0;
		//sequencer.pattern[VAR_B].step_led_mask[AC] = 0;
		//for (int i = 0; i <= sequencer.step_num_first; i++) { //now rebuild step led mask using pattern data
					//
			//if ((sequencer.pattern[VAR_A].accent >> i) &1) sequencer.pattern[VAR_A].step_led_mask[AC] |= 1<<i;
			//if ((sequencer.pattern[VAR_B].accent >> i) &1) sequencer.pattern[VAR_B].step_led_mask[AC] |= 1<<i;			
		//}
		//
	//}
		//
		//
	//}
	
	
}	

void refresh_step_leds(void) {
	
	spi_data[1] = 0;// (1 << sequencer.current_step) | sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst];
	spi_data[0] = 0;//((1 << sequencer.current_step) >> 8) | (sequencer.pattern[sequencer.variation].step_led_mask[sequencer.current_inst] >> 8);
	
}
