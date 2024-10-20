#include "finalTask.h"
#include "wrapper.h"

extern uint8_t au8Table[9];
extern uint8_t moveCount;

void linear_buf_reset(linear_buf *lb)
{
  lb->curr_index = 0;
  for(int i=0; i<LB_SIZE; i++) {
	  lb->buf[i] = 0;
	}
}

void linear_buf_add(linear_buf *lb, char c)
{
  lb->buf[lb->curr_index] = c;
  if(lb->curr_index < LB_SIZE - 1)
    lb->curr_index++;
}

uint8_t linear_buf_ready(linear_buf *lb)
{
  if(lb->buf[lb->curr_index - 1] == '\n')
    return 1;
  return 0;
}

void vCreateMessage(uint8_t headerType, uint8_t payloadByte1, uint8_t payloadByte2, uint8_t payloadByte3, Message *msg) {
	 if (msg == NULL) {
    // Handle the error...
    return;
  }
	
	msg->header[0] = headerType;
	msg->header[1] = ':';
	msg->payload[0] = payloadByte1;
	msg->payload[1] = payloadByte2;
	msg->payload[2] = payloadByte3;
	msg->crc = u32CalculateMessageCRC(*msg);
}

uint32_t u32CalculateMessageCRC(Message msg) {
  uint32_t u32PaddedMessage[2] = {msg.header[0]<<24 | msg.header[1]<<16 | msg.payload[0]<<8 | msg.payload[1], msg.payload[2]<<24};
	uint32_t crcValue = HAL_CRC_Calculate(&hcrc, (uint32_t*)u32PaddedMessage, 2);
	return crcValue; 
}

void vFunction1(ResponseCode code) {
	char err_msg[8];
	sprintf(err_msg, " RSP:%d", code);
	vDataTransmit(COM_USART1, (uint8_t *)&err_msg, sizeof(err_msg), POLLING);
}

