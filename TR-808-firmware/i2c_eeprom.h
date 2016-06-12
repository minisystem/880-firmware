/*
 * i2c_eeprom.h
 * JR-808 firmware ATMEGA328PB
 * minisystem
 * system79.com
 */


#ifndef I2C_EEPROM_H
#define I2C_EEPROM_H

#define TWBR TWBR0
#define TWSR TWSR0
#define TWCR TWCR0
#define TWDR TWDR0

#define I2C_ADDRESS		0b10100000//A2 = A1 = A0 = 0 - all address lines pulled low
#define I2C_READ		0b10100001
#define I2C_WRITE		0b10100000
#define PRESCALER 1
#define F_CPU 16000000UL //not defined anywhere else. Probaby should be so that more generic timer calculations can be used?
#define F_SCL 400000UL //SCL Frequency - start with 400 KHz and then try 1 MHz (ie. TWBR = 0)
#define I2C_BIT_RATE ((((F_CPU / F_SCL) / PRESCALER) - 16 ) / 2)

void i2c_Setup(void);

uint8_t i2c_Start(uint8_t address);

void i2c_Stop(void);

uint8_t i2c_Read_ACK(void);

uint8_t i2c_Read_NACK(void);

uint8_t i2c_Write(uint8_t data);

uint8_t i2c_Transmit(uint8_t* data, uint16_t length);
uint8_t i2c_Receive(uint8_t* data, uint16_t length);

#endif