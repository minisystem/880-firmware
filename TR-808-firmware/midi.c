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
	uint16_t ubbr_value = 31; //16MHz/(16*31250 BAUD) - 1 - should be a constant you nitwit!
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
	
	if (velocity == 0) return; //ignore velocity 0 notes - they are an alternative to note off
	
	//if ((sequencer.clock_mode != MIDI_SLAVE) || sequencer.START) return; //at the moment, only allow MIDI triggering of notes in MIDI SLAVE mode. Might be possible to get MIDI to work with sequencer, but at the moment the sequencer and incoming MIDI notes are incompatibily
	
	//if ((sequencer.clock_mode != MIDI_MASTER) || sequencer.START) return;
	
	if (sequencer.START) return; //you don't be receiving no midi notes when you sequencing, yo. don't interrupt me, I'm busy.
	
	if (note < 16) { //TODO: implement MIDI learn function to dynamically map notes to drum hits, or maybe just have an offset that allows first note of 16 to be set
		
		trigger_drum(note, velocity);
		
	}

	
}

void note_off_event(MidiDevice * device, uint8_t status, uint8_t note, uint8_t velocity) {}
	
void program_change_event(MidiDevice * device, uint8_t channel, uint8_t program_num) {
	//filter MIDI channel
	if (sequencer.midi_channel != (channel & MIDI_CHANMASK)) return;
	
	uint8_t pattern_num = (program_num+16)%16;
	uint8_t bank_num = program_num/16; //16 patterns per bank
	if (bank_num != sequencer.pattern_bank) {
		sequencer.pattern_bank = sequencer.previous_bank = bank_num;
		flag.pattern_change = 1;
	}
	if (sequencer.mode == MANUAL_PLAY) {
		if (pattern_num < NUM_BANKS) { //first 12 pattern places are for main patterns
			sequencer.new_pattern = sequencer.previous_pattern = pattern_num;
			if (sequencer.new_pattern != sequencer.current_pattern) flag.pattern_change = 1;
						
			} else { //remaining 4 patterns places are for intro/fills
						
			sequencer.current_intro_fill = pattern_num;
						
		}
	
	} else {
		sequencer.new_pattern = sequencer.previous_pattern = pattern_num;
		
	}
}
	

void real_time_event(MidiDevice * device, uint8_t real_time_byte) {
	if (sequencer.clock_mode != MIDI_SLAVE) return; //ignore incoming MIDI clock if not in MIDI slave. duh.
	switch (real_time_byte) {
		
		case MIDI_CLOCK:	
			//this would be the place to implement a MIDI clock divider
			//if (++clock.tick_counter == clock.divider) {
				//clock.tick_counter = 0;
				//PINC |= (1<<SYNC_LED_R);
			//} BAH, force an interrupt here to increment clock.tick_counter?
			process_external_clock_event();
	
		break;
		
		case MIDI_START:
			sequencer.START = 1;
			process_start();

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