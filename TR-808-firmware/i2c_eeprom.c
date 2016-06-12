/*
 * i2c_eeprom.c
 * JR-808 firmware ATMEGA328PB
 * minisystem
 * system79.com
 */

#include <stdio.h>
#include <avr/io.h>
#include <util/twi.h>
#include "i2c_eeprom.h"


void i2c_Setup(void) {
	
	TWBR = (uint8_t)I2C_BIT_RATE;
	//TWSR &= ~(1<<TWPS1 | 1<<TWPS0);
	
}

uint8_t i2c_Start(uint8_t address) {

	// reset TWI control register
	TWCR = 0;
	// transmit START condition
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	// wait for end of transmission
	while( !(TWCR0 & (1<<TWINT)) );
	
	// check if the start condition was successfully transmitted
	if((TWSR & 0xF8) != TW_START){ return 1; }
	
	// load slave address into data register
	TWDR = address;
	// start transmission of address
	TWCR = (1<<TWINT) | (1<<TWEN);
	// wait for end of transmission
	while( !(TWCR & (1<<TWINT)) );
	
	// check if the device has acknowledged the READ / WRITE mode
	uint8_t twst = TW_STATUS & 0xF8;
	if ( (twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK) ) return 1;
	
	return 0;

}




void i2c_Stop(void){
	
	// transmit STOP condition
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	
}

uint8_t i2c_Read_ACK(void) {
	
	// start TWI module and acknowledge data after reception
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	// wait for end of transmission
	while( !(TWCR & (1<<TWINT)) );
	// return received data from TWDR
	return TWDR;
	
}

uint8_t i2c_Read_NACK(void) {

	// start receiving without acknowledging reception
	TWCR = (1<<TWINT) | (1<<TWEN);
	// wait for end of transmission
	while( !(TWCR & (1<<TWINT)) );
	// return received data from TWDR
	return TWDR;

}


uint8_t i2c_Write(uint8_t data) {
	
	// load data into data register
	TWDR = data;
	// start transmission of data
	TWCR = (1<<TWINT) | (1<<TWEN);
	// wait for end of transmission
	while( !(TWCR & (1<<TWINT)) );
	
	if( (TWSR & 0xF8) != TW_MT_DATA_ACK ){ return 1; }
	
	return 0;

}



uint8_t i2c_Transmit(uint8_t* data, uint16_t length) {
	
	if (i2c_Start(I2C_READ)) return 1;
	
	for (uint16_t i = 0; i < (length-1); i++)
	{
		data[i] = i2c_Read_ACK();
	}
	data[(length-1)] = i2c_Read_NACK();
	
	i2c_Stop();
	
	return 0;

}

uint8_t i2c_Receive(uint8_t* data, uint16_t length) {

	if (i2c_Start(I2C_READ)) return 1;
	
	for (uint16_t i = 0; i < (length-1); i++)
	{
		data[i] = i2c_Read_ACK();
	}
	data[(length-1)] = i2c_Read_NACK();
	
	i2c_Stop();
	
	return 0;

}
