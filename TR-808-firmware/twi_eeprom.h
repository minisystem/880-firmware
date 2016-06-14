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
//Define format of eeprom data block.
struct pattern_data {
	struct pattern *variation_a;
	struct pattern *variation_b;
	uint8_t pre_scale:2;
	uint8_t step_num[NUM_PARTS];
	//uint8_t step_num_second;
};

//extern struct pattern_data eeprom_pattern;

typedef struct pattern PATTERN;

void eeprom_init();
PATTERN read_pattern(uint16_t memory_address);
void write_pattern(uint16_t memory_address, PATTERN *w_data);
void handle_TWI_result(uint8_t return_code);



#endif