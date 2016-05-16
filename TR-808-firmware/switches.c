/*
 * switches.c
 * JR-808 firmware ATMEGA328PB
 * minisystem
 * system79.com
 */

#include <stdio.h>
#include <avr/io.h>
#include "switches.h"
#include "spi.h"

struct button button[NUM_BUTTONS] = {
	
{	0	,	4	,0,0,0},
{	1	,	4	,0,0,0},
{	2	,	4	,0,0,0},
{	3	,	4	,0,0,0},
{	4	,	4	,0,0,0},
{	5	,	4	,0,0,0},
{	6	,	4	,0,0,0},
{	7	,	4	,0,0,0},
{	0	,	3	,0,0,0},
{	1	,	3	,0,0,0},
{	2	,	3	,0,0,0},
{	3	,	3	,0,0,0},
{	4	,	3	,0,0,0},
{	5	,	3	,0,0,0},
{	6	,	3	,0,0,0},
{	7	,	3	,0,0,0},
{	5	,	2	,0,0,0},
{	6	,	2	,0,0,0},
{	7	,	2	,0,0,0},
{	0	,	1	,0,0,0},
{	1	,	1	,0,0,0},
{	2	,	1	,0,0,0},
{	3	,	1	,0,0,0},
{	4	,	1	,0,0,0},
{	5	,	1	,0,0,0},
{	6	,	1	,0,0,0},
{	7	,	1	,0,0,0},
{	0	,	0	,0,0,0},
{	1	,	0	,0,0,0},
{	2	,	0	,0,0,0},
{	0	,	2	,0,0,0},
{	1	,	2	,0,0,0},
{	2	,	2	,0,0,0},
{	3	,	2	,0,0,0},
{	4	,	2	,0,0,0}	
	
	
	};
	
void parse_switch_data(void) {
	
	for (int i = 0; i < NUM_BUTTONS; i++) {
		
		//button[i].current_state = (switch_states[button[i].spi_byte] & button[i].spi_bit) & 1; //need to fix this
		uint8_t current_state = (spi_current_switch_data[button[i].spi_byte] >> button[i].spi_bit) &1 ;
		button[i].state ^= current_state;
		
	}
	
	
}	