#include <stdio.h>
#include <stdbool.h>
#include "select_instrument_with_soloing.h"
#include "switches.h"
#include "drums.h"

#define NUM_INSTRUMENT_BUTTONS 12


bool isUnswitchedInstrument(uint8_t instrument) {
	//return (instrument >= 1 && instrument <= 3) || (instrument >= 9 && instrument <= 12);
	return (instrument == BD || instrument == SD) || (instrument >= CB && instrument <= CH);
}


struct SoloState handleInstrumentTransition(struct SoloState state, uint8_t buttonPressed, bool isClearPressed) {
	//printf("Button press %d. Shift is %d\n", buttonPressed,isClearPressed);
	// first, handle the situation where we are in a solo mode and there's a secondary instrument
	// TODO: ONLY PROBLEM IS WHEN IN SWITCHED SOLO GOING TO OTHER PART OF SWITCH -- Jeff thinks this is ok
	if (state.secondaryInstrument != EMPTY) {
		if (isUnswitchedInstrument(state.secondaryInstrument)) {
			if(isClearPressed) {
				state.currentInstrument = buttonPressed;
				state.isSolo = true;
				state.secondaryInstrument = EMPTY;
				} else {
				if (buttonPressed == state.currentInstrument || (buttonPressed + SW_DRUM_OFFSET) == state.currentInstrument) {
					state.isSolo = false;
					state.secondaryInstrument = EMPTY;
					state.currentInstrument = state.currentInstrument;
					} else {
					state.secondaryInstrument = buttonPressed;
					state.currentInstrument = state.currentInstrument;
				}
			}
			} else {
			// ok, so it's a switched secondary instrument.
			if(isClearPressed) {
				if (buttonPressed == state.secondaryInstrument || (buttonPressed + SW_DRUM_OFFSET) == state.secondaryInstrument) {
					state.isSolo = true;
					state.currentInstrument = (state.secondaryInstrument + SW_DRUM_OFFSET) % (SW_DRUM_OFFSET*2);
					state.secondaryInstrument = EMPTY;
					} else {
					state.isSolo = true;
					state.currentInstrument = buttonPressed;
					state.secondaryInstrument = EMPTY;
				}
				} else {
				//
				if (buttonPressed == state.currentInstrument || (buttonPressed + SW_DRUM_OFFSET) == state.currentInstrument) {
					state.isSolo = false;
					state.secondaryInstrument = EMPTY;
					state.currentInstrument = state.currentInstrument;
					} else if (buttonPressed == state.secondaryInstrument || (buttonPressed + SW_DRUM_OFFSET) == state.secondaryInstrument) {
					state.secondaryInstrument = (state.secondaryInstrument + SW_DRUM_OFFSET) % (SW_DRUM_OFFSET*2);
					
					} else {
					state.secondaryInstrument = buttonPressed;
				}
			}
			
		}
		} else {
		
		if (isUnswitchedInstrument(state.currentInstrument)) {
			// Handle first set of instruments
			if (!state.isSolo) {
				if (isClearPressed) {
					state.currentInstrument = buttonPressed;
					state.isSolo = true;
					} else {
					state.currentInstrument = buttonPressed;
				}
				} else {
				if (isClearPressed) {
					state.currentInstrument = buttonPressed;
					state.isSolo = true;
					} else {
					if (state.currentInstrument == buttonPressed) {
						state.isSolo = false;
						state.secondaryInstrument = EMPTY;
						} else {
						state.secondaryInstrument = buttonPressed;
					}
				}
			}
			// now handle the switched buttons
			} else {
			if (!state.isSolo) {
				if (buttonPressed == state.currentInstrument || (buttonPressed + SW_DRUM_OFFSET) == state.currentInstrument) {
					if (!isClearPressed) {
						state.currentInstrument = (state.currentInstrument + SW_DRUM_OFFSET) % (SW_DRUM_OFFSET*2);
						} else {
						state.isSolo = true;
					}
					} else {
					if (!isClearPressed) {
						state.currentInstrument = buttonPressed;
						} else {
						state.currentInstrument = buttonPressed;
						state.isSolo = true;
					}
				}
				// is solo state on the switched buttons
				} else {
				if (buttonPressed == state.currentInstrument || (buttonPressed + SW_DRUM_OFFSET) == state.currentInstrument) {
					if (!isClearPressed) {
						state.secondaryInstrument = (state.currentInstrument + SW_DRUM_OFFSET) % (SW_DRUM_OFFSET*2);
						} else {
						state.currentInstrument = (state.currentInstrument + SW_DRUM_OFFSET) % (SW_DRUM_OFFSET*2);
					}
					} else {
					if (!isClearPressed) {
						state.secondaryInstrument = buttonPressed;
						} else {
						state.currentInstrument = buttonPressed;
						state.isSolo = true;
					}
				}
			}
		}
	}
	//printf("State: %d %d %d\n", state.currentInstrument, state.isSolo, state.secondaryInstrument);
	return state;
}

/*
int main() {
	// Test the FSM transitions (same as before)
	struct SoloState state;
	state.currentInstrument = 1;
	state.isSolo = false;
	state.secondaryInstrument = -1; // -1 indicates no secondary instrument

	state = handleInstrumentTransition(state, 3, true);
	state = handleInstrumentTransition(state, 4, false);
	state = handleInstrumentTransition(state, 4, false);
	state = handleInstrumentTransition(state, 4, false);
	state = handleInstrumentTransition(state, 3, false);

	// now reset to a switched solo
	state = handleInstrumentTransition(state, 5, true);
	state = handleInstrumentTransition(state, 5, false);
	state = handleInstrumentTransition(state, 5, false);
	state = handleInstrumentTransition(state, 5, true);
	state = handleInstrumentTransition(state, 5, true);
	state = handleInstrumentTransition(state, 5, false);
	
	// now reset to a switched solo
	state = handleInstrumentTransition(state, 2, true);
	state = handleInstrumentTransition(state, 3, false);
	state = handleInstrumentTransition(state, 4, false);
	state = handleInstrumentTransition(state, 5, true);
	state = handleInstrumentTransition(state, 5, true);
	state = handleInstrumentTransition(state, 5, false);
}*/