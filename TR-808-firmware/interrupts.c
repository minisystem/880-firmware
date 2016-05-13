/*
 * interrupts.c
 *JR-808 firmware ATMEGA328PB
 *minisystem
 *system79.com
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "drums.h"
#include "spi.h"

ISR (TIMER0_COMPA_vect) {
	
	TCCR0B = 0; //turn off timer
	TIMSK0 &= ~(1<<OCIE0A); //turn off output compare 
	
	spi_data[drum_hit[current_drum_hit].spi_byte_num] &= ~(drum_hit[current_drum_hit].trig_bit);
	//spi_data[drum_hit[note].spi_led_byte_num] &= ~(drum_hit[note].led_bit);
	spi_data[8] &= ~(1<<ACCENT);
	update_spi(); //should set flag here and update SPI from main loop. SPI should take about 10 microseconds
	
}