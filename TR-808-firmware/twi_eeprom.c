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
#include "sequencer.h"
#include "twi_eeprom.h"
#include "twi.h"



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
	uint8_t				high_byte;
	uint8_t				low_byte;
	rhythm_track_data	rhythm_track_data;
	} WRITE_TRACK;

typedef struct {
	uint8_t     high_byte;
	uint8_t     low_byte;
} SET_EEPROM_ADDRESS;


// Create structure pointers for the TWI/I2C buffer
WRITE_PATTERN            *p_write_pattern;
WRITE_TRACK				*p_write_track;
SET_EEPROM_ADDRESS      *p_set_eeprom_address;
pattern_data             *p_read_pattern;
rhythm_track_data		 *p_read_rhythm_track_data;

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
	p_set_eeprom_address = (SET_EEPROM_ADDRESS *)TWI_buffer;
	
	p_write_pattern = (WRITE_PATTERN *)TWI_buffer;
	p_read_pattern = (pattern_data *)TWI_buffer;
	
	p_write_track = (WRITE_TRACK *)TWI_buffer;
	p_read_rhythm_track_data = (rhythm_track_data *)TWI_buffer;
	
}

pattern_data read_pattern(uint16_t memory_address, uint8_t bank){
	// send 'set memory address' command to eeprom and then read data
	while(TWI_busy);
	memory_address = memory_address + (bank*BANK_SIZE);
	p_set_eeprom_address->high_byte = (memory_address >> 8);
	p_set_eeprom_address->low_byte = memory_address;
	TWI_master_start_write_then_read(   EEPROM_DEVICE_ID,               // device address of eeprom chip
	sizeof(SET_EEPROM_ADDRESS),     // number of bytes to write
	sizeof(pattern_data)             // number of bytes to read
	);
	
	// nothing else to do - wait for the data
	while(TWI_busy);
	// return the data

	return(*p_read_pattern);
}

pattern_data pattern_to_write;
#define PATTERN_PAGE_STOP_WRITING (NUM_PAGES_PATTERN + 1)
int current_pattern_page = PATTERN_PAGE_STOP_WRITING;
uint8_t current_pattern_num;
uint8_t current_pattern_bank;
void start_write_current_pattern(uint8_t pattern_num, uint8_t pattern_bank) {
	//TRIGGER_OUT |= (1<<TRIGGER_OUT_2); //holy jeeze, I think I left this here just to measure pattern write times. good golly
	current_pattern_bank = pattern_bank;
	current_pattern_num = pattern_num;
	
	pattern_to_write.variation_a = sequencer.pattern[VAR_A];
	pattern_to_write.variation_b = sequencer.pattern[VAR_B];
	pattern_to_write.step_num[FIRST] = sequencer.step_num[FIRST];
	pattern_to_write.step_num[SECOND] = sequencer.step_num[SECOND];
	pattern_to_write.pre_scale = sequencer.pre_scale;
	
	current_pattern_page = 0;
	//TRIGGER_OUT &= TRIGGER_OFF;
}

// TODO: this should just be constantly called 
void write_next_pattern_page() {
	if (current_pattern_page <= NUM_PAGES_PATTERN) {
		write_pattern_page(current_pattern_num*PAGES_PER_PATTERN*PAGE_SIZE, current_pattern_bank, &pattern_to_write, current_pattern_page);
	}
}


