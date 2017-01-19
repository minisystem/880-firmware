/*
 * adc.c
 * Open808 firmware ATMEGA328PB
 * minisystem
 * system80.net
 */

#include <avr/io.h>
#include <stdio.h>
#include "adc.h"
#include "hardware.h"




void setup_adc(void) {
	
	ADCSRA |= (1<<ADPS2); // 16 MHz/16 = 1 MHz ADC clock, could go as high as 8 MHz (/2)
	ADMUX |= (1<<REFS0); //set ADC reference to AVCC (+5V)
	ADMUX |= TEMPO_POT; //select channel ADC3 (PC3)
	DIDR0 |= (1<<ADC3D); //digital input disable
	ADCSRA |= (1<<ADEN); //enable ADC.	
}

uint16_t read_tempo_pot() {
	
	//SPCR1 = 0; //disable spi during adc read
	//DDRE &= ~(1<<SPI_SS); //ADC input is shared with SPI slave select
	//ADCSRA |= (1<<ADEN); //enable ADC. Enabling here adds 12 cycles to ADC conversion, but can't see way to get around it because of shared SPI SS pin

	ADCSRA |= (1<<ADSC); //start ADC conversion
	while ((ADCSRA & (1<<ADSC))); //wait for ADC conversion to complete (25 cycles of ADC clock - 25 us for 1 MHz ADC clock) - need to figure out what to do with this time - would interrupt be more efficient?	
	
	//uint16_t adc_read = ADCL;
	//adc_read = adc_read | (ADCH <<8);
	//DDRE |= (1<<SPI_SS); //setup SPI SS as output again
	//ADCSRA &= ~(1<<ADEN);
	//SPCR1 |= (1<<MSTR1); //setting SPI SS to input requires resetting SPI as master
	//SPCR1 = (1<<SPE1) | (1<<MSTR1) | (1<<SPI2X); //Start SPI as MASTER
	return ADC;	
}