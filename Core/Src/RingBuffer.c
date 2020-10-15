/*
 * RingBuffer.cpp
 *
 *  Created on: Oct 13, 2020
 *      Author: mikolaj
 */

#include <RingBuffer.h>
#define UART_BUF_SIZE 	50

volatile char uart_buff[UART_BUF_SIZE];
volatile circ_buffer_t uart_circ_buff = { uart_buff, 0, 0 };

uint8_t rng_buf_add(char byte){
	uint8_t temp_head = uart_circ_buff.head + 1;

	if (temp_head == UART_BUF_SIZE)	temp_head = 0;
	if (temp_head == uart_circ_buff.tail)	return 0;

	uart_circ_buff.buffer[temp_head] = byte;
	uart_circ_buff.head++;

	return 1;
};

uint8_t rng_buf_get(char *byte){
	if (uart_circ_buff.head == uart_circ_buff.tail) return 0;

	uart_circ_buff.tail++;
	if (uart_circ_buff.tail == UART_BUF_SIZE) uart_circ_buff.tail = 0;

	*byte = uart_circ_buff.buffer[uart_circ_buff.tail];
	return 1;
}

void rng_buf_get_buff(char* buffer){
	int i = 0;
	while(rng_buf_get(&buffer[i])){
		i++;
	}
}
