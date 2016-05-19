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
	
	//if (sequencer.mode == MANUAL_PLAY) { 
		//spi_data[drum_hit[current_drum_hit].spi_byte_num] &= ~(drum_hit[current_drum_hit].trig_bit);
	//} else {
		//
		//spi_data[8] = 0;
	//}
	////toggle(drum_hit[current_drum_hit].led_index);
	////toggle(ACCENT_1_LED);
	//update_spi(); //should set flag here and update SPI from main loop. SPI should take about 10 microseconds
	sequencer.trigger_finished = 1;
	
}

ISR (TIMER1_COMPA_vect) { //output compare match for internal clock
	
	
	if (sequencer.START) { 
		
		if (internal_clock.ppqn_counter == internal_clock.divider >> 1) { //50% step width, sort of
			
			spi_data[1] = sequencer.step_led_mask[sequencer.current_inst];
			spi_data[0] = sequencer.step_led_mask[sequencer.current_inst] >> 8;
		}

	} else if ((internal_clock.beat_counter == 2) && (internal_clock.divider >> 1)) { //1/4 note, regardless of scale (based on original 808 behavior) - don't take this as gospel. may need to adjust with different pre-scales
			spi_data[1] = 0;
			spi_data[0] = 0;
		
	}
	
	if (++internal_clock.ppqn_counter == internal_clock.divider)
	{
		sequencer.next_step_flag = 1;
		internal_clock.beat_counter++;
		internal_clock.ppqn_counter = 0;
		sequencer.current_step++; //hopefully this will overflow from 15 to 0 - it does!
	}
	
}