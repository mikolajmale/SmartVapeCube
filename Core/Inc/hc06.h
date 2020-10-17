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

enum _config_error
{
    E_SUCCESS = 0,
    E_RESPONSE_TIMEOUT = -1,
    E_UNKNOWN_RESPONSE = -2,
	E_INVALID_ARGS = -3,
};

/* type to provide in your API */
typedef enum _config_error error_t;

typedef struct errordesc_t {
    int  code;
    char *message;
}errordesc_t;

#define HC06_RX_BUFFER_LENGTH 		40 //maximum number of characters to hold in the receive buffer

extern volatile char HC06_rx_buffer[HC06_RX_BUFFER_LENGTH];	//used by the IRQ handler
extern volatile uint8_t HC06_rx_counter; //used by the IRQ handler
extern volatile char HC06_msg[HC06_RX_BUFFER_LENGTH]; //variable that contains the latest string received on the RX pin
extern volatile uint8_t new_HC06_msg; //flag variable to indicate if there is a new message to be serviced

void HC06_Init(UART_HandleTypeDef *huart);
error_t HC06_PutChar(char *c);
error_t HC06_PutStr(char *str);
void HC06_ClearRxBuffer(void);
error_t HC06_Test(void);
uint8_t HC06_SetBaud(uint32_t speed);
error_t HC06_SetName(char *name);
error_t HC06_SetPin(char *pin);

#endif /* INC_HC06_H_ */
