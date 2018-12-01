#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "main.h"
#include "uart.h"
#include "defineConfig.h"
#include "pin_config.h"
#include "command.h"
#include "spi.h"
#include "ioctl.h"

#define SERIAL_INPUT_BUFFER_SIZE 32

static void setup();
static void loop();
static void processSerialInput(void);

static void pong(char *);

static void sendCmd(char * arg);
static void statusCheck(char * arg);

static struct Command optList[] = {
  {"ping", pong, true}, // Alive check and debug
  {"send", sendCmd, true},
  {"io", statusCheck, false}
}; 

struct spi_deviceConfig avrTester;

static void sendCmd(char * arg) {
	uint8_t i = atoi(arg);
	uint8_t rx = 0;
	spi_transmit(i, &rx);
	fprintf(&uartStream, "Sent %"PRIx8", Got %"PRIx8"\n", i, rx);
	spi_transmit(0, &rx);
	fprintf(&uartStream, "Sent %"PRIx8", Got %"PRIx8"\n", 0, rx);
}

static void statusCheck(char * arg) {
	bool gpioVal = ioctl_read(&BBB_STATUS_PIN, BBB_STATUS_IO);
	fprintf(&uartStream, "GPIO Status %d\n", gpioVal);
}


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
	spi_open_master(SPI_CONTROL_MASTER_POLL);
	spi_ioctl_setDevice(&avrTester, SPI_MODE_0, SPI_ORDER_MSB_FIRST, 5000000);
	spi_ioctl_selectDevice(&avrTester);
	
	ioctl_setdir(&BBB_STATUS_DDR, BBB_STATUS_IO, INPUT);
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
	uint8_t i = atoi(arg);
	fprintf(&uartStream, "Pong! %" PRIu8 "\n", i);
}
