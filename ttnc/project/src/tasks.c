/*
 * tasks.c
 *
 * Created: 3/12/2024 4:49:08 pm
 *  Author: Yu Heng
 */ 

#include <tasks.h>
#include <protocol.h>
#include <avr/io.h>
#include <uart.h>

// FreeRTOS
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>

/*csp lib*/
#include <csp/csp.h>
#include <csp/interfaces/csp_if_i2c.h>
#include <csp/drivers/i2c.h>
#include <csp/csp_endian.h>
#include <csp/arch/csp_queue.h>

#include "project/include/packet.h"
#include "project/include/uart1.h"

TaskHandle_t I2C_task;

TC_packet command;

int calculate_checksum(unsigned char *bytes) {
	unsigned char calculated_checksum = 0x00;
	for (size_t i = 0; i < sizeof(TC_packet) - 2; i++) {// minus 2 to remove xor-ing empty checksum bytes
		calculated_checksum ^= bytes[i];
	}
	printf("calculated checksum: %d\r\n", calculated_checksum);
	return (int)calculated_checksum; // cast checksum to integer for comparison
}

void receive_from_ground(void *pvParameters) 
{
	csp_socket_t *socket;
	csp_conn_t *conn;
	csp_packet_t *packet;
	
	// hard code for testing for now
	TC_packet_init(&command);
	set_APID(&command.header, 255);
	set_application_data(&command, "TC08-01000000", strlen("TC08-01000000"));
	set_TC_checksum(&command);
	
	#if 1 // Debugging
	printf("BIT:\r\n");
	for(int i = 0; i < sizeof(command); i++)
	{
		printf("%02x,",(unsigned int) ((char*)&command)[i]); // get the bytes for a sample telecommand (temporary)
	}
	printf("\r\n");
	#endif
	
	while (1) 
	{
		//printf("starting here\r\n");
		uart1_rx_string();		
		if (!is_ready) {
			//printf("uart not ready\r\n");
			vTaskDelay(5);
			continue;
		}
		
		#if 0 //debugging
		for(int i = 0; i < sizeof(command.data.app_data); i++)
		{
			printf("%02x,",(unsigned int) ((char*)&command.data.app_data)[i]); // get the bytes for a sample telecommand (temporary)
		}
		printf("\r\n");
		#endif
		
		memcpy(&command, buf, sizeof(TC_packet));
		printf("sizeof(TC_packet): %d\r\n",  sizeof(TC_packet));
		printf("size: %d\r\n", sizeof(command));
		printf("strlen: %d\r\n", strlen(&(command.data.app_data)));
		printf("str: %s\r\n", command.data.app_data);
		printf("initial checksum: %d\r\n", command.data.checksum);
		
		if (calculate_checksum((unsigned char *)&command) != command.data.checksum) {
			printf("checksum wrong\r\n");
			continue; // ignore TC if checksum do not equal
		}
		else
		{
			// Log down that checksum is correct
			printf("Checksum correct, forwarding to OBC\r\n\r\n");
			
			conn = csp_connect(PROTOCOL_PRIO_NORM, PROTOCOL_OBC_ADDR, PROTOCOL_SUBSYS_PING_PORT, PROTOCOL_TIMEOUT_HM, 0);
			packet = csp_buffer_get(sizeof(csp_packet_t));
			memcpy(packet->data, &command, sizeof(TC_packet));
			packet->length = sizeof(TC_packet);
			
			printf("here\r\n");
			
			if(csp_send(conn, packet, PROTOCOL_TIMEOUT_HM) == 0)
			{
				printf("CSP send received tc failed");
			}
			csp_buffer_free(packet);
			csp_close(conn);			
			printf("here2\r\n");
		}
		printf("Task runing... \r\n");
		
		vTaskDelay(5);
	}
}

void send_to_ground() {
	
	csp_socket_t *socket;
	csp_conn_t *conn;
	csp_packet_t *packet;

	socket = csp_socket(0);
	csp_listen(socket, 1);
	csp_bind(socket, 8);
	
	while (1) {
		
		// LHM 07Dec2024: Changed timeout to 1ms, to prevent "blocking"
		conn = csp_accept(socket, PROTOCOL_TIMEOUT_HM);
		if (conn == NULL) {
			continue;
		}
		printf("Connection accepted\r\n");
		
		packet = csp_read(conn, PROTOCOL_TIMEOUT_HM);
		
		printf("Packet Read\r\n");
		
		char data_received[100];
		int size_received = (packet->length > sizeof(data_received))? sizeof(data_received): packet->length;
		printf("size of packet: %d\r\n", size_received);
		memcpy(data_received, packet->data, size_received);
		printf("Received OBC data: ");
		
		for(int i = 0; i < sizeof(data_received); i++)
		{
			printf("%02x,", data_received[i]); // get the bytes for a sample telecommand (temporary)
		}
		printf("\r\n\r\n");
		
		uart1_tx_string(data_received, size_received);
		
		TM_packet tm;
		memcpy(&tm, packet->data, sizeof(TM_packet));
		
		printf("apid: %d\r\n", tm.header.p_ID.APID);
		
		// Close the connection
		csp_buffer_free(packet);
		csp_close(conn);
	}
}

void scheduled_telemetry(void *pvParameters) {

	csp_conn_t *conn;
	csp_packet_t *packet;

	TC_packet telemetry_command;
	TC_packet_init(&telemetry_command);
	set_APID(&telemetry_command.header, 10);
	
	while (1) {
		printf("starting telemetry\r\n");
		conn = csp_connect(PROTOCOL_PRIO_NORM, PROTOCOL_OBC_ADDR, PROTOCOL_SUBSYS_PING_PORT, PROTOCOL_TIMEOUT_HM, 0);
		packet = csp_buffer_get(sizeof(csp_packet_t));
		memcpy(packet->data, &telemetry_command, sizeof(TC_packet));
		packet->length = sizeof(TC_packet);
		
		printf("here (tel)\r\n");
		
		if(csp_send(conn, packet, PROTOCOL_TIMEOUT_HM) == 0)
		{
			printf("CSP send received tc failed");
		}
		csp_buffer_free(packet);
		csp_close(conn);			
		printf("here2 (tel)\r\n");

		// change delay in protocol.h
		vTaskDelay(TELEMETRY_TIMEOUT);
	}
}