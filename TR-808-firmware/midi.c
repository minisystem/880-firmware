#include <avr/io.h>

#include "midi.h"
#include "hardware.h"
#include "clock.h"
#include "sequencer.h"

#include "xnormidi-develop/midi.h"
#include "xnormidi-develop/midi_device.h"
#include "xnormidi-develop/bytequeue/bytequeue.h" //this is required for MIDI sending

void setup_midi_usart(void)
{
	uint16_t ubbr_value = 31; //16MHz/(16*31250 BAUD) - 1
	//write ubbr_value to H and L UBBR1 registers:
	UBRR0L = (unsigned char) ubbr_value;
	UBRR0H = (unsigned char) (ubbr_value >> 8);
	
	UCSR0B = (1<<RXEN0)|(1<<TXEN0) | (1<<RXCIE0) | (1<<TXCIE0);
	DDRD |= (1<<PD1); //set PD1 and UART TX
	//UCSR0C |= (0<<UMSEL0)|(0<<UMSEL01)|(0<<UPM01)|(0<<UPM00)|(0<<USBS0)|(0<<UCSZ02)|(1<<UCSZ01)|(1<<UCSZ00);
}

void note_on_event(MidiDevice * device, uint8_t status, uint8_t note, uint8_t velocity) {
	

	if (note < 16) { //TODO: implement MIDI learn function to dynamically map notes to drum hits
		
		trigger_drum(note, velocity);
		
	}

	
}

void note_off_event(MidiDevice * device, uint8_t status, uint8_t note, uint8_t velocity) {}

void real_time_event(MidiDevice * device, uint8_t real_time_byte) {
	if (clock.source == INTERNAL) return; //ignore incoming MIDI clock
	switch (real_time_byte) {
		
		case MIDI_CLOCK://could set tick flag here and process it in one function used by both MIDI, DIN and INTERNAL clocks?
		process_tick(); //flag.tick = 1;
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