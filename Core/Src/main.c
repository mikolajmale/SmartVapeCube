/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "RingBuffer.h"
#include "hc06.h"
#include "queue.h"
#include "semphr.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
TaskHandle_t BleTaskHandle = NULL;
TaskHandle_t CliTaskHandle = NULL;
volatile uint8_t Received;
volatile uint8_t ReceivedHC06;
BaseType_t checkIfYieldRequired_BLE;
BaseType_t checkIfYieldRequired_CLI;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
void vLEDTask( void *pvParameters );
void vBleSendTask( void *pvParameters );
void vCliSendTask( void *pvParameters );
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  NVIC_SetPriorityGrouping(0U);
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */

  // Change priority to enable rtos callbacks
  HAL_NVIC_SetPriority(USART3_IRQn, 5, 0);
  HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);

  // Creating tasks
  xTaskCreate( vLEDTask, "LEDTask", 100, NULL, 1, NULL );
  xTaskCreate( vBleSendTask, "BleSendTask", 100, NULL, 1, &BleTaskHandle );
  xTaskCreate( vCliSendTask, "CliSendTask", 100, NULL, 1, &CliTaskHandle );
  HAL_GPIO_WritePin(B_LED_GPIO_Port, B_LED_Pin, GPIO_PIN_SET);

  // Start receiving interrupts
  HAL_UART_Receive_IT(&huart2, (uint8_t*)&Received, 1);
  HAL_UART_Receive_IT(&huart3, (uint8_t*)&ReceivedHC06, 1);

  //HC06_Init(&huart2);
  /* USER CODE END 2 */

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();
  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

	if(huart->Instance == USART2){
		rng_buf_add(Received);
		if(Received == 13){
			checkIfYieldRequired_CLI = xTaskResumeFromISR(CliTaskHandle);
			portYIELD_FROM_ISR(checkIfYieldRequired_CLI);
		}

		HAL_UART_Receive_IT(&huart2, (uint8_t*)&Received, 1);
	}

	else if(huart->Instance == USART3){
		HC06_rx_buffer[HC06_rx_counter] = (char)ReceivedHC06;

		if(HC06_rx_buffer[HC06_rx_counter] == 0x0d) {
			checkIfYieldRequired_BLE = xTaskResumeFromISR(BleTaskHandle);
			portYIELD_FROM_ISR(checkIfYieldRequired_BLE);
		} else {
			HC06_rx_counter++;
		}

		HAL_UART_Receive_IT(&huart3, (uint8_t*)&ReceivedHC06, 1);
	}

};

void vLEDTask(void *pvParameters) {

	for (;;) {

		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
		vTaskDelay( 100 / portTICK_RATE_MS );
	}

	vTaskDelete(NULL);
};

void vBleSendTask( void *pvParameters ){
	while(1){
		vTaskSuspend(NULL);
		  memcpy(HC06_msg, HC06_rx_buffer, HC06_rx_counter);
		  memset(HC06_rx_buffer, 0, HC06_RX_BUFFER_LENGTH);
		  HC06_rx_counter = 0;

		  char Data[20];
		  sprintf((char*)Data, "HC06 (received): %s \n\r", HC06_msg);
		  memset(HC06_msg, 0, HC06_RX_BUFFER_LENGTH);
		  HAL_UART_Transmit(&huart2, Data, strlen(Data), 100);
	}
	vTaskDelete(NULL);
};

void vCliSendTask( void *pvParameters ){
	uint8_t Data[50] = {0};
	uint8_t cmd[20] = {0};
	while(1){
		vTaskSuspend(NULL);
			rng_buf_get_buff(cmd);
			sprintf((char*)Data, "Odebrana wiadomosc: %s \n\r", cmd);
			HAL_UART_Transmit(&huart2, Data, strlen(Data), 1000);
			memset(cmd, 0, 20);
			memset(Data, 0, 50);
	}
	vTaskDelete(NULL);
};
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
