/*
 * twi_eeprom.h
 *
 * Created: 2016-06-12 1:48:20 PM
 *  Author: jeff
 */ 


#ifndef TWI_EEPROM_H
#define TWI_EEPROM_H

#include "sequencer.h"
#define EEPROM_DEVICE_ID 0b1010000

#define PAGE_SIZE 128//32 //EEPROM page size (32 bytes for AT24CS64 EEPROM, 64 or 128 bytes for larger EEPROMS) - using 128 page 512 kilobit EEPROM now
#define PAGES_PER_PATTERN (int)((sizeof(pattern_data)/PAGE_SIZE) + 1)//need to know how many pages there are per pattern for addressing EEPROM memory
#define BANK_SIZE PAGES_PER_PATTERN*PAGE_SIZE*16 //16 patterns in a bank

#define RHYTHM_TRACK_MEMORY_OFFSET (((NUM_BANKS*16) + 1)*PAGE_SIZE*PAGES_PER_PATTERN //base EEPROM address for rhythm tracks 

//Define format of eeprom pattern data block:
typedef struct {
	struct pattern variation_a;
	struct pattern variation_b;
	uint8_t pre_scale;
	uint8_t step_num[NUM_PARTS];

	//uint8_t shuffle:3;
	
} pattern_data;

//define format of eeprom rhythm track data block:

typedef struct {
	
	struct track_pattern patterns[NUM_PATTERNS];
	uint8_t length:7; //this is only required once per rhythm track, but it seems easiest to include it here.
	
	
} rhythm_track_data;


//extern struct pattern_data eeprom_pattern;

//typedef struct pattern_data pattern_data;

void eeprom_init();
pattern_data read_pattern(uint16_t memory_address, uint8_t bank);
void write_pattern(uint16_t memory_address, uint8_t bank, pattern_data *w_data);
//void write_bytes(uint16_t memory_address, (char *)w_data, int num_bytes_to_write);

rhythm_track_data eeprom_get_rhythm_track(uint16_t memory_address);
void write_rhythm_track(uint16_t memory_address, rhythm_track_data *w_data);

void handle_TWI_result(uint8_t return_code);



#endif