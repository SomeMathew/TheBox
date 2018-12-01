#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "uart.h"
#include "defineConfig.h"

#define BAUDRATE_REG_MAX 4095 // 12 bits register (2^12 - 1)
#define BAUDRATE_VALUE_MAX ((F_CPU)/16)
#define BAUDRATE_VALUE_MAX_DOUBLE ((F_CPU)/8)

#define UART_SINGLE_PRESCALER 16
#define UART_DOUBLE_PRESCALER 8

#if !defined(UART_TX_BUFFER_SIZE)
#define UART_TX_BUFFER_SIZE 64
#endif

#if !defined(UART_RX_BUFFER_SIZE)
#define UART_RX_BUFFER_SIZE 64
#endif

static char txBuffer[UART_TX_BUFFER_SIZE];
static char rxBuffer[UART_RX_BUFFER_SIZE];

static volatile size_t txBufferHead = 0;
static volatile size_t txBufferTail = 0;
static volatile size_t rxBufferHead = 0;
static volatile size_t rxBufferTail = 0;

static volatile bool rxOverflow = false;

FILE uartStream = FDEV_SETUP_STREAM(uart_write, NULL, _FDEV_SETUP_WRITE);

static int write(char c);

static inline void setBaudRateRegister(uint32_t baudRate) {
	uint16_t prescaler = UART_SINGLE_PRESCALER; // default single mode prescaler
	
	// constrain the baudRate to its maximum possible value
	if (baudRate > BAUDRATE_VALUE_MAX_DOUBLE) {
		baudRate = BAUDRATE_VALUE_MAX_DOUBLE;
	}
	
	// If the baudrate is higher than normal mode can do, set the double speed mode and reduce prescaler to 8.
	if (baudRate > BAUDRATE_VALUE_MAX) {
		prescaler = UART_DOUBLE_PRESCALER;
		UCSR0A |= _BV(U2X0); // double speed mode
	} else {
		UCSR0A &= _BV(U2X0);
	}
	
	uint16_t registerValue = (F_CPU/(prescaler * baudRate)) - 1;
	
	UBRR0L = registerValue & 0xFF; // LSB
	UBRR0H = (registerValue >> 8) & 0xF; // MSB
}

/*
 * This will initialize or change the uart device settings. 
 * Warning: Changing the baudrate will corrupt ongoing transmission. 
 */
int uart_open(uint32_t baudRate, uint8_t direction, uint8_t parity, uint8_t frameSize, uint8_t stopBit) {
	setBaudRateRegister(baudRate);
	UCSR0B = (UCSR0B & ~(_BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0))) | direction | _BV(RXCIE0);
	UCSR0C = (UCSR0B & ~(_BV(UCSZ01) | _BV(UCSZ00) | _BV(UPM01) | _BV(UPM00) | _BV(USBS0))) | parity | frameSize | stopBit;
	
	return 0;
}

int uart_write(char c, FILE *stream) {
	if (c == '\n') {
		uart_write('\r', stream);
	}	

	return write(c);
}

static int write(char c) {
	size_t i = (txBufferHead + 1) % UART_TX_BUFFER_SIZE;

	// Wait if buffer is full
	while (i == txBufferTail) {};

	txBuffer[txBufferHead] = c;

	ATOMIC_BLOCK(ATOMIC_FORCEON) {
	  UCSR0B |= _BV(UDRIE0);
	  txBufferHead = i;
	}

	return 1;
}

int uart_available() {
	return (UART_RX_BUFFER_SIZE + rxBufferHead - rxBufferTail) % UART_RX_BUFFER_SIZE;
}

int uart_read() {
	// Check if buffer is empty
	if (rxBufferHead == rxBufferTail) {
		return -1;
	} else {
		char c = rxBuffer[rxBufferTail];
		rxBufferTail = (rxBufferTail + 1) % UART_RX_BUFFER_SIZE;
		return c;
	}
}

/**
 * USART Rx Complete Interrupt handler, receives the next byte into the Rx buffer.
 */
ISR(USART_RX_vect) {
	// We know there is something in the rx buffer, read it
	char c = UDR0;
	size_t i = (rxBufferHead + 1) % UART_RX_BUFFER_SIZE;

	// Only write if we won't overflow the buffer, drop it otherwise
	if (i != rxBufferTail) {
		rxBuffer[rxBufferHead] = c;
		rxBufferHead = i;
	} else {
		rxOverflow = true;
	}
}

/**
 * USART TX Data register empty handler, send next byte in the Tx buffer.
 * 
 * 	This interrupt is managed to only be enabled when the tx buffer has data.
 */
ISR(USART_UDRE_vect) {
	char c = txBuffer[txBufferTail];
	txBufferTail = (txBufferTail + 1) % UART_TX_BUFFER_SIZE;
	UDR0 = c;

	// Stop the interrupt if no more data needs to be sent
	if (txBufferTail == txBufferHead) {
		UCSR0B &= ~_BV(UDRIE0);
	}
}
