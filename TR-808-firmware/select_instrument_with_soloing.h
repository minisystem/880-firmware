/*
 * select_instrument_with_soloing.h
 * Open808 firmware ATMEGA328PB
 * minisystem
 * system80.net
 */
#ifndef SELECT_INSTRUMENT_WITH_SOLOING_H
#define SELECT_INSTRUMENT_WITH_SOLOING_H

#define MAX_INSTRUMENTS 17

struct SoloState {
	uint8_t currentInstrument;
	bool isSolo;
	uint8_t secondaryInstrument; // -1 indicates no secondary instrument
};

struct SoloState handleInstrumentTransition(struct SoloState state, uint8_t buttonPressed, bool isClearPressed);

#endif