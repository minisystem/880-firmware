#ifndef SPI_H
#define SPI_H

#define SPIF0 7 //weird ommission - 7th bit of SPSR0 is not defined but SPIF1 is...
#define SPI0_MISO PB4
#define SPI0_LATCH PB3
#define SPI0_SCK PB5
#define SPI0_SS PB2
#define SPE0 6
#define MSTR0 4

extern uint8_t spi_data[9];	//array to hold SPI data (trigger + LEDs) for sending

extern uint8_t spi_current_switch_data[5]; //for debouncing
extern uint8_t spi_previous_switch_data[5]; //for debouncing
extern uint8_t switch_states[5]; //holder for debounced switch states - need to toggle once read

extern uint8_t spi0_current_trigger_byte0;
extern uint8_t spi0_current_trigger_byte1;
extern uint8_t spi0_previous_trigger_byte0;
extern uint8_t spi0_previous_trigger_byte1;

uint8_t spi_shift_byte(uint8_t byte);
void spi_read_write(void);
void update_spi(void);
void read_switches(void);

//new spi functions for SPI0 trigger inputs
uint8_t spi0_shift_byte(void);
void spi0_read_triggers(void);

#endif