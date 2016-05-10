/*
 * TR-808-firmware.c
 *
 * Created: 2016-05-08 3:25:05 PM
 * Author : minisystem
 */ 

#include <avr/io.h>
#define F_CPU 16000000UL

#include <avr/interrupt.h>
#include <util/delay.h>
#include "hardware.h"
#include "midi.h"
#include "drums.h"
#include "xnormidi-develop/midi.h"
#include "xnormidi-develop/midi_device.h"
//#include "xnormidi-develop/bytequeue/bytequeue.h" //this is required for MIDI sending

MidiDevice midi_device;

enum drum {
	
	BD,
	SD,
	LT,
	LC,
	MT,
	MC,
	HT,
	HC,
	RS,
	CL,
	CP,
	MA,
	CB,
	CY,
	OH,
	CH
	
	};
	
//struct spi_data { //make this array so that spi_data[drum_hit.spi_byte_num] |= drum_hit.trig_bit
	//
	//uint8_t byte0;
	//uint8_t byte1;
	//uint8_t byte2;
	//uint8_t byte3;
	//uint8_t byte4;
	//uint8_t byte5;
	//uint8_t byte6;
	//
	//};

uint8_t spi_data[16] = {0};	

	
//struct spi_data spi_data;	
	
uint8_t drum_map[] = {
	(1<<BD_TRIG), //MIDI NOTE 0
	(1<<SD_TRIG), //MIDI NOTE 1
	(1<<LT_TRIG), //MIDI NOTE 2
	(1<<MT_TRIG), //MIDI NOTE 3
	(1<<HT_TRIG), //MIDI NOTE 4
	(1<<RS_TRIG), //MIDI NOTE 5
	(1<<CP_TRIG), //MIDI NOTE 6
	(1<<CB_TRIG), //MIDI NOTE 7
	(1<<CY_TRIG), //MIDI NOTE 8
	(1<<OH_TRIG), //MIDI NOTE 9
	(1<<CH_TRIG), //MIDI NOTE 10
	(1<<LT_TRIG), //MIDI NOTE 11 LOW TOM
	(1<<MT_TRIG), //MIDI NOTE 12 MID TOM
	(1<<HT_TRIG), //MIDI NOTE 13 HI TOM
	(1<<RS_TRIG),  //MIDI NOTE 14 CLAVES
	(1<<MA_TRIG)  //MIDI NOTE 15 MARACAS
	
	}; //default map is MIDI note 0 to 14
uint8_t drum_led_map[] = {
	(1<<BD_LED), 
	(1<<SD_LED),
	(1<<LC_LED), 
	(1<<MC_LED),
	(1<<HC_LED),
	(1<<RS_LED),
	(1<<CP_LED),
	(1<<CB_LED),
	(1<<CY_LED),
	(1<<OH_LED),
	(1<<CH_LED),
	(1<<LT_LED),
	(1<<MT_LED),
	(1<<HT_LED),
	(1<<CL_LED),
	(1<<MA_LED)
	
	};	

