﻿# TickTackToe

This project implements the classic Tick Tack Toe game using two STM32 boards (Master and Slave) that communicate over UART.

The two STM32 boards work together to manage game state and user input. The Master board plays as 'X' and the Slave as 'O.' A unique twist in this version is that the first 'X' placement is randomly assigned by the Master board, adding a layer of unpredictability to the classic game.

The system uses UART communication:

  * Between the boards to synchronize game state and check for winning conditions.
  * Between the boards and a computer to receive user input and display the game status.


