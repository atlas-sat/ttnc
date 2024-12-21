/*
 * uart2.h
 *
 * Created: 3/11/2024 10:03:46 pm
 *  Author: huimin
 */ 


#ifndef UART2_H_
#define UART2_H_

#include <avr/io.h>
#include <string.h>
#include <avr/sfr_defs.h>			// For loop_until_bit_is_set()
#include <avr/interrupt.h>

#define FF_CPU2						16000000UL
#define BAUD2						9600
#define BRC2						((FF_CPU2/16/BAUD2) - 1)

void uart2_init(void);
void uart2_tx_char(char c);
void uart2_tx_string(char str[]);

#define RX2_BUFFER_SIZE 128

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

char uart2_rx_char(void);
char uart2_peek_char(void);
const uint8_t *uart2_rx_string(void);

#if 0
uint8_t uart2_rx_telecommand(uint8_t byte_array[], uint8_t arr_len) ;
#endif

#endif /* UART2_H_ */