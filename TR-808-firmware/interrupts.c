/*
 * interrupts.c
 * JR-808 firmware ATMEGA328PB
 * minisystem
 * system79.com
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "drums.h"
#include "spi.h"
#include "clock.h"
#include "sequencer.h"

ISR (TIMER0_COMPA_vect) {
	
	TCCR0B = 0; //turn off timer
	TIMSK0 &= ~(1<<OCIE0A); //turn off output compare 
	
	spi_data[drum_hit[current_drum_hit].spi_byte_num] &= ~(drum_hit[current_drum_hit].trig_bit);
	//toggle(drum_hit[current_drum_hit].led_index);
	//toggle(ACCENT_1_LED);
	update_spi(); //should set flag here and update SPI from main loop. SPI should take about 10 microseconds
	trigger_finished = 1;
	
}

ISR (TIMER1_COMPA_vect) { //output compare match for internal clock
	
	
	if (sequencer.START) { 
		
		if (internal_clock.ppqn_counter == internal_clock.divider >> 1) { //50% step width
			spi_data[1] = 0;
			spi_data[0] = 0;
			spi_data[1] |= sequencer.step_led_mask;
			spi_data[0] |= sequencer.step_led_mask >> 8;
		}

	} else if ((internal_clock.beat_counter == 2) && (internal_clock.divider >> 1)) { //1/4 note, regardless of scale (based on original 808 behavior) - don't take this as gospel. may need to adjust with different pre-scales
			spi_data[1] = 0;
			spi_data[0] = 0;
		
	}
	
	if (++internal_clock.ppqn_counter == internal_clock.divider) {
		
		internal_clock.beat_counter++;
		internal_clock.ppqn_counter = 0;
		sequencer.current_step++; //hopefully this will overflow from 15 to 0 - it does!
		
		if (sequencer.START) {
			spi_data[1] = (1 << sequencer.current_step) | sequencer.step_led_mask;// | sequencer.current_pattern.first_part[sequencer.current_inst];
			spi_data[0] = ((1 << sequencer.current_step) >> 8) | (sequencer.step_led_mask >> 8);// | (sequencer.current_pattern.first_part[sequencer.current_inst] >> 8);
			
		
		//now load current step's drum data into corresponding spi_data bytes:
		} else {
		
			spi_data[1] = 0;
			spi_data[0] = 0;
			turn_on(STEP_1_LED);	
			
		}
		
		
		

	}
	
}