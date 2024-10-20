#include "wrapper.h"

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;
SPI_HandleTypeDef hspi3;
DMA_HandleTypeDef hdma_spi1_rx;
DMA_HandleTypeDef hdma_spi1_tx;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

int fputc(int ch, FILE *f){
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
	//vDataTransmit(COM_USART1, (uint8_t *)&ch, 1, POLLING);
  return ch;
}
int fgetc(FILE *f){
  uint8_t ch = 0;
  __HAL_UART_CLEAR_OREFLAG(&huart1);
  HAL_UART_Receive(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
	//vDataReceive(COM_USART1, (uint8_t *)&ch, 1, POLLING);
  return ch;
}


void vBoardInit(void) {
  HAL_Init();
	vSystemClockInitConfig();
}



void vGPIOInit(GPIO_TypeDef *port , GPIOPin pin , GPIOMode mode) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  // Enable GPIO clock based on port
  if (port == GPIOA) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
  }
  else if (port == GPIOB) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
		
  }
  else if (port == GPIOC) {
    __HAL_RCC_GPIOC_CLK_ENABLE();
  }
  else if (port == GPIOD) {
    __HAL_RCC_GPIOD_CLK_ENABLE();
  }
  else if (port == GPIOE) {
    __HAL_RCC_GPIOE_CLK_ENABLE();
  }
  else if (port == GPIOF) {
    __HAL_RCC_GPIOF_CLK_ENABLE();
  }
	else {
	  vErrorCallback(ERROR_GPIO);
	}

  GPIO_InitStruct.Pin = pin;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
	GPIO_InitStruct.Mode = mode;
  HAL_GPIO_Init(port, &GPIO_InitStruct);
	
	if (mode == IT_RISING || mode == IT_RISING_FALLING) {
    IRQn_Type irqn;
		
		if(pin == P0) {
		  irqn = EXTI0_IRQn;
		}
		else if (pin == P1) {
		  irqn = EXTI1_IRQn;
		}
		else if (pin == P2) {
		  irqn = EXTI2_TSC_IRQn;
		}
		else if (pin == P3) {
		  irqn = EXTI3_IRQn;
		}
		else if (pin == P4) {
		  irqn = EXTI4_IRQn;
		}
		else if (pin == P5 || pin == P6 || pin == P7 || pin == P8 || pin == P9) {
		  irqn = EXTI9_5_IRQn;
		}
		else if (pin == P10 || pin == P11 || pin == P12 || pin == P13 || pin == P14 || pin == P15) {
		  irqn = EXTI15_10_IRQn;
		}
		else {
		  vErrorCallback(ERROR_GPIO);
		}

    HAL_NVIC_SetPriority(irqn, 0, 0);
    HAL_NVIC_EnableIRQ(irqn);
  }
}

void vGPIOWrite(GPIO_TypeDef *port, GPIOPin pin, GPIOPinState state){
    HAL_GPIO_WritePin(port, pin, (GPIO_PinState)state);
}

GPIOPinState GPIORead(GPIO_TypeDef *port, GPIOPin pin) {
	if (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET) {
	  return HIGH;
	}
  return LOW;
}

void vGPIOTogglePin(GPIO_TypeDef* port, GPIOPin pin) {
  HAL_GPIO_TogglePin(port, pin);
}

void vLedBSRR(uint8_t number, GPIOPinState state) {
  if (state == LOW) {
	  GPIOE->BRR = number << 8;
	}
	else if (state == HIGH) {
	  GPIOE->BSRR = number << 8;
	}
}


void vDelay(uint32_t delay) {
  HAL_Delay(delay);
}

__weak void vEXTICallback(uint16_t GPIO_Pin) {
  UNUSED(GPIO_Pin);
}
	
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  vEXTICallback(GPIO_Pin);
}