uint8_t spi_shift_byte(uint8_t byte) { //shifts out byte for LED data and simultaneously reads switch data
	
	SPDR1 = byte;
	while (!(SPSR1 & (1<<SPIF1)));
	return SPDR1;
	
}
void note_on_event(MidiDevice * device, uint8_t status, uint8_t note, uint8_t velocity) {
	
	
	if (note < 16) {
		
		spi_data[drum_hit[note].spi_byte_num] |= drum_hit[note].trig_bit;
		spi_data[drum_hit[note].spi_led_byte_num] |= drum_hit[note].led_bit;
		
		if (drum_hit[note].switch_bit != 255) {//need to set instrument switch
			
			
			spi_data[1] ^= (-(drum_hit[note].switch_value) ^ spi_data[1]) & drum_hit[note].switch_bit; //this sets switch_value in spi_data byte to switch_value (0 or 1)
			
		}
		
		PORTD |= 1<<TRIG;
		
		spi_shift_byte(spi_data[0]);
		spi_shift_byte(spi_data[1]);
		spi_shift_byte(spi_data[2]);
		spi_shift_byte(spi_data[3]);
		spi_shift_byte(spi_data[4]);
		spi_shift_byte(spi_data[5]);
		spi_shift_byte(spi_data[6]);
		
		PORTC &= ~(1<<SPI_LED_LATCH);
		PORTC |= (1<<SPI_LED_LATCH);
		
		PORTD &= ~(1<<TRIG);
		
		_delay_us(900);
		
		spi_data[drum_hit[note].spi_byte_num] &= ~(drum_hit[note].trig_bit);
		spi_data[drum_hit[note].spi_led_byte_num] &= ~(drum_hit[note].led_bit);
		
		spi_shift_byte(spi_data[0]);
		spi_shift_byte(spi_data[1]);
		spi_shift_byte(spi_data[2]);
		spi_shift_byte(spi_data[3]);
		spi_shift_byte(spi_data[4]);
		spi_shift_byte(spi_data[5]);
		spi_shift_byte(spi_data[6]);
		
		PORTC &= ~(1<<SPI_LED_LATCH);
		PORTC |= (1<<SPI_LED_LATCH);
		
		
	}
	//if (note <= 10)
	//{
		//if (note < 7) { //this sends triggers out sequentially, rather than simultaneously, in the case of two or more sounds triggering at the same time. should allow for independent accent control?
		//spi_data.byte6 = drum_map[note];
		//spi_data.byte4 = 0;
		//} else {
			//
			//spi_data.byte4 = drum_map[note];
			//spi_data.byte6 = 0;
			//
		//}
		//
//
	//} else {
		//
		//spi_data.byte4 = spi_data.byte6 = 0;
		////switch (note) {
			////
			////case 11:
				////spi_data.byte1 = 1<<LT_LC_SW;
			////
			////case 12:
			////
			////case 13:
			////
			////case 14:
			////
			////default:
			////
		////}
		//
	//}
	//
	//spi_data.byte6 &= ~(1<<ACCENT);
	//if (velocity > 64) spi_data.byte6 |= 1<<ACCENT;
	//
	//PORTD |= 1<<TRIG;
	//
	//spi_shift_byte(1<<ACCENT_LED);
	//spi_shift_byte(0x00);
	//spi_shift_byte(0x00);
	//spi_shift_byte(0x00);
	//spi_shift_byte(spi_data.byte4);
	//spi_shift_byte(0x00);
	//spi_shift_byte(spi_data.byte6);
	//
	//PORTC &= ~(1<<SPI_LED_LATCH);
	//PORTC |= (1<<SPI_LED_LATCH);
	//
	//PORTD &= ~(1<<TRIG);	
	//
	//_delay_us(900); //instrument triggers need to be 1 ms wide. Need to handle this better. set up timer interrupt here
//
	//spi_shift_byte(0x00);
	//spi_shift_byte(0x00);
	//spi_shift_byte(0x00);
	//spi_shift_byte(0x00);
	//spi_shift_byte(0x00);
	//spi_shift_byte(0x00);
	//spi_shift_byte(0x00);
	//PORTC &= ~(1<<SPI_LED_LATCH);
	//PORTC |= (1<<SPI_LED_LATCH);	
		
}

void note_off_event(MidiDevice * device, uint8_t status, uint8_t note, uint8_t velocity) {
	
	//spi_shift_byte(0x00);
	//spi_shift_byte(0x00);
	//spi_shift_byte(0x00);
	//spi_shift_byte(0x00);
	//spi_shift_byte(0x00);
	//spi_shift_byte(0x00);
	//spi_shift_byte(0x00);
	//PORTC &= ~(1<<SPI_LED_LATCH);
	//PORTC |= (1<<SPI_LED_LATCH);	

	//spi_data.byte4 = 0;
	//spi_data.byte6 = 0;		
}

void real_time_event(MidiDevice * device, uint8_t real_time_byte) {}

void setup_midi_usart(void)
{
	uint16_t ubbr_value = 31; //16MHz/(16*31250 BAUD) - 1
	//write ubbr_value to H and L UBBR1 registers:
	UBRR0L = (unsigned char) ubbr_value;
	UBRR0H = (unsigned char) (ubbr_value >> 8);
	
	UCSR0B = (1<<RXEN0)|(1<<TXEN0) | (1<<RXCIE0);
	//UCSR0C |= (0<<UMSEL0)|(0<<UMSEL01)|(0<<UPM01)|(0<<UPM00)|(0<<USBS0)|(0<<UCSZ02)|(1<<UCSZ01)|(1<<UCSZ00);
}



ISR (USART0_RX_vect) { // USART receive interrupt
	//PORTB ^= (1<<ARP_SYNC_LED); //toggle arp VCO_SYNC_LATCH_BIT LED
	uint8_t inByte = UDR0;
	midi_device_input(&midi_device, 1, &inByte);
	//calling a function in an interrupt is inefficient according to AVR C guidelines
	// so this function should maybe be inlined in main loop if inByte is made volatile
	//***HOWEVER***, xnor-midi example code has this function being called from USART_RX_vect ISR
}



