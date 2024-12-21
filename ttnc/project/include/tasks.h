/*
 * tasks.h
 *
 * Created: 18/8/2024 5:03:59 pm
 *  Author: huimin
 */ 

#ifndef BLINKY_TASK_H_
#define BLINKY_TASK_H_

// This task receive messages from Hc-12 and print out
void task_message_receiver (void* pvParameters);

// This task lights up LED at digital pin 13 (built-in)
void task_led_blinky (void* pvParameters);

void csp_server (void* pvParameters);
void csp_twoway_server (void* pvParameters);

#endif /* BLINKY_TASK_H_ */