void UART_InitConfig(UART_HandleTypeDef *huart, CommSpeed speed) {
	uint32_t baudRate = (speed == SLOW) ? 9600 : (speed == MEDIUM) ? 115200 : 230400;
  huart->Init.BaudRate = baudRate;
  huart->Init.WordLength = UART_WORDLENGTH_8B;
  huart->Init.StopBits = UART_STOPBITS_1;
  huart->Init.Parity = UART_PARITY_NONE;
  huart->Init.Mode = UART_MODE_TX_RX;
  huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart->Init.OverSampling = UART_OVERSAMPLING_16;
  huart->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
}

void SPI_InitConfig(SPI_HandleTypeDef *hspi, CommSpeed speed, SPIMode mode) {
  hspi->Init.BaudRatePrescaler = (speed == SLOW) ? SPI_BAUDRATEPRESCALER_256 :
                                 (speed == MEDIUM) ? SPI_BAUDRATEPRESCALER_32 :
                                 SPI_BAUDRATEPRESCALER_8;

  hspi->Init.Mode = (mode == MASTER) ? SPI_MODE_MASTER : SPI_MODE_SLAVE;
  hspi->Init.Direction = SPI_DIRECTION_2LINES;
  hspi->Init.DataSize = SPI_DATASIZE_8BIT;
  hspi->Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi->Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi->Init.NSS = SPI_NSS_SOFT;
  hspi->Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi->Init.TIMode = SPI_TIMODE_DISABLE;
  hspi->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi->Init.CRCPolynomial = 7;
  hspi->Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi->Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
}


void vCommInit(CommChannel channel, CommSpeed speed, CommMode mode, SPIMode spiMode) {
	
	if (mode == DMA) {
    __HAL_RCC_DMA1_CLK_ENABLE();
		if (channel == COM_USART1) {
		  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
      HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
      HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
      HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
		}
	  else if (channel == COM_USART2) {
		  HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 0, 0);
      HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
      HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 0, 0);
      HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);
		}
	  else if (channel == COM_SPI1) {
		  HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
      HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
      HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
      HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
		}	
	}
	
	
	if (channel == COM_USART1) {
	  huart1.Instance = USART1;
		UART_InitConfig(&huart1, speed);
		if (HAL_UART_Init(&huart1) != HAL_OK)
    {
			vErrorCallback(ERROR_USART);
    }
	}

	else if (channel == COM_USART2) {
	  huart2.Instance = USART2;
		UART_InitConfig(&huart2, speed);
		if (HAL_UART_Init(&huart2) != HAL_OK)
    {
      vErrorCallback(ERROR_USART);
    }
	}
	
	else if (channel == COM_USART3) {
	  huart3.Instance = USART3;
		UART_InitConfig(&huart3, speed);
		if (HAL_UART_Init(&huart3) != HAL_OK)
    {
      vErrorCallback(ERROR_USART);
    }
	}
	
	else if (channel == COM_SPI1) {
	  hspi1.Instance = SPI1;
		SPI_InitConfig(&hspi1, speed, spiMode);
    if (HAL_SPI_Init(&hspi1) != HAL_OK) 
		{
      vErrorCallback(ERROR_SPI);
    }
	}

	else if (channel == COM_SPI2) {
	  hspi2.Instance = SPI2;
		SPI_InitConfig(&hspi2, speed, spiMode);
    if (HAL_SPI_Init(&hspi2) != HAL_OK) 
		{
      vErrorCallback(ERROR_SPI);
    }
	}
	
	else if (channel == COM_SPI3) {
	  hspi3.Instance = SPI3;
		SPI_InitConfig(&hspi3, speed, spiMode);
    if (HAL_SPI_Init(&hspi3) != HAL_OK) 
		{
      vErrorCallback(ERROR_SPI);
    }
	}
}


