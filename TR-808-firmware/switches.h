#ifndef SWITCHES_H
#define SWITCHES_H

#define	INST_CH_12_SW	0
#define	SHIFT_SW		1
#define	IF_VAR_SW		2
#define	INST_LT_4_SW	0
#define	INST_MT_5_SW	1
#define	INST_HT_6_SW	2
#define	INST_RS_7_SW	3
#define	INST_CP_8_SW	4
#define	INST_CB_9_SW	5
#define	INST_CY_10_SW	6
#define	INST_OH_11_SW	7
#define	BASIC_VAR_A_SW	0
#define	BASIC_VAR_B_SW	1
#define	MODE_SW			2
#define	FILL_SW			3
#define	CLEAR_SW		4
#define	INST_AC_1_SW	5
#define	INST_BD_2_SW	6
#define	INST_SD_3_SW	7
#define	STEP_9_SW		0
#define	STEP_10_SW		1
#define	STEP_11_SW		2
#define	STEP_12_SW		3
#define	STEP_13_SW		4
#define	STEP_14_SW		5
#define	STEP_15_SW		6
#define	STEP_16_SW		7
#define	STEP_1_SW		0
#define	STEP_2_SW		1
#define	STEP_3_SW		2
#define	STEP_4_SW		3
#define	STEP_5_SW		4
#define	STEP_6_SW		5
#define	STEP_7_SW		6
#define	STEP_8_SW		7

#define NUM_BUTTONS		35

struct button {
	

	uint8_t spi_bit;
	uint8_t spi_byte:4;
	uint8_t current_state:1;
	uint8_t previous_state:1;
	uint8_t held:1;
	
	
	};
	
extern struct button button[NUM_BUTTONS];	

void parse_switch_data(void);
	
	
#endif