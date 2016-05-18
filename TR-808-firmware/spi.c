/*
 *spi.c
 *JR-808 firmware ATMEGA328PB
 *minisystem
 *system79.com 
*/

#include <stdio.h>
#include <avr/io.h>
#include "spi.h"
#include "hardware.h"
#include "sequencer.h"
#include "switches.h"

uint8_t spi_data[9] = {0};
uint8_t spi_current_switch_data[5] = {0};
uint8_t spi_previous_switch_data[5] = {0};
uint8_t switch_states[5] = {0};
	
uint8_t spi_shift_byte(uint8_t byte) { //shifts out byte for LED data and simultaneously reads switch data
	
	SPDR1 = byte;
	while (!(SPSR1 & (1<<SPIF1)));
	return SPDR1;
	
}	

void update_spi(void) { //updates LEDs and triggers, doesn't read data back
	
		spi_shift_byte(spi_data[0]);
		spi_shift_byte(spi_data[1]);
		spi_shift_byte(spi_data[2]);
		spi_shift_byte(spi_data[3]);
		spi_shift_byte(spi_data[4]);
		spi_shift_byte(spi_data[5]);
		spi_shift_byte(spi_data[6]);
		spi_shift_byte(spi_data[7]);
		spi_shift_byte(spi_data[8]);
		
		PORTC &= ~(1<<SPI_LED_LATCH);
		PORTC |= (1<<SPI_LED_LATCH);
	
	
};

void read_switches(void) { //reads switch data
	
	PORTC |= (1<<SPI_SW_LATCH); //latch switch data
	
	spi_current_switch_data[0] = spi_shift_byte(0x00); //not toggling LED latches so doesn't matter what we send out
	spi_current_switch_data[1] = spi_shift_byte(0x00);
	spi_current_switch_data[2] = spi_shift_byte(0x00);
	spi_current_switch_data[3] = spi_shift_byte(0x00);
	spi_current_switch_data[4] = spi_shift_byte(0x00);
	
	PORTC &= ~(1<<SPI_SW_LATCH);
	
	sequencer.SHIFT = ((spi_current_switch_data[0] >> SHIFT_BIT) & 1);
	//debounce
	spi_current_switch_data[0] ^= spi_previous_switch_data[0];
	spi_previous_switch_data[0] ^= spi_current_switch_data[0];
	spi_current_switch_data[0] &= spi_previous_switch_data[0];
	
	spi_current_switch_data[1] ^= spi_previous_switch_data[1];
	spi_previous_switch_data[1] ^= spi_current_switch_data[1];
	spi_current_switch_data[1] &= spi_previous_switch_data[1];
	
	spi_current_switch_data[2] ^= spi_previous_switch_data[2];
	spi_previous_switch_data[2] ^= spi_current_switch_data[2];
	spi_current_switch_data[2] &= spi_previous_switch_data[2];
	
	spi_current_switch_data[3] ^= spi_previous_switch_data[3];
	spi_previous_switch_data[3] ^= spi_current_switch_data[3];
	spi_current_switch_data[3] &= spi_previous_switch_data[3];			
	
	spi_current_switch_data[4] ^= spi_previous_switch_data[4];
	spi_previous_switch_data[4] ^= spi_current_switch_data[4];
	spi_current_switch_data[4] &= spi_previous_switch_data[4];
	
	//switch_states[0] ^= spi_current_switch_data[0];
	//switch_states[1] ^= spi_current_switch_data[1];
	//switch_states[2] ^= spi_current_switch_data[2];
	//switch_states[3] ^= spi_current_switch_data[3];
	//switch_states[4] ^= spi_current_switch_data[4];
	
	//spi_shift_byte(spi_data[5]);
	//spi_shift_byte(spi_data[6]);
	//spi_shift_byte(spi_data[7]);
	//spi_shift_byte(spi_data[8]);
	

	
};