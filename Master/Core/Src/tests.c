#include "tests.h"
#define TABLE_SIZE 9

typedef bool (*Test)(void);
	

// test for function vInitializeTable
static bool test1(void){
	
	uint8_t table[TABLE_SIZE];
	vInitializeTable(table);
	for (int i = 0; i < TABLE_SIZE; i++) {
	  if(table[i] != '*') {
		  return false;
		}
	}
	return true;
}

// test for function vCreateMessage
static bool test2(void){
	
  Message msg;
	vCreateMessage('T', 255, 127, 0, &msg);
	return msg.header[0] == 'T' && msg.header[1] == ':' && msg.payload[0] == 255 && msg.payload[1] == 127 && msg.payload[2] == 0;
}

// test for function vCreateMessage, it should handle NULL
static bool test3(void){
	
	vCreateMessage('T', 128, 63, 27, NULL);
	return true;
}

// test for function vPlaceRandomX
static bool test4(void){
	
  uint8_t table[TABLE_SIZE];
	vInitializeTable(table);
	vPlaceRandomX(table);
	uint8_t u8XCount = 0;
	for(int i = 0; i < TABLE_SIZE; i++) {
	  if(table[i] == 'X') {
		  u8XCount++;
		}
	}
	return u8XCount == 1;
}

// test for function vCompressTable
static bool test5(void){
	
  uint8_t table[TABLE_SIZE] = {'X', '*', '*', 'O', '*', '*', '*', 'O', 'X'};
	//it should consist of 0b1101 0100, 0b1001 0100 and 0b0110 1100
	uint8_t compressedTable[3];
	vCompressTable(table, compressedTable);
	return compressedTable[0] == 0xD4 && compressedTable[1] == 0x94 && compressedTable[2] == 0x6C;
}

// test for function vCompressTable - unexpected values
static bool test6(void){
	
  uint8_t table[TABLE_SIZE] = {'d', '-', '-', '*', '-', '-', 'a', 'b', 'c'};
	//it should consist of 0, 0b0100 0000 and 0
	uint8_t compressedTable[3];
	vCompressTable(table, compressedTable);
	return compressedTable[0] == 0x00 && compressedTable[1] == 0x40 && compressedTable[2] == 0x00;
}

// test for function vDecompressTable
static bool test7(void){
	
  uint8_t table[TABLE_SIZE] = {'*', '*', 'X', '*', 'O', '*', 'X', 'O', 'X'};
	uint8_t compressedTable[3] = {0x5C, 0x64, 0xEC};
	uint8_t decompressedTable[TABLE_SIZE];
	vDecompressTable(compressedTable, decompressedTable);
	for(int i = 0; i < TABLE_SIZE; i++) {
	  if (decompressedTable[i] != table[i]) {
		  return false;
		}
	}
	return true;
}

// test for function u8CheckWinCondition
static bool test8(void){
	
	// test multiple tables
  uint8_t table1[TABLE_SIZE] = {'*', '*', 'X', '*', 'O', '*', 'X', 'O', 'X'};
	uint8_t table2[TABLE_SIZE] = {'X', 'X', 'X', '*', 'O', '*', 'O', '*', '*'};
	uint8_t table3[TABLE_SIZE] = {'O', 'X', 'X', 'O', '*', '*', 'O', '*', 'X'};
	uint8_t table4[TABLE_SIZE] = {'*', 'O', 'X', '*', 'X', '*', 'X', '*', 'O'};
	
	uint8_t results[4] = {0, 'X', 'O', 'X'};
	uint8_t *tables[4] = {table1, table2, table3, table4};
	
	for (int i = 0; i < 4; i++) {
	  if(u8CheckWinCondition(tables[i]) != results[i]) {
		  return false;
		}
	}
	return true;
}

// test for function bCheckDrawCondition
// should improve function / code logic cause it only detects whether all characters are * right now
static bool test9(void){
	
	// test multiple tables
  uint8_t table1[TABLE_SIZE] = {'*', '*', 'X', '*', 'O', '*', 'X', 'O', 'X'};
	uint8_t table2[TABLE_SIZE] = {'X', 'X', 'X', 'O', 'O', 'X', 'O', 'X', 'O'};
	
	bool results[2] = {0, 1};
	uint8_t *tables[2] = {table1, table2};
	
	for (int i = 0; i < 2; i++) {
	  if(bCheckDrawCondition(tables[i]) != results[i]) {
		  return false;
		}
	}
	return true;
}

//test for function bCheckTableFormat
static bool test10(void){
	
	// test multiple tables
  uint8_t table1[TABLE_SIZE] = {'*', '*', 'X', '*', 'O', '*', 'X', 'O', 'X'};
	uint8_t table2[TABLE_SIZE] = {'X', 'b', 'X', 'O', 'O', 'X', 'O', 'X', 'O'};
	
	bool results[2] = {1, 0};
	uint8_t *tables[3] = {table1, table2};
	
	for (int i = 0; i < 2; i++) {
	  if(bCheckTableFormat(tables[i]) != results[i]) {
		  return false;
		}
	}
	return true;
}

//test for function bCheckTableFormat
static bool test11(void){
	
	// test multiple tables
  uint8_t oldTable1[TABLE_SIZE] = {'X', '*', 'X', '*', 'O', '*', 'X', 'O', '*'};
	uint8_t oldTable2[TABLE_SIZE] = {'X', '*', 'X', '*', 'O', '*', 'X', 'O', '*'};
	uint8_t newTable1[TABLE_SIZE] = {'X', 'O', 'X', '*', 'O', '*', 'X', 'O', '*'};
	uint8_t newTable2[TABLE_SIZE] = {'X', 'O', 'X', 'O', 'O', '*', 'X', 'O', '*'};
	
	bool results[2] = {1, 0};
	uint8_t *oldTables[2] = {oldTable1, oldTable2};
	uint8_t *newTables[2] = {newTable1, newTable2};
	
	for (int i = 0; i < 2; i++) {
	  if(bCheckSignValidity(oldTables[i], newTables[i]) == results[i]) {
		  return false;
		}
	}
	return true;
}

/*bool bCheckSignValidity(uint8_t oldTable[], uint8_t newTable[]) {
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
*/



Test allTests[NUM_OF_TESTS] = {test1, test2, test3, test4, test5, test6, test7, test8, test9, test10, test11};

bool runTests(void){
	for(uint8_t i = 0; i < 10; i++)
		if(!allTests[i]()) return false;
	return true;
}