void vDataTransmit(CommChannel channel, uint8_t *data, uint16_t size, CommMode mode) {

  switch (channel) {
    case COM_USART1:
      if (mode == POLLING) {
        HAL_UART_Transmit(&huart1, data, size, 0xFFFF);
      } 
			else if (mode == INTERRUPT) {
        HAL_UART_Transmit_IT(&huart1, data, size);
      } 
			else if (mode == DMA) {
        HAL_UART_Transmit_DMA(&huart1, data, size);
      }
      break;

    case COM_USART2:
      if (mode == POLLING) {
        HAL_UART_Transmit(&huart2, data, size, 0xFFFF);
      } 
			else if (mode == INTERRUPT) {
        HAL_UART_Transmit_IT(&huart2, data, size);
      } 
			else if (mode == DMA) {
        HAL_UART_Transmit_DMA(&huart2, data, size);
      }
      break;

    case COM_USART3:
      if (mode == POLLING) {
        HAL_UART_Transmit(&huart3, data, size, 0xFFFF);
      } 
			else if (mode == INTERRUPT) {
        HAL_UART_Transmit_IT(&huart3, data, size);
      } 
			else if (mode == DMA) {
				// No DMA channels for usart3!
        HAL_UART_Transmit_IT(&huart3, data, size);
      }
      break;

    case COM_SPI1:
      if (mode == POLLING) {
        HAL_SPI_Transmit(&hspi1, data, size, 0xFFFF);
      } 
			else if (mode == INTERRUPT) {
        HAL_SPI_Transmit_IT(&hspi1, data, size);
      } 
			else if (mode == DMA) {
        HAL_SPI_Transmit_DMA(&hspi1, data, size);
      }
      break;

    case COM_SPI2:
      if (mode == POLLING) {
        HAL_SPI_Transmit(&hspi2, data, size, 0xFFFF);
      } 
			else if (mode == INTERRUPT) {
        HAL_SPI_Transmit_IT(&hspi2, data, size);
      } 
			else if (mode == DMA) {
				// No DMA channels for spi2!
        HAL_SPI_Transmit_IT(&hspi2, data, size);
      }
      break;

    case COM_SPI3:
      if (mode == POLLING) {
        HAL_SPI_Transmit(&hspi3, data, size, 0xFFFF);
      } 
			else if (mode == INTERRUPT) {
        HAL_SPI_Transmit_IT(&hspi3, data, size);
      } 
			else if (mode == DMA) {
				// No DMA channels for spi3!
        HAL_SPI_Transmit_IT(&hspi3, data, size);
      }
      break;

    default:
      vErrorCallback(ERROR_NONE);
      break;
  }
	
}

void vDataReceive(CommChannel channel, uint8_t *data, uint16_t size, CommMode mode) {

  switch (channel) {
    case COM_USART1:
      if (mode == POLLING) {
        HAL_UART_Receive(&huart1, data, size, 0xFFFF);
      } 
			else if (mode == INTERRUPT) {
        HAL_UART_Receive_IT(&huart1, data, size);
      } 
			else if (mode == DMA) {
        HAL_UART_Receive_DMA(&huart1, data, size);
      }
      break;

    case COM_USART2:
      if (mode == POLLING) {
        HAL_UART_Receive(&huart2, data, size, 0xFFFF);
      } 
			else if (mode == INTERRUPT) {
        HAL_UART_Receive_IT(&huart2, data, size);
      } 
			else if (mode == DMA) {
        HAL_UART_Receive_DMA(&huart2, data, size);
      }
      break;

    case COM_USART3:
      if (mode == POLLING) {
        HAL_UART_Receive(&huart3, data, size, 0xFFFF);
      } else if (mode == INTERRUPT) {
        HAL_UART_Receive_IT(&huart3, data, size);
      } else if (mode == DMA) {
				// No DMA channels for usart3!
        HAL_UART_Receive_IT(&huart3, data, size);
      }
      break;

    case COM_SPI1:
      if (mode == POLLING) {
        HAL_SPI_Receive(&hspi1, data, size, 0xFFFF);
      } else if (mode == INTERRUPT) {
        HAL_SPI_Receive_IT(&hspi1, data, size);
      } else if (mode == DMA) {
        HAL_SPI_Receive_DMA(&hspi1, data, size);
      }
      break;

    case COM_SPI2:
      if (mode == POLLING) {
        HAL_SPI_Receive(&hspi2, data, size, 0xFFFF);
      } else if (mode == INTERRUPT) {
        HAL_SPI_Receive_IT(&hspi2, data, size);
      } else if (mode == DMA) {
				// No DMA channels for spi2!
        HAL_SPI_Receive_IT(&hspi2, data, size);
      }
      break;

    case COM_SPI3:
      if (mode == POLLING) {
        HAL_SPI_Receive(&hspi3, data, size, 0xFFFF);
      } 
			else if (mode == INTERRUPT) {
        HAL_SPI_Receive_IT(&hspi3, data, size);
      } 
			else if (mode == DMA) {
				// No DMA channels for spi3!
        HAL_SPI_Receive_IT(&hspi3, data, size);
      }
      break;

    default:
      vErrorCallback(ERROR_NONE);
      break;
  }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
  vUARTTransmitCallback(huart);
}

