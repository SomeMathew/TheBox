#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "main.h"
#include "uart.h"
#include "defineConfig.h"
#include "command.h"

#define SERIAL_INPUT_BUFFER_SIZE 32

static void setup();
static void loop();
static void processSerialInput(void);

static void pong(char *);


static struct Command optList[] = {
  {"ping", pong, false}, // Alive check and debug
}; 

int main() {
	setup();
	while(1) {
		loop();
	}
	return 0;
}

void setup() {
	uart_open(UART_BAUD_RATE, UART_DIRECTION, UART_PARITY, UART_FRAME_SIZE, UART_STOPBIT);
	command_setup(optList, LENGTH_OF_ARRAY(optList));
	sei();
}

void loop() {
	processSerialInput();
}

/**
 * Reads the Serial Buffer if an input is available until a newline or the buffer is full, then execute any found commands.
 */
static void processSerialInput(void) {
	static char inputBuffer[SERIAL_INPUT_BUFFER_SIZE];
	static size_t inputBufferNextPos = 0;

	while (uart_available() > 0) {
		char nextChar = uart_read();
		if (inputBufferNextPos == SERIAL_INPUT_BUFFER_SIZE - 1 || nextChar == '\n') {
			// Execute the command if its a newline or we're about to overflow the buffer
			inputBuffer[inputBufferNextPos] = '\0';
			command_execute(inputBuffer, NULL);
			inputBufferNextPos = 0;
		} else if (nextChar != '\r') { // We disregard \r for simplicity and compatibility with different newline standards
			inputBuffer[inputBufferNextPos++] = nextChar;
		}
	}
}

/**
 * Responds to a ping request.
 */
static void pong(char * arg) {
	fprintf(&uartStream, "Pong!\n");
}
