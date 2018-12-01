#ifndef _DEV_MAIN_H
#define _DEV_MAIN_H

#include "uart.h"

/*
 * UART Configuration
 */
#define UART_BAUD_RATE 9600
#define UART_FRAME_SIZE UART_FRAME_SIZE_8BIT
#define UART_PARITY UART_PARITY_NONE
#define UART_DIRECTION UART_DIRECTION_RXTX
#define UART_STOPBIT UART_STOP_1BIT

/*
 * SPI Configuration
 */
#define SPI_FREQUENCY 100000

#endif /* _DEV_MAIN_H */
