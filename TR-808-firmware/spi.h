#ifndef SPI_H
#define SPI_H

extern uint8_t spi_data[9];	//array to hold SPI data (trigger + LEDs) for sending

extern uint8_t spi_current_switch_data[5]; //for debouncing
extern uint8_t spi_previous_switch_data[5]; //for debouncing
extern uint8_t switch_states[5]; //holder for debounced switch states - need to toggle once read

uint8_t spi_shift_byte(uint8_t byte);
void update_spi(void);
void read_switches(void);


#endif