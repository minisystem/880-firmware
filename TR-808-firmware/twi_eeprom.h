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
#define PAGES_PER_PATTERN (uint16_t)((sizeof(pattern_data)/PAGE_SIZE) + 1)//need to know how many pages there are per pattern for addressing EEPROM memory
#define BANK_SIZE PAGES_PER_PATTERN*PAGE_SIZE*16 //16 patterns in a bank
#define NUM_PAGES_PATTERN (int)(sizeof(pattern_data) / PAGE_SIZE)
#define REMAINING_PATTERN (int)(sizeof(pattern_data) % PAGE_SIZE)

#define PAGES_PER_TRACK (uint16_t)((sizeof(rhythm_track_data)/PAGE_SIZE) + 1) //this must be one page, right? 128 bytes?
#define TRACK_SIZE PAGES_PER_TRACK*PAGE_SIZE


//#define RHYTHM_TRACK_ADDR_OFFSET ((NUM_BANKS*16) + 1)*PAGE_SIZE*PAGES_PER_PATTERN //base EEPROM address for rhythm tracks 

#define RHYTHM_TRACK_ADDR_OFFSET NUM_BANKS*BANK_SIZE + 1

#define RECALL_ADDR_OFFSET (RHYTHM_TRACK_ADDR_OFFSET*NUM_TRACKS + 1) //base EEPROM address for recall settings: MIDI channel, last pattern/bank edited, etc.

//Define format of eeprom pattern data block:
typedef struct {
	struct pattern variation_a;
	struct pattern variation_b;
	uint8_t pre_scale;
	uint8_t step_num[NUM_PARTS];
	uint8_t shuffle:3;
	
} pattern_data;

//Define format of eeprom rhythm track data block:
typedef struct {
	
	struct track_pattern patterns[NUM_PATTERNS];
	uint8_t length;
	
} rhythm_track_data;

//define format of recall data block
/*typedef struct {
	
	struct recall;
	
	} recall_data;

*/
//extern struct pattern_data eeprom_pattern;

//typedef struct pattern_data pattern_data;
void blocking_copy_active_pattern(uint8_t dest_pattern);

void eeprom_init();
pattern_data read_pattern(uint16_t memory_address, uint8_t bank);
void start_write_current_pattern(uint8_t pattern_num, uint8_t pattern_bank);
//void write_bytes(uint16_t memory_address, (char *)w_data, int num_bytes_to_write);
void write_next_pattern_page();
void write_pattern_page(uint16_t memory_address, uint8_t bank, pattern_data *w_data, int page_number);
struct rhythm_track eeprom_read_rhythm_track(uint16_t memory_address);
void eeprom_write_rhythm_track(uint16_t memory_address, rhythm_track_data *w_data);
void eeprom_read_recall_data();
void eeprom_write_recall_data();
void handle_TWI_result(uint8_t return_code);



#endif