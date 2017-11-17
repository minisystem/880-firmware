/*
 * midi.c
 * Open808 firmware ATMEGA328PB
 * minisystem
 * system80.net
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "midi.h"
#include "hardware.h"
#include "clock.h"
#include "sequencer.h"

#include "xnormidi-develop/midi.h"
#include "xnormidi-develop/midi_device.h"
#include "xnormidi-develop/bytequeue/bytequeue.h" //this is required for MIDI sending

byteQueue_t midi_byte_queue;
uint8_t midi_output_queue_data[MIDI_OUTPUT_QUEUE_LENGTH];

void setup_midi_usart(void)
{
	uint16_t ubbr_value = 31; //16MHz/(16*31250 BAUD) - 1
	//write ubbr_value to H and L UBBR1 registers:
	UBRR0L = (unsigned char) ubbr_value;
	UBRR0H = (unsigned char) (ubbr_value >> 8);
	
	UCSR0B = (1<<RXEN0)|(1<<TXEN0) | (1<<RXCIE0);// | (1<<TXCIE0);
	DDRD |= (1<<PD1); //set PD1 and UART TX
	//UCSR0C |= (0<<UMSEL0)|(0<<UMSEL01)|(0<<UPM01)|(0<<UPM00)|(0<<USBS0)|(0<<UCSZ02)|(1<<UCSZ01)|(1<<UCSZ00);
	
	bytequeue_init(&midi_byte_queue, midi_output_queue_data, MIDI_OUTPUT_QUEUE_LENGTH);
	midi_device_set_send_func(&midi_device, midi_send);
}

void note_on_event(MidiDevice * device, uint8_t channel, uint8_t note, uint8_t velocity) {
	
	//filter MIDI channel
	if (sequencer.midi_channel != (channel & MIDI_CHANMASK)) return;
	
	if ((sequencer.clock_mode != MIDI_SLAVE) || sequencer.START) return; //at the moment, only allow MIDI triggering of notes in MIDI SLAVE mode. Might be possible to get MIDI to work with sequencer, but at the moment the sequencer and incoming MIDI notes are incompatibily
	
	if (note < 16) { //TODO: implement MIDI learn function to dynamically map notes to drum hits, or maybe just have an offset that allows first note of 16 to be set
		
		trigger_drum(note, velocity);
		
	}

	
}

void note_off_event(MidiDevice * device, uint8_t status, uint8_t note, uint8_t velocity) {}

void real_time_event(MidiDevice * device, uint8_t real_time_byte) {
	if (clock.source == INTERNAL) return; //ignore incoming MIDI clock
	switch (real_time_byte) {
		
		case MIDI_CLOCK://could set tick flag here and process it in one function used by both MIDI, DIN and INTERNAL clocks?
		TRIGGER_OUT ^= (1<<TRIGGER_OUT_1);
		//clock.ppqn_counter+= PPQN_SKIP_VALUE;
		//process_tick(); //flag.tick = 1;
		//clock.previous_external_rate = clock.external_rate; //maybe don't need previous_external_rate? Not using it here
		//BUT, what about on first pulse? there is no reference before the first pulse, so TCNT could by anywhere in its cycle? But a start event should reset TCNT1 and TCNT3 maybe?
		flag.wait_for_master_tick = 0;
		clock.slave_ppqn_ticks = 0;
		clock.external_rate = TCNT3; //need to handle overflow, probably in Timer3 overflow interrupt
		//OCR1A = TCNT1 - 1; //force output compare match - need to disable interrupts before doing this?
		flag.slave_wait = 1;
		while (flag.slave_wait == 1);
		
		if (flag.slave_start) { //don't update clock if it's the first pulse
			flag.slave_start = 0;
			//process_tick();
		} else {
			update_clock_rate(clock.external_rate);
			
			
			//TIMSK1 |= (1<<OCIE1A); //turn OCR1A interrupt on 
			
		}
		
		
		TCNT3 = 0; //reset timer3
		//TCNT1 = 0; //reset timer1 - RESETTING TIMER1 HERE CAUSES SLOWER TEMPO (BY 3/4 OF MASTER TEMPO) - WHAT ELSE NEEDS TO BE SET HERE? - NOPE -can't do this here. setting TCTN1 disables next compare match you nitwit
		TRIGGER_OUT |= (1<<TRIGGER_OUT_2);
		process_tick();
		break;
		
		case MIDI_START:
		sequencer.START = 1;
		process_start();
		//process_tick();
		//TCNT3 = 0;
		break;
		
		case MIDI_STOP:
		sequencer.START = 0;
		process_stop();
		break;
		
		
	}
	
	
}

void midi_send(MidiDevice * device, uint16_t cnt, uint8_t inByte0, uint8_t inByte1, uint8_t inByte2) {
	
	  // enqueue into buffer & start interrupt
	  bytequeue_enqueue(&midi_byte_queue, inByte0);
	  if(cnt > 1)
	  bytequeue_enqueue(&midi_byte_queue, inByte1);
	  if(cnt == 3)
	  bytequeue_enqueue(&midi_byte_queue, inByte2);

	  // then turn on data transmit buffer interrupt
	  UCSR0B |= (1 << UDRIE0);
	
}