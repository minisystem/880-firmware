#ifndef SWITCHES_H
#define SWITCHES_H


struct button {
	
	uint8_t current_state:1;
	uint8_t previous_state:1;
	uint8_t spi_bit;
	uint8_t spi_byte;
	uint8_t held:1;
	
	
	};
#endif