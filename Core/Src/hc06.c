/*#############################################################
Driver name	    : hc06.c
Author 			: Grant Phillips
Date Modified   : 25/09/2014
Compiler        : Keil ARM-MDK (uVision V4.70.0.0)
Tested On       : STM32F4-Discovery

Description			: Provides a library to access the USART module
					  on the STM32F4-Discovery to establish serial
					  communication with a remote device (e.g. PC)
					  using the HC-06 Bluetooth module.

Requirements    : * STM32F4-Discovery Board
				  * HC-06 Bluetooth module

Functions		: HC06_Init
				  HC06_PutChar
				  HC06_PutStr
				  HC06_ClearRxBuffer
				  HC06_Test
				  HC06_SetBaud
				  HC06_SetName
				  HC06_SetPin

Special Note(s) : In this driver PC6 is used as USART_TX and
					PC7 as USART_RX. Any other UART and pins can
					be used, just change the relevant GPIO config-
					urations and occurrences of USART to the new
					UART/USART number.
##############################################################*/

#include <string.h>
#include "hc06.h"

extern UART_HandleTypeDef huart3;

volatile char HC06_rx_buffer[HC06_RX_BUFFER_LENGTH];	//used by the IRQ handler
volatile uint8_t HC06_rx_counter = 0; //used by the IRQ handler
volatile char HC06_msg[HC06_RX_BUFFER_LENGTH]; //variable that contains the latest string received on the RX pin
volatile uint8_t new_HC06_msg = 0; //flag variable to indicate if there is a new message to be serviced

/*********************************************************************************************
Function name   : HC06_Init
Author 			: Andrei Istodorescu
Date Modified   : 02/08/2014 (Grant Phillips)
Compiler        : Keil ARM-MDK (uVision V4.70.0.0)

Description		: Initializes the HC-06 Bluetooth module

Special Note(s) : NONE

Parameters		: speed	- 32-bit value to set the baud rate
Return value	: NONE
*********************************************************************************************/
void HC06_Init()
{
	/* Flush USART Tx Buffer */
	HC06_PutStr("\n\r");
	HC06_ClearRxBuffer();
	HAL_Delay(1000);
}


/*********************************************************************************************
Function name   : HC06_PutChar
Author 			: Grant Phillips
Date Modified   : 05/08/2013
Compiler        : Keil ARM-MDK (uVision V4.70.0.0)

Description		: Writes a character to the HC-06 Bluetooth module.

Special Note(s) : NONE

Parameters		: byte -	character to print

Return value	: NONE
*********************************************************************************************/
void HC06_PutChar(char *byte)
{
	/* Put character on the serial line */
	HAL_UART_Transmit(&huart3, byte, 1, 1000);
}

/*********************************************************************************************
Function name   : HC06_PutStr
Author 			: Grant Phillips
Date Modified   : 05/08/2013
Compiler        : Keil ARM-MDK (uVision V4.70.0.0)

Description		: Writes a string to the HC-06 Bluetooth module.

Special Note(s) : NONE

Parameters		: str - string (char array) to print

Return value	: NONE
*********************************************************************************************/
void HC06_PutStr(char *str)
{
	for(int i = 0; str[i] != '\0'; i++)
		HC06_PutChar(&str[i]);
}

/*********************************************************************************************
Function name   : HC06_ClearRxBuffer
Author 			: Grant Phillips
Date Modified   : 05/08/2013
Compiler        : Keil ARM-MDK (uVision V4.70.0.0)

Description		: Clears the software Rx buffer for the HC-06.

Special Note(s) : NONE

Parameters		: ch -	character to print

Return value	: NONE
*********************************************************************************************/
void HC06_ClearRxBuffer(void)
{
	memset(HC06_rx_buffer, 0, HC06_RX_BUFFER_LENGTH); //clear HC06_rx_buffer
	HC06_rx_counter = 0; //reset the Rx buffer counter
	new_HC06_msg = 0; //reset new message flag
}

