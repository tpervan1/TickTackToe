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
#include "tests.h"
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//#define TESTING
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CRC_HandleTypeDef hcrc;

/* USER CODE BEGIN PV */

uint8_t u8TimerCount = 0;
GameState currentState = SEND_ID;
bool bTimer1Elapsed = 0;
bool bMessageReceived = 0;
Message messageSend;
Message messageReceived;
uint8_t au8Table[9];
uint8_t au8TableCompressed[3];
uint8_t moveCount = 0;
volatile bool flag = 0;



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void MX_CRC_Init(void);
/* USER CODE BEGIN PFP */

void vType1MessagePrint(Message message, char msg[]);

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

  /* USER CODE BEGIN 2 */
	linear_buf lb;
  lb.curr_index = 0;
	uint8_t ch;
	
	vBoardInit();
	vGPIOInit(GPIOE , P8|P9|P10|P11|P12|P13|P14|P15, OUTPUT);
	vGPIOInit(GPIOA , P0, INPUT);
	MX_CRC_Init();
	vCommInit(COM_USART1, MEDIUM, POLLING, MASTER);
	vCommInit(COM_USART2, MEDIUM, INTERRUPT, MASTER);
	
	#ifdef TESTING
	uint8_t test=runTests();
	if(test) 
	{
		printf("All tests passed.\n");
	}
  else{
	  printf("One or more tests failed!\n");
	}
	#endif
	
	#ifndef TESTING
	
	vCreateMessage('I', '2', 0, 0, &messageSend);
	//sending first time, enable receive 
	vDataTransmit(COM_USART1, (uint8_t *)&messageSend, 9, POLLING);
	vDataTransmit(COM_USART2, (uint8_t *)&messageSend, 9, INTERRUPT);
	vDataReceive(COM_USART2, (uint8_t *)&messageReceived, 9, INTERRUPT);
	
		
	//start counting 5*1s
	vTimerInit(T1, 47999, 1000);
	vStartTimer(T1);
	
	
	
	//vDataTransmit(COM_USART1, &number, 1, DMA);
	
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
							   //printf("%s", lb.buf);
							   linear_buf_reset(&lb);

						 }
					}
				  if (flag) {
          break;
          }
				}
				break;
				
			case RST_SENT:
			  if(bMessageReceived) {
				  bMessageReceived = 0;
					vParseMessage(messageReceived, &currentState);
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
			
			case GAME_START:
				currentState = GAME_START_FIRST;
			  printf(" ");
			  vFunction1(RSP_GST);
			  printf(" Game start, initializing game");
				break;
			
			case GAME_START_FIRST:
				currentState = GAME_WAIT_SLAVE;
				vInitializeTable(au8Table);
			  vPlaceRandomX(au8Table);
			  moveCount++;
			  vCompressTable(au8Table, au8TableCompressed);
			  vCreateMessage('T', au8TableCompressed[0] , au8TableCompressed[1], au8TableCompressed[2], &messageSend);
				printf(" ");
			  vDataTransmit(COM_USART1, (uint8_t *)&messageSend, 9, POLLING);
			  //master is faster, so adding some delay time
			  //this should be handled better when slave sends rst message
			  vDelay(1100);
			  vDataTransmit(COM_USART2, (uint8_t *)&messageSend, 9, INTERRUPT);
			  printf(" ");
			  vFunction1(RSP_WSR);
			  vDataReceive(COM_USART2, (uint8_t *)&messageReceived, 9, INTERRUPT);
			  printf(" Waiting slave response");
				break;
			
			case GAME_WAIT_SLAVE:
			  if(bMessageReceived) {
				  bMessageReceived = 0;
					vParseMessage(messageReceived, &currentState);
					if(currentState == GAME_WAIT_SLAVE) {
					  vDataReceive(COM_USART2, (uint8_t *)&messageReceived, 9, INTERRUPT);
					}
					vDataReceive(COM_USART2, (uint8_t *)&messageReceived, 9, INTERRUPT);
				}
				break;
				
			case GAME_INPUT:
				vDataReceive(COM_USART1, &ch, 1, POLLING);
		    linear_buf_add(&lb, ch);
		    if(linear_buf_ready(&lb)) {
		      uint8_t x,y;
					//vCheckUserInputSPL(lb, &x, &y, &currentState);
					vCheckUserInputAllType1Messages(lb, &x , &y, &currentState);
					if(currentState == GAME_WAIT_SLAVE) {
						au8Table[(x-1)*3 + (y-1)] = 'X';
						moveCount++;
						if(moveCount >= 5) {
						  if (u8CheckWinCondition(au8Table)) {
						    currentState = CHECK_END;
							}
						}
						printf(" %d %d", moveCount, (!u8CheckWinCondition(au8Table)));
						vDelay(1);
						if(moveCount == 9 && (!u8CheckWinCondition(au8Table))) {
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
							vDataReceive(COM_USART2, (uint8_t *)&messageReceived, 9, INTERRUPT);
						}
						vDataTransmit(COM_USART2, (uint8_t *)&messageSend, 9, INTERRUPT);
						vDataTransmit(COM_USART1, (uint8_t *)&messageSend, 9, POLLING);
					}
			    linear_buf_reset(&lb);
				}
			  break;
				
			case CHECK_END:
			  if(bMessageReceived) {
				  bMessageReceived = 0;
					vParseMessage(messageReceived, &currentState);
					if(currentState == CHECK_END) {
					  vDataReceive(COM_USART2, (uint8_t *)&messageReceived, 9, INTERRUPT);
					}
					vDataReceive(COM_USART2, (uint8_t *)&messageReceived, 9, INTERRUPT);
				}
				break;				
			
			
			case SEND_ID:
				if(u8TimerCount < 5 && bTimer1Elapsed && !bMessageReceived) {
					//sends message each second until 5s is reached
					bTimer1Elapsed = 0;
					printf(" Sent %ds: ", u8TimerCount);
					vDataTransmit(COM_USART1, (uint8_t *)&messageSend, 9, POLLING);
	        vDataTransmit(COM_USART2, (uint8_t *)&messageSend, 9, INTERRUPT);
				}
				else if(u8TimerCount < 5 && bMessageReceived) {
					//id received
					bMessageReceived = 0;
					printf(" Received: ");
					vDataTransmit(COM_USART1, (uint8_t *)&messageReceived, 9, POLLING);
					printf(" Parsing: %c", messageReceived.header[0]);
					vParseMessage(messageReceived, &currentState);
					vDataReceive(COM_USART2, (uint8_t *)&messageReceived, 9, INTERRUPT);
					printf(" %d ", currentState);
					if(currentState == RECEIVE_ID) {
						u8TimerCount = 0;
						bTimer1Elapsed = 0;
					  vStopTimer(T1);
	          vSetTimerCount(T1, 0);
		        vStartTimer(T1);
					}
					
					else if(currentState == IDLE || currentState == GAME_START) {
					  bTimer1Elapsed = 0;
					  vStopTimer(T1);
	          vSetTimerCount(T1, 0);
					}
				}
				else if (u8TimerCount == 5) {
			    //5s elapsed, no response from slave
			    bTimer1Elapsed = 0;
			    vStopTimer(T1);
			    vSetTimerCount(T1, 0);
			    currentState = IDLE;
			    vFunction1(ERR_ID_EXCHANGE_FAILURE);
					printf(" Didn't receive K:I in 5s");
				}
				break;
				
			case RECEIVE_ID:
				if(u8TimerCount < 5 && bTimer1Elapsed && !bMessageReceived) {
					//sends message each second until 5s is reached
					bTimer1Elapsed = 0;
	        vDataReceive(COM_USART2, (uint8_t *)&messageReceived, 9, INTERRUPT);
				}
				else if(u8TimerCount < 5 && bMessageReceived) {
					//id received
					bMessageReceived = 0;
					printf(" Received: ");
					vDataTransmit(COM_USART1, (uint8_t *)&messageReceived, 9, POLLING);
					vParseMessage(messageReceived, &currentState);
					vDataReceive(COM_USART2, (uint8_t *)&messageReceived, 9, INTERRUPT);
					printf(" %d ", currentState);

					if(currentState == IDLE || currentState == GAME_START) {
					  bTimer1Elapsed = 0;
					  vStopTimer(T1);
	          vSetTimerCount(T1, 0);
					}
				}
				
				else if (u8TimerCount == 5) {
			    //5s elapsed, no response from slave
			    bTimer1Elapsed = 0;
			    vStopTimer(T1);
			    vSetTimerCount(T1, 0);
			    currentState = IDLE;
			    vFunction1(ERR_ID_EXCHANGE_FAILURE);
					printf(" Didn't receive slave ID in 5s");
				}
				break;

		}

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
	
	#endif
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */

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