// message parser for master
void vParseMessage(Message msg, GameState *currentState) {
	
  if (msg.header[0] == 'I' && msg.header[1] == ':') {
		
	  if(msg.payload[0] == '1') {
			printf("Crc I check: %d %d", msg.crc, u32CalculateMessageCRC(msg));
			//if(bCheckCRC(msg)) 
			if(1){
				//crc checked out, send K:I 
				*currentState = GAME_START; 
				Message msgSend;
				vCreateMessage('K', 'I', 0, 0, &msgSend);
				vDataTransmit(COM_USART2, (uint8_t *)&msgSend, sizeof(msgSend), INTERRUPT);
		    vFunction1(RSP_ID_EXCHANGE_OK);
				printf(" Received valid id, game start! ");
	      
			}
			else {
				//crc failed, send P:I
				*currentState = IDLE;
				Message msgSend;
				vCreateMessage('P', 'I', 0, 0, &msgSend);
				vDataTransmit(COM_USART2, (uint8_t *)&msgSend, sizeof(msgSend), INTERRUPT);
				vFunction1(ERR_ID_EXCHANGE_FAILURE);
				printf(" ID Failed - CRC");
				
				//printf(" %d ", *currentState);
				//printf(" State in function: %d ", *currentState);
				//vGPIOWrite(GPIOE, P8, HIGH);
			}
		}
		else {
		  //wrong number id, send P:I
			*currentState = IDLE;
			Message msgSend;
			vCreateMessage('P', 'I', 0, 0, &msgSend);
			vDataTransmit(COM_USART2, (uint8_t *)&msgSend, sizeof(msgSend), INTERRUPT);
			vFunction1(ERR_ID_EXCHANGE_FAILURE);
		  printf(" ID Failed - wrong ID");
			//printf(" %d ", *currentState);
		}
	}
	
	//P message check
	else if (msg.header[0] == 'P' && msg.header[1] == ':' && msg.payload[0] == 'I') {
		//if(bCheckCRC(*msg)) 
		printf("Crc %d %d", msg.crc, u32CalculateMessageCRC(msg));
		if(1){
			*currentState = IDLE;
		  vFunction1(ERR_ID_EXCHANGE_FAILURE);
			printf(" ID failed P message");
			//printf(" %d ", *currentState);
		}
		else {
		  //what if P:I failed crc??
			//do nothing
		}
	}
	
	//K message check for ID exchange
	else if (msg.header[0] == 'K' && msg.header[1] == ':' && msg.payload[0] == 'I') {
		*currentState = RECEIVE_ID;
		printf(" Successfull ID transmission to slave, waiting for slave ID!");
	}
	
	else if (msg.header[0] == 'K' && msg.header[1] == ':' && msg.payload[0] == 'T') {
		printf(" Sent table was valid, waiting slave response");
	}
	
	else if (msg.header[0] == 'P' && msg.header[1] == ':' && msg.payload[0] == 'T') {
		*currentState = IDLE;
		Message msgSend;
		vCreateMessage('P', 'T', 0, 0, &msgSend);
		vDataTransmit(COM_USART2, (uint8_t *)&msgSend, sizeof(msgSend), INTERRUPT);
		printf(" ");
		vFunction1(ERR_TABLE_DATA);
		printf(" Sent table was not valid,going to IDLE state");
	}
	
	else if (msg.header[0] == 'T' && msg.header[1] == ':') {
		uint8_t receivedTable[9];
		vDecompressTable(msg.payload, receivedTable);
		
		//check validity and sign placement!!!!
		if(bCheckTableFormat(receivedTable) && bCheckSignValidity(au8Table, receivedTable)) {
		  *currentState = GAME_INPUT;
			moveCount++;
			memcpy(au8Table, receivedTable, 9);
			//printf(" ");
			//vDataTransmit(COM_USART1, (uint8_t *)&receivedTable, sizeof(receivedTable), INTERRUPT);
			Message msgSend;
			vCreateMessage('K', 'T', 0, 0, &msgSend);
			vDataTransmit(COM_USART2, (uint8_t *)&msgSend, sizeof(msgSend), INTERRUPT);
			vPrintTable(au8Table);
			printf(" Received valid table, user input now!\n");
		}
		else {
		  *currentState = IDLE;
			Message msgSend;
			vCreateMessage('P', 'T', 0, 0, &msgSend);
			vDataTransmit(COM_USART2, (uint8_t *)&msgSend, sizeof(msgSend), INTERRUPT);
			printf(" Table is not valid, going to idle state! ");
		}
		
	}
	
	else if (msg.header[0] == 'W' && msg.header[1] == ':') {
	  uint8_t receivedTable[9];
		vDecompressTable(msg.payload, receivedTable);
		uint8_t ch;
		//same checks like for T
		if(bCheckTableFormat(receivedTable) && bCheckSignValidity(au8Table, receivedTable)) {
			moveCount++;
		  memcpy(au8Table, receivedTable, 9);
			if(moveCount >= 5 && u8CheckWinCondition(au8Table)) {
			  *currentState = CHECK_END;
				//should optimize this
				ch = u8CheckWinCondition(au8Table);
			}
			else if (moveCount == 9 && bCheckDrawCondition(au8Table)) {
			  *currentState = CHECK_END;
				ch = 'D';
			}
			else {
			  *currentState = IDLE;
			  Message msgSend;
			  vCreateMessage('P', 'W', 0, 0, &msgSend);
			  vDataTransmit(COM_USART2, (uint8_t *)&msgSend, sizeof(msgSend), INTERRUPT);
			  vFunction1(ERR_END_CHECK_FAILED);
			  printf(" Not valid winning condition or last table, going to idle state! "); 
			}
			
			if (*currentState == CHECK_END) {
				vPrintTable(au8Table);
        Message msgSend;
				//send K:W message
			  vCreateMessage('K', 'W', 0, 0, &msgSend);
			  vDataTransmit(COM_USART2, (uint8_t *)&msgSend, sizeof(msgSend), INTERRUPT);
				vDelay(10);

				//send V message
				vCreateMessage('V', ch, 0, 0, &msgSend);
				//#ifndef TESTING
				vDataTransmit(COM_USART2, (uint8_t *)&msgSend, sizeof(msgSend), INTERRUPT);
				//#endif
				vDelay(1);
				//printf(" End game condition: %c", ch);
			}
		}
		else {
		  *currentState = IDLE;
			Message msgSend;
			vCreateMessage('P', 'W', 0, 0, &msgSend);
			vDataTransmit(COM_USART2, (uint8_t *)&msgSend, sizeof(msgSend), INTERRUPT);
			vFunction1(ERR_END_CHECK_FAILED);
			printf(" Not valid winning condition or last table, going to idle state! ");
		}
	}
	
	else if (msg.header[0] == 'V' && msg.header[1] == ':') {
		if(msg.payload[0] == 'D') {
		  if(bCheckDrawCondition(au8Table)) {
				*currentState = IDLE;
			   Message msgSend;
			  vCreateMessage('K', 'V', 0, 0, &msgSend);
				vDelay(10);
			  vDataTransmit(COM_USART2, (uint8_t *)&msgSend, sizeof(msgSend), INTERRUPT);
				printf(" Game over, result: %c", 'D');
			}
			else {
			  //P:V
				*currentState = IDLE;
				printf(" Draw not confirmed");
			}
		}
		
		else if (msg.payload[0] == 'X' || msg.payload[0] == 'O') {
		  if(u8CheckWinCondition(au8Table) == msg.payload[0]) {
			  *currentState = IDLE;
			  Message msgSend;
			  vCreateMessage('K', 'V', 0, 0, &msgSend);
				vDelay(10);
			  vDataTransmit(COM_USART2, (uint8_t *)&msgSend, sizeof(msgSend), INTERRUPT);
				vDelay(1);
				printf(" Game over, result: %c", msg.payload[0]);
			}
			else {
				*currentState = IDLE;
				printf(" Win not confirmed");
			  //P:V
			}
		}
		
		else {
		  //P:V not valid P message
		  *currentState = IDLE;
			printf(" Not valid V message");
		}
	}

	
	else if (msg.header[0] == 'K' && msg.header[1] == ':' && msg.payload[0] == 'W') {
		printf(" Sent W message was valid, waiting for V message");
	}
	
	else if (msg.header[0] == 'K' && msg.header[1] == ':' && msg.payload[0] == 'V') {
		*currentState = IDLE;
		printf(" Sent W message checked out");
		uint8_t win = u8CheckWinCondition(au8Table);
		if(win) {
		  printf(" Game result: %c", win);
		}
		else {
		  printf(" Game result: %c", 'D');
		}
	}
	
	else if (msg.header[0] == 'P' && msg.header[1] == ':' && msg.payload[0] == 'W') {
		*currentState = IDLE;
		printf(" ");
		vFunction1(ERR_TABLE_DATA);
		printf(" Sent table was not valid, going to IDLE state");
	}
	
	else if (msg.header[0] == 'K' && msg.header[1] == ':' && msg.payload[0] == 'R') {
		*currentState = GAME_START;
		vInitializeTable(au8Table);
		moveCount = 0;
		vDelay(2000);
	}
	
	else if (msg.header[0] == 'R' && msg.header[1] == ':') {
		*currentState = RST_RECEIVED;
	}
	

}

