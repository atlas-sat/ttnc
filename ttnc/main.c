#define F_CPU 16000000L // Specify oscillator frequency

#include <protocol.h>
#include <tasks.h>
#include <uart.h>
#include <uart2.h>
#include <avr/interrupt.h>

// FreeRTOS
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>

// CSP Libs
#include <csp/csp.h>
#include <csp/interfaces/csp_if_i2c.h>
#include <csp/drivers/i2c.h>

extern TaskHandle_t I2C_task;

int main(void)
{
	// Initialize UART
	// Baud Rate: 57600
	uart_init();
	
	// Initialise UART
	// Baud Rate: 9600
	uart1_init();
	
	// Initialize UART2
	// Baud Rate: 9600
	uart2_init();
	sei(); // Turn on external interrupts
	
	printf("node\r\n");
	
	// CSP-related settings
	csp_buffer_init(2, 300); // Creating 2 buffers of size 300 bytes each
	csp_init(PROTOCOL_TTNC_ADDR); // Initialize to node's address
	csp_i2c_init(PROTOCOL_TTNC_ADDR, 0, 400); // Setting the I2C node to node's address and speeds
	csp_route_set(PROTOCOL_OBC_ADDR, &csp_if_i2c, CSP_NODE_MAC);
	csp_route_start_task(500, 1); // Start the router task in CSP
	csp_rtable_print();
	printf("\r\n");
	
	// LHM 07Dec2024: Higher FreeRTOS priorities are higher priorities
	// Put higher to prevent misses from the Antenna
	extern void receive_from_ground(void *pvParameters);
	xTaskCreate(receive_from_ground, "ABC", 200, NULL, 4, NULL);
	
	extern void send_to_ground(void *pvParameters);
	xTaskCreate(send_to_ground, "ABC", 250, NULL, 4, NULL);
	
	extern void scheduled_telemetry(void *pvParameters);
	xTaskCreate(scheduled_telemetry, "ABC", 150, NULL, 2, NULL);
	
	// Start Scheduler
	vTaskStartScheduler();
	
	/*--------------------------------*/
	printf("not enough\r\n");

	/* Execution will only reach here if there was insufficient heap to start the scheduler. */
	for ( ;; );
	
	return 0;
}
