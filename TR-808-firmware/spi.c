/*
 *spi.c
 *Open808 firmware ATMEGA328PB
 *minisystem
 *system80.net 
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
	
uint8_t spi0_current_trigger_byte0 = 0;
uint8_t spi0_current_trigger_byte1 = 0;
uint8_t spi0_previous_trigger_byte0 = 0;
uint8_t spi0_previous_trigger_byte1 = 0;	
	
uint8_t spi_shift_byte(uint8_t byte) { //shifts out byte for LED data and simultaneously reads switch data
	
	SPDR1 = byte;
	while (!(SPSR1 & (1<<SPIF1)));
	return SPDR1;
	
}	

uint8_t spi0_shift_byte(void) {
	
	SPDR0 = 0xFF; //MOSI is output for trigger SPI latch active LOW, so make sure it is LOW or HIGH? keep high then 
	while (!(SPSR0 & (1<<SPIF0)));
	return SPDR0;
}

void spi0_read_triggers() {
	
	//setup SPI0
	DDRB |= (1<<SPI0_SS) | (1<<SPI0_SCK) | (1<<SPI0_LATCH); //set MOSI and SS as outs (SS needs to be set as output or it breaks SPI
	PORTB |= (1<<SPI0_SS) | (1<<SPI0_LATCH);
	SPCR0 |= (1<<SPE0) | (1<<MSTR0); //Start SPI as MASTER
	SPSR0 |= (1<<SPI2X); //set clock rate to XTAL/2 (8 MHz)
	
	//read triggers
	
	PORTB |= (1 << SPI0_LATCH); //latch trigger data
	spi0_current_trigger_byte1 = ~(spi0_shift_byte()); //last byte in chain is first byte read, yeah? so byte1 is CB, CY, OH, CH
	spi0_current_trigger_byte0 = ~(spi0_shift_byte()); //first byte in chain is last byte read, so byte0 is BD, SD, LT, MT, HT, RS, CP, MA
	PORTB &= ~(1<<SPI0_LATCH); //latch next set of bits. this is kind of funny, first line ensures line is high, data is read and then this line latches for the next read, right?
	//disable SPI0
	SPCR0 &= ~(1<<SPE0);
	DDRB &= ~(1<<SPI0_SS); //need to set PB2 back to input for TAP input
	
	//debounce trigger data
	spi0_current_trigger_byte0 ^= spi0_previous_trigger_byte0;
	spi0_current_trigger_byte1 ^= spi0_previous_trigger_byte1;
	
	spi0_previous_trigger_byte0 ^= spi0_current_trigger_byte0;
	spi0_previous_trigger_byte1 ^= spi0_current_trigger_byte1;
	
	spi0_current_trigger_byte0 &= spi0_previous_trigger_byte0;
	spi0_current_trigger_byte1 &= spi0_previous_trigger_byte1;	
	
	//parse trigger data
	//ok, so try using something like trigger_step here. this way simultaneously arriving triggers will be processed and sound at the same time.
	process_external_triggers();
	//if ((spi0_current_trigger_byte0 >> BD) & 1) {
		////need to XOR individual bits
		//spi0_current_trigger_byte0 ^= (1<< BD);
		//trigger_drum(BD, 0);	
	//}
	 
}

void spi_read_write(void) {
	
	PORTE |= (1 << SPI_SW_LATCH); //latch switch data
	
	spi_current_switch_data[0] = spi_shift_byte(spi_data[LATCH_0]);
	spi_current_switch_data[1] = spi_shift_byte(spi_data[LATCH_1]);
	spi_current_switch_data[2] = spi_shift_byte(spi_data[LATCH_2]);
	spi_current_switch_data[3] = spi_shift_byte(spi_data[LATCH_3]);
	spi_current_switch_data[4] = spi_shift_byte(spi_data[LATCH_4]);
		
	PORTE &= ~(1<<SPI_SW_LATCH); //latch switch data

	spi_shift_byte(spi_data[LATCH_5]); //send out rest of spi data
	spi_shift_byte(spi_data[LATCH_6]);
	spi_shift_byte(spi_data[LATCH_7]);
	spi_shift_byte(spi_data[LATCH_8]);
		
	PORTE &= ~(1<<SPI_LED_LATCH); //latch data to output registers
	PORTE |= (1<<SPI_LED_LATCH);	
	
	//now process switch data, but this may be happening too fast for effective debouncing - maybe handle this in another function that runs less frequently?
	sequencer.SHIFT = ((spi_current_switch_data[0] >> SHIFT_BIT) & 1); //this detects press and hold rather than a toggle, like most other switch handling
	sequencer.ALT = ((spi_current_switch_data[0] >> ALT_BIT) & 1);
	sequencer.CLEAR = ((spi_current_switch_data[2] >> CLEAR_BIT) & 1);
		
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
	
	
}

void update_spi(void) { //updates LEDs and triggers, doesn't read data back
	    spi_shift_byte(spi_data[LATCH_0]);
		spi_shift_byte(spi_data[LATCH_1]);
		spi_shift_byte(spi_data[LATCH_2]);
		spi_shift_byte(spi_data[LATCH_3]);
		spi_shift_byte(spi_data[LATCH_4]);
		spi_shift_byte(spi_data[LATCH_5]);
		spi_shift_byte(spi_data[LATCH_6]);
		spi_shift_byte(spi_data[LATCH_7]);
		spi_shift_byte(spi_data[LATCH_8]);
		
		PORTE &= ~(1<<SPI_LED_LATCH);
		PORTE |= (1<<SPI_LED_LATCH);
	
};

void read_switches(void) { //reads switch data
	
	PORTE |= (1<<SPI_SW_LATCH); //latch switch data
	
	spi_current_switch_data[0] = spi_shift_byte(0x00); //not toggling LED latches so doesn't matter what we send out
	spi_current_switch_data[1] = spi_shift_byte(0x00);
	spi_current_switch_data[2] = spi_shift_byte(0x00);
	spi_current_switch_data[3] = spi_shift_byte(0x00);
	spi_current_switch_data[4] = spi_shift_byte(0x00);
	
	PORTE &= ~(1<<SPI_SW_LATCH);
	
	sequencer.SHIFT = ((spi_current_switch_data[0] >> SHIFT_BIT) & 1); //this detects press and hold rather than a toggle, like most other switch handling
	sequencer.ALT = ((spi_current_switch_data[0] >> ALT_BIT) & 1);
	sequencer.CLEAR = ((spi_current_switch_data[2] >> CLEAR_BIT) & 1);
	
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