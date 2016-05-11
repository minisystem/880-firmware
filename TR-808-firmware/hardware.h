#ifndef HARDWARE_H
#define HARDWARE_H

#define TRIG PD5
	
//define SPI pins
#define SPI_MOSI		PE3
#define SPI_MISO		PC0
#define SPI_CLK			PC1
#define SPI_SW_LATCH	PC2
#define SPI_LED_LATCH	PC3
#define SPI_SS			PE2 //this may conflict with Tempo pot!
#define SPI_EN			PB0

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
#define ACCENT_LED		4	
#define BD_LED			5
#define SD_LED			6
#define LT_LED			7
#define LC_LED			4
#define MT_LED			0
#define MC_LED			5
#define HT_LED			1
#define HC_LED			6
#define RS_LED			2
#define CL_LED			7
#define CP_LED			3
#define MA_LED			4
#define CB_LED			6
#define CY_LED			7
#define OH_LED			4
#define CH_LED			5



//define instrument switches
#define LT_LC_SW		0
#define MT_MC_SW		1
#define HT_HC_SW		2
#define RS_CL_SW		3
//#define CP_MA_SW		5 //this will eventually be MA trigger as MA can be triggered completely independently of CP, unlike toms/congas and rs/cl


#endif	