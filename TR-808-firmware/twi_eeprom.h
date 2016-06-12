/*
 * twi_eeprom.h
 *
 * Created: 2016-06-12 1:48:20 PM
 *  Author: jeff
 */ 


#ifndef TWI_EEPROM_H
#define TWI_EEPROM_H

#include "sequencer.h"
// Define format of eeprom data block.
struct PATTERN_DATA {
	uint16_t part[NUM_PARTS][NUM_STEPS];
	uint16_t accent[NUM_PARTS];
	uint8_t pre_scale:2;
	uint8_t step_num[NUM_PARTS];
};

typedef struct PATTERN_DATA PATTERN;

void eeprom_init();
PATTERN read_pattern(uint16_t memory_address);
void write_pattern(uint16_t memory_address, PATTERN *w_data);
void handle_TWI_result(uint8_t return_code);



#endif