/*void vEXTICallback(uint16_t GPIO_Pin) {
  if(GPIO_Pin == P0) {
		
		if(GPIORead(GPIOA, P0)) vGPIOTogglePin(GPIOE, P14);
		else vGPIOTogglePin(GPIOE, P15);
  }
}*/

void vUARTTransmitCallback(UART_HandleTypeDef *huart) {

	if (huart->Instance == USART2) {
		vDataReceive(COM_USART2, (uint8_t *)&messageReceived, 9, INTERRUPT);
		vGPIOTogglePin(GPIOE, P13);
		//vDataReceive(COM_USART2, (uint8_t *)&messageReceived, 9, INTERRUPT);
	}
	
	//vDataTransmit(COM_USART1, &number, 1, DMA);	
	//vDataReceive(COM_USART1, &number, 1, DMA);
}


void vUARTReceiveCallback(UART_HandleTypeDef *huart) {
	vGPIOTogglePin(GPIOE, P14);
	if (huart->Instance == USART2) {
		if(currentState == IDLE) {
		  flag = 1;
		}
		bMessageReceived = 1;
	}
}

void vTimer1Callback(void) {
	u8TimerCount++;
  vGPIOTogglePin(GPIOE, P12);
	bTimer1Elapsed = 1;
}

void vTimer2Callback(void) {
  vGPIOTogglePin(GPIOE, P13);
}

void vTimer3Callback(void) {
  vGPIOTogglePin(GPIOE, P14);
}

void vTimer4Callback(void) {
  vGPIOTogglePin(GPIOE, P15);
}

/*void vType1MessagePrint(Message message, char msg[]) {
  sprintf(msg, "%c%c%c%c%c", message.header[0], message.header[1], message.payload[0], message.payload[1], message.payload[2]);
}*/



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