// TODO: this is too slow. Instead, use the callback function: after every page is written, advance in the callback function to the 
// next page that needs writing. Do this until the whole pattern is written. This will get rid of this for loop
void write_pattern_page(uint16_t memory_address, uint8_t bank, pattern_data *w_data, int page_number){ //this writes whole pattern, including step number for each part and pre scale
	while(TWI_busy);
	//int num_pages = sizeof(pattern_data) / PAGE_SIZE; //isn't this a constant? should it be defined as sizeof(pattern_data)/PAGE_SIZE?
	memory_address = memory_address + (bank*BANK_SIZE);
	memory_address += page_number*PAGE_SIZE;
	if (page_number < NUM_PAGES_PATTERN) {	
		p_write_pattern->high_byte = (memory_address >> 8);
		p_write_pattern->low_byte = memory_address;
		// this is directly putting PAGE_SIZE bytes of the input PATTERN w_data into the TWI_buffer *after* the address bytes (hence the +2)
		// NOTE: p_write_pattern is a pointer to TWI_BUFFER
		memcpy(TWI_buffer+2, (char *)w_data + page_number*PAGE_SIZE, PAGE_SIZE);
		TWI_master_start_write(     EEPROM_DEVICE_ID,       // device address of eeprom chip
		2 + PAGE_SIZE
		);
	} else {
		//write remaining bytes (ie. remainder of sizeof(pattern_data)/PAGE_SIZE)
		//int remaining = sizeof(pattern_data) % PAGE_SIZE; //same thing here? Shouldn't this be defined as sizeof(pattern_data)%PAGE_SIZE?
		p_write_pattern->high_byte = (memory_address >> 8);
		p_write_pattern->low_byte = memory_address;
		memcpy(TWI_buffer+2, (char *)w_data + NUM_PAGES_PATTERN*PAGE_SIZE, REMAINING_PATTERN);
		TWI_master_start_write(     EEPROM_DEVICE_ID,       // device address of eeprom chip
		2 + REMAINING_PATTERN
		);	
	}
}

//void write_bytes(uint16_t memory_address, (char *)w_data, int num_bytes_to_write) {
	//
	//
	//
//}

struct rhythm_track eeprom_read_rhythm_track(uint16_t memory_address) {
	

	// send 'set memory address' command to eeprom and then read data
	while(TWI_busy);
	memory_address = (memory_address*TRACK_SIZE) + RHYTHM_TRACK_ADDR_OFFSET;
	p_set_eeprom_address->high_byte = (memory_address >> 8);
	p_set_eeprom_address->low_byte = memory_address;
	TWI_master_start_write_then_read(   EEPROM_DEVICE_ID,               // device address of eeprom chip
	sizeof(SET_EEPROM_ADDRESS),     // number of bytes to write
	sizeof(rhythm_track_data)             // number of bytes to read
	);
		
	// nothing else to do - wait for the data
	while(TWI_busy);
	// return the data
	
	struct rhythm_track* t = (struct rhythm_track*)p_read_rhythm_track_data;
	
	return(*t);
}

void eeprom_write_rhythm_track(uint16_t memory_address, rhythm_track_data *w_data) { //for the sake of speed, make this less generalized knowing that a rhythm track only takes up 1 page of eeprom data
	while(TWI_busy);
	// assert(sizeof(rhythm_track_data) <= PAGE_SIZE);
	int num_pages = sizeof(rhythm_track_data) / PAGE_SIZE; //isn't this a constant? should it be defined as sizeof(pattern_data)/PAGE_SIZE?
	memory_address = (memory_address*TRACK_SIZE) + RHYTHM_TRACK_ADDR_OFFSET;
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
	int remaining = sizeof(rhythm_track_data) % PAGE_SIZE; //same thing here? Shouldn't this be defined as sizeof(pattern_data)%PAGE_SIZE?
	p_write_pattern->high_byte = (memory_address >> 8);
	p_write_pattern->low_byte = memory_address;
	memcpy(TWI_buffer+2, (char *)w_data + num_pages*PAGE_SIZE, remaining);
	TWI_master_start_write(     EEPROM_DEVICE_ID,       // device address of eeprom chip
	2 + remaining
	);
	
	while(TWI_busy);	
	
}


// optional callback function for TWI/I2C driver
void handle_TWI_result(uint8_t return_code){
	//flag.twi_init_error = 0;
	if(return_code==TWI_success){
		current_pattern_page++;
		//flag.twi_init_error = 1;
		//turn_on(IF_VAR_B_LED);
	} else {
		current_pattern_page = PATTERN_PAGE_STOP_WRITING;
		// if there's a fail, I think we want to abort writing this pattern totally. We do this by making the current_pattern_page a big number
		
	}
}

