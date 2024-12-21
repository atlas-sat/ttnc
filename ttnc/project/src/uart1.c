/*
 * uart1.c
 *
 * Created: 29/11/2024 2:46:52 pm
 *  Author: Yu Heng
 */ 

#include <uart.h>
#include <uart1.h>
#include <avr/io.h>
#include <avr/interrupt.h>

char rx1_buffer[RX1_BUFFER_SIZE];
uint8_t rx1_read_pos = 0;
uint8_t rx1_write_pos = 0;

uint8_t buf[100] = {0};
int is_ready = 0;

#define FOSC 16000000UL// Clock Speed
#define BAUD 9600
#define MYUBRR (FOSC/16/BAUD-1)

void uart1_init (void) {
	// set baud rate
	UBRR1H = (unsigned char)(MYUBRR>>8);
	UBRR1L = (unsigned char) MYUBRR;
	
	// enable
	UCSR1B = (1<<RXEN1) | (1<<TXEN1) | (1 << RXCIE1);;	
	UCSR1C = (1<<UCSZ11) | (1<<UCSZ10);
}

void uart1_tx_char(char c) {
	// printf("Sending char: %c\r\n", c);
	loop_until_bit_is_set(UCSR1A, UDRE1);
	UDR1 = c;
}

void uart1_tx_string(char str[], int size) {
	
	// printf("size of tx string: %d\r\n");
	
	// for (int i = 0; i < sizeof(str)/sizeof(str[0]); i++) {
	for (int i = 0; i < size; i++) {
		uart1_tx_char(str[i]);
	}

	uart1_tx_char('\r');
	uart1_tx_char('\n');
}

ISR(USART1_RX_vect) {
	DDRB = 0b11111111;
	PORTB ^= (1 << PB7);
	char data = UDR1;  // Read received data
	rx1_buffer[rx1_write_pos++] = data;
	if (rx1_write_pos >= RX1_BUFFER_SIZE) {
		rx1_write_pos = 0;  // Wrap around
	}
}

char uart1_rx_char(void) {
	char ret = '\n';
	if (rx1_read_pos != rx1_write_pos) {
		ret = rx1_buffer[rx1_read_pos++];
		
		if (rx1_read_pos >= RX1_BUFFER_SIZE) {
			rx1_read_pos = 0;	
		}
	}
	
	return ret;
}

const uint8_t* uart1_rx_string(void) {
	int cntr = 0;
	
	while (1) {
		uint8_t a;
		a = uart1_rx_char();
		// printf("%02x, ",(unsigned int) ((char*)&a));
		if (a == '\r') {			
			is_ready = 1;
			break;
		}
		else if (a == 10 || a == '\r' || a == 13) {
			is_ready = 0;
			break;
		}
		else {
			// LHM 07Dec2024: Add placeholder to identify source of logs
			if(cntr == 0)
			{
				printf("UART1 Log:\r\n");
			}
			
			// LHM 07Dec2024: I suspect this line is necessary to prevent the compiler optimization
			// from removing it, but if you compile without pptimization, the code won't work either.
			// So we will live with this.
			printf("%02x [%d], ", (unsigned int)((char*)a), cntr);
			
			// Copy out into the buffer
			buf[cntr] = a;
			cntr++;
			is_ready = 0;
		}
	}
	
	if(is_ready == 1)
	{
		// LHM 07Dec2024: Print a break for formatting
		printf("\r\n\r\n");
		
		#if 0 // debugging
		printf("uart: ");
		for(int i = 0; i < 100; i++)
		{
			printf("%d ", buf[i]);
		}
		printf("\r\n\r\n");
		#endif
	}
	
	return buf;
}

int uart1_rx_has_data(void) {
	// Check if read position is different from write position
	if (rx1_read_pos != rx1_write_pos) {
		return 1;  // There is data in the buffer
	}
	return 0;  // No data in the buffer
}