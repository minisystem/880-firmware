/*
 * switches.h
 * Open808 firmware ATMEGA328PB
 * minisystem
 * system80.net
 */
#ifndef SWITCHES_H
#define SWITCHES_H

#define	INST_CH_12_SW	27
#define	WRITE_SW	28 //write/next switch - same as SHIFT switch, but read as a single press
#define	IF_VAR_SW	29
#define	INST_LT_4_SW	19
#define	INST_MT_5_SW	20
#define	INST_HT_6_SW	21
#define	INST_RS_7_SW	22
#define	INST_CP_8_SW	23
#define	INST_CB_9_SW	24
#define	INST_CY_10_SW	25
#define	INST_OH_11_SW	26
#define	BASIC_VAR_A_SW	30
#define	BASIC_VAR_B_SW	31
#define	MODE_SW	32
#define	FILL_SW	33
#define	CLEAR_SW	34
#define	INST_AC_1_SW	16 //subtract NUM_INST to get 0 index for AC (1/0) to OH (16/15)
#define	INST_BD_2_SW	17
#define	INST_SD_3_SW	18
#define	STEP_9_SW	8
#define	STEP_10_SW	9
#define	STEP_11_SW	10
#define	STEP_12_SW	11
#define	STEP_13_SW	12
#define	STEP_14_SW	13
#define	STEP_15_SW	14
#define	STEP_16_SW	15
#define	STEP_1_SW	0
#define	STEP_2_SW	1
#define	STEP_3_SW	2
#define	STEP_4_SW	3
#define	STEP_5_SW	4
#define	STEP_6_SW	5
#define	STEP_7_SW	6
#define	STEP_8_SW	7

#define SW_DRUM_OFFSET 9 //switched drums are offset by 9 in drum_hits array

#define SHIFT_BIT 1 //bit position in spi data byte
#define CLEAR_BIT 4 
#define ALT_BIT  2

enum {
	OMAR_WAS_HERE = 0,
	OMAR_NOT_HERE,
	OMAR_FINAL
	};
	
#define EMPTY 255 //quick and dirty null value	

#define NUM_BUTTONS	35 //use sizeof of button array to increase flexibility
#define NUM_INST 16 //handily the number of instruments and also the offset from 0 of the INST_AC_1_SW

struct button {
	

	uint8_t spi_bit:3;
	uint8_t spi_byte:3;
	uint8_t state:1;
	//uint8_t previous_state:1;
	uint8_t held:1;
	
	
	};
	
extern struct button button[NUM_BUTTONS];

extern uint8_t current_start_stop_tap_state;
extern uint8_t previous_start_stop_tap_state;	

void parse_switch_data(void);

void check_start_stop_tap(void);

void check_write_sw(void);

uint8_t read_track_switches(void);
void test_update_track_leds(void);

void check_inst_switches(void);	

void assign_triggers(uint8_t drum_index);

void assign_mutes(uint8_t drum_index);

void clear_mutes(void);

void assign_solo(uint8_t drum_index);

void select_instrument(uint8_t drum_index);

void process_track_press(void);

void check_variation_switches(void);

void check_clear_switch(void);

void check_intro_fill_variation_switch(void);

uint8_t check_step_press(void); //return pressed step switch

void clear_all_patterns(void); //reset utility to clear eeprom
void clear_pattern_bank(uint8_t bank);
	
#endif