__weak void vUARTTransmitCallback(UART_HandleTypeDef *huart) {
  UNUSED(huart);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  vUARTReceiveCallback(huart);
}

__weak void vUARTReceiveCallback(UART_HandleTypeDef *huart) {
  UNUSED(huart);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
  vSPITransmitCallback(hspi);
}

__weak void vSPITransmitCallback(SPI_HandleTypeDef *hspi) {
  UNUSED(hspi);
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
  vSPIReceiveCallback(hspi);
}

__weak void vSPIReceiveCallback(SPI_HandleTypeDef *hspi) {
  UNUSED(hspi);
}

void Timer_InitConfig(TIM_HandleTypeDef *htim, uint16_t prescaler, uint16_t period) {
	
	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
	
  htim->Init.Prescaler = prescaler;
  htim->Init.CounterMode = TIM_COUNTERMODE_UP;
  htim->Init.Period = period;
  htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	
  if (HAL_TIM_Base_Init(htim) != HAL_OK)
  {
    vErrorCallback(ERROR_TIMER);
  }
	
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	
  if (HAL_TIM_ConfigClockSource(htim, &sClockSourceConfig) != HAL_OK)
  {
    vErrorCallback(ERROR_TIMER);
  }
	
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	
  if (HAL_TIMEx_MasterConfigSynchronization(htim, &sMasterConfig) != HAL_OK)
  {
    vErrorCallback(ERROR_TIMER);
  }
}

void TimerAdvanced_InitConfig(TIM_HandleTypeDef *htim, uint16_t prescaler, uint16_t period)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim->Init.Prescaler = prescaler;
  htim->Init.CounterMode = TIM_COUNTERMODE_UP;
  htim->Init.Period = period;
  htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim->Init.RepetitionCounter = 0;
  htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(htim) != HAL_OK)
  {
    vErrorCallback(ERROR_TIMER);
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(htim, &sClockSourceConfig) != HAL_OK)
  {
    vErrorCallback(ERROR_TIMER);
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(htim, &sMasterConfig) != HAL_OK)
  {
    vErrorCallback(ERROR_TIMER);
  }

}

void vTimerInit(Timer timer, uint16_t prescaler, uint16_t period) {

  switch (timer) {
    case T1:
      htim1.Instance = TIM1;
      TimerAdvanced_InitConfig(&htim1, prescaler, period);
      break;
    case T2:
      htim2.Instance = TIM2;
      Timer_InitConfig(&htim2, prescaler, period);
      break;
    case T3:
      htim3.Instance = TIM3;
      Timer_InitConfig(&htim3, prescaler, period);
      break;
    case T4:
      htim4.Instance = TIM4;
      Timer_InitConfig(&htim4, prescaler, period);
      break;
    default:
			vErrorCallback(ERROR_TIMER);
      return;
  }
}

