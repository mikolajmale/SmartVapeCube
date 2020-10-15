/*
 * RingBuffer.h
 *
 *  Created on: Oct 13, 2020
 *      Author: mikolaj
 */

#ifndef SRC_RINGBUFFER_H_
#define SRC_RINGBUFFER_H_

#include "stdint.h"

typedef struct {
	volatile char * const buffer;
	uint8_t head;
	uint8_t tail;
} circ_buffer_t;

extern volatile circ_buffer_t uart_circ_buff;

uint8_t rng_buf_add(char byte);
uint8_t rng_buf_get(char* byte);


#endif /* SRC_RINGBUFFER_H_ */