int main(void)
{
	
	
	
    DDRD |= (1<<TRIG); //set PD5, TRIG to output
	
	//setup SPI
	DDRE |= (1<<SPI_MOSI) | (1<<SPI_SS); //set MOSI and SS as outs (SS needs to be set as output or it breaks SPI
	DDRC |= (1<<SPI_CLK) | (1<<SPI_LED_LATCH) | (1<<SPI_SW_LATCH);
	DDRB |= (1<<SPI_EN);
	
	PORTE &= ~(1<<SPI_MOSI );
	PORTC &= ~(1<<SPI_CLK | 1<<SPI_LED_LATCH | 1<<SPI_SW_LATCH);
	PORTB &= ~(1<<SPI_EN); //active low
	
	PORTC |= (1<<SPI_LED_LATCH); //toggle LED LATCH HIGH (disabled)
	
	SPCR1 = (1<<SPE1) | (1<<MSTR1); //Start SPI as MASTER	
	

	
	spi_shift_byte(1<<ACCENT_LED);
	spi_shift_byte(0x00);
	spi_shift_byte(0x00);
	spi_shift_byte(0x00);
	spi_shift_byte(0x00);
	spi_shift_byte(0x00);
	spi_shift_byte(0x00);
	
	
	
	PORTC &= ~(1<<SPI_LED_LATCH);
	PORTC |= (1<<SPI_LED_LATCH);
	
	//PORTB |= (1<<SPI_EN); //disable SPI for testing
	
	//setup MIDI
	//initialize MIDI device
	midi_device_init(&midi_device);
	//register callbacks
	midi_register_noteon_callback(&midi_device, note_on_event);
	midi_register_noteoff_callback(&midi_device, note_off_event);
	midi_register_realtime_callback(&midi_device, real_time_event);
	//midi_register_songposition_callback(&midi_device, song_position_event);
	//setup MIDI USART
	setup_midi_usart();
	
	sei(); //enable global interrupts	
	
    while (1) 
    {
	midi_device_process(&midi_device); //this needs to be called 'frequently' in order for MIDI to work		
	//PORTD |= (1<<TRIG);
	//
	//spi_shift_byte(1<<ACCENT_LED | 1<<BD_LED); //this should take about 20 us to execute based on defautl SPI clock settings
	//spi_shift_byte(1<<LT_LC_SW | 1<<MT_MC_SW | 1<<HT_HC_SW);
	//spi_shift_byte(0x00);
	//spi_shift_byte(0x00);
	//spi_shift_byte(1<<CB_TRIG | 1<<CY_TRIG | 1<<OH_TRIG);
	//spi_shift_byte(0x00);
	//spi_shift_byte(1<<ACCENT | 1<<BD_TRIG | 1<<LT_TRIG | 1<<MT_TRIG | 1<<HT_TRIG | 1<<RS_TRIG | 1<<CP_TRIG);
	////spi_shift_byte(0x00);	
	//PORTC &= ~(1<<SPI_LED_LATCH);
	//PORTC |= (1<<SPI_LED_LATCH);
	//
	//PORTD &= ~(1<<TRIG);
	//_delay_us(1000);
	//
	//spi_shift_byte(0x00);
	//spi_shift_byte(1<<LT_LC_SW | 1<<MT_MC_SW | 1<<HT_HC_SW);
	//spi_shift_byte(0x00);
	//spi_shift_byte(0x00);
	//spi_shift_byte(0x00);
	//spi_shift_byte(0x00);
	//spi_shift_byte(0x00);
//
	//PORTC &= ~(1<<SPI_LED_LATCH);
	//PORTC |= (1<<SPI_LED_LATCH);
	//_delay_ms(900);
	//
	//PORTD |= (1<<TRIG);
	//
	//spi_shift_byte(1<<ACCENT_LED | 1<<BD_LED | 1<<SD_LED); //this should take about 20 us to execute based on defautl SPI clock settings
	//spi_shift_byte(1<<LT_LC_SW | 1<<MT_MC_SW | 1<<HT_HC_SW);
	//spi_shift_byte(0x00);
	//spi_shift_byte(0x00);
	//spi_shift_byte(1<<CB_TRIG | 1<<CY_TRIG | 1<< CH_TRIG);
	//spi_shift_byte(0x00);
	//spi_shift_byte(1<<ACCENT | 1<<BD_TRIG | 1<<SD_TRIG | 1<<LT_TRIG | 1<<MT_TRIG | 1<<HT_TRIG | 1<<RS_TRIG | 1<<CP_TRIG);
	////spi_shift_byte(0x00);
	//PORTC &= ~(1<<SPI_LED_LATCH);
	//PORTC |= (1<<SPI_LED_LATCH);
	//
	//PORTD &= ~(1<<TRIG);
	//_delay_us(1000);
	//
	//spi_shift_byte(0x00);
	//spi_shift_byte(1<<LT_LC_SW | 1<<MT_MC_SW | 1<<HT_HC_SW);
	//spi_shift_byte(0x00);
	//spi_shift_byte(0x00);
	//spi_shift_byte(0x00);
	//spi_shift_byte(0x00);
	//spi_shift_byte(0x00);
//
	//PORTC &= ~(1<<SPI_LED_LATCH);
	//PORTC |= (1<<SPI_LED_LATCH);	
	//
	//_delay_ms(1000);	
    //}
	}
}

