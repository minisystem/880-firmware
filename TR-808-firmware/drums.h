#ifndef DRUMS_H
#define DRUMS_H

#define NO_SWITCH 255 //null value for drums that don't need any switching done
#define CONGAS_OFF 0b00000111 //mask for turning off congas

enum drum { //index for drum hits
	
	BD,
	SD,
	LT,
	MT,
	HT,
	RS,
	CP,
	CB,
	CY,
	OH,
	CH,
	LC,
	MC,
	HC,
	CL,
	MA,
	AC
		
};

struct drum_hit {

	uint8_t note_num:7; //assigned MIDI note number
	uint8_t spi_byte_num:4; //SPI byte number
	uint8_t trig_bit; //trigger bit for drum hit
	uint8_t switch_bit; //bit for switching toms/congas and rs/cl - will be -1 for all other instruments
	uint8_t switch_value:1; //is the switch one or 0?
	uint8_t led_index; //index for instrument LED
	uint8_t muted:1;
	uint8_t solo:1;
};

extern struct drum_hit drum_hit[17];

extern enum drum midi_note_queue[16]; //queue to hold incoming MIDI notes

extern uint8_t current_drum_hit; //global to hold index of current drum in MIDI note queue
//volatile uint8_t trigger_finished;

extern uint8_t previous_assign_port;
extern uint8_t current_assign_port;

void trigger_drum(uint8_t note, uint8_t velocity);
void trigger_step(uint8_t part_playing);
//void trigger_substep(void);
void clear_all_trigs(void);
void trigger_roll(void);
void live_hits(void);

void process_external_triggers(void);

#endif