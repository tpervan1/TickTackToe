/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2024 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "wrapper.h"
#include "finalTask.h"
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MESSAGE_HEADER_SIZE 2
#define MESSAGE_PAYLOAD_SIZE 3
#define CRC_SIZE 4
#define ID_MASTER 2
#define ID_SLAVE 1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CRC_HandleTypeDef hcrc;


/* USER CODE BEGIN PV */


uint8_t u8TimerCount = 0;
GameState currentState = RECEIVE_ID;
bool bTimer2Elapsed = 0;
bool bMessageReceived = 0;
Message messageSend;
Message messageReceived;
uint8_t au8Table[9];
uint8_t au8TableCompressed[3];
uint8_t moveCount = 0;
extern bool testTransmit;
volatile bool flag = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void MX_CRC_Init(void);
/* USER CODE BEGIN PFP */
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
  //HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  //SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  /*MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_USART3_UART_Init();
  MX_SPI2_Init();
  MX_SPI3_Init();
  MX_TIM4_Init();
  MX_CRC_Init();
  MX_TIM1_Init();*/
  /* USER CODE BEGIN 2 */
  linear_buf lb;
  lb.curr_index = 0;
	uint8_t ch;
	
	vBoardInit();
	MX_CRC_Init();
	vGPIOInit(GPIOE , P8|P9|P10|P11|P12|P13|P14|P15, OUTPUT);
	vGPIOInit(GPIOA , P0, IT_RISING);
	vCommInit(COM_USART1, MEDIUM, POLLING, MASTER);
	vCommInit(COM_USART2, MEDIUM, INTERRUPT, MASTER);
	
	vCreateMessage('I', '1', 0, 0, &messageSend);
	vDataReceive(COM_USART2, (uint8_t*)&messageReceived, 9, INTERRUPT);
	vTimerInit(T2, 47999, 1000);
	vStartTimer(T2);
	vInitializeTable(au8Table);
	
	//vCommInit(COM_SPI1, FAST, DMA, SLAVE);
	//vDataTransmit(COM_USART1, &number, 1, POLLING);
	//vDataReceive(COM_SPI1, &number, 1, DMA);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		switch(currentState) {
		  case IDLE:
				if(flag) {
					bMessageReceived = 0;
				  vParseMessage(messageReceived, &currentState);
					flag = 0;
				}
				vDataReceive(COM_USART2, (uint8_t *)&messageReceived, 9, INTERRUPT);
				while(currentState == IDLE) {
				  if(HAL_UART_Receive(&huart1, &ch, 1, 100) == HAL_OK) {
					  linear_buf_add(&lb, ch);
						if(linear_buf_ready(&lb)) {
						     vCheckUserInputRST(lb, &currentState);
							   if(currentState == RST_SENT) {
								   vDataReceive(COM_USART2, (uint8_t *)&messageReceived, 9, INTERRUPT);
								 }
							   linear_buf_reset(&lb);

						 }
					}
				  if (flag) {
          break;
          }
				}
				break;
		
			case RST_RECEIVED:
				currentState = GAME_START;
				vCreateMessage('K', 'R', 0, 0, &messageSend);
			  vDataTransmit(COM_USART2, (uint8_t *)&messageSend, 9, INTERRUPT);
			  moveCount = 0;
			  vInitializeTable(au8Table);
			  printf(" Reset received, memory succesfully deleted!");
			  vDelay(1000);
				break;
			
			case RST_SENT:
			  if(bMessageReceived) {
				  bMessageReceived = 0;
					vParseMessage(messageReceived, &currentState);
				}
				break;

			case GAME_START:
				currentState = GAME_WAIT_MASTER;
			  vFunction1(RSP_GST);
			  vFunction1(RSP_WMR);
			  vDataReceive(COM_USART2, (uint8_t *)&messageReceived, 9, INTERRUPT);
			  printf(" Game start, waiting master response");
				break;
			
			case GAME_WAIT_MASTER:
			  if(bMessageReceived) {
				  bMessageReceived = 0;
					vParseMessage(messageReceived, &currentState);
					if(currentState == GAME_WAIT_MASTER) {
					  vDataReceive(COM_USART2, (uint8_t *)&messageReceived, 9, INTERRUPT);
					}
					vDataReceive(COM_USART2, (uint8_t *)&messageReceived, 9, INTERRUPT);
				}
				break;
			
			case GAME_INPUT:
				vDataReceive(COM_USART1, &ch, 1, POLLING);
		    linear_buf_add(&lb, ch);
		    if(linear_buf_ready(&lb)) {
		      //printf("\nYou've entered: %s %d %d %d %d ", lb.buf, lb.curr_index, lb.buf[lb.curr_index-4], lb.buf[lb.curr_index-3], lb.buf[lb.curr_index-2]);
					uint8_t x,y;
					//vCheckUserInputSPL(lb, &x, &y, &currentState);
					vCheckUserInputAllType1Messages(lb, &x , &y, &currentState);
					if(currentState == GAME_WAIT_MASTER) {
						au8Table[(x-1)*3 + (y-1)] = 'O';
						moveCount++;
						if(moveCount >= 5) {
						  if (u8CheckWinCondition(au8Table)) {
						    currentState = CHECK_END;
							}
						}
						if(moveCount == 9) {
						  if (bCheckDrawCondition(au8Table)) {
						    currentState = CHECK_END;
							}
						}
						vCompressTable(au8Table, au8TableCompressed);
						if (currentState == CHECK_END) {
						  vCreateMessage('W', au8TableCompressed[0] , au8TableCompressed[1], au8TableCompressed[2], &messageSend);
						}
						else {
						  vCreateMessage('T', au8TableCompressed[0] , au8TableCompressed[1], au8TableCompressed[2], &messageSend);
							vFunction1(RSP_WMR);
						}
						vDataTransmit(COM_USART2, (uint8_t *)&messageSend, 9, INTERRUPT);
						vDataTransmit(COM_USART1, (uint8_t *)&messageSend, 9, POLLING);
					}
			    linear_buf_reset(&lb);
				}
			  break;
				
			case CHECK_END:
			  if(bMessageReceived) {
					printf("Received");
					vDataTransmit(COM_USART1, (uint8_t *)&messageReceived, 9, POLLING);
				  bMessageReceived = 0;
					vParseMessage(messageReceived, &currentState);
					if(currentState == CHECK_END) {
					  vDataReceive(COM_USART2, (uint8_t *)&messageReceived, 9, INTERRUPT);
					}
					vDataReceive(COM_USART2, (uint8_t *)&messageReceived, 9, INTERRUPT);
				}
				break;
			
			
			case RECEIVE_ID:
				if (u8TimerCount == 5) {
					currentState = IDLE;
					bTimer2Elapsed = 0;
					vStopTimer(T2);
					vSetTimerCount(T2, 0);
					vFunction1(ERR_ID_EXCHANGE_FAILURE);
					printf(" Didn't receive master ID in 5s");
				}
				else if(u8TimerCount < 5 && bTimer2Elapsed && !bMessageReceived) {
					bTimer2Elapsed = 0;
	        vDataReceive(COM_USART2, (uint8_t *)&messageReceived, 9, INTERRUPT);
					
				}
				else if(u8TimerCount < 5 && bMessageReceived) {
					bMessageReceived = 0;
					bTimer2Elapsed = 0;
					vStopTimer(T2);
					vSetTimerCount(T2, 0);
					printf(" Received: ");
					vDataTransmit(COM_USART1, (uint8_t *)&messageReceived, 9, POLLING);
					vParseMessage(messageReceived, &currentState);
					if(currentState == SEND_ID) {
						u8TimerCount = 0;
						bTimer2Elapsed = 0;
					  vStopTimer(T2);
	          vSetTimerCount(T2, 0);
		        vStartTimer(T2);
					}
					else if(currentState == IDLE) {
					  bTimer2Elapsed = 0;
					  vStopTimer(T1);
	          vSetTimerCount(T1, 0);
					}
				}
				break;
				
			case SEND_ID:
				if (u8TimerCount == 5) {
					currentState = IDLE;
					bTimer2Elapsed = 0;
					vStopTimer(T2);
					vSetTimerCount(T2, 0);
					vFunction1(ERR_ID_EXCHANGE_FAILURE);
					printf(" Didn't receive master ID in 5s");
				}
				else if(u8TimerCount < 5 && bTimer2Elapsed && !bMessageReceived) {
					bTimer2Elapsed = 0;
	        printf(" Sent %ds: ", u8TimerCount);
					vDataTransmit(COM_USART1, (uint8_t *)&messageSend, 9, POLLING);
	        vDataTransmit(COM_USART2, (uint8_t *)&messageSend, 9, INTERRUPT);
					vDataReceive(COM_USART2, (uint8_t *)&messageReceived, 9, INTERRUPT);
				}
				else if(u8TimerCount < 5 && bMessageReceived) {
					bMessageReceived = 0;
					printf(" Received: ");
					vDataTransmit(COM_USART1, (uint8_t *)&messageReceived, 9, POLLING);
					printf(" Parsing: %c", messageReceived.header[0]);
					vParseMessage(messageReceived, &currentState);
					vDataReceive(COM_USART2, (uint8_t *)&messageReceived, 9, INTERRUPT);
					printf(" %d ", currentState);
					if(currentState == IDLE || currentState == GAME_START) {
					  bTimer2Elapsed = 0;
					  vStopTimer(T2);
	          vSetTimerCount(T2, 0);
					}
				}
				break;
		}

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}