void vStartTimer(Timer timer) {

  switch (timer) {
    case T1:
			__HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE);
      HAL_TIM_Base_Start_IT(&htim1);
      break;
    case T2:
			__HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
      HAL_TIM_Base_Start_IT(&htim2);
      break;
    case T3:
			__HAL_TIM_CLEAR_FLAG(&htim3, TIM_FLAG_UPDATE);
      HAL_TIM_Base_Start_IT(&htim3);
      break;
    case T4:
			__HAL_TIM_CLEAR_FLAG(&htim4, TIM_FLAG_UPDATE);
      HAL_TIM_Base_Start_IT(&htim4);
      break;
    default:
			vErrorCallback(ERROR_TIMER);
      return;
  }
}

void vStopTimer(Timer timer) {
  switch (timer) {
    case T1:
      HAL_TIM_Base_Stop_IT(&htim1);
      break;
    case T2:
      HAL_TIM_Base_Stop_IT(&htim2);
      break;
    case T3:
      HAL_TIM_Base_Stop_IT(&htim3);
      break;
    case T4:
      HAL_TIM_Base_Stop_IT(&htim4);
      break;
    default:
			vErrorCallback(ERROR_TIMER);
      return;
  }
}

void vSetTimerCount(Timer timer, uint16_t value) {
  switch (timer) {
    case T1:
      __HAL_TIM_SET_COUNTER(&htim1, value);
      break;
    case T2:
      __HAL_TIM_SET_COUNTER(&htim2, value);
      break;
    case T3:
      __HAL_TIM_SET_COUNTER(&htim3, value);
      break;
    case T4:
      __HAL_TIM_SET_COUNTER(&htim4, value);
      break;
    default:
			vErrorCallback(ERROR_TIMER);
      return;
  }
}

uint32_t getTimerCount(Timer timer) {
	
	uint32_t timerCount;
	
	if (timer == T1) {
	  timerCount = __HAL_TIM_GET_COUNTER(&htim1);
	}
	else if (timer == T2) {
	  timerCount = __HAL_TIM_GET_COUNTER(&htim2);
	}
	else if (timer == T3) {
	  timerCount = __HAL_TIM_GET_COUNTER(&htim3);
	}
	else if (timer == T4) {
	  timerCount = __HAL_TIM_GET_COUNTER(&htim4);
	}
  else {
	  vErrorCallback(ERROR_TIMER);
		//return -1;
	}
	
	return timerCount;	
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  vTimerCallBack(htim);
}

__weak void vTimerCallBack(TIM_HandleTypeDef *htim) {
	
	if (htim->Instance == TIM1) {
    vTimer1Callback();
  } 
  else if (htim->Instance == TIM2) {
    vTimer2Callback();
  }
	else if (htim->Instance == TIM3) {
    vTimer3Callback();
  }
	else if (htim->Instance == TIM4) {
    vTimer4Callback();
  } 
	else {
	  vErrorCallback(ERROR_TIMER);
	}
}

__weak void vTimer1Callback(void) {
  // User can override this function to handle Timer 1 period elapsed events. 
}
__weak void vTimer2Callback(void) {
  // User can override this function to handle Timer 2 period elapsed events. 
}
__weak void vTimer3Callback(void) {
  // User can override this function to handle Timer 3 period elapsed events.  
}
__weak void vTimer4Callback(void) {
  // User can override this function to handle Timer 4 period elapsed events. 
}



void vSystemClockInitConfig(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    vErrorCallback(ERROR_RCC);
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    vErrorCallback(ERROR_RCC);
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_USART3;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    vErrorCallback(ERROR_RCC);
  }
}

__weak void vErrorCallback(ErrorType error) {
	/* User can add his own implementation to report the HAL error return state */
	vLedBSRR(0xFF, HIGH);
	printf("Error: %d", error);
  __disable_irq();
  while (1)
  {
  }  
}

uint32_t getTick(void) {
  return HAL_GetTick();
}


