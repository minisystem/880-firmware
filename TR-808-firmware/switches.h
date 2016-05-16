/*
 * switches.h
 * JR-808 firmware ATMEGA328PB
 * minisystem
 * system79.com
 */
#ifndef SWITCHES_H
#define SWITCHES_H

#define	INST_CH_12_SW	27
#define	SHIFT_SW	28
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
#define	INST_AC_1_SW	16
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

enum {
	OMAR_WAS_HERE = 0,
	OMAR_NOT_HERE,
	OMAR_FINAL
	};

#define NUM_BUTTONS		35 //use sizeof of button array to increase flexibility

struct button {
	

	uint8_t spi_bit:3;
	uint8_t spi_byte:3;
	uint8_t state:1;
	uint8_t previous_state:1;
	uint8_t held:1;
	
	
	};
	
extern struct button button[NUM_BUTTONS];	

void parse_switch_data(void);
	
	
#endif