#ifndef MIDI_H
#define MIDI_H

#include "xnormidi-develop/midi_device.h"

extern uint8_t midi_note_number;
extern MidiDevice midi_device;

void note_on_event(MidiDevice * device, uint8_t status, uint8_t note, uint8_t velocity);
void note_off_event(MidiDevice * device, uint8_t status, uint8_t note, uint8_t velocity);
void setup_midi_usart(void);

#endif