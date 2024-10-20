#ifndef WRAPPER_H
#define WRAPPER_H

#include "stm32f3xx_hal.h"
#include <stdio.h>

struct __FILE{
  int handle;
};

extern FILE __stdout;
extern FILE __stdin;

extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;
extern SPI_HandleTypeDef hspi3;
extern DMA_HandleTypeDef hdma_spi1_rx;
extern DMA_HandleTypeDef hdma_spi1_tx;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

extern CRC_HandleTypeDef hcrc;


typedef enum {
  P0 = GPIO_PIN_0,
  P1 = GPIO_PIN_1,
	P2 = GPIO_PIN_2,
  P3 = GPIO_PIN_3,
  P4 = GPIO_PIN_4,
  P5 = GPIO_PIN_5,
  P6 = GPIO_PIN_6,
  P7 = GPIO_PIN_7,
  P8 = GPIO_PIN_8,
  P9 = GPIO_PIN_9,
  P10 = GPIO_PIN_10,
  P11 = GPIO_PIN_11,
  P12 = GPIO_PIN_12,
  P13 = GPIO_PIN_13,
  P14 = GPIO_PIN_14,
  P15 = GPIO_PIN_15
} GPIOPin;

typedef enum {
  INPUT = GPIO_MODE_INPUT,
  OUTPUT = GPIO_MODE_OUTPUT_PP,
  IT_RISING = GPIO_MODE_IT_RISING,
	IT_RISING_FALLING = GPIO_MODE_IT_RISING_FALLING
} GPIOMode;

typedef enum {
  LOW = GPIO_PIN_RESET,
  HIGH = GPIO_PIN_SET
} GPIOPinState;

typedef enum {
  COM_USART1,
  COM_USART2,
	COM_USART3,
  COM_SPI1,
  COM_SPI2,
	COM_SPI3
} CommChannel;

typedef enum {
  SLOW,
  MEDIUM,
  FAST
} CommSpeed;

typedef enum {
  POLLING,
  INTERRUPT,
  DMA
} CommMode;

typedef enum {
  MASTER,
  SLAVE
} SPIMode;

typedef enum {
	ERROR_NONE,
  ERROR_USART,
  ERROR_SPI,
	ERROR_GPIO, 
	ERROR_RCC,
	ERROR_TIMER
} ErrorType;

typedef enum {
	T1,
  T2,
  T3,
  T4,
} Timer;


void vBoardInit(void);
void vSystemClockInitConfig(void);
void vGPIOInit(GPIO_TypeDef *port , GPIOPin pin, GPIOMode mode);
void vGPIOWrite(GPIO_TypeDef *port, GPIOPin pin, GPIOPinState state);
GPIOPinState GPIORead(GPIO_TypeDef *port, GPIOPin pin);
void vGPIOTogglePin(GPIO_TypeDef* port, GPIOPin pin);
void vDelay(uint32_t delay);
__weak void vEXTICallback(uint16_t GPIO_Pin);
void UART_InitConfig(UART_HandleTypeDef *huart, CommSpeed speed);
void SPI_InitConfig(SPI_HandleTypeDef *hspi, CommSpeed speed, SPIMode mode);
void vCommInit(CommChannel channel, CommSpeed speed, CommMode mode, SPIMode spiMode);
void vDataTransmit(CommChannel channel, uint8_t *data, uint16_t size, CommMode mode);
void vDataReceive(CommChannel channel, uint8_t *data, uint16_t size, CommMode mode);
__weak void vUARTTransmitCallback(UART_HandleTypeDef *huart);
__weak void vUARTReceiveCallback(UART_HandleTypeDef *huart);
__weak void vSPITransmitCallback(SPI_HandleTypeDef *hspi);
__weak void vSPIReceiveCallback(SPI_HandleTypeDef *hspi);
void vTimerInit(Timer timer, uint16_t prescaler, uint16_t period);
void vStartTimer(Timer timer);
void vStopTimer(Timer timer);
void vSetTimerCount(Timer timer, uint16_t value);
uint32_t getTimerCount(Timer timer);
__weak void vTimerCallBack(TIM_HandleTypeDef *htim);
__weak void vTimer1Callback(void);
__weak void vTimer2Callback(void);
__weak void vTimer3Callback(void);
__weak void vTimer4Callback(void);
__weak void vErrorCallback(ErrorType error);

void vLedBSRR(uint8_t number, GPIOPinState state);
uint32_t getTick(void);


#endif




