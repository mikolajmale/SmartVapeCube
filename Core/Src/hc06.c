#include <string.h>
#include "hc06.h"

extern UART_HandleTypeDef huart3;

errordesc_t errordesc[] = {
    { E_SUCCESS, "No error" },
    { E_RESPONSE_TIMEOUT, "No response" },
    { E_UNKNOWN_RESPONSE, "Unknown response" },
	{ E_INVALID_ARGS, "Invalid given params" }
};

void print_err_msg(UART_HandleTypeDef *huart, error_t err, char *custom_msg){
	char msg[30];
	sprintf(msg, "HC06 (%s) : %s \n\r", custom_msg, errordesc[err].message);
	HAL_UART_Transmit(huart, msg, strlen(msg), 100);
};


volatile char HC06_rx_buffer[HC06_RX_BUFFER_LENGTH];	//used by the IRQ handler
volatile uint8_t HC06_rx_counter = 0; //used by the IRQ handler
volatile char HC06_msg[HC06_RX_BUFFER_LENGTH]; //variable that contains the latest string received on the RX pin
volatile uint8_t new_HC06_msg = 0; //flag variable to indicate if there is a new message to be serviced


void HC06_Init(UART_HandleTypeDef *huart)
{
	print_err_msg(huart, HC06_Test(), "test");
	print_err_msg(huart, HC06_SetName("SmartVape"), "set_name");
	print_err_msg(huart, HC06_SetPin("1234"), "set_pin");
	HC06_ClearRxBuffer();
}


error_t HC06_PutChar(char *byte)
{
	/* Put character on the serial line */
	if(HAL_UART_Transmit(&huart3, byte, 1, 1000) != HAL_OK) return E_RESPONSE_TIMEOUT;
	return E_SUCCESS;
}


error_t HC06_PutStr(char *str)
{
	for(int i = 0; str[i] != '\0'; i++){
		if(HC06_PutChar(&str[i]) == E_RESPONSE_TIMEOUT){
			return E_RESPONSE_TIMEOUT;
		}
	}
	return E_SUCCESS;
}


void HC06_ClearRxBuffer(void)
{
	memset(HC06_rx_buffer, 0, HC06_RX_BUFFER_LENGTH); //clear HC06_rx_buffer
	HC06_rx_counter = 0; //reset the Rx buffer counter
	new_HC06_msg = 0; //reset new message flag
}


error_t HC06_Test(void)
{
	uint8_t tries = 10;
	char at_command[2] = "AT";

	HC06_ClearRxBuffer(); //clear rx buffer
	HC06_PutStr(at_command);
	while(HC06_rx_counter < 2) //wait for "OK" - i.e. waiting for 2 characters
	{
		tries--;
		HAL_Delay(100);
		if (tries == 0) return E_RESPONSE_TIMEOUT;
	}
	if(strcmp(HC06_rx_buffer, "OK") == 0)
		return E_SUCCESS; //success
	else
		return E_UNKNOWN_RESPONSE;
}


uint8_t HC06_SetBaud(uint32_t speed)
{
	uint8_t timeout = 100;
	char buf[20];

	HC06_ClearRxBuffer(); //clear rx buffer
	//AT command for SET BAUD speed
	if(speed == 1200)
	{
		strcpy(buf, "OK1200");
		HC06_PutStr("AT+BAUD1");
	}
	else if(speed == 2400)
	{
		strcpy(buf, "OK2400");
		HC06_PutStr("AT+BAUD2");
	}
	else if(speed == 4800)
	{
		strcpy(buf, "OK4800");
		HC06_PutStr("AT+BAUD3");
	}
	else if(speed == 9600)
	{
		strcpy(buf, "OK9600");
		HC06_PutStr("AT+BAUD4");
	}
	else if(speed == 19200)
	{
		strcpy(buf, "OK19200");
		HC06_PutStr("AT+BAUD5");
	}
	else if(speed == 38400)
	{
		strcpy(buf, "OK38400");
		HC06_PutStr("AT+BAUD6");
	}
	else if(speed == 57600)
	{
		strcpy(buf, "OK57600");
		HC06_PutStr("AT+BAUD7");
	}
	else if(speed == 115200)
	{
		strcpy(buf, "OK115200");
		HC06_PutStr("AT+BAUD8");
	}
	else
	{
		return 0x01; //error - incorrect speed
	}

	while(HC06_rx_counter < strlen(buf)) //wait for "OK" message
	{
		timeout--;
		HAL_Delay(100);
		if (timeout == 0) return 0x02; //if the timeout delay is exeeded, exit with error code
	}
	if(strcmp(HC06_rx_buffer, buf) == 0)
		return 0x00; //success
	else
		return 0x03; //unknown return AT msg from HC06
}


error_t HC06_SetName(char *name)
{
	uint8_t timeout = 10;
	char cmd[20];

	HC06_ClearRxBuffer(); //clear rx buffer

	if(strlen(name) > 13) return E_INVALID_ARGS;

	sprintf(cmd, "AT+NAME%s", name);
	HC06_PutStr(cmd);

	while(HC06_rx_counter < 9) //wait for "OKsetname" message, i.e. 9 chars
	{
		timeout--;
		HAL_Delay(100);
		if (timeout == 0) return E_RESPONSE_TIMEOUT;
	}
	if(strcmp(HC06_rx_buffer, "OKsetname") == 0)
		return E_SUCCESS;
	else
		return E_UNKNOWN_RESPONSE;
}


error_t HC06_SetPin(char *pin)
{
	uint32_t timeout = 100;
	char buf[20];

	HC06_ClearRxBuffer(); //clear rx buffer

	if(strlen(pin)!=4) return E_INVALID_ARGS;

	sprintf(buf, "AT+PIN%s", pin);
	HC06_PutStr(buf); //AT command for SET PIN

	while(HC06_rx_counter < 8) //wait for "OKsetpin" message, i.e. 8 chars
	{
		timeout--;
		HAL_Delay(100);
		if (timeout == 0) return E_RESPONSE_TIMEOUT;
	}
	if(strcmp(HC06_rx_buffer, "OKsetPIN") == 0)
		return E_SUCCESS;
	else
		return E_UNKNOWN_RESPONSE;
}
