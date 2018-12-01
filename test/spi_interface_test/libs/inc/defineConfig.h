/*
 * MCU specific configurations.
 * 
 * This files contain the compile time configurations and device specific pins for the avr's peripherals. 
 */
 
#ifndef _DEV_CONFIG_H
#define _DEV_CONFIG_H 

#include <avr/io.h>

/* Peripheral pins for atmega328p */
#define SPI_PORT PORTB
#define SPI_DDR DDRB
#define SPI_PIN PINB
#define SPI_DD_SCK DDB5
#define SPI_DD_MISO DDB4
#define SPI_DD_MOSI DDB3
#define SPI_DD_SS DDB2
#define SPI_PIN_SS PB2

/* UART Buffers size */
#define UART_TX_BUFFER_SIZE 64
#define UART_RX_BUFFER_SIZE 64

#define LENGTH_OF_ARRAY(__ARRAY__) (sizeof(__ARRAY__)/sizeof((__ARRAY__)[0]))

#define STUB(x) (void)(x)

#endif /* _DEV_CONFIG_H */

