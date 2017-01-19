/*
 * hardware.h
 * Open808 firmware ATMEGA328PB
 * minisystem
 * system80.net
 */
#ifndef HARDWARE_H
#define HARDWARE_H

#define TRIG PD5

#define TRIGGER_OUT_1	PB3
#define TRIGGER_OUT_2	PB4
#define TRIGGER_OUT		PORTB
#define TRIGGER_OFF		~((1<<PB3) | (1<<PB4)) //trigger off mask

#define TEMPO_POT		ADC3D

#define START_STOP		PB1
#define TAP				PB2

#define DIN_CLOCK		PD3
#define DIN_RUN_STOP	PD4
#define DIN_RESET		PD6
#define DIN_FILL		PD7

#define SYNC_IN			PD2

//define MIDI SYNC LED pins
#define SYNC_LED_Y		PE2
#define SYNC_LED_R		PC2
	
//define SPI pins
#define SPI_MOSI		PE3
#define SPI_MISO		PC0
#define SPI_CLK			PC1
#define SPI_SW_LATCH	PE0
#define SPI_LED_LATCH	PE1
#define SPI_SS			PE2
#define SPI_EN			PB0

//define SPI latch numbers
#define LATCH_0			0
#define LATCH_1			1
#define LATCH_2			2
#define LATCH_3			3
#define LATCH_4			4
#define LATCH_5			5
#define LATCH_6			6
#define LATCH_7			7
#define LATCH_8			8

//define instrument triggers

#define ACCENT			0
#define BD_TRIG			1
#define SD_TRIG			2
#define LT_TRIG			3
#define MT_TRIG			4
#define HT_TRIG			5
#define RS_TRIG			6
#define CP_TRIG			7
#define MA_TRIG			5
#define CB_TRIG			0
#define CY_TRIG			1
#define OH_TRIG			2
#define CH_TRIG			3
//define instrument LEDs 
#define ACCENT_LED			4	
#define BD_LED_BIT			5
#define SD_LED_BIT			6
#define LT_LED_BIT			7
#define LC_LED_BIT			4
#define MT_LED_BIT			0
#define MC_LED_BIT			5
#define HT_LED_BIT			1
#define HC_LED_BIT			6
#define RS_LED_BIT			2
#define CL_LED_BIT			7
#define CP_LED_BIT			3
#define MA_LED_BIT			4
#define CB_LED_BIT			6
#define CY_LED_BIT			7
#define OH_LED_BIT			4
#define CH_LED_BIT			5



//define instrument switches
#define LT_LC_SW		0
#define MT_MC_SW		1
#define HT_HC_SW		2
#define RS_CL_SW		3
//#define CP_MA_SW		5 //this will eventually be MA trigger as MA can be triggered completely independently of CP, unlike toms/congas and rs/cl


#endif	