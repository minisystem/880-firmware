/*
 * adc.h
 * JR-808 firmware ATMEGA328PB
 * minisystem
 * system79.com
 */



#ifndef ADC_H
#define ADC_H

#define ADC_MAX 1023 //max value of ADC output

void setup_adc(void);

uint16_t read_tempo_pot();



#endif 