/*********************************************************************************************
Function name   : HC06_Test
Author 			: Grant Phillips
Date Modified   : 06/08/2013
Compiler        : Keil ARM-MDK (uVision V4.70.0.0)

Description		: Tests if there is communications with the HC-06.

Special Note(s) : NONE

Parameters		: NONE

Return value	: 0		-		Success
				  1		-		Timeout error; not enough characters received for "OK" message
				  2		-		enough characters received, but incorrect message
*********************************************************************************************/
uint8_t HC06_Test(void)
{
	uint8_t tries = 10;
	char str[2] = "AT";

	HC06_ClearRxBuffer(); //clear rx buffer
	HC06_PutStr(str); //AT command for TEST COMMUNICATIONS
	while(HC06_rx_counter < 2) //wait for "OK" - i.e. waiting for 2 characters
	{
		tries--;
		HAL_Delay(100);
		if (tries == 0) return 0x00; //if the timeout delay is exeeded, exit with error code
	}
	if(strcmp(HC06_rx_buffer, "OK") == 0)
		return 0x01; //success
	else
		return 0x02; //unknown return AT msg from HC06
}

/*********************************************************************************************
Function name   : HC06_SetBaud
Author 			: Grant Phillips
Date Modified   : 06/08/2013
Compiler        : Keil ARM-MDK (uVision V4.70.0.0)

Description		: Set the default Baud rate for the HC-06.

Special Note(s) : NONE

Parameters		: speed - 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200 or 230400

Return value	: 0		-		Success
				  1		-		Incorrect speed selected/typed
				  2		-		Timeout error; not enough characters received for "OKxxxx" message
				  3		-		enough characters received, but incorrect message
*********************************************************************************************/
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

/*********************************************************************************************
Function name   : HC06_SetName
Author 					: Grant Phillips
Date Modified   : 06/08/2013
Compiler        : Keil ARM-MDK (uVision V4.70.0.0)

Description		: Set the default Bluetooth name for the HC-06.

Special Note(s) : NONE

Parameters		: name - string that represents the new name (up to 20 characters)

Return value	: 0		-		Success
				  1		-		error - more than 13 characters used for name
				  2		-		Timeout error; not enough characters received for "OKsetname" message
				  3		-		enough characters received, but incorrect message
*********************************************************************************************/
uint8_t HC06_SetName(char *name)
{
	uint32_t timeout = 100;
	char cmd[20];

	HC06_ClearRxBuffer(); //clear rx buffer

	if(strlen(name) > 13) //error - name more than 20 characters
		return 0x01;

	sprintf(cmd, "AT+NAME%s", name);
	HC06_PutStr(cmd); //AT command for SET NAME

	while(HC06_rx_counter < 9) //wait for "OKsetname" message, i.e. 9 chars
	{
		timeout--;
		HAL_Delay(100);
		if (timeout == 0) return 0x02; //if the timeout delay is exeeded, exit with error code
	}
	if(strcmp(HC06_rx_buffer, "OKsetname") == 0)
		return 0x00; //success
	else
		return 0x03; //unknown return AT msg from HC06
}

/*********************************************************************************************
Function name   : HC06_SetPin
Author 			: Grant Phillips
Date Modified   : 06/08/2013
Compiler        : Keil ARM-MDK (uVision V4.70.0.0)

Description		: Set the default Bluetooth name for the HC-06.

Special Note(s) : NONE

Parameters: pin - string that represents the new pin number (must be 4 characters); must
			be represented by "0" - "9" characters, e.g. "1234"

Return value	: 0		-		Success
			  	  1		-		pin less than or more than 4 characters or/and not valid
								characters ("0" - "9")
			  	  2		-		Timeout error; not enough characters received for "OKsetPIN" message
			  	  3		-		enough characters received, but incorrect message
*********************************************************************************************/
uint8_t HC06_SetPin(char *pin)
{
	uint32_t timeout = 10;
	char buf[20];

	HC06_ClearRxBuffer(); //clear rx buffer

	if(strlen(pin)!=4) return 0x01; //error - too few or many characetrs in pin

	sprintf(buf, "AT+PIN%s", pin);
	HC06_PutStr(buf); //AT command for SET PIN

	while(HC06_rx_counter < 8) //wait for "OKsetpin" message, i.e. 8 chars
	{
		timeout--;
		HAL_Delay(100);
		if (timeout == 0) return 0x02; //if the timeout delay is exeeded, exit with error code
	}
	if(strcmp(HC06_rx_buffer, "OKsetPIN") == 0)
		return 0x00; //success
	else
		return 0x03; //unknown return AT msg from HC06
}
