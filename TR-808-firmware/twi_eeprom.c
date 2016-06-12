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
#include "twi_eeprom.h"
#include "twi.h"
#include "sequencer.h"

// Define format of eeprom data block.
//typedef struct {
	//uint16_t part[NUM_PARTS][NUM_STEPS]; 
	//uint16_t accent[NUM_PARTS]; 
	//uint8_t pre_scale:2;
	//uint8_t step_num[NUM_PARTS];
//} PATTERN;

// prototype local functions
typedef struct PATTERN_DATA PATTERN;

// -----------------------------------------------------------------
void eeprom_stuff() {
	// Initialize the lcd
	//lcd_init();
	//fdev_setup_stream(&lcd_stream, lcd_putchar, 0, _FDEV_SETUP_WRITE);
	//lcd_clear_and_home();
	
	// initialize eeprom and TWI/I2C
	eeprom_init();
	
	// specify 7 bit device address of eeprom chip
	#define EEPROM_DEVICE_ID 0b1010000
	//fprintf_P(&lcd_stream, PSTR("24LC256 ADDR: %02X"), EEPROM_DEVICE_ID);
	
	// specify eeprom memory address for data storage
	#define EEPROM_DATA_ADDRESS 0x0000
	
	// create local copy of eeprom data
	PATTERN pattern_buffer;
	
	// read eeprom contents at location 0000
	pattern_buffer = read_pattern(0);
	
	
	// show what we read from the eeprom -
	// note: the very first read on a new eeprom
	// will show uninitalized data
	//show_eeprom_data(&e_data);
	
	// process data
	//if(e_data.need_initalization){
		//// set all data item values if first time
		//e_data.need_initalization = false;
		//strcpy_P(e_data.author, PSTR("Noter"));
		//e_data.read_count = 1;
		//e_data.brightness = 0x33;
		//e_data.version = 1.01;
		//} else {
		//// check contents against the values written when initializing
		//if((e_data.version != 1.01)||
		//(strcmp_P(e_data.author, PSTR("Noter")) !=0)||
		//(e_data.brightness != 0x33)){
			//lcd_line_four();
			//fprintf_P(&lcd_stream, PSTR("DATA ERROR - "));
			//while(true);    // and freeze
			//} else {
			//// increment read_count
			//e_data.read_count += 1;
		//}
	//}
	
	// write data to eeprom memory at location 0000
	write_pattern(0, &pattern_buffer);
	
	// and show the read count
	//lcd_line_two();
	//fprintf_P(&lcd_stream, PSTR("READ COUNT = %d"), e_data.read_count);
	
	// done
	//while(true);
}
// -----------------------------------------------------------------

// Define eeprom commands according to 24CXXX datasheet
typedef struct {
	uint8_t     high_byte;
	uint8_t     low_byte;
	PATTERN		pattern_data;
} WRITE_PATTERN;

typedef struct {
	uint8_t     high_byte;
	uint8_t     low_byte;
} SET_PATTERN_ADDRESS;

typedef struct {
	PATTERN pattern_data;
} READ_PATTERN;

// Create structure pointers for the TWI/I2C buffer
WRITE_PATTERN            *p_write_pattern;
SET_PATTERN_ADDRESS      *p_set_pattern_address;
READ_PATTERN             *p_read_pattern;

// Create TWI/I2C buffer, size to largest command
char    TWI_buffer[sizeof(WRITE_PATTERN)];

void eeprom_init(){
	// Specify startup parameters for the TWI/I2C driver
	TWI_init(   F_CPU,           // clock frequency
	400000L,                    // desired TWI/IC2 bitrate
	TWI_buffer,                 // pointer to comm buffer
	sizeof(TWI_buffer),         // size of comm buffer
	&handle_TWI_result          // pointer to callback function
	);
	
	// Enable interrupts
	//sei();
	
	// Set our structure pointers to the TWI/I2C buffer
	p_write_pattern = (WRITE_PATTERN *)TWI_buffer;
	p_set_pattern_address = (SET_PATTERN_ADDRESS *)TWI_buffer;
	p_read_pattern = (READ_PATTERN *)TWI_buffer;
	
}

PATTERN read_eeprom(uint16_t memory_address){
	// send 'set memory address' command to eeprom and then read data
	while(TWI_busy);
	p_set_pattern_address->high_byte = memory_address >> 8;
	p_set_pattern_address->low_byte = memory_address & 0x0F;
	TWI_master_start_write_then_read(   EEPROM_DEVICE_ID,               // device address of eeprom chip
	sizeof(SET_PATTERN_ADDRESS),     // number of bytes to write
	sizeof(PATTERN)             // number of bytes to read
	);
	
	// nothing else to do - wait for the data
	while(TWI_busy);
	// return the data
	return(p_read_pattern->pattern_data);
}

// write eeprom - note: page boundaries are not considered in this example
void write_eeprom(uint16_t memory_address, PATTERN *w_data){
	while(TWI_busy);
	p_write_pattern->high_byte = EEPROM_DATA_ADDRESS >> 8;
	p_write_pattern->low_byte = EEPROM_DATA_ADDRESS & 0x0F;
	p_write_pattern->pattern_data = *w_data;
	TWI_master_start_write(     EEPROM_DEVICE_ID,       // device address of eeprom chip
	sizeof(WRITE_PATTERN)    // number of bytes to write
	);
	
}

// optional callback function for TWI/I2C driver
void handle_TWI_result(uint8_t return_code){
	if(return_code!=TWI_success){
		//lcd_line_four();
		//fprintf_P(&lcd_stream, PSTR("I2C ERROR - %02X"), return_code);
	}
}

// format and display eeprom data on lcd
//void show_eeprom_data(EEPROM_DATA *data){
	//lcd_line_three();
	//fprintf_P(&lcd_stream, PSTR("%1d %s %04X %02X %4.2f"),
	//data->need_initalization,
	//data->author,
	//data->read_count,
	//data->brightness,
	//data->version
	//);
//}