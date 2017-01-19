/*
 * midi.h
 * Open808 firmware ATMEGA328PB
 * minisystem
 * system80.net
 */

#ifndef MIDI_H
#define MIDI_H

#include <inttypes.h>
#include "xnormidi-develop/midi_device.h"

#define MIDI_OUTPUT_QUEUE_LENGTH 192 //does this need to be so large?

extern uint8_t midi_note_number;
extern MidiDevice midi_device;
extern byteQueue_t midi_byte_queue;

void note_on_event(MidiDevice * device, uint8_t status, uint8_t note, uint8_t velocity);
void note_off_event(MidiDevice * device, uint8_t status, uint8_t note, uint8_t velocity);
void real_time_event(MidiDevice * device, uint8_t real_time_byte);
void setup_midi_usart(void);

void midi_send(MidiDevice * device, uint16_t cnt, uint8_t inByte0, uint8_t inByte1, uint8_t inByte2);

#endif