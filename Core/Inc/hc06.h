/*
 * hc06.h
 *
 *  Created on: Oct 16, 2020
 *      Author: mikolaj
 */

#ifndef INC_HC06_H_
#define INC_HC06_H_

#include "stdint.h"
#include "main.h"

#define HC06_RX_BUFFER_LENGTH 		40 //maximum number of characters to hold in the receive buffer

extern volatile char HC06_rx_buffer[HC06_RX_BUFFER_LENGTH];	//used by the IRQ handler
extern volatile uint8_t HC06_rx_counter; //used by the IRQ handler
extern volatile char HC06_msg[HC06_RX_BUFFER_LENGTH]; //variable that contains the latest string received on the RX pin
extern volatile uint8_t new_HC06_msg; //flag variable to indicate if there is a new message to be serviced

void HC06_Init(void);
void HC06_PutChar(char *c);
void HC06_PutStr(char *str);
void HC06_ClearRxBuffer(void);
uint8_t HC06_Test(void);
uint8_t HC06_SetBaud(uint32_t speed);
uint8_t HC06_SetName(char *name);
uint8_t HC06_SetPin(char *pin);

#endif /* INC_HC06_H_ */