bool bCheckCRC(Message msg) {
	//return true if crc inside received message equals calculated crc from header and payload of received message 
  return msg.crc == u32CalculateMessageCRC(msg);
	
}

void vInitializeTable(uint8_t table[]) {
  for (int i = 0; i < 9; i++) {
        table[i] = '*';
    }
}

void vPlaceRandomX(uint8_t table[]) {
	srand(HAL_GetTick());
  uint8_t index = rand() % 9;
  table[index] = 'X';
}

void vCompressTable(uint8_t table[], uint8_t compressedTable[]) {
	uint8_t tableCodes[9];
	for(int i = 0; i < 9; i++) {
	  if(table[i] == 'X') {
		  tableCodes[i] = 3;
		}
	  else if(table[i] == 'O') {
		  tableCodes[i] = 2;
		}
	  else if(table[i] == '*') {
		  tableCodes[i] = 1;
		}
		else {
		  tableCodes[i] = 0;
		}
	}
	
	compressedTable[0] = ((tableCodes[0] << 6) | (tableCodes[1] << 4) | (tableCodes[2] << 2));
	compressedTable[1] = ((tableCodes[3] << 6) | (tableCodes[4] << 4) | (tableCodes[5] << 2));
	compressedTable[2] = ((tableCodes[6] << 6) | (tableCodes[7] << 4) | (tableCodes[8] << 2));
}

void vDecompressTable(uint8_t compressedTable[], uint8_t table[]) {
  uint8_t tableCodes[9];

  tableCodes[0] = (compressedTable[0] >> 6) & 0x03;
  tableCodes[1] = (compressedTable[0] >> 4) & 0x03;
  tableCodes[2] = (compressedTable[0] >> 2) & 0x03;
  tableCodes[3] = (compressedTable[1] >> 6) & 0x03;
  tableCodes[4] = (compressedTable[1] >> 4) & 0x03;
  tableCodes[5] = (compressedTable[1] >> 2) & 0x03;
  tableCodes[6] = (compressedTable[2] >> 6) & 0x03;
  tableCodes[7] = (compressedTable[2] >> 4) & 0x03;
  tableCodes[8] = (compressedTable[2] >> 2) & 0x03;

  for (int i = 0; i < 9; i++) {
    if (tableCodes[i] == 3) {
      table[i] = 'X';
    } 
		else if (tableCodes[i] == 2) {
      table[i] = 'O';
    } 
		else if (tableCodes[i] == 1) {
      table[i] = '*';
    }
  }
}