static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}


/* USER CODE BEGIN 4 */

void vEXTICallback(uint16_t GPIO_Pin) {
	 if(GPIO_Pin == GPIO_PIN_0) {
	   vGPIOTogglePin(GPIOE, P15);
  } else {
  }
}

void vUARTTransmitCallback(UART_HandleTypeDef *huart) {

	if (huart->Instance == USART2) {
		//vDataReceive(COM_USART2, (uint8_t *)&messageReceived, 9, INTERRUPT);
	}
	
	//vDataTransmit(COM_USART1, &number, 1, DMA);	
	//vDataReceive(COM_USART1, &number, 1, DMA);
}

void vSPITransmitCallback(SPI_HandleTypeDef *hspi) {
	
	//vDataTransmit(COM_USART1, &number, 1, DMA);	
	//vDataReceive(COM_USART1, &number, 1, DMA);
}

void vUARTReceiveCallback(UART_HandleTypeDef *huart) {
	vGPIOTogglePin(GPIOE, P14);
	if (huart->Instance == USART2) {
		if(currentState == IDLE) {
			vGPIOTogglePin(GPIOE, P8);
		  flag = 1;
			//vDataTransmit(COM_USART1, (uint8_t*)&messageReceived, sizeof(messageReceived), POLLING);
			//vDelay(10);
			//HAL_UART_ReceiverTimeout_Config(&huart1, 10);
			//HAL_UART_AbortReceive(&huart1);
			//huart1.RxState = HAL_UART_STATE_TIMEOUT;
			//.Instance->CR1 &= ~USART_CR1_RE;
       //MODIFY_REG(huart1.Instance->RTOR, USART_RTOR_RTO, 0);
      //__HAL_UNLOCK(&huart1);
			//huart1.RxState = HAL_UART_STATE_READY;
			vGPIOTogglePin(GPIOE, P9);
			vGPIOTogglePin(GPIOE, P11);//
		}
		bMessageReceived = 1;
	}
	//vDataReceive(COM_USART2, (uint8_t *)&messageReceived, 9, INTERRUPT);
}

void vTimer1Callback(void) {
  vGPIOTogglePin(GPIOE, P12);
	
}

void vTimer2Callback(void) {
	u8TimerCount++;
  vGPIOTogglePin(GPIOE, P12);
	bTimer2Elapsed = 1;
}
void vTimer3Callback(void) {
  vGPIOTogglePin(GPIOE, P12);
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
