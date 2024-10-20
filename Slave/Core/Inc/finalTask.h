#ifndef FINALTASK_H
#define FINALTASK_H

#include "stm32f3xx_hal.h"
#include "wrapper.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define MESSAGE_HEADER_SIZE 2
#define MESSAGE_PAYLOAD_SIZE 3
#define CRC_SIZE 4
#define ID_MASTER 2
#define ID_SLAVE 1
#define BOARD_ID 1
#define LB_SIZE 256

typedef struct
{
	int16_t curr_index;
	uint8_t buf[LB_SIZE];
} linear_buf;


typedef struct {
  uint8_t header[MESSAGE_HEADER_SIZE];
  uint8_t payload[MESSAGE_PAYLOAD_SIZE];
  uint32_t crc;
} Message;

typedef enum {
  IDLE,
	GAME_START,
	SEND_ID,
	RECEIVE_ID,
	GAME_WAIT_MASTER,
	GAME_INPUT, 	
	CHECK_END, 
	RST_SENT,
	RST_RECEIVED
} GameState;

typedef enum {
    RSP_OK = 100,
    RSP_ID_EXCHANGE_OK = 101,
    RSP_GST = 102,
    RSP_WSR = 103,
    RSP_WMR = 104,
    ERR_ID_EXCHANGE_FAILURE = 201,
    ERR_TABLE_DATA = 202,
    ERR_INVALID_USER_MESSAGE = 203,
    ERR_INVALID_COORDINATES = 204,
    ERR_INVALID_SIGN_PLACEMENT = 205,
    ERR_END_CHECK_FAILED = 206,
    ERR_INVALID_E_MESSAGE = 301,
    ERR_INVALID_R_MESSAGE = 302,
    ERR_INVALID_W_MESSAGE = 303,
    ERR_INVALID_MESSAGE = 304,
    ERR_PROTOCOL_FAILURE = 305
} ResponseCode ;

uint32_t u32CalculateMessageCRC(Message msg);
void vFunction1(ResponseCode code);
void vParseMessage(Message msg, GameState *currentState);
bool bCheckCRC(Message msg);
void vCreateMessage(uint8_t headerType, uint8_t payloadByte1, uint8_t payloadByte2, uint8_t payloadByte3, Message *msg);
void vInitializeTable(uint8_t table[]);
void vCompressTable(uint8_t table[], uint8_t compressedTable[]);
void vDecompressTable(uint8_t compressedTable[], uint8_t table[]);

void linear_buf_reset(linear_buf *lb);
void linear_buf_add(linear_buf *lb, char c);
uint8_t linear_buf_ready(linear_buf *lb);

void vCheckUserInputSPL(linear_buf lb, uint8_t *x , uint8_t *y, GameState *currentState);
void vPrintTable(uint8_t table[]);
uint8_t u8CheckWinCondition(uint8_t table[]);
bool bCheckDrawCondition(uint8_t  table[]);
bool bCheckTableFormat(uint8_t table[]);
bool bCheckSignValidity(uint8_t oldTable[], uint8_t newTable[]);
void vCheckUserInputRST(linear_buf lb, GameState *currentState);

void vCheckUserInputAllType1Messages(linear_buf lb, uint8_t *x , uint8_t *y, GameState *currentState);

#endif

