/*
 * uart2.c
 *
 * Created: 3/11/2024 10:03:53 pm
 *  Author: huimin
 */ 

#include <uart.h>
#include <uart2.h>

char rx2_buffer[RX2_BUFFER_SIZE];
uint8_t rx2_read_pos = 0;
uint8_t rx2_write_pos = 0;

int flag_string_done = 0;

void uart2_init(void) {
	UBRR2H = (BRC2 >> 8);
	UBRR2L = BRC2;

	UCSR2B = (1 << TXEN2) | (1 << RXEN2) | (1 << RXCIE2);

	UCSR2C = (1 << UCSZ20) | (1 << UCSZ21);
}

void uart2_tx_char(char c) {
	loop_until_bit_is_set(UCSR2A, UDRE2);
	UDR2 = c;
}

void uart2_tx_string(char str[]) {
	for (int i = 0; i < strlen(str); i++) {
		uart2_tx_char(str[i]);
	}

	uart2_tx_char('\r');
	uart2_tx_char('\n');
}

ISR(USART2_RX_vect) {
	char c = UDR2;
	//printf("char is %c\r\n", c);

	// LF read
	if (c == 10) {
		flag_string_done = 1;
	}

	rx2_buffer[rx2_write_pos] = c;
	rx2_write_pos++;

	// Buffer may overflow
	if (rx2_write_pos >= RX2_BUFFER_SIZE) {
		rx2_write_pos = 0;
	}
}

char uart2_peek_char(void) {
	char ret = '\0';

	if (rx2_read_pos != rx2_write_pos) {
		ret = rx2_buffer[rx2_read_pos];
	}

	return ret;
}

char uart2_rx_char(void) {
	char ret = '\0';

	if (rx2_read_pos != rx2_write_pos) {
		ret = rx2_buffer[rx2_read_pos++];

		if (rx2_read_pos >= RX2_BUFFER_SIZE) {
			rx2_read_pos = 0;
		}
	}

	return ret;
}


const uint8_t *uart2_rx_string(void) {
	uint8_t str [100]; 
	int str_pos = 0;

	static uint8_t buf[100]; // Declare static variable to be transfered

	while (flag_string_done) {
		uint8_t c = uart2_rx_char();
		printf("Char is %d\r\n", c);

		// NOT CR
		if (c != 13 && c != 10 && c != 0) {
			str[str_pos++] = c;
		}
		else if (c == 13 || c == '\r' || c == 10) {
			str[str_pos] = '\0';
			uint8_t test = uart2_rx_char(); // Read the LF

			if (test == 10) {
				//printf("LF is read\r\n");
			}

			//printf("UART2 received %s\r\n\r\n", str);

			flag_string_done = 0;
			str_pos = 0;
			
			sprintf(buf, "%s", str);
			return buf;
		}		
		else if (c == 10) {
			// ignore
		}
	}

	return "random";
}

#if 0
// NOTE: telecommands are encoded in utf8
// Returns 1 if received a telecommand, 0 if did not receive a telecommand
uint8_t uart2_rx_telecommand(uint8_t arr[], uint8_t arr_len) {
	uint8_t num_char = 0;
	uint8_t start_flag = 0;

	while (1) {
		uint8_t c = uart2_rx_char();

		if (c == TELECOMMAND_PACKET_START_CHAR && start_flag == 0) {
			//printf("Start char -- %d\r\n", c);

			// Signal start of telecommand
			while (num_char < TELECOMMAND_PACKET_LEN_BYTES) {
				uint8_t c = uart2_rx_char();
				//printf("Char rx is %d\r\n", c);
				arr[num_char++] = c;
			}

			printf("Rx cmd\r\n");
			return TELECOMMAND_RX;
		}
		else {
			break;
		}
	}

	return TELECOMMAND_NOT_RX;
}
#endif
