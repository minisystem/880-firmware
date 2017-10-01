/*
 * twi_eeprom.c
 *
 * Created: 2016-06-12 12:18:57 PM
 *  Author: jeff
 */ 
// hacked from: http://www.nerdkits.com/forum/thread/1423/

#include <avr/interrupt.h>
#include <stdbool.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "twi_eeprom.h"
#include "twi.h"
#include "sequencer.h"


// prototype local functiosn
//typedef struct pattern_data pattern_data;
//struct pattern_data eeprom_pattern;

//PATTERN PATTERN;
// -----------------------------------------------------------------


// Define eeprom commands according to 24CXXX datasheet
typedef struct {
	uint8_t			high_byte;
	uint8_t			low_byte;
	pattern_data	pattern_data;
} WRITE_PATTERN;

typedef struct {
	uint8_t     high_byte;
	uint8_t     low_byte;
} SET_PATTERN_ADDRESS;


// Create structure pointers for the TWI/I2C buffer
WRITE_PATTERN            *p_write_pattern;
SET_PATTERN_ADDRESS      *p_set_pattern_address;
pattern_data             *p_read_pattern;

rhythm_track_data		 *p_read_rhythm_track;

// Create TWI/I2C buffer, size to largest command
char    TWI_buffer[sizeof(WRITE_PATTERN)];

void eeprom_init(){
	// Specify startup parameters for the TWI/I2C driver
	TWI_init(   F_CPU,           // clock frequency
	800000L,                    // desired TWI/IC2 bitrate
	TWI_buffer,                 // pointer to comm buffer
	sizeof(TWI_buffer),         // size of comm buffer
	&handle_TWI_result          // pointer to callback function
	);
	
	// Enable interrupts
	//sei();
	
	// Set our structure pointers to the TWI/I2C buffer
	p_write_pattern = (WRITE_PATTERN *)TWI_buffer;
	p_set_pattern_address = (SET_PATTERN_ADDRESS *)TWI_buffer;
	p_read_pattern = (pattern_data *)TWI_buffer;
	
}

pattern_data read_pattern(uint16_t memory_address, uint8_t bank){
	// send 'set memory address' command to eeprom and then read data
	while(TWI_busy);
	memory_address = memory_address + (bank*BANK_SIZE);
	p_set_pattern_address->high_byte = (memory_address >> 8);
	p_set_pattern_address->low_byte = memory_address;
	TWI_master_start_write_then_read(   EEPROM_DEVICE_ID,               // device address of eeprom chip
	sizeof(SET_PATTERN_ADDRESS),     // number of bytes to write
	sizeof(pattern_data)             // number of bytes to read
	);
	
	// nothing else to do - wait for the data
	while(TWI_busy);
	// return the data

	return(*p_read_pattern);
}


void write_pattern(uint16_t memory_address, uint8_t bank, pattern_data *w_data){ //this writes whole pattern, including step number for each part and pre scale
	while(TWI_busy);
	int num_pages = sizeof(pattern_data) / PAGE_SIZE; //isn't this a constant? should it be defined as sizeof(pattern_data)/PAGE_SIZE?
	memory_address = memory_address + (bank*BANK_SIZE);
	//write 128 byte pages of data
	for (int i = 0; i < num_pages; ++i) {
		
		p_write_pattern->high_byte = (memory_address >> 8);
		p_write_pattern->low_byte = memory_address;
		// this is directly putting PAGE_SIZE bytes of the input PATTERN w_data into the TWI_buffer *after* the address bytes (hence the +2)
		// NOTE: p_write_pattern is a pointer to TWI_BUFFER
		memcpy(TWI_buffer+2, (char *)w_data + i*PAGE_SIZE, PAGE_SIZE);
		TWI_master_start_write(     EEPROM_DEVICE_ID,       // device address of eeprom chip
		2 + PAGE_SIZE
		);
		memory_address+= PAGE_SIZE;
		while(TWI_busy);
	}
	//write remaining bytes (ie. remainder of sizeof(pattern_data)/PAGE_SIZE)
	int remaining = sizeof(pattern_data) % PAGE_SIZE; //same thing here? Shouldn't this be defined as sizeof(pattern_data)%PAGE_SIZE?
	p_write_pattern->high_byte = (memory_address >> 8);
	p_write_pattern->low_byte = memory_address;
	memcpy(TWI_buffer+2, (char *)w_data + num_pages*PAGE_SIZE, remaining);
	TWI_master_start_write(     EEPROM_DEVICE_ID,       // device address of eeprom chip
	2 + remaining
	);
	
	while(TWI_busy);
}

//void write_bytes(uint16_t memory_address, (char *)w_data, int num_bytes_to_write) {
	//
	//
	//
//}

rhythm_track_data eeprom_read_rhythm_track(uint16_t memory_address) {
	
	return(*p_read_rhythm_track);
}

void write_rhythm_track(uint16_t memory_address, rhythm_track_data *w_data) {
	
	
}


// optional callback function for TWI/I2C driver
void handle_TWI_result(uint8_t return_code){
	//flag.twi_init_error = 0;
	if(return_code!=TWI_success){
		//flag.twi_init_error = 1;
		//turn_on(IF_VAR_B_LED);
	}
}

