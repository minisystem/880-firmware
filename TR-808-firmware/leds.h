#ifndef LEDS_H
#define LEDS_H

#include "hardware.h"
//define LED indices
#define	STEP_9_LED	8
#define	STEP_10_LED	9
#define	STEP_11_LED	10
#define	STEP_12_LED	11
#define	STEP_13_LED	12
#define	STEP_14_LED	13
#define	STEP_15_LED	14
#define	STEP_16_LED	15
#define	STEP_1_LED	0
#define	STEP_2_LED	1
#define	STEP_3_LED	2
#define	STEP_4_LED	3
#define	STEP_5_LED	4
#define	STEP_6_LED	5
#define	STEP_7_LED	6
#define	STEP_8_LED	7
#define	FILL_12_LED	16
#define	FILL_8_LED	17
#define	FILL_4_LED	18
#define	FILL_2_LED	19
#define	ACCENT_1_LED	20
#define	BD_2_LED	21
#define	SD_3_LED	22
#define	LT_4_LED	23
#define	LC_LED	24
#define	MC_LED	25
#define	HC_LED	26
#define	CL_LED	27
#define	MODE_1_PATTERN_CLEAR	28
#define	MODE_2_PATTERN_FIRST_PART	29
#define	MODE_3_PATTERN_SECOND_PART	30
#define	MODE_4_MANUAL_PLAY	31
#define	MODE_5_RHYTHM_PLAY	32
#define	MODE_6_RHYTHM_COMPOSE	33
#define	FILL_MANUAL	34
#define	FILL_16	35
#define	BASIC_VAR_A_LED	36
#define	BASIC_VAR_B_LED	37
#define	SCALE_1_LED	38
#define	SCALE_2_LED	39
#define	SCALE_3_LED	40
#define	SCALE_4_LED	41
#define	FIRST_PART_LED	42
#define	SECOND_PART_LED	43
#define	OH_11_LED	44
#define	CH_12_LED	45
#define	IF_VAR_A_LED	46
#define	IF_VAR_B_LED	47
#define	MT_5_LED	48
#define	HT_6_LED	49
#define	RS_7_LED	50
#define	CP_8_LED	51
#define	MA_LED	52
#define	CB_9_LED	53
#define	CY_10_LED	54


struct led{
	
	uint8_t bit;
	uint8_t spi_byte:4;
	uint8_t state:1;
	enum {NO_BLINK, SLOW_BLINK, FAST_BLINK} mode;
	
	
	};

struct led led[55] = {
	
{	1<<0	,	1	,	0	,	NO_BLINK},
{	1<<1	,	1	,	0	,	NO_BLINK},
{	1<<2	,	1	,	0	,	NO_BLINK},
{	1<<3	,	1	,	0	,	NO_BLINK},
{	1<<4	,	1	,	0	,	NO_BLINK},
{	1<<5	,	1	,	0	,	NO_BLINK},
{	1<<6	,	1	,	0	,	NO_BLINK},
{	1<<7	,	1	,	0	,	NO_BLINK},
{	1<<0	,	0	,	0	,	NO_BLINK},
{	1<<1	,	0	,	0	,	NO_BLINK},
{	1<<2	,	0	,	0	,	NO_BLINK},
{	1<<3	,	0	,	0	,	NO_BLINK},
{	1<<4	,	0	,	0	,	NO_BLINK},
{	1<<5	,	0	,	0	,	NO_BLINK},
{	1<<6	,	0	,	0	,	NO_BLINK},
{	1<<7	,	0	,	0	,	NO_BLINK},
{	1<<0	,	2	,	0	,	NO_BLINK},
{	1<<1	,	2	,	0	,	NO_BLINK},
{	1<<2	,	2	,	0	,	NO_BLINK},
{	1<<3	,	2	,	0	,	NO_BLINK},
{	1<<4	,	2	,	0	,	NO_BLINK},
{	1<<5	,	2	,	0	,	NO_BLINK},
{	1<<6	,	2	,	0	,	NO_BLINK},
{	1<<7	,	2	,	0	,	NO_BLINK},
{	1<<4	,	3	,	0	,	NO_BLINK},
{	1<<5	,	3	,	0	,	NO_BLINK},
{	1<<6	,	3	,	0	,	NO_BLINK},
{	1<<7	,	3	,	0	,	NO_BLINK},
{	1<<0	,	4	,	0	,	NO_BLINK},
{	1<<1	,	4	,	0	,	NO_BLINK},
{	1<<2	,	4	,	0	,	NO_BLINK},
{	1<<3	,	4	,	0	,	NO_BLINK},
{	1<<4	,	4	,	0	,	NO_BLINK},
{	1<<5	,	4	,	0	,	NO_BLINK},
{	1<<6	,	4	,	0	,	NO_BLINK},
{	1<<7	,	4	,	0	,	NO_BLINK},
{	1<<0	,	5	,	0	,	NO_BLINK},
{	1<<1	,	5	,	0	,	NO_BLINK},
{	1<<2	,	5	,	0	,	NO_BLINK},
{	1<<3	,	5	,	0	,	NO_BLINK},
{	1<<4	,	5	,	0	,	NO_BLINK},
{	1<<5	,	5	,	0	,	NO_BLINK},
{	1<<6	,	5	,	0	,	NO_BLINK},
{	1<<7	,	5	,	0	,	NO_BLINK},
{	1<<4	,	6	,	0	,	NO_BLINK},
{	1<<5	,	6	,	0	,	NO_BLINK},
{	1<<6	,	6	,	0	,	NO_BLINK},
{	1<<7	,	6	,	0	,	NO_BLINK},
{	1<<0	,	7	,	0	,	NO_BLINK},
{	1<<1	,	7	,	0	,	NO_BLINK},
{	1<<2	,	7	,	0	,	NO_BLINK},
{	1<<3	,	7	,	0	,	NO_BLINK},
{	1<<4	,	7	,	0	,	NO_BLINK},
{	1<<6	,	7	,	0	,	NO_BLINK},
{	1<<7	,	7	,	0	,	NO_BLINK}	
	
	
	
	
	};
#endif