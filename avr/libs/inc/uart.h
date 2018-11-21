/*
 * Simple uart driver for atmega328p. This driver uses a standard open/write/read interface.
 * The implementation is non-blocking and uses interrupts.
 * 
 * Usage:
 * 	Write - fprintf(&uartStream, ....)
 * 	Read - Check for available() bytes and then read each bytes.
 * 
 * The driver will fill an internal buffer when new bytes are available.
 * 		The buffer can be set by the user by defines before include of uart.h
 * 			UART_TX_BUFFER_SIZE
 * 			UART_RX_BUFFER_SIZE
 * 		Buffers defaults to 64 bytes.
 */
 
#ifndef _DEV_UART_H
#define _DEV_UART_H

#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>


/*
 * UART parameters
 */
#define UART_FRAME_SIZE_5BIT 0x0
#define UART_FRAME_SIZE_6BIT (_BV(UCSZ00))
#define UART_FRAME_SIZE_7BIT (_BV(UCSZ01))
#define UART_FRAME_SIZE_8BIT (_BV(UCSZ00) | _BV(UCSZ01))
 
#define UART_PARITY_NONE 0x0
#define UART_PARITY_EVEN (_BV(UPM01))
#define UART_PARITY_ODD (_BV(UPM01) | _BV(UPM00))

#define UART_DIRECTION_TX (_BV(TXEN0))
#define UART_DIRECTION_RX (_BV(RXEN0))
#define UART_DIRECTION_RXTX (UART_DIRECTION_TX | UART_DIRECTION_RX)

#define UART_STOP_1BIT 0x0
#define UART_STOP_2BIT (_BV(USBS0))

extern FILE uartStream;

/**
 * Initializes the UART device of the 328p with the configuration.
 * 
 * @param baudRate Clock speed for the UART comm
 * @param direction Select the mode for tx, rx or rxtx. (UART_DIRECTION_x defines)
 * @param parity Select parity mode. (UART_PARITY_x defines)
 * @param frameSize Select the frame size for comm (UART_FRAM_SIZE_x defines)
 * @param stopBit UART_STOP_x
 * 
 * @returns 0 on success
 */
int uart_open(uint32_t baudRate, uint8_t direction, uint8_t parity, uint8_t frameSize, uint8_t stopBit);

/**
 * Write a character to the UART.
 * 
 * This module should be used with the uartStream FILE object and fprintf.
 */
int uart_write(char c, FILE *stream);

/**
 * Returns the number of available bytes to be read.
 * 
 * @returns Number of bytes reading to be read.
 */
int uart_available();

/**
 * Reads the next byte from the UART input.
 * 
 * @returns -1 on error, the next byte otherwise
 */
int uart_read();

#endif /* _DEV_UART_H */