void vCheckUserInputAllType1Messages(linear_buf lb, uint8_t *x , uint8_t *y, GameState *currentState) {
  if (strncmp((char*)lb.buf, "SPL:", 4) == 0  && lb.buf[5] == ',' && lb.buf[8] == '\n') {
		if(lb.buf[4] >= '1' && lb.buf[4] <= '3' && lb.buf[6] >= '1' && lb.buf[6] <= '3') {
			*x = lb.buf[4] - '0';
			*y = lb.buf[6] - '0';
		  if(au8Table[(*x-1)*3 + (*y-1)] == 'X' || au8Table[(*x-1)*3 + (*y-1)] == 'O') {
		    printf(" Please input again, not valid sign placement! ");
		    vFunction1(ERR_INVALID_SIGN_PLACEMENT);  
			}
			else {
			  *currentState = GAME_WAIT_SLAVE;  
			}
		  
		}
		else {
		  printf(" Please input again, not valid coordinates! ");
		  vFunction1(ERR_INVALID_COORDINATES);
		}
	}
	else if(strncmp((char*)lb.buf, "RST:", 4) == 0) {
	  *currentState = RST_SENT;
		vGPIOTogglePin(GPIOE, P10);
	  Message msgSend;
	  vCreateMessage('R', 0, 0, 0, &msgSend);
		vDataTransmit(COM_USART2, (uint8_t *)&msgSend, sizeof(msgSend), INTERRUPT);
		vDelay(10); 
	}
	
	else if(strncmp((char*)lb.buf, "UPD:", 4) == 0) {
	  vPrintTable(au8Table);
		vDelay(10); 
	}
	
	else {
	  printf(" Please input again, not valid user message! ");
		vFunction1(ERR_INVALID_USER_MESSAGE);
	} 
}

void vCheckUserInputRST(linear_buf lb, GameState *currentState) {
  if (strncmp((char*)lb.buf, "RST:", 4) == 0) {
		*currentState = RST_SENT;
		vGPIOTogglePin(GPIOE, P10);
	  Message msgSend;
	  vCreateMessage('R', 0, 0, 0, &msgSend);
		vDataTransmit(COM_USART2, (uint8_t *)&msgSend, sizeof(msgSend), INTERRUPT);
		vDelay(10);
	}
}

void vCheckUserInputSPL(linear_buf lb, uint8_t *x , uint8_t *y, GameState *currentState) {
  if (strncmp((char*)lb.buf, "SPL:", 4) == 0  && lb.buf[5] == ',' && lb.buf[8] == '\n') {
		if(lb.buf[4] >= '1' && lb.buf[4] <= '3' && lb.buf[6] >= '1' && lb.buf[6] <= '3') {
			*x = lb.buf[4] - '0';
			*y = lb.buf[6] - '0';
		  if(au8Table[(*x-1)*3 + (*y-1)] == 'X' || au8Table[(*x-1)*3 + (*y-1)] == 'O') {
		    printf(" Please input again, not valid sign placement! ");
		    vFunction1(ERR_INVALID_SIGN_PLACEMENT);  
			}
			else {
			  *currentState = GAME_WAIT_SLAVE;  
			}
		  
		}
		else {
		  printf(" Please input again, not valid coordinates! ");
		  vFunction1(ERR_INVALID_COORDINATES);
		}
	}
	else {
	  printf(" Please input again, not valid user message! ");
		vFunction1(ERR_INVALID_USER_MESSAGE);
	}
}

void vPrintTable(uint8_t table[]) {
	printf("\n");
  for (int i = 0; i < 9; i++) {
    printf("%c ", table[i]);
    if ((i + 1) % 3 == 0) {
      printf("\n");
    }
	}
}

uint8_t u8CheckWinCondition(uint8_t table[]) {
	
  uint8_t winCombos[8][3] = {
    {0, 1, 2}, {3, 4, 5}, {6, 7, 8},  // Rows
    {0, 3, 6}, {1, 4, 7}, {2, 5, 8},  // Columns
    {0, 4, 8}, {2, 4, 6}              // Diagonals
  };

  for (int i = 0; i < 8; i++) {
		//check if first sign is X or O, then check all signs for equality
    if (table[winCombos[i][0]] != '*' && table[winCombos[i][0]] == table[winCombos[i][1]] && table[winCombos[i][1]] == table[winCombos[i][2]]) {
      return table[winCombos[i][0]];
    }
  }
  return 0;
}

bool bCheckDrawCondition(uint8_t  table[]) {
  for (int i = 0; i < 9; i++) {
		//at least one * means no win
    if (table[i] == '*') {
      return false;
    }
  }
  return true;
}

bool bCheckTableFormat(uint8_t table[]) {
  for (int i = 0; i < 9; i++) {
    if (table[i] != 'X' && table[i] != 'O' && table[i] != '*') {
      return false;
    }
  }
  return true;
}

bool bCheckSignValidity(uint8_t oldTable[], uint8_t newTable[]) {
  uint8_t diffCount = 0;
  for (int i = 0; i < 9; i++) {
    if (oldTable[i] != newTable[i]) {
      if (oldTable[i] == '*' && (newTable[i] == 'X' || newTable[i] == 'O')) {
        diffCount++;
      } 
			else {
        return 0;
      }
    }
  }
	// Only one new sign can occur
  return diffCount == 1;
}
