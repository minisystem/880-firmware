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

#define PAGE_SIZE 32 //EEPROM page size (32 bytes for AT24CS64 EEPROM, 64 or 128 bytes for larger EEPROMS)
#define PAGES_PER_PATTERN (int)((sizeof(pattern_data)/PAGE_SIZE) + 1)//need to know how many pages there are per pattern for addressing EEPROM memory

//Define format of eeprom data block.
typedef struct {
	struct pattern variation_a;
	struct pattern variation_b;
	uint8_t pre_scale:2;
	uint8_t step_num[NUM_PARTS];
	//uint8_t shuffle_amount:4;
} pattern_data;

//extern struct pattern_data eeprom_pattern;

//typedef struct pattern_data pattern_data;

void eeprom_init();
pattern_data read_pattern(uint16_t memory_address);
void write_pattern(uint16_t memory_address, pattern_data *w_data);
//void write_bytes(uint16_t memory_address, (char *)w_data, int num_bytes_to_write);
void handle_TWI_result(uint8_t return_code);



#endif