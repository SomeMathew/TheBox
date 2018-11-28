#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdint.h>
#include "main.h"
#include "uart.h"
#include "defineConfig.h"
#include "command.h"
#include "box_control.h"
#include "servo.h"

#define SERIAL_INPUT_BUFFER_SIZE 32

static void setup();
static void loop();
static void processSerialInput(void);

static void pong(char *);
static void moveA(char *);
static void moveB(char *);

static void open();
static void close();
static void lock();
static void unlock();

static void read();

static struct Command optList[] = {
  {"ping", pong, true}, // Alive check and debug
  {"moveA", moveA, true},
  {"moveB", moveB, true},
  {"open", open, false},
  {"close", close, false},
  {"lock", lock, false},
  {"unlock", unlock, false},
  {"read", read, false}
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
	box_init();
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

/**
 * Moves servo on channel A to desired angle
 */
static void moveA(char * arg) {
	int i = atoi(arg);
	servo_write(SERVO_CHANNELA, i);
}

/**
 * Moves servo on channel B to desired angle
 */
static void moveB(char * arg) {
	int i = atoi(arg);
	fprintf(&uartStream, " Moving %d\n", i);
	servo_write(SERVO_CHANNELB, i);
}

/**
 * Moves servo on channel A to desired angle
 */
static void read() {
	fprintf(&uartStream, "%ld\n", servo_read(SERVO_CHANNELB));
}

static void open() {
	box_open();
}

static void close() {
	box_close();
}

static void lock() {
	box_lock();
}

static void unlock() {
	box_unlock();
}